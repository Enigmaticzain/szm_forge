import engines.geometry_engine as ge
import cad_backends as cb


def test_available_libraries():
    libs = ge.get_available_libraries()
    assert isinstance(libs, dict)
    assert "build123d" in libs
    assert "cadquery" in libs
    assert "scikit_spatial" in libs


def test_cad_backends_available():
    backends = cb.available_backends()
    assert isinstance(backends, dict)
    assert "build123d" in backends
    assert "cadquery" in backends


def test_fit_plane_requires_skspatial_or_raises():
    points = [[0, 0, 0], [1, 0, 0], [0, 1, 0], [1, 1, 0.01]]
    if ge.get_available_libraries().get("scikit_spatial"):
        result = ge.fit_plane_to_points(points)
        assert "normal" in result
    else:
        try:
            ge.fit_plane_to_points(points)
            assert False, "expected RuntimeError"
        except RuntimeError:
            pass


def test_fit_line_requires_skspatial_or_raises():
    points = [[0, 0, 0], [1, 1, 1], [2, 2, 2.01]]
    if ge.get_available_libraries().get("scikit_spatial"):
        result = ge.fit_line_to_points(points)
        assert "direction" in result
    else:
        try:
            ge.fit_line_to_points(points)
            assert False, "expected RuntimeError"
        except RuntimeError:
            pass


def test_create_from_spec_or_skip():
    backends = cb.available_backends()
    if not any(backends.values()):
        return
    part, name = cb.create_from_spec("box", {"length": 10, "width": 5, "height": 2})
    assert part is not None
    assert name in ("build123d", "cadquery")


def test_text_to_cad_reports_backend():
    from text_to_cad import text_to_cad
    result = text_to_cad("100mm aluminum box", "/tmp")
    assert result["status"] == "success"
    assert "backend" in result
    assert "engine" in result
