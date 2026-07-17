"""
Engineering Examples for Fluid Statics and Dynamics Module

These examples demonstrate practical applications for engineers:
- Submarine design and stability
- Pipe flow analysis
- Swimming pool pressure analysis
- Wing aerodynamics
- Oil pipeline transport
- Floating structure design
"""

from fluid_simulation import FluidSimulator, Fluid, DragModel, BernoulliResult
from fluid_utils import FlowVisualizer, ThermodynamicCalculator, GeometryHelpers, ResultsAnalyzer
import numpy as np


def example_submarine_stability():
    """
    Design analysis for a submarine considering buoyancy and stability.
    """
    print("\n" + "="*70)
    print("EXAMPLE 1: Submarine Stability Analysis")
    print("="*70)
    
    sim = FluidSimulator(Fluid.WATER)
    
    # Submarine parameters
    hull_volume = 3000.0  # m³ total volume
    dry_mass = 7000.0    # kg without ballast
    ballast_capacity = 500.0  # kg of water can be added
    
    print(f"\nSubmarine Specifications:")
    print(f"  Hull Volume: {hull_volume} m³")
    print(f"  Dry Mass: {dry_mass} kg")
    print(f"  Ballast Capacity: {ballast_capacity} kg")
    
    # Neutral buoyancy calculation
    required_mass = sim.state.density * hull_volume
    print(f"\nNeutral Buoyancy Analysis:")
    print(f"  Required total mass for neutral buoyancy: {required_mass:.0f} kg")
    print(f"  Ballast needed: {required_mass - dry_mass:.0f} kg")
    print(f"  Can achieve neutrality: {'YES' if (required_mass - dry_mass) <= ballast_capacity else 'NO'}")
    
    # Depth analysis
    depths = [10, 50, 100, 200]
    print(f"\nPressure at Various Depths:")
    for depth in depths:
        pressure = sim.get_absolute_pressure(depth)
        pressure_atm = pressure / 101325.0
        print(f"  {depth:3d} m: {pressure:,.0f} Pa ({pressure_atm:6.1f} atm)")
    
    # Hull strength calculation (simplified)
    max_operating_depth = 300  # meters
    pressure_at_depth = sim.get_pressure_at_depth(max_operating_depth)
    print(f"\nHull Strength Analysis (Max Depth: {max_operating_depth} m):")
    print(f"  Gauge Pressure: {pressure_at_depth:,.0f} Pa")
    print(f"  Force on 1 m² area: {pressure_at_depth:,.0f} N")
    print(f"  Recommended Hull Material: Steel or Titanium")


def example_pipe_flow_design():
    """
    Water pipeline design for pressure drop and flow rate.
    """
    print("\n" + "="*70)
    print("EXAMPLE 2: Water Pipeline Design")
    print("="*70)
    
    sim = FluidSimulator(Fluid.WATER)
    
    # Pipeline specifications
    pipe_diameter = 0.5  # meters
    pipe_length = 1000  # meters
    flow_velocity = 2.0  # m/s
    
    print(f"\nPipeline Specifications:")
    print(f"  Diameter: {pipe_diameter} m")
    print(f"  Length: {pipe_length} m")
    print(f"  Flow Velocity: {flow_velocity} m/s")
    
    # Flow analysis
    re = sim.get_reynolds_number(flow_velocity, pipe_diameter)
    flow_regime = sim.get_flow_regime(re)
    
    print(f"\nFlow Analysis:")
    print(f"  Reynolds Number: {re:,.0f}")
    print(f"  Flow Regime: {flow_regime.upper()}")
    
    # Flow rate calculation
    area = np.pi * (pipe_diameter / 2) ** 2
    flow_rate = area * flow_velocity
    mass_flow_rate = sim.state.density * flow_rate
    
    print(f"  Cross-sectional Area: {area:.4f} m²")
    print(f"  Volumetric Flow Rate: {flow_rate:.4f} m³/s ({flow_rate*1000:.2f} L/s)")
    print(f"  Mass Flow Rate: {mass_flow_rate:.2f} kg/s")
    
    # Pressure loss
    pressure_loss = sim.get_viscous_resistance(pipe_length, pipe_diameter, flow_velocity)
    
    print(f"\nPressure Loss Analysis:")
    print(f"  Total Pressure Loss: {pressure_loss:,.0f} Pa ({pressure_loss/101325:.3f} atm)")
    print(f"  Pressure Loss per 100m: {pressure_loss * 100 / pipe_length:,.0f} Pa")
    
    # Pump power requirement
    pump_power = sim.get_pump_power(flow_rate, pressure_loss)
    
    print(f"\nPump Design:")
    print(f"  Required Power: {pump_power:.2f} W ({pump_power/1000:.2f} kW)")
    print(f"  Recommended Pump Type: Centrifugal")


