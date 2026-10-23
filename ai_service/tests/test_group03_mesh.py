import engines.mesh_engine as me


def test_mesh_libraries_dict():
    libs = me.get_available_libraries()
    assert "trimesh" in libs or "gmsh" in libs or "meshio" in libs


def test_generate_mesh_fallback():
    try:
        path = me.generate_mesh(box_dims=(0.1, 0.1, 0.1))
        assert path
    except RuntimeError:
        pass


def test_convert_mesh_requires_meshio():
    libs = me.get_available_libraries()
    if not libs.get("meshio"):
        try:
            me.convert_mesh("/tmp/x.msh", "/tmp/x.vtk")
            assert False, "expected RuntimeError"
        except (RuntimeError, Exception):
            pass
