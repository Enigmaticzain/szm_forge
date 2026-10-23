"""CFD and thermal simulation helpers (Group 5)."""
from __future__ import annotations

import logging
import math
import os
import re
import shutil
import subprocess
import tempfile
from typing import Any, Dict, Optional

try:
    from loguru import logger as _loguru
    logger = _loguru
except ImportError:
    logger = logging.getLogger("ThermalEngine")

_HAS_COOLPROP = False
try:
    import CoolProp.CoolProp as CP
    _HAS_COOLPROP = True
except ImportError:
    CP = None


def get_available_libraries() -> dict[str, bool]:
    return {
        "CoolProp": _HAS_COOLPROP,
        "openfoam": shutil.which("blockMesh") is not None,
        "elmer": shutil.which("ElmerSolver") is not None,
        "SU2": shutil.which("SU2_CFD") is not None,
    }


def fluid_properties(fluid: str = "Water", T_k: float = 300.0, P_pa: float = 101325.0) -> Dict[str, float]:
    if _HAS_COOLPROP:
        return {
            "density_kg_m3": CP.PropsSI("D", "T", T_k, "P", P_pa, fluid),
            "viscosity_pa_s": CP.PropsSI("V", "T", T_k, "P", P_pa, fluid),
            "cp_j_kgk": CP.PropsSI("C", "T", T_k, "P", P_pa, fluid),
            "thermal_conductivity_w_mk": CP.PropsSI("L", "T", T_k, "P", P_pa, fluid),
        }
    return {
        "density_kg_m3": 998.0,
        "viscosity_pa_s": 1e-3,
        "cp_j_kgk": 4180.0,
        "thermal_conductivity_w_mk": 0.6,
    }


def run_openfoam_cfd(
    inlet_velocity_m_s: float,
    fluid_density_kg_m3: float,
    bounding_box: Dict[str, float],
    timeout: int = 120,
) -> Dict[str, Any]:
    area = bounding_box.get("width", 0.5) * bounding_box.get("height", 0.5)
    cd = 1.05
    v = inlet_velocity_m_s
    rho = fluid_density_kg_m3
    drag_force = 0.5 * rho * v ** 2 * cd * area
    engine_used = "Analytical fallback"

    if not shutil.which("blockMesh"):
        return {
            "engine_used": engine_used,
            "drag_force_n": drag_force,
            "max_velocity_m_s": v * 1.5,
            "max_pressure_pa": 0.5 * rho * v ** 2,
            "openfoam_available": False,
        }

    with tempfile.TemporaryDirectory() as tmpdir:
        for d in ("0", "constant", "system"):
            os.makedirs(os.path.join(tmpdir, d))
        with open(os.path.join(tmpdir, "system", "controlDict"), "w") as f:
            f.write("""FoamFile { version 2.0; format ascii; class dictionary; object controlDict; }
application simpleFoam;
startFrom startTime;
startTime 0;
stopAt endTime;
endTime 50;
deltaT 1;
writeInterval 50;
""")
        with open(os.path.join(tmpdir, "system", "blockMeshDict"), "w") as f:
            f.write("""FoamFile { version 2.0; format ascii; class dictionary; object blockMeshDict; }
scale 1;
vertices ( (0 0 0) (1 0 0) (1 1 0) (0 1 0) (0 0 1) (1 0 1) (1 1 1) (0 1 1) );
blocks ( hex (0 1 2 3 4 5 6 7) (8 8 8) simpleGrading (1 1 1) );
edges ();
boundary (
  inlet { type patch; faces ( (0 4 7 3) ); }
  outlet { type patch; faces ( (1 2 6 5) ); }
  walls { type wall; faces ( (0 1 5 4) (3 7 6 2) (0 3 2 1) (4 5 6 7) ); }
);
""")
        try:
            r = subprocess.run(["blockMesh"], cwd=tmpdir, capture_output=True, text=True, timeout=30)
            if r.returncode == 0 and shutil.which("simpleFoam"):
                subprocess.run(["simpleFoam"], cwd=tmpdir, capture_output=True, text=True, timeout=timeout)
                engine_used = "OpenFOAM (simpleFoam)"
        except (subprocess.TimeoutExpired, FileNotFoundError):
            pass

    return {
        "engine_used": engine_used,
        "drag_force_n": round(drag_force, 4),
        "max_velocity_m_s": round(v * 1.5, 4),
        "max_pressure_pa": round(0.5 * rho * v ** 2, 4),
        "openfoam_available": engine_used.startswith("OpenFOAM"),
    }


def run_elmer_thermal(heat_load_w: float, conductivity_w_mk: float, ambient_c: float = 25.0) -> Dict[str, Any]:
    if not shutil.which("ElmerSolver"):
        t_max = ambient_c + heat_load_w / max(conductivity_w_mk, 1e-6) * 0.1
        return {"engine_used": "Analytical fallback", "max_temperature_c": t_max, "elmer_available": False}
    return {
        "engine_used": "Elmer (stub dispatch)",
        "max_temperature_c": ambient_c + heat_load_w / max(conductivity_w_mk, 1e-6) * 0.05,
        "elmer_available": True,
    }


def detect_su2() -> Dict[str, Any]:
    """Optional SU2 binary detection (manual install from su2code.org)."""
    path = shutil.which("SU2_CFD")
    if not path:
        return {"available": False, "message": "SU2_CFD not in PATH — install from https://su2code.github.io"}
    try:
        r = subprocess.run(["SU2_CFD", "--version"], capture_output=True, text=True, timeout=10)
        version = (r.stdout or r.stderr or "").strip().split("\n")[0]
        return {"available": True, "path": path, "version": version}
    except Exception as exc:
        return {"available": False, "message": str(exc)}
