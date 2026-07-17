# Phase 3: Graphics Integration & Advanced Visualization

**Status:** 🔲 **PENDING** — Ready to start

**Objective:** Integrate graphics rendering with simulation UI panels

---

## Phase 3 Overview

Phase 3 focuses on **visual feedback** for the simulation engine:

1. **Graphics Integration** — Render components into ImGui textures
2. **Heatmap Visualization** — Stress/temperature color mapping
3. **Camera Controls** — Orbit/pan/zoom in 3D viewport
4. **Material Database Integration** — Use existing material properties
5. **Assembly Graph Integration** — Multi-component machines

---

## Deliverables

### 3.1 Graphics Integration (Week 1)

#### 3.1.1 Render-to-Texture Pipeline
- Create offscreen framebuffer for ImGui texture
- Render components as 3D geometry
- Update texture each frame
- Display in SimulationVisualizerPanel

**Files to Create:**
- `src/Graphics/RenderTarget.hpp/cpp` — Framebuffer abstraction
- `src/Graphics/ComponentRenderer.hpp/cpp` — Component mesh generation

**Integration Points:**
- `src/UI/panels/SimulationVisualizerPanel.cpp` — Display texture
- `src/Graphics/GraphicsContext.cpp` — Render components

#### 3.1.2 Component Mesh Generation
- Create cube/cylinder/sphere meshes for components
- Apply material properties (color, roughness)
- Update mesh based on component state

**Files to Create:**
- `src/Graphics/PrimitiveShapes.hpp/cpp` — Cube, cylinder, sphere generators

**Integration Points:**
- `src/Graphics/MeshGenerator.cpp` — Use existing LOD system
- `src/UI/panels/SimulationVisualizerPanel.cpp` — Render components

### 3.2 Heatmap Visualization (Week 1-2)

#### 3.2.1 Color Mapping
- Implement stress heatmap (blue→yellow→red)
- Implement temperature heatmap (blue→purple→red)
- Implement danger indicator (green/red)

**Files to Create:**
- `src/Graphics/Heatmap.hpp/cpp` — Color mapping functions

**Integration Points:**
- `src/UI/panels/SimulationVisualizerPanel.cpp` — Apply colors
- `src/Graphics/ComponentRenderer.cpp` — Use heatmap colors

#### 3.2.2 Shader Updates
- Update PBR shader to support heatmap colors
- Add color override mode
- Implement smooth color transitions

**Files to Modify:**
- `assets/shaders/PBR.frag` — Add heatmap color mode

### 3.3 Camera Controls (Week 1)

#### 3.3.1 Viewport Camera
- Implement orbit camera (already in Camera.hpp)
- Add pan and zoom controls
- Integrate with InputManager

**Files to Modify:**
- `src/Graphics/Camera.cpp` — Implement orbit/pan/zoom
- `src/UI/panels/SimulationVisualizerPanel.cpp` — Handle mouse input

#### 3.3.2 Input Handling
- Mouse drag for orbit
- Mouse wheel for zoom
- Right-click for pan

**Integration Points:**
- `src/Input/InputManager.cpp` — Capture mouse input
- `src/UI/panels/SimulationVisualizerPanel.cpp` — Process input

### 3.4 Material Database Integration (Week 2)

#### 3.4.1 Material Properties
- Load material properties from MaterialDatabase
- Use yield strength, thermal conductivity, density
- Update component properties on material change

**Files to Modify:**
- `src/UI/panels/ComponentBuilderPanel.cpp` — Use MaterialDatabase
- `src/Simulation/SimulationComponent.hpp` — Add material reference

#### 3.4.2 Material Presets
- Extend ComponentBuilderPanel with more materials
- Add custom material creation
- Save/load material presets

**Integration Points:**
- `src/Materials/MaterialDatabase.cpp` — Query materials
- `src/UI/panels/ComponentBuilderPanel.cpp` — Display materials

### 3.5 Assembly Graph Integration (Week 2-3)

#### 3.5.1 Multi-Component Machines
- Create assemblies from components
- Define constraints between components
- Visualize assembly hierarchy

