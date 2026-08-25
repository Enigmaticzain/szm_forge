from tool_registry import get_health_report, verify_all, ToolStatus


def test_health_report_structure():
    report = get_health_report()
    assert "status" in report
    assert "total_tools" in report
    assert "tools" in report
    assert report["total_tools"] > 0


def test_verify_group2():
    results = verify_all(2)
    assert len(results) >= 5
    names = {r.name for r in results}
    assert "build123d" in names or "trimesh" in names


def test_group_summary():
    from tool_registry import get_group_summary
    summary = get_group_summary(2)
    assert "groups" in summary
