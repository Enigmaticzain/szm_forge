#!/usr/bin/env python3
"""Central registry for SZM Forge engineering stack tools (Groups 2–18)."""
from __future__ import annotations

import importlib
import importlib.util
import json
import logging
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from enum import Enum
from typing import Any, Callable, Dict, List, Optional

logger = logging.getLogger(__name__)


class ToolStatus(str, Enum):
    OK = "OK"
    MISSING = "MISSING"
    ERROR = "ERROR"
    OPTIONAL = "OPTIONAL"


@dataclass
class ToolSpec:
    name: str
    group: int
    group_name: str
    license: str
    purpose: str
    import_name: Optional[str] = None
    verify_cmd: Optional[List[str]] = None
    verify_fn: Optional[Callable[[], bool]] = None
    optional: bool = False
    gpl: bool = False


@dataclass
class ToolResult:
    name: str
    group: int
    status: ToolStatus
    version: str = ""
    message: str = ""
    license: str = ""
    purpose: str = ""


def _import_ok(module: str) -> tuple[bool, str]:
    try:
        mod = importlib.import_module(module)
        version = getattr(mod, "__version__", "")
        return True, str(version)
    except Exception as exc:
        return False, str(exc)


def _cmd_ok(cmd: List[str], timeout: int = 15) -> tuple[bool, str]:
    if not shutil.which(cmd[0]):
        return False, f"{cmd[0]} not in PATH"
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        if result.returncode == 0:
            out = (result.stdout or result.stderr or "OK").strip().split("\n")[0]
            return True, out
        return False, (result.stderr or result.stdout or "non-zero exit").strip()[:200]
    except Exception as exc:
        return False, str(exc)


def _verify_gmsh() -> bool:
    try:
        ok, _ = _import_ok("gmsh")
        if not ok:
            return False
        import gmsh
        gmsh.initialize()
        gmsh.finalize()
        return True
    except Exception:
        return False


def _verify_torch_cuda() -> bool:
    try:
        ok, _ = _import_ok("torch")
        if not ok:
            return False
        import torch  # noqa: F401
        return True
    except Exception:
        return False


def _verify_neuralop() -> bool:
    spec = importlib.util.find_spec("neuralop")
    if spec is None:
        spec = importlib.util.find_spec("neuraloperator")
    return spec is not None


