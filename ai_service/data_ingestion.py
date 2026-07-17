#!/usr/bin/env python3
"""
Data Ingestion & Corpus Management for SZM Forge
Handles crawling and ingesting engineering knowledge from:
- Patents (USPTO, WIPO, EPO)
- Technical papers (arXiv, IEEE, ASME)
- Standards (ISO, ASTM, ASME)
- Vendor datasheets and specs
- NASA/ESA technical reports
- Failure/incident databases
"""

import logging
import json
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, asdict, field
from enum import Enum
from datetime import datetime
import hashlib

logger = logging.getLogger(__name__)


class SourceType(Enum):
    """Source types for knowledge"""
    PATENT = "patent"
    TECHNICAL_PAPER = "paper"
    STANDARD = "standard"
    DATASHEET = "datasheet"
    TECHNICAL_REPORT = "report"
    FAILURE_ANALYSIS = "failure_analysis"
    VENDOR_SPEC = "vendor_spec"
    REGULATORY = "regulatory"
    TEXTBOOK = "textbook"


class SourceStatus(Enum):
    """Status of knowledge source"""
    PENDING = "pending"
    CRAWLING = "crawling"
    CRAWLED = "crawled"
    PROCESSING = "processing"
    PROCESSED = "processed"
    INDEXED = "indexed"
    FAILED = "failed"


@dataclass
class SourceDocument:
    """Single document from knowledge source"""
    id: str
    title: str
    source_type: SourceType
    url: Optional[str] = None
    authors: List[str] = field(default_factory=list)
    published_date: Optional[str] = None
    content: str = ""
    summary: str = ""
    
    # Metadata
    source_system: str = ""  # e.g., "USPTO", "arXiv", "IEEE"
    original_id: Optional[str] = None  # ID in source system
    license: str = ""  # License or terms of use
    confidence: float = 1.0
    
    # Processing
    crawled_date: Optional[str] = None
    processed_date: Optional[str] = None
    extracted_entities: List[str] = field(default_factory=list)
    extracted_relations: List[Dict[str, str]] = field(default_factory=list)
    
    # Quality metrics
    quality_score: float = 0.0  # 0-1
    relevance_score: float = 0.0  # 0-1 to design/engineering
    
    created_at: str = field(default_factory=lambda: datetime.now().isoformat())
    
    def to_dict(self) -> Dict:
        d = asdict(self)
        d['source_type'] = self.source_type.value
        return d

    def compute_hash(self) -> str:
        """Compute document hash for deduplication"""
        content_hash = hashlib.sha256(self.content.encode()).hexdigest()
        return content_hash


@dataclass
class Corpus:
    """Collection of documents from various sources"""
    name: str
    documents: Dict[str, SourceDocument] = field(default_factory=dict)
    metadata: Dict[str, Any] = field(default_factory=dict)
    statistics: Dict[str, int] = field(default_factory=dict)
    created_at: str = field(default_factory=lambda: datetime.now().isoformat())

    def add_document(self, doc: SourceDocument):
        """Add document to corpus"""
        self.documents[doc.id] = doc
        self._update_statistics()
        logger.info(f"Added document: {doc.title} ({doc.source_type.value})")

    def _update_statistics(self):
        """Update corpus statistics"""
        self.statistics["total_documents"] = len(self.documents)
        by_type = {}
        for doc in self.documents.values():
            doc_type = doc.source_type.value
            by_type[doc_type] = by_type.get(doc_type, 0) + 1
        self.statistics["by_source_type"] = by_type

    def get_documents_by_type(self, source_type: SourceType) -> List[SourceDocument]:
        """Get all documents of a specific type"""
        return [doc for doc in self.documents.values() if doc.source_type == source_type]

    def export_to_json(self, filepath: str):
        """Export corpus to JSON"""
        export_data = {
            "name": self.name,
            "metadata": self.metadata,
            "statistics": self.statistics,
            "documents": [doc.to_dict() for doc in self.documents.values()]
        }
        with open(filepath, 'w') as f:
            json.dump(export_data, f, indent=2)
        logger.info(f"Exported corpus to {filepath}")


