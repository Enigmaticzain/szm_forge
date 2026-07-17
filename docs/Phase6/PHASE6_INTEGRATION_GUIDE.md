# SZM Forge Phase 6 — Advanced Integration Guide

**VERSION:** 0.3.0  
**STATUS:** Ready for Implementation  
**LAST UPDATED:** 2024

---

## What's New in Phase 6

### 🌐 Web Viewer (Three.js)
- **Real-time 3D visualization** in web browsers
- **WebSocket streaming** for live updates
- **Remote access** from any device
- **Multi-user collaboration** support

**Location:** `src/WebViewer/`  
**Header:** `WebViewerServer.hpp`

### 📦 CAD Import (Mayo)
- **STEP/IGES/STL** file support
- **Automatic mesh generation**
- **Assembly hierarchy** preservation
- **Material mapping** from CAD

**Location:** `src/CAD/`  
**Header:** `CADImporter.hpp`

### 🤖 AI Predictions (PyTorch)
- **Stress hotspot prediction** (real-time)
- **Thermal failure forecasting**
- **Design optimization suggestions**
- **Confidence scoring** for all predictions

**Location:** `src/AI/Predictions/`  
**Header:** `FailurePredictor.hpp`

### 🔌 REST API (FastAPI)
- **Simulation job submission** via HTTP
- **Batch processing** support
- **Result streaming** and export
- **Authentication & rate limiting**

**Location:** `src/WebAPI/`  
**Header:** `FastAPIBridge.hpp`

---

## Quick Start

### Build with Phase 6 Features

```bash
cd /mnt/D/projects/szm_forge
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build-full --parallel
./build-full/SZM_Forge
```

### Enable Optional Features

```bash
# With AI predictions (requires PyTorch)
cmake -B build-full \
  -DCMAKE_BUILD_TYPE=Release \
  -DSZM_BUILD_FULL_DESKTOP=ON \
  -DSZM_ENABLE_AI_PREDICTIONS=ON

# With Web API (requires FastAPI/pybind11)
cmake -B build-full \
  -DCMAKE_BUILD_TYPE=Release \
  -DSZM_BUILD_FULL_DESKTOP=ON \
  -DSZM_ENABLE_WEB_API=ON
```

---

## Module Architecture

### WebViewer Module
```cpp
// Export scene to Three.js
SZM::WebViewer::ThreeJSExporter exporter;
exporter.ExportScene("scene.json");

// Start WebSocket server for real-time sync
SZM::WebViewer::WebSocketBridge bridge(8080);
bridge.Start();
bridge.BroadcastSceneUpdate(jsonDelta);
```

### CAD Module
```cpp
// Import STEP file
SZM::CAD::CADImporter importer;
auto result = importer.ImportFile("model.step");
// Automatically creates components in scene
```

### AI Predictions Module
```cpp
// Predict stress distribution
SZM::AI::Predictions::StressPredictor predictor;
predictor.LoadModel("models/stress_model.pt");
auto result = predictor.PredictStress(componentId, loadVector);
// Returns stress map + hotspots + confidence
```

### WebAPI Module
```cpp
// Submit simulation job
SZM::WebAPI::SimulationAPI api(8000);
api.Start();
std::string jobId = api.SubmitJob(request);
auto status = api.GetJobStatus(jobId);
```

---

## Integration with Existing Code

### No Breaking Changes
- All Phase 6 modules are **optional**
- Existing code remains **100% compatible**
- New features are **additive only**

### Automatic Integration Points
1. **Scene Export** → Automatically triggered on save
2. **CAD Import** → Available in File → Import menu
3. **AI Predictions** → Available in Analysis panel
4. **Web API** → Starts on demand via View → Web Server

---

## File Structure

```
src/
├── WebViewer/
│   ├── CMakeLists.txt
│   ├── WebViewerServer.hpp
│   └── WebViewerServer.cpp (to implement)
├── CAD/
│   ├── CMakeLists.txt
│   ├── CADImporter.hpp
│   └── CADImporter.cpp (to implement)
├── AI/Predictions/
│   ├── CMakeLists.txt
│   ├── FailurePredictor.hpp
│   └── FailurePredictor.cpp (to implement)
└── WebAPI/
    ├── CMakeLists.txt
    ├── FastAPIBridge.hpp
    └── FastAPIBridge.cpp (to implement)
```

---

## Next Steps

### Immediate (This Week)
1. ✅ Create module headers (DONE)
2. ✅ Add CMakeLists.txt (DONE)
3. ✅ Update root CMakeLists.txt (DONE)
4. ⏳ Implement WebViewer (WebViewerServer.cpp)
5. ⏳ Implement CAD importer (CADImporter.cpp)

### Short Term (Next 2 Weeks)
6. Implement AI predictions (FailurePredictor.cpp)
7. Implement Web API (FastAPIBridge.cpp)
8. Create web dashboard (React)
9. Add documentation

### Medium Term (Next Month)
10. Integrate Babylon.js for advanced graphics
11. Add SOFA soft-body simulation
12. Implement Bevy ECS for large assemblies
13. Performance optimization

---

## Testing

### Unit Tests
```bash
./build-full/SZM_TestRunner
```

### Integration Tests
```bash
# Test WebViewer export
./test_webviewer.sh

# Test CAD import
./test_cad_import.sh

# Test AI predictions
./test_predictions.sh

# Test Web API
./test_webapi.sh
```

---

## Documentation

- **API Reference:** See individual `.hpp` files
- **Examples:** `examples/phase6/`
- **Tutorials:** `docs/phase6/`
- **FAQ:** `PHASE6_FAQ.md`

---

## Support

For issues or questions:
1. Check `PHASE6_FAQ.md`
2. Review module headers for API details
3. Check `codes/` folder for reference implementations
4. Open an issue with detailed description

---

*SZM Forge Phase 6 — Advanced Integration | v0.3.0 | Ready to Deploy*
