"""
Seed the in-memory KnowledgeGraph from data/knowledge/*.json (shared with C++ backend).
"""

from __future__ import annotations

import json
import logging
from pathlib import Path
from typing import Any, Dict, Optional

from knowledge_graph import EntityType, KnowledgeGraph

logger = logging.getLogger(__name__)

DOMAIN_ENTITY_TYPE: Dict[str, EntityType] = {
    "materials": EntityType.MATERIAL,
    "equations": EntityType.EQUATION,
    "manufacturing": EntityType.PROCESS,
    "machine_elements": EntityType.MACHINE_ELEMENT,
    "machine_types": EntityType.SYSTEM,
    "robotics": EntityType.SYSTEM,
    "control_systems": EntityType.SYSTEM,
    "treatments": EntityType.TREATMENT,
    "standards": EntityType.STANDARD,
    "cad_cam": EntityType.SYSTEM,
}


def _resolve_kb_dir() -> Optional[Path]:
    candidates = [
        Path(__file__).resolve().parents[1] / "data" / "knowledge",
        Path.cwd() / "data" / "knowledge",
        Path.cwd().parent / "data" / "knowledge",
    ]
    for path in candidates:
        if path.is_dir():
            return path
    return None


def seed_knowledge_graph(kg: KnowledgeGraph) -> int:
    """Ingest all JSON domain files into the knowledge graph. Returns entity count added."""
    kb_dir = _resolve_kb_dir()
    if kb_dir is None:
        logger.warning("data/knowledge not found — skipping KB seed")
        return 0

    index_path = kb_dir / "index.json"
    domain_files: Dict[str, Path] = {}

    if index_path.is_file():
        index = json.loads(index_path.read_text(encoding="utf-8"))
        for d in index.get("domains", []):
            domain_id = d.get("id")
            file_name = d.get("file")
            if domain_id and file_name:
                domain_files[domain_id] = kb_dir / file_name
    else:
        for json_file in kb_dir.glob("*.json"):
            if json_file.name == "index.json":
                continue
            domain_files[json_file.stem] = json_file

    added = 0
    for domain_id, path in domain_files.items():
        if not path.is_file():
            continue
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            logger.error("Failed to parse %s: %s", path, exc)
            continue

        entity_type = DOMAIN_ENTITY_TYPE.get(domain_id, EntityType.PROPERTY)
        entries = data.get("entries", [])
        if not isinstance(entries, list):
            continue

        for entry in entries:
            if not isinstance(entry, dict) or "id" not in entry:
                continue
            name = entry.get("name", entry["id"])
            description = entry.get("notes") or entry.get("expression") or entry.get("category") or domain_id
            props = {k: v for k, v in entry.items() if k not in ("id", "name")}
            kg.add_entity(
                name=str(name),
                entity_type=entity_type,
                description=str(description)[:500],
                properties=props,
                source=f"data/knowledge/{path.name}",
                entity_id=f"kb_{domain_id}_{entry['id']}",
                metadata={"domain": domain_id, "kb_id": entry["id"]},
            )
            added += 1

    logger.info("Seeded %d KB entities from %s", added, kb_dir)
    return added
