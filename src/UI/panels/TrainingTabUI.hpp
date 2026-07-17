#pragma once

#include "../IPanel.hpp"
#include "../../AI/TrainingStudio.hpp"
#include <imgui.h>
#include <vector>
#include <string>

/**
 * @class TrainingTabUI
 * @brief Clean, organized training interface
 */
class TrainingTabUI : public IPanel {
public:
    TrainingTabUI();
    ~TrainingTabUI() override = default;

    void Draw() override;
    std::string GetTitle() const override { return "Training Studio"; }

private:
    // UI Sections
    void DrawDataImportSection();
    void DrawTrainingControlSection();
    void DrawConfigurationSection();
    void DrawMetricsSection();
    void DrawCheckpointsSection();

    // Helpers
    void DrawProgressBar(const char* label, float progress);
    void DrawMetricCard(const char* label, const std::string& value);
    void DrawButton(const char* label, bool& state);

    // State
    bool m_ShowImportDialog = false;
    bool m_IsTraining = false;
    float m_TrainingProgress = 0.0f;
    int m_SelectedTrainingMode = 0;
    
    std::vector<std::string> m_TrainingModes = {
        "Passive Learning",
        "Repository Learning",
        "Architecture Learning",
        "Code Understanding",
        "Simulation Learning"
    };
};
