#!/usr/bin/env python3
"""
Advanced Visualization Service
Uses Plotly for interactive visualizations of:
- FEA results (stress, displacement, temperature)
- Design space exploration
- Optimization Pareto fronts
- Real-time monitoring dashboards
"""

from typing import Dict, List, Optional
import plotly.graph_objects as go
import plotly.express as px
from plotly.subplots import make_subplots
import numpy as np
import json
import logging
from fastapi import FastAPI
from fastapi.responses import HTMLResponse

logger = logging.getLogger(__name__)


class VisualizationService:
    """Create interactive Plotly visualizations"""
    
    @staticmethod
    def create_stress_heatmap(nodal_stress: Dict, title: str = "Stress Distribution") -> str:
        """Create heatmap of stress distribution"""
        x = np.array(nodal_stress.get("x_coords", []))
        y = np.array(nodal_stress.get("y_coords", []))
        stress = np.array(nodal_stress.get("values", []))
        
        fig = go.Figure(data=go.Scatter(
            x=x, y=y,
            mode='markers',
            marker=dict(
                size=8,
                color=stress,
                colorscale='Viridis',
                showscale=True,
                colorbar=dict(title="Stress (MPa)")
            ),
            text=[f"Stress: {s:.2f} MPa" for s in stress],
            hoverinfo='text'
        ))
        
        fig.update_layout(
            title=title,
            xaxis_title="X Coordinate",
            yaxis_title="Y Coordinate",
            hovermode='closest'
        )
        
        return fig.to_html(include_plotlyjs='cdn')
    
    @staticmethod
    def create_displacement_vectors(displacements: Dict) -> str:
        """Create vector field of displacements"""
        x = np.array(displacements.get("x", []))
        y = np.array(displacements.get("y", []))
        u = np.array(displacements.get("u", []))
        v = np.array(displacements.get("v", []))
        
        fig = go.Figure(data=go.Scatter(
            x=x, y=y,
            mode='markers+text',
            marker=dict(size=10, color='blue'),
            text=[f"({u[i]:.3f}, {v[i]:.3f})" for i in range(len(x))],
            textposition="top center"
        ))
        
        # Add arrows for vector field
        for i in range(len(x)):
            fig.add_annotation(
                ax=x[i], ay=y[i],
                x=x[i] + u[i]*10, y=y[i] + v[i]*10,
                xref="x", yref="y",
                axref="x", ayref="y",
                arrowhead=2,
                arrowsize=1,
                arrowwidth=2,
                arrowcolor="red"
            )
        
        fig.update_layout(
            title="Displacement Vectors",
            xaxis_title="X",
            yaxis_title="Y"
        )
        
        return fig.to_html(include_plotlyjs='cdn')
    
    @staticmethod
    def create_pareto_front(objectives: Dict[str, List[float]]) -> str:
        """Create Pareto front visualization for multi-objective optimization"""
        fig = go.Figure()
        
        # Assume 2 or 3 objectives
        if len(objectives) == 2:
            keys = list(objectives.keys())
            fig.add_trace(go.Scatter(
                x=objectives[keys[0]],
                y=objectives[keys[1]],
                mode='markers',
                marker=dict(size=8, color='red'),
                name='Pareto Solutions'
            ))
            fig.update_layout(
                title="Pareto Front (2D)",
                xaxis_title=keys[0],
                yaxis_title=keys[1],
                hovermode='closest'
            )
        elif len(objectives) == 3:
            keys = list(objectives.keys())
            fig.add_trace(go.Scatter3d(
                x=objectives[keys[0]],
                y=objectives[keys[1]],
                z=objectives[keys[2]],
                mode='markers',
                marker=dict(size=5, color='red'),
                name='Pareto Solutions'
            ))
            fig.update_layout(
                title="Pareto Front (3D)",
                scene=dict(
                    xaxis_title=keys[0],
                    yaxis_title=keys[1],
                    zaxis_title=keys[2]
                )
            )
        
        return fig.to_html(include_plotlyjs='cdn')
    
    @staticmethod
    def create_convergence_plot(iterations: List[int], 
                               values: List[float], 
                               metric_name: str = "Objective") -> str:
        """Create convergence plot for optimization"""
        fig = go.Figure()
        
        fig.add_trace(go.Scatter(
            x=iterations,
            y=values,
            mode='lines+markers',
            name='Convergence'
        ))
        
        fig.update_layout(
            title=f"{metric_name} Convergence",
            xaxis_title="Iteration",
            yaxis_title=metric_name,
            hovermode='x unified'
        )
        
        return fig.to_html(include_plotlyjs='cdn')
    
    @staticmethod
    def create_thermal_map(temperature_field: Dict) -> str:
        """Create thermal distribution heatmap"""
        x = np.array(temperature_field.get("x", []))
        y = np.array(temperature_field.get("y", []))
        temp = np.array(temperature_field.get("temp", []))
        
        fig = go.Figure(data=go.Scatter(
            x=x, y=y,
            mode='markers',
            marker=dict(
                size=10,
                color=temp,
                colorscale='Hot',
                showscale=True,
                colorbar=dict(title="Temperature (°C)")
            ),
            text=[f"T: {t:.1f}°C" for t in temp],
            hoverinfo='text'
        ))
        
        fig.update_layout(
            title="Thermal Distribution",
            xaxis_title="X",
            yaxis_title="Y"
        )
        
        return fig.to_html(include_plotlyjs='cdn')
    
    @staticmethod
    def create_dashboard(fea_results: Dict) -> str:
        """Create comprehensive FEA results dashboard"""
        fig = make_subplots(
            rows=2, cols=2,
            subplot_titles=("Stress", "Displacement", "Temperature", "Safety Factor"),
            specs=[[{"type": "scatter"}, {"type": "scatter"}],
                   [{"type": "scatter"}, {"type": "scatter"}]]
        )
        
        # Add subplots (simplified)
        stress = fea_results.get("stress", [])
        disp = fea_results.get("displacement", [])
        temp = fea_results.get("temperature", [])
        safety = fea_results.get("safety_factor", [])
        
        fig.add_trace(
            go.Scatter(y=stress, name="Stress"),
            row=1, col=1
        )
        fig.add_trace(
            go.Scatter(y=disp, name="Displacement"),
            row=1, col=2
        )
        fig.add_trace(
            go.Scatter(y=temp, name="Temperature"),
            row=2, col=1
        )
        fig.add_trace(
            go.Scatter(y=safety, name="Safety Factor"),
            row=2, col=2
        )
        
        fig.update_layout(height=800, title_text="FEA Results Dashboard")
        return fig.to_html(include_plotlyjs='cdn')


