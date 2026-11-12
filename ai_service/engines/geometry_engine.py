"""Backward-compatible re-export of canonical `ai_service/geometry_engine.py`."""
from __future__ import annotations

import importlib
import os
import sys

_PARENT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _PARENT_DIR not in sys.path:
    sys.path.insert(0, _PARENT_DIR)

try:
    _impl = importlib.import_module("geometry_engine")
except ImportError:
    _impl = importlib.import_module("ai_service.geometry_engine")

sys.modules[__name__] = _impl
