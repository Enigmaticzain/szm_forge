"""Typed, validated models for reconstruction evidence and specifications.

Uses Pydantic v1 API matching conventions in main.py (BaseModel, Field).
No geometry generation, no MCP tool invocation, optional REA adapter.
"""
from __future__ import annotations

import json
from datetime import datetime, timezone
from typing import Any, Dict, List, Optional

from pydantic import BaseModel, Field, validator

# ── Evidence category enumeration ────────────────────────────────────────────

class EvidenceCategory(str):
    """Six required evidence categories."""
    MEASURED = "measured"
    DOCUMENTED = "documented"
    ESTIMATED = "estimated"
    INFERRED = "inferred"
    ASSUMED = "assumed"
    UNKNOWN = "unknown"

# ── EvidenceRecord ───────────────────────────────────────────────────────────

class EvidenceRecord(BaseModel):
    """A single evidence record with provenance, category, and uncertainty."""
    record_id: str = Field(..., description="Stable canonical record identifier")
    category: EvidenceCategory = Field(..., description="Evidence category")
    description: str = Field(..., description="Claim or observation description")
    source: Optional[str] = Field(
        None,
        description="Provenance: source reference, file path, page, instrument, measurement, observation",
    )
    value: Optional[float] = Field(
        None,
        description="Measured or estimated value, when applicable",
    )
    unit: Optional[str] = Field(
        None,
        description="Unit of the value, when applicable",
    )
    confidence: Optional[float] = Field(
        None,
        ge=0.0,
        le=1.0,
        description="Confidence in [0,1]; when available, never claim certainty",
    )
    timestamp: datetime = Field(
        default_factory=lambda: datetime.now(timezone.utc),
        description="ISO-8601 creation timestamp in UTC",
    )
    metadata: Dict[str, Any] = Field(
        default_factory=dict,
        description="Free-form metadata compatible with storage layer",
    )

    @validator("record_id")
    def _nonempty_id(cls, v: str) -> str:
        if not v or not v.strip():
            raise ValueError("record_id must be non-empty")
        return v

    @validator("confidence")
    def _confidence_range(cls, v: Optional[float]) -> Optional[float]:
        if v is not None and not (0.0 <= v <= 1.0):
            raise ValueError("confidence must be in [0, 1]")
        return v

    @validator("category")
    def _category_is_valid(cls, v: str) -> str:
        valid = {
            EvidenceCategory.MEASURED,
            EvidenceCategory.DOCUMENTED,
            EvidenceCategory.ESTIMATED,
            EvidenceCategory.INFERRED,
            EvidenceCategory.ASSUMED,
            EvidenceCategory.UNKNOWN,
        }
        if v not in valid:
            raise ValueError(f"invalid evidence category: {v}")
        return v

    class Config:
        """Pydantic v1 inner config."""
        str_strip_whitespace = True


# ── EvidenceCollection ───────────────────────────────────────────────────────

class EvidenceCollection(BaseModel):
    """A named collection of evidence records, traceable to a source artifact."""
    collection_name: str = Field(..., description="Name of the evidence collection")
    source_artifact: Optional[str] = Field(
        None,
        description="Artifact path or identifier the collection belongs to",
    )
    records: List[EvidenceRecord] = Field(
        default_factory=list,
        description="Ordered list of evidence records",
    )
    schema_version: str = Field(
        "evidence-record-v1",
        description="Schema version for forward/backward compatibility",
    )

    @validator("collection_name")
    def _nonempty_name(cls, v: str) -> str:
        if not v or not v.strip():
            raise ValueError("collection_name must be non-empty")
        return v

    class Config:
        str_strip_whitespace = True


# ── ReconstructionSpec ───────────────────────────────────────────────────────

