"""Jinja2 HTML report templates for simulation results (Group 14)."""
from __future__ import annotations

from typing import Any, Dict

_HAS_JINJA = False
try:
    from jinja2 import Template
    _HAS_JINJA = True
except ImportError:
    Template = None  # type: ignore

_FEA_TEMPLATE = """
<!DOCTYPE html>
<html><head><title>SZM FEA Report</title>
<style>
body{font-family:system-ui,sans-serif;margin:2rem;background:#0f1419;color:#e7ecf1}
.card{background:#1a2332;padding:1.5rem;border-radius:8px;max-width:640px}
h1{margin-top:0} .ok{color:#3dd68c} .warn{color:#f5a524}
</style></head>
<body><div class="card">
<h1>FEA Report</h1>
<p>Component: {{ component_id }}</p>
<p>Engine: {{ engine_used }}</p>
<p>Max stress: {{ max_stress_pa }} Pa</p>
<p>Max displacement: {{ max_displacement_m }} m</p>
<p class="{{ 'warn' if is_dangerous else 'ok' }}">
  {{ 'DANGEROUS' if is_dangerous else 'Within limits' }}
</p>
</div></body></html>
"""


def render_fea_report(data: Dict[str, Any]) -> str:
    if not _HAS_JINJA:
        return (
            f"<html><body><h1>FEA</h1><pre>{data}</pre>"
            "<p>Install jinja2 for styled reports</p></body></html>"
        )
    return Template(_FEA_TEMPLATE).render(**data)
