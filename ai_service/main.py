#!/usr/bin/env python3
"""
SZM Forge AI Service
Python backend for agentic reasoning, knowledge graphs, and design optimization
Integrates: Knowledge Graphs, Causal Reasoning, Training Loops, Data Ingestion
"""

import os
import sys
from typing import Dict, List, Optional, Tuple
import json
import logging
from fastapi import FastAPI, HTTPException, BackgroundTasks, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import numpy as np

# Import AI modules
from knowledge_graph import KnowledgeGraph, Node, Edge, NodeType, RelationType
from causal_reasoning import CausalReasoningEngine, ReasoningMode
from training_loop import TrainingLoop, TrainingObjective, LearningPhase
from data_ingestion import DataIngestionPipeline, SourceType, PatentCrawler, AcademicCrawler
from agentic_system import Agent, Tool, ToolType, Memory
from neo4j_client import Neo4jKnowledgeGraph
from simulation_integration import SimulationEngine, SimulationInputs, SolverType, SimulationOutputs
from batch_simulator import AutonomousBatchSimulator
from manufacturing_pipeline import ManufacturabilityValidator, GCodeExporter
from benchmarking import AIBenchmarker
from sandbox import EvolutionSandbox

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# ============================================================
# Data Models for API Requests
# ============================================================

# ======================== WebSocket Event Model ========================
class ForgeEvent(BaseModel):
    event_type: str
    payload: dict


class MaterialProperties(BaseModel):
    """Material properties for AI model"""
    name: str
    density: float
    youngs_modulus: float
    poissons_ratio: float
    yield_strength: float
    ultimate_strength: float
    thermal_conductivity: float
    specific_heat: float
    thermal_expansion: float

class DesignOptimizationRequest(BaseModel):
    """Request for design optimization"""
    component_id: int
    optimization_type: str  # "weight", "cost", "strength"
    constraints: Dict[str, float]
    bounds: Dict[str, Tuple[float, float]]
    material_id: Optional[int] = None

class FailurePredictionRequest(BaseModel):
    """Request for failure prediction"""
    component_id: int
    load_cases: List[Dict[str, float]]
    environment: Dict[str, float]
    material_id: int
    confidence_level: float = 0.95

class MaterialEstimationRequest(BaseModel):
    """Request for material property estimation"""
    unknown_properties: List[str]
    known_properties: Dict[str, float]
    material_category: str  # "metal", "composite", "polymer", etc

# New models for AI reasoning system
class ReasoningRequest(BaseModel):
    """Request for reasoning about a design problem"""
    problem: str
    reasoning_mode: str = "forward_chain"  # forward_chain, backward_chain, counterfactual
    context: Optional[Dict[str, str]] = None

class DesignProblemRequest(BaseModel):
    """Request to solve a design problem"""
    problem_description: str
    objectives: List[TrainingObjective] = [TrainingObjective.DESIGN_VALIDITY]
    constraints: Optional[Dict[str, float]] = None

class KnowledgeGraphQueryRequest(BaseModel):
    """Query knowledge graph"""
    query_type: str  # "node", "path", "similar", "relations"
    target: str
    filters: Optional[Dict[str, str]] = None

class DataIngestionRequest(BaseModel):
    """Request to ingest data sources"""
    source_type: str  # patent, paper, standard, datasheet, etc.
    title: str
    content: str
    metadata: Dict[str, str] = {}

class BatchSimulationRequest(BaseModel):
    """Request to run headless batch simulations"""
    num_variations: int = 20

class DFMValidationRequest(BaseModel):
    """Request for DFM validation"""
    component_name: str
    parameters: Dict[str, float]
    process: str = "CNC"

class GCodeExportRequest(BaseModel):
    """Request to export G-Code"""
    component_name: str
    bounds: Dict[str, float]

class GraphRAGRequest(BaseModel):
    """Request for GraphRAG query against Neo4j"""
    query: str

# ============================================================
# Integrated AI Service (SZM Forge Core)
# ============================================================

