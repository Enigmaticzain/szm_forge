class EventBus {
public:
    using EventCallback = std::function<void(const std::any&)>;

    static EventBus& GetInstance() {
        static EventBus instance;
        return instance;
    }

    void Subscribe(const std::string& eventName, EventCallback callback) {
        listeners[eventName].push_back(callback);
    }

    void Publish(const std::string& eventName, const std::any& payload) {
        if (listeners.find(eventName) != listeners.end()) {
            for (auto& callback : listeners[eventName]) {
                callback(payload);
            }
        }
    }

private:
    std::unordered_map<std::string, std::vector<EventCallback>> listeners;
};

// IPanel.h

// ===== NEW ADDITIONS FROM CODE BUNDLES =====

class EventBus {
public:
    using EventCallback = std::function<void(const std::any&)>;

    static EventBus& GetInstance() {
        static EventBus instance;
        return instance;
    }

    void Subscribe(const std::string& eventName, EventCallback callback) {
        listeners[eventName].push_back(callback);
    }

    void Publish(const std::string& eventName, const std::any& payload) {
        if (listeners.find(eventName) != listeners.end()) {
            for (auto& callback : listeners[eventName]) {
                callback(payload);
            }
        }
    }
private:
    std::unordered_map<std::string, std::vector<EventCallback>> listeners;
};
""",

    "SZM_Forge_UI/src/IPanel.h": """// SZM_Forge_UI/src/IPanel.h