#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/BridgeClient.hpp"
#include <chrono>
#include <optional>
#include <string>

/** Shows Python bridge (port 8003) stack health inside the desktop app. */
class EngineeringStackPanel : public IPanel {
public:
    EngineeringStackPanel();
    ~EngineeringStackPanel() override = default;

    void Draw() override;
    std::string GetTitle() const override { return "Engineering Stack"; }
    const char* GetName() const override { return "Engineering Stack"; }

private:
    void RefreshStatus();
    void DrawGroupTable();

    bool m_AutoRefresh = true;
    double m_RefreshIntervalSec = 5.0;
    std::chrono::steady_clock::time_point m_LastRefresh{};
    std::optional<SZM::BridgeClient::StackStatus> m_Status;
    std::string m_StatusMessage;
};