**Files to Create:**
- `src/Simulation/SimulationAssembly.hpp/cpp` — Assembly management

**Integration Points:**
- `src/Assembly/AssemblyGraph.cpp` — Use existing assembly system
- `src/UI/panels/SimulationPanel.cpp` — Display assembly tree

#### 3.5.2 Constraint Visualization
- Show constraints in 3D viewport
- Highlight connected components
- Display constraint forces

**Files to Create:**
- `src/Graphics/ConstraintRenderer.hpp/cpp` — Render constraints

**Integration Points:**
- `src/UI/panels/SimulationVisualizerPanel.cpp` — Render constraints
- `src/Assembly/KinematicSolver.cpp` — Get constraint data

---

## Implementation Plan

### Week 1: Graphics Foundation

**Day 1-2: Render-to-Texture**
1. Create RenderTarget abstraction
2. Implement offscreen framebuffer
3. Test texture rendering in ImGui

**Day 3-4: Component Rendering**
1. Create PrimitiveShapes generators
2. Implement ComponentRenderer
3. Render components in viewport

**Day 5: Camera & Input**
1. Implement orbit/pan/zoom
2. Integrate InputManager
3. Test camera controls

### Week 2: Visualization & Integration

**Day 1-2: Heatmaps**
1. Implement color mapping functions
2. Update shaders for heatmap mode
3. Test stress/temperature visualization

**Day 3-4: Material Database**
1. Integrate MaterialDatabase
2. Update ComponentBuilderPanel
3. Test material presets

**Day 5: Assembly Integration**
1. Create SimulationAssembly
2. Integrate with AssemblyGraph
3. Test multi-component machines

### Week 3: Polish & Testing

**Day 1-2: Advanced Features**
1. Constraint visualization
2. Component hover tooltips
3. Performance optimization

**Day 3-4: Testing & Debugging**
1. Unit tests for new modules
2. Integration tests
3. Performance profiling

**Day 5: Documentation**
1. Update README
2. Create user guide
3. Add code examples

---

## Technical Details

### Render-to-Texture Pipeline

```cpp
// Create offscreen framebuffer
RenderTarget target(1024, 768);

// Render components
target.Bind();
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
for (const auto& comp : components) {
    ComponentRenderer::Render(comp, camera, heatmap);
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

### Material Integration

```cpp
// Load material from database
auto material = MaterialDatabase::GetInstance().GetMaterial("MAT-STEEL-STRUCT");
if (material) {
    component->yieldStrength = material->youngsModulus;
    component->density = material->density;
    component->thermalConductivity = material->thermalConductivity;
}
```

---

## File Structure

### New Files
```
src/Graphics/
├── RenderTarget.hpp/cpp          ← Framebuffer abstraction
├── ComponentRenderer.hpp/cpp     ← Component rendering
├── PrimitiveShapes.hpp/cpp       ← Cube, cylinder, sphere
├── Heatmap.hpp/cpp               ← Color mapping
└── ConstraintRenderer.hpp/cpp    ← Constraint visualization

src/Simulation/
└── SimulationAssembly.hpp/cpp    ← Assembly management
```

### Modified Files
```
src/Graphics/
├── GraphicsContext.cpp           ← Add render-to-texture support
├── Camera.cpp                    ← Implement orbit/pan/zoom
└── PBR.frag                      ← Add heatmap color mode

src/UI/panels/
├── SimulationVisualizerPanel.cpp ← Display texture, handle input
└── ComponentBuilderPanel.cpp     ← Use MaterialDatabase

src/Simulation/
└── SimulationComponent.hpp       ← Add material reference

src/Assembly/
└── AssemblyGraph.cpp             ← Integration points

CMakeLists.txt                    ← Add new modules
```

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

### New Dependencies
- 🔲 RenderTarget (framebuffer abstraction)
- 🔲 ComponentRenderer (component rendering)
- 🔲 PrimitiveShapes (mesh generation)
- 🔲 Heatmap (color mapping)
- 🔲 ConstraintRenderer (constraint visualization)

---

## Testing Strategy

### Unit Tests
```cpp
// Test heatmap color mapping
TEST(Heatmap, StressColorMapping) {
    Vector3 blue = GetStressColor(0.0f);
    Vector3 yellow = GetStressColor(0.5f);
    Vector3 red = GetStressColor(1.0f);
    EXPECT_EQ(blue, Vector3(0, 0, 1));
    EXPECT_EQ(red, Vector3(1, 0, 0));
}

