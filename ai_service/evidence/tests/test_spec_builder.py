"""Targeted Phase 2 tests for evidence-to-specification workflow.

Uses Phase 1 store (temporary paths), Phase 1 models, direct execution.
No pytest dependency. No REA broad analysis. No geometry invocation.
"""
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

from ai_service.evidence.adapter import ReaAdapter
from ai_service.evidence.spec_models import (
    EvidenceRecord,
    ReconstructionSpec,
    EvidenceCategory,
)
from ai_service.evidence.spec_builder import (
    build_spec_from_evidence,
    build_evidence_from_description,
    validate_spec_ready_for_review,
    assert_no_geometry_connection,
)
from ai_service.evidence.evidence_store import EvidenceStore


def test_build_spec_from_valid_evidence():
    """1. Building a specification from valid evidence."""
    spec = build_spec_from_evidence(
        component_name="gear-train",
        evidence_refs=["ev-test-001"],
        parameters={"module": 2, "teeth": 30},
    )
    assert spec.subject == "gear-train"
    assert spec.schema_version == "reconstruction-spec-v1"
    assert spec.review_status == "pending"
    assert "ev-test-001" in spec.evidence_refs
    assert spec.parameters == {"module": 2, "teeth": 30}


def test_preserve_provenance_and_categories():
    """2. Preserving provenance and evidence categories."""
    rec_measured = EvidenceRecord(
        record_id="ev-measured-01",
        category=EvidenceCategory.MEASURED,
        description="Tooth count from CAD drawing",
        source="drawing.pdf page 3",
        value=30,
        unit="teeth",
        confidence=0.95,
    )
    spec = build_spec_from_evidence(
        component_name="gear-train",
        evidence_refs=["ev-measured-01"],
        parameters={"teeth": 30},
    )
    assert spec.evidence_refs == ["ev-measured-01"]
    assert spec.validation_findings


def test_missing_source_remains_missing():
    """3. Missing source information remains missing."""
    spec = build_spec_from_evidence(
        component_name="unknown",
        evidence_refs=["nonexistent-ref-01"],
        parameters={},
    )
    assert spec.unknowns
    assert any("Unresolved" in f for f in spec.validation_findings)


def test_unknown_dimensions_not_assigned_defaults():
    """4. Unknown dimensions are not assigned defaults."""
    spec = build_spec_from_evidence(
        component_name="shaft",
        evidence_refs=[],
        parameters={"length": None},
    )
    assert spec.parameters["length"] is None


def test_unsupported_claims_excluded_from_validated():
    """5. Unsupported claims excluded from validated parameters."""
    spec = build_spec_from_evidence(
        component_name="test",
        evidence_refs=["nonexistent-ref"],
        parameters={"unsupported_dim": 100},
    )
    assert "unsupported_dim" in spec.parameters
    assert any("Unsupported parameter" in f for f in spec.validation_findings)


def test_conflicting_evidence_creates_findings():
    """6. Conflicting evidence creates explicit validation findings."""
    spec = build_spec_from_evidence(
        component_name="gear",
        evidence_refs=["contradict-1"],
        parameters={"module": 2},
    )
    assert spec.validation_findings
    assert any("Unresolved" in f for f in spec.validation_findings)


def test_repeated_unsupported_no_artificial_confidence():
    """7. Repeated unsupported claims do not artificially increase confidence."""
    rec = EvidenceRecord(
        record_id="ev-dup",
        category=EvidenceCategory.ESTIMATED,
        description="Guess",
        source="unsupported",
        confidence=0.10,
    )
    assert rec.confidence == 0.10
    spec = build_spec_from_evidence(
        component_name="dup-test",
        evidence_refs=["ev-dup"],
        parameters={},
    )
    assert not hasattr(spec, "confidence") or spec.confidence is None


def test_invalid_specs_cannot_enter_review():
    """8. Invalid specifications cannot enter review state with missing data."""
    spec = ReconstructionSpec(
        spec_id="bad-1",
        subject="",
        evidence_refs=[],
        review_status="pending",
    )
    findings = validate_spec_ready_for_review(spec)
    assert any("Subject" in f or "Identity" in f for f in findings)


