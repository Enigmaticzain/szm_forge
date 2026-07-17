#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "IPanel.hpp"

namespace SZM {

/**
 * @enum PanelCategory
 * @brief Logical grouping of panels for better organization
 */
enum class PanelCategory {
    Viewport,      // 3D view, rendering, visualization
    Inspector,     // Properties, selection, details
    Simulation,    // Simulation control, playback, results
    Tools,         // Toolbar, quick actions, utilities
    Debug,         // Console, performance, diagnostics
    Workshop,      // Assembly, parts, library
    Analysis       // FEA, thermal, results
};

/**
 * @class UIPanelRegistry
 * @brief Centralized panel registration and lifecycle management
 * 
 * Responsibilities:
 *  - Register panels by category
 *  - Manage panel visibility and state
 *  - Provide efficient panel lookup
 *  - Handle panel initialization/shutdown
 */
class UIPanelRegistry {
public:
    static UIPanelRegistry& GetInstance();

    // Registration
    void RegisterPanel(
        const std::string& id,
        std::shared_ptr<IPanel> panel,
        PanelCategory category
    );

    // Lookup
    std::shared_ptr<IPanel> GetPanel(const std::string& id) const;
    std::vector<std::shared_ptr<IPanel>> GetPanelsByCategory(PanelCategory category) const;
    std::vector<std::shared_ptr<IPanel>> GetAllPanels() const;

    // Visibility control
    void SetPanelVisible(const std::string& id, bool visible);
    void SetCategoryVisible(PanelCategory category, bool visible);
    bool IsPanelVisible(const std::string& id) const;

    // State management
    void SavePanelState(const std::string& filepath);
    void LoadPanelState(const std::string& filepath);

    // Lifecycle
    void Initialize();
    void Shutdown();

private:
    UIPanelRegistry() = default;
    ~UIPanelRegistry() = default;

    struct PanelEntry {
        std::shared_ptr<IPanel> panel;
        PanelCategory category;
        bool visible = true;
    };

    static UIPanelRegistry* s_Instance;
    std::unordered_map<std::string, PanelEntry> m_Panels;
    std::unordered_map<int, std::vector<std::string>> m_CategorizedPanels;
};

} // namespace SZM
