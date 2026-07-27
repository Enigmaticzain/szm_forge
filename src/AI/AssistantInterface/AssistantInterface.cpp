#include "AssistantInterface.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace SZM::AI::UI {

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ASSISTANT INTERFACE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

AssistantInterface& AssistantInterface::GetInstance() {
    static AssistantInterface instance;
    return instance;
}

AssistantInterface::AssistantInterface() = default;

AssistantInterface::~AssistantInterface() {
    Shutdown();
}

void AssistantInterface::Initialize() {
    std::cout << "[AssistantInterface] Initializing..." << std::endl;
    
    // Initialize learning integration
    m_Learning.Initialize();
    
    // Set up callbacks
    m_Learning.SetScreenAnalysisCallback([this](const Learning::ScreenUnderstanding& analysis) {
        m_State.current_screen = analysis.screen_description;
        m_State.detected_elements = analysis.active_elements;
    });
    
    m_Learning.SetSuggestionCallback([this](const Learning::AssistantResponse& suggestion) {
        m_CurrentSuggestions.push_back(suggestion);
        
        // Add to conversation as a suggestion
        ConversationMessage msg;
        msg.id = suggestion.id;
        msg.role = MessageRole::ASSISTANT;
        msg.content = suggestion.message;
        msg.action_id = suggestion.id;
        msg.timestamp = suggestion.timestamp;
        m_ConversationHistory.push_back(msg);
    });
    
    // Start in passive mode
    m_Learning.SetLearningMode(Learning::LearningMode::PASSIVE);
    m_State.learning_mode = Learning::LearningMode::PASSIVE;
    
    // Add welcome message
    AddMessage(MessageRole::SYSTEM, 
        "AI Assistant ready. I'm watching and learning from your actions.\n"
        "Switch to TEACHING mode to demonstrate tasks, or ACTIVE mode for suggestions.");
    
    std::cout << "[AssistantInterface] Initialized successfully" << std::endl;
}

void AssistantInterface::Shutdown() {
    m_Learning.Shutdown();
    std::cout << "[AssistantInterface] Shutdown complete" << std::endl;
}

void AssistantInterface::Update() {
    // Update learning system
    m_Learning.Update();
    
    // Update local state
    UpdateState();
    
    // Process suggestions
    ProcessSuggestions();
}

void AssistantInterface::UpdateState() {
    auto stats = m_Learning.GetStatistics();
    
    m_State.is_active = stats.value("is_active", false);
    m_State.learning_mode = m_Learning.GetLearningMode();
    m_State.patterns_learned = stats.value("total_learned_patterns", 0);
    m_State.confidence = stats.value("overall_confidence", 0.0f);
    m_State.actions_observed = stats.value("observation", json::object())
                                  .value("total_actions", 0);
    
    // Update listening state based on learning mode
    m_State.is_listening = (m_State.learning_mode == Learning::LearningMode::PASSIVE ||
                           m_State.learning_mode == Learning::LearningMode::TEACHING);
    
    // Get current screen info
    try {
        auto understanding = m_Learning.AnalyzeCurrentScreen();
        m_State.current_app = understanding.detected_apps.empty() ? 
                              "Unknown" : understanding.detected_apps[0];
    } catch (...) {
        m_State.current_app = "Unknown";
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// RENDERING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void AssistantInterface::Render() {
    if (!m_ShowInterface) return;
    
    RenderMainPanel();
}

void AssistantInterface::RenderMainPanel() {
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("AI Assistant", &m_ShowInterface, 
                      ImGuiWindowFlags_NoScrollbar | 
                      ImGuiWindowFlags_NoScrollWithMouse)) {
        ImGui::End();
        return;
    }
    
    // Header with status
    RenderHeader();
    ImGui::Separator();
    
    // Learning mode selector
    RenderLearningModeSelector();
    ImGui::Spacing();
    
    // Quick actions
    RenderQuickActions();
    ImGui::Spacing();
    
    // Conversation history
    if (m_ShowConversation) {
        RenderConversationHistory();
    }
    
    // Suggestions bar
    RenderSuggestionBar();
    
    // Stats (collapsible)
    if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderStats();
    }
    
    // Settings
    if (m_ShowSettings) {
        RenderSettings();
    }
    
    ImGui::End();
}