class SZMForgeAIService:
    """
    Comprehensive AI service for autonomous engineering design.
    
    Integrates:
    - Knowledge Graph (ontologies, design patterns)
    - Causal Reasoning Engine (inference, hypothesis testing)
    - Training Loop (learning from simulation feedback)
    - Data Ingestion (patents, papers, standards)
    - Agent System (planning, tool orchestration)
    """
    
    def __init__(self):
        logger.info("Initializing SZM Forge AI Service...")
        
        # Initialize core components
        self.knowledge_graph = KnowledgeGraph(name="SZM_Legendary_KG")
        self.neo4j_graph = Neo4jKnowledgeGraph()  # Connects to Neo4j database or falls back to mock
        self.reasoning_engine = CausalReasoningEngine(self.knowledge_graph)
        self.data_pipeline = DataIngestionPipeline(self.knowledge_graph)
        self.simulation_engine = SimulationEngine()
        self.batch_simulator = AutonomousBatchSimulator(self.simulation_engine, self.knowledge_graph, max_workers=8)
        self.active_training_status = "Idle"
        
        def run_sim(design_str: str, thickness: float):
            self.active_training_status = f"Testing {design_str} against C++ SimulationEngine"
            inputs = SimulationInputs(
                component_name="AI_Generated_Part",
                geometry_file="",
                material="Steel",
                material_properties={"materialId": "MAT-STEEL-STRUCT"},
                loads={"loadKg": 500.0}, # Force failure on first try
                boundary_conditions={},
                parameters={"thickness": thickness}
            )
            out = self.simulation_engine.run_structural_analysis(inputs, solver=SolverType.SZM_CPP_ENGINE)
            return [SimulationResult(
                design_id="ai_design",
                simulation_type="structural",
                parameters={"thickness": thickness},
                outputs={"max_stress": out.max_stress},
                passed_checks=(out.min_safety_factor >= 1.0),
                margin_of_safety=out.min_safety_factor,
                confidence=0.95
            )]
            
        self.training_loop = TrainingLoop(
            knowledge_graph=self.knowledge_graph,
            reasoning_engine=self.reasoning_engine,
            agent=None,
            simulator=run_sim,
            learning_rate=0.01
        )
        
        self.evolution_sandbox = EvolutionSandbox(self.training_loop)
        
        # Initialize crawlers
        self.patent_crawler = PatentCrawler()
        self.academic_crawler = AcademicCrawler()
        
        # Initialize knowledge base with core rules
        self._initialize_core_knowledge()
        self._initialize_core_rules()
        
        logger.info("SZM Forge AI Service initialized successfully")
        
    def _initialize_core_knowledge(self):
        """Populate knowledge graph with foundational engineering knowledge"""
        logger.info("Initializing core knowledge graph...")
        
        # Add materials
        materials = [
            {"name": "Titanium", "properties": {"density": 4.5, "strength": 900}},
            {"name": "Aluminum", "properties": {"density": 2.7, "strength": 300}},
            {"name": "Steel", "properties": {"density": 7.85, "strength": 400}},
        ]
        
        for mat in materials:
            node = Node(
                id=f"mat_{mat['name'].lower()}",
                name=mat['name'],
                node_type=NodeType.MATERIAL,
                properties=mat['properties'],
                source="Engineering Handbook"
            )
            self.knowledge_graph.add_node(node)
        
        logger.info(f"Added {len(materials)} material nodes to KG")
    
    def _initialize_core_rules(self):
        """Initialize basic causal rules for reasoning"""
        logger.info("Initializing causal rules...")
        
        # Basic physics rules
        self.reasoning_engine.add_rule(
            antecedent="high_load",
            consequent="material_stress",
            reasoning="High loads cause increased stress in materials",
            confidence=0.95
        )
        
        self.reasoning_engine.add_rule(
            antecedent="material_stress",
            consequent="yield_risk",
            reasoning="Material stress above yield strength causes permanent deformation",
            confidence=0.92
        )
        
        self.reasoning_engine.add_rule(
            antecedent="high_temperature",
            consequent="thermal_expansion",
            reasoning="Temperature increase causes materials to expand",
            confidence=0.98
        )
        
        logger.info("Core causal rules initialized")
    
    # =============== Knowledge Graph Operations ===============
    
    def query_knowledge_graph(self, query_type: str, target: str, filters: Optional[Dict] = None) -> Dict:
        """
        Query the knowledge graph.
        
        Query types:
        - "node": Get node details
        - "path": Find causal path between concepts
        - "similar": Find similar components
        - "relations": Get related entities
        """
        try:
            if query_type == "node":
                node = self.knowledge_graph.query_node(target)
                if node:
                    return {"found": True, "node": node.to_dict()}
                return {"found": False, "message": f"Node {target} not found"}
            
            elif query_type == "path":
                # Find causal path from source to target
                # (simplified - would need source specified in filters)
                return {"path": [], "message": "Causal path query requires source node"}
            
            elif query_type == "similar":
                similar = self.knowledge_graph.find_similar_components(target)
                return {"similar_to": target, "matches": similar}
            
            elif query_type == "relations":
                node = self.knowledge_graph.query_node(target)
                if not node:
                    return {"found": False}
                
                outgoing = self.knowledge_graph.get_outgoing_edges(target)
                incoming = self.knowledge_graph.get_incoming_edges(target)
                
                return {
                    "node": target,
                    "outgoing_relations": len(outgoing),
                    "incoming_relations": len(incoming)
                }
            
            else:
                return {"error": f"Unknown query type: {query_type}"}
        
        except Exception as e:
            logger.error(f"KG query error: {e}")
            return {"error": str(e)}
    
    # =============== Reasoning Operations ===============
    
    def reason_about_problem(self, problem: str, mode: str = "forward_chain") -> Dict:
        """
        Apply causal reasoning to understand a design problem.
        
        Reasoning modes:
        - "forward_chain": Given facts, derive conclusions
        - "backward_chain": Given goal, find prerequisites
        - "counterfactual": What-if scenario analysis
        """
        try:
            if mode == "forward_chain":
                # Extract facts from problem and forward chain
                facts = {"high_load", "structural_analysis"}  # Placeholder
                derived = self.reasoning_engine.forward_chain(facts)
                return {
                    "mode": mode,
                    "derived_facts": derived,
                    "count": len(derived)
                }
            
            elif mode == "backward_chain":
                # Find path to goal
                chain = self.reasoning_engine.backward_chain(problem)
                if chain:
                    return {
                        "goal": problem,
                        "chain": self.reasoning_engine.export_reasoning_chain(chain),
                        "confidence": chain.confidence
                    }
                return {"goal": problem, "found": False, "message": "No reasoning path found"}
            
            elif mode == "counterfactual":
                # Counterfactual: "If X were different, what would happen?"
                reasoning = self.reasoning_engine.counterfactual_reasoning(problem, "material", "aluminum")
                return {"counterfactual": reasoning}
            
            else:
                return {"error": f"Unknown reasoning mode: {mode}"}
        
        except Exception as e:
            logger.error(f"Reasoning error: {e}")
            return {"error": str(e)}
    
    # =============== Design Problem Solving ===============
    
    def solve_design_problem(self, problem: str, objectives: List[str]) -> Dict:
        """
        Solve a design problem using the training loop.
        """
        try:
            self.active_training_status = f"Starting autonomous problem solving: {problem}"
            obj = TrainingObjective[objectives[0].upper()] if objectives else TrainingObjective.DESIGN_VALIDITY
            
            # Run training episode (this will autonomously loop)
            episode = self.training_loop.run_episode(problem, objective=obj)
            self.active_training_status = f"Idle (Last episode {'succeeded' if episode.success else 'failed'})"
            
            return {
                "episode_id": episode.episode_id,
                "problem": problem,
                "success": episode.success,
                "reward": episode.reward,
                "reasoning": episode.reasoning_trace.to_dict() if episode.reasoning_trace else None,
                "duration_seconds": episode.duration_seconds
            }
        
        except Exception as e:
            logger.error(f"Design problem solving error: {e}")
            return {"error": str(e)}
    
    # =============== Batch Simulation ===============
    
    def run_batch_simulation(self, num_variations: int, background_tasks: BackgroundTasks) -> Dict:
        """Trigger autonomous batch learning"""
        if self.batch_simulator.is_running:
            return {"status": "error", "message": "Batch simulator is already running"}
            
        background_tasks.add_task(self.batch_simulator.run_headless_batch, num_variations)
        return {"status": "started", "message": f"Dispatched {num_variations} variations to background thread pool"}

    # =============== Data Ingestion ===============
    
    def ingest_data(self, source_type: str, title: str, content: str, metadata: Dict) -> Dict:
        """
        Ingest new knowledge from various sources.
        Supported sources: patent, paper, standard, datasheet, failure_analysis
        """
        try:
            source = SourceType[source_type.upper()]
            
            if source == SourceType.PATENT:
                doc = self.data_pipeline.ingest_patent(
                    title=title,
                    url=metadata.get("url", ""),
                    patent_id=metadata.get("patent_id", "unknown"),
                    abstract=metadata.get("abstract", ""),
                    content=content,
                    inventors=metadata.get("inventors", []),
                    publication_date=metadata.get("pub_date", ""),
                    claims=metadata.get("claims", [])
                )
            
            elif source == SourceType.TECHNICAL_PAPER:
                doc = self.data_pipeline.ingest_paper(
                    title=title,
                    url=metadata.get("url", ""),
                    arxiv_id=metadata.get("arxiv_id"),
                    authors=metadata.get("authors", []),
                    abstract=content[:500],  # Summary
                    content=content,
                    journal=metadata.get("journal", "")
                )
            
            elif source == SourceType.STANDARD:
                doc = self.data_pipeline.ingest_standard(
                    title=title,
                    standard_id=metadata.get("standard_id", "unknown"),
                    content=content,
                    organization=metadata.get("organization", "ISO"),
                    scope=metadata.get("scope", "")
                )
            
            elif source == SourceType.DATASHEET:
                doc = self.data_pipeline.ingest_datasheet(
                    component_name=metadata.get("component", ""),
                    manufacturer=metadata.get("manufacturer", ""),
                    content=content,
                    properties=metadata.get("properties", {}),
                    specifications=metadata.get("specs", {})
                )
            
            elif source == SourceType.FAILURE_ANALYSIS:
                doc = self.data_pipeline.ingest_failure_analysis(
                    incident_id=metadata.get("incident_id", "unknown"),
                    title=title,
                    description=content,
                    root_cause=metadata.get("root_cause", ""),
                    component_failed=metadata.get("component", ""),
                    failure_mode=metadata.get("failure_mode", ""),
                    mitigation=metadata.get("mitigation", ""),
                    source=metadata.get("source", "unknown")
                )
            
            else:
                return {"error": f"Unsupported source type: {source_type}"}
            
            # Process the document
            self.data_pipeline.process_document(doc)
            
            return {
                "status": "success",
                "document_id": doc.id,
                "source_type": source_type,
                "entities_extracted": len(doc.extracted_entities),
                "relations_extracted": len(doc.extracted_relations)
            }
        
        except Exception as e:
            logger.error(f"Data ingestion error: {e}")
            return {"error": str(e)}
    
    def get_corpus_stats(self) -> Dict:
        """Get statistics about ingested corpus"""
        return self.data_pipeline.get_corpus_stats()
    
    # =============== Training and Learning ===============
    
    def get_training_report(self) -> Dict:
        """Get comprehensive training report"""
        return self.training_loop.get_training_report()
    
    def export_training_data(self, filepath: str = "/tmp/training_data.json"):
        """Export training data for analysis"""
        self.training_loop.export_training_data(filepath)
        return {"status": "exported", "filepath": filepath}
    
    # =============== System Status ===============
    
    def get_system_status(self) -> Dict:
        """Get overall system status"""
        kg_stats = self.knowledge_graph.get_stats()
        corpus_stats = self.data_pipeline.get_corpus_stats()
        training_stats = self.training_loop.get_training_report()
        
        return {
            "service": "SZM Forge AI",
            "version": "2.0.0",
            "status": "operational",
            "knowledge_graph": kg_stats,
            "corpus": corpus_stats,
            "training": {
                "phase": self.training_loop.phase.value,
                "episodes": training_stats["total_episodes"],
                "success_rate": training_stats["success_rate"],
                "active_status": self.active_training_status,
                "batch_status": self.batch_simulator.active_batch_status
            }
        }

