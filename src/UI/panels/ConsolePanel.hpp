#pragma once

#include "../IPanel.hpp"
#include <vector>
#include <string>

/**
 * @class ConsolePanel
 * @brief Bottom panel — SZM-CEA AI Assistant console.
 *
 * Receives log messages from any system via AddLog().
 * Also accepts typed commands routed to the CEA AI module.
 *
 * Log levels: [System], [Info], [Warning], [Error], [CEA]
 */
class ConsolePanel : public IPanel {
public:
    ConsolePanel();

    void        Draw()     override;
    std::string GetTitle() const override { return "SZM-CEA Console"; }

    void AddLog(const std::string& message);
    void ClearLogs();

private:
    std::vector<std::string> m_Logs;
    char                     m_InputBuffer[256] = "";
    bool                     m_ScrollToBottom   = false;
};
