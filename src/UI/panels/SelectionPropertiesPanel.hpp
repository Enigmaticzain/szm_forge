#pragma once

#include "IPanel.hpp"
#include "../Graphics/Advanced/SelectionManager.hpp"
#include "../../Simulation/SimulationEngine.hpp"

namespace SZM::UI {

/**
 * @class SelectionPropertiesPanel
 * @brief Displays properties of selected components
 */
class SelectionPropertiesPanel : public IPanel {
public:
    SelectionPropertiesPanel();
    ~SelectionPropertiesPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Selection Properties"; }

private:
    Graphics::SelectionManager& m_SelectionManager;
    ::SZM::SimulationEngine& m_SimEngine;

    void DrawComponentProperties(uint32_t componentId);
    void DrawStressInfo(const ::SZM::SimulationComponent& comp);
    void DrawThermalInfo(const ::SZM::SimulationComponent& comp);
};

} // namespace SZM::UI
