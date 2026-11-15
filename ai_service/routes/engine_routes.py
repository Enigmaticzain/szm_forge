"""Additional engineering stack API routes."""
from __future__ import annotations

import ipaddress
import os
import re
import tempfile
from pathlib import Path
from typing import Any, Dict, List, Optional
from urllib.parse import urlparse

from fastapi import APIRouter, HTTPException, UploadFile
from pydantic import BaseModel, Field

router = APIRouter(tags=["engineering-stack"])

_PROJECT_ROOT = Path(__file__).resolve().parents[2]
_ELEMENT_SYMBOL_RE = re.compile(r"^(?:[A-Z][a-z]?|[a-z]{1,2})$")


def _allowed_roots() -> List[Path]:
    roots = [
        _PROJECT_ROOT,
        Path(tempfile.gettempdir()).resolve(),
    ]
    extra = os.environ.get("SZM_ALLOWED_PATHS", "")
    for entry in extra.split(os.pathsep):
        if entry.strip():
            roots.append(Path(entry.strip()).expanduser().resolve())
    return roots


def _resolve_safe_path(raw_path: str, *, must_exist: bool = False) -> str:
    if not raw_path or "\x00" in raw_path:
        raise HTTPException(status_code=400, detail="Invalid filesystem path")
    try:
        candidate = Path(raw_path).expanduser().resolve(strict=must_exist)
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=f"Path not found: {raw_path}") from exc
    except OSError as exc:
        raise HTTPException(status_code=400, detail=f"Invalid path: {exc}") from exc

    if not any(candidate.is_relative_to(root) for root in _allowed_roots()):
        raise HTTPException(status_code=403, detail="Path outside allowed workspace directories")
    return str(candidate)


def _validate_allowed_host(host: Optional[str], env_var: str) -> str:
    if not host:
        raise HTTPException(status_code=400, detail="Missing target host")
    normalized = host.strip().lower()
    allowed = {"127.0.0.1", "localhost", "::1"}
    extra = os.environ.get(env_var, "")
    for entry in extra.split(","):
        if entry.strip():
            allowed.add(entry.strip().lower())
    try:
        ip = ipaddress.ip_address(normalized)
        if ip.is_link_local or ip.is_unspecified:
            raise HTTPException(status_code=403, detail=f"Disallowed host IP: {normalized}")
    except ValueError:
        pass
    if normalized not in allowed:
        raise HTTPException(status_code=403, detail=f"Host '{normalized}' is not in the allowlist")
    return normalized


def _validate_opcua_endpoint(endpoint: str) -> str:
    parsed = urlparse(endpoint.strip())
    if parsed.scheme.lower() != "opc.tcp" or not parsed.hostname:
        raise HTTPException(status_code=400, detail="OPC-UA endpoint must use opc.tcp://scheme")
    _validate_allowed_host(parsed.hostname, "SZM_ALLOWED_OPCUA_HOSTS")
    return endpoint.strip()


class SurrogateTrainRequest(BaseModel):
    name: str = "default"
    backend: str = "torch"
    epochs: int = 100


class SurrogatePredictRequest(BaseModel):
    name: str = "default"
    inputs: Dict[str, float] = Field(default_factory=dict)


class OptimizationRequest(BaseModel):
    initial_stress_mpa: float = 847.0
    initial_mass_kg: float = 12.4
    target_stress_mpa: float = 800.0
    iterations: int = 50
    backend: str = "auto"


class MeshGenerateRequest(BaseModel):
    geometry_path: Optional[str] = None
    box_length: float = 1.0
    box_width: float = 1.0
    box_height: float = 1.0
    max_element_size: float = 0.05


class URDFParseRequest(BaseModel):
    content: str


class RoboticsSimRequest(BaseModel):
    steps: int = 240


class IoTMqttRequest(BaseModel):
    broker: str = "localhost"
    port: int = 1883
    topic: str = "szm/telemetry/#"


class PipelineRunRequest(BaseModel):
    geometry_path: str = ""


class OpcuaReadRequest(BaseModel):
    endpoint: str
    node_id: str


@router.post("/simulation/surrogate/train")
async def surrogate_train(req: SurrogateTrainRequest):
    from physics_ml_engine import get_physics_ml_engine
    return get_physics_ml_engine().train_surrogate(req.name, req.backend, epochs=req.epochs)


@router.post("/simulation/surrogate/predict")
async def surrogate_predict(req: SurrogatePredictRequest):
    from physics_ml_engine import get_physics_ml_engine
    return get_physics_ml_engine().predict(req.name, req.inputs)


@router.get("/mesh/libraries")
async def mesh_libraries():
    from engines.mesh_engine import get_available_libraries, get_active_backend
    libs = get_available_libraries()
    return {
        "status": "success",
        "libraries": libs,
        "active_backend": get_active_backend(),
        "mesh_ready": libs.get("gmsh") or libs.get("meshio") or libs.get("trimesh"),
    }


