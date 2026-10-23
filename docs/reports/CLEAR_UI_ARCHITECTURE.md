# Clear UI System - Architecture Reference

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ClearUIManager                           │
│  (Main orchestrator - singleton)                            │
└─────────────────────────────────────────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼
    ┌────────┐          ┌────────┐         ┌────────┐
    │ UITab  │          │ UITab  │   ...   │ UITab  │
    │        │          │        │         │        │
    │ Active │          │Inactive│         │Inactive│
    └────────┘          └────────┘         └────────┘
        │
        ├─ UISection (Data Import)
        │   └─ Content Lambda
        │
        ├─ UISection (Training Control)
        │   └─ Content Lambda
        │
        └─ UISection (Metrics)
            └─ Content Lambda
```

## Class Hierarchy

```
ClearUIManager (Singleton)
├── std::vector<UITab>
│   └── UITab
│       ├── TabType type
│       ├── std::string name
│       ├── std::string icon
│       └── std::vector<UISection>
│           └── UISection
│               ├── std::string title
│               ├── std::string icon
│               ├── std::function<void()> drawFunc
│               └── bool isCollapsed
└── UITab* activeTab
```

## Rendering Pipeline

```
Frame Start
    │
    ├─ ClearUIManager::RenderUI()
    │   │
    │   ├─ Setup main window
    │   │
    │   ├─ RenderTabBar()
    │   │   └─ For each tab:
    │   │       └─ DrawTabButton()
    │   │           └─ If clicked: SetActiveTab()
    │   │
    │   └─ RenderActiveTab()
    │       └─ activeTab->Draw()
    │           └─ For each section:
    │               └─ section->Draw()
    │                   ├─ Draw header with collapse button
    │                   └─ If not collapsed:
    │                       └─ Call m_DrawFunc()
    │
Frame End
```

## Data Flow

```
User Input
    │
    ├─ Click Tab Button
    │   └─ SetActiveTab(type)
    │       └─ Update m_ActiveTab
    │
    ├─ Click Section Header
    │   └─ Toggle m_IsCollapsed
    │
    └─ Interact with Section Content
        └─ m_DrawFunc() handles it

Next Frame
    │
    └─ RenderUI() reflects changes
```

## Tab Organization

```
┌─ Dashboard ─────────────────────────────────────┐
│ ├─ System Status                                │
│ ├─ Quick Actions                                │
│ └─ Recent Activity                              │
├─ Modeling ──────────────────────────────────────┤
│ ├─ Part Library                                 │
│ ├─ Geometry Tools                               │
│ └─ Properties                                   │
├─ Assembly ──────────────────────────────────────┤
│ ├─ Assembly Tree                                │
│ ├─ Mates                                        │
│ └─ Constraints                                  │
├─ Simulation ────────────────────────────────────┤
│ ├─ Simulation Control                           │
│ ├─ Results                                      │
│ └─ Visualization                                │
├─ Training ─────────────────────────────────────┤
│ ├─ Data Import                                  │
│ ├─ Training Control                             │
│ ├─ Configuration                                │
│ ├─ Metrics                                      │
│ └─ Checkpoints                                  │
├─ Results ──────────────────────────────────────┤
│ ├─ FEA Results                                  │
│ ├─ Thermal Analysis                             │
│ └─ Reports                                      │
└─ Settings ─────────────────────────────────────┘
  ├─ Preferences
  ├─ Themes
  └─ Advanced
```

## API Reference

### ClearUIManager

```cpp
// Singleton access
static ClearUIManager& GetInstance();

// Initialization
void Initialize();
void Shutdown();

// Tab management
void CreateTab(TabType type, const std::string& name, 
               const std::string& icon = "");
void SetActiveTab(TabType type);
void AddSectionToTab(TabType tabType, 
                     std::shared_ptr<UISection> section);

// Rendering
void RenderUI();
void RenderTabBar();
void RenderActiveTab();

// Accessors
std::shared_ptr<UITab> GetTab(TabType type) const;
std::shared_ptr<UITab> GetActiveTab() const;
```

### UITab

```cpp
// Constructor
UITab(TabType type, const std::string& name, 
      const std::string& icon = "");

// Section management
void AddSection(std::shared_ptr<UISection> section);

// Rendering
void Draw();

// State
void SetActive(bool active);
bool IsActive() const;

