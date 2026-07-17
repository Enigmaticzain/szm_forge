import logging
import time
from typing import Dict, List, Any, Optional
try:
    from neo4j import GraphDatabase
    NEO4J_AVAILABLE = True
except ImportError:
    NEO4J_AVAILABLE = False

logger = logging.getLogger(__name__)

class Neo4jKnowledgeGraph:
    """
    Interface for the Industrial Knowledge Graph.
    Uses Neo4j if available, otherwise falls back to an in-memory mock graph
    to ensure the service remains operational.
    """
    def __init__(self, uri="bolt://localhost:7687", user="neo4j", password="password"):
        self.driver = None
        self.connected = False
        
        if NEO4J_AVAILABLE:
            try:
                self.driver = GraphDatabase.driver(uri, auth=(user, password))
                self.driver.verify_connectivity()
                self.connected = True
                logger.info("Successfully connected to Neo4j Knowledge Graph.")
                self._initialize_schema()
                self._seed_mock_designs()
            except Exception as e:
                logger.warning(f"Failed to connect to Neo4j: {e}. Falling back to in-memory GraphRAG.")
                self.connected = False
        else:
            logger.warning("neo4j python package not installed. Falling back to in-memory GraphRAG.")
            
        # In-memory fallback dataset (used if Neo4j is offline)
        self.mock_nodes = {
            "node_1": {"id": "node_1", "type": "Part", "name": "Table Leg", "material": "Steel", "stress": 850},
            "node_2": {"id": "node_2", "type": "Part", "name": "Tabletop", "material": "Wood", "stress": 120},
            "node_3": {"id": "node_3", "type": "Joint", "name": "Leg-Top Bracket", "stress": 950, "status": "critical"},
            "node_4": {"id": "node_4", "type": "Material", "name": "Steel 4340", "yield_strength": 800},
            "node_5": {"id": "node_5", "type": "Force", "name": "Vertical Load", "magnitude_N": 2000}
        }
        
    def close(self):
        if self.driver:
            self.driver.close()

    def _initialize_schema(self):
        """Creates constraints and indexes for EngineeringNode."""
        if not self.connected:
            return
        query = """
        CREATE CONSTRAINT engineering_node_id IF NOT EXISTS
        FOR (n:EngineeringNode) REQUIRE n.id IS UNIQUE
        """
        try:
            with self.driver.session() as session:
                session.run(query)
                logger.info("Neo4j schema initialized: EngineeringNode constraints created.")
        except Exception as e:
            logger.error(f"Error initializing schema: {e}")

    def _seed_mock_designs(self):
        """Seeds the database with proven parametric design graphs (Table, Ladder, etc)."""
        if not self.connected:
            return
            
        query = """
        MERGE (leg:EngineeringNode:Part {id: 'leg_v1'}) SET leg.name = 'Table Leg', leg.material = 'Steel', leg.stress = 850
        MERGE (top:EngineeringNode:Part {id: 'top_v1'}) SET top.name = 'Tabletop', top.material = 'Wood', top.stress = 120
        MERGE (joint:EngineeringNode:Joint {id: 'joint_v1'}) SET joint.name = 'Leg-Top Bracket', joint.stress = 950, joint.status = 'critical'
        MERGE (mat:EngineeringNode:Material {id: 'mat_steel'}) SET mat.name = 'Steel 4340', mat.yield_strength = 800
        MERGE (force:EngineeringNode:Force {id: 'force_v1'}) SET force.name = 'Vertical Load', force.magnitude_N = 2000
        
        MERGE (joint)-[:CONNECTS]->(leg)
        MERGE (joint)-[:CONNECTS]->(top)
        MERGE (leg)-[:MADE_OF]->(mat)
        MERGE (force)-[:APPLIED_TO]->(top)
        """
        try:
            with self.driver.session() as session:
                session.run(query)
                logger.info("Neo4j mock designs seeded.")
        except Exception as e:
            logger.error(f"Error seeding mock designs: {e}")

    def graph_rag_lookup(self, query_text: str) -> Dict[str, Any]:
        """
        Performs a GraphRAG lookup based on the user's query.
        """
        query_text_lower = query_text.lower()
        
        if self.connected:
            # We construct a cypher query based on keyword matching
            # In a real GraphRAG, this would use vector embeddings mapped to nodes.
            if "stress" in query_text_lower and "joint" in query_text_lower:
                cypher = """
                MATCH (j:EngineeringNode:Joint)-[:CONNECTS]->(p:Part)
                WHERE j.stress > 800
                RETURN j.name as joint_name, j.stress as stress, p.name as connected_part
                LIMIT 5
                """
                with self.driver.session() as session:
                    result = session.run(cypher)
                    records = [dict(record) for record in result]
                
                return {
                    "matched_nodes": len(records),
                    "context": records,
                    "insight": f"Found {len(records)} high-stress joints. Consider reinforcing these brackets or using higher yield strength materials."
                }
            else:
                # Generic fallback query
                cypher = """
                MATCH (n:EngineeringNode)
                RETURN labels(n) as labels, n.name as name LIMIT 3
                """
                with self.driver.session() as session:
                    result = session.run(cypher)
                    records = [dict(record) for record in result]
                return {
                    "matched_nodes": len(records),
                    "context": records,
                    "insight": "Found related engineering nodes in the graph."
                }
        else:
            # In-memory mock response for GraphRAG
            time.sleep(0.5) # Simulate latency
            if "stress" in query_text_lower and "joint" in query_text_lower:
                return {
                    "matched_nodes": 1,
                    "context": [self.mock_nodes["node_3"]],
                    "insight": "In-memory GraphRAG: Found critical joint 'Leg-Top Bracket' exceeding 900 MPa. Suggest increasing bracket thickness by 2mm."
                }
            elif "lift" in query_text_lower or "formula" in query_text_lower:
                return {
                    "matched_nodes": 1,
                    "context": [{"formula": "L = 1/2 * d * v^2 * s * CL"}],
                    "insight": "Lift is generated according to L = 0.5 * rho * v^2 * S * CL."
                }
            else:
                return {
                    "matched_nodes": len(self.mock_nodes),
                    "context": list(self.mock_nodes.values()),
                    "insight": "In-memory GraphRAG: Analyzing entire local schema. Multiple structural nodes detected."
                }
