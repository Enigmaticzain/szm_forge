from engines.optimization_engine import optimize_design


def test_optimize_design():
    r = optimize_design(iterations=10)
    assert r["status"] == "success"
    assert len(r["history"]) == 10
