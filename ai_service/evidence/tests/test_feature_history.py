"""Task 13 — Feature history tests."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from ai_service.evidence.feature_history import FeatureRecord, FeatureHistory

def test_feature_history_cycle_and_regenerate():
    h = FeatureHistory("test-rect")
    h.add_feature(FeatureRecord("f1", "sketch", upstream_ids=[], parameters={"length":100,"width":50}))
    h.add_feature(FeatureRecord("f2", "extrude", upstream_ids=["f1"], parameters={"thickness":25}))
    assert h.detect_cycle() == []
    res = h.regenerate()
    assert res["status"] == "validated"
    assert "f1" in res["features"]
    assert "f2" in res["features"]
    # Edit upstream
    h.invalidate_downstream("f1")
    f2 = h.get_feature("f2")
    assert f2.status == "failed"
    assert any("f1" in u for u in f2.unknowns)

def test_feature_history_persist_roundtrip():
    h = FeatureHistory("persist-test")
    h.add_feature(FeatureRecord("f1", "sketch", upstream_ids=[], parameters={"length":100}))
    h.save("/tmp/task13_history.json")
    h2 = FeatureHistory("persist-test-2")
    h2.load("/tmp/task13_history.json")
    assert len(h2.features) == 1
    assert h2.get_feature("f1").feature_type == "sketch"
