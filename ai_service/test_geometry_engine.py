#!/usr/bin/env python3
"""Test suite for SZM Forge Geometry Engine."""
import os
import sys
import traceback

sys.path.insert(0, os.path.dirname(__file__))
import engines.geometry_engine as ge

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "geometry_output")
os.makedirs(OUTPUT_DIR, exist_ok=True)

passed = 0
failed = 0


def run_test(name: str, fn):
    global passed, failed
    try:
        fn()
        print(f"  ✅ {name}")
        passed += 1
    except Exception as e:
        print(f"  ❌ {name}: {e}")
        traceback.print_exc()
        failed += 1


# ── Test 1: Smart Part Creation ───────────────────────────────────────────────
print("\n═══ Test 1: Smart Part Creation ═══")


def test_bracket():
    part = ge.create_bracket(length=80, width=40, thickness=5, holes=2, fillet_r=1.0)
    path = ge.export_step(part, os.path.join(OUTPUT_DIR, "bracket.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


def test_shaft():
    part = ge.create_shaft(diameter=20, length=100, keyway=True, chamfer=True)
    path = ge.export_step(part, os.path.join(OUTPUT_DIR, "shaft.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


def test_flange():
    part = ge.create_flange(outer_d=80, inner_d=30, thickness=10, bolt_holes=6, bolt_d=8)
    path = ge.export_step(part, os.path.join(OUTPUT_DIR, "flange.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


def test_housing():
    part = ge.create_housing(length=120, width=80, height=60, wall_thickness=5, bore_d=30)
    path = ge.export_step(part, os.path.join(OUTPUT_DIR, "housing.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


def test_gear():
    part = ge.create_gear(module=2, num_teeth=20, thickness=10, bore_d=10)
    path = ge.export_step(part, os.path.join(OUTPUT_DIR, "gear.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


def test_plate():
    part = ge.create_plate(length=200, width=100, thickness=5, hole_pattern="grid")
    path = ge.export_step(part, os.path.join(OUTPUT_DIR, "plate.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


run_test("Bracket → STEP", test_bracket)
run_test("Shaft → STEP", test_shaft)
run_test("Flange → STEP", test_flange)
run_test("Housing → STEP", test_housing)
run_test("Gear → STEP", test_gear)
run_test("Plate → STEP", test_plate)

# ── Test 2: 2D Profile → 3D ──────────────────────────────────────────────────
print("\n═══ Test 2: 2D Profile → 3D Extrusion ═══")


def test_2d_profile():
    profile = ge.create_2d_profile([(0, 0), (100, 0), (100, 50), (50, 75), (0, 50)])
    assert profile.area > 0
    offset = ge.offset_profile(profile, -3.0)
    assert offset.area < profile.area
    solid = ge.profile_to_3d(profile, height=10)
    path = ge.export_step(solid, os.path.join(OUTPUT_DIR, "profile_extruded.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


run_test("2D profile → offset → extrude → STEP", test_2d_profile)

# ── Test 3: Mesh Operations ──────────────────────────────────────────────────
print("\n═══ Test 3: Mesh Import/Convert/Analyze ═══")


def test_mesh_from_step():
    # Create a part, export to STL, import as mesh, analyze
    part = ge.create_shaft(diameter=20, length=50)
    stl_path = ge.export_stl(part, os.path.join(OUTPUT_DIR, "shaft_mesh.stl"))
    mesh = ge.import_mesh(stl_path)
    props = ge.mesh_properties(mesh)
    assert props["vertex_count"] > 0
    assert props["face_count"] > 0
    assert props["is_watertight"]
    assert props["volume"] > 0


def test_mesh_repair():
    part = ge.create_flange(outer_d=60, inner_d=20, thickness=8)
    stl_path = ge.export_stl(part, os.path.join(OUTPUT_DIR, "flange_mesh.stl"))
    mesh = ge.import_mesh(stl_path)
    repaired = ge.repair_mesh(mesh)
    assert repaired.is_watertight


run_test("STEP → STL → mesh analysis", test_mesh_from_step)
run_test("Mesh repair", test_mesh_repair)

# ── Test 4: Interference Check ───────────────────────────────────────────────
print("\n═══ Test 4: Interference Checking ═══")


def test_interference():
    shaft = ge.create_shaft(diameter=20, length=100)
    flange = ge.create_flange(outer_d=80, inner_d=30, thickness=10)
    result = ge.check_interference(shaft, flange)
    # Both at origin, shaft passes through flange center → should interfere
    assert isinstance(result["interferes"], bool)


def test_clearance():
    p1 = ge.create_shaft(diameter=10, length=30)
    p2 = ge.create_shaft(diameter=10, length=30)
    dist = ge.calculate_clearance(p1, p2)
    assert isinstance(dist, float)


def test_bounding_box():
    part = ge.create_housing(length=100, width=60, height=40)
    bb = ge.fit_bounding_box(part)
    assert len(bb["extents"]) == 3
    assert all(e > 0 for e in bb["extents"])


def test_dfm():
    part = ge.create_plate(length=200, width=100, thickness=5)
    warnings = ge.check_manufacturability(part, process="cnc")
    assert isinstance(warnings, list)


run_test("Interference check", test_interference)
run_test("Clearance calculation", test_clearance)
run_test("Bounding box", test_bounding_box)
run_test("Manufacturability check", test_dfm)

# ── Test 5: Assembly ─────────────────────────────────────────────────────────
print("\n═══ Test 5: Assembly Creation + Export ═══")


def test_assembly():
    asm = ge.create_assembly("TestAssembly")
    shaft = ge.create_shaft(diameter=20, length=100)
    flange = ge.create_flange(outer_d=80, inner_d=25, thickness=10)
    ge.add_to_assembly(asm, shaft, "Shaft", position=(0, 0, 0))
    ge.add_to_assembly(asm, flange, "Flange", position=(0, 0, 50))
    assert asm.part_count == 2
    path = ge.export_assembly(asm, os.path.join(OUTPUT_DIR, "assembly.step"))
    assert os.path.exists(path) and os.path.getsize(path) > 0


run_test("Assembly with 2 parts → STEP", test_assembly)

# ── Test 6: Format Conversion ────────────────────────────────────────────────
print("\n═══ Test 6: Format Conversion ═══")


def test_step_to_stl():
    step_in = os.path.join(OUTPUT_DIR, "flange.step")
    stl_out = os.path.join(OUTPUT_DIR, "flange_converted.stl")
    ge.convert_format(step_in, stl_out)
    assert os.path.exists(stl_out) and os.path.getsize(stl_out) > 0


def test_dxf_export_import():
    profile = ge.create_2d_profile([(0, 0), (100, 0), (100, 50), (0, 50)])
    dxf_path = ge.export_dxf(profile, os.path.join(OUTPUT_DIR, "profile.dxf"))
    assert os.path.exists(dxf_path)
    reimported = ge.import_dxf(dxf_path)
    assert reimported.area > 0


run_test("STEP → STL conversion", test_step_to_stl)
run_test("DXF export + reimport", test_dxf_export_import)

# ── Summary ──────────────────────────────────────────────────────────────────
print(f"\n{'═' * 50}")
print(f"Results: {passed} passed, {failed} failed out of {passed + failed}")
if failed == 0:
    print("🎉 ALL TESTS PASSED!")
else:
    print(f"⚠️  {failed} test(s) need attention")
print(f"Output files in: {OUTPUT_DIR}")
print(f"{'═' * 50}")

# List generated files
print("\nGenerated files:")
for f in sorted(os.listdir(OUTPUT_DIR)):
    size = os.path.getsize(os.path.join(OUTPUT_DIR, f))
    print(f"  {f:30s} {size:>8,d} bytes")

sys.exit(0 if failed == 0 else 1)
