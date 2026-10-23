# Phase 3: Graphics Integration — Implementation Guide

**Status:** 🔲 **STARTING** — Ready to implement

**Objective:** Integrate graphics rendering with simulation UI panels

**Timeline:** 3 weeks

---

## Phase 3 Overview

Phase 3 focuses on **visual feedback** for the simulation engine:

1. **Render-to-Texture Pipeline** — Render components into ImGui textures
2. **Heatmap Visualization** — Stress/temperature color mapping
3. **Camera Controls** — Orbit/pan/zoom in 3D viewport
4. **Material Database Integration** — Use existing material properties
5. **Assembly Graph Integration** — Multi-component machines

---

## Module 301: Graphics Integration

### Deliverables

#### 3.1 Render Target (Framebuffer Abstraction)
**File:** `src/Graphics/RenderTarget.hpp/cpp`

```cpp
class RenderTarget {
public:
    RenderTarget(uint32_t width, uint32_t height);
    ~RenderTarget();
    
    void Bind();
    void Unbind();
    
    uint32_t GetTexture() const;
    uint32_t GetFramebuffer() const;
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    
    bool IsBound() const { return m_IsBound; }
    
private:
    uint32_t m_Framebuffer = 0;
    uint32_t m_Texture = 0;
    uint32_t m_Renderbuffer = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    bool m_IsBound = false;
};
```

**Features:**
- [x] OpenGL framebuffer creation
- [x] Texture attachment
- [x] Renderbuffer for depth
- [x] Bind/unbind operations
- [x] Error checking

#### 3.2 Component Renderer
**File:** `src/Graphics/ComponentRenderer.hpp/cpp`

```cpp
class ComponentRenderer {
public:
    static void Init();
    static void Shutdown();
    
    static void RenderComponent(
        const SimulationComponent& comp,
        const Camera& camera,
        const Heatmap& heatmap,
        RenderTarget& target
    );
    
    static void RenderAssembly(
        const AssemblyGraph& assembly,
        const Camera& camera,
        const Heatmap& heatmap,
        RenderTarget& target
    );
    
private:
    static void SetupShaders();
    static void RenderCube(const Vector3& pos, const Vector3& scale, const Vector3& color);
    static void RenderSphere(const Vector3& pos, float radius, const Vector3& color);
};
```

**Features:**
- [x] Component mesh rendering
- [x] Assembly rendering
- [x] Shader management
- [x] Color application
- [x] Lighting

#### 3.3 Primitive Shapes
**File:** `src/Graphics/PrimitiveShapes.hpp/cpp`

```cpp
class PrimitiveShapes {
public:
    static uint32_t CreateCube(float size = 1.0f);
    static uint32_t CreateSphere(float radius = 1.0f, uint32_t segments = 32);
    static uint32_t CreateCylinder(float radius = 1.0f, float height = 1.0f, uint32_t segments = 32);
    static uint32_t CreatePlane(float width = 1.0f, float height = 1.0f);
    
    static void RenderMesh(uint32_t meshId);
    static void DeleteMesh(uint32_t meshId);
    
private:
    struct Mesh {
        uint32_t VAO = 0;
        uint32_t VBO = 0;
        uint32_t EBO = 0;
        uint32_t vertexCount = 0;
    };
    
    static std::unordered_map<uint32_t, Mesh> s_Meshes;
};
```

**Features:**
- [x] Cube generation
- [x] Sphere generation (UV sphere)
- [x] Cylinder generation
- [x] Plane generation
- [x] VAO/VBO management

#### 3.4 Heatmap Color Mapping
**File:** `src/Graphics/Heatmap.hpp/cpp`

```cpp
class Heatmap {
public:
    enum class Mode {
        Stress,
        Temperature,
        DangerIndicator,
        None
    };
    
    Heatmap(Mode mode = Mode::Stress);
    
    Vector3 GetColor(float ratio) const;  // ratio: 0-1
    void SetMode(Mode mode) { m_Mode = mode; }
    Mode GetMode() const { return m_Mode; }
    
private:
    Mode m_Mode;
    
    Vector3 GetStressColor(float ratio) const;
    Vector3 GetTemperatureColor(float ratio) const;
    Vector3 GetDangerColor(float ratio) const;
};
```

**Color Schemes:**
```
Stress Heatmap:
  0% (Blue)   → 50% (Yellow) → 100% (Red)
  
Temperature Heatmap:
  20°C (Blue) → 60°C (Purple) → 100°C (Red)
  
Danger Indicator:
  Safe (Green) ↔ Dangerous (Red)
```

#### 3.5 Constraint Renderer
**File:** `src/Graphics/ConstraintRenderer.hpp/cpp`

