"""
Lightweight tests for the LLM provider integration.

These tests do NOT download model weights, start servers, or require
Ollama / torch / transformers to be installed.  They verify:

1. Provider configuration from environment variables
2. The "none" provider pass-through behaviour
3. Graceful fallback when Ollama is unreachable
4. Graceful fallback when adapter dependencies are missing
5. GraphRAG context formatting
6. Provider status reporting
"""

from __future__ import annotations

import importlib
import os
import sys
from pathlib import Path
from unittest import mock

import pytest

# Ensure ai_service is importable
AI_SERVICE_DIR = str(Path(__file__).resolve().parents[1])
if AI_SERVICE_DIR not in sys.path:
    sys.path.insert(0, AI_SERVICE_DIR)

import llm_provider
from ai.llm_provider import (
    LLMResult,
    ProviderConfig,
    ProviderKind,
    _build_messages,
    _generate_none,
    generate,
    provider_status,
)


# ====================== Configuration tests ======================


class TestProviderConfig:
    """ProviderConfig.from_env reads env vars correctly."""

    def test_defaults(self):
        with mock.patch.dict(os.environ, {}, clear=True):
            cfg = ProviderConfig.from_env()
        assert cfg.kind == ProviderKind.NONE
        assert cfg.ollama_url == "http://localhost:11434"
        assert cfg.ollama_model == "llama2"

    def test_ollama_from_env(self):
        env = {
            "SZM_LLM_PROVIDER": "ollama",
            "SZM_OLLAMA_URL": "http://gpu-box:11434",
            "SZM_OLLAMA_MODEL": "mistral",
        }
        with mock.patch.dict(os.environ, env, clear=True):
            cfg = ProviderConfig.from_env()
        assert cfg.kind == ProviderKind.OLLAMA
        assert cfg.ollama_url == "http://gpu-box:11434"
        assert cfg.ollama_model == "mistral"

    def test_szm_adapter_from_env(self):
        env = {
            "SZM_LLM_PROVIDER": "szm_adapter",
            "SZM_ADAPTER_PATH": "/tmp/test_adapter",
        }
        with mock.patch.dict(os.environ, env, clear=True):
            cfg = ProviderConfig.from_env()
        assert cfg.kind == ProviderKind.SZM_ADAPTER
        assert cfg.adapter_path == "/tmp/test_adapter"

    def test_unknown_provider_falls_back_to_none(self):
        with mock.patch.dict(os.environ, {"SZM_LLM_PROVIDER": "gpt-5-turbo"}, clear=True):
            cfg = ProviderConfig.from_env()
        assert cfg.kind == ProviderKind.NONE

    def test_case_insensitive(self):
        with mock.patch.dict(os.environ, {"SZM_LLM_PROVIDER": "OLLAMA"}, clear=True):
            cfg = ProviderConfig.from_env()
        assert cfg.kind == ProviderKind.OLLAMA


# ====================== Message formatting tests ======================


class TestMessageBuilding:
    """_build_messages constructs the right chat-completion payload."""

    def test_without_context(self):
        msgs = _build_messages("What is stress?", "")
        assert len(msgs) == 2
        assert msgs[0]["role"] == "system"
        assert msgs[1]["role"] == "user"
        assert "What is stress?" in msgs[1]["content"]
        assert "Retrieved Engineering Context" not in msgs[1]["content"]

    def test_with_context(self):
        ctx = "Steel has yield strength 250 MPa."
        msgs = _build_messages("Is this safe?", ctx)
        assert "Retrieved Engineering Context" in msgs[1]["content"]
        assert "Steel has yield strength" in msgs[1]["content"]
        assert "Is this safe?" in msgs[1]["content"]


# ====================== None-provider tests ======================


class TestNoneProvider:
    """The 'none' provider returns an empty non-generated result."""

    def test_pass_through(self):
        cfg = ProviderConfig(kind=ProviderKind.NONE)
        result = generate("hello", "some context", config=cfg)
        assert isinstance(result, LLMResult)
        assert result.generated is False
        assert result.provider == "none"
        assert result.text == ""
        assert result.error is None

    def test_generate_dispatches_to_none_by_default(self):
        with mock.patch.dict(os.environ, {}, clear=True):
            result = generate("any question")
        assert result.provider == "none"
        assert result.generated is False


# ====================== Ollama fallback tests ======================


class TestOllamaFallback:
    """Ollama provider handles unreachable server gracefully."""

    def test_connection_refused(self):
        cfg = ProviderConfig(
            kind=ProviderKind.OLLAMA,
            ollama_url="http://localhost:99999",  # nothing listens here
            ollama_model="nonexistent",
        )
        result = generate("test query", config=cfg)
        assert result.generated is False
        assert result.provider == "ollama"
        assert result.error is not None
        assert result.text == ""

    def test_http_error_response(self):
        """Ollama returns a non-200 status."""
        cfg = ProviderConfig(
            kind=ProviderKind.OLLAMA,
            ollama_url="http://localhost:11434",
            ollama_model="test-model",
        )
        with mock.patch("llm_provider.requests") as mock_requests:
            mock_resp = mock.Mock()
            mock_resp.status_code = 500
            mock_resp.raise_for_status.side_effect = Exception("500 Server Error")
            mock_requests.post.return_value = mock_resp
            result = generate("test", config=cfg)
        assert result.generated is False
        assert result.error is not None

    def test_empty_ollama_response(self):
        """Ollama returns 200 but with no content."""
        cfg = ProviderConfig(
            kind=ProviderKind.OLLAMA,
            ollama_url="http://localhost:11434",
            ollama_model="test-model",
        )
        with mock.patch("llm_provider.requests") as mock_requests:
            mock_resp = mock.Mock()
            mock_resp.status_code = 200
            mock_resp.raise_for_status.return_value = None
            mock_resp.json.return_value = {"message": {"content": ""}}
            mock_requests.post.return_value = mock_resp
            result = generate("test", config=cfg)
        assert result.generated is False
        assert result.error == "Ollama returned an empty response"

    def test_successful_ollama_response(self):
        """Ollama returns a valid response."""
        cfg = ProviderConfig(
            kind=ProviderKind.OLLAMA,
            ollama_url="http://localhost:11434",
            ollama_model="test-model",
        )
        with mock.patch("llm_provider.requests") as mock_requests:
            mock_resp = mock.Mock()
            mock_resp.status_code = 200
            mock_resp.raise_for_status.return_value = None
            mock_resp.json.return_value = {
                "message": {"content": "The stress exceeds the yield strength."}
            }
            mock_requests.post.return_value = mock_resp
            result = generate("What about stress?", "context here", config=cfg)
        assert result.generated is True
        assert result.text == "The stress exceeds the yield strength."
        assert result.provider == "ollama"
        assert result.error is None