void AssistantInterface::RenderHeader() {
    ImGui::BeginGroup();
    
    // Status indicator
    ImVec4 status_color;
    if (m_State.is_listening) {
        status_color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);  // Green
    } else if (m_State.is_thinking) {
        status_color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);  // Yellow
    } else {
        status_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);  // Gray
    }
    
    ImGui::Text("Status: ");
    ImGui::SameLine();
    ImGui::TextColored(status_color, m_State.is_listening ? "LISTENING" : 
                                        m_State.is_thinking ? "THINKING" : "IDLE");
    
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    
    ImGui::Text("App: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), 
                       m_State.current_app.c_str());
    
    // Mode indicator
    std::string mode_str;
    switch (m_State.learning_mode) {
        case Learning::LearningMode::PASSIVE:    mode_str = "Passive"; break;
        case Learning::LearningMode::ACTIVE:      mode_str = "Active"; break;
        case Learning::LearningMode::SUPERVISED: mode_str = "Supervised"; break;
        case Learning::LearningMode::AUTONOMOUS: mode_str = "Autonomous"; break;
        case Learning::LearningMode::TEACHING:    mode_str = "Teaching"; break;
    }
    
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::Text("Mode: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), mode_str.c_str());
    
    ImGui::EndGroup();
    
    // Settings button
    ImGui::SameLine(ImGui::GetWindowWidth() - 80);
    if (ImGui::Button(m_ShowSettings ? "Hide Settings" : "Settings")) {
        m_ShowSettings = !m_ShowSettings;
    }
}

void AssistantInterface::RenderLearningModeSelector() {
    ImGui::Text("Learning Mode:");
    ImGui::SameLine();
    
    const char* mode_names[] = {
        "Passive (Watch & Learn)",
        "Active (Suggest Actions)",
        "Supervised (Guided Learning)",
        "Autonomous (AI Takes Actions)",
        "Teaching (User Demonstrates)"
    };
    
    int current_mode = static_cast<int>(m_State.learning_mode);
    if (ImGui::Combo("##LearningMode", &current_mode, mode_names, 5)) {
        m_Learning.SetLearningMode(static_cast<Learning::LearningMode>(current_mode));
        m_State.learning_mode = static_cast<Learning::LearningMode>(current_mode);
        
        // Add system message
        std::string msg = "Switched to " + std::string(mode_names[current_mode]) + " mode";
        AddMessage(MessageRole::SYSTEM, msg);
    }
    
    // Mode descriptions
    ImGui::BeginChild("ModeDesc", ImVec2(0, 50), true);
    switch (m_State.learning_mode) {
        case Learning::LearningMode::PASSIVE:
            ImGui::TextWrapped("I watch your actions and learn patterns. No interference.");
            break;
        case Learning::LearningMode::ACTIVE:
            ImGui::TextWrapped("I suggest actions based on learned patterns. You decide.");
            break;
        case Learning::LearningMode::SUPERVISED:
            ImGui::TextWrapped("I learn from your explicit guidance and corrections.");
            break;
        case Learning::LearningMode::AUTONOMOUS:
            ImGui::TextWrapped("I take actions and learn from outcomes. Use with caution!");
            break;
        case Learning::LearningMode::TEACHING:
            ImGui::TextWrapped("Demonstrate tasks and I'll learn from your examples.");
            break;
    }
    ImGui::EndChild();
}

