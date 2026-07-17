"""
Fluid Statics and Dynamics Simulation Module
============================================

A comprehensive physics library for engineers to simulate, create, and innovate with
fluid mechanics. Supports both statics (pressure, buoyancy) and dynamics (flow, drag).

Usage Examples:
    >>> from fluid_simulation import FluidSimulator, Fluid
    >>> 
    >>> # Create a water simulation
    >>> sim = FluidSimulator(Fluid.WATER)
    >>> 
    >>> # Calculate buoyant force on a sphere
    >>> sphere_volume = 1.0  # m³
    >>> buoyancy = sim.get_buoyant_force(sphere_volume)
    >>> 
    >>> # Calculate drag on moving object
    >>> velocity = 10.0  # m/s
    >>> drag = sim.get_drag_force(velocity, reference_area=1.0)
"""

import numpy as np
from enum import Enum
from dataclasses import dataclass
from typing import Tuple, List, Optional

class Fluid(Enum):
    """Common fluid types with predefined properties."""
    WATER = "water"
    AIR = "air"
    OIL = "oil"
    MERCURY = "mercury"
    HONEY = "honey"
    CUSTOM = "custom"

class DragModel(Enum):
    """Drag calculation models for different object shapes."""
    SPHERE = "sphere"
    CYLINDER = "cylinder"
    FLAT_PLATE = "flat"
    CUSTOM = "custom"

@dataclass
class FluidState:
    """Represents the state of a fluid."""
    density: float              # kg/m³
    dynamic_viscosity: float    # Pa·s
    temperature: float          # °C
    pressure: float            # Pa
    gravity: float             # m/s²

@dataclass
class BuoyancyResult:
    """Result of buoyancy calculation."""
    buoyant_force: float       # N
    weight: float              # N
    is_floating: bool
    submerged_fraction: float  # 0-1
    draft: float               # m
    
@dataclass
class DragResult:
    """Result of drag force calculation."""
    drag_force: float          # N
    drag_coefficient: float    # Cd
    dynamic_pressure: float    # Pa
    reynolds_number: float
    flow_regime: str           # 'laminar', 'transitional', 'turbulent'

@dataclass
class BernoulliResult:
    """Result from Bernoulli equation application."""
    total_head: float          # m
    elevation_head: float      # m
    velocity_head: float       # m
    pressure_head: float       # m
    velocity_at_point2: float  # m/s

