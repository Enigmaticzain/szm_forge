# Quick Reference Card — Beginner Tab System

## Copy-Paste Template

```cpp
#include "UI/EasyTabBar.hpp"

class MyPanel : public BeginnerPanel {
public:
    MyPanel() : BeginnerPanel("Panel Title") {
        AddTab("tab1", "Tab 1", "📌", [this]() { DrawTab1(); });
        AddTab("tab2", "Tab 2", "📌", [this]() { DrawTab2(); });
    }

private:
    void DrawTab1() {
        ImGui::TextUnformatted("Content for tab 1");
    }

    void DrawTab2() {
        ImGui::TextUnformatted("Content for tab 2");
    }
};

// In main loop:
MyPanel panel;
panel.Draw();
```

---

## Common Icons

```
🏠 Home          📌 Pin           ⚙️ Settings
📊 Chart         📈 Graph         📋 List
✓ Check          ✗ Close          ℹ️ Info
🔧 Tools         🎨 Design        🔊 Audio
🎯 Target        ⭐ Star          🚀 Rocket
📝 Edit          💾 Save          🗑️ Delete
🔍 Search        📱 Mobile        💻 Desktop
```

---

## Common Tasks

### Add Button
```cpp
if (ImGui::Button("Click Me")) {
    // Do something
}
```

### Add Input
```cpp
static char text[128] = "";
ImGui::InputText("Label", text, sizeof(text));
```

### Add Checkbox
```cpp
static bool checked = false;
ImGui::Checkbox("Option", &checked);
```

### Add Slider
```cpp
static int value = 50;
ImGui::SliderInt("Value", &value, 0, 100);
```

### Add Text
```cpp
ImGui::TextUnformatted("Normal text");
ImGui::TextColored(ImVec4(1, 0, 0, 1), "Red text");
ImGui::TextDisabled("Disabled text");
```

### Add Separator
```cpp
ImGui::Separator();
```

### Add Spacing
```cpp
ImGui::Spacing();
```

---

## API Cheat Sheet

```cpp
// Create panel
BeginnerPanel panel("Title");

// Add tab
panel.AddTab("id", "Label", "icon", [this]() { /* content */ });

// Draw panel
panel.Draw();

// Switch tab
panel.SetActiveTab("tab_id");

// Get active tab
std::string active = panel.GetActiveTab();
```

---

## Tab Naming Convention

```cpp
// Good names
AddTab("home", "Home", "🏠", ...);
AddTab("settings", "Settings", "⚙️", ...);
AddTab("about", "About", "ℹ️", ...);

// Avoid
AddTab("tab1", "Tab 1", "📌", ...);
AddTab("tab2", "Tab 2", "📌", ...);
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Tabs not showing | Call `panel.Draw()` in main loop |
| Content not updating | Make sure draw function runs every frame |
| Tab not switching | Check tab ID is correct |
| Button not working | Make sure button is in draw function |
| Text not showing | Use `ImGui::TextUnformatted()` |

---

## File Locations

```
src/UI/
├── EasyTabBar.hpp          ← Include this
├── BEGINNER_GUIDE.md       ← Read this
└── panels/
    └── SimpleExamplePanel.hpp  ← Study this
```

---

## 3-Step Process

1. **Create class** inheriting from `BeginnerPanel`
2. **Add tabs** in constructor with `AddTab()`
3. **Draw content** in private functions

---

## Example: Settings Panel

```cpp
class SettingsPanel : public BeginnerPanel {
public:
    SettingsPanel() : BeginnerPanel("Settings") {
        AddTab("display", "Display", "🎨", [this]() { DrawDisplay(); });
        AddTab("audio", "Audio", "🔊", [this]() { DrawAudio(); });
    }

private:
    void DrawDisplay() {
        static bool darkMode = false;
        ImGui::Checkbox("Dark Mode", &darkMode);
    }

    void DrawAudio() {
        static int volume = 80;
        ImGui::SliderInt("Volume", &volume, 0, 100);
    }
};
```

---

## Remember

- ✅ Keep it simple
- ✅ Use clear tab names
- ✅ Use consistent icons
- ✅ One purpose per tab
- ✅ Test your code

---

*SZM Forge Quick Reference | Beginner Tab System*
