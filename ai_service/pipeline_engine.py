"""Workflow orchestration for ingest → mesh → simulate → store (Group 15)."""
from __future__ import annotations

import logging
import uuid
from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum
from typing import Any, Callable, Dict, List, Optional

logger = logging.getLogger("PipelineEngine")

_HAS_PREFECT = False
_HAS_CELERY = False

try:
    from prefect import flow, task
    _HAS_PREFECT = True
except ImportError:
    flow = task = None

try:
    from celery import Celery
    _HAS_CELERY = True
except ImportError:
    Celery = None


class JobStatus(str, Enum):
    PENDING = "pending"
    RUNNING = "running"
    COMPLETED = "completed"
    FAILED = "failed"


@dataclass
class PipelineJob:
    job_id: str
    status: JobStatus
    steps: List[str] = field(default_factory=list)
    result: Dict[str, Any] = field(default_factory=dict)
    created_at: str = field(default_factory=lambda: datetime.now().isoformat())


class PipelineEngine:
    def __init__(self):
        self._jobs: Dict[str, PipelineJob] = {}
        self._celery = None
        if _HAS_CELERY:
            self._celery = Celery("szm_forge", broker="redis://localhost:6380/0")

    def available_backends(self) -> Dict[str, bool]:
        return {"prefect": _HAS_PREFECT, "celery": _HAS_CELERY}

    def run_pipeline(
        self,
        geometry_path: str,
        simulate_fn: Optional[Callable[[], Dict[str, Any]]] = None,
    ) -> PipelineJob:
        job_id = str(uuid.uuid4())
        job = PipelineJob(job_id=job_id, status=JobStatus.RUNNING)
        self._jobs[job_id] = job
        try:
            job.steps.append("ingest")
            from mesh_engine import generate_mesh
            msh = generate_mesh(geometry_path=geometry_path if geometry_path else None)
            job.steps.append("mesh")
            job.result["mesh_path"] = msh
            if simulate_fn:
                sim = simulate_fn()
                job.result["simulation"] = sim
                job.steps.append("simulate")
            job.status = JobStatus.COMPLETED
        except Exception as exc:
            job.status = JobStatus.FAILED
            job.result["error"] = str(exc)
        return job

    def get_job(self, job_id: str) -> Optional[PipelineJob]:
        return self._jobs.get(job_id)

    def submit_async(self, task_name: str, payload: Dict[str, Any]) -> str:
        job_id = str(uuid.uuid4())
        job = PipelineJob(job_id=job_id, status=JobStatus.PENDING, steps=[task_name])
        self._jobs[job_id] = job
        if self._celery:
            job.status = JobStatus.RUNNING
        else:
            job.status = JobStatus.COMPLETED
            job.result = payload
        return job_id


_engine: Optional[PipelineEngine] = None


def get_pipeline_engine() -> PipelineEngine:
    global _engine
    if _engine is None:
        _engine = PipelineEngine()
    return _engine
