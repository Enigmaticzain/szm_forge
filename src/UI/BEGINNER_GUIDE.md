# Beginner's Guide to SZM Forge Tabs

## 🎯 What Are Tabs?

Tabs let you organize content into sections. Click a tab to see different content.

```
┌─────────────────────────────────────┐
│ 🏠 Home  ⚙️ Settings  ℹ️ About      │
├─────────────────────────────────────┤
│                                     │
│  Welcome to SZM Forge!              │
│  Click tabs above to switch         │
│                                     │
└─────────────────────────────────────┘
```

---

## 🚀 Quick Start (5 Minutes)

### Step 1: Create Your Panel Class

```cpp
#include "UI/EasyTabBar.hpp"

class MyPanel : public BeginnerPanel {
public:
    MyPanel() : BeginnerPanel("My Panel") {
        // Add tabs here
    }
};
```

### Step 2: Add Tabs

```cpp
class MyPanel : public BeginnerPanel {
public:
    MyPanel() : BeginnerPanel("My Panel") {
        AddTab("tab1", "First Tab", "📌", [this]() { DrawTab1(); });
        AddTab("tab2", "Second Tab", "📌", [this]() { DrawTab2(); });
    }

private:
    void DrawTab1() {
        ImGui::TextUnformatted("Content for tab 1");
    }

    void DrawTab2() {
        ImGui::TextUnformatted("Content for tab 2");
    }
};
```

### Step 3: Draw It

```cpp
MyPanel panel;

// In your main loop:
panel.Draw();
```

**That's it! You have a tabbed panel!**

---

## 📚 Complete Examples

### Example 1: Simple Info Panel

```cpp
class InfoPanel : public BeginnerPanel {
public:
    InfoPanel() : BeginnerPanel("Information") {
        AddTab("general", "General", "📋", [this]() { DrawGeneral(); });
        AddTab("details", "Details", "📝", [this]() { DrawDetails(); });
    }

private:
    void DrawGeneral() {
        ImGui::TextUnformatted("General Information");
        ImGui::TextUnformatted("Version: 1.0");
        ImGui::TextUnformatted("Status: Active");
    }

    void DrawDetails() {
        ImGui::TextUnformatted("Detailed Information");
        ImGui::TextUnformatted("Author: SZM");
        ImGui::TextUnformatted("License: MIT");
    }
};
```

### Example 2: Settings Panel

```cpp
class SettingsPanel : public BeginnerPanel {
public:
    SettingsPanel() : BeginnerPanel("Settings") {
        AddTab("display", "Display", "🎨", [this]() { DrawDisplay(); });
        AddTab("audio", "Audio", "🔊", [this]() { DrawAudio(); });
        AddTab("advanced", "Advanced", "🔧", [this]() { DrawAdvanced(); });
    }

private:
    void DrawDisplay() {
        static bool darkMode = false;
        static int brightness = 50;

        ImGui::Checkbox("Dark Mode", &darkMode);
        ImGui::SliderInt("Brightness", &brightness, 0, 100);
    }

    void DrawAudio() {
        static int volume = 80;
        static bool muted = false;

        ImGui::SliderInt("Volume", &volume, 0, 100);
        ImGui::Checkbox("Muted", &muted);
    }

    void DrawAdvanced() {
        ImGui::TextUnformatted("Advanced Settings");
        ImGui::TextUnformatted("(Add your settings here)");
    }
};
```

### Example 3: Data Panel

```cpp
class DataPanel : public BeginnerPanel {
public:
    DataPanel() : BeginnerPanel("Data") {
        AddTab("table", "Table", "📊", [this]() { DrawTable(); });
        AddTab("stats", "Statistics", "📈", [this]() { DrawStats(); });
    }

private:
    void DrawTable() {
        ImGui::TextUnformatted("Data Table");
        ImGui::Separator();
        ImGui::TextUnformatted("Row 1: Value A");
        ImGui::TextUnformatted("Row 2: Value B");
        ImGui::TextUnformatted("Row 3: Value C");
    }

    void DrawStats() {
        ImGui::TextUnformatted("Statistics");
        ImGui::Separator();
        ImGui::TextUnformatted("Total: 100");
        ImGui::TextUnformatted("Average: 50");
        ImGui::TextUnformatted("Max: 99");
    }
};
```

---

## 🎨 Customization

### Change Tab Icons

```cpp
// Use any emoji or text
AddTab("id", "Label", "🎯", [this]() { /* content */ });
AddTab("id", "Label", "✓", [this]() { /* content */ });
AddTab("id", "Label", "★", [this]() { /* content */ });
```

### Add More Tabs

```cpp
AddTab("tab1", "Tab 1", "1️⃣", [this]() { DrawTab1(); });
AddTab("tab2", "Tab 2", "2️⃣", [this]() { DrawTab2(); });
AddTab("tab3", "Tab 3", "3️⃣", [this]() { DrawTab3(); });
AddTab("tab4", "Tab 4", "4️⃣", [this]() { DrawTab4(); });
```

### Switch Tabs Programmatically