def example_swimming_pool_design():
    """
    Swimming pool design with pressure on walls and floor.
    """
    print("\n" + "="*70)
    print("EXAMPLE 3: Swimming Pool Design")
    print("="*70)
    
    sim = FluidSimulator(Fluid.WATER)
    
    # Pool dimensions
    pool_length = 50.0   # meters
    pool_width = 25.0    # meters
    pool_depth = 2.0     # meters
    
    print(f"\nPool Dimensions:")
    print(f"  Length: {pool_length} m")
    print(f"  Width: {pool_width} m")
    print(f"  Depth: {pool_depth} m")
    print(f"  Total Volume: {pool_length * pool_width * pool_depth:,.0f} m³")
    
    # Water pressure analysis
    pressure_at_depth = sim.get_pressure_at_depth(pool_depth)
    
    print(f"\nWater Pressure Analysis:")
    print(f"  Gauge Pressure at Bottom: {pressure_at_depth:,.0f} Pa")
    print(f"  Absolute Pressure at Bottom: {sim.get_absolute_pressure(pool_depth):,.0f} Pa")
    
    # Force on pool walls
    floor_area = pool_length * pool_width
    wall_area_long = 2 * pool_length * pool_depth
    wall_area_short = 2 * pool_width * pool_depth
    
    print(f"\nStructural Analysis:")
    print(f"  Floor Area: {floor_area:,.0f} m²")
    print(f"  Long Wall Area: {wall_area_long:,.0f} m²")
    print(f"  Short Wall Area: {wall_area_short:,.0f} m²")
    
    # Total force on floor
    total_force_floor = sim.getTotalForceOnContainerBottom(floor_area, pool_depth)
    
    print(f"\nForces on Structure:")
    print(f"  Total Force on Floor: {total_force_floor:,.0f} N")
    print(f"  Force per m² on Floor: {pressure_at_depth:,.0f} N/m²")
    
    # Force on walls (average pressure is half of bottom pressure)
    avg_pressure = sim.get_pressure_at_depth(pool_depth / 2)
    force_on_long_wall = avg_pressure * wall_area_long
    force_on_short_wall = avg_pressure * wall_area_short
    
    print(f"  Force on Long Walls: {force_on_long_wall:,.0f} N each")
    print(f"  Force on Short Walls: {force_on_short_wall:,.0f} N each")
    
    print(f"\nDesign Recommendation:")
    print(f"  Minimum concrete thickness: 30 cm")
    print(f"  Use reinforced concrete with steel rebar")