# Create FastAPI routes
async def register_visualization_routes(app: FastAPI):
    """Register visualization API endpoints"""
    
    vis = VisualizationService()
    
    @app.post("/api/visualize/stress-heatmap")
    async def stress_heatmap(nodal_stress: Dict):
        """Create stress heatmap"""
        return HTMLResponse(content=vis.create_stress_heatmap(nodal_stress))
    
    @app.post("/api/visualize/displacement")
    async def displacement_vectors(displacements: Dict):
        """Create displacement vector field"""
        return HTMLResponse(content=vis.create_displacement_vectors(displacements))
    
    @app.post("/api/visualize/pareto-front")
    async def pareto_front(objectives: Dict):
        """Create Pareto front plot"""
        return HTMLResponse(content=vis.create_pareto_front(objectives))
    
    @app.post("/api/visualize/convergence")
    async def convergence(iterations: List[int], values: List[float], 
                         metric_name: str = "Objective"):
        """Create convergence plot"""
        return HTMLResponse(content=vis.create_convergence_plot(iterations, values, metric_name))
    
    @app.post("/api/visualize/thermal-map")
    async def thermal_map(temperature_field: Dict):
        """Create thermal distribution"""
        return HTMLResponse(content=vis.create_thermal_map(temperature_field))
    
    @app.post("/api/visualize/dashboard")
    async def dashboard(fea_results: Dict):
        """Create full FEA dashboard"""
        return HTMLResponse(content=vis.create_dashboard(fea_results))
