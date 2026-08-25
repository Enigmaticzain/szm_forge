#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/BridgeClient.hpp"
#include <string>

class OptimizationToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "Optimization Tools"; }
    const char* GetName() const override { return "Optimization Tools"; }
private:
    float m_InitialStress = 847.f, m_InitialMass = 12.4f, m_TargetStress = 800.f;
    int m_Iterations = 50;
    char m_Log[1024] = "";
};

class MaterialsToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "Materials Tools"; }
    const char* GetName() const override { return "Materials Tools"; }
private:
    char m_Formula[64] = "Fe";
    char m_NistQuery[64] = "steel";
    char m_Log[1024] = "";
};

class ManufacturingToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "Manufacturing Tools"; }
    const char* GetName() const override { return "Manufacturing Tools"; }
private:
    int m_ComponentId = 1;
    char m_PartName[64] = "Workshop Part";
    float m_LayerHeight = 0.2f;
    char m_Log[1024] = "";
};

class RoboticsToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "Robotics Tools"; }
    const char* GetName() const override { return "Robotics Tools"; }
private:
    char m_UrdfSnippet[512] = "<robot name=\"demo\"><link name=\"base\"/></robot>";
    int m_SimSteps = 240;
    char m_Log[1024] = "";
};

class IoTToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "IoT Tools"; }
    const char* GetName() const override { return "IoT Tools"; }
private:
    char m_Broker[64] = "localhost";
    char m_Topic[128] = "szm/telemetry/#";
    int m_Port = 1883;
    char m_Log[512] = "";
};

class PhysicsMLToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "Physics ML Tools"; }
    const char* GetName() const override { return "Physics ML Tools"; }
private:
    char m_ModelName[64] = "default";
    int m_Epochs = 100;
    char m_Log[1024] = "";
};

class PipelineToolsPanel : public IPanel {
public:
    void Draw() override;
    std::string GetTitle() const override { return "Pipeline Tools"; }
    const char* GetName() const override { return "Pipeline Tools"; }
private:
    char m_GeometryPath[512] = "/tmp/szm_export.step";
    char m_JobId[128] = "";
    char m_Log[1024] = "";
};
