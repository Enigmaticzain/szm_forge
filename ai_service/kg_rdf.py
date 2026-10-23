"""RDF/SPARQL helpers for knowledge graph (Group 13)."""
from __future__ import annotations

import logging
from typing import Any, Dict, List, Optional

logger = logging.getLogger("KGRDF")

_HAS_RDFLIB = False
try:
    from rdflib import Graph, Literal, Namespace, RDF, URIRef
    _HAS_RDFLIB = True
    EX = Namespace("http://szm.forge/engineering/")
except ImportError:
    Graph = None
    EX = None


def available() -> bool:
    return _HAS_RDFLIB


def ingest_document_triples(doc_id: str, title: str, entities: List[str]) -> Dict[str, Any]:
    if not _HAS_RDFLIB:
        return {"status": "error", "message": "rdflib not installed"}
    g = Graph()
    doc_uri = URIRef(EX[doc_id])
    g.add((doc_uri, RDF.type, EX.Document))
    g.add((doc_uri, EX.title, Literal(title)))
    for ent in entities:
        g.add((doc_uri, EX.mentions, Literal(ent)))
    return {"status": "success", "triples": len(g), "ttl": g.serialize(format="turtle")[:2000]}


def query_sparql(ttl: str, sparql: str) -> List[Dict[str, str]]:
    if not _HAS_RDFLIB:
        return []
    g = Graph()
    g.parse(data=ttl, format="turtle")
    results = []
    for row in g.query(sparql):
        results.append({str(k): str(v) for k, v in zip(row.labels, row)})
    return results
