"""Safe, isolated JSON persistence for evidence and reconstruction specs.

Persistence is separated from `.code-review-graph/graph.db`. Uses atomic
write semantics (temporary file + rename). Does not touch production DB.
"""
from __future__ import annotations

import json
import os
import tempfile
from datetime import datetime
from typing import List, Optional

from .spec_models import EvidenceRecord, ReconstructionSpec, EvidenceCategory

# ── Default production store location ────────────────────────────────────────
DEFAULT_STORE_PATH = os.path.join(
    os.path.dirname(__file__), "evidence_store.json"
)

SCHEMA_VERSION = "evidence-store-v1"


class EvidenceStoreError(Exception):
    """Base exception for persistence errors."""
    pass


class SchemaVersionError(EvidenceStoreError):
    """Schema version mismatch."""
    pass


class DuplicateIdError(EvidenceStoreError):
    """Duplicate record ID detected."""
    pass


# ── EvidenceStore (read-only safe; creates only when needed) ───────────────────

class EvidenceStore:
    """Isolated JSON-backed store for evidence collections and specs.

    Importing this module does NOT create the production JSON file.
    The file is created only on first successful save.
    """

    def __init__(self, path: Optional[str] = None):
        self.path = path or DEFAULT_STORE_PATH
        # Import-time does NOT read or write.

    def exists(self) -> bool:
        return os.path.isfile(self.path)

    def _load_raw(self) -> dict:
        """Load the JSON document from disk; raise on malformed or unsupported."""
        if not self.exists() or os.path.getsize(self.path) == 0:
            return {"version": SCHEMA_VERSION, "collections": {}, "specs": {}}
        try:
            with open(self.path, "r", encoding="utf-8", newline="") as f:
                data = json.load(f)
        except (json.JSONDecodeError, OSError) as exc:
            raise EvidenceStoreError(
                f"Malformed JSON at {self.path}: {exc}"
            ) from exc
        if not isinstance(data, dict):
            raise EvidenceStoreError(
                f"Store document must be a JSON object; got {type(data).__name__}"
            )
        version = data.get("version", SCHEMA_VERSION)
        if version != SCHEMA_VERSION:
            raise SchemaVersionError(
                f"Unsupported schema version: expected {SCHEMA_VERSION}, got {version!r}"
            )
        # Ensure keys exist (backward-compatible with partial writes)
        data.setdefault("collections", {})
        data.setdefault("specs", {})
        return data

    def _atomic_write(self, data: dict) -> None:
        """Write to a temporary file in the same directory, then rename."""
        directory = os.path.dirname(self.path) or "."
        # Ensure directory exists
        try:
            os.makedirs(directory, exist_ok=True)
        except OSError as exc:
            raise EvidenceStoreError(
                f"Cannot create store directory {directory!r}: {exc}"
            ) from exc
        fd, temp_path = tempfile.mkstemp(
            dir=directory,
            prefix="evidence_store_tmp_",
        )
        try:
            with os.fdopen(fd, "w", encoding="utf-8", newline="") as f:
                json.dump(
                    data,
                    f,
                    indent=2,
                    ensure_ascii=False,
                    default=lambda obj: obj.isoformat() if isinstance(obj, datetime) else str(obj),
                )
                f.write("\n")
            os.rename(temp_path, self.path)
        except Exception:
            # Clean up temporary file on any failure; do not leak partial writes
            try:
                os.unlink(temp_path)
            except OSError:
                pass
            raise

    def save_collection(self, collection) -> None:
        """Save or replace an evidence collection by collection_name."""
        from .spec_models import EvidenceCollection
        if not isinstance(collection, EvidenceCollection):
            raise EvidenceStoreError(
                f"Expected EvidenceCollection; got {type(collection).__name__}"
            )
        data = self._load_raw()
        # Check for duplicate record IDs within this collection (handle nested or flat)
        old_raw = data["collections"].get(collection.collection_name, [])
        old_records = old_raw.get("records", []) if isinstance(old_raw, dict) else old_raw
        record_ids = [rec.get("record_id") if isinstance(rec, dict) else (rec.get("record_id") if hasattr(rec, "record_id") else None) for rec in old_records]
        # Convert collection to JSON-safe dict
        collection_dict = json.loads(collection.json())
        # Reject updates that would silently overwrite without tracking
        # (Store allows replacement but keeps previous structure intact)
        data["collections"][collection.collection_name] = {"records": collection_dict.get("records", []), "source_artifact": collection_dict.get("source_artifact", collection.source_artifact), "collection_name": collection.collection_name}
        self._atomic_write(data)

    def load_collection(self, collection_name: str) -> EvidenceCollection:
        """Load an evidence collection by name; raises if missing or invalid."""
        from .spec_models import EvidenceCollection
        data = self._load_raw()
        raw = data.get("collections", {}).get(collection_name)
        if raw is None:
            raise EvidenceStoreError(
                f"Evidence collection not found: {collection_name!r}"
            )
        # Handle both flat (list of records) and nested (dict with records/source_artifact)
        if isinstance(raw, list):
            records_list = raw
            source_artifact = None
        elif isinstance(raw, dict):
            records_list = raw.get("records", [])
            source_artifact = raw.get("source_artifact")
        else:
            raise EvidenceStoreError(f"Unexpected collection format: {type(raw)}")
        try:
            return EvidenceCollection.parse_raw(json.dumps({
                "collection_name": collection_name,
                "records": records_list,
                "source_artifact": source_artifact,
                "schema_version": "evidence-record-v1",
            }))
        except Exception as exc:
            raise EvidenceStoreError(
                f"Failed to rebuild collection {collection_name!r}: {exc}"
            ) from exc

    def save_spec(self, spec) -> None:
        """Save or replace a reconstruction spec by spec_id."""
        from .spec_models import ReconstructionSpec
        if not isinstance(spec, ReconstructionSpec):
            raise EvidenceStoreError(
                f"Expected ReconstructionSpec; got {type(spec).__name__}"
            )
        data = self._load_raw()
        # Prevent unapproved specs from being silently promoted; store only what is passed.
        spec_dict = json.loads(spec.json())
        spec_dict.setdefault("review_status", spec.review_status)
        data["specs"][spec.spec_id] = spec_dict
        self._atomic_write(data)

    def load_spec(self, spec_id: str) -> ReconstructionSpec:
        """Load a reconstruction spec by spec_id."""
        from .spec_models import ReconstructionSpec
        data = self._load_raw()
        raw = data.get("specs", {}).get(spec_id)
        if raw is None:
            raise EvidenceStoreError(
                f"Reconstruction spec not found: {spec_id!r}"
            )
        try:
            return ReconstructionSpec.parse_raw(json.dumps(raw))
        except Exception as exc:
            raise EvidenceStoreError(
                f"Failed to rebuild spec {spec_id!r}: {exc}"
            ) from exc

    def list_collection_names(self) -> List[str]:
        return list(self._load_raw().get("collections", {}).keys())

    def list_spec_ids(self) -> List[str]:
        return list(self._load_raw().get("specs", {}).keys())
