#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/BridgeClient.hpp"
#include <string>

/** Phase 3 FEA tools — cantilever beam analysis via CalculiX or analytical fallback (Group 4). */
class FeaToolsPanel : public IPanel {
public:
    FeaToolsPanel() = default;
    ~FeaToolsPanel() override = default;

    void Draw() override;
    std::string GetTitle() const override { return "FEA Tools"; }
    const char* GetName() const override { return "FEA Tools"; }

private:
    void DrawRunAnalysis();
    void DrawResults();

    int m_ComponentId = 1;
    float m_ForceN = 1000.f;
    float m_AreaMm2 = 500.f;
    float m_ThicknessMm = 200.f;
    float m_YoungsGpa = 210.f;
    float m_YieldMpa = 250.f;
    float m_Density = 7850.f;

    char m_ResultLog[2048] = "";
    char m_MeshPath[512] = "";
    SZM::BridgeClient::FeaJobResult m_LastResult{};
    bool m_BridgeOnline = false;
};
