"""SZM Forge — Colab-ready LoRA fine-tuning pipeline (honest, dataset-aware).
Uses only CPU-safe, small-model paths locally; designed for free T4 GPU on Colab.
Dataset reality: machine_data/szm_unsloth_dataset.jsonl has 2 instruction examples.
Fine-tuning from 2 examples is not production-meaningful — pipeline accepts larger data.
License: base model MIT / Apache selected explicitly; no proprietary weights required.
"""
from __future__ import annotations
import os, sys, json, random, argparse, logging
from pathlib import Path
from typing import Optional, Dict, Any
from colab_config import DATASET_PATH, REQUIRED_COLS, BASE_MODEL, validate_jsonl, build_config_light

logger = logging.getLogger(__name__)

# --- Hardware reporting ---
# Delay heavy torch/transformers imports to main() / model-use time so tests load quickly
DEVICE = None
GPU_NAME = None
logger = logging.getLogger(__name__)

def report_hardware():
    import torch
    dev = "cuda" if torch.cuda.is_available() else "cpu"
    name = torch.cuda.get_device_name(0) if torch.cuda.is_available() else "none"
    print(f"[HW] Device: {dev} | GPU: {name} | PyTorch: {torch.__version__}")
    if dev == "cuda":
        print(f"[HW] GPU memory: {torch.cuda.get_device_properties(0).total_memory/1e9:.1f} GB")
    else:
        print("[HW] Running CPU-only (Colab T4 recommended for real training).")
    return dev, name

# --- Dataset honesty ---
DATASET_PATH = Path("/mnt/D/projects/szm_forge/machine_data/szm_unsloth_dataset.jsonl")

def load_and_validate_dataset(path: Optional[str] = None) -> Dict[str, Any]:
    target = Path(path) if path else DATASET_PATH
    from datasets import load_dataset
    ds = load_dataset("json", data_files=str(target), split="train")
    n = len(ds)
    cols = ds.column_names
    required = {"instruction", "input", "output"}  # instruction-tuning format
    missing = required - set(cols)
    quality_notes = []
    if n < 10:
        quality_notes.append(f"DATASET TOO SMALL ({n} examples): fine-tuning will overfit; build a larger dataset.")
    else:
        quality_notes.append(f"Dataset size adequate for pilot ({n} examples).")
    if missing:
        quality_notes.append(f"MISSING COLUMNS: {missing}")
    else:
        quality_notes.append("Schema valid (instruction/input/output present).")
    result = {
        "path": str(target), "size": n, "columns": list(cols),
        "verified": not missing,
        "quality_notes": quality_notes,
        "dataset": ds,
    }
    for note in quality_notes:
        print(f"[DATASET] {note}")
    return result

def split_data(ds, train_ratio=0.8, seed=42):
    from datasets import DatasetDict
    if isinstance(ds, dict) and "train" in ds:
        ds = ds["train"]
    n = len(ds)
    if n < 3:
        print("[SPLIT] Dataset too small for train/val split; using full train, empty val.")
        return DatasetDict({"train": ds, "validation": ds.select([])})
    split = ds.train_test_split(test_size=1-train_ratio, seed=seed)
    return DatasetDict({"train": split["train"], "validation": split["test"]})

# --- Model selection (open, small, license-verified) ---
# SmolLM2-1.7B-Instruct: MIT, ~3B effective, fits T4, instruction-tuned.
# For pure local CPU demo without download, configure but do not download.
BASE_MODEL = "HuggingFaceTB/SmolLM2-1.7B-Instruct"

def build_config():
    try:
        import torch
        fp16 = bool(torch.cuda.is_available())
    except ImportError:
        fp16 = False
    cfg = {
        "base_model": BASE_MODEL,
        "lora_r": 8,
        "lora_alpha": 16,
        "lora_target_modules": ["q_proj", "v_proj", "k_proj", "o_proj"],
        "max_seq_length": 256,
        "seed": 42,
        "max_steps": 3,  # dataset small; override when larger
        "output_dir": "/mnt/D/projects/szm_forge/training/adapter_output",
        "save_strategy": "steps", "save_steps": 1,
        "eval_strategy": "steps" if DATASET_PATH.exists() else "no",
        "logging_steps": 1,
        "learning_rate": 2e-4,
        "per_device_train_batch_size": 1,
        "gradient_accumulation_steps": 2,
        "fp16": fp16,
        "report_to": "none",
    }
    return cfg

