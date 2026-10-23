"""Optional REA CLI adapter for artifact inspection.

Importing this module does NOT require REA to be installed.
If the CLI binary is missing, methods return an unavailable result.
"""
from __future__ import annotations

import os
import subprocess
from dataclasses import dataclass
from typing import Optional

# ── Verified low-cost REA CLI operation ──────────────────────────────────────

# Confirmed available via `rea capabilities` on this host:
#   cli: node <package>/scripts/rea.mjs inspect_artifact --detail=summary --path=<path>
# This is a read-only artifact inventory operation — no binary target required.
REA_PACKAGE_DIR = "/home/szm7226/.npm/_npx/9b9809298fe40d2c/node_modules/rea-agents"
REA_CLI_ENTRY = os.path.join(REA_PACKAGE_DIR, "scripts", "rea.mjs")


# ── Typed result ─────────────────────────────────────────────────────────────

@dataclass
class ReaResult:
    """Structured result from a REA CLI invocation."""
    status: str  # "success" | "unavailable" | "timeout" | "command_failure" | "invalid_input"
    stdout: str
    stderr: str
    returncode: Optional[int] = None
    reason: Optional[str] = None  # Human-readable failure detail (never secrets)

    @property
    def ok(self) -> bool:
        return self.status == "success"

    @property
    def error_message(self) -> str:
        if self.status == "unavailable":
            return f"REA CLI unavailable: {self.reason or 'binary not found'}"
        if self.status == "timeout":
            return f"REA CLI timed out: {self.reason or 'exceeded deadline'}"
        if self.status == "command_failure":
            return f"REA CLI command failed (exit={self.returncode}): {self.reason or self.stderr}"
        if self.status == "invalid_input":
            return f"Invalid input: {self.reason}"
        return ""


# ── ReaAdapter ───────────────────────────────────────────────────────────────

class ReaAdapter:
    """Thin wrapper around the REA CLI `inspect_artifact --detail=summary` command."""

    def __init__(self, cli_entry: str = REA_CLI_ENTRY, timeout_seconds: float = 30.0):
        self.cli_entry = cli_entry
        self.timeout_seconds = timeout_seconds
        self._cli_verified: Optional[bool] = None

    def _verify_cli(self) -> bool:
        """Check that the REA CLI entry point exists; cache the result."""
        if self._cli_verified is not None:
            return self._cli_verified
        try:
            self._cli_verified = os.path.isfile(self.cli_entry) and os.access(
                self.cli_entry, os.R_OK
            )
        except OSError:
            self._cli_verified = False
        return self._cli_verified

    def inspect_artifact(self, path: str) -> ReaResult:
        """Run `rea.mjs inspect_artifact --detail=summary --path=<path>`.

        Returns a ReaResult with status distinguishing:
        - success: artifact inventory retrieved
        - unavailable: CLI binary missing or unreadable
        - timeout: command exceeded deadline
        - command_failure: nonzero exit or error text
        - invalid_input: path missing, empty, or not a file
        """
        # Validate input before touching subprocess
        if not path or not isinstance(path, str):
            return ReaResult(
                status="invalid_input",
                stdout="",
                stderr="",
                returncode=None,
                reason="artifact path must be a non-empty string",
            )
        if not path.strip():
            return ReaResult(
                status="invalid_input",
                stdout="",
                stderr="",
                returncode=None,
                reason="artifact path must not be blank",
            )

        # Avoid leaking secrets into error messages: path itself is caller-supplied,
        # but we never log or echo environment variables.
        if not os.path.isfile(path):
            return ReaResult(
                status="invalid_input",
                stdout="",
                stderr="",
                returncode=None,
                reason=f"artifact path is not a readable file: {os.path.basename(path)}",
            )

        if not self._verify_cli():
            return ReaResult(
                status="unavailable",
                stdout="",
                stderr="",
                returncode=None,
                reason=f"REA CLI not found at {self.cli_entry}",
            )

        # Build argument list — never shell interpolation
        cmd = [
            "node",
            self.cli_entry,
            "inspect_artifact",
            "--detail=summary",
            f"--path={path}",
        ]

        try:
            completed = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=self.timeout_seconds,
            )
        except subprocess.TimeoutExpired as exc:
            return ReaResult(
                status="timeout",
                stdout=exc.stdout.decode(errors="replace") if exc.stdout else "",
                stderr=exc.stderr.decode(errors="replace") if exc.stderr else "",
                returncode=-1,
                reason=f"command exceeded {self.timeout_seconds}s deadline",
            )
        except FileNotFoundError:
            return ReaResult(
                status="unavailable",
                stdout="",
                stderr="",
                returncode=None,
                reason="node executable not found",
            )
        except OSError as exc:
            return ReaResult(
                status="command_failure",
                stdout="",
                stderr=str(exc),
                returncode=None,
                reason="failed to launch REA CLI",
            )

        if completed.returncode != 0:
            return ReaResult(
                status="command_failure",
                stdout=completed.stdout or "",
                stderr=completed.stderr or "",
                returncode=completed.returncode,
                reason="nonzero exit code",
            )

        # Any stderr from a successful return code is non-fatal but noted
        return ReaResult(
            status="success",
            stdout=completed.stdout or "",
            stderr=completed.stderr or "",
            returncode=completed.returncode,
            reason=None,
        )
