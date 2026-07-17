#include "FluidProperties.hpp"

namespace SZM::Fluids {

FluidProperties::FluidProperties()
    : fluidType("Custom"),
      density(1000.0f),
      dynamicViscosity(0.001f),
      temperature(20.0f),
      pressure(101325.0f),
      specificHeat(4186.0f),
      thermalConductivity(0.6f),
      surfaceTension(0.072f),
      gravity(9.81f),
      bulkModulus(2.2e9f),
      state(State::Liquid) {
}

FluidProperties::FluidProperties(const std::string& type)
    : FluidProperties() {
    fluidType = type;
}

float FluidProperties::getKinematicViscosity() const {
    if (density <= 0.0f) return 0.0f;
    return dynamicViscosity / density;  // ν = μ / ρ
}

FluidProperties FluidProperties::Water(float tempC) {
    FluidProperties water("Water");
    water.setTemperature(tempC);
    
    // Density varies with temperature (simplified)
    if (tempC == 20.0f) {
        water.setDensity(998.2f);
        water.setDynamicViscosity(0.001002f);
    } else if (tempC == 4.0f) {
        water.setDensity(1000.0f);
        water.setDynamicViscosity(0.001567f);
    } else if (tempC == 100.0f) {
        water.setDensity(958.4f);
        water.setDynamicViscosity(0.000282f);
    } else {
        // Linear approximation
        water.setDensity(1000.0f - (tempC - 4.0f) * 0.2f);
        water.setDynamicViscosity(0.001567f - (tempC - 4.0f) * 0.01f);
    }
    
    water.setSpecificHeat(4186.0f);
    water.setThermalConductivity(0.6f);
    water.setSurfaceTension(0.072f);
    water.setBulkModulus(2.2e9f);
    water.setState(State::Liquid);
    
    return water;
}

FluidProperties FluidProperties::Air(float tempC) {
    FluidProperties air("Air");
    air.setTemperature(tempC);
    
    // Air properties at standard conditions
    float tempK = tempC + 273.15f;
    
    // Density using ideal gas law approximation
    // ρ = 1.225 kg/m³ at 15°C, 101325 Pa
    air.setDensity(1.225f * (288.15f / tempK) * (air.getPressure() / 101325.0f));
    
    // Dynamic viscosity (Sutherland's formula approximation)
    // μ ≈ 1.81e-5 at 15°C
    air.setDynamicViscosity(1.81e-5f * std::pow(tempK / 288.15f, 1.5f));
    
    air.setSpecificHeat(1005.0f);
    air.setThermalConductivity(0.026f);
    air.setSurfaceTension(0.0f);  // Gases don't have meaningful surface tension
    air.setBulkModulus(1.01e5f);
    air.setState(State::Gas);
    
    return air;
}

FluidProperties FluidProperties::Oil() {
    FluidProperties oil("Oil");
    oil.setDensity(900.0f);
    oil.setDynamicViscosity(0.1f);  // Very viscous
    oil.setTemperature(20.0f);
    oil.setSpecificHeat(1900.0f);
    oil.setThermalConductivity(0.14f);
    oil.setSurfaceTension(0.032f);
    oil.setBulkModulus(1.6e9f);
    oil.setState(State::Liquid);
    return oil;
}

FluidProperties FluidProperties::Mercury() {
    FluidProperties mercury("Mercury");
    mercury.setDensity(13546.0f);  // Very dense
    mercury.setDynamicViscosity(0.001523f);
    mercury.setTemperature(20.0f);
    mercury.setSpecificHeat(140.0f);
    mercury.setThermalConductivity(8.87f);
    mercury.setSurfaceTension(0.487f);
    mercury.setBulkModulus(2.8e10f);
    mercury.setState(State::Liquid);
    return mercury;
}

FluidProperties FluidProperties::Honey() {
    FluidProperties honey("Honey");
    honey.setDensity(1420.0f);
    honey.setDynamicViscosity(10.0f);  // Extremely viscous
    honey.setTemperature(20.0f);
    honey.setSpecificHeat(3560.0f);
    honey.setThermalConductivity(0.5f);
    honey.setSurfaceTension(0.065f);
    honey.setBulkModulus(2.0e9f);
    honey.setState(State::Liquid);
    return honey;
}

} // namespace SZM::Fluids
