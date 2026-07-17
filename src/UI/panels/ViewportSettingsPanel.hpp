#pragma once

#include "IPanel.hpp"
#include <glm/glm.hpp>

namespace SZM::UI {

/**
 * @class ViewportSettingsPanel
 * @brief Viewport settings and preferences
 */
class ViewportSettingsPanel : public IPanel {
public:
    ViewportSettingsPanel();
    ~ViewportSettingsPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Viewport Settings"; }

private:
    glm::vec3 m_BackgroundColor = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 m_GridColor = glm::vec3(0.3f, 0.3f, 0.3f);
    bool m_ShowGrid = true;
    bool m_ShowAxes = true;
    bool m_ShowStats = true;
    float m_GridSize = 1.0f;
    float m_GridSpacing = 0.1f;

    void DrawRenderSettings();
    void DrawGridSettings();
    void DrawDisplaySettings();
};

} // namespace SZM::UI
