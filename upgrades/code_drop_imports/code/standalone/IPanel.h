#pragma once
#include <string>
#include <imgui.h>

class IPanel {
public:
    virtual ~IPanel() = default;

    // Called every frame by the UI Manager
    virtual void Draw() = 0;

    // Unique identifier for the docking system
    virtual std::string GetTitle() const = 0;

    // Panel state
    bool IsVisible = true;
};

