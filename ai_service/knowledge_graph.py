"""
Knowledge Graph and Ontology System for SZM Forge

This module implements the foundation for a domain knowledge graph that captures:
- Engineering components and their relationships
- Material properties and constraints
- Failure modes and mitigation strategies
- Design rationale and causal connections
"""

from typing import Dict, List, Optional, Set, Tuple, Any
from dataclasses import dataclass, field
from enum import Enum
import json
import logging
from datetime import datetime
import uuid

logger = logging.getLogger(__name__)


class EntityType(Enum):
    """Ontology entity types"""
    COMPONENT = "component"
    MATERIAL = "material"
    FUNCTION = "function"
    CONSTRAINT = "constraint"
    FAILURE_MODE = "failure_mode"
    DESIGN_PRINCIPLE = "design_principle"
    STANDARD = "standard"
    PROPERTY = "property"
    SYSTEM = "system"


class RelationType(Enum):
    """Relationship types in the knowledge graph"""
    COMPOSED_OF = "composed_of"
    SUPPORTS_FUNCTION = "supports_function"
    SUBJECT_TO_CONSTRAINT = "subject_to_constraint"
    CAUSES_FAILURE = "causes_failure"
    MITIGATES = "mitigates"
    IMPLEMENTS_PRINCIPLE = "implements_principle"
    CONFORMS_TO_STANDARD = "conforms_to_standard"
    HAS_PROPERTY = "has_property"
    SIMILAR_TO = "similar_to"
    DERIVED_FROM = "derived_from"
    INFLUENCES = "influences"
    ENABLES = "enables"


@dataclass
class Entity:
    """Core ontology entity"""
    id: str
    name: str
    entity_type: EntityType
    description: str
    properties: Dict[str, Any] = field(default_factory=dict)
    metadata: Dict[str, Any] = field(default_factory=dict)
    created_at: datetime = field(default_factory=datetime.now)
    updated_at: datetime = field(default_factory=datetime.now)
    source: Optional[str] = None  # Paper, patent, standard, etc.
    confidence: float = 0.95  # Confidence level (0-1)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "name": self.name,
            "type": self.entity_type.value,
            "description": self.description,
            "properties": self.properties,
            "metadata": self.metadata,
            "source": self.source,
            "confidence": self.confidence
        }


@dataclass
class Relationship:
    """Relationship between entities with causal semantics"""
    id: str
    source_id: str
    target_id: str
    relation_type: RelationType
    strength: float = 0.8  # Strength of relationship (0-1)
    causal: bool = False  # Is this a causal relationship?
    reasoning: str = ""  # Why does this relationship exist?
    evidence: List[str] = field(default_factory=list)  # References
    bidirectional: bool = False
    metadata: Dict[str, Any] = field(default_factory=dict)
    created_at: datetime = field(default_factory=datetime.now)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "source": self.source_id,
            "target": self.target_id,
            "type": self.relation_type.value,
            "strength": self.strength,
            "causal": self.causal,
            "reasoning": self.reasoning,
            "evidence": self.evidence
        }


