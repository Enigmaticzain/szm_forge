#pragma once
#include <vector>
#include <memory>
#include "IPanel.h"

class UIManager {
public:
    UIManager();
    ~UIManager();

    void Initialize();
    void RegisterPanel(std::shared_ptr<IPanel> panel);
    
    // Main render loop call
    void RenderUI();

    // Forces a specific layout if no save file is found
    void BuildDefaultDockspace(uint32_t dockspaceId);

private:
    std::vector<std::shared_ptr<IPanel>> m_Panels;
    bool m_FirstTimeDocking = true;
};