// Accessors
TabType GetType() const;
const std::string& GetName() const;
const std::string& GetIcon() const;
```

### UISection

```cpp
// Constructor
UISection(const std::string& title, 
          const std::string& icon = "");

// Content
void SetContent(std::function<void()> drawFunc);

// Rendering
void Draw();

// State
bool IsCollapsed() const;
void SetCollapsed(bool collapsed);

// Accessors
const std::string& GetTitle() const;
```

## Color Scheme

```
Active Tab:        #4080FF (Blue)
Inactive Tab:      #1F1F28 (Dark Gray)
Tab Hover:         #3050CC (Darker Blue)
Background:        #0F0F12 (Very Dark)
Frame:             #1A1A22 (Dark)
Text:              #FFFFFF (White)
Accent:            #64B6FF (Light Blue)
Success:           #50FF96 (Green)
Warning:           #FFD400 (Yellow)
Error:             #FF5050 (Red)
```

## Styling Constants

```cpp
WindowRounding:    4.0f
FrameRounding:     3.0f
GrabRounding:      3.0f
WindowPadding:     12.0f, 12.0f
FramePadding:      8.0f, 6.0f
ItemSpacing:       8.0f, 8.0f
```

## State Management

```
ClearUIManager
├── m_Tabs: vector<UITab>
├── m_ActiveTab: UITab*
└── m_DockspaceId: ImGuiID

UITab
├── m_Type: TabType
├── m_Name: string
├── m_Icon: string
├── m_Sections: vector<UISection>
└── m_IsActive: bool

UISection
├── m_Title: string
├── m_Icon: string
├── m_DrawFunc: function<void()>
└── m_IsCollapsed: bool
```

## Memory Layout

```
ClearUIManager (Singleton)
    ├─ 7 UITab objects
    │   ├─ Dashboard
    │   │   ├─ 3 UISection objects
    │   │   └─ Each with lambda
    │   ├─ Modeling
    │   │   ├─ 3 UISection objects
    │   │   └─ Each with lambda
    │   └─ ... (5 more tabs)
    └─ 1 active UITab pointer
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| CreateTab | O(1) | Append to vector |
| SetActiveTab | O(n) | n = number of tabs (7) |
| AddSectionToTab | O(n) | n = number of tabs |
| RenderUI | O(m) | m = sections in active tab |
| RenderTabBar | O(n) | n = number of tabs |
| Section Draw | O(1) | Calls user lambda |

## Thread Safety

- **Not thread-safe** - ImGui is single-threaded
- All operations must occur on main thread
- No internal locking

## Dependencies

- ImGui (for rendering)
- C++20 (for std::function, std::shared_ptr)
- Standard library (vector, string, functional)

## Integration Points

```
main_desktop.cpp
    │
    ├─ Application::Init()
    │   └─ ClearUIManager::Initialize()
    │
    ├─ Application::TickSystems()
    │   └─ ClearUIManager::RenderUI()
    │
    └─ Application::Shutdown()
        └─ ClearUIManager::Shutdown()
```

## Extension Points

1. **Add New Tab**
   ```cpp
   CreateTab(TabType::MyTab, "My Tab", "🎯");
   ```

2. **Add New Section**
   ```cpp
   auto section = std::make_shared<UISection>("Title");
   section->SetContent([]() { /* UI code */ });
   AddSectionToTab(TabType::MyTab, section);
   ```

3. **Customize Styling**
   ```cpp
   ImGuiStyle& style = ImGui::GetStyle();
   style.Colors[ImGuiCol_Button] = ImVec4(...);
   ```

## Future Enhancements

- [ ] Keyboard shortcuts for tab switching
- [ ] Tab persistence (save/load state)
- [ ] Drag-and-drop section reordering
- [ ] Custom section templates
- [ ] Section search/filter
- [ ] Tab grouping
- [ ] Floating sections
- [ ] Section history/undo

## Debugging

Enable ImGui metrics:
```cpp
ImGui::ShowMetricsWindow();
```

Check active tab:
```cpp
auto active = ui.GetActiveTab();
if (active) {
    std::cout << "Active: " << active->GetName() << std::endl;
}
```

## Summary

The Clear UI System provides:
- ✓ Clean, organized interface
- ✓ Easy to navigate
- ✓ Professional appearance
- ✓ Simple API
- ✓ Extensible design
- ✓ Production-ready code
