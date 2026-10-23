"""Typer CLI for SZM Forge ai_service (Group 18)."""
from __future__ import annotations

import json
import sys
from pathlib import Path

try:
    import typer
    from rich import print as rprint
    from rich.table import Table
except ImportError:
    typer = None

app = typer.Typer(help="SZM Forge AI Service CLI") if typer else None
ROOT = Path(__file__).resolve().parent


def _ensure_path():
    sys.path.insert(0, str(ROOT))


if app:

    @app.command()
    def verify(group: int = typer.Option(0, help="Group 2-18, 0=all")):
        """Run stack verification."""
        _ensure_path()
        from tool_registry import get_health_report, verify_all
        if group:
            results = verify_all(group)
            for r in results:
                rprint(f"[{'green' if r.status.value == 'OK' else 'yellow'}]{r.name}[/]: {r.status.value}")
        else:
            report = get_health_report()
            rprint(f"Available: {report['available']}/{report['total_tools']}")

    @app.command()
    def install_info():
        """Show install script paths."""
        scripts = ROOT.parent / "scripts" / "install_stack.sh"
        rprint(f"Install: bash {scripts}")
        rprint(f"Requirements: {ROOT / 'requirements'}")

    @app.command()
    def run_bridge(host: str = "127.0.0.1", port: int = 8003):
        """Start external engine bridge."""
        import uvicorn
        uvicorn.run("external_engine_bridge:app", host=host, port=port, reload=False)


def main():
    if app is None:
        print("Install typer and rich: pip install typer rich")
        sys.exit(1)
    app()


if __name__ == "__main__":
    main()
