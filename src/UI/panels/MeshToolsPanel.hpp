#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/BridgeClient.hpp"
#include <string>

/** Phase 2 mesh tools — volume mesh generation and tessellation (Group 3). */
class MeshToolsPanel : public IPanel {
public:
    MeshToolsPanel() = default;
    ~MeshToolsPanel() override = default;

    void Draw() override;
    std::string GetTitle() const override { return "Mesh Tools"; }
    const char* GetName() const override { return "Mesh Tools"; }

private:
    void DrawVolumeMesh();
    void DrawTessellate();
    void DrawResultActions();

    float m_LengthMm = 100.f;
    float m_WidthMm = 50.f;
    float m_HeightMm = 20.f;
    float m_MaxElementSize = 0.05f;
    float m_ChordalError = 0.01f;
    int m_PrimitiveIndex = 0;

    char m_GeometryPath[512] = "";
    char m_ResultLog[2048] = "";
    char m_LastFilePath[512] = "";
    bool m_BridgeOnline = false;
};
