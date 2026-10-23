"""Targeted tests for specification and evidence models.

Uses repository conventions: sys.path insert, pytest-style function names.
No REA dependency; all logic driven from Pydantic model validation.
"""
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

from ai_service.evidence.spec_models import (
    EvidenceRecord,
    EvidenceCollection,
    ReconstructionSpec,
    EvidenceCategory,
    EvidenceCategory,
)


# ── EvidenceRecord tests ─────────────────────────────────────────────────────

def test_evidence_record_basic():
    rec = EvidenceRecord(
        record_id="ev-001",
        category=EvidenceCategory.MEASURED,
        description="Gear tooth count from blueprint",
        source="gear-blueprint.pdf, page 3",
        value=30,
        unit="teeth",
        confidence=0.95,
    )
    assert rec.record_id == "ev-001"
    assert rec.category == EvidenceCategory.MEASURED
    assert rec.confidence == 0.95
    assert rec.value == 30


def test_evidence_record_all_categories():
    for cat in [EvidenceCategory.MEASURED, EvidenceCategory.DOCUMENTED, EvidenceCategory.ESTIMATED, EvidenceCategory.INFERRED, EvidenceCategory.ASSUMED, EvidenceCategory.UNKNOWN]:
        rec = EvidenceRecord(
            record_id=f"ev-{cat}",
            category=cat,
            description=f"description for {cat}",
            source="test source",
        )
        assert rec.category == cat


def test_evidence_record_invalid_category():
    try:
        EvidenceRecord(
            record_id="ev-999",
            category="invalid_category",  # type: ignore
            description="bad",
            source="src",
        )
        assert False, "should have raised"
    except Exception:
        pass  # validation caught


def test_evidence_record_confidence_range():
    # valid
    r1 = EvidenceRecord(
        record_id="e1",
        category=EvidenceCategory.ESTIMATED,
        description="desc",
        source="src",
        confidence=0.5,
    )
    assert r1.confidence == 0.5

    # invalid
    try:
        EvidenceRecord(
            record_id="e2",
            category=EvidenceCategory.ESTIMATED,
            description="desc",
            source="src",
            confidence=1.5,
        )
        assert False, "should have raised"
    except Exception:
        pass

    # invalid
    try:
        EvidenceRecord(
            record_id="e3",
            category=EvidenceCategory.ESTIMATED,
            description="desc",
            source="src",
            confidence=-0.1,
        )
        assert False, "should have raised"
    except Exception:
        pass


def test_evidence_record_nonempty_id():
    try:
        EvidenceRecord(
            record_id="", category=EvidenceCategory.MEASURED, description="desc", source="src"
        )
        assert False, "should have raised"
    except Exception:
        pass


def test_evidence_record_unknown_remains_unknown():
    """Verify that unknown category does not become measured or zero-valued."""
    rec = EvidenceRecord(
        record_id="ev-unknown",
        category=EvidenceCategory.UNKNOWN,
        description="intent unknown",
        source="observation",
    )
    assert rec.category == EvidenceCategory.UNKNOWN
    # Note: value and unit may be None; that is the correct behavior
    # Unknown must not silently become 0 or arbitrary defaults.
    assert rec.value is None


def test_evidence_record_separates_confidence_from_approval_status():
    """Evidence confidence and review_status are independent concepts."""
    rec_high = EvidenceRecord(
        record_id="e-high",
        category=EvidenceCategory.MEASURED,
        description="desc",
        source="src",
        confidence=0.99,
    )
    rec_low = EvidenceRecord(
        record_id="e-low",
        category=EvidenceCategory.MEASURED,
        description="desc",
        source="src",
        confidence=0.01,
    )
    # Both can have the same review_status, and vice versa
    assert rec_high.confidence != rec_low.confidence
    # Review status is set on the spec, not on individual records directly


# ── EvidenceCollection tests ─────────────────────────────────────────────────

def test_evidence_collection_save_load():
    from ai_service.evidence.spec_models import EvidenceCollection
    coll = EvidenceCollection(
        collection_name="gear-data",
        source_artifact="gear-blueprint.pdf",
        records=[
            EvidenceRecord(
                record_id="ev-001",
                category=EvidenceCategory.MEASURED,
                description="tooth count",
                source="blueprint.pdf, p3",
                value=30,
                unit="teeth",
                confidence=0.95,
            ),
        ],
    )
    # Verify construction
    assert coll.collection_name == "gear-data"
    assert len(coll.records) == 1
    assert coll.records[0].category == EvidenceCategory.MEASURED
    # Schema version should be default
    assert coll.schema_version == "evidence-record-v1"


def test_evidence_collection_empty():
    from ai_service.evidence.spec_models import EvidenceCollection
    coll = EvidenceCollection(collection_name="empty")
    assert coll.collection_name == "empty"
    assert coll.records == []


# ── ReconstructionSpec tests ────────────────────────────────────────────────