def example_wing_aerodynamics():
    """
    Aircraft wing design with lift and drag calculations.
    """
    print("\n" + "="*70)
    print("EXAMPLE 4: Aircraft Wing Aerodynamics")
    print("="*70)
    
    sim = FluidSimulator(Fluid.AIR)
    
    # Wing specifications
    wing_area = 122.0    # m² (typical for large aircraft)
    cruise_velocity = 250.0  # m/s (Mach 0.85 at altitude)
    altitude = 11000.0   # meters
    
    # Adjust air density for altitude (simplified)
    # Density decreases exponentially: ρ = ρ₀ * exp(-h/8435)
    sea_level_density = sim.state.density
    altitude_density = sea_level_density * np.exp(-altitude / 8435)
    sim.state.density = altitude_density
    
    print(f"\nAircraft Wing Specifications:")
    print(f"  Wing Area: {wing_area} m²")
    print(f"  Cruise Velocity: {cruise_velocity} m/s")
    print(f"  Altitude: {altitude} m")
    print(f"  Air Density at Altitude: {altitude_density:.4f} kg/m³ (vs {sea_level_density:.2f} at sea level)")
    
    # Reynolds number at cruise
    wing_chord = 8.0  # meters (estimated average chord)
    re = sim.get_reynolds_number(cruise_velocity, wing_chord)
    
    print(f"\nAerodynamic Parameters:")
    print(f"  Wing Chord: {wing_chord} m")
    print(f"  Reynolds Number: {re:,.0f}")
    print(f"  Flow Regime: TURBULENT")
    
    # Lift calculation
    cl_cruise = 0.5  # Cruise lift coefficient
    lift = sim.get_lift_force(cruise_velocity, wing_area, cl_cruise)
    
    # Drag calculation
    drag_result = sim.get_drag_force(cruise_velocity, wing_area, DragModel.FLAT_PLATE)
    
    print(f"\nForces:")
    print(f"  Lift Coefficient (Cl): {cl_cruise}")
    print(f"  Lift Force: {lift:,.0f} N ({lift/9.81:,.0f} kg-force)")
    print(f"  Drag Coefficient (Cd): {drag_result.drag_coefficient:.4f}")
    print(f"  Drag Force: {drag_result.drag_force:,.0f} N")
    print(f"  Lift-to-Drag Ratio: {lift/drag_result.drag_force:.1f}")
    
    # Takeoff analysis
    print(f"\nTakeoff Analysis (at Sea Level):")
    sim.state.density = sea_level_density
    takeoff_velocity = 80.0  # m/s
    cl_takeoff = 1.2
    lift_takeoff = sim.get_lift_force(takeoff_velocity, wing_area, cl_takeoff)
    
    aircraft_mass = 412000  # kg (Boeing 747 approximate)
    aircraft_weight = aircraft_mass * 9.81
    
    print(f"  Takeoff Velocity: {takeoff_velocity} m/s")
    print(f"  Aircraft Mass: {aircraft_mass:,.0f} kg")
    print(f"  Aircraft Weight: {aircraft_weight:,.0f} N")
    print(f"  Lift at Takeoff: {lift_takeoff:,.0f} N")
    print(f"  Can Take Off: {'YES' if lift_takeoff > aircraft_weight else 'NO'}")


def example_oil_pipeline():
    """
    Oil transport pipeline with viscosity effects.
    """
    print("\n" + "="*70)
    print("EXAMPLE 5: Oil Pipeline Transport")
    print("="*70)
    
    sim = FluidSimulator(Fluid.OIL)
    
    # Pipeline specifications
    pipe_diameter = 0.3  # meters
    pipe_length = 500.0  # kilometers = 500,000 meters
    flow_velocity = 1.5  # m/s
    oil_temperature = 40.0  # Celsius
    
    print(f"\nPipeline Specifications:")
    print(f"  Diameter: {pipe_diameter} m")
    print(f"  Length: {pipe_length/1000:.1f} km")
    print(f"  Flow Velocity: {flow_velocity} m/s")
    print(f"  Oil Temperature: {oil_temperature}°C")
    
    # Flow characteristics
    re = sim.get_reynolds_number(flow_velocity, pipe_diameter)
    flow_regime = sim.get_flow_regime(re)
    
    print(f"\nFlow Characteristics:")
    print(f"  Reynolds Number: {re:,.0f}")
    print(f"  Flow Regime: {flow_regime.upper()}")
    
    # Temperature effect on viscosity
    ref_temp = 20.0
    viscosity_at_ref = sim.state.dynamic_viscosity
    viscosity_at_operating = ThermodynamicCalculator.get_temperature_dependent_viscosity(
        viscosity_at_ref, ref_temp, oil_temperature, "oil"
    )
    
    print(f"\nViscosity Analysis:")
    print(f"  Viscosity at {ref_temp}°C: {viscosity_at_ref} Pa·s")
    print(f"  Viscosity at {oil_temperature}°C: {viscosity_at_operating:.4f} Pa·s")
    print(f"  Viscosity Change: {(viscosity_at_operating/viscosity_at_ref - 1)*100:.1f}%")
    
    # Pressure loss (very significant for oil due to high viscosity)
    # Use updated viscosity
    sim.state.dynamic_viscosity = viscosity_at_operating
    pressure_loss_total = sim.get_viscous_resistance(pipe_length, pipe_diameter, flow_velocity)
    
    print(f"\nPressure Loss:")
    print(f"  Total Pressure Loss: {pressure_loss_total:,.0f} Pa ({pressure_loss_total/1e5:.1f} bar)")
    print(f"  Loss per 100 km: {(pressure_loss_total / (pipe_length/1000)) * 100:,.0f} Pa")
    
    # Flow rate and pump requirements
    pipe_area = np.pi * (pipe_diameter/2)**2
    flow_rate = pipe_area * flow_velocity
    pump_power_total = sim.get_pump_power(flow_rate, pressure_loss_total)
    
    print(f"\nFlow and Power:")
    print(f"  Flow Rate: {flow_rate:.4f} m³/s ({flow_rate*3600:.1f} m³/h)")
    print(f"  Total Pump Power Required: {pump_power_total/1e6:.2f} MW")
    print(f"  Pumping Stations Needed: At least {max(1, int(pressure_loss_total / (50e5)))} stations")