class FluidSimulator:
    """
    Main interface for fluid statics and dynamics simulations.
    
    Provides easy-to-use methods for engineers to:
    - Calculate buoyancy and stability
    - Determine drag and lift forces
    - Analyze flow and pressure
    - Simulate floating objects
    """
    
    def __init__(self, fluid_type: Fluid = Fluid.WATER, gravity: float = 9.81):
        """
        Initialize a fluid simulator.
        
        Args:
            fluid_type: Type of fluid (WATER, AIR, OIL, etc.)
            gravity: Gravitational acceleration (m/s²)
        """
        self.fluid_type = fluid_type
        self.gravity = gravity
        self.state = self._initialize_fluid(fluid_type)
    
    def _initialize_fluid(self, fluid_type: Fluid) -> FluidState:
        """Initialize fluid properties based on type."""
        
        fluids = {
            Fluid.WATER: FluidState(
                density=998.2,
                dynamic_viscosity=0.001002,
                temperature=20.0,
                pressure=101325.0,
                gravity=self.gravity
            ),
            Fluid.AIR: FluidState(
                density=1.225,
                dynamic_viscosity=1.81e-5,
                temperature=15.0,
                pressure=101325.0,
                gravity=self.gravity
            ),
            Fluid.OIL: FluidState(
                density=900.0,
                dynamic_viscosity=0.1,
                temperature=20.0,
                pressure=101325.0,
                gravity=self.gravity
            ),
            Fluid.MERCURY: FluidState(
                density=13546.0,
                dynamic_viscosity=0.001523,
                temperature=20.0,
                pressure=101325.0,
                gravity=self.gravity
            ),
            Fluid.HONEY: FluidState(
                density=1420.0,
                dynamic_viscosity=10.0,
                temperature=20.0,
                pressure=101325.0,
                gravity=self.gravity
            ),
        }
        
        if fluid_type in fluids:
            return fluids[fluid_type]
        else:
            return FluidState(1000, 0.001, 20, 101325, self.gravity)
    
    def set_custom_fluid(self, density: float, viscosity: float, 
                        temperature: float = 20.0):
        """Set custom fluid properties."""
        self.state = FluidState(
            density=density,
            dynamic_viscosity=viscosity,
            temperature=temperature,
            pressure=self.state.pressure,
            gravity=self.gravity
        )
        self.fluid_type = Fluid.CUSTOM
    
    # ============ STATICS ============
    
    def get_pressure_at_depth(self, depth: float) -> float:
        """
        Calculate pressure at a given depth.
        
        Args:
            depth: Depth below surface (m)
            
        Returns:
            Pressure (Pa)
        """
        return self.state.density * self.state.gravity * depth
    
    def get_absolute_pressure(self, depth: float) -> float:
        """Get absolute pressure (including atmospheric)."""
        return self.state.pressure + self.get_pressure_at_depth(depth)
    
    def get_buoyant_force(self, volume: float) -> float:
        """
        Calculate buoyant force (Archimedes' principle).
        
        Args:
            volume: Volume of object (m³)
            
        Returns:
            Buoyant force (N)
        """
        return self.state.density * self.state.gravity * volume
    
    def analyze_floating_object(self, volume: float, mass: float) -> BuoyancyResult:
        """
        Analyze whether an object floats or sinks.
        
        Args:
            volume: Volume of object (m³)
            mass: Mass of object (kg)
            
        Returns:
            BuoyancyResult with detailed analysis
        """
        object_weight = mass * self.state.gravity
        max_buoyancy = self.get_buoyant_force(volume)
        
        is_floating = max_buoyancy >= object_weight
        
        if is_floating:
            submerged_volume = mass / self.state.density
            submerged_fraction = submerged_volume / volume if volume > 0 else 1.0
        else:
            submerged_volume = volume
            submerged_fraction = 1.0
        
        return BuoyancyResult(
            buoyant_force=max_buoyancy,
            weight=object_weight,
            is_floating=is_floating,
            submerged_fraction=submerged_fraction,
            draft=submerged_fraction
        )
    
    def get_hydrostatic_force(self, area: float, depth: float) -> float:
        """
        Calculate hydrostatic force on a submerged surface.
        
        Args:
            area: Area of surface (m²)
            depth: Depth of centroid (m)
            
        Returns:
            Force (N)
        """
        pressure = self.get_pressure_at_depth(depth)
        return pressure * area
    
    def get_pressure_distribution(self, max_depth: float, num_points: int = 10) -> List[float]:
        """
        Get pressure distribution from surface to depth.
        
        Args:
            max_depth: Maximum depth to calculate (m)
            num_points: Number of points in distribution
            
        Returns:
            List of pressures (Pa)
        """
        depths = np.linspace(0, max_depth, num_points)
        return [self.get_pressure_at_depth(d) for d in depths]
    
    # ============ DYNAMICS ============
    
    def get_reynolds_number(self, velocity: float, characteristic_length: float) -> float:
        """
        Calculate Reynolds number (dimensionless flow parameter).
        
        Args:
            velocity: Flow velocity (m/s)
            characteristic_length: Characteristic dimension (m)
            
        Returns:
            Reynolds number (dimensionless)
        """
        if self.state.dynamic_viscosity <= 0:
            return 0.0
        return (self.state.density * velocity * characteristic_length) / self.state.dynamic_viscosity
    
    def get_dynamic_pressure(self, velocity: float) -> float:
        """
        Calculate dynamic pressure (ram pressure).
        
        Args:
            velocity: Velocity (m/s)
            
        Returns:
            Dynamic pressure (Pa)
        """
        return 0.5 * self.state.density * velocity ** 2
    
    def get_drag_force(self, velocity: float, reference_area: float,
                      model: DragModel = DragModel.SPHERE,
                      custom_cd: Optional[float] = None) -> DragResult:
        """
        Calculate drag force on an object in flow.
        
        Args:
            velocity: Flow velocity (m/s)
            reference_area: Reference area for drag (m²)
            model: Drag model (SPHERE, CYLINDER, FLAT_PLATE, CUSTOM)
            custom_cd: Custom drag coefficient (if model=CUSTOM)
            
        Returns:
            DragResult with force and analysis
        """
        re = self.get_reynolds_number(velocity, np.sqrt(reference_area))
        cd = custom_cd if custom_cd is not None else self._get_drag_coefficient(re, model)
        
        q = self.get_dynamic_pressure(velocity)
        drag_force = cd * q * reference_area
        
        if re < 2300:
            flow_regime = "laminar"
        elif re < 4000:
            flow_regime = "transitional"
        else:
            flow_regime = "turbulent"
        
        return DragResult(
            drag_force=drag_force,
            drag_coefficient=cd,
            dynamic_pressure=q,
            reynolds_number=re,
            flow_regime=flow_regime
        )
    
    def _get_drag_coefficient(self, reynolds: float, model: DragModel) -> float:
        """Get drag coefficient based on shape and Reynolds number."""
        
        if model == DragModel.SPHERE:
            if reynolds < 1:
                return 24 / reynolds
            elif reynolds < 1000:
                return 24 / reynolds + 4 / (reynolds ** 0.5) + 0.4
            else:
                return 0.47
        elif model == DragModel.CYLINDER:
            if reynolds < 100:
                return 1.2
            elif reynolds < 1e5:
                return 1.0
            else:
                return 0.3
        elif model == DragModel.FLAT_PLATE:
            return 1.28
        else:
            return 0.5
    
    def get_lift_force(self, velocity: float, wing_area: float,
                      lift_coefficient: float = 1.0) -> float:
        """
        Calculate lift force on an object.
        
        Args:
            velocity: Flow velocity (m/s)
            wing_area: Lifting surface area (m²)
            lift_coefficient: Lift coefficient (dimensionless)
            
        Returns:
            Lift force (N)
        """
        q = self.get_dynamic_pressure(velocity)
        return lift_coefficient * q * wing_area
    
    def apply_bernoulli(self, h1: float, v1: float, p1: float,
                       h2: float, p2: float) -> BernoulliResult:
        """
        Apply Bernoulli equation between two points.
        
        Bernoulli: h + v²/(2g) + P/(ρg) = constant
        
        Args:
            h1: Elevation at point 1 (m)
            v1: Velocity at point 1 (m/s)
            p1: Pressure at point 1 (Pa)
            h2: Elevation at point 2 (m)
            p2: Pressure at point 2 (Pa)
            
        Returns:
            BernoulliResult with head terms and v2
        """
        g = self.state.gravity
        rho = self.state.density
        
        head1 = h1 + (v1 ** 2) / (2 * g) + p1 / (rho * g)
        head2 = h2 + p2 / (rho * g)
        
        velocity_head_2 = head1 - head2
        v2 = np.sqrt(2 * g * max(velocity_head_2, 0))
        
        return BernoulliResult(
            total_head=head1,
            elevation_head=h1,
            velocity_head=(v1 ** 2) / (2 * g),
            pressure_head=p1 / (rho * g),
            velocity_at_point2=v2
        )
    
    def get_continuity_equation(self, a1: float, v1: float, a2: float) -> float:
        """
        Apply continuity equation to find velocity at second point.
        
        Continuity: A1*v1 = A2*v2
        
        Args:
            a1: Area at point 1 (m²)
            v1: Velocity at point 1 (m/s)
            a2: Area at point 2 (m²)
            
        Returns:
            Velocity at point 2 (m/s)
        """
        if a2 <= 0:
            return v1
        return (a1 * v1) / a2
    
    def get_orifice_flow_rate(self, orifice_area: float, pressure_diff: float) -> float:
        """
        Calculate flow rate through an orifice (Bernoulli).
        
        Args:
            orifice_area: Area of orifice (m²)
            pressure_diff: Pressure difference (Pa)
            
        Returns:
            Flow rate (m³/s)
        """
        if self.state.density <= 0:
            return 0.0
        
        velocity = np.sqrt(2 * pressure_diff / self.state.density)
        discharge_coefficient = 0.65
        return discharge_coefficient * orifice_area * velocity
    
    def get_pump_power(self, flow_rate: float, pressure_diff: float) -> float:
        """
        Calculate power required for a pump.
        
        Args:
            flow_rate: Volume flow rate (m³/s)
            pressure_diff: Pressure difference (Pa)
            
        Returns:
            Power (W)
        """
        return flow_rate * pressure_diff
    
    def get_viscous_resistance(self, length: float, diameter: float, velocity: float) -> float:
        """
        Calculate pressure loss in pipe due to friction.
        
        Args:
            length: Pipe length (m)
            diameter: Pipe diameter (m)
            velocity: Flow velocity (m/s)
            
        Returns:
            Pressure loss (Pa)
        """
        if diameter <= 0:
            return 0.0
        
        re = self.get_reynolds_number(velocity, diameter)
        
        if re < 2300:
            friction_factor = 64 / re if re > 0 else 0
        else:
            friction_factor = 0.316 / (re ** 0.25)  # Simplified Blausius
        
        q = self.get_dynamic_pressure(velocity)
        return friction_factor * (length / diameter) * q
    
    def get_flow_regime(self, reynolds: float) -> str:
        """Determine flow regime from Reynolds number."""
        if reynolds < 2300:
            return "laminar"
        elif reynolds < 4000:
            return "transitional"
        else:
            return "turbulent"


