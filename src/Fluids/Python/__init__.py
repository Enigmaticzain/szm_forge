"""
SZM Forge - Fluid Statics and Dynamics Module
=============================================

A comprehensive physics library for engineers to simulate fluid mechanics.

Modules:
    - fluid_simulation: Core simulation engine with statics and dynamics
    - fluid_utils: Utilities for visualization, analysis, and calculations
    - examples: Engineering examples and use cases

Quick Start:
    >>> from fluid_simulation import FluidSimulator, Fluid
    >>> sim = FluidSimulator(Fluid.WATER)
    >>> 
    >>> # Calculate buoyancy
    >>> buoyancy = sim.get_buoyant_force(volume=1.0)
    >>> 
    >>> # Calculate drag
    >>> drag = sim.get_drag_force(velocity=5.0, reference_area=1.0)
    >>> 
    >>> # Analyze floating object
    >>> result = sim.analyze_floating_object(volume=2.0, mass=1500)
    >>> print(f"Floats: {result.is_floating}")
    >>> print(f"Submerged: {result.submerged_fraction:.1%}")
"""

__version__ = "1.0.0"
__author__ = "SZM Forge Team"

from .fluid_simulation import (
    FluidSimulator,
    Fluid,
    DragModel,
    FluidState,
    BuoyancyResult,
    DragResult,
    BernoulliResult,
    quick_buoyancy,
    quick_drag,
    quick_pressure_at_depth,
)

from .fluid_utils import (
    FlowVisualizer,
    VelocityField,
    ThermodynamicCalculator,
    GeometryHelpers,
    DataInterpolator,
    ResultsAnalyzer,
)

__all__ = [
    # Main Classes
    'FluidSimulator',
    'FluidState',
    
    # Enums
    'Fluid',
    'DragModel',
    
    # Result Types
    'BuoyancyResult',
    'DragResult',
    'BernoulliResult',
    'VelocityField',
    
    # Utility Classes
    'FlowVisualizer',
    'ThermodynamicCalculator',
    'GeometryHelpers',
    'DataInterpolator',
    'ResultsAnalyzer',
    
    # Quick Functions
    'quick_buoyancy',
    'quick_drag',
    'quick_pressure_at_depth',
]
