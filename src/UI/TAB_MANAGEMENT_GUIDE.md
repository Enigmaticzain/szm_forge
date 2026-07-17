# Modern Tab Management System — SZM Forge UI

## Overview

The tab management system provides smooth, animated tab switching with a clean modern design inspired by professional web applications. All tabs feature:

- **Smooth animations** - Fade transitions between tabs
- **Modern styling** - Violet accent colors, rounded buttons
- **Responsive layout** - Adapts to window size
- **Easy integration** - Simple API for adding tabs

## Components

### TabManager
Core tab management with animation support.

```cpp
TabManager tabMgr;
tabMgr.AddTab("overview", "Overview", "📊", []() {
    ImGui::TextUnformatted("Overview content");
});
tabMgr.AddTab("tasks", "Tasks", "✓", []() {
    ImGui::TextUnformatted("Tasks content");
});

// Draw tab bar
tabMgr.DrawTabBar();

// Draw active tab content
tabMgr.DrawTabContent();

// Update animations
tabMgr.UpdateAnimation(deltaTime);
```

### ModernTabPanel
Base class for tabbed panels with automatic layout.

```cpp
class MyTabPanel : public ModernTabPanel {
public:
    const char* GetTitle() const override { return "My Panel"; }
    
    MyTabPanel() {
        AddTab("tab1", "Tab 1", "📌", [this]() { DrawTab1(); });
        AddTab("tab2", "Tab 2", "📌", [this]() { DrawTab2(); });
    }
    
private:
    void DrawTab1() { /* content */ }
    void DrawTab2() { /* content */ }
};
```

### AnimatedPanel
Individual panels with fade animations.

```cpp
AnimatedPanel panel("my_panel");
panel.UpdateAnimation(deltaTime);
panel.SetVisible(true);
// Alpha automatically animates
```

## Usage Examples

### Example 1: Simple Tabbed Panel

```cpp
class AnalysisPanel : public ModernTabPanel {
public:
    const char* GetTitle() const override { return "Analysis"; }
    
    AnalysisPanel() {
        AddTab("results", "Results", "📊", [this]() {
            ImGui::TextUnformatted("Analysis results here");
        });
        AddTab("details", "Details", "📋", [this]() {
            ImGui::TextUnformatted("Detailed information");
        });
    }
};
```

### Example 2: Tab with Dynamic Content

```cpp
class DataPanel : public ModernTabPanel {
private:
    std::vector<std::string> m_Data;
    
public:
    const char* GetTitle() const override { return "Data"; }
    
    DataPanel() {
        AddTab("table", "Table", "📊", [this]() { DrawTable(); });
        AddTab("chart", "Chart", "📈", [this]() { DrawChart(); });
    }
    
    void DrawTable() {
        for (const auto& item : m_Data) {
            ImGui::TextUnformatted(item.c_str());
        }
    }
    
    void DrawChart() {
        // Chart rendering
    }
};
```

### Example 3: Settings Panel with Tabs

```cpp
class SettingsPanel : public ModernTabPanel {
public:
    const char* GetTitle() const override { return "Settings"; }
    
    SettingsPanel() {
        AddTab("general", "General", "⚙️", [this]() { DrawGeneral(); });
        AddTab("advanced", "Advanced", "🔧", [this]() { DrawAdvanced(); });
        AddTab("about", "About", "ℹ️", [this]() { DrawAbout(); });
    }
    
private:
    void DrawGeneral() {
        static bool darkMode = false;
        ImGui::Checkbox("Dark Mode", &darkMode);
    }
    
    void DrawAdvanced() {
        static int quality = 2;
        ImGui::SliderInt("Quality", &quality, 1, 5);
    }
    
    void DrawAbout() {
        ImGui::TextUnformatted("SZM Forge v0.3.0");
    }
};
```

## Animation System

### Tab Switching Animation
- Duration: 0.3 seconds (configurable)
- Type: Fade in/out
- Progress: 0.0 → 1.0

### Panel Visibility Animation
- Speed: 5.0 units/second (configurable)
- Type: Smooth alpha transition
- Range: 0.0 → 1.0

### Customizing Animations

```cpp
// In TabManager
m_AnimationDuration = 0.5f;  // Slower animation

// In AnimatedPanel
panel.m_AnimationSpeed = 3.0f;  // Faster fade
```

## Styling

### Tab Bar Colors
- **Active tab**: Violet500 → Violet600 (gradient)
- **Inactive tab**: Slate100 → Slate200 (hover)
- **Text**: White (active), Slate700 (inactive)
- **Border radius**: 6px

### Tab Content
- **Fade in**: 0.3s duration
- **Alpha range**: 0.0 → 1.0
- **Easing**: Linear

## Integration with Existing Panels

### Converting Old Panel to Tabbed

**Before:**
```cpp
class OldPanel : public IPanel {
    void Draw() override {
        ImGui::Begin("Panel");
        // All content mixed
        ImGui::End();
    }
};
```

**After:**
```cpp
class NewPanel : public ModernTabPanel {
    const char* GetTitle() const override { return "Panel"; }
    
    NewPanel() {
        AddTab("section1", "Section 1", "📌", [this]() { DrawSection1(); });
        AddTab("section2", "Section 2", "📌", [this]() { DrawSection2(); });
    }
    
private:
    void DrawSection1() { /* content */ }
    void DrawSection2() { /* content */ }
};
```

## Performance Considerations

- **Tab switching**: O(1) - instant
- **Animation updates**: O(1) per frame
- **Memory**: ~100 bytes per tab
- **Rendering**: No additional overhead

## Best Practices

1. **Keep tabs focused** - Each tab should have a single purpose
2. **Use clear icons** - Help users identify tabs quickly
3. **Limit tab count** - 3-5 tabs per panel is ideal
4. **Consistent naming** - Use clear, concise labels
5. **Preserve state** - Remember user's last active tab

## Troubleshooting

### Tabs not switching
- Ensure `UpdateAnimation()` is called each frame
- Check that tab IDs are unique

### Animation stuttering
- Verify deltaTime is correct
- Check for frame rate drops

### Content not showing
- Ensure content function is provided
- Check that tab is visible

## API Reference

### TabManager

```cpp
void AddTab(const std::string& id, const std::string& label, 
            const std::string& icon, std::function<void()> content);
void RemoveTab(const std::string& id);
void SetActiveTab(const std::string& id);
const std::string& GetActiveTab() const;
void DrawTabBar(float width = -1.0f);
void DrawTabContent();
void UpdateAnimation(float deltaTime);
float GetTabAnimationProgress() const;
```

### ModernTabPanel

```cpp
void Draw();
void AddTab(const std::string& id, const std::string& label, 
            const std::string& icon, std::function<void()> content);
virtual const char* GetTitle() const = 0;
virtual const char* GetIcon() const;
virtual void DrawHeader();
virtual void DrawFooter();
```

### AnimatedPanel

```cpp
void Draw() = 0;
const char* GetTitle() const = 0;
void SetVisible(bool visible);
bool IsVisible() const;
float GetAlpha() const;
void UpdateAnimation(float deltaTime);
```

---

*SZM Forge Tab Management | v0.1.0 | Modern Animated UI*
