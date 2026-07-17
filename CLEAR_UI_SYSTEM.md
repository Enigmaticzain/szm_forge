# Clear UI System - SZM Forge

## Overview

A professional, clean tab-based UI system that organizes the entire application into logical tabs with organized sections.

## Architecture

```
ClearUIManager (Main orchestrator)
├── UITab (Dashboard, Modeling, Assembly, Simulation, Training, Results, Settings)
│   └── UISection (Organized content areas)
│       └── Content (Rendered via lambda functions)
```

## Main Tabs

| Tab | Purpose | Sections |
|-----|---------|----------|
| **Dashboard** | Overview and quick stats | System Status, Quick Actions, Recent Activity |
| **Modeling** | CAD and geometry work | Part Library, Geometry Tools, Properties |
| **Assembly** | Assembly and kinematics | Assembly Tree, Mates, Constraints |
| **Simulation** | Simulation and analysis | Simulation Control, Results, Visualization |
| **Training** | AI training studio | Data Import, Training Control, Metrics |
| **Results** | Analysis results | FEA Results, Thermal Analysis, Reports |
| **Settings** | Application settings | Preferences, Themes, Advanced |

## Usage

### Initialize
```cpp
auto& ui = SZM::UI::ClearUIManager::GetInstance();
ui.Initialize();
```

### Add Section to Tab
```cpp
auto section = std::make_shared<SZM::UI::UISection>("Data Import", "📁");
section->SetContent([]() {
    ImGui::Text("Import your training data here");
    if (ImGui::Button("Browse")) {
        // Handle import
    }
});
ui.AddSectionToTab(SZM::UI::TabType::Training, section);
```

### Render
```cpp
ui.RenderUI();
```

## UI Components

### UISection
Represents a collapsible section within a tab.

```cpp
UISection section("Title", "Icon");
section.SetContent([]() {
    // Your UI code here
});
section.Draw();
```

### UITab
Represents a main application tab containing multiple sections.

```cpp
auto tab = std::make_shared<UITab>(TabType::Training, "Training", "🧠");
tab->AddSection(section);
tab->Draw();
```

## Design Principles

1. **Clear Organization** - Each tab has a specific purpose
2. **Logical Sections** - Related controls grouped together
3. **Consistent Styling** - Professional dark theme with accent colors
4. **Easy Navigation** - Tab bar at top for quick switching
5. **Collapsible Sections** - Hide/show sections as needed
6. **Responsive Layout** - Adapts to window size

## Example: Training Tab

The Training tab demonstrates the clear UI pattern:

```
┌─ Training Studio ─────────────────────────────────┐
├─ 📁 Data Import ──────────────────────────────────┤
│  Import Source: [Browse Files]                    │
│  Data Type: [GitHub ▼]                            │
│  [Import]                                         │
├─ ▶️ Training Control ─────────────────────────────┤
│  Training Mode: [Passive Learning ▼]              │
│  Status: IDLE                                     │
│  Progress: ████░░░░░░ 40%                         │
│  [Start] [Pause] [Stop]                           │
├─ ⚙️ Configuration ────────────────────────────────┤
│  Chunk Size: [512 ────────]                       │
│  Context Size: [2048 ────────]                    │
│  GPU Usage: [0.8 ────────]                        │
│  [Save Configuration]                             │
├─ 📊 Metrics ─────────────────────────────────────┤
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ │
│  │Accuracy │ │  Loss   │ │ Tokens  │ │Learning │ │
│  │  85.2%  │ │  0.152  │ │  1.2M   │ │  0.001  │ │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘ │
├─ 💾 Checkpoints ─────────────────────────────────┤
│  Name        │ Accuracy │ Date       │ Action    │
│  Checkpoint1 │  85.2%   │ 2024-01-15 │ [Load]    │
│  Checkpoint2 │  82.1%   │ 2024-01-14 │ [Load]    │
│  [Save Checkpoint]                                │
└───────────────────────────────────────────────────┘
```

## Styling

### Colors
- **Active Tab**: Blue (#4080FF)
- **Inactive Tab**: Dark Gray (#1F1F28)
- **Accent**: Light Blue (#64B6FF)
- **Success**: Green (#50FF96)
- **Warning**: Yellow (#FFD400)
- **Error**: Red (#FF5050)

### Spacing
- Window Padding: 12px
- Frame Padding: 8px, 6px
- Item Spacing: 8px
- Section Spacing: 12px

## Creating New Tabs

1. Add to `TabType` enum
2. Create tab in `SetupDefaultTabs()`
3. Add sections with `AddSectionToTab()`

```cpp
// In SetupDefaultTabs()
CreateTab(TabType::MyTab, "My Tab", "🎯");

// Later, add sections
auto section = std::make_shared<UISection>("My Section");
section->SetContent([]() { /* UI code */ });
ui.AddSectionToTab(TabType::MyTab, section);
```

## Files

- `ClearUIManager.hpp/cpp` - Main UI orchestrator
- `panels/TrainingTabUI.hpp/cpp` - Training tab example
- `UI_CLEAR_SYSTEM.md` - This documentation

## Benefits

✓ **Professional Appearance** - Clean, organized interface
✓ **Easy to Navigate** - Clear tab structure
✓ **Extensible** - Easy to add new tabs and sections
✓ **Consistent** - Unified styling across app
✓ **User-Friendly** - Intuitive layout
✓ **Maintainable** - Clear separation of concerns

## Next Steps

1. Apply ClearUIManager to main_desktop.cpp
2. Create tabs for all major features
3. Migrate existing panels to new tab structure
4. Add keyboard shortcuts for tab switching
5. Implement tab persistence
