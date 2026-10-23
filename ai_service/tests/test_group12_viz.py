from viz3d_engine import available_backends


def test_viz3d_backends():
    assert isinstance(available_backends(), dict)