@router.post("/mesh/generate")
async def mesh_generate(req: MeshGenerateRequest):
    from engines.mesh_engine import generate_mesh, get_active_backend, get_available_libraries
    try:
        safe_geometry_path = (
            _resolve_safe_path(req.geometry_path, must_exist=True)
            if req.geometry_path
            else None
        )
        path = generate_mesh(
            geometry_path=safe_geometry_path,
            settings={"max_element_size": req.max_element_size},
            box_dims=(req.box_length, req.box_width, req.box_height),
        )
        backend = get_active_backend()
        return {
            "status": "success",
            "mesh_path": path,
            "backend": backend,
            "available_libraries": get_available_libraries(),
        }
    except HTTPException:
        raise
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc


class MeshConvertRequest(BaseModel):
    src_path: str
    dst_path: str


@router.get("/simulation/fea/status")
async def fea_status():
    from tool_registry import get_ui_status, is_available
    ui = get_ui_status()
    return {
        "status": "success",
        "fea_ready": ui.get("fea_ready", False),
        "calculix_available": ui.get("calculix_available", False),
        "mesh_ready": ui.get("mesh_ready", False),
        "scipy_available": is_available("scipy"),
        "numpy_available": is_available("numpy"),
        "solver": "calculix" if ui.get("calculix_available") else "analytical",
    }


@router.get("/simulation/thermal/status")
async def thermal_status():
    from tool_registry import get_ui_status, is_available
    from thermal_engine import get_available_libraries
    ui = get_ui_status()
    libs = get_available_libraries()
    return {
        "status": "success",
        "thermal_ready": ui.get("thermal_ready", False),
        "cfd_ready": ui.get("cfd_ready", False),
        "libraries": libs,
        "coolprop_available": is_available("CoolProp"),
        "solver": "elmer" if libs.get("elmer") else ("calculix" if ui.get("calculix_available") else "analytical"),
    }


@router.get("/simulation/thermal/fluid")
async def thermal_fluid_props(fluid: str = "Water", T_k: float = 300.0):
    from thermal_engine import fluid_properties
    try:
        props = fluid_properties(fluid, T_k=T_k)
        return {"status": "success", "fluid": fluid, "T_k": T_k, "properties": props}
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc


@router.post("/mesh/convert")
async def mesh_convert(req: MeshConvertRequest):
    from engines.mesh_engine import convert_mesh, get_available_libraries
    try:
        src_path = _resolve_safe_path(req.src_path, must_exist=True)
        dst_path = _resolve_safe_path(req.dst_path, must_exist=False)
        out = convert_mesh(src_path, dst_path)
        return {
            "status": "success",
            "output_path": out,
            "backend": "meshio",
            "available_libraries": get_available_libraries(),
        }
    except HTTPException:
        raise
    except FileNotFoundError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    except PermissionError as exc:
        raise HTTPException(status_code=403, detail=str(exc)) from exc
    except ValueError as exc:
        raise HTTPException(status_code=400, detail=str(exc)) from exc
    except RuntimeError as exc:
        code = 501 if "required" in str(exc).lower() else 500
        raise HTTPException(status_code=code, detail=str(exc)) from exc
    except Exception as exc:
        raise HTTPException(status_code=500, detail=str(exc)) from exc


@router.get("/materials/lookup/{formula}")
async def materials_lookup(formula: str):
    from material_synthesis import MaterialSynthesisEngine
    clean = formula.strip()
    if _ELEMENT_SYMBOL_RE.fullmatch(clean):
        return MaterialSynthesisEngine.lookup_element(clean)
    return MaterialSynthesisEngine.lookup_formula(clean)


@router.post("/robotics/urdf/parse")
async def robotics_urdf_parse(req: URDFParseRequest):
    from robotics_engine import parse_urdf
    return parse_urdf(req.content)


@router.post("/robotics/simulate")
async def robotics_simulate(req: RoboticsSimRequest):
    from robotics_engine import simulate_pybullet
    return simulate_pybullet(steps=req.steps)


@router.post("/iot/mqtt/start")
async def iot_mqtt_start(req: IoTMqttRequest):
    from iot_bridge import get_iot_bridge
    broker = _validate_allowed_host(req.broker, "SZM_ALLOWED_MQTT_HOSTS")
    if not (1 <= req.port <= 65535):
        raise HTTPException(status_code=400, detail="MQTT port must be between 1 and 65535")
    ok = get_iot_bridge().start_mqtt_subscriber(broker, req.port, req.topic)
    return {"status": "success" if ok else "error", "mqtt_started": ok}


@router.post("/iot/opcua/read")
async def iot_opcua_read(req: OpcuaReadRequest):
    from iot_bridge import get_iot_bridge
    endpoint = _validate_opcua_endpoint(req.endpoint)
    return get_iot_bridge().read_opcua(endpoint, req.node_id)


@router.post("/pipeline/run")
async def pipeline_run(req: PipelineRunRequest):
    from ai.pipeline_engine import get_pipeline_engine
    safe_path = _resolve_safe_path(req.geometry_path, must_exist=True) if req.geometry_path.strip() else ""
    job = get_pipeline_engine().run_pipeline(safe_path)
    return {
        "status": job.status.value,
        "job_id": job.job_id,
        "steps": job.steps,
        "result": job.result,
    }


