"""Additional engineering stack API routes."""
from __future__ import annotations

from typing import Any, Dict, List, Optional

from fastapi import APIRouter, HTTPException
from pydantic import BaseModel, Field

router = APIRouter(tags=["engineering-stack"])


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
    from mesh_engine import get_available_libraries, get_active_backend
    libs = get_available_libraries()
    return {
        "status": "success",
        "libraries": libs,
        "active_backend": get_active_backend(),
        "mesh_ready": libs.get("gmsh") or libs.get("meshio") or libs.get("trimesh"),
    }


@router.post("/mesh/generate")
async def mesh_generate(req: MeshGenerateRequest):
    from mesh_engine import generate_mesh, get_active_backend, get_available_libraries
    try:
        path = generate_mesh(
            geometry_path=req.geometry_path,
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
    from mesh_engine import convert_mesh, get_available_libraries
    try:
        out = convert_mesh(req.src_path, req.dst_path)
        return {
            "status": "success",
            "output_path": out,
            "backend": "meshio",
            "available_libraries": get_available_libraries(),
        }
    except Exception as exc:
        raise HTTPException(status_code=501, detail=str(exc)) from exc


@router.get("/materials/lookup/{formula}")
async def materials_lookup(formula: str):
    from material_synthesis import MaterialSynthesisEngine
    if len(formula) <= 3:
        return MaterialSynthesisEngine.lookup_element(formula)
    return MaterialSynthesisEngine.lookup_formula(formula)


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
    ok = get_iot_bridge().start_mqtt_subscriber(req.broker, req.port, req.topic)
    return {"status": "success" if ok else "error", "mqtt_started": ok}


@router.post("/iot/opcua/read")
async def iot_opcua_read(req: OpcuaReadRequest):
    from iot_bridge import get_iot_bridge
    return get_iot_bridge().read_opcua(req.endpoint, req.node_id)


@router.post("/pipeline/run")
async def pipeline_run(req: PipelineRunRequest):
    from pipeline_engine import get_pipeline_engine
    job = get_pipeline_engine().run_pipeline(req.geometry_path)
    return {
        "status": job.status.value,
        "job_id": job.job_id,
        "steps": job.steps,
        "result": job.result,
    }


@router.get("/pipeline/job/{job_id}")
async def pipeline_job(job_id: str):
    from pipeline_engine import get_pipeline_engine
    job = get_pipeline_engine().get_job(job_id)
    if not job:
        raise HTTPException(status_code=404, detail="Job not found")
    return {"job_id": job.job_id, "status": job.status.value, "steps": job.steps, "result": job.result}


@router.post("/viz3d/load")
async def viz3d_load(path: str):
    from viz3d_engine import load_mesh
    return load_mesh(path)


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


def register_engine_routes(app) -> None:
    app.include_router(router)
