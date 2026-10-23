#!/usr/bin/env python3
"""Experimental CPU chat runner for the SZM AI LoRA adapter."""

import sys
from pathlib import Path

PROJECT = Path(__file__).resolve().parents[1]
ADAPTER = (
    Path.home()
    / "SZM_AI_recovery"
    / "SZM_AI_Experimental_Backup"
    / "extracted"
    / "final_adapter"
)
BASE_MODEL = "Qwen/Qwen2.5-1.5B-Instruct"

def main():
    if not ADAPTER.is_dir():
        sys.exit(f"Adapter directory not found: {ADAPTER}")

    try:
        import torch
        from transformers import AutoTokenizer, AutoModelForCausalLM
        from peft import PeftModel
    except ImportError as exc:
        sys.exit(
            f"Missing dependency: {exc}\n"
            "Install transformers, peft and torch in the project venv first."
        )

    print("Loading base model on CPU; first run may download several GB.")
    tokenizer = AutoTokenizer.from_pretrained(BASE_MODEL)
    base = AutoModelForCausalLM.from_pretrained(
        BASE_MODEL,
        torch_dtype=torch.float32,
        low_cpu_mem_usage=True,
    )
    model = PeftModel.from_pretrained(base, str(ADAPTER))
    model.eval()

    print("\nExperimental SZM AI chat. Type 'exit' to quit.")
    while True:
        question = input("\nYou: ").strip()
        if question.lower() in {"exit", "quit"}:
            break
        if not question:
            continue

        messages = [{"role": "user", "content": question}]
        prompt = tokenizer.apply_chat_template(
            messages, tokenize=False, add_generation_prompt=True
        )
        inputs = tokenizer(prompt, return_tensors="pt")
        with torch.inference_mode():
            output = model.generate(
                **inputs,
                max_new_tokens=180,
                do_sample=False,
                pad_token_id=tokenizer.eos_token_id,
            )
        answer_tokens = output[0][inputs["input_ids"].shape[1]:]
        print("\nSZM AI:", tokenizer.decode(answer_tokens, skip_special_tokens=True))

if __name__ == "__main__":
    main()
