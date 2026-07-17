# Phase 4: Advanced Domains — Thermal, Electrical & AI

**Status:** 🔲 **PENDING** — Ready to start

**Objective:** Extend SZM Forge with thermal, electrical, and AI capabilities

**Timeline:** 4-6 weeks

---

## Phase 4 Overview

Phase 4 adds three advanced simulation domains:

1. **Module 300: Thermal Engine** — Heat transfer simulation
2. **Module 400: Electrical Engine** — Circuit simulation
3. **Module 500: CEA AI Assistant** — Design optimization & failure prediction

---

## Module 300: Thermal Engine

### Objective
Full heat transfer simulation with conduction, convection, and radiation.

### Deliverables

#### 3.1 Thermal Data Types
**File:** `src/Thermal/ThermalDataTypes.hpp`

```cpp
struct ThermalNode {
    uint32_t id;
    Vector3 position;
    float temperature;      // Kelvin
    float heatCapacity;     // J/(kg·K)
    float density;          // kg/m³
    float thermalConductivity; // W/(m·K)
};

struct ThermalElement {
    uint32_t id;
    std::vector<uint32_t> nodeIds;
    float area;             // m²
    float thickness;        // m
    MaterialType material;
};

struct ThermalBoundaryCondition {
    uint32_t nodeId;
    float temperature;      // Fixed temperature
    float heatFlux;         // W/m² (Neumann)
    float convectionCoeff;  // W/(m²·K)
    float ambientTemp;      // K
};
```

#### 3.2 Thermal Solver
**File:** `src/Thermal/ThermalSolver.hpp/cpp`

```cpp
class ThermalSolver {
public:
    static ThermalSolver& GetInstance();
    
    // Mesh management
    uint32_t AddNode(const Vector3& pos, float temp);
    uint32_t AddElement(const std::vector<uint32_t>& nodeIds);
    void AddBoundaryCondition(const ThermalBoundaryCondition& bc);
    
    // Simulation
    void AssembleSystem();
    void ApplyBoundaryConditions();
    void Solve(double timeStep);
    
    // Results
    float GetNodeTemperature(uint32_t nodeId) const;
    float GetElementHeatFlux(uint32_t elemId) const;
    std::vector<float> GetTemperatureField() const;
    
private:
    void CalculateConduction();
    void CalculateConvection();
    void CalculateRadiation();
    void UpdateTemperatures(double dt);
};
```

#### 3.3 Heat Transfer Models

