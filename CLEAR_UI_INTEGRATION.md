# Clear UI Integration Guide

## Quick Start

### 1. Initialize in main_desktop.cpp

```cpp
#include "UI/ClearUIManager.hpp"

int main() {
    // ... existing setup ...
    
    auto& ui = SZM::UI::ClearUIManager::GetInstance();
    ui.Initialize();
    
    // ... main loop ...
}
```

### 2. Render in Application Loop

```cpp
void Application::TickSystems() {
    // ... other systems ...
    
    auto& ui = SZM::UI::ClearUIManager::GetInstance();
    ui.RenderUI();
}
```

### 3. Add Sections to Tabs

```cpp
// Example: Add to Training tab
auto trainingSection = std::make_shared<SZM::UI::UISection>("Data Import", "📁");
trainingSection->SetContent([]() {
    ImGui::Text("Import training data");
    if (ImGui::Button("Browse")) {
        // Handle import
    }
});
ui.AddSectionToTab(SZM::UI::TabType::Training, trainingSection);
```

## Tab Structure

### Dashboard Tab
```cpp
// System Status
auto statusSection = std::make_shared<UISection>("System Status", "📊");
statusSection->SetContent([]() {
    ImGui::Text("FPS: 60");
    ImGui::Text("Memory: 2.5GB / 8GB");
});
ui.AddSectionToTab(TabType::Dashboard, statusSection);
```

### Modeling Tab
```cpp
// Part Library
auto librarySection = std::make_shared<UISection>("Part Library", "📚");
librarySection->SetContent([]() {
    ImGui::Text("Available Parts:");
    ImGui::BulletText("Beam");
    ImGui::BulletText("Bracket");
});
ui.AddSectionToTab(TabType::Modeling, librarySection);
```

### Assembly Tab
```cpp
// Assembly Tree
auto treeSection = std::make_shared<UISection>("Assembly Tree", "🌳");
treeSection->SetContent([]() {
    if (ImGui::TreeNode("Assembly")) {
        ImGui::TreePop();
    }
});
ui.AddSectionToTab(TabType::Assembly, treeSection);
```

### Simulation Tab
```cpp
// Simulation Control
auto controlSection = std::make_shared<UISection>("Simulation Control", "▶️");
controlSection->SetContent([]() {
    if (ImGui::Button("Start")) { /* ... */ }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) { /* ... */ }
});
ui.AddSectionToTab(TabType::Simulation, controlSection);
```

### Training Tab
```cpp
// Already implemented in TrainingTabUI
auto trainingUI = std::make_shared<TrainingTabUI>();
// Register as panel if needed
```

### Results Tab
```cpp
// Analysis Results
auto resultsSection = std::make_shared<UISection>("FEA Results", "📈");
resultsSection->SetContent([]() {
    ImGui::Text("Max Stress: 250 MPa");
    ImGui::Text("Max Temperature: 85°C");
});
ui.AddSectionToTab(TabType::Results, resultsSection);
```

### Settings Tab
```cpp
// Preferences
auto prefsSection = std::make_shared<UISection>("Preferences", "⚙️");
prefsSection->SetContent([]() {
    static bool darkMode = true;
    ImGui::Checkbox("Dark Mode", &darkMode);
});
ui.AddSectionToTab(TabType::Settings, prefsSection);
```

## Migration Checklist

- [ ] Add ClearUIManager to main_desktop.cpp
- [ ] Initialize in Application::Init()
- [ ] Call RenderUI() in Application::TickSystems()
- [ ] Create sections for Dashboard tab
- [ ] Create sections for Modeling tab
- [ ] Create sections for Assembly tab
- [ ] Create sections for Simulation tab
- [ ] Create sections for Training tab
- [ ] Create sections for Results tab
- [ ] Create sections for Settings tab
- [ ] Test tab switching
- [ ] Test section collapsing
- [ ] Verify styling consistency

## Code Example: Complete Setup

```cpp
#include "UI/ClearUIManager.hpp"

class Application {
public:
    void Init() {
        // ... existing init ...
        
        auto& ui = SZM::UI::ClearUIManager::GetInstance();
        ui.Initialize();
        
        SetupUITabs();
    }
    
    void SetupUITabs() {
        auto& ui = SZM::UI::ClearUIManager::GetInstance();
        
        // Dashboard
        auto dashSection = std::make_shared<UISection>("Overview");
        dashSection->SetContent([]() {
            ImGui::Text("System Status: OK");
        });
        ui.AddSectionToTab(TabType::Dashboard, dashSection);
        
        // Modeling
        auto modelSection = std::make_shared<UISection>("Tools");
        modelSection->SetContent([]() {
            if (ImGui::Button("Create Part")) { /* ... */ }
        });
        ui.AddSectionToTab(TabType::Modeling, modelSection);
        
        // ... more tabs ...
    }
    
    void TickSystems() {
        auto& ui = SZM::UI::ClearUIManager::GetInstance();
        ui.RenderUI();
    }
};
```

## Styling Customization

The ClearUIManager uses professional dark theme by default. To customize:

```cpp
ImGuiStyle& style = ImGui::GetStyle();
style.WindowRounding = 4.0f;
style.FrameRounding = 3.0f;
style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.35f, 0.50f, 1.0f);
```

## Keyboard Shortcuts (Future)

- `Ctrl+1` - Dashboard
- `Ctrl+2` - Modeling
- `Ctrl+3` - Assembly
- `Ctrl+4` - Simulation
- `Ctrl+5` - Training
- `Ctrl+6` - Results
- `Ctrl+7` - Settings

## Performance Tips

1. Use lambda captures efficiently
2. Avoid heavy computations in Draw()
3. Cache frequently accessed data
4. Use ImGui::BeginDisabled() for inactive sections
5. Profile with ImGui metrics window

## Troubleshooting

**Tabs not showing?**
- Ensure Initialize() is called
- Check that RenderUI() is called every frame

**Sections not appearing?**
- Verify AddSectionToTab() is called with correct TabType
- Check that SetContent() lambda is valid

**Styling looks wrong?**
- Verify ImGui context is initialized
- Check style settings in Initialize()

## Support

For issues or questions, refer to:
- `CLEAR_UI_SYSTEM.md` - Full documentation
- `ClearUIManager.hpp` - API reference
- `panels/TrainingTabUI.cpp` - Example implementation
