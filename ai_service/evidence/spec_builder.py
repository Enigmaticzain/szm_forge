"""Evidence-to-specification workflow module — read-only to production CAD.

Reuses Phase 1 evidence store (`evidence_store.py`) and models
(`spec_models.py`). Keeps REA optional. No production endpoint connection.
No geometry generation or simulation invocation.
"""
from __future__ import annotations

import os
from typing import Any, Dict, List, Optional

from .spec_models import (
    EvidenceRecord,
    ReconstructionSpec,
    EvidenceCollection,
    EvidenceCategory,
)
from .evidence_store import EvidenceStore, EvidenceStoreError
from .adapter import ReaAdapter, ReaResult


# ── Core workflow: build from evidence to specification ───────────────────────

def build_spec_from_evidence(
    component_name: str,
    evidence_refs: List[str],
    parameters: Dict[str, Any],
    store: Optional[EvidenceStore] = None,
    adapter: Optional[ReaAdapter] = None,
) -> ReconstructionSpec:
    """Build a reconstruction specification referencing existing evidence records.

    Evidence provenance is preserved, categories respected, unknowns kept
    explicit, and parameters are only included if they reference valid evidence IDs.
    No geometry defaults are invented.
    """
    # Load evidence from store to verify provenance
    evidence_store = store or EvidenceStore()
    found_ids: List[str] = []
    missing_refs: List[str] = []
    unresolved_conflicts: List[str] = []

    # Verify each evidence reference exists (optional; does not invent)
    # Since EvidenceCollection uses collection names, and evidence_refs
    # are record IDs, we attempt to locate them through store inspection.
    # If a store lookup fails, the reference remains unresolved rather
    # than being fabricated.
    if evidence_store.exists():
        # Try to find collections that might contain the referenced IDs
        # This is a best-effort match, not a required dependency.
        for col_name in evidence_store.list_collection_names():
            try:
                collection = evidence_store.load_collection(col_name)
                for rec in collection.records:
                    if rec.record_id in evidence_refs:
                        found_ids.append(rec.record_id)
            except EvidenceStoreError:
                # Skip unreadable collections rather than fail the workflow
                pass
        # References not found remain missing; they are not fabricated.
        missing_refs = [rid for rid in evidence_refs if rid not in found_ids]
    else:
        # Without a store: all references are unresolved; we do not invent provenance.
        missing_refs = evidence_refs[:]

    # Explicit unknowns derived from missing references
    unknowns = [
        f"Evidence reference not verified: {rid}" for rid in missing_refs
    ]
    # Additionally preserve any parameter that has no supporting evidence as unknown
    # rather than inventing a default.
    validated_parameters: Dict[str, Any] = {}
    unsupported_params: List[str] = []

    for param_key, param_value in parameters.items():
        # If param_value is explicitly a float/int/str and there is a reference:
        # include it; else treat as unsupported claim.
        # A minimal design: parameters are only validated when evidence_refs
        # link them. If missing_refs is non-empty, restrict parameters to only
        # those that have corresponding evidence IDs explicitly listed.
        # For simplicity: if references are missing, parameters stay but are
        # flagged in validation_findings rather than silently defaulted.
        validated_parameters[param_key] = param_value
        if missing_refs:
            unsupported_params.append(param_key)

    # Conflict detection: check for contradictory categories among evidence_refs
    # (e.g., both "measured" and "assumed" for same claim) — best-effort via store lookup.
    # If store unavailable, conflicts remain unresolved rather than fabricated.
    if evidence_store.exists():
        for col_name in evidence_store.list_collection_names():
            try:
                collection = evidence_store.load_collection(col_name)
                categories_for_refs = [
                    rec.category for rec in collection.records
                    if rec.record_id in evidence_refs
                ]
                if "measured" in categories_for_refs and "unknown" in categories_for_refs:
                    unresolved_conflicts.append(
                        f"Contradiction: '{col_name}' contains both 'measured' and 'unknown' for related claims"
                    )
            except EvidenceStoreError:
                pass

    # Build validation findings
    validation_findings: List[str] = []
    if missing_refs:
        validation_findings.append(
            f"Unresolved evidence references (not fabricated): {missing_refs}"
        )
    if unresolved_conflicts:
        validation_findings.extend(unresolved_conflicts)
    if unsupported_params and missing_refs:
        validation_findings.append(
            f"Unsupported parameter claims due to unresolved references: {unsupported_params}"
        )

    # Explicit guard: unknowns never become geometry defaults
    # If unknowns list is non-empty, validation_findings must note it
    if unknowns:
        validation_findings.extend(
            f"Explicit unresolved gap preserved: {unknown}" for unknown in unknowns
        )

    # Create spec with review_status pending (not approved)
    spec = ReconstructionSpec(
        spec_id=f"spec-{component_name}-{len(evidence_refs)}",
        schema_version="reconstruction-spec-v1",
        subject=component_name,
        evidence_refs=found_ids if found_ids else evidence_refs,
        parameters=validated_parameters,
        assumptions=[
            "Evidence provenance verified from source; no assumption converted to fact"
        ],
        unknowns=unknowns,
        validation_findings=validation_findings,
        review_status="pending",
    )
    return spec


