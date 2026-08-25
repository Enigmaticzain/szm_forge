from thermal_engine import fluid_properties, get_available_libraries


def test_fluid_properties():
    props = fluid_properties("Water", 300.0)
    assert props["density_kg_m3"] > 0


def test_cfd_analytical():
    from thermal_engine import run_openfoam_cfd
    r = run_openfoam_cfd(10.0, 1.2, {"width": 0.5, "height": 0.5})
    assert r["drag_force_n"] > 0
