"""
Fluid utilities and helper functions for visualization and analysis.
"""

import numpy as np
from typing import Tuple, List, Optional, Dict
from dataclasses import dataclass

@dataclass
class VelocityField:
    """Represents a velocity field in 3D space."""
    positions: np.ndarray      # (N, 3) array of positions
    velocities: np.ndarray     # (N, 3) array of velocity vectors
    magnitudes: np.ndarray     # (N,) array of velocity magnitudes
    streamlines: Optional[List[np.ndarray]] = None

class FlowVisualizer:
    """Utilities for visualizing fluid flow."""
    
    @staticmethod
    def create_velocity_field(flow_direction: Tuple[float, float, float],
                             flow_velocity: float,
                             grid_size: int = 10,
                             domain_size: float = 1.0) -> VelocityField:
        """
        Create a uniform velocity field.
        
        Args:
            flow_direction: Direction vector (x, y, z)
            flow_velocity: Magnitude of velocity (m/s)
            grid_size: Number of points per dimension
            domain_size: Size of domain (m)
            
        Returns:
            VelocityField object with positions and velocities
        """
        # Create grid
        coords = np.linspace(-domain_size/2, domain_size/2, grid_size)
        xx, yy, zz = np.meshgrid(coords, coords, coords)
        
        positions = np.column_stack([xx.ravel(), yy.ravel(), zz.ravel()])
        
        # Create velocity field
        direction = np.array(flow_direction)
        direction = direction / np.linalg.norm(direction)
        
        n_points = grid_size ** 3
        velocities = np.tile(direction * flow_velocity, (n_points, 1))
        magnitudes = np.linalg.norm(velocities, axis=1)
        
        return VelocityField(
            positions=positions,
            velocities=velocities,
            magnitudes=magnitudes
        )
    
    @staticmethod
    def create_wake_field(object_position: Tuple[float, float, float],
                         flow_direction: Tuple[float, float, float],
                         flow_velocity: float,
                         grid_size: int = 10) -> VelocityField:
        """
        Create a velocity field with object wake.
        
        Wake velocity reduces behind the object.
        """
        field = FlowVisualizer.create_velocity_field(
            flow_direction, flow_velocity, grid_size
        )
        
        obj_pos = np.array(object_position)
        
        # Reduce velocity in wake region
        for i, pos in enumerate(field.positions):
            distance = np.linalg.norm(pos - obj_pos)
            
            # Wake effect - velocity reduction proportional to closeness
            if distance > 0:
                wake_factor = 1.0 / (1.0 + distance)
                field.velocities[i] *= wake_factor
                field.magnitudes[i] = np.linalg.norm(field.velocities[i])
        
        return field
    
    @staticmethod
    def get_streamlines(velocity_field: VelocityField,
                       start_positions: List[Tuple[float, float, float]],
                       num_steps: int = 100,
                       step_size: float = 0.01) -> List[np.ndarray]:
        """
        Calculate streamlines starting from given positions.
        
        Args:
            velocity_field: VelocityField object
            start_positions: List of starting positions
            num_steps: Number of integration steps
            step_size: Time step for integration
            
        Returns:
            List of streamline trajectories
        """
        streamlines = []
        
        for start_pos in start_positions:
            streamline = [np.array(start_pos)]
            current_pos = np.array(start_pos)
            
            for _ in range(num_steps):
                # Find nearest grid point
                distances = np.linalg.norm(
                    velocity_field.positions - current_pos, axis=1
                )
                nearest_idx = np.argmin(distances)
                
                # Get velocity at this point
                velocity = velocity_field.velocities[nearest_idx]
                
                # Update position
                current_pos = current_pos + velocity * step_size
                streamline.append(current_pos.copy())
            
            streamlines.append(np.array(streamline))
        
        return streamlines


class ThermodynamicCalculator:
    """Utilities for thermodynamic calculations."""
    
    @staticmethod
    def get_temperature_dependent_viscosity(base_viscosity: float,
                                           base_temp: float,
                                           new_temp: float,
                                           fluid_type: str = "water") -> float:
        """
        Calculate viscosity at different temperature (simplified).
        
        Args:
            base_viscosity: Viscosity at base temperature (Pa·s)
            base_temp: Base temperature (°C)
            new_temp: New temperature (°C)
            fluid_type: Type of fluid
            
        Returns:
            Viscosity at new temperature (Pa·s)
        """
        temp_diff = new_temp - base_temp
        
        # Temperature sensitivity (simplified)
        if fluid_type == "water":
            # Water viscosity decreases with temperature
            temp_factor = np.exp(-0.03 * temp_diff)
        elif fluid_type == "oil":
            temp_factor = np.exp(-0.05 * temp_diff)
        else:
            temp_factor = np.exp(-0.02 * temp_diff)
        
        return base_viscosity * temp_factor
    
    @staticmethod
    def get_temperature_dependent_density(base_density: float,
                                         base_temp: float,
                                         new_temp: float,
                                         thermal_expansion: float = 0.0002) -> float:
        """
        Calculate density at different temperature.
        
        Args:
            base_density: Density at base temperature (kg/m³)
            base_temp: Base temperature (°C)
            new_temp: New temperature (°C)
            thermal_expansion: Volumetric thermal expansion coefficient (1/°C)
            
        Returns:
            Density at new temperature (kg/m³)
        """
        temp_diff = new_temp - base_temp
        # ρ = ρ₀ / (1 + α * ΔT)
        return base_density / (1.0 + thermal_expansion * temp_diff)


