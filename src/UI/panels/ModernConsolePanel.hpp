#pragma once

#include "../ModernPanelBase.hpp"
#include <vector>
#include <string>

namespace SZM {

/**
 * @class ModernConsolePanel
 * @brief Modern styled console with log filtering and search
 */
class ModernConsolePanel : public ModernPanelBase {
public:
    ModernConsolePanel();
    ~ModernConsolePanel() override = default;

    const char* GetName() const override { return "Modern Console"; }
    std::string GetTitle() const override { return "System Console"; }
    const char* GetIcon() const override { return "💻"; }

    void AddLog(const std::string& message, const std::string& level = "Info");
    void ClearLogs();

protected:
    void DrawContent() override;
    void DrawHeader() override;

private:
    struct LogEntry {
        std::string message;
        std::string level;
        float timestamp;
    };

    void DrawLogFilters();
    void DrawLogEntries();
    void DrawInputField();

    std::vector<LogEntry> m_Logs;
    char m_InputBuffer[512] = "";
    char m_SearchBuffer[256] = "";
    bool m_ScrollToBottom = true;
    bool m_ShowSystem = true;
    bool m_ShowInfo = true;
    bool m_ShowWarning = true;
    bool m_ShowError = true;
};

} // namespace SZM
