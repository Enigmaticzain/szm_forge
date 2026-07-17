#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "IPanel.hpp"

namespace SZM {

/**
 * @class UITab
 * @brief Represents a single tab containing multiple panels
 */
class UITab {
public:
    UITab(const std::string& name, const std::string& icon = "");
    
    void AddPanel(std::shared_ptr<IPanel> panel);
    void RemovePanel(const std::string& panelId);
    
    void Draw();
    void SetActive(bool active) { m_IsActive = active; }
    bool IsActive() const { return m_IsActive; }
    
    const std::string& GetName() const { return m_Name; }
    const std::string& GetIcon() const { return m_Icon; }
    const std::vector<std::shared_ptr<IPanel>>& GetPanels() const { return m_Panels; }

private:
    std::string m_Name;
    std::string m_Icon;
    std::vector<std::shared_ptr<IPanel>> m_Panels;
    bool m_IsActive = false;
};

/**
 * @class UITabManager
 * @brief Manages tabs and their content
 */
class UITabManager {
public:
    static UITabManager& GetInstance();

    // Tab management
    void CreateTab(const std::string& tabName, const std::string& icon = "");
    void AddPanelToTab(const std::string& tabName, std::shared_ptr<IPanel> panel);
    void RemoveTab(const std::string& tabName);
    void SetActiveTab(const std::string& tabName);
    
    // Rendering
    void DrawTabBar();
    void DrawActiveTabContent();
    
    // Accessors
    std::shared_ptr<UITab> GetTab(const std::string& tabName) const;
    std::shared_ptr<UITab> GetActiveTab() const;
    const std::vector<std::shared_ptr<UITab>>& GetAllTabs() const { return m_Tabs; }

private:
    UITabManager() = default;
    ~UITabManager() = default;

    static UITabManager* s_Instance;
    std::vector<std::shared_ptr<UITab>> m_Tabs;
    std::unordered_map<std::string, std::shared_ptr<UITab>> m_TabMap;
    std::shared_ptr<UITab> m_ActiveTab;
};

} // namespace SZM