class GeometryHelpers:
    """Geometry utilities for common shapes."""
    
    @staticmethod
    def get_sphere_properties(radius: float) -> Dict[str, float]:
        """Get geometric properties of a sphere."""
        return {
            "radius": radius,
            "diameter": 2 * radius,
            "volume": (4/3) * np.pi * radius**3,
            "surface_area": 4 * np.pi * radius**2,
            "center_of_volume": 0.0,  # Relative to geometric center
        }
    
    @staticmethod
    def get_cylinder_properties(radius: float, height: float) -> Dict[str, float]:
        """Get geometric properties of a cylinder."""
        return {
            "radius": radius,
            "height": height,
            "volume": np.pi * radius**2 * height,
            "surface_area": 2 * np.pi * radius * (radius + height),
            "lateral_area": 2 * np.pi * radius * height,
            "center_of_volume": height / 2.0,  # From base
        }
    
    @staticmethod
    def get_rectangular_box_properties(length: float, width: float,
                                       height: float) -> Dict[str, float]:
        """Get geometric properties of a rectangular box."""
        return {
            "length": length,
            "width": width,
            "height": height,
            "volume": length * width * height,
            "surface_area": 2 * (length*width + width*height + height*length),
            "center_of_volume": height / 2.0,  # From base
        }
    
    @staticmethod
    def get_cone_properties(radius: float, height: float) -> Dict[str, float]:
        """Get geometric properties of a cone."""
        return {
            "radius": radius,
            "height": height,
            "volume": (1/3) * np.pi * radius**2 * height,
            "surface_area": np.pi * radius * (radius + np.sqrt(radius**2 + height**2)),
            "center_of_volume": height / 4.0,  # From base
        }


class DataInterpolator:
    """Utilities for interpolating fluid properties and results."""
    
    @staticmethod
    def interpolate_property(x: float, x_data: List[float],
                            y_data: List[float]) -> float:
        """
        Linear interpolation of property values.
        
        Args:
            x: Point to interpolate
            x_data: Known x values
            y_data: Known y values
            
        Returns:
            Interpolated value
        """
        x_data = np.array(x_data)
        y_data = np.array(y_data)
        
        if x <= x_data[0]:
            return y_data[0]
        if x >= x_data[-1]:
            return y_data[-1]
        
        idx = np.searchsorted(x_data, x) - 1
        x1, x2 = x_data[idx], x_data[idx + 1]
        y1, y2 = y_data[idx], y_data[idx + 1]
        
        return y1 + (x - x1) * (y2 - y1) / (x2 - x1)
    
    @staticmethod
    def interpolate_pressure_field(pressures_at_depths: Dict[float, float],
                                  query_depth: float) -> float:
        """Interpolate pressure at arbitrary depth."""
        depths = sorted(pressures_at_depths.keys())
        pressure_values = [pressures_at_depths[d] for d in depths]
        
        return DataInterpolator.interpolate_property(
            query_depth, depths, pressure_values
        )


class ResultsAnalyzer:
    """Analyze and summarize simulation results."""
    
    @staticmethod
    def summarize_buoyancy(object_volume: float, object_mass: float,
                          fluid_density: float, gravity: float = 9.81) -> str:
        """Generate summary of buoyancy analysis."""
        buoyant_force = fluid_density * gravity * object_volume
        weight = object_mass * gravity
        submerged_volume = object_mass / fluid_density if fluid_density > 0 else 0
        
        summary = f"""
Buoyancy Analysis Summary
========================
Object Volume:      {object_volume:.4f} m³
Object Mass:        {object_mass:.2f} kg
Object Weight:      {weight:.2f} N
Fluid Density:      {fluid_density:.2f} kg/m³

Buoyant Force:      {buoyant_force:.2f} N
Required Submersion: {sub_merged_volume:.4f} m³
Float Status:       {"FLOATS" if buoyant_force >= weight else "SINKS"}

If Floating:
  - Submerged Fraction: {min(submerged_volume/object_volume, 1.0):.1%}
  - Free Board Volume: {max(object_volume - submerged_volume, 0):.4f} m³
"""
        return summary
    
    @staticmethod
    def summarize_drag(drag_force: float, lift_force: float,
                      velocity: float, reynolds: float,
                      flow_regime: str) -> str:
        """Generate summary of aerodynamic analysis."""
        summary = f"""
Aerodynamic Analysis Summary
===========================
Velocity:           {velocity:.2f} m/s
Drag Force:         {drag_force:.2f} N
Lift Force:         {lift_force:.2f} N

Reynolds Number:    {reynolds:.0f}
Flow Regime:        {flow_regime.upper()}

Force Ratio (L/D):  {lift_force/drag_force if drag_force > 0 else 0:.2f}
"""
        return summary


if __name__ == "__main__":
    print("Fluid Utilities Module - Testing")
    print("=" * 50)
    
    # Test velocity field creation
    print("\n1. Velocity Field Creation")
    field = FlowVisualizer.create_velocity_field(
        flow_direction=(1, 0, 0),
        flow_velocity=5.0,
        grid_size=5
    )
    print(f"   Grid points: {len(field.positions)}")
    print(f"   Velocity range: {field.magnitudes.min():.2f} - {field.magnitudes.max():.2f} m/s")
    
    # Test geometry properties
    print("\n2. Sphere Properties")
    sphere = GeometryHelpers.get_sphere_properties(radius=0.5)
    print(f"   Volume: {sphere['volume']:.4f} m³")
    print(f"   Surface Area: {sphere['surface_area']:.4f} m²")
