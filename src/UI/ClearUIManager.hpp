#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <imgui.h>

namespace SZM::UI {

/**
 * @enum TabType
 * @brief Main application tabs
 */
enum class TabType {
    Dashboard,      // Overview and quick stats
    Modeling,       // CAD and geometry
    Assembly,       // Assembly and kinematics
    Simulation,     // Simulation and analysis
    Training,       // AI training studio
    Results,        // Analysis results
    Settings        // Application settings
};

/**
 * @class UISection
 * @brief Represents a section within a tab
 */
class UISection {
public:
    UISection(const std::string& title, const std::string& icon = "");
    
    void SetContent(std::function<void()> drawFunc) { m_DrawFunc = drawFunc; }
    void Draw();
    
    const std::string& GetTitle() const { return m_Title; }
    bool IsCollapsed() const { return m_IsCollapsed; }
    void SetCollapsed(bool collapsed) { m_IsCollapsed = collapsed; }

private:
    std::string m_Title;
    std::string m_Icon;
    std::function<void()> m_DrawFunc;
    bool m_IsCollapsed = false;
};

/**
 * @class UITab
 * @brief Represents a main application tab
 */
class UITab {
public:
    UITab(TabType type, const std::string& name, const std::string& icon = "");
    
    void AddSection(std::shared_ptr<UISection> section);
    void Draw();
    void SetActive(bool active) { m_IsActive = active; }
    bool IsActive() const { return m_IsActive; }
    
    TabType GetType() const { return m_Type; }
    const std::string& GetName() const { return m_Name; }
    const std::string& GetIcon() const { return m_Icon; }

private:
    TabType m_Type;
    std::string m_Name;
    std::string m_Icon;
    std::vector<std::shared_ptr<UISection>> m_Sections;
    bool m_IsActive = false;
};

/**
 * @class ClearUIManager
 * @brief Professional, clean UI manager with tab-based organization
 * 
 * Features:
 *  - Clear tab-based navigation
 *  - Organized sections within tabs
 *  - Consistent styling
 *  - Easy to extend
 */
class ClearUIManager {
public:
    static ClearUIManager& GetInstance();

    void Initialize();
    void Shutdown();
    
    // Tab management
    void CreateTab(TabType type, const std::string& name, const std::string& icon = "");
    void SetActiveTab(TabType type);
    void AddSectionToTab(TabType tabType, std::shared_ptr<UISection> section);
    
    // Rendering
    void RenderUI();
    void RenderTabBar();
    void RenderActiveTab();
    
    // Accessors
    std::shared_ptr<UITab> GetTab(TabType type) const;
    std::shared_ptr<UITab> GetActiveTab() const { return m_ActiveTab; }

private:
    ClearUIManager() = default;
    ~ClearUIManager() = default;

    void SetupDefaultTabs();
    void DrawTabButton(const std::shared_ptr<UITab>& tab);

    static ClearUIManager* s_Instance;
    std::vector<std::shared_ptr<UITab>> m_Tabs;
    std::shared_ptr<UITab> m_ActiveTab;
    ImGuiID m_DockspaceId = 0;
};

} // namespace SZM::UI