# Convenience functions for quick calculations
def quick_buoyancy(volume: float, fluid: Fluid = Fluid.WATER) -> float:
    """Quick calculation of buoyant force."""
    sim = FluidSimulator(fluid)
    return sim.get_buoyant_force(volume)

def quick_drag(velocity: float, reference_area: float, 
              fluid: Fluid = Fluid.WATER) -> float:
    """Quick calculation of drag force."""
    sim = FluidSimulator(fluid)
    result = sim.get_drag_force(velocity, reference_area)
    return result.drag_force

def quick_pressure_at_depth(depth: float, fluid: Fluid = Fluid.WATER) -> float:
    """Quick calculation of pressure at depth."""
    sim = FluidSimulator(fluid)
    return sim.get_pressure_at_depth(depth)

if __name__ == "__main__":
    # Example usage
    print("Fluid Statics and Dynamics Module - Examples")
    print("=" * 50)
    
    # Example 1: Buoyancy in water
    sim_water = FluidSimulator(Fluid.WATER)
    sphere_volume = 0.1  # m³
    buoyancy = sim_water.get_buoyant_force(sphere_volume)
    print(f"\nExample 1: Buoyancy")
    print(f"Sphere volume: {sphere_volume} m³")
    print(f"Buoyant force: {buoyancy:.2f} N")
    
    # Example 2: Drag on sphere
    velocity = 5.0  # m/s
    area = 0.1  # m²
    drag_result = sim_water.get_drag_force(velocity, area, DragModel.SPHERE)
    print(f"\nExample 2: Drag Force")
    print(f"Velocity: {velocity} m/s")
    print(f"Drag force: {drag_result.drag_force:.2f} N")
    print(f"Reynolds number: {drag_result.reynolds_number:.0f}")
    print(f"Flow regime: {drag_result.flow_regime}")
    
    # Example 3: Pressure at depth
    depth = 10.0  # m
    pressure = sim_water.get_pressure_at_depth(depth)
    print(f"\nExample 3: Pressure")
    print(f"Depth: {depth} m")
    print(f"Gauge pressure: {pressure:.0f} Pa")
    print(f"Absolute pressure: {sim_water.get_absolute_pressure(depth):.0f} Pa")
    
    # Example 4: Floating object analysis
    obj_volume = 1.0  # m³
    obj_mass = 800  # kg
    float_result = sim_water.analyze_floating_object(obj_volume, obj_mass)
    print(f"\nExample 4: Floating Object")
    print(f"Volume: {obj_volume} m³")
    print(f"Mass: {obj_mass} kg")
    print(f"Floating: {float_result.is_floating}")
    print(f"Submerged fraction: {float_result.submerged_fraction:.2%}")
