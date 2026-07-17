# SZM_Forge + Codes Integration Manifest

## Strategic Integration Plan

### Tier 1: High-Impact (Direct Integration)

#### 1. **Three.js** (3D Graphics Visualization)
- **Current**: WebViewer exports to Three.js format
- **Enhancement**: Embed full Three.js library for real-time web visualization
- **Benefits**: 
  - Interactive 3D scene exploration
  - Real-time physics preview
  - Cross-platform web access
- **Integration Path**:
  ```
  codes/three.js-dev/ → external/three.js/
  src/WebViewer/ThreeJSRenderer.hpp/cpp
  ```

#### 2. **FastAPI** (REST API Server)
- **Current**: FastAPIBridge placeholder
- **Enhancement**: Full FastAPI-based REST server for simulations
- **Benefits**:
  - Distributed simulation jobs
  - Async task queuing
  - Real-time WebSocket updates
- **Integration Path**:
  ```
  codes/fastapi-master/ → external/fastapi/
  src/WebAPI/SimulationServer.hpp/cpp
  ```

#### 3. **React** (Web UI Frontend)
- **Current**: None (desktop ImGui only)
- **Enhancement**: React-based web dashboard
- **Benefits**:
  - Remote simulation submission
  - Real-time results visualization
  - Material/component property management
  - Cross-platform access
- **Integration Path**:
  ```
  codes/react-main/ → webui/
  Create SZM_Dashboard React app
  ```

### Tier 2: Architecture Enhancement

#### 4. **entt** (Entity Component System)
- **Current**: Custom scene graph + component architecture
- **Enhancement**: Refactor to proper ECS for better data-driven design
- **Benefits**:
  - Improved modularity
  - Better parallel processing
  - Flexible component composition
- **Integration Path**:
  ```
  codes/entt-main/single_include/entt.hpp → external/entt/
  Gradual refactoring: Core → Physics → Graphics
  ```

#### 5. **Babylon.js** (Alternative 3D Engine)
- **Current**: OpenGL/Vulkan native rendering
- **Enhancement**: Optional Babylon.js web rendering
- **Benefits**:
  - Advanced material system (PBR already implemented)
  - Better performance analytics
  - Built-in physics integration option
- **Integration Path**: Secondary option if Three.js insufficient

### Tier 3: AI/ML Enhancement

#### 6. **PyTorch** (Deep Learning)
- **Current**: AI module pending implementation
- **Enhancement**: Python binding for design optimization + failure prediction
- **Benefits**:
  - Neural network-based material property estimation
  - Design space exploration
  - Predictive maintenance
- **Integration Path**:
  ```
  Create Python service: AI_Service/pytorch_models/
  C++ ↔ Python bridge via REST/gRPC
  ```

#### 7. **TensorFlow** (ML Alternative)
- **Current**: None
- **Enhancement**: Alternative ML backend
- **Benefits**: Better for inference, mobile deployment
- **Integration Path**: Post-Phase 6 consideration

### Tier 4: Supporting Libraries

#### 8. **Plotly.py** (Data Visualization)
- **Use Case**: FEA results visualization, thermal mapping
- **Integration**: Python service for advanced plots
- **Path**: `webui/dashboards/`

#### 9. **Node.js** (JavaScript Runtime)
- **Use Case**: Webpack build system, NPM dependencies
- **Integration**: Package management for React frontend
- **Path**: Already built into React workflow

---

## Integration Priority Timeline

| Phase | Libraries | Effort | Impact |
|-------|-----------|--------|--------|
| **6A** | Three.js, FastAPI | Medium | 🔴 HIGH |
| **6B** | React Dashboard | Medium | 🔴 HIGH |
| **6C** | entt ECS | High | 🟡 MEDIUM |
| **7A** | PyTorch AI Service | High | 🟡 MEDIUM |
| **7B** | Babylon.js (if needed) | Medium | 🟢 LOW |

---

## Architecture After Integration

```
SZM_Forge (C++ Core)
├── Desktop App (ImGui + OpenGL/Vulkan)
├── FastAPI Server (REST/WebSocket)
│   ├── Simulation Job Manager
│   ├── Material Database API
│   └── Results Cache
├── Web UI (React)
│   ├── Three.js Viewer
│   ├── Dashboard
│   └── Real-time Updates (WebSocket)
└── AI Service (Python/PyTorch)
    ├── Material Estimator
    ├── Design Optimizer
    └── Failure Predictor
```

---

## Next Steps

1. **Symlink integration** - Link codes folders to external/
2. **Create integration modules** - Bridge code for each library
3. **Update CMakeLists.txt** - Add new build targets
4. **Phase-based rollout** - Start with Three.js → React → PyTorch
5. **Testing framework** - Validate each integration point

---

## Files to Modify/Create

### Modified
- `CMakeLists.txt` - Add three.js, fastapi build targets
- `src/WebViewer/CMakeLists.txt` - Link Three.js
- `src/WebAPI/CMakeLists.txt` - Enhanced API server

### New
- `src/WebViewer/ThreeJSRenderer.hpp/.cpp`
- `src/WebAPI/SimulationServer.hpp/.cpp`
- `webui/package.json` (React app)
- `ai_service/requirements.txt` (PyTorch service)
- `config/integration.json` - Configuration for all services
