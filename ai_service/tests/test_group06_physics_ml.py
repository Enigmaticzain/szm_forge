from physics_ml_engine import get_physics_ml_engine


def test_surrogate_train_predict():
    eng = get_physics_ml_engine()
    train = eng.train_surrogate("test_model", "torch", epochs=5)
    assert train["status"] == "success"
    pred = eng.predict("test_model", {"x": 0.5})
    assert "prediction" in pred