void AssistantInterface::RenderQuickActions() {
    ImGui::Text("Quick Actions:");
    
    ImGui::BeginGroup();
    
    if (ImGui::Button("Capture Screen")) {
        auto capture = Vision::VisionSystem::GetInstance().CaptureScreen();
        std::string desc = Vision::VisionSystem::GetInstance().DescribeScreen(capture.image);
        AddMessage(MessageRole::ASSISTANT, "Screen captured: " + desc);
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Analyze Context")) {
        std::string context = m_Learning.UnderstandScreenContext();
        AddMessage(MessageRole::ASSISTANT, "Current context: " + context);
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Show Patterns")) {
        auto patterns = m_Learning.GetLearnedSkills();
        std::ostringstream oss;
        oss << "Learned " << patterns.size() << " patterns:\n";
        for (size_t i = 0; i < std::min((size_t)5, patterns.size()); i++) {
            oss << "• " << patterns[i].description << "\n";
        }
        AddMessage(MessageRole::ASSISTANT, oss.str());
    }
    
    ImGui::EndGroup();
}

void AssistantInterface::RenderConversationHistory() {
    if (ImGui::CollapsingHeader("Conversation", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        // Toggle button
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        if (ImGui::Button(m_ShowConversation ? "Hide" : "Show")) {
            m_ShowConversation = !m_ShowConversation;
        }
        
        ImGui::BeginChild("ConversationArea", ImVec2(0, 200), true);
        
        for (const auto& msg : m_ConversationHistory) {
            RenderMessage(msg);
        }
        
        ImGui::EndChild();
        
        // Input field
        ImGui::Separator();
        ImGui::InputTextMultiline("##Input", m_InputBuffer, sizeof(m_InputBuffer),
                                   ImVec2(-80, 60));
        
        ImGui::SameLine();
        if (ImGui::Button("Send", ImVec2(70, 0))) {
            if (strlen(m_InputBuffer) > 0) {
                SendMessage(m_InputBuffer);
                memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
            }
        }
    }
}

void AssistantInterface::RenderMessage(const ConversationMessage& message) {
    ImVec4 color;
    const char* prefix;
    
    switch (message.role) {
        case MessageRole::USER:
            color = m_UserColor;
            prefix = "You: ";
            break;
        case MessageRole::ASSISTANT:
            color = m_AssistantColor;
            prefix = "AI: ";
            break;
        case MessageRole::SYSTEM:
            color = m_SystemColor;
            prefix = "[System] ";
            break;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    
    ImGui::Text(prefix);
    ImGui::SameLine();
    ImGui::TextWrapped(message.content.c_str());
    
    ImGui::PopStyleColor();
    
    // If it's a suggestion, show accept/reject buttons
    if (!message.action_id.empty() && message.role == MessageRole::ASSISTANT) {
        ImGui::Indent();
        if (ImGui::Button(("Accept##" + message.id).c_str())) {
            AcceptSuggestion(message.action_id);
        }
        ImGui::SameLine();
        if (ImGui::Button(("Dismiss##" + message.id).c_str())) {
            RejectSuggestion(message.action_id);
        }
        ImGui::Unindent();
    }
    
    ImGui::Spacing();
}

void AssistantInterface::RenderSuggestionBar() {
    if (m_CurrentSuggestions.empty()) return;
    
    ImGui::Separator();
    ImGui::Text("Suggestions:");
    ImGui::BeginGroup();
    
    for (const auto& suggestion : m_CurrentSuggestions) {
        RenderSuggestion(suggestion);
    }
    
    ImGui::EndGroup();
}

void AssistantInterface::RenderSuggestion(const Learning::AssistantResponse& suggestion) {
    ImGui::BeginGroup();
    
    // Suggestion card
    ImGui::BeginChild(("Suggestion_" + suggestion.id).c_str(), 
                       ImVec2(0, 70), true);
    
    // Confidence indicator
    ImVec4 conf_color;
    if (suggestion.confidence > 0.8f) {
        conf_color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
    } else if (suggestion.confidence > 0.5f) {
        conf_color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
    } else {
        conf_color = ImVec4(1.0f, 0.4f, 0.2f, 1.0f);
    }
    
    ImGui::TextColored(conf_color, "%.0f%% confident", suggestion.confidence * 100);
    
    ImGui::TextWrapped(suggestion.message.c_str());
    
    if (ImGui::Button(("Accept##" + suggestion.id).c_str())) {
        AcceptSuggestion(suggestion.id);
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button(("Dismiss##" + suggestion.id).c_str())) {
        RejectSuggestion(suggestion.id);
    }
    
    ImGui::EndChild();
    ImGui::EndGroup();
}

void AssistantInterface::RenderStats() {
    ImGui::Columns(2);
    
    // Left column
    ImGui::Text("Patterns Learned:");
    ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), 
                       "%d", m_State.patterns_learned);
    
    ImGui::NextColumn();
    
    // Right column
    ImGui::Text("Actions Observed:");
    ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), 
                       "%d", m_State.actions_observed);
    
    ImGui::NextColumn();
    
    ImGui::Text("Overall Confidence:");
    ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), 
                       "%.1f%%", m_State.confidence * 100);
    
    ImGui::NextColumn();
    
    ImGui::Text("Current Screen:");
    ImGui::TextWrapped(m_State.current_screen.c_str());
    
    ImGui::Columns(1);
}