```cpp
class ConstraintRenderer {
public:
    static void Init();
    static void Shutdown();
    
    static void RenderConstraints(
        const AssemblyGraph& assembly,
        const Camera& camera,
        RenderTarget& target
    );
    
private:
    static void RenderLine(const Vector3& p1, const Vector3& p2, const Vector3& color);
    static void RenderArrow(const Vector3& origin, const Vector3& direction, const Vector3& color);
};
```

**Features:**
- [x] Constraint visualization
- [x] Force vector display
- [x] Connection highlighting
- [x] Mate constraint rendering

---

## Implementation Plan

### Week 1: Render-to-Texture Pipeline

**Day 1-2: RenderTarget Implementation**
1. Create RenderTarget class
2. Implement OpenGL framebuffer creation
3. Add texture attachment
4. Test bind/unbind operations

**Day 3-4: Component Rendering**
1. Create ComponentRenderer class
2. Implement cube/sphere rendering
3. Add shader management
4. Test component rendering

**Day 5: Integration**
1. Hook RenderTarget into SimulationVisualizerPanel
2. Display texture in ImGui
3. Test rendering pipeline

### Week 2: Visualization & Camera

**Day 1-2: Heatmap Implementation**
1. Create Heatmap class
2. Implement color mapping functions
3. Update shaders for heatmap mode
4. Test color transitions

**Day 3-4: Camera Controls**
1. Implement orbit/pan/zoom
2. Integrate InputManager
3. Add mouse input handling
4. Test camera controls

**Day 5: Assembly Integration**
1. Render multi-component assemblies
2. Display component hierarchy
3. Test assembly rendering

### Week 3: Polish & Testing

**Day 1-2: Advanced Features**
1. Constraint visualization
2. Component hover tooltips
3. Performance optimization
4. LOD system integration

**Day 3-4: Testing & Debugging**
1. Unit tests for new modules
2. Integration tests
3. Performance profiling
4. Bug fixes

**Day 5: Documentation**
1. Update README
2. Create user guide
3. Add code examples
4. Final polish

---

## File Structure

### New Files to Create

```
src/Graphics/
├── RenderTarget.hpp/cpp          ← Framebuffer abstraction
├── ComponentRenderer.hpp/cpp     ← Component rendering
├── PrimitiveShapes.hpp/cpp       ← Mesh generation
├── Heatmap.hpp/cpp               ← Color mapping
└── ConstraintRenderer.hpp/cpp    ← Constraint visualization

src/UI/panels/
├── ThermalPanel.hpp/cpp          ← Thermal visualization
├── ElectricalPanel.hpp/cpp       ← Electrical visualization
└── AIPanel.hpp/cpp               ← AI recommendations
```

### Modified Files

```
src/Graphics/
├── GraphicsContext.cpp           ← Add render-to-texture support
├── Camera.cpp                    ← Already implemented
└── PBR.frag                      ← Add heatmap color mode

src/UI/panels/
├── SimulationVisualizerPanel.cpp ← Display texture, handle input
└── ComponentBuilderPanel.cpp     ← Use MaterialDatabase

src/Core/
└── Application.cpp               ← Hook Thermal/Electrical/AI ticks

CMakeLists.txt                    ← Add new modules
```

---

## Technical Details

### Render-to-Texture Pipeline

```cpp
// Create offscreen framebuffer
RenderTarget target(1024, 768);

// Render components
target.Bind();
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

for (const auto& comp : components) {
    ComponentRenderer::RenderComponent(comp, camera, heatmap, target);
}

target.Unbind();

// Display in ImGui
ImGui::Image((void*)(intptr_t)target.GetTexture(), ImVec2(1024, 768));
```

### Heatmap Color Mapping

```cpp
// Stress heatmap: 0% (blue) → 50% (yellow) → 100% (red)
Vector3 GetStressColor(float stressRatio) {
    if (stressRatio < 0.5f) {
        // Blue to yellow
        float t = stressRatio * 2.0f;
        return Vector3(t, t, 1.0f - t);
    } else {
        // Yellow to red
        float t = (stressRatio - 0.5f) * 2.0f;
        return Vector3(1.0f, 1.0f - t, 0.0f);
    }
}
```

### Camera Controls

```cpp
// Orbit: rotate around target
camera.Orbit(deltaX, deltaY, sensitivity);

// Pan: move camera and target
camera.Pan(deltaX, deltaY, sensitivity);

// Zoom: move closer/farther
camera.Zoom(deltaScroll, sensitivity);
```

---

## Testing Strategy

### Unit Tests

```cpp
// RenderTarget tests
TEST(RenderTarget, CreateFramebuffer)
TEST(RenderTarget, BindUnbind)
TEST(RenderTarget, GetTexture)

// Heatmap tests
TEST(Heatmap, StressColorMapping)
TEST(Heatmap, TemperatureColorMapping)
TEST(Heatmap, DangerIndicator)

// ComponentRenderer tests
TEST(ComponentRenderer, RenderCube)
TEST(ComponentRenderer, RenderSphere)
TEST(ComponentRenderer, RenderAssembly)
```

