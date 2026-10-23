# SZM Forge - Codes Integration Setup

This document outlines the integration of libraries from the `codes` folder into SZM_Forge.

## What's Been Integrated

### 1. **Three.js** (3D Graphics Visualization)
- **Path**: `codes/three.js-dev/ → external/three.js/`
- **Usage**: Enhanced web-based 3D visualization
- **Module**: `src/WebViewer/ThreeJSRenderer`
- **Features**:
  - Real-time physics preview
  - Material/PBR rendering
  - LOD streaming
  - glTF/glb export
  - Interactive scene exploration

### 2. **EnTT** (Entity Component System)
- **Path**: `codes/entt-main/ → external/entt/`
- **Usage**: Modern ECS architecture for data-driven design
- **Module**: `src/AI/ECSArchitecture`
- **Features**:
  - Flexible entity-component composition
  - High-performance queries
  - Event handling
  - Archetype-based storage

### 3. **FastAPI** (Python Web Framework)
- **Path**: `codes/fastapi-master/ → external/fastapi/`
- **Usage**: REST API server for remote simulations
- **Module**: `src/WebAPI/SimulationServer`
- **Features**:
  - Async job queue
  - WebSocket support
  - Material database API
  - Result caching

### 4. **React** (Web Dashboard)
- **Path**: `codes/react-main/ → webui/`
- **Usage**: Modern web UI for remote access
- **Structure**:
  ```
  webui/
  ├── package.json
  ├── vite.config.js
  ├── index.html
  └── src/
      ├── App.jsx
      ├── pages/
      │   ├── Dashboard.jsx
      │   ├── SceneViewer.jsx
      │   ├── MaterialLibrary.jsx
      │   ├── SimulationJobs.jsx
      │   └── Analysis.jsx
  ```

### 5. **Python AI Service** (PyTorch/TensorFlow)
- **Path**: `ai_service/`
- **Usage**: ML-based design optimization and failure prediction
- **Features**:
  - Material property estimation
  - Failure prediction
  - Design space exploration
  - Multi-objective optimization

## Build Configuration

### CMake Options

Enable integrations during build configuration:

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DSZM_BUILD_FULL_DESKTOP=ON \
  -DSZM_ENABLE_THREEJS_INTEGRATION=ON \
  -DSZM_ENABLE_ENTT_ECS=ON \
  -DSZM_ENABLE_FASTAPI_SERVER=ON
```

Optional for web UI:
```bash
  -DSZM_BUILD_WEBUI=ON        # Requires Node.js + npm
  -DSZM_BUILD_AI_SERVICE=ON   # Requires Python 3.8+
```

## Running the Integrated System

### 1. **Desktop Application** (C++ ImGui)
```bash
./build/SZM_Forge
```

### 2. **Simulation REST Server** (C++)
```bash
# Server runs on port 8000
curl http://localhost:8000/api/stats
```

### 3. **Web Dashboard** (React)
```bash
cd webui
npm install
npm run dev          # Development: http://localhost:3000
npm run build        # Production build
```

### 4. **AI Service** (Python/FastAPI)
```bash
cd ai_service
pip install -r requirements.txt
python main.py       # Runs on port 8001
```

## Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│         SZM Forge Core (C++20)                      │
│  ┌────────────────────────────────────────────────┐ │
│  │ Physics | FEA | Materials | Assembly | Thermal │ │
│  └────────────────────────────────────────────────┘ │
└────────────────────────────────────────────────────┬┘
     │
     ├─ Desktop UI (ImGui)
     │   └─ Graphics (OpenGL/Vulkan)
     │
     ├─ FastAPI Server (C++, port 8000)
     │   ├─ REST Endpoints
     │   ├─ Job Queue
     │   └─ WebSocket Updates
     │
     ├─ Web Dashboard (React, port 3000)
     │   ├─ Three.js Viewer
     │   ├─ Job Management
     │   └─ Material Library
     │
     └─ AI Service (Python/PyTorch, port 8001)
         ├─ Design Optimization
         ├─ Failure Prediction
         └─ Material Estimation
```

## Integration Points

### C++ ↔ Web API
- **Port**: 8000 (FastAPI bridge)
- **Protocol**: REST + WebSocket
- **Authentication**: Token-based (TODO)

### C++ ↔ AI Service  
- **Port**: 8001 (Python FastAPI)
- **Protocol**: REST JSON
- **Async**: Job-based (can take minutes)

### Web ↔ Three.js Viewer
- **Format**: glTF 2.0 / JSON
- **Real-time**: WebSocket for updates
- **Performance**: LOD streaming

## File Structure

```
szm_forge/
├── external/
│   ├── three.js/          (symlink to codes/three.js-dev)
│   ├── entt/              (symlink to codes/entt-main)
│   └── fastapi/           (symlink to codes/fastapi-master)
├── src/
│   ├── WebViewer/
│   │   ├── ThreeJSRenderer.hpp/.cpp  (NEW)
│   │   └── ...
│   ├── WebAPI/
│   │   ├── SimulationServer.hpp/.cpp (NEW)
│   │   └── ...
│   ├── AI/
│   │   ├── ECSArchitecture.hpp       (NEW)
│   │   └── ...
│   └── ...
├── webui/                 (NEW React app)
│   ├── src/
│   ├── package.json
│   └── ...
├── ai_service/            (NEW Python service)
│   ├── main.py
│   ├── requirements.txt
│   └── ...
└── CMakeLists.txt (UPDATED)
```

## Next Steps

1. **Build Core C++ with Integrations**
   ```bash
   cmake -B build -DSZM_BUILD_FULL_DESKTOP=ON -DSZM_ENABLE_THREEJS_INTEGRATION=ON
   cmake --build build --parallel
   ```

2. **Start Simulation Server**
   ```bash
   ./build/SZM_Forge  # Desktop + API server starts automatically
   ```

3. **Launch Web Dashboard** (optional)
   ```bash
   cd webui && npm install && npm run dev
   ```

4. **Deploy AI Service** (optional)
   ```bash
   cd ai_service && pip install -r requirements.txt && python main.py
   ```

## Troubleshooting

### Three.js Import Errors
- Ensure `external/three.js/` exists (symlink active)
- Check CMake: `SZM_ENABLE_THREEJS_INTEGRATION=ON`

### EnTT Compilation Issues
- Header-only library - requires C++17 minimum
- Check compiler flags in CMakeLists.txt

### FastAPI Server Won't Start
- Check if port 8000 is already in use
- Review `SimulationServer.cpp` implementation

### React App Build Fails
- Ensure Node.js v16+ installed
- Run `npm install` in webui/ directory
- Check `.env` for API URL configuration

### Python Service Issues
- Create virtual environment: `python -m venv venv`
- Activate: `source venv/bin/activate` (Linux/Mac)
- Install deps: `pip install -r requirements.txt`
- PyTorch may require CUDA setup for GPU support

## Performance Notes

- **Three.js Viewer**: Handles up to ~1M vertices with LOD
- **Job Queue**: Default 4 concurrent simulations
- **Result Cache**: Stores up to 1000 completed jobs
- **AI Service**: GPU acceleration recommended for optimization tasks

## References

- Three.js: https://threejs.org
- EnTT: https://github.com/skypjack/entt
- FastAPI: https://fastapi.tiangolo.com
- React: https://react.dev
- PyTorch: https://pytorch.org