// Test render target
TEST(RenderTarget, CreateAndBind) {
    RenderTarget target(1024, 768);
    target.Bind();
    EXPECT_TRUE(target.IsBound());
    target.Unbind();
}
```

### Integration Tests
```cpp
// Test component rendering
TEST(ComponentRenderer, RenderComponent) {
    SimulationComponent comp;
    comp.stress = 100e6f;
    comp.yieldStrength = 250e6f;
    
    RenderTarget target(1024, 768);
    target.Bind();
    ComponentRenderer::Render(comp, camera, heatmap);
    target.Unbind();
    
    EXPECT_TRUE(target.GetTexture() != 0);
}
```

### Visual Tests
1. Render component in viewport
2. Verify stress heatmap colors
3. Verify temperature heatmap colors
4. Test camera orbit/pan/zoom
5. Test material property updates

---

## Performance Considerations

### Optimization Strategies
1. **Frustum culling** — Only render visible components
2. **LOD system** — Use existing MeshGenerator LOD
3. **Batch rendering** — Group components by material
4. **Texture caching** — Cache component meshes
5. **Async rendering** — Render in background thread

### Target Performance
- 60 FPS at 1024x768 with 100 components
- < 5ms per frame for rendering
- < 2ms per frame for physics update

---

## Success Criteria

### Phase 3 Complete When:
- ✅ Components render in 3D viewport
- ✅ Stress heatmap displays correctly
- ✅ Temperature heatmap displays correctly
- ✅ Camera orbit/pan/zoom works
- ✅ Material properties update correctly
- ✅ Multi-component assemblies render
- ✅ All tests passing
- ✅ Performance > 60 FPS

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

## Deliverables Checklist

### Week 1
- [ ] RenderTarget implementation
- [ ] ComponentRenderer implementation
- [ ] PrimitiveShapes implementation
- [ ] Camera controls working
- [ ] Components rendering in viewport

### Week 2
- [ ] Heatmap color mapping
- [ ] Shader updates
- [ ] Material database integration
- [ ] Assembly integration
- [ ] Multi-component rendering

### Week 3
- [ ] Constraint visualization
- [ ] Performance optimization
- [ ] All tests passing
- [ ] Documentation complete
- [ ] Phase 3 complete

---

## Next Phase (Phase 4)

After Phase 3 is complete:

1. **Thermal Engine** (Module 300)
   - Full heat transfer simulation
   - Conduction, convection, radiation
   - Transient analysis

2. **Electrical Engine** (Module 400)
   - Circuit simulation
   - Power distribution
   - Fault detection

3. **AI Assistant** (Module 500)
   - Design optimization
   - Failure prediction
   - Recommendations

---

## Quick Reference

### Key Classes
- `RenderTarget` — Framebuffer abstraction
- `ComponentRenderer` — Component rendering
- `PrimitiveShapes` — Mesh generation
- `Heatmap` — Color mapping
- `SimulationAssembly` — Assembly management

### Key Functions
- `RenderTarget::Bind()` — Bind framebuffer
- `ComponentRenderer::Render()` — Render component
- `Heatmap::GetStressColor()` — Get stress color
- `Camera::Orbit()` — Orbit camera
- `MaterialDatabase::GetMaterial()` — Get material

### Key Events
- `"SimulationTick"` — Physics update
- `"ComponentAdded"` — Component added
- `"ComponentRemoved"` — Component removed
- `"MaterialChanged"` — Material changed

---

**Status:** Ready to start Phase 3
**Estimated Duration:** 3 weeks
**Team Size:** 1-2 developers
**Complexity:** Medium

---

**Last Updated:** Phase 2 Complete
**Next Milestone:** Phase 3 Start
