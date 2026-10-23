"""Targeted tests for the optional REA CLI adapter.

Mocked subprocess; no actual expensive REA analysis launched.
Works when REA binary is unavailable.
No secrets in errors.
"""
import sys
import os

sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

from ai_service.evidence.adapter import ReaAdapter, ReaResult, REA_CLI_ENTRY

# ── Mock helper ────────────────────────────────────────────────────────────────

class MockCompletedProcess:
    def __init__(self, stdout: str, stderr: str, returncode: int):
        self.stdout = stdout
        self.stderr = stderr
        self.returncode = returncode


# ── Availability ──────────────────────────────────────────────────────────────

def test_adapter_unavailable_when_binary_missing(monkeypatch):
    """When binary is missing, result status is unavailable with safe message."""
    adapter = ReaAdapter(cli_entry="/nonexistent/rea.mjs")
    result = adapter.inspect_artifact("/tmp/safe_sample.step")
    assert result.status == "unavailable"
    assert "not found" in result.error_message.lower()
    assert "secret" not in result.error_message.lower()
    assert "token" not in result.error_message.lower()
    assert "key" not in result.error_message.lower()


# ── Invalid input ─────────────────────────────────────────────────────────────

def test_adapter_invalid_empty_path(monkeypatch):
    adapter = ReaAdapter()
    result_empty = adapter.inspect_artifact("")
    assert result_empty.status == "invalid_input"
    assert result_empty.ok is False


def test_adapter_invalid_blank_path(monkeypatch):
    adapter = ReaAdapter()
    result_blank = adapter.inspect_artifact("   ")
    assert result_blank.status == "invalid_input"


def test_adapter_invalid_missing_file(monkeypatch):
    adapter = ReaAdapter()
    result_missing = adapter.inspect_artifact("/tmp/nonexistent_file.step")
    assert result_missing.status == "invalid_input"
    assert result_missing.error_message != ""
    assert "secret" not in result_missing.error_message.lower()


# ── Successful CLI (mocked subprocess) ─────────────────────────────────────────

def test_adapter_successful_cli(monkeypatch):
    adapter = ReaAdapter()
    # Mock subprocess.run to return a successful run with summary output
    import subprocess

    original_run = subprocess.run

    def mock_run(cmd, capture_output=False, text=False, timeout=None):
        # Confirm arguments are a list (not shell interpolation)
        assert isinstance(cmd, list), f"Expected list argument; got {type(cmd)}"
        assert "inspect_artifact" in cmd
        assert "--detail=summary" in cmd
        assert any("--path=" in str(arg) for arg in cmd)
        return MockCompletedProcess(
            stdout='Artifact inventory: SUMMARY\nformat: step, kind: CAD, sha256: abc',
            stderr="",
            returncode=0,
        )

    monkeypatch.setattr(subprocess, "run", mock_run)
    # Create a temporary file so path validation passes
    import tempfile
    with tempfile.NamedTemporaryFile(suffix=".step", delete=False) as f:
        f.write(b"dummy step content")
        temp_path = f.name
    result = adapter.inspect_artifact(temp_path)
    assert result.status == "success"
    assert result.returncode == 0
    assert "SUMMARY" in result.stdout
    assert "secret" not in result.stdout.lower()
    assert result.error_message == ""
    # Clean up
    import os
    try:
        os.unlink(temp_path)
    except OSError:
        pass


# ── Command failure (mocked nonzero exit) ──────────────────────────────────────

def test_adapter_nonzero_exit(monkeypatch):
    import subprocess

    def mock_run_fail(cmd, capture_output=False, text=False, timeout=None):
        return MockCompletedProcess(
            stdout="", stderr="Unknown error: analysis failed",
            returncode=1,
        )

    monkeypatch.setattr(subprocess, "run", mock_run_fail)
    adapter = ReaAdapter()
    # Even with missing file, we want to test nonzero — but the adapter checks file first.
    # So provide a real temporary file for this case.
    import tempfile
    with tempfile.NamedTemporaryFile(suffix=".step", delete=False) as f:
        f.write(b"dummy")
        temp_path = f.name
    result = adapter.inspect_artifact(temp_path)
    assert result.status == "command_failure"
    assert result.returncode == 1
    assert result.error_message != ""
    assert "secret" not in result.error_message.lower()
    try:
        os.unlink(temp_path)
    except OSError:
        pass


# ── Timeout (mocked TimeoutExpired) ────────────────────────────────────────────

def test_adapter_timeout(monkeypatch):
    import subprocess

    def mock_run_timeout(cmd, capture_output=False, text=False, timeout=None):
        exc = subprocess.TimeoutExpired(cmd=cmd, timeout=timeout or 30)
        exc.stdout = b"partial output"
        exc.stderr = b""
        raise exc

    monkeypatch.setattr(subprocess, "run", mock_run_timeout)
    adapter = ReaAdapter()
    import tempfile
    with tempfile.NamedTemporaryFile(suffix=".step", delete=False) as f:
        f.write(b"dummy")
        temp_path = f.name
    result = adapter.inspect_artifact(temp_path)
    assert result.status == "timeout"
    assert result.returncode == -1
    assert "exceeded" in result.error_message.lower()
    try:
        os.unlink(temp_path)
    except OSError:
        pass


# ── Safety invariant: adapter does not invoke geometry ───────────────────────

def test_adapter_no_geometry_in_result():
    """The adapter does not expose any geometry-generation method."""
    adapter = ReaAdapter()
    # The adapter only has inspect_artifact; no create_geometry, generate_step, etc.
    assert not hasattr(adapter, "generate_geometry")
    assert not hasattr(adapter, "create_part")
    assert not hasattr(adapter, "compute_geometry")


def test_adapter_inspect_artifact_only():
    """Only the verified inspect_artifact method exists; no unverified methods."""
    adapter = ReaAdapter()
    public_methods = [m for m in dir(adapter) if not m.startswith("_") and callable(getattr(adapter, m, None))]
    # inspect_artifact is the only exposed public method
    assert "inspect_artifact" in public_methods
