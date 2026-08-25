"""WebSocket telemetry broadcast helper (Group 14)."""
from __future__ import annotations

import asyncio
import json
import logging
from typing import Any, Dict, Set

logger = logging.getLogger("TelemetryWS")

try:
    import websockets
    _HAS_WS = True
except ImportError:
    websockets = None
    _HAS_WS = False

_subscribers: Set[Any] = set()


async def broadcast_telemetry(payload: Dict[str, Any]) -> None:
    """Push telemetry JSON to all connected WebSocket clients."""
    if not _subscribers:
        return
    msg = json.dumps(payload)
    dead = set()
    for ws in _subscribers:
        try:
            await ws.send(msg)
        except Exception:
            dead.add(ws)
    _subscribers -= dead


def register_subscriber(ws) -> None:
    _subscribers.add(ws)


def unregister_subscriber(ws) -> None:
    _subscribers.discard(ws)
