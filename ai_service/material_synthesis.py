from __future__ import annotations

import math
import re
from dataclasses import dataclass
from typing import Any, Dict, List, Tuple


ELEMENT_PROPERTY_TABLE: Dict[str, Dict[str, float]] = {
    # density kg/m^3, elastic modulus GPa, thermal conductivity W/(m*K), CTE 1/K, melt C
    "Al": {"density": 2700, "youngs": 69, "thermal": 237, "cte": 23.1e-6, "melt": 660},
    "Ti": {"density": 4507, "youngs": 116, "thermal": 21.9, "cte": 8.6e-6, "melt": 1668},
    "Fe": {"density": 7874, "youngs": 211, "thermal": 80, "cte": 11.8e-6, "melt": 1538},
    "Ni": {"density": 8908, "youngs": 200, "thermal": 90.9, "cte": 13.4e-6, "melt": 1455},
    "Cr": {"density": 7190, "youngs": 279, "thermal": 93.9, "cte": 4.9e-6, "melt": 1907},
    "Cu": {"density": 8960, "youngs": 117, "thermal": 401, "cte": 16.5e-6, "melt": 1085},
    "C": {"density": 2260, "youngs": 70, "thermal": 140, "cte": 2.0e-6, "melt": 3550},
    "Si": {"density": 2330, "youngs": 130, "thermal": 149, "cte": 2.6e-6, "melt": 1414},
    "Mg": {"density": 1738, "youngs": 45, "thermal": 156, "cte": 26e-6, "melt": 650},
    "Mn": {"density": 7470, "youngs": 198, "thermal": 7.8, "cte": 21.7e-6, "melt": 1246},
    "Mo": {"density": 10280, "youngs": 329, "thermal": 138, "cte": 4.8e-6, "melt": 2623},
    "W": {"density": 19250, "youngs": 411, "thermal": 174, "cte": 4.5e-6, "melt": 3422},
    "V": {"density": 6110, "youngs": 128, "thermal": 30.7, "cte": 8.4e-6, "melt": 1910},
    "B": {"density": 2340, "youngs": 400, "thermal": 27, "cte": 6e-6, "melt": 2076},
    "O": {"density": 1140, "youngs": 3, "thermal": 0.026, "cte": 50e-6, "melt": -219},
    "H": {"density": 0.09, "youngs": 0.001, "thermal": 0.18, "cte": 200e-6, "melt": -259},
}


@dataclass
class Constituent:
    symbol: str
    fraction: float