# ====================== SZM adapter fallback tests ======================


class TestSZMAdapterFallback:
    """SZM adapter provider handles missing deps / directory gracefully."""

    def setup_method(self):
        """Reset adapter module state between tests."""
        llm_provider._adapter_model = None
        llm_provider._adapter_tokenizer = None
        llm_provider._adapter_load_error = None

    def test_missing_adapter_directory(self, tmp_path):
        cfg = ProviderConfig(
            kind=ProviderKind.SZM_ADAPTER,
            adapter_path=str(tmp_path / "nonexistent"),
        )
        result = generate("test", config=cfg)
        assert result.generated is False
        assert result.provider == "szm_adapter"
        assert "not found" in result.error

    def test_missing_torch_dependency(self, tmp_path):
        """Adapter dir exists but torch is not installed."""
        adapter_dir = tmp_path / "adapter"
        adapter_dir.mkdir()
        (adapter_dir / "adapter_config.json").write_text("{}")

        cfg = ProviderConfig(
            kind=ProviderKind.SZM_ADAPTER,
            adapter_path=str(adapter_dir),
        )

        # Mock the import to fail for torch
        original_import = __builtins__.__import__ if hasattr(__builtins__, '__import__') else __import__

        def mock_import(name, *args, **kwargs):
            if name in ("torch", "transformers", "peft"):
                raise ImportError(f"No module named '{name}'")
            return original_import(name, *args, **kwargs)

        with mock.patch("builtins.__import__", side_effect=mock_import):
            result = generate("test", config=cfg)

        assert result.generated is False
        assert "Missing dependency" in (result.error or "")

    def test_load_error_cached(self, tmp_path):
        """Once loading fails, the error is cached and not retried."""
        cfg = ProviderConfig(
            kind=ProviderKind.SZM_ADAPTER,
            adapter_path=str(tmp_path / "nonexistent"),
        )
        r1 = generate("test", config=cfg)
        assert r1.generated is False

        # Second call should reuse cached error
        r2 = generate("test again", config=cfg)
        assert r2.generated is False
        assert r2.error == r1.error


# ====================== Provider status tests ======================


class TestProviderStatus:
    """provider_status() returns accurate diagnostics."""

    def setup_method(self):
        llm_provider._adapter_model = None
        llm_provider._adapter_tokenizer = None
        llm_provider._adapter_load_error = None

    def test_none_provider_always_ready(self):
        cfg = ProviderConfig(kind=ProviderKind.NONE)
        status = provider_status(cfg)
        assert status["ready"] is True
        assert status["provider"] == "none"

    def test_ollama_unreachable(self):
        cfg = ProviderConfig(
            kind=ProviderKind.OLLAMA,
            ollama_url="http://localhost:99999",
        )
        status = provider_status(cfg)
        assert status["ready"] is False
        assert "Cannot reach" in status["detail"]

    def test_adapter_missing_directory(self, tmp_path):
        cfg = ProviderConfig(
            kind=ProviderKind.SZM_ADAPTER,
            adapter_path=str(tmp_path / "nope"),
        )
        status = provider_status(cfg)
        assert status["ready"] is False
        assert "not found" in status["detail"]


# ====================== GraphRAG integration test ======================


class TestGraphRAGIntegration:
    """Verify GraphRAG context flows into the LLM provider correctly."""

    def test_context_passed_to_ollama(self):
        """When Ollama is configured, GraphRAG context appears in the prompt."""
        cfg = ProviderConfig(
            kind=ProviderKind.OLLAMA,
            ollama_url="http://localhost:11434",
            ollama_model="test",
        )
        captured_body = {}

        with mock.patch("llm_provider.requests") as mock_requests:
            def capture_post(url, json=None, timeout=None):
                captured_body.update(json or {})
                resp = mock.Mock()
                resp.status_code = 200
                resp.raise_for_status.return_value = None
                resp.json.return_value = {"message": {"content": "Synthesised answer"}}
                return resp

            mock_requests.post.side_effect = capture_post

            result = generate(
                "What about the bracket stress?",
                "Found critical joint exceeding 900 MPa.",
                config=cfg,
            )

        assert result.generated is True
        # Verify the GraphRAG context was embedded in the user message
        messages = captured_body.get("messages", [])
        user_msg = next((m for m in messages if m["role"] == "user"), None)
        assert user_msg is not None
        assert "900 MPa" in user_msg["content"]
        assert "bracket stress" in user_msg["content"]


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
