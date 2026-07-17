#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace SZM {

/**
 * @class TabManager
 * @brief Modern animated tab system with smooth transitions
 */
class TabManager {
public:
    struct Tab {
        std::string id;
        std::string label;
        std::string icon;
        std::function<void()> content;
        bool visible = true;
    };

    TabManager() = default;
    ~TabManager() = default;

    // Tab management
    void AddTab(const std::string& id, const std::string& label, const std::string& icon, std::function<void()> content);
    void RemoveTab(const std::string& id);
    void SetActiveTab(const std::string& id);
    const std::string& GetActiveTab() const { return m_ActiveTab; }
    bool HasVisibleTab(const std::string& id) const;

    // Rendering
    void DrawTabBar(float width = -1.0f);
    void DrawTabContent();

    // Animation state
    float GetTabAnimationProgress() const { return m_AnimationProgress; }
    void UpdateAnimation(float deltaTime);

private:
    std::vector<Tab> m_Tabs;
    std::string m_ActiveTab;
    std::string m_PreviousTab;
    float m_AnimationProgress = 1.0f;
    float m_AnimationDuration = 0.3f;
    bool m_IsAnimating = false;

    void EnsureValidActiveTab();
    void DrawTabBarButton(const Tab& tab, bool isActive, float width);
};

/**
 * @class AnimatedPanel
 * @brief Panel with smooth fade/slide animations
 */
class AnimatedPanel {
public:
    AnimatedPanel(const std::string& id) : m_Id(id) {}
    virtual ~AnimatedPanel() = default;

    virtual void Draw() = 0;
    virtual const char* GetTitle() const = 0;

    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }

    float GetAlpha() const { return m_Alpha; }
    void UpdateAnimation(float deltaTime);

protected:
    std::string m_Id;
    bool m_Visible = true;
    float m_Alpha = 1.0f;
    float m_TargetAlpha = 1.0f;
    float m_AnimationSpeed = 5.0f;
};

/**
 * @class ModernTabPanel
 * @brief Base class for modern tabbed panels with animations
 */
class ModernTabPanel {
public:
    ModernTabPanel() = default;
    virtual ~ModernTabPanel() = default;

    void Draw();
    void AddTab(const std::string& id, const std::string& label, const std::string& icon, std::function<void()> content);

    virtual const char* GetTitle() const = 0;
    virtual const char* GetIcon() const { return "📋"; }

protected:
    TabManager m_TabManager;
    float m_DeltaTime = 0.016f;

    virtual void DrawHeader() {}
    virtual void DrawFooter() {}
};

} // namespace SZM
