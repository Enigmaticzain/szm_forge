#pragma once

#include "IPanel.hpp"
#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace SZM {

/**
 * @class SimpleTab
 * @brief Easy-to-use tab with minimal setup
 */
struct SimpleTab {
    std::string id;
    std::string label;
    std::string icon;
    std::string tooltip;
    std::function<void()> draw;

    SimpleTab(
        const std::string& _id,
        const std::string& _label,
        const std::string& _icon,
        std::function<void()> _draw,
        std::string _tooltip = {}
    )
        : id(_id), label(_label), icon(_icon), tooltip(std::move(_tooltip)), draw(std::move(_draw)) {}
};

/**
 * @class EasyTabBar
 * @brief Beginner-friendly tab bar - just add tabs and draw!
 */
class EasyTabBar {
public:
    EasyTabBar() = default;
    ~EasyTabBar() = default;

    // Add a tab - super simple!
    void AddTab(
        const std::string& id,
        const std::string& label,
        const std::string& icon,
        std::function<void()> draw,
        const std::string& tooltip = {}
    ) {
        m_Tabs.emplace_back(id, label, icon, std::move(draw), tooltip);
        if (m_ActiveTabId.empty()) {
            m_ActiveTabId = id;
        }
    }

    // Draw everything - one function!
    void Draw();

    // Get/Set active tab
    void SetActiveTab(const std::string& id);
    const std::string& GetActiveTab() const { return m_ActiveTabId; }
    bool HasTab(const std::string& id) const;

    // Clear all tabs
    void Clear() { 
        m_Tabs.clear(); 
        m_ActiveTabId.clear();
    }

    // Get tab count
    size_t GetTabCount() const { return m_Tabs.size(); }

private:
    std::vector<SimpleTab> m_Tabs;
    std::string m_ActiveTabId;

    void EnsureValidActiveTab();
    void DrawTabButtons();
    void DrawTabContent();
};

/**
 * @class BeginnerPanel
 * @brief Super simple panel with built-in tab support
 * 
 * Usage:
 *   class MyPanel : public BeginnerPanel {
 *       MyPanel() {
 *           AddTab("tab1", "Tab 1", "📌", [this]() { DrawTab1(); });
 *           AddTab("tab2", "Tab 2", "📌", [this]() { DrawTab2(); });
 *       }
 *       void DrawTab1() { ImGui::TextUnformatted("Content"); }
 *       void DrawTab2() { ImGui::TextUnformatted("More content"); }
 *   };
 */
class BeginnerPanel : public IPanel {
public:
    BeginnerPanel(const std::string& title) : m_Title(title) {}
    ~BeginnerPanel() override = default;

    // Add a tab - that's it!
    void AddTab(
        const std::string& id,
        const std::string& label,
        const std::string& icon,
        std::function<void()> draw,
        const std::string& tooltip = {}
    ) {
        m_TabBar.AddTab(id, label, icon, std::move(draw), tooltip);
    }

    // Draw the panel - handles everything
    void Draw() override {
        if (ImGui::Begin(m_Title.c_str(), &m_IsOpen)) {
            m_TabBar.Draw();
        }
        ImGui::End();
    }

    std::string GetTitle() const override { return m_Title; }
    const char* GetName() const override { return m_Title.c_str(); }

    // Helpers
    void SetActiveTab(const std::string& id) { m_TabBar.SetActiveTab(id); }
    const std::string& GetActiveTab() const { return m_TabBar.GetActiveTab(); }

protected:
    EasyTabBar m_TabBar;
    std::string m_Title;
};

} // namespace SZM
