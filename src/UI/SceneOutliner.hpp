#pragma once

#include "IPanel.hpp"
#include <vector>
#include <string>

namespace SZM {

    class SceneOutliner : public IPanel {
    public:
        SceneOutliner();
        ~SceneOutliner() = default;

        void Draw() override;
        std::string GetTitle() const override { return "Scene Outliner"; }

        void AddNode(const std::string& nodeName);
        void Clear();

    private:
        std::vector<std::string> m_RootNodes;
    };

} // namespace SZM


// ===== NEW ADDITIONS FROM CODE BUNDLES =====

class SceneOutliner : public IPanel {
public:
    SceneOutliner();
    void Draw() override;
    std::string GetTitle() const override { return "Scene Outliner"; }

private:
    // Mock data for the tree nodes
    std::vector<std::string> m_RootNodes;
};
""",

    "SZM_Forge_UI/src/panels/SceneOutliner.cpp": """// SZM_Forge_UI/src/panels/SceneOutliner.cpp