```cpp
// Set active tab by ID
panel.SetActiveTab("settings");

// Get current active tab
std::string current = panel.GetActiveTab();
```

---

## 🔧 Common Tasks

### Task 1: Add a Button in a Tab

```cpp
void DrawSettings() {
    ImGui::TextUnformatted("Settings");
    ImGui::Spacing();
    
    if (ImGui::Button("Save")) {
        // Do something
    }
}
```

### Task 2: Add Input Fields

```cpp
void DrawSettings() {
    static char name[128] = "";
    static int value = 0;
    
    ImGui::InputText("Name", name, sizeof(name));
    ImGui::InputInt("Value", &value);
}
```

### Task 3: Add Checkboxes

```cpp
void DrawSettings() {
    static bool option1 = true;
    static bool option2 = false;
    
    ImGui::Checkbox("Option 1", &option1);
    ImGui::Checkbox("Option 2", &option2);
}
```

### Task 4: Add Sliders

```cpp
void DrawSettings() {
    static int quality = 2;
    static float speed = 1.0f;
    
    ImGui::SliderInt("Quality", &quality, 1, 5);
    ImGui::SliderFloat("Speed", &speed, 0.1f, 2.0f);
}
```

---

## 📖 API Reference

### BeginnerPanel

```cpp
// Constructor
BeginnerPanel(const std::string& title);

// Add a tab
void AddTab(const std::string& id, const std::string& label, 
            const std::string& icon, std::function<void()> draw);

// Draw the panel
void Draw();

// Get/Set active tab
void SetActiveTab(const std::string& id);
const std::string& GetActiveTab() const;
```

### EasyTabBar

```cpp
// Add a tab
void AddTab(const std::string& id, const std::string& label, 
            const std::string& icon, std::function<void()> draw);

// Draw everything
void Draw();

// Get/Set active tab
void SetActiveTab(const std::string& id);
const std::string& GetActiveTab() const;

// Clear all tabs
void Clear();

// Get tab count
size_t GetTabCount() const;
```

---

## ❓ FAQ

### Q: How many tabs can I have?
**A:** As many as you want! But 3-5 is usually best.

### Q: Can I change tab order?
**A:** Tabs appear in the order you add them.

### Q: Can I hide a tab?
**A:** Not directly, but you can add/remove tabs dynamically.

### Q: Can I have nested tabs?
**A:** Yes! Create a tab that contains another BeginnerPanel.

### Q: How do I save tab state?
**A:** Store the active tab ID and restore it on startup.

---

## 🎓 Learning Path

1. **Start here:** SimpleExamplePanel
2. **Try this:** Create your own 2-tab panel
3. **Next:** Add buttons and inputs to tabs
4. **Advanced:** Create nested tabs

---

## 💡 Tips & Tricks

### Tip 1: Use Descriptive Labels
```cpp
// Good
AddTab("settings", "Settings", "⚙️", [this]() { DrawSettings(); });

// Not as good
AddTab("tab2", "Tab 2", "📌", [this]() { DrawTab2(); });
```

### Tip 2: Use Consistent Icons
```cpp
// Settings always use ⚙️
// Info always uses ℹ️
// Home always uses 🏠
```

### Tip 3: Keep Tabs Focused
```cpp
// Good - each tab has one purpose
AddTab("display", "Display", "🎨", [this]() { DrawDisplay(); });
AddTab("audio", "Audio", "🔊", [this]() { DrawAudio(); });

// Not as good - too much in one tab
AddTab("all", "All Settings", "⚙️", [this]() { DrawEverything(); });
```

### Tip 4: Use Lambda Captures
```cpp
// Access member variables
AddTab("data", "Data", "📊", [this]() { 
    ImGui::TextUnformatted(m_Data.c_str());
});
```

---

## 🐛 Troubleshooting

### Problem: Tabs not showing
**Solution:** Make sure you called `panel.Draw()` in your main loop

### Problem: Content not updating
**Solution:** Make sure your draw function updates every frame

### Problem: Tab not switching
**Solution:** Check that tab IDs are unique

### Problem: Buttons not working
**Solution:** Make sure button is inside the draw function

---

## 📝 Template

Copy this template to create your own panel:

```cpp
#include "UI/EasyTabBar.hpp"

class MyPanel : public BeginnerPanel {
public:
    MyPanel() : BeginnerPanel("My Panel Title") {
        AddTab("tab1", "Tab 1", "📌", [this]() { DrawTab1(); });
        AddTab("tab2", "Tab 2", "📌", [this]() { DrawTab2(); });
    }

private:
    void DrawTab1() {
        ImGui::TextUnformatted("Tab 1 content");
    }

    void DrawTab2() {
        ImGui::TextUnformatted("Tab 2 content");
    }
};
```

---

## 🎉 You're Ready!

You now know everything you need to create tabbed panels in SZM Forge!

**Next steps:**
1. Create your first panel
2. Add some tabs
3. Add buttons and inputs
4. Have fun!

---

*SZM Forge Beginner's Guide | Easy Tabs for Everyone*
