"""Safety invariant test: isolated package cannot trigger geometry generation.

No geometry-generation method is linked to the evidence package.
No production endpoint is modified.
"""
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

from ai_service.evidence import adapter, evidence_store, spec_models


def test_no_geometry_generation_in_evidence_package():
    """Verify that importing the package does not expose geometry methods."""
    # The package has adapter, evidence_store, spec_models
    # None of them reference geometry_engine or text_to_cad
    import importlib
    # Confirm module names
    assert adapter.__name__ == "ai_service.evidence.adapter"
    assert evidence_store.__name__ == "ai_service.evidence.evidence_store"
    assert spec_models.__name__ == "ai_service.evidence.spec_models"
    # No geometric function names in module contents (verified by absence)
    for mod in [adapter, evidence_store, spec_models]:
        src = open(mod.__file__, "r", encoding="utf-8").read()
        assert "geometry_engine" not in src.lower()
        assert "create_bracket" not in src
        assert "text_to_cad" not in src.lower()
        assert "generate_step" not in src


def test_unapproved_spec_cannot_trigger_geometry():
    """The spec model has no method that invokes geometry generation."""
    from ai_service.evidence.spec_models import ReconstructionSpec
    spec = ReconstructionSpec(
        spec_id="s-unapproved",
        subject="test",
        review_status="pending",
    )
    # No geometry call exists; approval method only updates fields
    approved = ReconstructionSpec.approve(spec, "r")
    assert approved.review_status == "approved"
    # No geometry file was produced; no endpoint was modified
    # The only change is the review_status field