# Legacy AIService (kept for backward compatibility)
class AIService:
    """
    Legacy PyTorch-based AI/ML service for engineering analysis
    - Material property estimation (neural network)
    - Design space exploration (genetic algorithm)
    - Failure prediction (random forest / neural network)
    - Multi-objective optimization (Pareto front)
    """
    
    def __init__(self):
        self.models = {}
        self.scaler = None
        logger.info("Initializing Legacy AIService...")
        self._load_models()
        
    def _load_models(self):
        """Load pre-trained PyTorch models"""
        # TODO: Load trained models from checkpoints
        logger.info("Loading pre-trained models...")
        # self.material_estimator = torch.load("models/material_estimator.pth")
        # self.failure_predictor = torch.load("models/failure_predictor.pth")
        # self.material_estimator = torch.load("models/material_estimator.pth")
        # self.failure_predictor = torch.load("models/failure_predictor.pth")
        
    def estimate_material_properties(self, request: MaterialEstimationRequest) -> Dict:
        """
        Use neural network to estimate unknown material properties
        Given known properties, predict others
        """
        logger.info(f"Estimating properties for {len(request.unknown_properties)} properties")
        
        # Normalize inputs
        known_vec = np.array([request.known_properties.get(k, 0) for k in sorted(request.known_properties.keys())])
        
        # TODO: Run through neural network
        # predictions = self.material_estimator(torch.tensor(known_vec, dtype=torch.float32))
        
        predictions = {
            prop: float(np.random.normal(100, 10))  # Placeholder
            for prop in request.unknown_properties
        }
        
        return {
            "status": "success",
            "estimations": predictions,
            "confidence": 0.85,
            "material_category": request.material_category
        }
    
    def predict_failure(self, request: FailurePredictionRequest) -> Dict:
        """
        Predict likelihood of component failure under load cases
        Returns failure probability and failure modes
        """
        logger.info(f"Predicting failure for component {request.component_id}")
        
        # TODO: Prepare features from load cases and material properties
        # Combine: stress state, strain state, temperature, material properties
        # features = self._prepare_features(request)
        
        # TODO: Run through failure predictor model
        # failure_prob = self.failure_predictor(features)
        
        failure_prediction = {
            "component_id": request.component_id,
            "failure_probability": 0.12,  # Placeholder: 12% chance
            "failure_modes": [
                {"mode": "yielding", "probability": 0.08},
                {"mode": "fatigue", "probability": 0.03},
                {"mode": "brittle_fracture", "probability": 0.01}
            ],
            "safety_factor": 2.5,
            "recommended_material": "Aluminum 7075-T73",
            "confidence_level": request.confidence_level
        }
        
        return failure_prediction
    
    def optimize_design(self, request: DesignOptimizationRequest) -> Dict:
        """
        Optimize component design using:
        - Genetic algorithms for discrete topology
        - Gradient descent for parametric optimization
        - Multi-objective Pareto front exploration
        """
        logger.info(f"Optimizing design for component {request.component_id}")
        
        # TODO: Run optimization algorithm
        # if request.optimization_type == "weight":
        #     result = genetic_algorithm_minimize_weight(...)
        # elif request.optimization_type == "cost":
        #     result = gradient_optimize_cost(...)
        
        optimization_result = {
            "component_id": request.component_id,
            "optimization_type": request.optimization_type,
            "status": "completed",
            "original_value": 100.0,  # Placeholder: weight in kg
            "optimized_value": 75.5,  # Placeholder: 24.5% reduction
            "improvement_percent": 24.5,
            "pareto_solutions": [
                {"weight": 75.5, "cost": 120, "strength_factor": 2.1},
                {"weight": 78.0, "cost": 95, "strength_factor": 1.9},
                {"weight": 72.0, "cost": 150, "strength_factor": 2.4}
            ],
            "converged": True,
            "iterations": 47
        }
        
        return optimization_result

