#pragma once

#include "../IPanel.hpp"
#include <string>

/**
 * @class TemplatePanel
 * @brief Template for creating new UI panels
 * 
 * Copy this file and customize:
 * 1. Rename class from TemplatePanel to YourPanelName
 * 2. Update GetTitle() to return your panel name
 * 3. Implement Draw() with your UI code
 * 4. Add private methods for UI sections
 * 5. Add member variables for state
 */
class TemplatePanel : public IPanel {
public:
    TemplatePanel();
    ~TemplatePanel() override = default;

    void Draw() override;
    std::string GetTitle() const override { return "Template Panel"; }

private:
    // UI sections
    void DrawHeader();
    void DrawContent();
    void DrawFooter();

    // State
    bool m_ShowDetails = false;
    float m_Value = 0.5f;
};
