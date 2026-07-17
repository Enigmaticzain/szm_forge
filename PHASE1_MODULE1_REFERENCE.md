# Phase 1 Module 1 - Quick Reference

## Files Created

### 1. ApplicationModule.hpp (1.4 KB)
```cpp
namespace SZM {
    struct WindowConfig { /* width, height, title, vsync */ };
    class Application { /* singleton, window mgmt, delta-time */ };
}
```
**Key Methods:**
- `Init(config)` - Initialize GLFW and create window
- `Run()` - Main event loop with delta-time calculation
- `Shutdown()` - Clean resource cleanup
- `GetDeltaTime()` - Returns frame delta time in seconds

### 2. ApplicationModule.cpp (2.9 KB)
**Implements:**
- GLFW window creation with DPI awareness
- High-resolution delta-time calculation
- Main loop with FPS diagnostics
- Framebuffer resize callback

### 3. EventBus.hpp (1.1 KB)
```cpp
namespace SZM {
    class EventBus { /* observer pattern, zero-coupling */ };
}
```
**Key Methods:**
- `Subscribe(eventName, callback)` - Register listener
- `Publish(eventName, payload)` - Broadcast event

---

## Integration Points

### CMakeLists.txt Changes
```cmake
# src/Core/CMakeLists.txt
add_library(SZM_Core STATIC
    Application.cpp
    ApplicationModule.cpp  # ← NEW
)
```

### Usage Example
```cpp
#include "Core/ApplicationModule.hpp"
#include "Core/EventBus.hpp"

int main() {
    SZM::WindowConfig config;
    config.Title = "My App";
    config.VSync = true;
    
    SZM::Application& app = SZM::Application::GetInstance();
    app.Init(config);
    app.Run();
    app.Shutdown();
    
    return 0;
}
```

---

## Architecture

```
Application (Singleton)
├── GLFW Window Handle
├── Delta-Time Calculator
├── Main Event Loop
└── Framebuffer Resize Handler

EventBus (Singleton)
├── Event Listeners Map
└── Publish/Subscribe System
```

---

## Dependencies

| Dependency | Version | Purpose |
|-----------|---------|---------|
| GLFW | 3.4 | Window creation & events |
| GLAD | 2.0 | OpenGL function loading |
| C++ | 20 | Modern language features |

---

## Next Module: Input & Event System (010.3)

Will integrate:
- `InputManager` - Keyboard/mouse polling
- `InputEvents` - Event type definitions
- `SpaceMouseDriver` - 6-DOF input support

**Estimated Time:** 1-2 days
