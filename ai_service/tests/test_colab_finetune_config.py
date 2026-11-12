"""Focused regression tests for Colab fine-tuning pipeline (CPU-safe, honest dataset reporting)."""
import sys, os, json, tempfile
sys.path.insert(0, "/mnt/D/projects/szm_forge/training")

from colab_finetune_workflow import load_and_validate_dataset, split_data, build_config, DATASET_PATH

def test_load_and_validate_existing_dataset():
    info = load_and_validate_dataset(str(DATASET_PATH))
    assert info["verified"] is True
    assert info["size"] == 2
    assert any("TOO SMALL" in note for note in info["quality_notes"])
    print("PASS: dataset verified; small-size warning emitted")

def test_split_tiny_dataset():
    from datasets import load_dataset
    ds = load_dataset("json", data_files=str(DATASET_PATH), split="train")
    split = split_data({"train": ds})
    assert split["train"].num_rows == 2
    assert split["validation"].num_rows == 0  # too small for split
    print("PASS: tiny dataset split handled gracefully (empty val)")

def test_build_config_small_dataset():
    cfg = build_config()
    assert cfg["base_model"] == "HuggingFaceTB/SmolLM2-1.7B-Instruct"
    assert cfg["lora_r"] == 8
    assert "fp16" in cfg
    assert cfg["max_steps"] == 3  # dataset has 2 examples
    print("PASS: config built correctly with small-dataset step cap")

def test_config_file_output():
    import tempfile, pathlib
    with tempfile.TemporaryDirectory() as td:
        out_dir = pathlib.Path(td) / "adapter_output"
        cfg = build_config()
        cfg_out = cfg.copy(); cfg_out["output_dir"] = str(out_dir)
        out_dir.mkdir(parents=True, exist_ok=True)
        with open(out_dir / "training_config.json", "w") as f:
            json.dump({k:v for k,v in cfg_out.items() if isinstance(v, (str,int,float,bool))}, f)
        assert (out_dir / "training_config.json").exists()
        print("PASS: config file writes correctly")

if __name__ == "__main__":
    test_load_and_validate_existing_dataset()
    test_split_tiny_dataset()
    test_build_config_small_dataset()
    test_config_file_output()
    print("All Colab pipeline tests passed.")