TOOLS: List[ToolSpec] = [
    # Group 2 — CAD
    ToolSpec("cadquery", 2, "Geometry/CAD", "Apache 2.0", "Parametric CAD", "cadquery"),
    ToolSpec("build123d", 2, "Geometry/CAD", "Apache 2.0", "Modern Python CAD", "build123d"),
    ToolSpec("trimesh", 2, "Geometry/CAD", "MIT", "Mesh boolean + repair", "trimesh"),
    ToolSpec("shapely", 2, "Geometry/CAD", "BSD", "2D geometry", "shapely"),
    ToolSpec("numpy-stl", 2, "Geometry/CAD", "BSD", "STL I/O", "stl"),
    ToolSpec("ezdxf", 2, "Geometry/CAD", "MIT", "DXF I/O", "ezdxf"),
    ToolSpec("pyvista", 2, "Geometry/CAD", "MIT", "3D visualization", "pyvista"),
    ToolSpec("scikit-spatial", 2, "Geometry/CAD", "BSD", "Geometric fitting", "skspatial"),
    # Group 3 — Meshing
    ToolSpec("gmsh", 3, "Meshing", "GPL 2.0", "Mesh generator", "gmsh", verify_fn=_verify_gmsh, gpl=True),
    ToolSpec("meshio", 3, "Meshing", "MIT", "Mesh converter", "meshio"),
    ToolSpec("pygmsh", 3, "Meshing", "GPL", "Gmsh wrapper", "pygmsh", gpl=True, optional=True),
    # Group 4 — FEA
    ToolSpec("calculix", 4, "FEA", "GPL 2.0", "Structural FEA", verify_cmd=["ccx", "-v"], gpl=True, optional=True),
    ToolSpec("dolfinx", 4, "FEA", "LGPL 3.0", "FEniCS", "dolfinx", optional=True),
    ToolSpec("code_aster", 4, "FEA", "GPL 3.0", "Advanced FEA", verify_cmd=["as_run", "--version"], gpl=True, optional=True),
    ToolSpec("sfepy", 4, "FEA", "BSD", "Python FEA", "sfepy"),
    ToolSpec("scipy", 4, "FEA", "BSD", "Numerical solvers", "scipy"),
    ToolSpec("numpy", 4, "FEA", "BSD", "Numerical computing", "numpy"),
    # Group 5 — CFD/Thermal
    ToolSpec("openfoam", 5, "CFD/Thermal", "GPL 3.0", "CFD solver", verify_cmd=["simpleFoam", "-help"], gpl=True, optional=True),
    ToolSpec("SU2", 5, "CFD/Thermal", "LGPL 2.1", "Aerospace CFD", verify_cmd=["SU2_CFD", "--version"], optional=True),
    ToolSpec("elmer", 5, "CFD/Thermal", "GPL 2.0", "Multiphysics", verify_cmd=["ElmerSolver", "--version"], gpl=True, optional=True),
    ToolSpec("CoolProp", 5, "CFD/Thermal", "MIT", "Thermophysical props", "CoolProp"),
    # Group 6 — Physics ML
    ToolSpec("torch", 6, "Physics-ML", "BSD", "Deep learning", "torch", verify_fn=_verify_torch_cuda),
    ToolSpec("modulus", 6, "Physics-ML", "Apache 2.0", "Physics-informed ML", "modulus", optional=True),
    ToolSpec("deepxde", 6, "Physics-ML", "LGPL 2.1", "PINN library", "deepxde", optional=True),
    ToolSpec("neuraloperator", 6, "Physics-ML", "MIT", "FNO", verify_fn=_verify_neuralop, optional=True),
    ToolSpec("jax", 6, "Physics-ML", "Apache 2.0", "Numerical computing", "jax", optional=True),
    # Group 7 — Optimization
    ToolSpec("openmdao", 7, "Optimization", "Apache 2.0", "MDO", "openmdao"),
    ToolSpec("pygmo", 7, "Optimization", "GPL/MPL", "Evolutionary opt", "pygmo", gpl=True, optional=True),
    ToolSpec("optuna", 7, "Optimization", "MIT", "Hyperparameter opt", "optuna"),
    ToolSpec("pymoo", 7, "Optimization", "Apache 2.0", "Multi-objective", "pymoo"),
    ToolSpec("deap", 7, "Optimization", "LGPL 3.0", "Evolutionary alg", "deap"),
    # Group 8 — Materials
    ToolSpec("pymatgen", 8, "Materials", "MIT", "Materials analysis", "pymatgen"),
    ToolSpec("matminer", 8, "Materials", "MIT", "Materials informatics", "matminer"),
    ToolSpec("ase", 8, "Materials", "LGPL 2.1", "Atomic simulation", "ase"),
    ToolSpec("mendeleev", 8, "Materials", "MIT", "Element properties", "mendeleev"),
    ToolSpec("mp-api", 8, "Materials", "MIT", "Materials Project API", "mp_api", optional=True),
    # Group 9 — IoT
    ToolSpec("paho-mqtt", 9, "Digital Twin/IoT", "EPL 2.0", "MQTT client", "paho.mqtt.client"),
    ToolSpec("opcua", 9, "Digital Twin/IoT", "LGPL 3.0", "OPC-UA", "opcua", optional=True),
    ToolSpec("pymodbus", 9, "Digital Twin/IoT", "BSD", "Modbus", "pymodbus"),
    # Group 10 — Robotics
    ToolSpec("urdfpy", 10, "Robotics", "MIT", "URDF parser", "urdfpy"),
    ToolSpec("pybullet", 10, "Robotics", "zlib", "Physics sim", "pybullet"),
    # Group 11 — Manufacturing
    ToolSpec("linuxcnc", 11, "Manufacturing", "GPL 2.0", "CNC control", verify_cmd=["linuxcnc", "--version"], gpl=True, optional=True),
    ToolSpec("cura-engine", 11, "Manufacturing", "LGPL 3.0", "3D slicer", verify_cmd=["CuraEngine", "--version"], optional=True),
    ToolSpec("gcodepy", 11, "Manufacturing", "MIT", "G-code parser", "gcodepy", optional=True),
    # Group 12 — AR/VR
    ToolSpec("open3d", 12, "AR/VR Viz", "MIT", "3D data processing", "open3d"),
    # Group 13 — Knowledge Graph
    ToolSpec("neo4j", 13, "Knowledge Graph", "Apache 2.0", "Neo4j driver", "neo4j"),
    ToolSpec("falkordb", 13, "Knowledge Graph", "MIT", "FalkorDB driver", "falkordb", optional=True),
    ToolSpec("networkx", 13, "Knowledge Graph", "BSD", "Graph algorithms", "networkx"),
    ToolSpec("rdflib", 13, "Knowledge Graph", "BSD", "RDF/SPARQL", "rdflib"),
    # Group 14 — Web/API
    ToolSpec("fastapi", 14, "Web/API", "MIT", "REST API", "fastapi"),
    ToolSpec("streamlit", 14, "Web/API", "Apache 2.0", "Web UI", "streamlit", optional=True),
    ToolSpec("gradio", 14, "Web/API", "Apache 2.0", "AI demos", "gradio", optional=True),
    ToolSpec("websockets", 14, "Web/API", "BSD", "Real-time comms", "websockets", optional=True),
    ToolSpec("jinja2", 14, "Web/API", "BSD", "HTML templating", "jinja2"),
    # Group 15 — Data Pipeline
    ToolSpec("prefect", 15, "Data Pipeline", "Apache 2.0", "Workflow orchestration", "prefect", optional=True),
    ToolSpec("celery", 15, "Data Pipeline", "BSD", "Task queue", "celery", optional=True),
    ToolSpec("redis", 15, "Data Pipeline", "MIT", "Cache/broker", "redis", optional=True),
    ToolSpec("psycopg2", 15, "Data Pipeline", "BSD", "PostgreSQL driver", "psycopg2", optional=True),
    # Group 16 — DevOps
    ToolSpec("docker", 16, "DevOps", "Apache 2.0", "Containers", verify_cmd=["docker", "--version"], optional=True),
    ToolSpec("prometheus_client", 16, "DevOps", "Apache 2.0", "Metrics", "prometheus_client", optional=True),
    # Group 17 — Testing
    ToolSpec("pytest", 17, "Testing", "MIT", "Test framework", "pytest", optional=True),
    ToolSpec("pytest_cov", 17, "Testing", "MIT", "Coverage", "pytest_cov", optional=True),
    ToolSpec("hypothesis", 17, "Testing", "MPL 2.0", "Property testing", "hypothesis", optional=True),
    ToolSpec("ruff", 17, "Testing", "MIT", "Linter", verify_cmd=["ruff", "--version"], optional=True),
    ToolSpec("mypy", 17, "Testing", "MIT", "Type checker", verify_cmd=["mypy", "--version"], optional=True),
    # Group 18 — Utilities
    ToolSpec("rich", 18, "Utilities", "MIT", "Terminal formatting", "rich"),
    ToolSpec("tqdm", 18, "Utilities", "MPL 2.0", "Progress bars", "tqdm"),
    ToolSpec("loguru", 18, "Utilities", "MIT", "Logging", "loguru"),
    ToolSpec("click", 18, "Utilities", "BSD", "CLI framework", "click"),
    ToolSpec("typer", 18, "Utilities", "MIT", "Modern CLI", "typer"),
    ToolSpec("dotenv", 18, "Utilities", "BSD", "Env loading", "dotenv"),
    ToolSpec("requests", 18, "Utilities", "Apache 2.0", "HTTP client", "requests"),
    ToolSpec("httpx", 18, "Utilities", "BSD", "Async HTTP", "httpx"),
]


