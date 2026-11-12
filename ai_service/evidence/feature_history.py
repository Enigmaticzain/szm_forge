"""Parametric feature history — ordered CAD operations with provenance.
Uses Phase 1 evidence_store (JSON). Keeps REA optional. No new DB.
Constraints solved: documented by evidence category; full geometric solver unavailable (Limited to build123d primitives).
"""
from __future__ import annotations
import os, tempfile, json
from typing import Any, Dict, List, Optional, Tuple, Set
from .evidence_store import EvidenceStore, EvidenceStoreError
from .spec_models import ReconstructionSpec

class FeatureRecord:
    feature_id: str
    feature_type: str  # sketch, extrude, boolean-cut, fillet
    upstream_ids: List[str]
    parameters: Dict[str, Any]
    provenance: Dict[str, Any]  # parameter -> category (measured/documented/etc)
    status: str  # requested, validated, generated, failed
    validation_findings: List[str]
    unknowns: List[str]
    assumptions: List[str]

    def __init__(self, feature_id: str, feature_type: str, upstream_ids=None, parameters=None, provenance=None, status="requested"):
        self.feature_id = feature_id
        self.feature_type = feature_type
        self.upstream_ids = upstream_ids or []
        self.parameters = parameters or {}
        self.provenance = provenance or {}
        self.status = status
        self.validation_findings = []
        self.unknowns = []
        self.assumptions = []

class FeatureHistory:
    def __init__(self, history_id: str, store_path: Optional[str] = None):
        self.history_id = history_id
        self.store = EvidenceStore(store_path) if store_path else EvidenceStore()
        self.features: List[FeatureRecord] = []
        self._feature_map: Dict[str, FeatureRecord] = {}
    
    def add_feature(self, record: FeatureRecord) -> None:
        if record.feature_id in self._feature_map:
            raise ValueError(f"Duplicate feature_id: {record.feature_id}")
        self.features.append(record)
        self._feature_map[record.feature_id] = record
    
    def get_feature(self, feature_id: str) -> FeatureRecord:
        if feature_id not in self._feature_map:
            raise ValueError(f"Feature not found: {feature_id}")
        return self._feature_map[feature_id]
    
    def detect_cycle(self) -> List[str]:
        visited = set()
        path = []
        def visit(fid):
            if fid in path:
                return path[path.index(fid):]
            if fid in visited:
                return []
            visited.add(fid)
            path.append(fid)
            f = self._feature_map.get(fid)
            if f:
                for up in f.upstream_ids:
                    cycle = visit(up)
                    if cycle:
                        return cycle
            path.pop()
            return []
        for f in self.features:
            cycle = visit(f.feature_id)
            if cycle:
                return cycle
        return []
    
    def detect_missing_dependencies(self) -> List[str]:
        ids = set(self._feature_map.keys())
        missing = []
        for f in self.features:
            for up in f.upstream_ids:
                if up not in ids:
                    missing.append(up)
        return sorted(set(missing))
    
    def invalidate_downstream(self, edited_feature_id: str) -> List[str]:
        # Simple downstream invalidation: any feature that references edited_feature_id
        # must be marked failed/unknown until regenerated.
        downstream = []
        for f in self.features:
            if edited_feature_id in f.upstream_ids:
                f.status = "failed"
                f.unknowns.append(f"Upstream {edited_feature_id} edited — downstream invalidated")
                downstream.append(f.feature_id)
        return sorted(set(downstream))
    
    def regenerate(self, feature_id: str = None) -> Dict[str, Any]:
        # Order by upstream dependency; generate only when all upstream generated
        order = self._topological_order()
        result = {"features": {}, "findings": [], "status": "pending"}
        for fid in order:
            f = self._feature_map.get(fid)
            if not f:
                result["findings"].append(f"Feature {fid} missing")
                continue
            # Check upstream validated or generated
            upstream_ok = all(
                self._feature_map[up].status in ("validated", "generated")
                if up in self._feature_map
                else False
                for up in f.upstream_ids
            )
            if not upstream_ok:
                f.status = "failed"
                result["findings"].append(f"Feature {fid} blocked: upstream not generated")
                continue
            # Only validate; do NOT claim solved constraints (honest limitation)
            f.status = "validated"  # Ready for geometry build (not automatically generated here)
            result["features"][fid] = {
                "feature_type": f.feature_type,
                "parameters": f.parameters,
                "provenance": f.provenance,
                "status": f.status,
            }
        result["status"] = "validated" if not result["findings"] else "partial"
        return result
    
    def _topological_order(self) -> List[str]:
        # Simple order: upstream first, using feature_id sequence
        # For rectangle with 1 feature, just the feature itself
        ordered = []
        visited = set()
        def visit(fid):
            if fid in visited: return
            visited.add(fid)
            f = self._feature_map.get(fid)
            if f:
                for up in f.upstream_ids:
                    visit(up)
            ordered.append(fid)
        for f in self.features:
            visit(f.feature_id)
        return ordered
    
    def save(self, path: Optional[str] = None) -> None:
        path = path or (self.store.path.replace("evidence_store.json", "feature_history.json") if self.store.path else "/tmp/feature_history.json")
        data = {
            "version": "feature-history-v1",
            "history_id": self.history_id,
            "features": [
                {
                    "feature_id": f.feature_id,
                    "feature_type": f.feature_type,
                    "upstream_ids": f.upstream_ids,
                    "parameters": f.parameters,
                    "provenance": f.provenance,
                    "status": f.status,
                    "validation_findings": f.validation_findings,
                    "unknowns": f.unknowns,
                    "assumptions": f.assumptions,
                }
                for f in self.features
            ],
        }
        import tempfile
        fd, temp_path = tempfile.mkstemp(dir=os.path.dirname(path) or ".", prefix="fh_tmp_")
        try:
            with os.fdopen(fd, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2, default=str)
                f.write("\n")
            os.rename(temp_path, path)
        except Exception:
            try: os.unlink(temp_path)
            except OSError: pass
            raise
    
    def load(self, path: Optional[str] = None) -> None:
        path = path or (self.store.path.replace("evidence_store.json", "feature_history.json") if self.store.path else "/tmp/feature_history.json")
        if not os.path.isfile(path):
            return
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        for feat in data.get("features", []):
            rec = FeatureRecord(
                feature_id=feat.get("feature_id"),
                feature_type=feat.get("feature_type"),
                upstream_ids=feat.get("upstream_ids", []),
                parameters=feat.get("parameters", {}),
                provenance=feat.get("provenance", {}),
                status=feat.get("status", "requested"),
            )
            rec.validation_findings = feat.get("validation_findings", [])
            rec.unknowns = feat.get("unknowns", [])
            rec.assumptions = feat.get("assumptions", [])
            self.add_feature(rec)
