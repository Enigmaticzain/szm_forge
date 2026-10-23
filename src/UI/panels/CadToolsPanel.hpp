#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/BridgeClient.hpp"
#include <string>

/** Phase 1 CAD tools — text-to-CAD, STEP export, geometric fit (Group 2). */
class CadToolsPanel : public IPanel {
public:
    CadToolsPanel() = default;
    ~CadToolsPanel() override = default;

    void Draw() override;
    std::string GetTitle() const override { return "CAD Tools"; }
    const char* GetName() const override { return "CAD Tools"; }

private:
    void DrawTextToCad();
    void DrawExportBox();
    void DrawGeometricFit();
    void DrawResultActions();

    char m_Prompt[512] = "100mm aluminum bracket with two holes";
    char m_OutputDir[256] = "/tmp";
    int m_T2CadBackendIndex = 0;
    int m_ExportBackendIndex = 0;

    char m_Shape[32] = "box";
    float m_Length = 100.f;
    float m_Width = 50.f;
    float m_Height = 20.f;

    char m_ResultLog[2048] = "";
    char m_LastFilePath[512] = "";
    bool m_BridgeOnline = false;
};