**Conduction (Fourier's Law):**
```
q = -k * dT/dx
```

**Convection (Newton's Law):**
```
q = h * (T_surface - T_ambient)
```

**Radiation (Stefan-Boltzmann):**
```
q = ε * σ * (T⁴ - T_ambient⁴)
```

#### 3.4 Thermal UI Panel
**File:** `src/UI/panels/ThermalPanel.hpp/cpp`

- Temperature field visualization
- Boundary condition editor
- Heat source/sink controls
- Transient analysis timeline
- Temperature contour plots

### Integration Points
- `SimulationEngine` — Get component temperatures
- `MaterialDatabase` — Get thermal properties
- `Graphics` — Render temperature heatmaps
- `EventBus` — Publish "ThermalUpdate" events

### Tests
- [ ] Conduction through plate
- [ ] Convection cooling
- [ ] Radiation equilibrium
- [ ] Transient heating/cooling
- [ ] Boundary condition application

---

## Module 400: Electrical Engine

### Objective
Circuit simulation with power distribution and fault detection.

### Deliverables

#### 4.1 Electrical Data Types
**File:** `src/Electrical/ElectricalDataTypes.hpp`

```cpp
enum class ComponentType {
    Resistor, Capacitor, Inductor, Voltage, Current, Diode, Transistor
};

struct ElectricalComponent {
    uint32_t id;
    ComponentType type;
    float value;            // Ohms, Farads, Henries, Volts, Amps
    float tolerance;        // %
    float powerRating;      // Watts
    float temperature;      // K
    bool isFaulty;
};

struct ElectricalNode {
    uint32_t id;
    std::string name;
    float voltage;          // Volts
    float current;          // Amps
    std::vector<uint32_t> componentIds;
};

struct Circuit {
    std::vector<ElectricalNode> nodes;
    std::vector<ElectricalComponent> components;
    std::vector<std::pair<uint32_t, uint32_t>> connections;
};
```

#### 4.2 Circuit Solver
**File:** `src/Electrical/CircuitSolver.hpp/cpp`

```cpp
class CircuitSolver {
public:
    static CircuitSolver& GetInstance();
    
    // Circuit construction
    uint32_t AddNode(const std::string& name);
    uint32_t AddComponent(ComponentType type, float value);
    void Connect(uint32_t nodeA, uint32_t nodeB, uint32_t componentId);
    
    // Analysis
    void SolveAC(float frequency);
    void SolveDC();
    void SolveTransient(double timeStep);
    
    // Results
    float GetNodeVoltage(uint32_t nodeId) const;
    float GetComponentCurrent(uint32_t componentId) const;
    float GetComponentPower(uint32_t componentId) const;
    
    // Fault detection
    bool DetectFault(uint32_t componentId);
    std::vector<uint32_t> GetFaultyComponents() const;
    
private:
    void AssembleAdmittanceMatrix();
    void ApplyKirchhoffLaws();
    void CheckThermalStress();
};
```

#### 4.3 Analysis Methods

**DC Analysis (Ohm's Law):**
```
V = I * R
P = V * I = I² * R
```

**AC Analysis (Impedance):**
```
Z = R + j(ωL - 1/(ωC))
|Z| = √(R² + (ωL - 1/(ωC))²)
```

**Transient Analysis:**
```
dI/dt = (V - I*R) / L
dV/dt = I / C
```

#### 4.4 Electrical UI Panel
**File:** `src/UI/panels/ElectricalPanel.hpp/cpp`

- Circuit schematic editor
- Node voltage display
- Component current/power display
- Fault indicator lights
- AC/DC/Transient analysis selector
- Power distribution visualization

### Integration Points
- `SimulationEngine` — Get component power dissipation
- `ThermalEngine` — Heat from electrical losses
- `MaterialDatabase` — Component ratings
- `Graphics` — Render circuit topology
- `EventBus` — Publish "ElectricalFault" events

### Tests
- [ ] Simple resistor circuit
- [ ] RC charging/discharging
- [ ] AC impedance calculation
- [ ] Power dissipation
- [ ] Fault detection (overcurrent)

---

## Module 500: CEA AI Assistant

### Objective
Design optimization and failure prediction using machine learning.

### Deliverables

#### 5.1 AI Data Types
**File:** `src/AI/AIDataTypes.hpp`

```cpp
struct DesignMetrics {
    float stress;
    float temperature;
    float power;
    float cost;
    float weight;
    float reliability;      // 0-1
};

struct DesignRecommendation {
    std::string title;
    std::string description;
    float expectedImprovement;  // %
    std::vector<std::string> actions;
    float confidence;       // 0-1
};

struct FailurePrediction {
    uint32_t componentId;
    std::string failureMode;
    float probability;      // 0-1
    float timeToFailure;    // hours
    std::vector<std::string> preventiveMeasures;
};
```

#### 5.2 AI Engine
**File:** `src/AI/AIEngine.hpp/cpp`

```cpp
class AIEngine {
public:
    static AIEngine& GetInstance();
    
    // Training
    void TrainOnHistoricalData(const std::vector<DesignMetrics>& data);
    void UpdateFailureDatabase(const std::vector<FailurePrediction>& failures);
    
    // Analysis
    std::vector<DesignRecommendation> AnalyzeDesign(const DesignMetrics& metrics);
    FailurePrediction PredictFailure(uint32_t componentId);
    float CalculateReliability(const DesignMetrics& metrics);
    
    // Optimization
    DesignMetrics OptimizeForCost(const DesignMetrics& current);
    DesignMetrics OptimizeForReliability(const DesignMetrics& current);
    DesignMetrics OptimizeForWeight(const DesignMetrics& current);
    
    // Explanations
    std::string ExplainRecommendation(const DesignRecommendation& rec);
    std::string ExplainFailurePrediction(const FailurePrediction& pred);
    
private:
    void CalculateStressMargin();
    void CalculateThermalMargin();
    void CalculateReliabilityScore();
};
```

#### 5.3 Optimization Algorithms

**Design Space Exploration:**
- Genetic algorithm for multi-objective optimization
- Pareto frontier calculation
- Sensitivity analysis

**Failure Prediction:**
- Weibull distribution for component lifetime
- Stress-life (S-N) curves
- Arrhenius model for thermal acceleration

**Recommendations:**
- Rule-based expert system
- Fuzzy logic for uncertainty
- Confidence scoring

#### 5.4 AI UI Panel
**File:** `src/UI/panels/AIPanel.hpp/cpp`

- Design metrics summary
- Recommendations list with confidence scores
- Failure predictions with timeline
- Optimization suggestions
- Design comparison (current vs. optimized)
- Explanation panel for each recommendation

### Integration Points
- `SimulationEngine` — Get current design metrics
- `ThermalEngine` — Temperature data
- `ElectricalEngine` — Power/fault data
- `MaterialDatabase` — Material properties & limits
- `EventBus` — Publish "AIRecommendation" events

### Tests
- [ ] Stress margin calculation
- [ ] Thermal margin calculation
- [ ] Reliability scoring
- [ ] Failure prediction accuracy
- [ ] Optimization convergence

---

## Implementation Timeline

### Week 1: Thermal Engine
**Day 1-2:** Data types and solver framework
**Day 3-4:** Heat transfer models (conduction, convection, radiation)
**Day 5:** UI panel and integration

### Week 2: Electrical Engine
**Day 1-2:** Data types and circuit solver
**Day 3-4:** Analysis methods (DC, AC, transient)
**Day 5:** UI panel and integration

### Week 3: AI Assistant
**Day 1-2:** Data types and AI engine framework
**Day 3-4:** Optimization algorithms and failure prediction
**Day 5:** UI panel and integration

### Week 4: Integration & Testing
**Day 1-2:** Cross-module integration
**Day 3-4:** Comprehensive testing
**Day 5:** Documentation and polish

---

## Architecture

### Module Dependencies

```
Application
    ↓
SimulationEngine (Phase 2)
    ├─ ThermalEngine (Phase 4)
    ├─ ElectricalEngine (Phase 4)
    └─ AIEngine (Phase 4)

UIManager
    ├─ ThermalPanel
    ├─ ElectricalPanel
    └─ AIPanel

EventBus
    ├─ "SimulationTick"
    ├─ "ThermalUpdate"
    ├─ "ElectricalFault"
    └─ "AIRecommendation"
```

### Event Flow

```
Application::TickSystems()
    ├─ SimulationEngine::Tick()
    │   └─ EventBus::Publish("SimulationTick")
    │
    ├─ ThermalEngine::Tick()
    │   └─ EventBus::Publish("ThermalUpdate")
    │
    ├─ ElectricalEngine::Tick()
    │   └─ EventBus::Publish("ElectricalFault")
    │
    ├─ AIEngine::Analyze()
    │   └─ EventBus::Publish("AIRecommendation")
    │
    └─ UIManager::RenderUI()
        ├─ ThermalPanel::Draw()
        ├─ ElectricalPanel::Draw()
        └─ AIPanel::Draw()
```

---

## File Structure

### New Directories
```
src/Thermal/
├── ThermalDataTypes.hpp
├── ThermalSolver.hpp/cpp
├── ThermalModels.hpp/cpp
└── CMakeLists.txt

src/Electrical/
├── ElectricalDataTypes.hpp
├── CircuitSolver.hpp/cpp
├── AnalysisMethods.hpp/cpp
└── CMakeLists.txt

src/AI/
├── AIDataTypes.hpp
├── AIEngine.hpp/cpp
├── OptimizationAlgorithms.hpp/cpp
├── FailurePrediction.hpp/cpp
└── CMakeLists.txt

src/UI/panels/
├── ThermalPanel.hpp/cpp
├── ElectricalPanel.hpp/cpp
└── AIPanel.hpp/cpp
```

### Modified Files
```
CMakeLists.txt (root)           ← Add 3 new modules
src/Core/Application.cpp        ← Hook Thermal/Electrical/AI ticks
src/UI/UIManager.cpp            ← Register 3 new panels
src/UI/CMakeLists.txt           ← Add 3 new panels
```

---

## Dependencies

### Required Libraries
- Eigen3 (already available)
- Standard library

### Optional Libraries
- BLAS/LAPACK (for advanced linear algebra)
- GSL (GNU Scientific Library) for special functions
- TensorFlow Lite (for ML inference, optional)

---

## Testing Strategy

### Unit Tests
```cpp
// Thermal tests
TEST(ThermalSolver, ConductionThroughPlate)
TEST(ThermalSolver, ConvectionCooling)
TEST(ThermalSolver, RadiationEquilibrium)

// Electrical tests
TEST(CircuitSolver, OhmsLaw)
TEST(CircuitSolver, ACImpedance)
TEST(CircuitSolver, TransientResponse)

// AI tests
TEST(AIEngine, StressMarginCalculation)
TEST(AIEngine, FailurePredictionAccuracy)
TEST(AIEngine, OptimizationConvergence)
```

### Integration Tests
```cpp
// Cross-module tests
TEST(Integration, ThermalElectricalCoupling)
TEST(Integration, AIRecommendationAccuracy)
TEST(Integration, EventBusIntegration)
```

### Performance Tests
```cpp
// Performance benchmarks
TEST(Performance, ThermalSolverSpeed)
TEST(Performance, CircuitSolverSpeed)
TEST(Performance, AIAnalysisSpeed)
```

---

## Success Criteria

### Phase 4 Complete When:
- ✅ Thermal engine fully functional
- ✅ Electrical engine fully functional
- ✅ AI assistant fully functional
- ✅ All three UI panels rendering
- ✅ Cross-module integration working
- ✅ All tests passing
- ✅ Performance > 60 FPS
- ✅ Documentation complete

---

## Deliverables Checklist

### Thermal Engine
- [ ] ThermalDataTypes.hpp
- [ ] ThermalSolver.hpp/cpp
- [ ] ThermalModels.hpp/cpp (conduction, convection, radiation)
- [ ] ThermalPanel.hpp/cpp
- [ ] CMakeLists.txt
- [ ] Tests (5+ test cases)
- [ ] Documentation

### Electrical Engine
- [ ] ElectricalDataTypes.hpp
- [ ] CircuitSolver.hpp/cpp
- [ ] AnalysisMethods.hpp/cpp (DC, AC, transient)
- [ ] ElectricalPanel.hpp/cpp
- [ ] CMakeLists.txt
- [ ] Tests (5+ test cases)
- [ ] Documentation

### AI Assistant
- [ ] AIDataTypes.hpp
- [ ] AIEngine.hpp/cpp
- [ ] OptimizationAlgorithms.hpp/cpp
- [ ] FailurePrediction.hpp/cpp
- [ ] AIPanel.hpp/cpp
- [ ] CMakeLists.txt
- [ ] Tests (5+ test cases)
- [ ] Documentation

### Integration
- [ ] Application.cpp updated
- [ ] UIManager.cpp updated
- [ ] Root CMakeLists.txt updated
- [ ] EventBus integration
- [ ] Cross-module tests
- [ ] Performance tests

---

## Quick Reference

### Key Classes
- `ThermalSolver` — Heat transfer simulation
- `CircuitSolver` — Electrical circuit analysis
- `AIEngine` — Design optimization & failure prediction

### Key Events
- `"ThermalUpdate"` — Temperature field updated
- `"ElectricalFault"` — Fault detected
- `"AIRecommendation"` — New recommendation available

### Key Metrics
- Thermal: Temperature, heat flux, thermal stress
- Electrical: Voltage, current, power, fault probability
- AI: Reliability score, failure probability, optimization margin

---

## Next Steps After Phase 4

### Phase 5: Digital Twin
- Real-world machine control
- Sensor integration
- Predictive maintenance
- Remote monitoring

### Phase 6: Advanced Features
- Multi-physics coupling
- Optimization under constraints
- Machine learning model training
- Cloud integration

---

## Summary

**Phase 4 extends SZM Forge with three advanced domains:**

1. **Thermal Engine** — Full heat transfer simulation
2. **Electrical Engine** — Circuit analysis and fault detection
3. **AI Assistant** — Design optimization and failure prediction

**Timeline:** 4-6 weeks
**Complexity:** High
**Team Size:** 2-3 developers

**Status:** Ready to start

---

**Last Updated:** Phase 2 Complete
**Next Milestone:** Phase 4 Start
