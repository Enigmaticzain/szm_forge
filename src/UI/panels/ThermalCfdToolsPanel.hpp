#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/BridgeClient.hpp"
#include <string>

class ThermalCfdToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "Thermal/CFD Tools"; }
    const char* GetName() const override { return "Thermal/CFD Tools"; }

private:
    void DrawThermal();
    void DrawCfd();
    void DrawFluidLookup();

    int m_ComponentId = 1;
    int m_ModeIndex = 0;
    float m_AmbientC = 20.f;
    float m_HeatW = 100.f;
    float m_Convection = 10.f;
    float m_InletVelocity = 10.f;
    char m_FluidName[32] = "Water";
    float m_FluidTempK = 300.f;
    char m_ResultLog[2048] = "";
    bool m_BridgeOnline = false;
};
