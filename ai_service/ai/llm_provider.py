"""
Configurable LLM provider for the SZM Forge AI assistant.

Supports three backends:
  - "ollama"       : Ollama /api/chat endpoint (any model it serves)
  - "szm_adapter"  : Qwen2.5-1.5B-Instruct + SZM LoRA via transformers/PEFT
  - "none"         : No LLM — returns GraphRAG context verbatim (current behaviour)

Environment variables (all optional):
  SZM_LLM_PROVIDER   : "ollama" | "szm_adapter" | "none"  (default "none")
  SZM_OLLAMA_URL      : Ollama base URL                    (default "http://localhost:11434")
  SZM_OLLAMA_MODEL    : Ollama model name                  (default "llama2")
  SZM_ADAPTER_PATH    : Override adapter directory          (default ~/SZM_AI_recovery/…/final_adapter)
  SZM_ADAPTER_BASE    : HF base model id                   (default "Qwen/Qwen2.5-1.5B-Instruct")
"""

from __future__ import annotations

import logging
import os
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import Any, Dict, List, Optional
import requests

logger = logging.getLogger(__name__)

# ---------------------------------------------------------------------------
# Public types
# ---------------------------------------------------------------------------

class ProviderKind(str, Enum):
    NONE = "none"
    OLLAMA = "ollama"
    SZM_ADAPTER = "szm_adapter"


@dataclass
class LLMResult:
    """Outcome of a single LLM generation request."""
    text: str
    provider: str               # which provider actually answered
    model: str                  # model name / path used
    generated: bool             # True if a real LLM generated this
    error: Optional[str] = None # non-None if generation failed


@dataclass
class ProviderConfig:
    """All tuneable knobs, populated from env or passed explicitly."""
    kind: ProviderKind = ProviderKind.NONE
    ollama_url: str = "http://localhost:11434"
    ollama_model: str = "llama2"
    adapter_path: str = str(
        Path.home()
        / "SZM_AI_recovery"
        / "SZM_AI_Experimental_Backup"
        / "extracted"
        / "final_adapter"
    )
    adapter_base_model: str = "Qwen/Qwen2.5-1.5B-Instruct"
    max_new_tokens: int = 256
    temperature: float = 0.7

    @classmethod
    def from_env(cls) -> "ProviderConfig":
        """Build a config from environment variables, falling back to defaults."""
        raw_kind = os.environ.get("SZM_LLM_PROVIDER", "none").lower().strip()
        try:
            kind = ProviderKind(raw_kind)
        except ValueError:
            logger.warning(
                "Unknown SZM_LLM_PROVIDER=%r — falling back to 'none'", raw_kind
            )
            kind = ProviderKind.NONE
        return cls(
            kind=kind,
            ollama_url=os.environ.get("SZM_OLLAMA_URL", cls.ollama_url),
            ollama_model=os.environ.get("SZM_OLLAMA_MODEL", cls.ollama_model),
            adapter_path=os.environ.get("SZM_ADAPTER_PATH", cls.adapter_path),
            adapter_base_model=os.environ.get("SZM_ADAPTER_BASE", cls.adapter_base_model),
        )


# ---------------------------------------------------------------------------
# System prompt used by all LLM providers
# ---------------------------------------------------------------------------

_SYSTEM_PROMPT = (
    "You are the SZM Forge engineering assistant. "
    "Answer questions about stress analysis, materials, thermal behaviour, "
    "design optimisation, and manufacturing using the retrieved engineering "
    "context below.  Keep answers concise, precise, and actionable.  "
    "If the context does not contain enough information, say so honestly."
)


def _build_messages(
    query: str,
    graphrag_context: str,
    system_prompt: str = _SYSTEM_PROMPT,
) -> List[Dict[str, str]]:
    """Assemble the chat-completion messages list."""
    user_content = query
    if graphrag_context:
        user_content = (
            f"### Retrieved Engineering Context\n{graphrag_context}\n\n"
            f"### User Question\n{query}"
        )
    return [
        {"role": "system", "content": system_prompt},
        {"role": "user", "content": user_content},
    ]


# ---------------------------------------------------------------------------
# Provider implementations
# ---------------------------------------------------------------------------

