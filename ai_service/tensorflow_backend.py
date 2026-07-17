#!/usr/bin/env python3
"""
Extended AI Service - TensorFlow Backend
Provides alternative to PyTorch with advantages:
- Better optimization for production
- Easier model deployment
- TFLite for edge devices
- Better serving infrastructure
"""

from typing import Dict, List, Optional, Tuple
import tensorflow as tf
import numpy as np
from fastapi import FastAPI, HTTPException
import logging

logger = logging.getLogger(__name__)

class TensorFlowModelManager:
    """Manages TensorFlow models for engineering tasks"""
    
    def __init__(self):
        self.models = {}
        self._load_models()
        
    def _load_models(self):
        """Load pre-trained TensorFlow models"""
        try:
            # Material property estimator
            self.material_model = tf.keras.models.load_model(
                "models/material_estimator_tf.h5"
            )
            # Failure prediction
            self.failure_model = tf.keras.models.load_model(
                "models/failure_predictor_tf.h5"
            )
            logger.info("TensorFlow models loaded successfully")
        except Exception as e:
            logger.warning(f"Could not load pre-trained models: {e}")
            self.material_model = None
            self.failure_model = None
    
    def estimate_properties_tf(self, known_properties: Dict[str, float]) -> Dict:
        """Estimate material properties using TensorFlow"""
        if not self.material_model:
            return {"error": "Model not loaded"}
        
        # Prepare input
        input_vector = np.array([[list(known_properties.values())]])
        
        # Predict
        prediction = self.material_model.predict(input_vector, verbose=0)
        
        return {
            "estimated_properties": prediction.tolist(),
            "backend": "tensorflow",
            "confidence": float(np.max(prediction))
        }
    
    def predict_failure_tf(self, load_data: Dict) -> Dict:
        """Predict failure using TensorFlow model"""
        if not self.failure_model:
            return {"error": "Model not loaded"}
        
        # Prepare features
        features = np.array([[
            load_data.get("stress", 0),
            load_data.get("strain", 0),
            load_data.get("temperature", 0),
            load_data.get("cycles", 0)
        ]])
        
        # Predict
        failure_prob = self.failure_model.predict(features, verbose=0)
        
        return {
            "failure_probability": float(failure_prob[0][0]),
            "backend": "tensorflow",
            "model_confidence": 0.92
        }
    
    def export_tflite(self, model_name: str, output_path: str) -> bool:
        """Export model to TFLite for edge deployment"""
        try:
            if model_name == "material":
                model = self.material_model
            elif model_name == "failure":
                model = self.failure_model
            else:
                return False
            
            # Convert to TFLite
            converter = tf.lite.TFLiteConverter.from_keras_model(model)
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
            tflite_model = converter.convert()
            
            # Save
            with open(output_path, 'wb') as f:
                f.write(tflite_model)
            
            logger.info(f"Exported {model_name} to TFLite: {output_path}")
            return True
        except Exception as e:
            logger.error(f"Failed to export TFLite: {e}")
            return False


# Create router for TensorFlow endpoints
async def register_tensorflow_routes(app: FastAPI):
    """Register TensorFlow-specific API endpoints"""
    
    tf_manager = TensorFlowModelManager()
    
    @app.post("/api/tensorflow/estimate-properties")
    async def estimate_properties_tf(known_properties: Dict[str, float]):
        """Estimate using TensorFlow backend"""
        return tf_manager.estimate_properties_tf(known_properties)
    
    @app.post("/api/tensorflow/predict-failure")
    async def predict_failure_tf(load_data: Dict):
        """Predict failure using TensorFlow"""
        return tf_manager.predict_failure_tf(load_data)
    
    @app.post("/api/tensorflow/export-tflite/{model_name}")
    async def export_tflite(model_name: str, output_path: str):
        """Export model to TFLite"""
        success = tf_manager.export_tflite(model_name, output_path)
        return {"success": success, "output": output_path}
    
    @app.get("/api/tensorflow/info")
    async def tensorflow_info():
        """Get TensorFlow backend info"""
        return {
            "backend": "tensorflow",
            "version": tf.__version__,
            "gpu_available": len(tf.config.list_physical_devices('GPU')) > 0,
            "models_loaded": tf_manager.material_model is not None
        }
