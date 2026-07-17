# SZM Forge UI Architecture Improvements

## Overview

The UI has been reorganized into a clean, layered architecture with better separation of concerns:

```
UIManagerV2 (Main orchestrator)
├── UIPanelRegistry (Panel lifecycle & organization)
├── UILayoutManager (Workspace layouts)
└── UIThemeManager (Styling & themes)
```

## Components

### 1. UIPanelRegistry
**Purpose**: Centralized panel management with categorization

**Features**:
- Register panels by category (Viewport, Inspector, Simulation, Tools, Debug, Workshop, Analysis)
- Efficient panel lookup and filtering
- Visibility control per panel or category
- State persistence (save/load panel visibility)

**Usage**:
```cpp
auto& registry = UIPanelRegistry::GetInstance();
registry.RegisterPanel("my_panel", panel, PanelCategory::Tools);
registry.SetPanelVisible("my_panel", true);
```

### 2. UILayoutManager
**Purpose**: Workspace organization with preset layouts

**Preset Layouts**:
- **Default**: Balanced layout for general use
- **Modeling**: Optimized for CAD work (large viewport, minimal UI)
- **Simulation**: Optimized for analysis (results panel, playback controls)
- **Assembly**: Optimized for assembly work (tree, properties, builder)
- **Debug**: Maximized console and diagnostics
- **Fullscreen**: Single viewport, minimal UI

**Usage**:
```cpp
auto& layout = UILayoutManager::GetInstance();
layout.ApplyLayout(LayoutPreset::Simulation, dockspaceId);
```

### 3. UIThemeManager
**Purpose**: Consistent UI styling across all panels

**Available Themes**:
- **Dark**: Professional dark theme (default)
- **Light**: Professional light theme
- **HighContrast**: Accessibility-focused
- **Engineering**: Engineering-specific with accent colors

**Usage**:
```cpp
auto& theme = UIThemeManager::GetInstance();
theme.ApplyTheme(UITheme::Engineering);
```

### 4. UIManagerV2
**Purpose**: Main orchestrator integrating all UI systems

**Features**:
- Unified panel registration
- Layout switching via menu
- Theme switching via menu
- Panel visibility toggle via menu
- State persistence

**Usage**:
```cpp
auto& ui = UIManagerV2::GetInstance();
ui.Initialize();
ui.RegisterPanel("console", console_panel, PanelCategory::Debug);
ui.RenderUI();
```

## Panel Categories

Panels are organized into logical categories:

| Category | Purpose | Examples |
|----------|---------|----------|
| **Viewport** | 3D rendering and visualization | Workshop Viewport, Simulation Visualizer |
| **Inspector** | Properties and selection details | Properties Inspector, Selection Properties |
| **Simulation** | Simulation control and results | Simulation Panel, Analysis Results |
| **Tools** | Quick actions and utilities | Toolbar, Component Builder |
| **Debug** | Diagnostics and monitoring | Console, Performance Monitor |
| **Workshop** | Assembly and parts | Scene Outliner, Workshop Library |
| **Analysis** | FEA and results | Analysis Results, Heatmap Legend |

## Migration Guide

### Old Code (UIManager)
```cpp
auto& ui = UIManager::GetInstance();
ui.RegisterPanel(panel);
ui.RenderUI();
```

### New Code (UIManagerV2)
```cpp
auto& ui = UIManagerV2::GetInstance();
ui.Initialize();
ui.RegisterPanel("panel_id", panel, PanelCategory::Tools);
ui.RenderUI();
```

## Menu Structure

### View → Layouts
Switch between preset workspace layouts

### View → Themes
Switch between UI themes

### View → Panels
Toggle individual panel visibility

## State Persistence

The UI automatically saves and loads:
- Panel visibility state
- Theme preference
- Layout configuration (via ImGui ini files)

```cpp
ui.SaveState("./config");
ui.LoadState("./config");
```

## Adding New Panels

1. Create panel class inheriting from `IPanel`
2. Register with category:
```cpp
auto panel = std::make_shared<MyPanel>();
ui.RegisterPanel("my_panel", panel, PanelCategory::Tools);
```
3. Panel automatically appears in View → Panels menu

## Best Practices

1. **Use Categories**: Always specify appropriate category when registering
2. **Unique IDs**: Use unique, descriptive panel IDs
3. **Lazy Initialization**: Initialize panels only when needed
4. **State Management**: Save/load state for user preferences
5. **Theme Awareness**: Use theme colors for consistency

## Files

- `UIManagerV2.hpp/cpp` - Main orchestrator
- `UIPanelRegistry.hpp/cpp` - Panel management
- `UILayoutManager.hpp/cpp` - Layout management
- `UIThemeManager.hpp/cpp` - Theme management
- `IPanel.hpp` - Panel base class (unchanged)

## Future Improvements

- Custom layout editor
- Dockable floating windows
- Panel-specific settings
- Keyboard shortcuts for layout switching
- Theme customization UI
