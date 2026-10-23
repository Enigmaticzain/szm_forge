"""Robotics simulation helpers (Group 10)."""
from __future__ import annotations

import logging
import os
import tempfile
from typing import Any, Dict, List, Optional

logger = logging.getLogger("RoboticsEngine")

_HAS_URDFPY = False
_HAS_PYBULLET = False

try:
    import urdfpy
    _HAS_URDFPY = True
except ImportError:
    urdfpy = None

try:
    import pybullet as p
    import pybullet_data
    _HAS_PYBULLET = True
except ImportError:
    p = None
    pybullet_data = None


def available_backends() -> Dict[str, bool]:
    return {"urdfpy": _HAS_URDFPY, "pybullet": _HAS_PYBULLET}


def parse_urdf(content: str) -> Dict[str, Any]:
    if not _HAS_URDFPY:
        return {"status": "error", "message": "urdfpy not installed"}
    with tempfile.NamedTemporaryFile(mode="w", suffix=".urdf", delete=False) as f:
        f.write(content)
        path = f.name
    try:
        robot = urdfpy.URDF.load(path)
        links = [l.name for l in robot.links]
        joints = [j.name for j in robot.joints]
        return {"status": "success", "links": links, "joints": joints, "link_count": len(links)}
    finally:
        os.unlink(path)


def simulate_pybullet(steps: int = 240, gravity: float = -9.81) -> Dict[str, Any]:
    if not _HAS_PYBULLET:
        return {"status": "error", "message": "pybullet not installed"}
    cid = p.connect(p.DIRECT)
    try:
        p.setAdditionalSearchPath(pybullet_data.getDataPath())
        p.setGravity(0, 0, gravity)
        p.loadURDF("r2d2.urdf")
        positions: List[List[float]] = []
        for _ in range(steps):
            p.stepSimulation()
            pos, _ = p.getBasePositionAndOrientation(1)
            positions.append(list(pos))
        return {"status": "success", "steps": steps, "final_position": positions[-1] if positions else [0, 0, 0]}
    finally:
        p.disconnect(cid)
