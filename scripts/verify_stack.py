#!/usr/bin/env python3
"""Verify SZM Forge engineering stack installation."""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "ai_service"))

from tool_registry import get_group_summary, get_health_report, get_ui_status, verify_all  # noqa: E402


def _print_rich(report: dict) -> None:
    try:
        from rich.console import Console
        from rich.table import Table
    except ImportError:
        return
    console = Console()
    console.print(f"\n[bold cyan]SZM Forge Stack[/] — {report.get('available', 0)}/{report.get('total_tools', 0)} tools OK\n")
    table = Table(show_header=True, header_style="bold")
    table.add_column("Group")
    table.add_column("Layer")
    table.add_column("OK")
    table.add_column("Total")
    table.add_column("Health")
    for g in report.get("groups", []):
        pct = f"{g.get('health_pct', 0):.0f}%"
        style = "green" if g.get("ok") == g.get("total") else "yellow"
        table.add_row(str(g["group"]), g["name"], str(g["ok"]), str(g["total"]), f"[{style}]{pct}[/{style}]")
    console.print(table)
    ui = get_ui_status()
    cad = "[green]READY[/]" if ui.get("cad_ready") else "[yellow]PARTIAL[/]"
    mesh = "[green]READY[/]" if ui.get("mesh_ready") else "[yellow]PARTIAL[/]"
    fea = "[green]READY[/]" if ui.get("fea_ready") else "[yellow]PARTIAL[/]"
    console.print(f"\nCAD layer (Group 2): {cad}")
    console.print(f"Mesh layer (Group 3): {mesh}")
    console.print(f"FEA layer (Group 4): {fea}")
    thermal = "[green]READY[/]" if ui.get("thermal_ready") else "[yellow]PARTIAL[/]"
    cfd = "[green]READY[/]" if ui.get("cfd_ready") else "[yellow]PARTIAL[/]"
    console.print(f"Thermal layer (Group 5): {thermal}")
    console.print(f"CFD layer (Group 5): {cfd}")
    console.print(f"Bridge UI endpoint: http://127.0.0.1:8003/stack/status\n")


def main() -> int:
    parser = argparse.ArgumentParser(description="Verify SZM Forge engineering stack")
    parser.add_argument("--group", type=int, action="append", help="Verify group N only (2-18)")
    parser.add_argument("--json", action="store_true", help="Output JSON report")
    parser.add_argument("--strict", action="store_true", help="Exit 1 if any required tool missing")
    args = parser.parse_args()

    if args.group:
        results = []
        for g in args.group:
            results.extend(verify_all(g))
        summary = get_group_summary()
        filtered_groups = [grp for grp in summary.get("groups", []) if grp["group"] in args.group]
        report = {
            "groups": [
                {
                    **grp,
                    "health_pct": round(100.0 * grp["ok"] / max(grp["total"], 1), 1),
                }
                for grp in filtered_groups
            ],
            "filtered_groups": args.group,
            "tools": [
                {"name": r.name, "group": r.group, "status": r.status.value,
                 "version": r.version, "message": r.message}
                for r in results
            ],
            "available": sum(1 for r in results if r.status.value == "OK"),
            "total_tools": len(results),
            "status": "healthy" if results else "degraded",
        }
    else:
        report = get_health_report()
        report["groups"] = [
            {**g, "health_pct": round(100.0 * g["ok"] / max(g["total"], 1), 1)}
            for g in get_group_summary().get("groups", [])
        ]

    if args.json:
        print(json.dumps(report, indent=2))
    else:
        _print_rich(report)
        tools = report.get("tools", [])
        if tools:
            print(f"Stack health: {report.get('available', 0)}/{report.get('total_tools', len(tools))} tools available")
            for t in tools:
                mark = t["status"]
                ver = f" ({t['version']})" if t.get("version") else ""
                print(f"  [{mark}] g{t['group']} {t['name']}{ver}")
        elif report.get("groups"):
            for grp in report["groups"]:
                if args.group and grp["group"] not in args.group:
                    continue
                print(f"\nGroup {grp['group']}: {grp['name']} — {grp['ok']}/{grp['total']} OK")
                for t in grp.get("tools", []):
                    mark = t["status"]
                    ver = f" ({t['version']})" if t.get("version") else ""
                    print(f"  [{mark}] {t['name']}{ver}")

    if args.strict:
        tools = report.get("tools", [])
        missing = [t for t in tools if t["status"] not in ("OK", "OPTIONAL")]
        if missing:
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