def _slugify(value: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")
    return slug or "candidate"


def _positive(value: Any, fallback: float) -> float:
    try:
        parsed = float(value)
    except (TypeError, ValueError):
        return fallback
    return parsed if math.isfinite(parsed) and parsed > 0 else fallback


def _normalize_constituents(raw_constituents: List[Dict[str, Any]]) -> List[Constituent]:
    constituents: List[Constituent] = []
    for item in raw_constituents:
        symbol = str(item.get("symbol") or item.get("element") or item.get("name") or "").strip()
        if not symbol:
            continue
        symbol = symbol[0].upper() + symbol[1:].lower()
        if symbol not in ELEMENT_PROPERTY_TABLE:
            continue
        fraction = _positive(item.get("fraction", item.get("percent", 0.0)), 0.0)
        constituents.append(Constituent(symbol=symbol, fraction=fraction))

    if not constituents:
        constituents = [
            Constituent("Fe", 0.70),
            Constituent("C", 0.02),
            Constituent("Cr", 0.18),
            Constituent("Ni", 0.10),
        ]

    total = sum(item.fraction for item in constituents)
    if total <= 0:
        equal = 1.0 / len(constituents)
        return [Constituent(item.symbol, equal) for item in constituents]

    # Accept either fractions summing to 1 or percentages summing to 100.
    return [Constituent(item.symbol, item.fraction / total) for item in constituents]


def _weighted(constituents: List[Constituent], property_name: str) -> float:
    return sum(ELEMENT_PROPERTY_TABLE[item.symbol][property_name] * item.fraction for item in constituents)


def _family_factor(family: str) -> Tuple[float, float, float]:
    normalized = family.lower()
    if "composite" in normalized:
        return 1.18, 1.22, 0.72
    if "ceramic" in normalized:
        return 1.45, 0.85, 0.55
    if "polymer" in normalized:
        return 0.08, 0.12, 0.35
    if "alloy" in normalized or "metal" in normalized:
        return 1.0, 1.0, 1.0
    return 0.85, 0.9, 0.8


def _score_process(process: str, density: float, youngs: float, thermal: float, melt: float) -> float:
    normalized = process.lower()
    score = 0.65
    if "cnc" in normalized or "machin" in normalized:
        score += 0.20 if youngs < 220 else 0.06
        score += 0.08 if thermal > 20 else -0.04
        score += 0.04 if density < 9000 else -0.03
    elif "additive" in normalized or "print" in normalized:
        score += 0.15 if melt < 1700 else 0.03
        score += 0.08 if density < 6000 else -0.03
    elif "cast" in normalized:
        score += 0.18 if melt < 1600 else 0.04
        score += 0.06 if thermal > 40 else 0.0
    elif "injection" in normalized:
        score += 0.22 if youngs < 20 else -0.10
    return max(0.0, min(score, 0.98))


class MaterialSynthesisEngine:
    """Rule-based material synthesis bridge between Chemistry Lab and manufacturing."""

    optional_dependency_notes = {
        "rdkit": "Optional molecular featurization for polymers and organic additives.",
        "pymatgen": "Optional crystal/composition analysis for alloys and ceramics.",
        "matminer": "Optional featurizers and dataset-backed property models.",
    }

    @staticmethod
    def synthesize(request: Dict[str, Any]) -> Dict[str, Any]:
        name = str(request.get("name") or "Synthesized Material").strip()
        family = str(request.get("family") or "alloy").strip()
        process = str(request.get("process") or request.get("manufacturing_process") or "CNC machining").strip()
        application = str(request.get("target_application") or "general manufacturing").strip()
        constituents = _normalize_constituents(request.get("constituents") or [])

        e_factor, strength_factor, ductility_factor = _family_factor(family)
        density = _weighted(constituents, "density")
        youngs = max(0.5, _weighted(constituents, "youngs") * e_factor)
        thermal = max(0.01, _weighted(constituents, "thermal"))
        cte = max(0.1e-6, _weighted(constituents, "cte"))
        melt = _weighted(constituents, "melt")

        alloying_bonus = min(1.35, 1.0 + max(0, len(constituents) - 1) * 0.045)
        carbon_fraction = next((item.fraction for item in constituents if item.symbol == "C"), 0.0)
        refractory_fraction = sum(item.fraction for item in constituents if item.symbol in {"W", "Mo", "V", "Ti"})
        base_strength = 0.42 * youngs * 1000.0 / 2.8
        yield_strength = base_strength * strength_factor * alloying_bonus
        yield_strength *= 1.0 + min(0.30, carbon_fraction * 2.0) + min(0.22, refractory_fraction * 0.35)
        ultimate_strength = yield_strength * (1.22 + ductility_factor * 0.18)
        poissons = 0.31
        if "polymer" in family.lower():
            poissons = 0.38
        elif "ceramic" in family.lower():
            poissons = 0.23

        process_score = _score_process(process, density, youngs, thermal, melt)
        performance_score = max(0.0, min((yield_strength / 900.0) * 0.45 + (youngs / 220.0) * 0.25 + process_score * 0.30, 0.99))
        viability = process_score >= 0.62 and performance_score >= 0.42

        recommendations = []
        if thermal < 15 and ("cnc" in process.lower() or "machin" in process.lower()):
            recommendations.append("Use flood coolant and conservative feeds; low conductivity may localize heat.")
        if density > 8500:
            recommendations.append("Review mass budget; candidate is heavy for mobile assemblies.")
        if yield_strength < 250:
            recommendations.append("Increase strengthening phase or choose a higher-strength base system.")
        if melt > 1700 and ("cast" in process.lower() or "print" in process.lower()):
            recommendations.append("High melting point raises process energy and equipment requirements.")
        if not recommendations:
            recommendations.append("Candidate is viable for first-pass coupon testing and DFM review.")

        composition = [
            {"symbol": item.symbol, "fraction": round(item.fraction, 4), "percent": round(item.fraction * 100, 2)}
            for item in constituents
        ]
        material_id = f"MAT-SYNTH-{_slugify(name).upper()}"
        material_record = {
            "id": material_id,
            "name": name,
            "family": f"Synthesized {family.title()}",
            "density": round(density, 2),
            "youngs_modulus": round(youngs, 3),
            "poissons_ratio": round(poissons, 3),
            "yield_strength": round(yield_strength, 2),
            "ultimate_strength": round(ultimate_strength, 2),
            "thermal_conductivity": round(thermal, 3),
            "thermal_expansion": cte,
            "tags": ["Synthesized", family.title(), process],
            "composition": composition,
        }

        return {
            "status": "success",
            "engine_used": "SZM deterministic material synthesis",
            "target_application": application,
            "manufacturing_process": process,
            "viable": viability,
            "scores": {
                "manufacturing": round(process_score, 3),
                "performance": round(performance_score, 3),
                "confidence": round(min(0.92, 0.62 + 0.04 * len(constituents)), 3),
            },
            "material": material_record,
            "recommendations": recommendations,
            "optional_dependency_notes": MaterialSynthesisEngine.optional_dependency_notes,
        }