class DataIngestionPipeline:
    """
    Manages data ingestion from multiple sources.
    Orchestrates crawling, extraction, and knowledge graph population.
    """

    def __init__(self, knowledge_graph=None):
        self.kg = knowledge_graph
        self.corpus = Corpus(name="SZM_Engineering_Corpus")
        self.processors: Dict[SourceType, Any] = {}
        logger.info("Initialized Data Ingestion Pipeline")

    def register_processor(self, source_type: SourceType, processor):
        """Register a processor for a source type"""
        self.processors[source_type] = processor
        logger.info(f"Registered processor for {source_type.value}")

    def ingest_patent(
        self,
        title: str,
        url: str,
        patent_id: str,
        abstract: str,
        content: str,
        inventors: List[str],
        publication_date: str,
        claims: List[str]
    ) -> SourceDocument:
        """Ingest a patent document"""
        doc = SourceDocument(
            id=f"patent_{patent_id}",
            title=title,
            source_type=SourceType.PATENT,
            url=url,
            authors=inventors,
            published_date=publication_date,
            content=content,
            summary=abstract,
            source_system="USPTO/WIPO",
            original_id=patent_id,
            license="public_domain"
        )
        self.corpus.add_document(doc)
        return doc

    def ingest_paper(
        self,
        title: str,
        url: str,
        arxiv_id: Optional[str] = None,
        doi: Optional[str] = None,
        authors: List[str] = None,
        abstract: str = "",
        content: str = "",
        publication_date: Optional[str] = None,
        journal: str = ""
    ) -> SourceDocument:
        """Ingest academic paper"""
        doc_id = arxiv_id or doi or f"paper_{len(self.corpus.documents)}"
        doc = SourceDocument(
            id=f"paper_{doc_id}",
            title=title,
            source_type=SourceType.TECHNICAL_PAPER,
            url=url,
            authors=authors or [],
            published_date=publication_date,
            content=content,
            summary=abstract,
            source_system="arXiv/IEEE/ASME",
            original_id=doc_id,
            metadata={"journal": journal}
        )
        self.corpus.add_document(doc)
        return doc

    def ingest_standard(
        self,
        title: str,
        standard_id: str,
        content: str,
        organization: str,  # ISO, ASTM, IEEE, ASME
        scope: str = "",
        requirements: List[str] = None
    ) -> SourceDocument:
        """Ingest engineering standard"""
        doc = SourceDocument(
            id=f"standard_{standard_id}",
            title=title,
            source_type=SourceType.STANDARD,
            content=content,
            source_system=organization,
            original_id=standard_id,
            summary=scope,
            extracted_entities=requirements or []
        )
        self.corpus.add_document(doc)
        return doc

    def ingest_datasheet(
        self,
        component_name: str,
        manufacturer: str,
        content: str,
        properties: Dict[str, Any],
        specifications: Dict[str, Any],
        url: Optional[str] = None
    ) -> SourceDocument:
        """Ingest component datasheet"""
        doc = SourceDocument(
            id=f"datasheet_{manufacturer}_{component_name}",
            title=f"{component_name} Datasheet ({manufacturer})",
            source_type=SourceType.DATASHEET,
            url=url,
            content=content,
            source_system=manufacturer,
            metadata={
                "component": component_name,
                "manufacturer": manufacturer,
                "properties": properties,
                "specifications": specifications
            }
        )
        self.corpus.add_document(doc)
        return doc

    def ingest_failure_analysis(
        self,
        incident_id: str,
        title: str,
        description: str,
        root_cause: str,
        component_failed: str,
        failure_mode: str,
        mitigation: str,
        source: str
    ) -> SourceDocument:
        """Ingest failure/incident analysis"""
        doc = SourceDocument(
            id=f"failure_{incident_id}",
            title=title,
            source_type=SourceType.FAILURE_ANALYSIS,
            content=description,
            source_system=source,
            original_id=incident_id,
            summary=f"Component: {component_failed}, Mode: {failure_mode}, Root: {root_cause}",
            extracted_entities=[component_failed, failure_mode, root_cause],
            metadata={
                "component_failed": component_failed,
                "failure_mode": failure_mode,
                "root_cause": root_cause,
                "mitigation": mitigation
            }
        )
        self.corpus.add_document(doc)
        return doc

    def extract_entities(self, document: SourceDocument) -> List[str]:
        """
        Extract engineering entities from document.
        In production, would use NLP/NER models.
        
        Entities include:
        - Components (bearing, turbine, etc.)
        - Materials (titanium, aluminum, steel)
        - Functions (transmit_force, dissipate_heat)
        - Failures (fatigue, corrosion, buckling)
        """
        entities = []

        # Simple keyword extraction (replace with ML model in production)
        keywords = {
            "components": ["bearing", "turbine", "pump", "compressor", "valve", "piston"],
            "materials": ["titanium", "aluminum", "steel", "carbon", "composite", "ceramic"],
            "failures": ["fatigue", "corrosion", "buckling", "creep", "fracture", "wear"],
            "functions": ["transmit", "support", "dissipate", "compress", "expand"]
        }

        content_lower = document.content.lower()
        for category, terms in keywords.items():
            for term in terms:
                if term in content_lower:
                    entities.append(term)

        document.extracted_entities = entities
        return entities

    def extract_relations(self, document: SourceDocument) -> List[Dict[str, str]]:
        """
        Extract engineering relationships from document.
        E.g., "material X is used in component Y"
        
        Returns list of dicts with {subject, relation, object}
        """
        relations = []

        # Simple pattern matching (replace with ML extraction in production)
        patterns = [
            ("material", "used_in", "component"),
            ("component", "performs", "function"),
            ("design", "prevents", "failure"),
            ("constraint", "applies_to", "system")
        ]

        # Placeholder implementation
        relations.append({
            "subject": "material",
            "relation": "type_of",
            "object": "resource"
        })

        document.extracted_relations = relations
        return relations

    def process_document(self, document: SourceDocument):
        """Run full processing pipeline on document"""
        logger.info(f"Processing document: {document.title}")

        # Extract entities
        self.extract_entities(document)

        # Extract relations
        self.extract_relations(document)

        # Score relevance to engineering design
        self.score_relevance(document)

        document.processed_date = datetime.now().isoformat()
        logger.info(f"Document processed: {len(document.extracted_entities)} entities found")

    def score_relevance(self, document: SourceDocument):
        """Score document relevance to engineering design"""
        relevance_keywords = [
            "design", "component", "material", "failure", "constraint",
            "optimization", "performance", "safety", "reliability"
        ]

        content_lower = document.content.lower()
        matches = sum(1 for keyword in relevance_keywords if keyword in content_lower)
        
        # Normalize to 0-1
        relevance = min(matches / len(relevance_keywords), 1.0)
        document.relevance_score = relevance

    def get_corpus_stats(self) -> Dict[str, Any]:
        """Get corpus statistics"""
        return {
            "total_documents": len(self.corpus.documents),
            "by_type": self.corpus.statistics.get("by_source_type", {}),
            "avg_relevance": np.mean([
                doc.relevance_score for doc in self.corpus.documents.values()
            ]) if self.corpus.documents else 0
        }

    def export_corpus(self, filepath: str):
        """Export ingested corpus"""
        self.corpus.export_to_json(filepath)
        logger.info(f"Corpus exported to {filepath}")