void AssistantInterface::RenderSettings() {
    ImGui::Separator();
    ImGui::Text("Settings:");
    
    // Auto-save toggle
    static bool auto_save = true;
    if (ImGui::Checkbox("Auto-save patterns", &auto_save)) {
        Learning::LearningConfig config = m_Learning.GetConfig();
        config.auto_save = auto_save;
        m_Learning.SetConfig(config);
    }
    
    // Confidence threshold
    Learning::LearningConfig config = m_Learning.GetConfig();
    float threshold = config.confidence_threshold;
    if (ImGui::SliderFloat("Confidence threshold", &threshold, 0.0f, 1.0f)) {
        config.confidence_threshold = threshold;
        m_Learning.SetConfig(config);
    }
    
    // Continuous learning
    static bool continuous = true;
    if (ImGui::Checkbox("Continuous learning", &continuous)) {
        config.continuous_learning = continuous;
        m_Learning.SetConfig(config);
    }
    
    // Clear data buttons
    if (ImGui::Button("Clear Conversation")) {
        m_ConversationHistory.clear();
        AddMessage(MessageRole::SYSTEM, "Conversation cleared.");
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Save State")) {
        m_Learning.AutoSave();
        AddMessage(MessageRole::SYSTEM, "State saved.");
    }
}