def example_floating_barge():
    """
    Floating barge design and stability analysis.
    """
    print("\n" + "="*70)
    print("EXAMPLE 6: Floating Barge Design")
    print("="*70)
    
    sim = FluidSimulator(Fluid.WATER)
    
    # Barge specifications
    barge_length = 100.0    # meters
    barge_width = 20.0      # meters
    barge_height = 5.0      # meters (draft + freeboard)
    cargo_mass = 500000.0   # kg (500 tons)
    empty_mass = 300000.0   # kg (300 tons - structure)
    
    total_mass = empty_mass + cargo_mass
    total_volume = barge_length * barge_width * barge_height
    
    print(f"\nBarge Specifications:")
    print(f"  Length: {barge_length} m")
    print(f"  Width: {barge_width} m")
    print(f"  Height: {barge_height} m")
    print(f"  Total Volume: {total_volume:,.0f} m³")
    print(f"  Empty Mass: {empty_mass:,.0f} kg")
    print(f"  Cargo Mass: {cargo_mass:,.0f} kg")
    print(f"  Total Mass: {total_mass:,.0f} kg")
    
    # Buoyancy analysis
    result = sim.analyze_floating_object(total_volume, total_mass)
    
    print(f"\nBuoyancy Analysis:")
    print(f"  Max Buoyant Force: {result.buoyant_force[1]:,.0f} N")
    print(f"  Total Weight: {total_mass * 9.81:,.0f} N")
    print(f"  Floating Status: {'FLOATS' if result.is_floating else 'SINKS'}")
    
    if result.is_floating:
        draft = result.draft * barge_height
        freeboard = (1 - result.draft) * barge_height
        print(f"  Draft (Submerged Depth): {draft:.2f} m")
        print(f"  Free Board (Above Water): {freeboard:.2f} m")
        print(f"  Submerged Volume: {result.submerged_volume:,.0f} m³")
    
    # Stability under tilt
    print(f"\nStability Analysis:")
    print(f"  Metacentric Height: Needs detailed calculation")
    print(f"  Stability Rating: GOOD (wide, low center of gravity)")
    print(f"  Maximum Safe Tilt: ~15 degrees")


def main():
    """Run all examples."""
    print("\n" + "="*70)
    print("FLUID STATICS AND DYNAMICS - ENGINEERING EXAMPLES")
    print("Practical applications for simulation, creation, and innovation")
    print("="*70)
    
    example_submarine_stability()
    example_pipe_flow_design()
    example_swimming_pool_design()
    example_wing_aerodynamics()
    example_oil_pipeline()
    example_floating_barge()
    
    print("\n" + "="*70)
    print("Examples completed successfully!")
    print("="*70)


if __name__ == "__main__":
    main()
