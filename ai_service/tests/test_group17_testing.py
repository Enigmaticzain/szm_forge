def test_ruff_config_exists():
    from pathlib import Path
    assert (Path(__file__).parent.parent / "pyproject.toml").exists()