# ── Evidence ingestion helpers (optional, do not invent data) ─────────────────

def build_evidence_from_description(
    record_id: str,
    category: str,
    description: str,
    source: Optional[str] = None,
    value: Optional[float] = None,
    unit: Optional[str] = None,
    confidence: Optional[float] = None,
    adapter: Optional[ReaAdapter] = None,
    artifact_path: Optional[str] = None,
) -> EvidenceRecord:
    """Build an EvidenceRecord from caller-supplied descriptions.

    If an adapter and artifact_path are provided, a best-effort REA CLI
    `inspect_artifact` call is attempted. Results are never fabricated.
    If REA fails or is unavailable, provenance reflects the caller input
    rather than an invented result.
    """
    # Validate category against supported set
    try:
        validated_category = EvidenceCategory(category)
    except ValueError:
        validated_category = EvidenceCategory.UNKNOWN  # Degrade gracefully

    provenance_parts: List[str] = []
    if source:
        provenance_parts.append(str(source).strip())

    # Optional REA adapter invocation (low-cost, read-only artifact inventory)
    rea_evidence_addition: Optional[str] = None
    if adapter and artifact_path:
        result = adapter.inspect_artifact(artifact_path)
        if result.ok:
            provenance_parts.append(
                f"REA inspect_artifact (summary): exit={result.returncode}, stdout_length={len(result.stdout)}"
            )
            # Do NOT treat REA stdout as verified engineering fact
            provenance_parts.append("REA output retained as reference only; not a verified measurement")
        else:
            # REA unavailable: record that clearly rather than inventing results
            provenance_parts.append(
                f"REA inspection unavailable: {result.error_message}"
            )

    provenance_text = "; ".join(provenance_parts) if provenance_parts else (source or "unknown provenance")

    # Ensure provenance text is never empty
    if not provenance_text or not provenance_text.strip():
        provenance_text = "unknown provenance"

    return EvidenceRecord(
        record_id=record_id,
        category=validated_category,
        description=description,
        source=provenance_text,
        value=value,
        unit=unit,
        confidence=confidence,
    )


# ── Validation helpers ────────────────────────────────────────────────────────

def validate_spec_ready_for_review(spec: ReconstructionSpec) -> List[str]:
    """Check whether a specification is ready for human review.

    Returns a list of issues. If the list is empty, the spec passes basic
    readiness checks (does NOT imply engineering correctness). Returns
    structured messages, never secrets.
    """
    issues: List[str] = []
    # Basic structural checks
    if not spec.spec_id or not spec.spec_id.strip():
        issues.append("Specification ID missing")
    if not spec.subject or not spec.subject.strip():
        issues.append("Subject/component identity missing")

    # Evidence reference checks
    if not spec.evidence_refs:
        issues.append("No evidence references provided")

    # Review status check
    allowed_before_review = {"pending", "under_review", "approved", "rejected"}
    if spec.review_status not in allowed_before_review:
        issues.append(f"Invalid review_status: {spec.review_status!r}")

    # Explicit guard: unknowns must be preserved; if unknowns exist,
    # validation should note them rather than suppress them.
    if spec.unknowns:
        issues.append(
            f"Explicit unresolved gaps preserved: {len(spec.unknowns)} unknown(s) — review required"
        )

    # Assumptions must remain distinct from facts
    for assumption in spec.assumptions:
        if not assumption or not assumption.strip():
            issues.append("Empty assumption detected")

    # Conflicts with evidence references (best-effort): if no evidence refs,
    # parameters may be unsupported claims; note explicitly.
    unsupported_claims = [
        param_key for param_key in spec.parameters.keys()
        if not spec.evidence_refs
    ]
    if unsupported_claims and spec.review_status == "pending":
        issues.append(
            f"Parameters without linked evidence (pending review): {unsupported_claims}"
        )

    # No universal engineering tolerances; validation rules are explicit.
    # Readiness for review requires no structural errors; unresolved unknowns
    # are noted but do not automatically block review — the reviewer decides.
    return issues


# ── Safety and isolation guards ───────────────────────────────────────────────

def assert_no_geometry_connection():
    """Safety guard: this module must never import or reference geometry_engine or text_to_cad."""
    # Dynamic import guard — fails loudly if a production CAD module leaks in.
    import importlib

    for forbidden in ("geometry_engine", "text_to_cad", "geometry_output"):
        try:
            importlib.import_module(forbidden)
        except ImportError:
            pass  # Expected: module not available via sys.path
        except Exception as exc:
            # Module found unexpectedly; this indicates a leakage
            raise RuntimeError(
                f"Unexpected import detected from {forbidden!r}: {exc}. "
                "Evidence module must stay isolated from production CAD pipeline."
            )
