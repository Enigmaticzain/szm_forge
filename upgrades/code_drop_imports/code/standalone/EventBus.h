#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <any>

// Minimal Observer Pattern for Engine-UI Decoupling
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