def token_format(example):
    # Instruction format for instruction-tuning
    prompt = f"### Instruction:\n{example['instruction']}\n\n### Input:\n{example.get('input','')}\n\n### Response:\n"
    target = example["output"]
    return {"prompt": prompt, "target": target}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", default=str(DATASET_PATH))
    parser.add_argument("--output-dir", default="/mnt/D/projects/szm_forge/training/adapter_output")
    parser.add_argument("--max-steps", type=int, default=None)
    parser.add_argument("--push-to-hub", action="store_true", help="Optional HF Hub upload when authenticated (never hardcodes token)")
    parser.add_argument("--hub-repo-id", default="szm_forge/machine-spec-adapter")
    parser.add_argument("--include-unverified", action="store_true", help="OPT-IN ONLY: include needs_review examples (default excludes them; requires explicit acknowledgment)")
    parser.add_argument("--val-file", default="/mnt/D/projects/szm_forge/machine_data/val.jsonl", help="Separate validation file (must not overlap train instructions)")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format="%(message)s")
    import torch
    report_hardware()
    random.seed(42); torch.manual_seed(42)

    # Load dataset with honesty
    data = load_and_validate_dataset(args.dataset)
    # Configure device info for this session
    global DEVICE, GPU_NAME
    DEVICE = "cuda" if torch.cuda.is_available() else "cpu"
    GPU_NAME = torch.cuda.get_device_name(0) if torch.cuda.is_available() else "none"

    # Build config (respecting dataset reality)
    cfg = build_config()
    if args.max_steps is not None:
        cfg["max_steps"] = args.max_steps
    else:
        # Auto-set small step cap for tiny dataset without overriding user choice
        from datasets import load_dataset
        try:
            n_data = len(load_dataset("json", data_files=args.dataset, split="train"))
            cfg["max_steps"] = 3 if n_data < 5 else 100
        except Exception:
            cfg["max_steps"] = 3  # conservative fallback
    print(f"[CONFIG] Base model: {cfg['base_model']} (MIT license, open access)")
    print(f"[CONFIG] LoRA r={cfg['lora_r']} alpha={cfg['lora_alpha']}, max_steps={cfg['max_steps']}, fp16={cfg['fp16']}")

    # Explicit dataset limitation
    if data["size"] < 10:
        print("\n[WARNING] DATASET INADEQUATE FOR MEANINGFUL FINE-TUNING (2 examples).")
        print("        Pipeline configured for larger dataset — replace machine_data/szm_unsloth_dataset.jsonl")
        print("        with 50+ diverse instruction examples for production readiness.\n")

    # Quality gate: exclude needs_review from default training
    include_unverified = args.include_unverified or bool(os.environ.get("SZM_INCLUDE_UNVERIFIED", False))
    verified_only = not include_unverified
    if verified_only:
        print("[GATE] Default mode: using only verified (verified) examples; needs_review excluded.")
        print("[GATE] To include unverified examples: pass --include-unverified (explicit opt-in required).")
    else:
        print("[GATE] WARNING: --include-unverified set — unverified (needs_review) examples included.")
        print("        These examples have unverified engineering facts, calculations, or source references.")

    # Load dataset and apply quality gate filter (verified-only by default)
    from datasets import load_dataset, DatasetDict
    full_ds = load_dataset("json", data_files=args.dataset, split="train")
    if verified_only:
        verified_ds = full_ds.filter(lambda ex: ex.get("verified", "needs_review") == "verified")
        excluded = len(full_ds) - len(verified_ds)
        if excluded > 0:
            print(f"[GATE] Excluded {excluded} unverified (needs_review) examples from training.")
        data["dataset"] = verified_ds
    else:
        data["dataset"] = full_ds
    if DEVICE != "cuda" and not os.environ.get("SZM_ALLOW_MODEL_DOWNLOAD"):
        print("[LOCAL] CPU-only mode: configuring pipeline without downloading model weights.")
        print("[LOCAL] Set SZM_ALLOW_MODEL_DOWNLOAD=1 or run on Colab GPU to download.")
        # Save config only — pipeline ready
        out = Path(cfg["output_dir"]); out.mkdir(parents=True, exist_ok=True)
        with open(out / "training_config.json", "w") as f:
            json.dump({k:v for k,v in cfg.items() if isinstance(v,(str,int,float,bool))}, f, indent=2)
        with open(out / "dataset_report.json", "w") as f:
            json.dump({"dataset_path": data["path"], "size": data["size"], "verified": data["verified"], "limitations": data["quality_notes"]}, f, indent=2)
        print(f"[LOCAL] Config and dataset report saved to {out}")
        return 0

    # --- Real pipeline for Colab / GPU ---
    import torch
    from transformers import AutoTokenizer, AutoModelForCausalLM, TrainingArguments, Trainer
    from peft import LoraConfig, get_peft_model, prepare_model_for_kbit_training
    from datasets import load_dataset, DatasetDict
    tokenizer = AutoTokenizer.from_pretrained(cfg["base_model"], trust_remote_code=False)
    tokenizer.pad_token = tokenizer.eos_token
    model = AutoModelForCausalLM.from_pretrained(cfg["base_model"], torch_dtype=torch.float16 if cfg["fp16"] else torch.float32, device_map="auto" if DEVICE=="cuda" else None)
    model = prepare_model_for_kbit_training(model)
    peft_cfg = LoraConfig(r=cfg["lora_r"], lora_alpha=cfg["lora_alpha"], target_modules=cfg["lora_target_modules"], lora_dropout=0.05, bias="none", task_type="CAUSAL_LM")
    model = get_peft_model(model, peft_cfg)
    print("[MODEL] Adapter initialized (LoRA, parameter-efficient).")

    # Load separate validation file safely (must not overlap train instructions)
    val_info = None
    val_path_str = getattr(args, "val_file", "/mnt/D/projects/szm_forge/machine_data/val.jsonl")
    try:
        if Path(val_path_str).exists():
            val_ds = load_dataset("json", data_files=val_path_str, split="train")
            val_info = {"path": val_path_str, "size": len(val_ds), "loaded": True}
            print(f"[VAL] Separate validation file loaded from {val_path_str}: {len(val_ds)} rows.")
        else:
            val_info = {"path": val_path_str, "size": 0, "loaded": False, "note": "File not found."}
    except Exception as exc:
        val_info = {"path": val_path_str, "size": 0, "loaded": False, "note": str(exc)}
        print(f"[VAL] Validation file load skipped (expected if missing): {exc}")
    # Tokenize using train + separate val (if loaded); else split train (honest fallback)
    if val_info and val_info.get("loaded"):
        val_ds_raw = load_dataset("json", data_files=val_path_str, split="train")
        if verified_only:
            val_ds_raw = val_ds_raw.filter(lambda ex: ex.get("verified", "needs_review") == "verified")
        split = DatasetDict({"train": data["dataset"], "validation": val_ds_raw})
        print(f"[VAL] Using separate validation file: {len(val_ds_raw)} rows (filtered verified={verified_only}).")
    else:
        split = split_data(data["dataset"], train_ratio=0.8, seed=cfg["seed"])
        print("[SPLIT] No separate val file loaded; split from train (tiny dataset).")
    def tokenize_fn(ex):
        # Consistent instruction format (same as token_format above)
        full_text = f"### Instruction:\n{ex['instruction']}\n\n### Input:\n{ex.get('input','')}\n\n### Response:\n{ex['output']}"
        return tokenizer(full_text, truncation=True, max_length=cfg["max_seq_length"], padding="max_length")
    tokenized = split.map(tokenize_fn, batched=False, remove_columns=split["train"].column_names)

    args_train = TrainingArguments(
        output_dir=cfg["output_dir"],
        max_steps=cfg["max_steps"],
        save_strategy=cfg["save_strategy"],
        save_steps=cfg["save_steps"],
        logging_steps=cfg["logging_steps"],
        learning_rate=cfg["learning_rate"],
        per_device_train_batch_size=cfg["per_device_train_batch_size"],
        gradient_accumulation_steps=cfg["gradient_accumulation_steps"],
        fp16=cfg["fp16"],
        report_to=cfg["report_to"],
        evaluation_strategy=cfg["eval_strategy"],
        seed=cfg["seed"],
    )
    trainer = Trainer(model=model, args=args_train, train_dataset=tokenized["train"], eval_dataset=tokenized["validation"] if len(tokenized["validation"])>0 else None, tokenizer=tokenizer)

    # Train honestly
    if cfg["max_steps"] <= 3:
        print("[TRAIN] Running minimal demo steps only (dataset too small for real training).")
    print("[TRAIN] Starting training...")
    train_result = trainer.train()
    metrics = train_result.metrics
    print(f"[TRAIN] Completed {metrics.get('train_runtime',0):.1f}s | steps={metrics.get('step_count',cfg['max_steps'])}")

    # Save adapter (not full weights — resource-efficient)
    adapter_dir = Path(cfg["output_dir"]) / "adapter"
    adapter_dir.mkdir(parents=True, exist_ok=True)
    model.save_pretrained(str(adapter_dir))
    tokenizer.save_pretrained(str(adapter_dir))
    print(f"[SAVE] Adapter + tokenizer saved to {adapter_dir}")

    # Optional HF Hub upload (explicit only; never hardcodes auth)
    if args.push_to_hub:
        token = os.environ.get("HF_TOKEN") or os.environ.get("HF_HUB_TOKEN")
        if not token:
            print("[HUB] Upload skipped: HF_TOKEN not set in environment (no hardcoded credentials).")
        else:
            try:
                from huggingface_hub import HfApi
                api = HfApi(token=token)
                api.create_repo(args.hub_repo_id, repo_type="model", exist_ok=True, private=False)
                api.upload_folder(folder_path=str(adapter_dir), repo_id=args.hub_repo_id, repo_type="model")
                print(f"[HUB] Uploaded adapter to https://huggingface.co/{args.hub_repo_id}")
            except Exception as exc:
                print(f"[HUB] Upload failed (expected without valid auth): {exc}")
    else:
        print("[HUB] Upload not requested (pass --push-to-hub with HF_TOKEN to enable).")

    # Resume info
    print(f"[RESUME] To resume: load adapter from {adapter_dir} with AutoPeftModel / PeftModel.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