class PatentCrawler:
    """Crawler for patent databases"""

    def __init__(self, api_key: Optional[str] = None):
        self.api_key = api_key
        logger.info("Initialized Patent Crawler")

    def search_patents(self, query: str, limit: int = 10) -> List[Dict]:
        """
        Search for patents matching query.
        In production, would use Google Patents API or WIPO API.
        """
        logger.info(f"Searching patents for: {query}")
        # Placeholder - returns mock results
        return [
            {
                "id": f"patent_US123456",
                "title": "Optimized Bearing Design",
                "abstract": "A novel bearing design using composite materials",
                "inventors": ["John Doe"],
                "publication_date": "2024-01-15"
            }
        ]


class AcademicCrawler:
    """Crawler for academic papers"""

    def __init__(self, api_key: Optional[str] = None):
        self.api_key = api_key
        logger.info("Initialized Academic Paper Crawler")

    def search_papers(self, query: str, source: str = "arxiv", limit: int = 10) -> List[Dict]:
        """
        Search for papers on arXiv, IEEE, ASME.
        """
        logger.info(f"Searching {source} for: {query}")
        # Placeholder
        return [
            {
                "arxiv_id": "2401.12345",
                "title": "Machine Learning for Engineering Design",
                "authors": ["Alice Smith"],
                "abstract": "Applying ML to design optimization",
                "url": "https://arxiv.org/abs/2401.12345"
            }
        ]


import numpy as np