# ============================================================
# FastAPI Application
# ============================================================

app = FastAPI(
    title="SZM Forge AI Service",
    version="2.0.0",
    description="Agentic AI backend for autonomous engineering design with knowledge graphs and reasoning"
)

# Enable CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize services
szm_ai = SZMForgeAIService()  # New integrated AI service
legacy_ai = AIService()  # Legacy service for backward compatibility

# ======================== WebSocket Endpoint ========================
from fastapi import WebSocket
from typing import List

active_connections: List[WebSocket] = []

@app.websocket("/ws/events")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    active_connections.append(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            # Parse event
            try:
                event = ForgeEvent.parse_raw(data)
                # Here you can route event to AI logic
                logger.info(f"Received event: {event.event_type} | Payload: {event.payload}")
                # Example: respond to a specific event
                if event.event_type == "screen_action":
                    # Optionally, process with szm_ai and send live feedback
                    await websocket.send_text(json.dumps({"response": "Action received", "echo": event.payload}))
                else:
                    await websocket.send_text(json.dumps({"response": "Event received", "type": event.event_type}))
            except Exception as e:
                logger.error(f"WebSocket event error: {e}")
                await websocket.send_text(json.dumps({"error": str(e)}))
    except WebSocketDisconnect:
        active_connections.remove(websocket)
        logger.info("WebSocket disconnected")

# ======================== Health & Status ========================

@app.get("/health")
async def health():
    """Health check endpoint"""
    return {"status": "healthy", "service": "SZM_Forge_AI_v2"}

@app.get("/api/status")
async def system_status():
    """Get comprehensive system status"""
    return szm_ai.get_system_status()

# ======================== Knowledge Graph Endpoints ========================

@app.post("/api/kg/query")
async def kg_query(request: KnowledgeGraphQueryRequest):
    """Query the knowledge graph"""
    result = szm_ai.query_knowledge_graph(
        query_type=request.query_type,
        target=request.target,
        filters=request.filters
    )
    return result

@app.get("/api/kg/stats")
async def kg_stats():
    """Get knowledge graph statistics"""
    stats = szm_ai.knowledge_graph.get_stats()
    return stats

# ======================== GraphRAG Endpoints ========================

@app.post("/api/graphrag/query")
async def graphrag_query(request: GraphRAGRequest):
    """Query the Neo4j Knowledge Graph using GraphRAG"""
    try:
        result = szm_ai.neo4j_graph.graph_rag_lookup(request.query)
        # Format the response to be easily readable by the UI
        answer = f"{result['insight']}\n\n"
        if result['context']:
            answer += "Context from Knowledge Graph:\n"
            for item in result['context']:
                answer += f"- {item}\n"
        
        return {
            "query": request.query,
            "answer": answer,
            "context": result['context'],
            "confidence": 0.92,
            "matchedTerms": ["GraphRAG", "Neo4j"] + [str(k) for item in result['context'] for k in item.keys() if k == 'name' or k == 'type'][:3]
        }
    except Exception as e:
        logger.error(f"Error querying GraphRAG: {e}")
        raise HTTPException(status_code=500, detail=str(e))

# ======================== Reasoning Endpoints ========================

@app.post("/api/reason")
async def reason(request: ReasoningRequest):
    """Apply causal reasoning to a problem"""
    result = szm_ai.reason_about_problem(request.problem, request.reasoning_mode)
    return result

# ======================== Design Problem Solving ========================

@app.post("/api/solve-design-problem")
async def solve_design_problem(request: DesignProblemRequest, background_tasks: BackgroundTasks):
    """Solve a design problem using agentic reasoning"""
    result = szm_ai.solve_design_problem(
        problem=request.problem_description,
        objectives=[obj.value for obj in request.objectives]
    )
    return result

# ======================== Batch Learning Endpoints ========================

@app.post("/api/batch-simulate")
async def batch_simulate(request: BatchSimulationRequest, background_tasks: BackgroundTasks):
    """Run headless batch parameter sweeps to teach the AI"""
    return szm_ai.run_batch_simulation(request.num_variations, background_tasks)

# ======================== Manufacturing Endpoints ========================

@app.post("/api/manufacturability/validate")
async def validate_dfm(request: DFMValidationRequest):
    """Validate component for manufacturability"""
    return ManufacturabilityValidator.validate_design(request.component_name, request.parameters, request.process)

@app.post("/api/manufacturability/export-gcode")
async def export_gcode(request: GCodeExportRequest):
    """Export robotic tool paths for manufacturing"""
    gcode = GCodeExporter.export_gcode(request.component_name, request.bounds)
    return {"gcode": gcode}

# ======================== Evolution Endpoints ========================

@app.post("/api/evolution/benchmark")
async def run_benchmark():
    """Run an AI performance benchmark"""
    return AIBenchmarker.run_benchmark(szm_ai.training_loop, num_runs=3)

@app.post("/api/evolution/mutate")
async def run_mutation():
    """Run a sandbox parameter mutation"""
    return szm_ai.evolution_sandbox.mutate_parameters()

# ======================== Data Ingestion Endpoints ========================

@app.post("/api/ingest-data")
async def ingest_data(request: DataIngestionRequest):
    """Ingest new knowledge from various sources"""
    result = szm_ai.ingest_data(
        source_type=request.source_type,
        title=request.title,
        content=request.content,
        metadata=request.metadata
    )
    return result

@app.get("/api/corpus/stats")
async def corpus_stats():
    """Get corpus statistics"""
    return szm_ai.get_corpus_stats()

# ======================== Training & Learning Endpoints ========================

@app.get("/api/training/report")
async def training_report():
    """Get comprehensive training report"""
    return szm_ai.get_training_report()

@app.post("/api/training/export")
async def export_training(filepath: str = "/tmp/training_data.json"):
    """Export training data for analysis"""
    return szm_ai.export_training_data(filepath)

# ======================== Legacy Endpoints (backward compatibility) ========================

@app.post("/api/estimate-materials")
async def estimate_materials(request: MaterialEstimationRequest):
    """Estimate unknown material properties from known ones"""
    try:
        result = legacy_ai.estimate_material_properties(request)
        return result
    except Exception as e:
        logger.error(f"Error estimating materials: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/predict-failure")
async def predict_failure(request: FailurePredictionRequest):
    """Predict component failure probability and modes"""
    try:
        result = legacy_ai.predict_failure(request)
        return result
    except Exception as e:
        logger.error(f"Error predicting failure: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/optimize-design")
async def optimize_design(request: DesignOptimizationRequest, background_tasks: BackgroundTasks):
    """Optimize component design (async)"""
    try:
        result = legacy_ai.optimize_design(request)
        return result
    except Exception as e:
        logger.error(f"Error optimizing design: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/models")
async def list_models():
    """List available AI models"""
    return {
        "models": {
            "material_estimator": "PyTorch Neural Network",
            "failure_predictor": "Random Forest",
            "design_optimizer": "Genetic Algorithm + Gradient Descent",
            "knowledge_graph": "SZM Forge Ontology v2.0",
            "causal_reasoner": "Logic-based inference engine",
            "agentic_system": "Planning and tool orchestration"
        },
        "ai_service": "SZM Forge AI v2.0"
    }

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8001)
