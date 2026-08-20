#pragma once

#include <string>
#include <vector>
#include "imgui.h"

namespace SZM::Omniverse {

/**
 * @class ImGuiOmniverseLiveSyncPanel
 * @brief ImGui desktop panel for Omniverse LiveSync
 * 
 * Provides UI for session management, change sync, and conflict resolution
 */
class ImGuiOmniverseLiveSyncPanel {
public:
    ImGuiOmniverseLiveSyncPanel();
    
    /**
     * @brief Render the panel
     */
    void Render();
    
    /**
     * @brief Set visibility
     */
    void SetVisible(bool visible);
    
    /**
     * @brief Check if visible
     */
    bool IsVisible() const;

private:
    // Panel state
    bool is_visible_;
    bool show_connect_dialog_;
    bool show_session_manager_;
    bool show_sync_controls_;
    bool show_conflict_resolver_;
    
    // Connection parameters
    char nucleus_url_[256];
    char username_[128];
    char password_[128];
    char project_path_[256];
    char scene_file_[128];
    
    // Session parameters
    char session_id_[64];
    char user_id_[64];
    char display_name_[128];
    
    // Sync parameters
    float sync_interval_;
    int merge_strategy_;  // 0=Last Write, 1=Keep Local, etc.
    bool auto_sync_;
    
    // State
    int active_tab_;
    int selected_session_;
    int selected_conflict_;
    
    // UI methods
    void RenderConnectTab();
    void RenderSessionTab();
    void RenderSyncTab();
    void RenderConflictTab();
    void RenderSettingsTab();
    
    // Helper methods
    void ShowConnectDialog();
    void ShowSessionManager();
    void ShowSyncControls();
    void ShowConflictResolver();
};

} // namespace SZM::Omniverse
