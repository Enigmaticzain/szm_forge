# SZM Forge — Auto-Updating Desktop Build

## Quick Setup

Your desktop icon is now configured to **auto-update** whenever you add new features.

### How It Works

1. **Desktop Icon** → Launches `launch-auto.sh`
2. **Smart Launcher** → Detects source file changes
3. **Auto-Rebuild** → If changes detected, rebuilds incrementally
4. **Launch** → Starts the app with all new features

---

## Initial Build (One-Time)

The `build-full` directory already exists. To ensure it's fully built:

```bash
cd /mnt/D/projects/szm_forge
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build-full --parallel $(nproc)
```

---

## Desktop Icon Location

- **File**: `~/.local/share/applications/szm-forge.desktop`
- **Launcher**: `/mnt/D/projects/szm_forge/launch-auto.sh`
- **Executable**: `/mnt/D/projects/szm_forge/build-full/SZM_Forge`

---

## Adding New Features

### 1. Create New Module

```bash
mkdir -p src/MyNewModule
cat > src/MyNewModule/CMakeLists.txt << 'EOF'
add_library(SZM_MyNewModule_Module
    MyNewModule.cpp
)
target_include_directories(SZM_MyNewModule_Module PUBLIC .)
EOF
```

### 2. Add Source Files

```bash
cat > src/MyNewModule/MyNewModule.hpp << 'EOF'
namespace SZM {
    class MyNewFeature {
    public:
        void DoSomething();
    };
}
EOF

cat > src/MyNewModule/MyNewModule.cpp << 'EOF'
#include "MyNewModule.hpp"
namespace SZM {
    void MyNewFeature::DoSomething() {
        // Implementation
    }
}
EOF
```

### 3. Link to Main App

Edit `src/main_desktop.cpp` to use your new module:

```cpp
#include "MyNewModule/MyNewModule.hpp"

// In Application::TickSystems() or similar:
myFeature.DoSomething();
```

### 4. Update Root CMakeLists.txt

Add to `/mnt/D/projects/szm_forge/CMakeLists.txt`:

```cmake
add_subdirectory(src/MyNewModule)
```

And link to the main executable:

```cmake
target_link_libraries(SZM_Forge PRIVATE
    # ... existing targets ...
    SZM_MyNewModule_Module
)
```

### 5. Launch and Auto-Update

Just click the desktop icon. The launcher will:
- Detect your new files
- Rebuild automatically
- Launch with your new features

---

## Manual Rebuild (if needed)

```bash
cd /mnt/D/projects/szm_forge/build-full
cmake --build . --parallel $(nproc)
```

---

## Troubleshooting

### Build Fails

Check the build log:
```bash
tail -100 /mnt/D/projects/szm_forge/build-full/CMakeFiles/CMakeOutput.log
```

### Executable Not Found

Ensure initial build completed:
```bash
ls -lh /mnt/D/projects/szm_forge/build-full/SZM_Forge
```

### Changes Not Detected

The launcher checks for `.cpp` and `.hpp` file modifications. If you only changed headers, touch a source file:
```bash
touch /mnt/D/projects/szm_forge/src/main_desktop.cpp
```

---

## Architecture

```
Desktop Icon (szm-forge.desktop)
    ↓
launch-auto.sh (Smart Launcher)
    ↓
    ├─ Check for source changes
    ├─ If changed: cmake --build build-full
    └─ Launch: ./build-full/SZM_Forge
```

---

## Files Created

- `launch-auto.sh` — Smart launcher with auto-rebuild
- `szm-forge.desktop` — Desktop icon entry
- `CMakeLists_AutoBuild.txt` — Auto-discovery CMake config (optional)
- `build.sh` — Full build script with module scanning (optional)

---

## Next Steps

1. **Click the desktop icon** to launch SZM Forge
2. **Add new modules** to `src/`
3. **Edit CMakeLists.txt** to link them
4. **Click the icon again** — auto-rebuild happens automatically

Enjoy building! 🚀