class KnowledgeGraph:
    """
    Ontology-driven knowledge graph for engineering domain.
    
    Stores and retrieves engineering knowledge including:
    - Components, materials, functions
    - Constraints and failure modes
    - Causal relationships
    - Design rationales
    """
    
    def __init__(self, name: str = "SZM_Forge_KG"):
        self.name = name
        self.entities: Dict[str, Entity] = {}
        self.relationships: Dict[str, Relationship] = {}
        self.entity_index: Dict[str, Set[str]] = {}  # type -> set of entity_ids
        self.logger = logger
        
    def add_entity(
        self,
        name: str,
        entity_type: EntityType,
        description: str,
        properties: Optional[Dict[str, Any]] = None,
        source: Optional[str] = None,
        confidence: float = 0.95,
        metadata: Optional[Dict[str, Any]] = None
    ) -> Entity:
        """Add entity to knowledge graph"""
        entity_id = str(uuid.uuid4())
        
        entity = Entity(
            id=entity_id,
            name=name,
            entity_type=entity_type,
            description=description,
            properties=properties or {},
            source=source,
            confidence=confidence,
            metadata=metadata or {}
        )
        
        self.entities[entity_id] = entity
        
        # Index by type
        if entity_type not in self.entity_index:
            self.entity_index[entity_type] = set()
        self.entity_index[entity_type].add(entity_id)
        
        self.logger.info(f"Added entity: {name} ({entity_type.value})")
        return entity
    
    def add_relationship(
        self,
        source_id: str,
        target_id: str,
        relation_type: RelationType,
        strength: float = 0.8,
        causal: bool = False,
        reasoning: str = "",
        evidence: Optional[List[str]] = None,
        bidirectional: bool = False,
        metadata: Optional[Dict[str, Any]] = None
    ) -> Relationship:
        """Add relationship between entities"""
        rel_id = str(uuid.uuid4())
        
        relationship = Relationship(
            id=rel_id,
            source_id=source_id,
            target_id=target_id,
            relation_type=relation_type,
            strength=strength,
            causal=causal,
            reasoning=reasoning,
            evidence=evidence or [],
            bidirectional=bidirectional,
            metadata=metadata or {}
        )
        
        self.relationships[rel_id] = relationship
        
        if bidirectional and source_id != target_id:
            reverse_rel_id = str(uuid.uuid4())
            reverse_rel = Relationship(
                id=reverse_rel_id,
                source_id=target_id,
                target_id=source_id,
                relation_type=relation_type,
                strength=strength,
                causal=causal,
                reasoning=reasoning,
                evidence=evidence or [],
                bidirectional=False,
                metadata=metadata or {}
            )
            self.relationships[reverse_rel_id] = reverse_rel
        
        self.logger.info(
            f"Added relationship: {source_id} --{relation_type.value}--> {target_id}"
        )
        return relationship
    
    def get_entity(self, entity_id: str) -> Optional[Entity]:
        """Retrieve entity by ID"""
        return self.entities.get(entity_id)
    
    def find_entity(self, name: str) -> Optional[Entity]:
        """Find entity by name"""
        for entity in self.entities.values():
            if entity.name.lower() == name.lower():
                return entity
        return None
    
    def get_entities_by_type(self, entity_type: EntityType) -> List[Entity]:
        """Get all entities of a specific type"""
        entity_ids = self.entity_index.get(entity_type, set())
        return [self.entities[eid] for eid in entity_ids]
    
    def get_neighbors(
        self,
        entity_id: str,
        relation_types: Optional[List[RelationType]] = None,
        depth: int = 1
    ) -> Dict[str, List[Entity]]:
        """
        Get neighboring entities (graph traversal).
        Returns dict mapping relation_type -> list of neighboring entities
        """
        neighbors = {}
        visited = {entity_id}
        
        def traverse(current_id: str, current_depth: int):
            if current_depth > depth:
                return
            
            for rel in self.relationships.values():
                # Check outgoing relationships
                if rel.source_id == current_id:
                    if relation_types is None or rel.relation_type in relation_types:
                        target_entity = self.entities.get(rel.target_id)
                        if target_entity and rel.target_id not in visited:
                            rel_type = rel.relation_type.value
                            if rel_type not in neighbors:
                                neighbors[rel_type] = []
                            neighbors[rel_type].append(target_entity)
                            visited.add(rel.target_id)
                            traverse(rel.target_id, current_depth + 1)
        
        traverse(entity_id, 0)
        return neighbors
    
    def find_causal_paths(
        self,
        source_id: str,
        target_id: str,
        max_depth: int = 5
    ) -> List[List[Entity]]:
        """
        Find causal paths between entities.
        Returns list of paths (each path is list of entities)
        """
        paths = []
        
        def dfs(current_id: str, target: str, path: List[str], visited: Set[str]):
            if current_id == target:
                paths.append([self.entities[eid] for eid in path])
                return
            
            if len(path) >= max_depth or current_id in visited:
                return
            
            visited.add(current_id)
            
            for rel in self.relationships.values():
                if rel.source_id == current_id and rel.causal:
                    next_id = rel.target_id
                    if next_id not in visited:
                        dfs(next_id, target, path + [next_id], visited.copy())
        
        dfs(source_id, target_id, [source_id], set())
        return paths
    
    def query_by_properties(
        self,
        entity_type: EntityType,
        properties: Dict[str, Any]
    ) -> List[Entity]:
        """Query entities by type and properties"""
        results = []
        for entity in self.get_entities_by_type(entity_type):
            matches = all(
                entity.properties.get(k) == v
                for k, v in properties.items()
            )
            if matches:
                results.append(entity)
        return results
    
    def export_to_json(self, filepath: str):
        """Export knowledge graph to JSON"""
        export_data = {
            "name": self.name,
            "entities": [e.to_dict() for e in self.entities.values()],
            "relationships": [r.to_dict() for r in self.relationships.values()],
            "exported_at": datetime.now().isoformat()
        }
        
        with open(filepath, 'w') as f:
            json.dump(export_data, f, indent=2)
        
        self.logger.info(f"Knowledge graph exported to {filepath}")
    
    def import_from_json(self, filepath: str):
        """Import knowledge graph from JSON"""
        with open(filepath, 'r') as f:
            data = json.load(f)
        
        # Reconstruct entities
        for entity_data in data.get("entities", []):
            entity = Entity(
                id=entity_data["id"],
                name=entity_data["name"],
                entity_type=EntityType(entity_data["type"]),
                description=entity_data["description"],
                properties=entity_data.get("properties", {}),
                source=entity_data.get("source"),
                confidence=entity_data.get("confidence", 0.95),
                metadata=entity_data.get("metadata", {})
            )
            self.entities[entity.id] = entity
            
            # Index
            if entity.entity_type not in self.entity_index:
                self.entity_index[entity.entity_type] = set()
            self.entity_index[entity.entity_type].add(entity.id)
        
        # Reconstruct relationships
        for rel_data in data.get("relationships", []):
            rel = Relationship(
                id=rel_data["id"],
                source_id=rel_data["source"],
                target_id=rel_data["target"],
                relation_type=RelationType(rel_data["type"]),
                strength=rel_data.get("strength", 0.8),
                causal=rel_data.get("causal", False),
                reasoning=rel_data.get("reasoning", ""),
                evidence=rel_data.get("evidence", [])
            )
            self.relationships[rel.id] = rel
        
        self.logger.info(f"Knowledge graph imported from {filepath}")
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get knowledge graph statistics"""
        entity_counts = {etype.value: len(eids) for etype, eids in self.entity_index.items()}
        relation_counts = {}
        
        for rel in self.relationships.values():
            rtype = rel.relation_type.value
            relation_counts[rtype] = relation_counts.get(rtype, 0) + 1
        
        return {
            "total_entities": len(self.entities),
            "entity_counts": entity_counts,
            "total_relationships": len(self.relationships),
            "relationship_counts": relation_counts,
            "causal_relationships": sum(
                1 for r in self.relationships.values() if r.causal
            )
        }