void AssistantInterface::RenderScreenPreview() {
    ImGui::Text("Current Screen:");
    
    ImGui::BeginChild("ScreenPreview", ImVec2(0, 150), true);
    
    try {
        auto capture = Vision::VisionSystem::GetInstance().CaptureScreen();
        
        // Convert cv::Mat to ImGui texture
        // In production, you'd cache this texture
        ImGui::Text("Screenshot captured (%dx%d)", 
                     capture.image.cols, capture.image.rows);
        
        // Display detected elements
        ImGui::Text("Detected %zu elements", capture.detected_elements.size());
        
    } catch (const std::exception& e) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), 
                           "Error: %s", e.what());
    }
    
    ImGui::EndChild();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// INTERACTION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void AssistantInterface::SendMessage(const std::string& message) {
    if (message.empty()) return;
    
    // Add user message
    AddMessage(MessageRole::USER, message);
    
    // Trigger callback
    if (m_OnMessageSent) {
        m_OnMessageSent(message);
    }
    
    // Generate response based on context
    std::ostringstream response;
    
    // Simple response logic - in production this would use the actual AI
    if (message.find("help") != std::string::npos) {
        response << "I can help you by:\n";
        response << "• Watching and learning from your actions\n";
        response << "• Suggesting next steps based on patterns\n";
        response << "• Automating repetitive tasks\n";
        response << "• Answering questions about the current screen\n";
        response << "\nTry switching to TEACHING mode and showing me a task!";
    } else if (message.find("status") != std::string::npos) {
        response << "Current status:\n";
        response << "• Mode: " << (m_State.is_listening ? "Listening" : "Idle") << "\n";
        response << "• Patterns learned: " << m_State.patterns_learned << "\n";
        response << "• Actions observed: " << m_State.actions_observed << "\n";
        response << "• Confidence: " << (int)(m_State.confidence * 100) << "%";
    } else if (message.find("what") != std::string::npos && 
               message.find("see") != std::string::npos) {
        response << "I see: " << m_State.current_screen;
        if (!m_State.detected_elements.empty()) {
            response << "\n\nElements I can see:\n";
            for (size_t i = 0; i < std::min((size_t)5, m_State.detected_elements.size()); i++) {
                response << "• " << m_State.detected_elements[i] << "\n";
            }
        }
    } else {
        response << "I understand you're saying: \"" << message << "\"\n";
        response << "\nI'm currently in " << (m_State.is_listening ? "watching" : "idle") << " mode.\n";
        response << "Switch to ACTIVE mode to get suggestions or TEACHING mode to show me tasks.";
    }
    
    AddMessage(MessageRole::ASSISTANT, response.str());
}

void AssistantInterface::AcceptSuggestion(const std::string& suggestion_id) {
    m_Learning.ApplySuggestion(suggestion_id);
    
    // Remove from current suggestions
    m_CurrentSuggestions.erase(
        std::remove_if(m_CurrentSuggestions.begin(), m_CurrentSuggestions.end(),
            [&suggestion_id](const Learning::AssistantResponse& s) { 
                return s.id == suggestion_id; 
            }),
        m_CurrentSuggestions.end()
    );
    
    AddMessage(MessageRole::SYSTEM, "Suggestion accepted and applied.");
    
    if (m_OnSuggestionAccepted) {
        m_OnSuggestionAccepted(suggestion_id);
    }
}

void AssistantInterface::RejectSuggestion(const std::string& suggestion_id) {
    // Remove from current suggestions
    m_CurrentSuggestions.erase(
        std::remove_if(m_CurrentSuggestions.begin(), m_CurrentSuggestions.end(),
            [&suggestion_id](const Learning::AssistantResponse& s) { 
                return s.id == suggestion_id; 
            }),
        m_CurrentSuggestions.end()
    );
    
    AddMessage(MessageRole::SYSTEM, "Suggestion dismissed.");
}

void AssistantInterface::StartTeachingMode() {
    m_Learning.SetLearningMode(Learning::LearningMode::TEACHING);
    m_State.learning_mode = Learning::LearningMode::TEACHING;
    AddMessage(MessageRole::SYSTEM, 
        "Teaching mode started. Demonstrate tasks and I'll learn from your actions!");
}

void AssistantInterface::StopTeachingMode() {
    m_Learning.SetLearningMode(Learning::LearningMode::PASSIVE);
    m_State.learning_mode = Learning::LearningMode::PASSIVE;
    AddMessage(MessageRole::SYSTEM, "Teaching mode stopped. Switching to passive observation.");
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// HELPERS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void AssistantInterface::ProcessSuggestions() {
    // Get fresh suggestions from learning system
    auto suggestions = m_Learning.GetSuggestions();
    
    for (const auto& suggestion : suggestions) {
        // Check if we already have this suggestion
        bool exists = false;
        for (const auto& existing : m_CurrentSuggestions) {
            if (existing.id == suggestion.id) {
                exists = true;
                break;
            }
        }
        
        if (!exists && suggestion.confidence > 0.5f) {
            m_CurrentSuggestions.push_back(suggestion);
        }
    }
}

