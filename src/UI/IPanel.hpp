#pragma once

#include <string>

namespace SZM {

    class IPanel {
    public:
        virtual ~IPanel() = default;

        // Called every frame by the UI Manager
        virtual void Draw() = 0;

        // Unique identifier for the docking system
        virtual std::string GetTitle() const { return GetName(); }

        // Backward-compatible name accessor used by older panel implementations
        virtual const char* GetName() const { return "Panel"; }

        // Panel state
        bool IsVisible = true;

    protected:
        bool& m_IsOpen = IsVisible;
    };

} // namespace SZM

// Backward-compatibility alias
using IPanel = SZM::IPanel;
