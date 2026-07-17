#pragma once

#include "IPanel.hpp"
#include <vector>
#include <string>

namespace SZM {

    class ConsolePanel : public IPanel {
    public:
        ConsolePanel();
        ~ConsolePanel() = default;

        void Draw() override;
        std::string GetTitle() const override { return "SZM-CEA Console"; }

        void AddLog(const std::string& log);
        void Clear();

    private:
        std::vector<std::string> m_Logs;
        char m_InputBuffer[256] = "";
        bool m_AutoScroll = true;
    };

} // namespace SZM
