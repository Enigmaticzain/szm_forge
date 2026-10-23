"""Stack health and integration routes for external_engine_bridge."""
from __future__ import annotations

from typing import Optional

from fastapi import APIRouter

from tool_registry import get_group_summary, get_health_report, get_ui_status

router = APIRouter(prefix="/stack", tags=["stack"])


@router.get("/health")
async def stack_health():
    return get_health_report()


@router.get("/status")
async def stack_status():
    """Compact status for SZM Forge desktop dashboard."""
    return get_ui_status()


@router.get("/groups")
async def stack_groups():
    return get_group_summary()


@router.get("/groups/{group_id}")
async def stack_group_detail(group_id: int):
    return get_group_summary(group_id)


def register_stack_routes(app) -> None:
    app.include_router(router)