@router.get("/pipeline/job/{job_id}")
async def pipeline_job(job_id: str):
    from ai.pipeline_engine import get_pipeline_engine
    job = get_pipeline_engine().get_job(job_id)
    if not job:
        raise HTTPException(status_code=404, detail="Job not found")
    return {"job_id": job.job_id, "status": job.status.value, "steps": job.steps, "result": job.result}


@router.post("/viz3d/load")
async def viz3d_load(path: str):
    from viz3d_engine import load_mesh
    safe_path = _resolve_safe_path(path, must_exist=True)
    return load_mesh(safe_path)


@router.get("/metrics")
async def prometheus_metrics():
    try:
        from prometheus_client import generate_latest, CONTENT_TYPE_LATEST
        from fastapi.responses import Response
        return Response(generate_latest(), media_type=CONTENT_TYPE_LATEST)
    except ImportError:
        return {"status": "prometheus_client not installed"}


@router.get("/data/search/arxiv")
async def data_search_arxiv(q: str, limit: int = 5):
    from data_ingestion import ArxivCrawler
    return {"results": ArxivCrawler().search(q, limit)}


@router.get("/data/search/patents")
async def data_search_patents(q: str, limit: int = 5):
    from data_ingestion import PatentCrawler
    return {"results": PatentCrawler().search_patents(q, limit)}


@router.get("/data/search/semantic-scholar")
async def data_search_semantic(q: str, limit: int = 5):
    from data_ingestion import SemanticScholarCrawler
    return {"results": SemanticScholarCrawler().search(q, limit)}


@router.get("/data/search/nasa")
async def data_search_nasa(q: str, limit: int = 5):
    from data_ingestion import NasaCrawler
    return {"results": NasaCrawler().search(q, limit)}


@router.post("/kg/rdf/ingest")
async def kg_rdf_ingest(doc_id: str, title: str, entities: str = ""):
    from kg_rdf import ingest_document_triples
    ent_list = [e.strip() for e in entities.split(",") if e.strip()]
    return ingest_document_triples(doc_id, title, ent_list)


@router.get("/data/search/nist")
async def data_search_nist(material: str = "steel"):
    from data_ingestion import NistMaterialsLookup
    return NistMaterialsLookup().lookup(material)


@router.post("/reports/fea/html")
async def fea_html_report(payload: Dict[str, Any]):
    from report_templates import render_fea_report
    from fastapi.responses import HTMLResponse
    return HTMLResponse(render_fea_report(payload))


@router.post("/api/import/step")
async def import_step_file(file: UploadFile):
    """Convert an uploaded STEP solid to three.js-ready JSON meshes (mm coordinates).

    Uses the existing build123d/OpenCascade backend. Returns an honest error
    payload when the engine is missing or the file is unreadable — never a
    placeholder mesh.
    """
    MAX_STEP_BYTES = 50 * 1024 * 1024
    raw = await file.read(MAX_STEP_BYTES + 1)
    if not raw:
        raise HTTPException(status_code=400, detail="Empty upload")
    if len(raw) > MAX_STEP_BYTES:
        raise HTTPException(status_code=413, detail="STEP file exceeds 50 MB limit")
    if b"ISO-10303-21" not in raw[:256]:
        raise HTTPException(status_code=400, detail="Not a STEP file (missing ISO-10303-21 header)")

    try:
        import build123d as bd  # noqa: WPS433 — optional heavy import, loaded on demand
    except ImportError as exc:
        raise HTTPException(
            status_code=503,
            detail="build123d not installed on this backend — use the in-browser WASM importer",
        ) from exc

    tmp_path = None
    try:
        with tempfile.NamedTemporaryFile(suffix=".step", delete=False) as tmp:
            tmp.write(raw)
            tmp_path = tmp.name
        shape = bd.import_step(tmp_path)
        solids = list(shape.solids()) or [shape]

        meshes: List[Dict[str, Any]] = []
        for idx, solid in enumerate(solids):
            vertices, triangles = solid.tessellate(tolerance=0.1)
            positions: List[float] = []
            for v in vertices:
                positions.extend((v.X, v.Y, v.Z))
            indices: List[int] = []
            for tri in triangles:
                indices.extend(tri)
            if not indices:
                continue
            meshes.append({
                "name": f"{file.filename or 'part'} solid {idx + 1}",
                "positions": positions,
                "indices": indices,
            })
        if not meshes:
            raise HTTPException(status_code=422, detail="STEP contained no tessellatable solids")
        return {"ok": True, "meshes": meshes, "units": "mm"}
    except HTTPException:
        raise
    except Exception as exc:  # honest surface for corrupt/unsupported STEP content
        raise HTTPException(status_code=422, detail=f"STEP import failed: {exc}") from exc
    finally:
        if tmp_path:
            try:
                os.unlink(tmp_path)
            except OSError:
                pass


def register_engine_routes(app) -> None:
    app.include_router(router)