### Integration Tests

```cpp
// Full pipeline tests
TEST(Integration, RenderToTexture)
TEST(Integration, HeatmapVisualization)
TEST(Integration, CameraControls)
TEST(Integration, AssemblyRendering)
```

### Performance Tests

```cpp
// Performance benchmarks
TEST(Performance, RenderTargetCreation)
TEST(Performance, ComponentRenderingSpeed)
TEST(Performance, HeatmapCalculation)
```

---

## Success Criteria

### Phase 3 Complete When:
- ✅ Components render in 3D viewport
- ✅ Stress heatmap displays correctly
- ✅ Temperature heatmap displays correctly
- ✅ Camera orbit/pan/zoom works
- ✅ Material properties update correctly
- ✅ Multi-component assemblies render
- ✅ Constraints visualize correctly
- ✅ All tests passing
- ✅ Performance > 60 FPS

---

## Deliverables Checklist

### Week 1: Render-to-Texture
- [ ] RenderTarget.hpp/cpp created
- [ ] ComponentRenderer.hpp/cpp created
- [ ] PrimitiveShapes.hpp/cpp created
- [ ] Components rendering in viewport
- [ ] Texture displaying in ImGui

### Week 2: Visualization & Camera
- [ ] Heatmap.hpp/cpp created
- [ ] Stress heatmap working
- [ ] Temperature heatmap working
- [ ] Camera controls working
- [ ] Assembly rendering working

### Week 3: Polish & Testing
- [ ] ConstraintRenderer.hpp/cpp created
- [ ] Constraint visualization working
- [ ] All tests passing
- [ ] Performance > 60 FPS
- [ ] Documentation complete

---

## Dependencies

### Existing (Already Available)
- ✅ Graphics::GraphicsContext (OpenGL/Vulkan)
- ✅ Graphics::Camera (orbit/pan/zoom)
- ✅ Graphics::MeshGenerator (LOD)
- ✅ Materials::MaterialDatabase (material properties)
- ✅ Assembly::AssemblyGraph (multi-component machines)
- ✅ Input::InputManager (mouse/keyboard)
- ✅ EventBus (event system)
- ✅ Simulation::SimulationEngine (component data)

### New Dependencies
- 🔲 RenderTarget (framebuffer abstraction)
- 🔲 ComponentRenderer (component rendering)
- 🔲 PrimitiveShapes (mesh generation)
- 🔲 Heatmap (color mapping)
- 🔲 ConstraintRenderer (constraint visualization)

---

## Performance Targets

### Rendering Performance
- **Framebuffer Creation:** < 1ms
- **Component Rendering:** < 5ms per 100 components
- **Heatmap Calculation:** < 1ms
- **Total Frame Time:** < 16ms (60 FPS)

### Memory Usage
- **RenderTarget (1024x768):** ~3 MB
- **Component Meshes:** ~1 MB per 100 components
- **Total Graphics Memory:** ~50 MB

---

## Risk Mitigation

### Potential Issues
1. **Graphics API compatibility** — Use existing GraphicsContext abstraction
2. **Performance bottleneck** — Profile and optimize render pipeline
3. **ImGui integration** — Use existing UIManager framework
4. **Material database coupling** — Use optional material reference

### Mitigation Strategies
1. Start with simple cube rendering
2. Profile early and often
3. Use existing ImGui integration
4. Keep material reference optional

---

## Quick Reference

### Key Classes to Create
- `RenderTarget` — Framebuffer abstraction
- `ComponentRenderer` — Component rendering
- `PrimitiveShapes` — Mesh generation
- `Heatmap` — Color mapping
- `ConstraintRenderer` — Constraint visualization

### Key Functions to Implement
- `RenderTarget::Bind()` — Bind framebuffer
- `ComponentRenderer::RenderComponent()` — Render component
- `Heatmap::GetColor()` — Get stress/temperature color
- `Camera::Orbit()` — Orbit camera
- `ConstraintRenderer::RenderConstraints()` — Render constraints

### Key Events
- `"RenderTargetCreated"` — Framebuffer ready
- `"ComponentRendered"` — Component rendered
- `"HeatmapUpdated"` — Heatmap colors updated
- `"CameraChanged"` — Camera position changed

---

## Summary

**Phase 3 extends SZM Forge with full graphics visualization:**

1. **Render-to-Texture Pipeline** — Offscreen rendering to ImGui textures
2. **Heatmap Visualization** — Stress/temperature color mapping
3. **Camera Controls** — Orbit/pan/zoom in 3D viewport
4. **Material Integration** — Use existing material properties
5. **Assembly Rendering** — Multi-component machine visualization

**Timeline:** 3 weeks
**Complexity:** Medium
**Team Size:** 1-2 developers

**Status:** Ready to start

---

**Last Updated:** Phase 4 Complete
**Next Milestone:** Phase 3 Start
