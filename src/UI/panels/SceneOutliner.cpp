#include "SceneOutliner.hpp"
#include "../../Input/EventBus.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include <imgui.h>

SceneOutliner::SceneOutliner() {
    // We now fetch data live from the SimulationEngine.
}

void SceneOutliner::RefreshNodes(const std::vector<std::string>& rootNodeNames) {
    m_RootNodes = rootNodeNames;
    if (m_RootNodes.empty()) {
        m_SelectedNode.clear();
    } else if (m_SelectedNode.empty()) {
        m_SelectedNode = m_RootNodes.front();
    }
}

void SceneOutliner::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    const auto& components = SZM::SimulationEngine::GetInstance().GetComponents();

    if (components.empty()) {
        ImGui::TextDisabled("No components loaded.");
        ImGui::End();
        return;
    }

    for (const auto& comp : components) {
        if (!comp) continue;
        
        // Show name and ID for clarity
        std::string label = comp->name + " (ID: " + std::to_string(comp->id) + ")";
        const bool isSelected = (m_SelectedNode == std::to_string(comp->id));
        
        if (ImGui::Selectable(label.c_str(), isSelected)) {
            m_SelectedNode = std::to_string(comp->id);
            SZM::EventBus::GetInstance().Publish("SelectionChanged", m_SelectedNode);
        }
    }

    ImGui::End();
}