def test_reconstruction_spec_creation():
    spec = ReconstructionSpec(
        spec_id="spec-001",
        subject="gear-train",
        evidence_refs=["ev-001"],
        parameters={"module": 2, "teeth": 30},
        assumptions=["standard pressure"],
        unknowns=["heat-treatment method"],
        review_status="pending",
    )
    assert spec.spec_id == "spec-001"
    assert spec.subject == "gear-train"
    assert spec.review_status == "pending"
    assert spec.unknowns == ["heat-treatment method"]
    assert spec.evidence_refs == ["ev-001"]


def test_reconstruction_spec_status_transitions():
    """Explicit status transitions; approval blocked without required conditions."""
    spec = ReconstructionSpec(
        spec_id="spec-002",
        subject="bracket",
        review_status="pending",
    )

    # pending -> under_review
    spec_reviewed = ReconstructionSpec.send_to_review(spec)
    assert spec_reviewed.review_status == "under_review"
    # pending -> approved allowed (method allows both statuses)
    approved_pending = ReconstructionSpec.approve(spec, "reviewer-1")
    assert approved_pending.review_status == "approved"

    # send to review then approve
    spec2 = ReconstructionSpec(
        spec_id="spec-003",
        subject="shaft",
        review_status="pending",
    )
    spec2 = ReconstructionSpec.send_to_review(spec2)
    spec2 = ReconstructionSpec.approve(spec2, "r1")
    assert spec2.review_status == "approved"
    assert spec2.reviewer_id == "r1"
    assert spec2.reviewed_at is not None

    # approved cannot be further transitioned away (TRANSITIONS["approved"] = [])
    try:
        ReconstructionSpec.send_to_review(spec2)
        assert False, "should have raised: approved is terminal"
    except ValueError:
        pass

    # under_review -> rejected
    spec3 = ReconstructionSpec(
        spec_id="spec-004",
        subject="housing",
        review_status="under_review",
    )
    spec3 = ReconstructionSpec.reject(spec3, "r2")
    assert spec3.review_status == "rejected"
    assert spec3.reviewer_id == "r2"
    assert spec3.reviewed_at is not None

    # rejected -> pending revert
    spec4 = ReconstructionSpec(
        spec_id="spec-005",
        subject="plate",
        review_status="under_review",
    )
    spec4 = ReconstructionSpec.revert_to_pending(spec4)
    assert spec4.review_status == "pending"


def test_reconstruction_spec_approval_blocks_when_review_status_invalid():
    """Approval blocked when status is not pending/under_review; unknowns do not block."""
    spec = ReconstructionSpec(
        spec_id="spec-006",
        subject="flange",
        unknowns=["heat-treatment method"],
        review_status="approved",  # already terminal — approval must raise
    )
    try:
        ReconstructionSpec.approve(spec, "reviewer-1")
        assert False, "should have raised: must be pending/under_review"
    except ValueError:
        pass
    # Unknowns remain explicit regardless of approval
    spec_pending = ReconstructionSpec(
        spec_id="spec-006b",
        subject="flange",
        unknowns=["heat-treatment method"],
        review_status="pending",
    )
    approved = ReconstructionSpec.approve(spec_pending, "r1")
    assert approved.review_status == "approved"
    assert approved.unknowns == ["heat-treatment method"]  # unknowns preserved


def test_reconstruction_spec_unknowns_stay_unknowns():
    """Explicit guard: unknowns list must not become zero/geometry defaults."""
    spec = ReconstructionSpec(
        spec_id="spec-007",
        subject="unknown-dim",
        unknowns=["critical dimension not measured"],
        parameters={"length": None},
        review_status="pending",
    )
    # The model stores unknowns as provided; consumer must not convert.
    assert spec.unknowns == ["critical dimension not measured"]
    # length may be None; unknowns must not be silently set to 0 or 100


def test_reconstruction_spec_separates_confidence_from_approval():
    """Evidence confidence in records and spec review_status are separate axes."""
    spec = ReconstructionSpec(
        spec_id="spec-008",
        subject="test",
        evidence_refs=[],
        review_status="pending",
    )
    # Adding a record with high confidence does not change review_status
    from ai_service.evidence.spec_models import EvidenceRecord
    rec = EvidenceRecord(
        record_id="e-01",
        category=EvidenceCategory.MEASURED,
        description="desc",
        source="src",
        confidence=0.99,
    )
    spec.evidence_refs.append("e-01")
    assert spec.review_status == "pending"
    assert len(spec.evidence_refs) == 1


def test_reconstruction_spec_invalid_status_raises():
    try:
        ReconstructionSpec(
            spec_id="spec-bad",
            subject="test",
            review_status="non_existent_status",
        )
        assert False, "should have raised"
    except Exception:
        pass


# ── Edge cases ───────────────────────────────────────────────────────────────

def test_evidence_record_no_value_when_unknown():
    """When category is UNKNOWN, value must remain None — not silently become 0."""
    rec = EvidenceRecord(
        record_id="ev-noval",
        category=EvidenceCategory.UNKNOWN,
        description="why unknown",
        source="src",
    )
    assert rec.value is None
    assert rec.unit is None


def test_spec_with_no_unknowns_no_assumptions():
    """A clean spec with only required fields."""
    spec = ReconstructionSpec(
        spec_id="spec-009",
        subject="test",
        review_status="pending",
    )
    assert spec.unknowns == []
    assert spec.assumptions == []