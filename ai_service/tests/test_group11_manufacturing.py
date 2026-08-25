from manufacturing_pipeline import GCodeExporter, CNCToolpathGenerator


def test_gcode_export():
    g = GCodeExporter.export_gcode("part", {"length": 0.1, "width": 0.05, "depth": 0.01})
    assert "G21" in g


def test_cnc_toolpath():
    r = CNCToolpathGenerator.generate_rectangular_milling_job("part", {"length": 0.1, "width": 0.05, "depth": 0.01})
    assert r["status"] == "success"