def _generate_none(
    query: str,
    graphrag_context: str,
    config: ProviderConfig,
) -> LLMResult:
    """Pass-through: return the GraphRAG insight verbatim (pre-integration behaviour)."""
    return LLMResult(
        text="",  # empty signals caller to use its own formatting
        provider="none",
        model="n/a",
        generated=False,
    )


def _generate_ollama(
    query: str,
    graphrag_context: str,
    config: ProviderConfig,
) -> LLMResult:
    """Call a local Ollama instance."""
    messages = _build_messages(query, graphrag_context)
    url = f"{config.ollama_url.rstrip('/')}/api/chat"

    try:
        resp = requests.post(
            url,
            json={
                "model": config.ollama_model,
                "messages": messages,
                "stream": False,
                "options": {
                    "temperature": config.temperature,
                    "num_predict": config.max_new_tokens,
                },
            },
            timeout=120,
        )
        resp.raise_for_status()
        data = resp.json()
        content = data.get("message", {}).get("content", "")
        if not content:
            return LLMResult(
                text="",
                provider="ollama",
                model=config.ollama_model,
                generated=False,
                error="Ollama returned an empty response",
            )
        return LLMResult(
            text=content,
            provider="ollama",
            model=config.ollama_model,
            generated=True,
        )
    except Exception as exc:
        logger.warning("Ollama generation failed: %s", exc)
        return LLMResult(
            text="",
            provider="ollama",
            model=config.ollama_model,
            generated=False,
            error=str(exc),
        )


# Lazy-loaded transformer artefacts (expensive — loaded once, kept in module state)
_adapter_model = None
_adapter_tokenizer = None
_adapter_load_error: Optional[str] = None


def _ensure_adapter_loaded(config: ProviderConfig) -> Optional[str]:
    """Load the Qwen base + LoRA adapter once.  Returns an error string or None."""
    global _adapter_model, _adapter_tokenizer, _adapter_load_error

    if _adapter_model is not None:
        return None  # already loaded
    if _adapter_load_error is not None:
        return _adapter_load_error  # previously failed — don't retry

    adapter_dir = Path(config.adapter_path)
    if not adapter_dir.is_dir():
        _adapter_load_error = f"Adapter directory not found: {adapter_dir}"
        logger.error(_adapter_load_error)
        return _adapter_load_error

    try:
        import torch
        from transformers import AutoModelForCausalLM, AutoTokenizer
        from peft import PeftModel
    except ImportError as exc:
        _adapter_load_error = (
            f"Missing dependency for szm_adapter provider: {exc}. "
            "Install transformers, peft, and torch."
        )
        logger.error(_adapter_load_error)
        return _adapter_load_error

    try:
        logger.info("Loading base model %s …", config.adapter_base_model)
        _adapter_tokenizer = AutoTokenizer.from_pretrained(config.adapter_base_model)
        base = AutoModelForCausalLM.from_pretrained(
            config.adapter_base_model,
            torch_dtype=torch.float32,
            low_cpu_mem_usage=True,
        )
        _adapter_model = PeftModel.from_pretrained(base, str(adapter_dir))
        _adapter_model.eval()
        logger.info("SZM adapter loaded successfully.")
        return None
    except Exception as exc:
        _adapter_load_error = f"Failed to load SZM adapter: {exc}"
        logger.error(_adapter_load_error)
        return _adapter_load_error


def _generate_szm_adapter(
    query: str,
    graphrag_context: str,
    config: ProviderConfig,
) -> LLMResult:
    """Generate using the local Qwen2.5 + LoRA adapter via transformers/PEFT."""
    load_err = _ensure_adapter_loaded(config)
    if load_err:
        return LLMResult(
            text="",
            provider="szm_adapter",
            model=config.adapter_base_model,
            generated=False,
            error=load_err,
        )

    import torch  # safe — already imported by _ensure_adapter_loaded

    messages = _build_messages(query, graphrag_context)
    prompt = _adapter_tokenizer.apply_chat_template(
        messages, tokenize=False, add_generation_prompt=True
    )
    inputs = _adapter_tokenizer(prompt, return_tensors="pt")

    try:
        with torch.inference_mode():
            output = _adapter_model.generate(
                **inputs,
                max_new_tokens=config.max_new_tokens,
                do_sample=config.temperature > 0,
                temperature=config.temperature if config.temperature > 0 else None,
                pad_token_id=_adapter_tokenizer.eos_token_id,
            )
        answer_tokens = output[0][inputs["input_ids"].shape[1]:]
        text = _adapter_tokenizer.decode(answer_tokens, skip_special_tokens=True)
        return LLMResult(
            text=text.strip(),
            provider="szm_adapter",
            model=config.adapter_base_model,
            generated=bool(text.strip()),
            error=None if text.strip() else "Adapter returned empty output",
        )
    except Exception as exc:
        logger.warning("SZM adapter generation failed: %s", exc)
        return LLMResult(
            text="",
            provider="szm_adapter",
            model=config.adapter_base_model,
            generated=False,
            error=str(exc),
        )


