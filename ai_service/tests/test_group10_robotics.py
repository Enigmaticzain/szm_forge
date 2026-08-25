from robotics_engine import available_backends


def test_robotics_backends():
    b = available_backends()
    assert isinstance(b, dict)
