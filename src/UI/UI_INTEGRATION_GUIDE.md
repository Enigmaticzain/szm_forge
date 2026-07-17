# SZM Forge Modern UI Integration Guide

## Quick Start

### 1. Apply Modern Theme
In your application initialization (e.g., `Application::Initialize()`):

```cpp
#include "UI/UIStyleHelper.hpp"

void Application::Initialize() {
    // ... other initialization ...
    UIStyleHelper::ApplyModernTheme();
}
```

### 2. Register Modern Panels
In `UIManager::Initialize()` or your panel registration code:

```cpp
#include "UI/panels/ModernDashboardPanel.hpp"
#include "UI/panels/ModernConsolePanel.hpp"
#include "UI/panels/ModernPropertiesPanel.hpp"
#include "UI/panels/ModernSimulationPanel.hpp"

UIManager& uiMgr = UIManager::GetInstance();
uiMgr.RegisterPanel(std::make_shared<ModernDashboardPanel>());
uiMgr.RegisterPanel(std::make_shared<ModernConsolePanel>());
uiMgr.RegisterPanel(std::make_shared<ModernPropertiesPanel>());
uiMgr.RegisterPanel(std::make_shared<ModernSimulationPanel>());
```

## Creating New Modern Panels

### Template
```cpp
#pragma once
#include "UI/ModernPanelBase.hpp"

class MyModernPanel : public ModernPanelBase {
public:
    const char* GetName() const override { return "My Panel"; }
    std::string GetTitle() const override { return "My Panel Title"; }
    const char* GetIcon() const override { return "📌"; }

protected:
    void DrawContent() override {
        // Your panel content here
    }
    
    void DrawHeader() override {
        ImGui::TextUnformatted("Custom Header");
    }
};
```

### Using Style Helpers
```cpp
void DrawContent() override {
    // Status badge
    UIStyleHelper::StatusBadge("active");
    
    // Buttons
    if (UIStyleHelper::ModernButtonPrimary("Action")) {
        // Handle click
    }
    
    // Progress bar
    UIStyleHelper::ProgressBar(0.75f, ImVec2(200, 0), "75%");
    
    // Separator
    UIStyleHelper::HorizontalSeparator();
    
    // Spacing
    UIStyleHelper::Spacing(2);
    
    // Card
    BeginCard();
    ImGui::TextUnformatted("Card content");
    EndCard();
}
```

## Color Usage

### Semantic Colors
```cpp
// Get color for status
ImU32 color = UIStyleHelper::GetStatusColor("active");  // Emerald
color = UIStyleHelper::GetStatusColor("error");         // Red
color = UIStyleHelper::GetStatusColor("warning");       // Amber
color = UIStyleHelper::GetStatusColor("processing");    // Blue
```

### Direct Color Access
```cpp
ImU32 violet = UIStyleHelper::Colors::Violet500;
ImU32 emerald = UIStyleHelper::Colors::Emerald500;
ImU32 slate = UIStyleHelper::Colors::Slate200;

ImGui::PushStyleColor(ImGuiCol_Button, violet);
ImGui::Button("Styled Button");
ImGui::PopStyleColor();
```

## Styling Existing Panels

### Before (Old Style)
```cpp
class OldPanel : public IPanel {
    void Draw() override {
        ImGui::Begin("Old Panel");
        ImGui::Text("Content");
        ImGui::End();
    }
};
```

### After (Modern Style)
```cpp
class NewPanel : public ModernPanelBase {
    const char* GetName() const override { return "New Panel"; }
    
    void DrawContent() override {
        ImGui::Text("Content");
    }
};
```

## Layout Patterns

### Two-Column Layout
```cpp
void DrawContent() override {
    ImGui::Columns(2, nullptr, true);
    
    // Left column
    ImGui::TextUnformatted("Left");
    ImGui::NextColumn();
    
    // Right column
    ImGui::TextUnformatted("Right");
    ImGui::Columns(1);
}
```

### Card Grid
```cpp
void DrawContent() override {
    ImGui::Columns(3, nullptr, false);
    
    for (int i = 0; i < 9; ++i) {
        BeginCard();
        ImGui::TextUnformatted(("Card " + std::to_string(i)).c_str());
        EndCard();
        ImGui::NextColumn();
    }
    
    ImGui::Columns(1);
}
```

### Collapsible Sections
```cpp
void DrawContent() override {
    static bool expanded = true;
    
    ImGui::PushStyleColor(ImGuiCol_Header, UIStyleHelper::Colors::Violet500);
    if (ImGui::CollapsingHeader("Section", &expanded)) {
        ImGui::PopStyleColor();
        ImGui::TextUnformatted("Content");
    } else {
        ImGui::PopStyleColor();
    }
}
```

## Common Patterns

### Status Display
```cpp
ImGui::TextUnformatted("Status:");
ImGui::SameLine();
UIStyleHelper::StatusBadge("active");
```

### Metric Card
```cpp
BeginCard();
ImGui::TextUnformatted("Metric Name");
ImGui::TextColored(ImVec4(0.55f, 0.36f, 0.96f, 1.0f), "123");
ImGui::TextDisabled("Unit");
EndCard();
```

### Progress Task
```cpp
ImGui::TextUnformatted("Task Name");
ImGui::SameLine(200);
UIStyleHelper::StatusBadge("running");
ImGui::Spacing();
DrawProgressBar(0.65f, "65%");
```

### Input Section
```cpp
ImGui::TextUnformatted("Input:");
static char buffer[256] = "";
ImGui::InputTextWithHint("##input", "Enter value...", buffer, sizeof(buffer));
ImGui::SameLine();
if (UIStyleHelper::ModernButtonPrimary("Submit")) {
    // Handle input
}
```

## Migration Checklist

- [ ] Apply `UIStyleHelper::ApplyModernTheme()` on startup
- [ ] Register modern panels in UIManager
- [ ] Update existing panels to inherit from `ModernPanelBase`
- [ ] Replace hardcoded colors with `UIStyleHelper::Colors`
- [ ] Use `UIStyleHelper` helper methods for buttons/badges
- [ ] Test all panels with new theme
- [ ] Verify responsive layouts
- [ ] Check accessibility (contrast ratios)

## Troubleshooting

### Theme not applying
- Ensure `UIStyleHelper::ApplyModernTheme()` is called after ImGui context creation
- Check that `UIManager::Initialize()` is called

### Colors look wrong
- Verify ImGui context is initialized before applying theme
- Check color values in `UIStyleHelper::Colors`
- Use `ImGui::ShowStyleEditor()` for debugging

### Panels not showing
- Ensure panels are registered with `UIManager::RegisterPanel()`
- Check panel `IsVisible` flag
- Verify docking layout in `UIManager::BuildDefaultDockspace()`

---

*SZM Forge UI Integration | v0.3.0*