def verify_tool(spec: ToolSpec) -> ToolResult:
    try:
        if spec.verify_fn:
            ok = spec.verify_fn()
            if ok:
                return ToolResult(spec.name, spec.group, ToolStatus.OK, message="verify_fn passed",
                                  license=spec.license, purpose=spec.purpose)
            status = ToolStatus.OPTIONAL if spec.optional else ToolStatus.MISSING
            return ToolResult(spec.name, spec.group, status, message="verify_fn failed",
                              license=spec.license, purpose=spec.purpose)
        if spec.verify_cmd:
            ok, msg = _cmd_ok(spec.verify_cmd)
            if ok:
                return ToolResult(spec.name, spec.group, ToolStatus.OK, version=msg,
                                  license=spec.license, purpose=spec.purpose)
            status = ToolStatus.OPTIONAL if spec.optional else ToolStatus.MISSING
            return ToolResult(spec.name, spec.group, status, message=msg,
                              license=spec.license, purpose=spec.purpose)
        if spec.import_name:
            ok, info = _import_ok(spec.import_name)
            if ok:
                return ToolResult(spec.name, spec.group, ToolStatus.OK, version=info,
                                  license=spec.license, purpose=spec.purpose)
            status = ToolStatus.OPTIONAL if spec.optional else ToolStatus.MISSING
            return ToolResult(spec.name, spec.group, status, message=info,
                              license=spec.license, purpose=spec.purpose)
        return ToolResult(spec.name, spec.group, ToolStatus.ERROR, message="no verify method",
                          license=spec.license, purpose=spec.purpose)
    except Exception as exc:
        status = ToolStatus.OPTIONAL if spec.optional else ToolStatus.ERROR
        return ToolResult(spec.name, spec.group, status, message=str(exc),
                          license=spec.license, purpose=spec.purpose)


