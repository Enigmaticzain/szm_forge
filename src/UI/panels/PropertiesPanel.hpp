#pragma once

#include "../IPanel.hpp"
#include <string>

/**
 * @class PropertiesPanel
 * @brief Right-side inspector panel.
 *
 * Displays and edits properties of the currently selected entity.
 * Subscribes to the "SelectionChanged" EventBus event published
 * by the SceneOutliner.
 *
 * Status: Stub — awaiting 200-Series component system.
 */
class PropertiesPanel : public IPanel {
public:
    PropertiesPanel();

    void        Draw()     override;
    std::string GetTitle() const override { return "Properties Inspector"; }

    void SetSelectedEntityName(const std::string& name);

private:
    std::string m_SelectedEntity = "(nothing selected)";
};
