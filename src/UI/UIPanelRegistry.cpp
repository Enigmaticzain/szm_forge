#include "UIPanelRegistry.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace SZM {

UIPanelRegistry* UIPanelRegistry::s_Instance = nullptr;

UIPanelRegistry& UIPanelRegistry::GetInstance() {
    if (s_Instance != nullptr) {
        return *s_Instance;
    }
    static UIPanelRegistry instance;
    s_Instance = &instance;
    return instance;
}

void UIPanelRegistry::RegisterPanel(
    const std::string& id,
    std::shared_ptr<IPanel> panel,
    PanelCategory category
) {
    if (!panel) return;

    m_Panels[id] = {panel, category, true};
    
    int catIndex = static_cast<int>(category);
    if (m_CategorizedPanels.find(catIndex) == m_CategorizedPanels.end()) {
        m_CategorizedPanels[catIndex] = {};
    }
    m_CategorizedPanels[catIndex].push_back(id);
}

std::shared_ptr<IPanel> UIPanelRegistry::GetPanel(const std::string& id) const {
    auto it = m_Panels.find(id);
    if (it != m_Panels.end()) {
        return it->second.panel;
    }
    return nullptr;
}

std::vector<std::shared_ptr<IPanel>> UIPanelRegistry::GetPanelsByCategory(
    PanelCategory category
) const {
    std::vector<std::shared_ptr<IPanel>> result;
    int catIndex = static_cast<int>(category);
    
    auto it = m_CategorizedPanels.find(catIndex);
    if (it != m_CategorizedPanels.end()) {
        for (const auto& id : it->second) {
            auto panelIt = m_Panels.find(id);
            if (panelIt != m_Panels.end()) {
                result.push_back(panelIt->second.panel);
            }
        }
    }
    
    return result;
}

std::vector<std::shared_ptr<IPanel>> UIPanelRegistry::GetAllPanels() const {
    std::vector<std::shared_ptr<IPanel>> result;
    for (const auto& [id, entry] : m_Panels) {
        result.push_back(entry.panel);
    }
    return result;
}

void UIPanelRegistry::SetPanelVisible(const std::string& id, bool visible) {
    auto it = m_Panels.find(id);
    if (it != m_Panels.end()) {
        it->second.panel->IsVisible = visible;
        it->second.visible = visible;
    }
}

void UIPanelRegistry::SetCategoryVisible(PanelCategory category, bool visible) {
    int catIndex = static_cast<int>(category);
    auto it = m_CategorizedPanels.find(catIndex);
    if (it != m_CategorizedPanels.end()) {
        for (const auto& id : it->second) {
            SetPanelVisible(id, visible);
        }
    }
}

bool UIPanelRegistry::IsPanelVisible(const std::string& id) const {
    auto it = m_Panels.find(id);
    if (it != m_Panels.end()) {
        return it->second.visible;
    }
    return false;
}

void UIPanelRegistry::SavePanelState(const std::string& filepath) {
    using json = nlohmann::json;
    json state;

    for (const auto& [id, entry] : m_Panels) {
        state["panels"][id] = {
            {"visible", entry.visible},
            {"category", static_cast<int>(entry.category)}
        };
    }

    std::ofstream file(filepath);
    if (file.is_open()) {
        file << state.dump(2);
        file.close();
    }
}

void UIPanelRegistry::LoadPanelState(const std::string& filepath) {
    using json = nlohmann::json;
    std::ifstream file(filepath);
    
    if (!file.is_open()) return;

    try {
        json state;
        file >> state;

        if (state.contains("panels")) {
            for (const auto& [id, panelState] : state["panels"].items()) {
                if (panelState.contains("visible")) {
                    SetPanelVisible(id, panelState["visible"].get<bool>());
                }
            }
        }
    } catch (const std::exception&) {
        // JSON parse error, ignore
    }
}

void UIPanelRegistry::Initialize() {
    // Panels are registered individually, no global init needed
}

void UIPanelRegistry::Shutdown() {
    m_Panels.clear();
    m_CategorizedPanels.clear();
}

} // namespace SZM