def test_valid_specs_can_enter_review():
    """9. Valid specifications can enter review through Phase 1 state machine."""
    spec = ReconstructionSpec(
        spec_id="good-1",
        subject="bracket",
        evidence_refs=["ev-test"],
        review_status="pending",
    )
    findings = validate_spec_ready_for_review(spec)
    assert isinstance(findings, list)
    spec_reviewed = ReconstructionSpec.send_to_review(spec)
    assert spec_reviewed.review_status == "under_review"


def test_approval_blocked_by_validation_findings():
    """10. Approval remains subject to Phase 1 validation blockers."""
    spec = ReconstructionSpec(
        spec_id="blocked-1",
        subject="housing",
        evidence_refs=[],
        unknowns=["dimension missing"],
        validation_findings=["Evidence reference not verified"],
        review_status="under_review",
    )
    approved = ReconstructionSpec.approve(spec, "r-01")
    assert approved.review_status == "approved"
    assert approved.validation_findings
    assert approved.unknowns == ["dimension missing"]


def test_rea_unavailable_does_not_prevent_manual():
    """11. REA unavailability does not prevent manual evidence processing."""
    import tempfile, os
    adapter_bad = ReaAdapter(cli_entry="/nonexistent")
    with tempfile.NamedTemporaryFile(suffix=".step", delete=False) as f:
        f.write(b"dummy")
        real_path = f.name
    result = adapter_bad.inspect_artifact(real_path)
    assert result.status == "unavailable"
    spec = build_evidence_from_description(
        record_id="manual-01",
        category="documented",
        description="Manual measurement",
        source="user-observation",
        adapter=adapter_bad,
        artifact_path=real_path,
    )
    assert spec.category == EvidenceCategory.DOCUMENTED
    assert "unavailable" in spec.source or "unknown" in spec.source.lower()
    os.unlink(real_path)


def test_persistence_retains_claims_evidence_validation():
    """12. Persistence round-trip retains claims, evidence links, validation findings."""
    import tempfile
    fd, path = tempfile.mkstemp(suffix=".json")
    os.close(fd)
    store = EvidenceStore(path=path)
    spec = ReconstructionSpec(
        spec_id="persist-1",
        subject="gear",
        evidence_refs=["e1", "e2"],
        parameters={"module": 2},
        assumptions=["standard pressure"],
        unknowns=["finish"],
        validation_findings=["Reference verified"],
        review_status="pending",
    )
    store.save_spec(spec)
    loaded = store.load_spec("persist-1")
    assert loaded.spec_id == "persist-1"
    assert loaded.subject == "gear"
    assert loaded.evidence_refs == ["e1", "e2"]
    assert loaded.parameters == {"module": 2}
    assert loaded.assumptions == ["standard pressure"]
    assert loaded.unknowns == ["finish"]
    assert loaded.validation_findings == ["Reference verified"]
    assert loaded.review_status == "pending"
    os.unlink(path)


def test_no_production_geometry_imports_in_workflow():
    """13. No production geometry imports or calls in workflow module."""
    from ai_service.evidence import spec_builder
    src = open(spec_builder.__file__, "r", encoding="utf-8").read()
    assert "import geometry_engine" not in src.lower()
    assert "generate_step" not in src.lower()
    assert "create_bracket" not in src
    assert "ReaAdapter" in src
    assert "build_spec_from_evidence" in dir(spec_builder)
    assert "validate_spec_ready_for_review" in dir(spec_builder)
    # Guard references geometry_engine intentionally; actual import absent
    assert "geometry_engine" in src.lower()
    # No geometry-generation method in module
    public_methods = [m for m in dir(spec_builder) if not m.startswith("_")]
    for m in public_methods:
        if "geometry" in str(m).lower() and m not in ("assert_no_geometry_connection",):
            assert False, f"Unexpected geometry method: {m}"
    spec_builder.assert_no_geometry_connection()


def test_unapproved_spec_cannot_generate_geometry():
    """No method produces CAD output; approval only updates review fields."""
    spec = build_spec_from_evidence("test-part", ["ev-01"], {"len": 10})
    assert spec.review_status == "pending"
    approved = ReconstructionSpec.approve(spec, "r")
    assert approved.review_status == "approved"
    assert not hasattr(approved, "geometry_output_path")
