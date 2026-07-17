#ifndef FLUID_PROPERTIES_HPP
#define FLUID_PROPERTIES_HPP

#include <glm/glm.hpp>
#include <string>
#include <map>

namespace SZM::Fluids {

class FluidProperties {
public:
    FluidProperties();
    explicit FluidProperties(const std::string& fluidType);
    
    // Basic properties
    float getDensity() const { return density; }
    void setDensity(float rho) { density = rho; }
    
    float getDynamicViscosity() const { return dynamicViscosity; }
    void setDynamicViscosity(float mu) { dynamicViscosity = mu; }
    
    float getKinematicViscosity() const;
    
    float getTemperature() const { return temperature; }
    void setTemperature(float temp) { temperature = temp; }
    
    float getPressure() const { return pressure; }
    void setPressure(float p) { pressure = p; }
    
    // Thermal properties
    float getSpecificHeat() const { return specificHeat; }
    void setSpecificHeat(float cp) { specificHeat = cp; }
    
    float getThermalConductivity() const { return thermalConductivity; }
    void setThermalConductivity(float k) { thermalConductivity = k; }
    
    // Surface properties
    float getSurfaceTension() const { return surfaceTension; }
    void setSurfaceTension(float sigma) { surfaceTension = sigma; }
    
    // Gravity for the fluid (typically 9.81 m/s^2)
    float getGravity() const { return gravity; }
    void setGravity(float g) { gravity = g; }
    
    // Fluid type identifier
    const std::string& getFluidType() const { return fluidType; }
    void setFluidType(const std::string& type) { fluidType = type; }
    
    // Compressibility
    float getBulkModulus() const { return bulkModulus; }
    void setBulkModulus(float k) { bulkModulus = k; }
    
    // Helper methods for common fluids
    static FluidProperties Water(float tempC = 20.0f);
    static FluidProperties Air(float tempC = 20.0f);
    static FluidProperties Oil();
    static FluidProperties Mercury();
    static FluidProperties Honey();
    
    // State of matter
    enum class State { Liquid, Gas, Plasma };
    State getState() const { return state; }
    void setState(State s) { state = s; }

private:
    std::string fluidType;
    float density;              // kg/m^3
    float dynamicViscosity;     // Pa·s (kg/(m·s))
    float temperature;          // Celsius
    float pressure;             // Pa
    float specificHeat;         // J/(kg·K)
    float thermalConductivity;  // W/(m·K)
    float surfaceTension;       // N/m
    float gravity;              // m/s^2
    float bulkModulus;          // Pa (for compressibility)
    State state;
};

} // namespace SZM::Fluids

#endif // FLUID_PROPERTIES_HPP
