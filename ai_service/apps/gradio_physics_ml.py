"""Gradio demo for physics-ML surrogates (Group 14)."""
from __future__ import annotations

try:
    import gradio as gr
except ImportError:
    raise SystemExit("gradio required: pip install gradio")

from physics_ml_engine import get_physics_ml_engine


def predict(x: float, model_name: str = "demo"):
    eng = get_physics_ml_engine()
    eng.train_surrogate(model_name, "torch", epochs=5)
    return eng.predict(model_name, {"x": x})


with gr.Blocks(title="SZM Forge Physics ML") as demo:
    gr.Markdown("# SZM Forge Surrogate Predictor")
    x = gr.Slider(0, 1, value=0.5, label="Input x")
    out = gr.JSON(label="Prediction")
    btn = gr.Button("Predict")
    btn.click(fn=lambda v: predict(v), inputs=x, outputs=out)

if __name__ == "__main__":
    demo.launch(server_name="127.0.0.1", server_port=7860)
