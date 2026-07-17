#pragma once

#include <functional>
#include <string>

namespace SZM {

    enum class EventType {
        None = 0,
        WindowLostFocus,
        KeyPressed, KeyReleased, KeyTyped,
        MouseMoved, MouseButtonPressed, MouseButtonReleased, MouseScrolled,
        SpaceMouseMoved, SpaceMouseButton
    };

    enum EventCategory {
        None = 0,
        EventCategoryInput      = 1 << 0,
        EventCategoryKeyboard   = 1 << 1,
        EventCategoryMouse      = 1 << 2,
        EventCategorySpaceMouse = 1 << 3
    };

    // Base Event Class
    class Event {
    public:
        virtual ~Event() = default;
        bool Handled = false; // The core routing mechanism (UI vs 3D Viewport)

        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); }

        bool IsInCategory(EventCategory category) {
            return GetCategoryFlags() & category;
        }
    };

    // Event Dispatcher for Routing
    class EventDispatcher {
    public:
        EventDispatcher(Event& event) : m_Event(event) {}

        // F will be deduced by the compiler
        template<typename T, typename F>
        bool Dispatch(const F& func) {
            if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
                m_Event.Handled = func(static_cast<T&>(m_Event));
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };

} // namespace SZM
