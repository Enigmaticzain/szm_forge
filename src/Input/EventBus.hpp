#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <any>
#include <mutex>

namespace SZM {

/**
 * @class EventBus
 * @brief Thread-safe singleton Observer / Publisher.
 *
 * Decouples the engine layer from the UI layer: any system can
 * Publish("EventName", payload) and any other system can Subscribe
 * to that name without a hard dependency between the two.
 *
 * Usage:
 *   EventBus::GetInstance().Subscribe("KeyPressed", [](const std::any& p) { ... });
 *   EventBus::GetInstance().Publish("KeyPressed", myEvent);
 */
class EventBus {
public:
    using EventCallback = std::function<void(const std::any&)>;

    static EventBus& GetInstance() {
        static EventBus instance;
        return instance;
    }

    EventBus(const EventBus&)            = delete;
    EventBus& operator=(const EventBus&) = delete;

    void Subscribe(const std::string& eventName, EventCallback callback) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Listeners[eventName].push_back(std::move(callback));
    }

    void Publish(const std::string& eventName, const std::any& payload) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Listeners.find(eventName);
        if (it != m_Listeners.end()) {
            for (auto& cb : it->second) {
                cb(payload);
            }
        }
    }

    /// Remove all listeners for a given event (useful during scene resets)
    void Unsubscribe(const std::string& eventName) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Listeners.erase(eventName);
    }

private:
    EventBus() = default;

    std::unordered_map<std::string, std::vector<EventCallback>> m_Listeners;
    std::mutex m_Mutex;
};

} // namespace SZM
