# Phase 1 Module 2 - Quick Reference

## Event System Architecture

```
Event (Base Class)
├── KeyPressedEvent
├── KeyReleasedEvent
├── KeyTypedEvent
├── MouseMovedEvent
├── MouseButtonPressedEvent
├── MouseButtonReleasedEvent
├── MouseScrolledEvent
├── SpaceMouseMovedEvent
├── SpaceMouseButtonEvent
└── WindowLostFocusEvent
```

---

## Usage Examples

### 1. Polling Input State
```cpp
#include "Input/InputManager.hpp"

// Check if key is currently pressed
if (InputManager::IsKeyPressed(GLFW_KEY_W)) {
    camera.MoveForward();
}

// Get mouse position
auto [x, y] = InputManager::GetMousePosition();
```

### 2. Event-Based Input
```cpp
#include "Input/InputEvents.hpp"
#include "Input/Event.hpp"

void OnKeyEvent(SZM::Event& e) {
    SZM::EventDispatcher dispatcher(e);
    
    dispatcher.Dispatch<SZM::KeyPressedEvent>([](SZM::KeyPressedEvent& e) {
        std::cout << "Key pressed: " << e.GetKeyCode() << "\n";
        return true;  // Mark as handled
    });
}
```

### 3. Event Routing (UI Priority)
```cpp
void Application::OnEvent(SZM::Event& e) {
    // UI gets first chance
    if (ui_manager.OnEvent(e)) {
        return;  // Event consumed by UI
    }
    
    // Then 3D viewport
    if (viewport.OnEvent(e)) {
        return;
    }
}
```

### 4. SpaceMouse Input
```cpp
#include "Input/SpaceMouseDriver.hpp"

SpaceMouseDriver::Init();

if (SpaceMouseDriver::IsConnected()) {
    auto state = SpaceMouseDriver::GetState();
    camera.Translate(state.tx, state.ty, state.tz);
    camera.Rotate(state.rx, state.ry, state.rz);
}
```

---

## Event Categories

| Category | Events |
|----------|--------|
| **Keyboard** | KeyPressed, KeyReleased, KeyTyped |
| **Mouse** | MouseMoved, MouseButtonPressed, MouseButtonReleased, MouseScrolled |
| **SpaceMouse** | SpaceMouseMoved, SpaceMouseButton |
| **Window** | WindowLostFocus |

---

## Key Features

### Event Routing
- `Handled` flag prevents duplicate processing
- Type-safe dispatch with templates
- Category-based filtering

### Input Polling
- Direct state checking without events
- 512 key states (covers GLFW_KEY_LAST)
- 8 mouse button states

### Edge Case Handling
- `ClearStateOnFocusLost()` prevents stuck keys on Alt-Tab
- Mouse position tracking
- Button state persistence

### 6-DOF Support
- SpaceMouseDriver for 3Dconnexion devices
- Translation (tx, ty, tz) and rotation (rx, ry, rz)
- Sensitivity scaling (0.0 - 1.0)

---

## Integration Points

### In Application Loop
```cpp
// In Application::Run()
while (!glfwWindowShouldClose(window)) {
    // Update input state from GLFW callbacks
    InputManager::UpdateKeyState(keycode, pressed);
    InputManager::UpdateMousePosition(x, y);
    
    // Create and dispatch events
    KeyPressedEvent event(keycode, repeatCount);
    OnEvent(event);
    
    // Or poll directly
    if (InputManager::IsKeyPressed(GLFW_KEY_W)) {
        // Handle input
    }
}
```

### GLFW Callbacks
```cpp
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        InputManager::UpdateKeyState(key, true);
        KeyPressedEvent event(key, 0);
        app.OnEvent(event);
    } else if (action == GLFW_RELEASE) {
        InputManager::UpdateKeyState(key, false);
        KeyReleasedEvent event(key);
        app.OnEvent(event);
    }
}
```

---

## Files Summary

| File | Size | Purpose |
|------|------|---------|
| Event.hpp | 1.6 KB | Base event class & dispatcher |
| InputEvents.hpp | 6.7 KB | Concrete event implementations |
| InputManager.hpp | 1.1 KB | Input state interface |
| InputManager.cpp | 1.7 KB | Input state implementation |
| SpaceMouseDriver.hpp | 808 B | 6-DOF device interface |
| SpaceMouseDriver.cpp | 1.5 KB | 6-DOF device implementation |

**Total:** ~14 KB of production-quality input handling code

---

## Next Module: Graphics Context (010.4)

Will integrate:
- GraphicsContext (abstract graphics API)
- OpenGLContext (OpenGL 4.5 implementation)
- VulkanContext (Vulkan stub)
- ContextFactory (factory pattern)

**Estimated Time:** 2-3 days