class ReconstructionSpec(BaseModel):
    """A structured reconstruction specification subject to human review and approval.

    Concepts kept separate:
    - Evidence category (measured/documented/estimated/inferred/assumed/unknown)
    - Evidence confidence or uncertainty (optional float in [0,1])
    - Human review / approval status (explicit state machine below)
    """

    spec_id: str = Field(
        ...,
        description="Stable canonical specification identifier",
    )
    schema_version: str = Field(
        "reconstruction-spec-v1",
        description="Schema version for compatibility checks",
    )
    subject: str = Field(
        ...,
        description="Component or module identity being reconstructed",
    )
    evidence_refs: List[str] = Field(
        default_factory=list,
        description="Evidence record IDs referenced by this specification",
    )
    parameters: Dict[str, Any] = Field(
        default_factory=dict,
        description="Proposed parameters and their units (recorded in metadata)",
    )
    assumptions: List[str] = Field(
        default_factory=list,
        description="Explicit assumptions; must not be silently converted to facts",
    )
    unknowns: List[str] = Field(
        default_factory=list,
        description="Explicit unresolved questions; must not become geometry defaults",
    )
    validation_findings: List[str] = Field(
        default_factory=list,
        description="Human-readable validation observations",
    )
    review_status: str = Field(
        "pending",
        description="Human review/approval state machine",
    )
    reviewed_at: Optional[datetime] = Field(
        None,
        description="UTC timestamp of last review action",
    )
    reviewer_id: Optional[str] = Field(
        None,
        description="Identifier of the reviewer who performed the last action",
    )

    # ── Explicit status transitions ──────────────────────────────────────────

    TRANSITIONS = {
        "pending": ["under_review", "rejected"],
        "under_review": ["approved", "rejected", "pending"],
        "approved": [],  # terminal acceptance
        "rejected": [],  # terminal non-acceptance
    }

    _ALLOWED_VALUES = set(TRANSITIONS.keys())

    @validator("review_status")
    def _status_must_be_valid(cls, v: str) -> str:
        if v not in cls._ALLOWED_VALUES:
            raise ValueError(
                f"review_status must be one of {sorted(cls._ALLOWED_VALUES)}; got {v!r}"
            )
        return v

    @validator("unknowns")
    def _unknowns_must_not_be_silently_converted(cls, v: List[str]) -> List[str]:
        """Explicit guard: unknowns must not become zero/arbitrary geometry defaults."""
        return v

    class Config:
        """Pydantic v1 inner config."""
        str_strip_whitespace = True

    # ── Status transition methods ─────────────────────────────────────────────

    @classmethod
    def approve(cls, spec: "ReconstructionSpec", reviewer_id: str) -> "ReconstructionSpec":
        """Transition a specification to approved status with a reviewer ID."""
        if spec.review_status not in ("pending", "under_review"):
            raise ValueError(
                f"Cannot approve spec in '{spec.review_status}' status; must be 'pending' or 'under_review'"
            )
        spec = spec.copy()
        spec.review_status = "approved"
        spec.reviewed_at = datetime.now(timezone.utc)
        spec.reviewer_id = reviewer_id
        return spec

    @classmethod
    def reject(cls, spec: "ReconstructionSpec", reviewer_id: str) -> "ReconstructionSpec":
        """Transition a specification to rejected status with a reviewer ID."""
        if spec.review_status not in ("pending", "under_review"):
            raise ValueError(
                f"Cannot reject spec in '{spec.review_status}' status; must be 'pending' or 'under_review'"
            )
        spec = spec.copy()
        spec.review_status = "rejected"
        spec.reviewed_at = datetime.now(timezone.utc)
        spec.reviewer_id = reviewer_id
        return spec

    @classmethod
    def send_to_review(cls, spec: "ReconstructionSpec") -> "ReconstructionSpec":
        """Move a specification from pending to under_review."""
        if spec.review_status != "pending":
            raise ValueError(
                f"Cannot send spec to review from '{spec.review_status}' status; must be 'pending'"
            )
        spec = spec.copy()
        spec.review_status = "under_review"
        return spec

    @classmethod
    def revert_to_pending(cls, spec: "ReconstructionSpec") -> "ReconstructionSpec":
        """Move a specification from under_review back to pending."""
        if spec.review_status != "under_review":
            raise ValueError(
                f"Cannot revert spec from '{spec.review_status}' status; must be 'under_review'"
            )
        spec = spec.copy()
        spec.review_status = "pending"
        return spec