def verify_all(group: Optional[int] = None) -> List[ToolResult]:
    specs = [t for t in TOOLS if group is None or t.group == group]
    return [verify_tool(s) for s in specs]


def get_group_summary(group: Optional[int] = None) -> Dict[str, Any]:
    results = verify_all(group)
    by_group: Dict[int, Dict[str, Any]] = {}
    for r in results:
        g = by_group.setdefault(r.group, {
            "group": r.group,
            "name": next((t.group_name for t in TOOLS if t.group == r.group), f"Group {r.group}"),
            "total": 0, "ok": 0, "missing": 0, "optional_missing": 0, "error": 0, "tools": [],
        })
        g["total"] += 1
        g["tools"].append({"name": r.name, "status": r.status.value, "version": r.version, "message": r.message})
        if r.status == ToolStatus.OK:
            g["ok"] += 1
        elif r.status == ToolStatus.MISSING:
            g["missing"] += 1
        elif r.status == ToolStatus.OPTIONAL:
            g["optional_missing"] += 1
        else:
            g["error"] += 1
    return {"groups": list(by_group.values())}


def get_health_report() -> Dict[str, Any]:
    results = verify_all()
    ok = sum(1 for r in results if r.status == ToolStatus.OK)
    return {
        "status": "healthy" if ok > 0 else "degraded",
        "total_tools": len(results),
        "available": ok,
        "tools": [
            {"name": r.name, "group": r.group, "status": r.status.value,
             "version": r.version, "message": r.message, "license": r.license}
            for r in results
        ],
    }


def get_ui_status() -> Dict[str, Any]:
    """Compact status payload for desktop app / dashboard."""
    report = get_health_report()
    groups = get_group_summary().get("groups", [])
    return {
        "connected": True,
        "available": report.get("available", 0),
        "total_tools": report.get("total_tools", 0),
        "status": report.get("status", "degraded"),
        "groups": [
            {
                "group": g["group"],
                "name": g["name"],
                "ok": g["ok"],
                "total": g["total"],
                "health_pct": round(100.0 * g["ok"] / max(g["total"], 1), 1),
            }
            for g in groups
        ],
        "cad_ready": all(
            t["status"] == "OK"
            for t in report.get("tools", [])
            if t.get("group") == 2 and t.get("name") in {
                "build123d", "trimesh", "shapely", "cadquery", "scikit-spatial"
            }
        ),
        "mesh_ready": (
            any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "meshio")
            and (
                any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "gmsh")
                or any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "trimesh")
            )
        ),
        "fea_ready": (
            any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "scipy")
            and any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "numpy")
        ),
        "calculix_available": any(
            t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "calculix"
        ),
        "thermal_ready": (
            any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "CoolProp")
            and (
                any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "scipy")
                or any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "elmer")
            )
        ),
        "cfd_ready": (
            any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "CoolProp")
            or any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "openfoam")
            or any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "SU2")
        ),
        "optimization_ready": any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") in {"optuna", "openmdao", "pymoo"}),
        "materials_ready": any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") in {"pymatgen", "mendeleev", "ase"}),
        "manufacturing_ready": any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") in {"cura-engine", "gcodepy"}),
        "robotics_ready": any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") in {"urdfpy", "pybullet"}),
        "iot_ready": any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "paho-mqtt"),
        "ml_ready": any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") == "torch"),
        "pipeline_ready": any(t["status"] == "OK" for t in report.get("tools", []) if t.get("name") in {"prefect", "celery", "redis"}),
    }


def is_available(tool_name: str) -> bool:
    for spec in TOOLS:
        if spec.name == tool_name or spec.import_name == tool_name:
            return verify_tool(spec).status == ToolStatus.OK
    return False


if __name__ == "__main__":
    print(json.dumps(get_health_report(), indent=2))