void AssistantInterface::AddMessage(MessageRole role, const std::string& content) {
    ConversationMessage msg;
    msg.id = "msg_" + std::to_string(m_ConversationHistory.size());
    msg.role = role;
    msg.content = content;
    msg.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    m_ConversationHistory.push_back(msg);
    
    // Keep history manageable
    while (m_ConversationHistory.size() > 100) {
        m_ConversationHistory.erase(m_ConversationHistory.begin());
    }
}

void AssistantInterface::SetOnSuggestionAccepted(
    std::function<void(const std::string&)> callback) {
    m_OnSuggestionAccepted = callback;
}

void AssistantInterface::SetOnMessageSent(
    std::function<void(const std::string&)> callback) {
    m_OnMessageSent = callback;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ASSISTANT PANEL
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

AssistantPanel::AssistantPanel() = default;

AssistantPanel::~AssistantPanel() {
    Shutdown();
}

void AssistantPanel::Initialize() {
    m_Interface.Initialize();
}

void AssistantPanel::Shutdown() {
    m_Interface.Shutdown();
}

void AssistantPanel::Render() {
    if (!m_AlwaysVisible && !m_Interface.IsVisible()) return;
    
    if (m_IsExpanded) {
        RenderExpanded();
    } else {
        RenderMinimized();
    }
}

void AssistantPanel::RenderExpanded() {
    // Animated transition
    if (m_ExpandAnim < 1.0f) {
        m_ExpandAnim = std::min(1.0f, m_ExpandAnim + ImGui::GetIO().DeltaTime * 5.0f);
    }
    
    ImGui::SetNextWindowPos(m_Position);
    ImGui::SetNextWindowSize(ImVec2(m_Size.x, m_Size.y * m_ExpandAnim));
    
    ImGui::Begin("AI Assistant", nullptr,
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse);
    
    // Header with minimize button
    ImGui::Text("🤖 AI Assistant");
    ImGui::SameLine(m_Size.x - 60);
    if (ImGui::Button("−##Minimize")) {
        m_IsExpanded = false;
    }
    
    // Body
    RenderBody();
    
    // Footer
    RenderFooter();
    
    ImGui::End();
}

void AssistantPanel::RenderMinimized() {
    // Floating button when minimized
    ImGui::SetNextWindowPos(ImVec2(10, ImGui::GetIO().DisplaySize.y - 50));
    ImGui::Begin("AI Assistant Minimized", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoBackground);
    
    // Pulse animation if there are new suggestions
    if (m_NewSuggestionPulse > 0.0f) {
        ImGui::PushStyleColor(ImGuiCol_Button, 
                              ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
    }
    
    if (ImGui::Button("🤖##Expand", ImVec2(40, 40))) {
        m_IsExpanded = true;
        m_ExpandAnim = 0.0f;
    }
    
    if (m_NewSuggestionPulse > 0.0f) {
        ImGui::PopStyleColor();
        m_NewSuggestionPulse -= ImGui::GetIO().DeltaTime;
    }
    
    ImGui::End();
}

void AssistantPanel::RenderHeader() {
    ImGui::Text("AI Assistant");
}

void AssistantPanel::RenderBody() {
    // Main content from AssistantInterface
    m_Interface.RenderMainPanel();
}

void AssistantPanel::RenderFooter() {
    ImGui::Separator();
    
    // Quick status
    auto state = m_Interface.GetState();
    ImGui::Text("Patterns: %d | Confidence: %.0f%%", 
                 state.patterns_learned, state.confidence * 100);
}

void AssistantPanel::SetPosition(ImVec2 position) {
    m_Position = position;
}

void AssistantPanel::SetSize(ImVec2 size) {
    m_Size = size;
}

void AssistantPanel::SetOnSuggestionCallback(
    std::function<void(const std::string&)> callback) {
    m_OnSuggestionCallback = callback;
    m_Interface.SetOnSuggestionAccepted(callback);
}

} // namespace SZM::AI::UI
