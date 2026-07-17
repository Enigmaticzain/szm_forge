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

// UIManager.cpp

// ===== NEW ADDITIONS FROM CODE BUNDLES =====

class UIManager {
public:
    UIManager();
    ~UIManager();

    void Initialize();
    void RegisterPanel(std::shared_ptr<IPanel> panel);
    void RenderUI();
    void BuildDefaultDockspace(unsigned int dockspaceId);

private:
    std::vector<std::shared_ptr<IPanel>> m_Panels;
    bool m_FirstTimeDocking = true;
};
""",

    "SZM_Forge_UI/src/UIManager.cpp": """// SZM_Forge_UI/src/UIManager.cpp