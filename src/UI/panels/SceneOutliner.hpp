#pragma once

#include "../IPanel.hpp"
#include <vector>
#include <string>

/**
 * @class SceneOutliner
 * @brief Left-side panel showing the scene hierarchy tree.
 *
 * Reads from the SceneGraphManager (Data module) and displays
 * a collapsible tree of all entities in the workspace.
 * Clicking a node publishes a "SelectionChanged" event via EventBus.
 */
class SceneOutliner : public IPanel {
public:
    SceneOutliner();

    void        Draw()     override;
    std::string GetTitle() const override { return "Scene Outliner"; }

    /// Called by the scene graph manager when the hierarchy changes.
    void RefreshNodes(const std::vector<std::string>& rootNodeNames);

private:
    std::vector<std::string> m_RootNodes;
    std::string m_SelectedNode;
};
