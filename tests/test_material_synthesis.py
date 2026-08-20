import sys
import unittest
from pathlib import Path


sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ai_service"))

from material_synthesis import MaterialSynthesisEngine


class MaterialSynthesisTests(unittest.TestCase):
    def test_synthesizes_viable_alloy_material_record(self):
        result = MaterialSynthesisEngine.synthesize({
            "name": "Synth Alpha Alloy",
            "family": "alloy",
            "target_application": "lightweight bracket",
            "manufacturing_process": "CNC machining",
            "constituents": [
                {"symbol": "Al", "fraction": 0.86},
                {"symbol": "Mg", "fraction": 0.08},
                {"symbol": "Si", "fraction": 0.06},
            ],
        })

        self.assertEqual(result["status"], "success")
        self.assertTrue(result["viable"])
        self.assertEqual(result["material"]["id"], "MAT-SYNTH-SYNTH-ALPHA-ALLOY")
        self.assertGreater(result["material"]["density"], 0)
        self.assertGreater(result["material"]["yield_strength"], 0)
        self.assertGreater(result["scores"]["confidence"], 0.6)
        self.assertEqual(
            sum(item["percent"] for item in result["material"]["composition"]),
            100.0,
        )


if __name__ == "__main__":
    unittest.main()
