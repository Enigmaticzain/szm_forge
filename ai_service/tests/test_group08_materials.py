from material_synthesis import MaterialSynthesisEngine


def test_lookup_element():
    r = MaterialSynthesisEngine.lookup_element("Fe")
    assert r["symbol"] == "Fe"
    assert r["properties"]


def test_synthesize():
    r = MaterialSynthesisEngine.synthesize({"name": "Test Alloy", "family": "alloy"})
    assert r["status"] == "success"
