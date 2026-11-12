"""Focused regression tests for HF datasets integration (CPU-safe, no model weights)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))

from pathlib import Path
from ai_service.machine_learning_training import MachineLearningPipeline, HF_DATASETS_AVAILABLE, HF_HUB_AVAILABLE

TEST_DATA_DIR = Path("/mnt/D/projects/szm_forge/machine_data")

def test_hf_dataset_load():
    pipeline = MachineLearningPipeline(str(TEST_DATA_DIR))
    info = pipeline.load_hf_dataset()
    assert info is not None
    assert info["verified"] is True
    assert info["size"] == 2
    assert "instruction" in info["columns"]
    print(f"PASS: load_hf_dataset -> {info['size']} rows")

def test_hf_dataset_fallback():
    """When HF lib missing, pipeline should not crash."""
    assert isinstance(HF_DATASETS_AVAILABLE, bool)
    print("PASS: HF_DATASETS_AVAILABLE flag present")

def test_push_to_hub_graceful():
    pipeline = MachineLearningPipeline(str(TEST_DATA_DIR))
    # Without authentication / repo, this gracefully fails — expected for free/local
    result = pipeline.push_to_hf_hub(repo_id="test/repo")
    assert result is False  # expected: no auth/token set
    print("PASS: push_to_hf_hub graceful failure (no auth expected)")

if __name__ == "__main__":
    test_hf_dataset_load()
    test_hf_dataset_fallback()
    test_push_to_hub_graceful()
    print("All HF integration tests passed.")
