#include "OmniverseLiveSyncPanel.hpp"
#include <cstring>
#include <iostream>

namespace SZM::Omniverse {

ImGuiOmniverseLiveSyncPanel::ImGuiOmniverseLiveSyncPanel()
    : is_visible_(false),
      show_connect_dialog_(false),
      show_session_manager_(false),
      show_sync_controls_(false),
      show_conflict_resolver_(false),
      sync_interval_(1.0f),
      merge_strategy_(0),
      auto_sync_(false),
      active_tab_(0),
      selected_session_(-1),
      selected_conflict_(-1) {
    
    // Initialize strings
    std::strcpy(nucleus_url_, "http://localhost:3100");
    std::strcpy(username_, "admin");
    std::strcpy(password_, "");
    std::strcpy(project_path_, "/projects/default");
    std::strcpy(scene_file_, "scene.usd");
    std::strcpy(session_id_, "");
    std::strcpy(user_id_, "");
    std::strcpy(display_name_, "");
    
    std::cout << "[ImGuiOmniverseLiveSyncPanel] Initialized" << std::endl;
}

void ImGuiOmniverseLiveSyncPanel::Render() {
    if (!is_visible_) return;
    
    ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Omniverse LiveSync", &is_visible_, ImGuiWindowFlags_NoMove)) {
        ImGui::Text("🌌 Real-time Collaborative Editing");
        ImGui::Separator();
        
        // Tab bar
        if (ImGui::BeginTabBar("OmniverseTabs")) {
            if (ImGui::BeginTabItem("Connect")) {
                RenderConnectTab();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Session")) {
                RenderSessionTab();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Sync")) {
                RenderSyncTab();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Conflicts")) {
                RenderConflictTab();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Settings")) {
                RenderSettingsTab();
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    
    ImGui::End();
}

void ImGuiOmniverseLiveSyncPanel::RenderConnectTab() {
    ImGui::Text("Connect to Nucleus Server");
    ImGui::Separator();
    
    ImGui::Text("Nucleus Configuration");
    ImGui::InputText("Nucleus URL##url", nucleus_url_, sizeof(nucleus_url_));
    ImGui::InputText("Username##user", username_, sizeof(username_));
    ImGui::InputText("Password##pwd", password_, sizeof(password_), ImGuiInputTextFlags_Password);
    ImGui::InputText("Project Path##proj", project_path_, sizeof(project_path_));
    ImGui::InputText("Scene File##scene", scene_file_, sizeof(scene_file_));
    
    ImGui::Spacing();
    ImGui::Separator();
    
    if (ImGui::Button("Connect to Nucleus", ImVec2(200, 0))) {
        show_connect_dialog_ = true;
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Connecting to " << nucleus_url_ << std::endl;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Test Connection", ImVec2(200, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Testing connection to " << nucleus_url_ << std::endl;
    }
    
    if (show_connect_dialog_) {
        ImGui::OpenPopup("Connection Dialog");
    }
    
    // Connection status dialog
    if (ImGui::BeginPopupModal("Connection Dialog", &show_connect_dialog_)) {
        ImGui::Text("Connecting to Nucleus Server...");
        ImGui::ProgressBar(0.5f, ImVec2(-1, 0));
        
        if (ImGui::Button("OK", ImVec2(100, 0))) {
            show_connect_dialog_ = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void ImGuiOmniverseLiveSyncPanel::RenderSessionTab() {
    ImGui::Text("Session Management");
    ImGui::Separator();
    
    ImGui::Text("Session Details");
    ImGui::InputText("Session ID##sess", session_id_, sizeof(session_id_));
    ImGui::InputText("User ID##uid", user_id_, sizeof(user_id_));
    ImGui::InputText("Display Name##disp", display_name_, sizeof(display_name_));
    
    ImGui::Spacing();
    ImGui::Separator();
    
    if (ImGui::Button("Join Session", ImVec2(150, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] User " << user_id_ 
                  << " joining session " << session_id_ << std::endl;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Leave Session", ImVec2(150, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] User " << user_id_ 
                  << " leaving session " << session_id_ << std::endl;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Close Session", ImVec2(150, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Closing session " << session_id_ << std::endl;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Connected Users");
    
    // Simulate user list
    ImGui::BeginChild("UsersList", ImVec2(0, 150), true);
    ImGui::Text("user_admin");
    ImGui::Text("user_artist");
    ImGui::Text("user_developer");
    ImGui::EndChild();
    
    ImGui::Spacing();
    ImGui::Text("Session Statistics");
    ImGui::Columns(2, "stats");
    ImGui::Text("State:"); ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Connected");
    ImGui::NextColumn();
    ImGui::Text("Users:"); ImGui::NextColumn();
    ImGui::Text("3");
    ImGui::NextColumn();
    ImGui::Text("Changes:"); ImGui::NextColumn();
    ImGui::Text("42");
    ImGui::NextColumn();
    ImGui::Text("Latency:"); ImGui::NextColumn();
    ImGui::Text("50ms");
    ImGui::Columns(1);
}

void ImGuiOmniverseLiveSyncPanel::RenderSyncTab() {
    ImGui::Text("Synchronization Control");
    ImGui::Separator();
    
    ImGui::Checkbox("Auto Sync Enabled", &auto_sync_);
    ImGui::SliderFloat("Sync Interval (s)", &sync_interval_, 0.1f, 10.0f);
    
    ImGui::Spacing();
    ImGui::Separator();
    
    if (ImGui::Button("Pull Remote Changes", ImVec2(200, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Pulling remote changes" << std::endl;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Push Local Changes", ImVec2(200, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Pushing local changes" << std::endl;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Force Sync", ImVec2(200, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Forcing sync" << std::endl;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Change History");
    
    ImGui::BeginChild("ChangeHistory", ImVec2(0, 200), true);
    
    // Simulate change list
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ Property Changed: /scene/cube.translate");
    ImGui::Text("  Old: 0 0 0 -> New: 1 0 0");
    ImGui::Text("  User: user_artist (2ms ago)");
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ Prim Created: /scene/sphere");
    ImGui::Text("  User: user_developer (15ms ago)");
    
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "✓ Metadata Changed: root.metadata");
    ImGui::Text("  User: user_admin (45ms ago)");
    
    ImGui::EndChild();
    
    ImGui::Spacing();
    ImGui::Text("Sync Statistics");
    ImGui::Text("Total Changes: 1,247");
    ImGui::Text("Pending: 5");
    ImGui::Text("Average Latency: 45ms");
}

void ImGuiOmniverseLiveSyncPanel::RenderConflictTab() {
    ImGui::Text("Conflict Resolution");
    ImGui::Separator();
    
    if (ImGui::Button("Detect Conflicts", ImVec2(200, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Detecting conflicts" << std::endl;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Refresh", ImVec2(100, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Refreshing conflicts" << std::endl;
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Detected Conflicts: 2");
    
    ImGui::BeginChild("ConflictsList", ImVec2(0, 200), true);
    
    // Conflict 1
    bool conflict1_selected = selected_conflict_ == 0;
    if (ImGui::Selectable("Conflict #1: /scene/cube.scale", conflict1_selected)) {
        selected_conflict_ = 0;
    }
    if (conflict1_selected) {
        ImGui::BulletText("Local: 1.0 1.0 1.0");
        ImGui::BulletText("Remote: 2.0 2.0 2.0");
    }
    
    ImGui::Spacing();
    
    // Conflict 2
    bool conflict2_selected = selected_conflict_ == 1;
    if (ImGui::Selectable("Conflict #2: /scene/material.color", conflict2_selected)) {
        selected_conflict_ = 1;
    }
    if (conflict2_selected) {
        ImGui::BulletText("Local: RGB(1, 0, 0)");
        ImGui::BulletText("Remote: RGB(0, 1, 0)");
    }
    
    ImGui::EndChild();
    
    if (selected_conflict_ >= 0) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Resolve Conflict");
        
        const char* strategies[] = {
            "Last Write Wins",
            "Keep Local",
            "Merge Properties",
            "Manual Review"
        };
        ImGui::Combo("Merge Strategy##strat", &merge_strategy_, strategies, 4);
        
        if (ImGui::Button("Resolve Selected Conflict", ImVec2(200, 0))) {
            std::cout << "[ImGuiOmniverseLiveSyncPanel] Resolving conflict with strategy " 
                      << merge_strategy_ << std::endl;
        }
    }
}

void ImGuiOmniverseLiveSyncPanel::RenderSettingsTab() {
    ImGui::Text("Omniverse LiveSync Settings");
    ImGui::Separator();
    
    ImGui::Text("Synchronization");
    ImGui::Checkbox("Enable Auto Sync", &auto_sync_);
    ImGui::SliderFloat("Sync Interval (seconds)", &sync_interval_, 0.1f, 10.0f);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Conflict Resolution");
    
    const char* merge_options[] = {
        "Last Write Wins",
        "Keep Local",
        "Merge Properties",
        "Manual"
    };
    ImGui::Combo("Default Merge Strategy", &merge_strategy_, merge_options, 4);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Network");
    ImGui::SliderFloat("Request Timeout (s)", &sync_interval_, 0.5f, 30.0f);
    ImGui::Checkbox("Enable Compression", (bool*)&merge_strategy_);
    ImGui::Checkbox("Enable Caching", (bool*)&auto_sync_);
    
    ImGui::Spacing();
    ImGui::Separator();
    
    if (ImGui::Button("Save Settings", ImVec2(150, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Saving settings" << std::endl;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset to Defaults", ImVec2(150, 0))) {
        std::cout << "[ImGuiOmniverseLiveSyncPanel] Resetting to defaults" << std::endl;
    }
}

void ImGuiOmniverseLiveSyncPanel::SetVisible(bool visible) {
    is_visible_ = visible;
}

bool ImGuiOmniverseLiveSyncPanel::IsVisible() const {
    return is_visible_;
}

} // namespace SZM::Omniverse
