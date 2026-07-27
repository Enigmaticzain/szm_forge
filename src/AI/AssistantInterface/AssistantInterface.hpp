#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include "../LearningIntegration/LearningIntegration.hpp"

namespace SZM::AI::UI {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CONVERSATION MESSAGE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class MessageRole {
    USER,
    ASSISTANT,
    SYSTEM
};

struct ConversationMessage {
    std::string id;
    MessageRole role;
    std::string content;
    uint64_t timestamp;
    std::string action_id;  // For action suggestions
    bool is_read = false;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ASSISTANT STATE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

struct AssistantState {
    bool is_active = false;
    bool is_listening = false;
    bool is_thinking = false;
    Learning::LearningMode learning_mode = Learning::LearningMode::PASSIVE;
    
    // Statistics
    int patterns_learned = 0;
    float confidence = 0.0f;
    int actions_observed = 0;
    
    // Current context
    std::string current_screen;
    std::string current_app;
    std::vector<std::string> detected_elements;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ASSISTANT INTERFACE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class AssistantInterface {
public:
    static AssistantInterface& GetInstance();

    // Lifecycle
    void Initialize();
    void Shutdown();
    void Update();  // Called every frame
    
    // Rendering
    void Render();
    void RenderMainPanel();
    void RenderHeader();
    void RenderSuggestionBar();
    void RenderQuickActions();
    void RenderStats();
    void RenderSettings();
    void RenderConversationHistory();
    
    // Interaction
    void SendMessage(const std::string& message);
    void AcceptSuggestion(const std::string& suggestion_id);
    void RejectSuggestion(const std::string& suggestion_id);
    void StartTeachingMode();
    void StopTeachingMode();
    
    // State
    AssistantState GetState() const { return m_State; }
    bool IsVisible() const { return m_ShowInterface; }
    void SetVisible(bool visible) { m_ShowInterface = visible; }
    
    // Callbacks
    void SetOnSuggestionAccepted(std::function<void(const std::string&)> callback);
    void SetOnMessageSent(std::function<void(const std::string&)> callback);

private:
    AssistantInterface();
    ~AssistantInterface();

    AssistantInterface(const AssistantInterface&) = delete;
    AssistantInterface& operator=(const AssistantInterface&) = delete;

    // State
    bool m_ShowInterface = true;
    bool m_ShowSettings = false;
    bool m_ShowConversation = true;
    
    AssistantState m_State;
    std::vector<ConversationMessage> m_ConversationHistory;
    std::vector<Learning::AssistantResponse> m_CurrentSuggestions;
    
    // Learning integration reference
    Learning::LearningIntegration& m_Learning = Learning::LearningIntegration::GetInstance();
    
    // Callbacks
    std::function<void(const std::string&)> m_OnSuggestionAccepted;
    std::function<void(const std::string&)> m_OnMessageSent;
    
    // UI state
    char m_InputBuffer[1024] = "";
    ImVec4 m_AssistantColor = ImVec4(0.3f, 0.6f, 0.9f, 1.0f);
    ImVec4 m_UserColor = ImVec4(0.2f, 0.8f, 0.4f, 1.0f);
    ImVec4 m_SystemColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
    
    // Helpers
    void UpdateState();
    void ProcessSuggestions();
    void AddMessage(MessageRole role, const std::string& content);
    void RenderMessage(const ConversationMessage& message);
    void RenderSuggestion(const Learning::AssistantResponse& suggestion);
    void RenderLearningModeSelector();
    void RenderScreenPreview();
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ASSISTANT PANEL (Ready-to-use ImGui panel)
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class AssistantPanel {
public:
    AssistantPanel();
    ~AssistantPanel();
    
    void Initialize();
    void Shutdown();
    void Render();
    
    // Configuration
    void SetPosition(ImVec2 position);
    void SetSize(ImVec2 size);
    void SetAlwaysVisible(bool visible) { m_AlwaysVisible = visible; }
    
    // Callbacks
    void SetOnSuggestionCallback(std::function<void(const std::string&)> callback);
    
private:
    void RenderHeader();
    void RenderBody();
    void RenderFooter();
    void RenderMinimized();
    void RenderExpanded();
    
    bool m_IsExpanded = true;
    bool m_AlwaysVisible = false;
    ImVec2 m_Position = ImVec2(10, 10);
    ImVec2 m_Size = ImVec2(350, 500);
    
    AssistantInterface& m_Interface = AssistantInterface::GetInstance();
    
    std::function<void(const std::string&)> m_OnSuggestionCallback;
    
    // Animation state
    float m_ExpandAnim = 1.0f;
    float m_NewSuggestionPulse = 0.0f;
};

} // namespace SZM::AI::UI