# ---------------------------------------------------------------------------
# Dispatcher
# ---------------------------------------------------------------------------

_PROVIDERS = {
    ProviderKind.NONE: _generate_none,
    ProviderKind.OLLAMA: _generate_ollama,
    ProviderKind.SZM_ADAPTER: _generate_szm_adapter,
}


def generate(
    query: str,
    graphrag_context: str = "",
    config: Optional[ProviderConfig] = None,
) -> LLMResult:
    """
    Top-level entry point.  Dispatches to the configured provider.

    Parameters
    ----------
    query : str
        The user's question.
    graphrag_context : str
        Pre-formatted context string retrieved by GraphRAG.
    config : ProviderConfig | None
        If None, reads from environment variables.

    Returns
    -------
    LLMResult
        Always returns a result — never raises.  Check `.generated` and `.error`.
    """
    if config is None:
        config = ProviderConfig.from_env()

    handler = _PROVIDERS.get(config.kind, _generate_none)
    return handler(query, graphrag_context, config)


def provider_status(config: Optional[ProviderConfig] = None) -> Dict[str, Any]:
    """Return a JSON-friendly status dict describing the active provider."""
    if config is None:
        config = ProviderConfig.from_env()
    status: Dict[str, Any] = {
        "provider": config.kind.value,
        "ready": False,
        "detail": "",
    }

    if config.kind == ProviderKind.NONE:
        status["ready"] = True
        status["detail"] = "No LLM provider configured — GraphRAG context returned verbatim."

    elif config.kind == ProviderKind.OLLAMA:
        status["model"] = config.ollama_model
        status["ollama_url"] = config.ollama_url
        try:
            resp = requests.get(
                f"{config.ollama_url.rstrip('/')}/api/tags", timeout=3
            )
            if resp.status_code == 200:
                models = [m["name"] for m in resp.json().get("models", [])]
                status["ready"] = config.ollama_model in models or any(
                    m.startswith(config.ollama_model) for m in models
                )
                status["available_models"] = models
                if not status["ready"]:
                    status["detail"] = (
                        f"Ollama is reachable but model '{config.ollama_model}' "
                        f"not found. Available: {models}"
                    )
                else:
                    status["detail"] = f"Ollama ready with model '{config.ollama_model}'."
            else:
                status["detail"] = f"Ollama returned HTTP {resp.status_code}."
        except Exception as exc:
            status["detail"] = f"Cannot reach Ollama at {config.ollama_url}: {exc}"

    elif config.kind == ProviderKind.SZM_ADAPTER:
        status["model"] = config.adapter_base_model
        status["adapter_path"] = config.adapter_path
        adapter_dir = Path(config.adapter_path)
        if not adapter_dir.is_dir():
            status["detail"] = f"Adapter directory not found: {adapter_dir}"
        else:
            # Check deps without importing heavy libs
            missing = []
            for mod in ("torch", "transformers", "peft"):
                try:
                    __import__(mod)
                except ImportError:
                    missing.append(mod)
            if missing:
                status["detail"] = f"Missing Python packages: {missing}"
            elif _adapter_load_error:
                status["detail"] = f"Previous load failure: {_adapter_load_error}"
            elif _adapter_model is not None:
                status["ready"] = True
                status["detail"] = "SZM adapter loaded and ready."
            else:
                status["ready"] = True  # deps + files present; will load on first call
                status["detail"] = "SZM adapter available (will load on first query)."

    return status
