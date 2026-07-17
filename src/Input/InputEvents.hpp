#pragma once

#include "Event.hpp"
#include <sstream>

namespace SZM {

    // --- KEYBOARD EVENTS ---
    class KeyPressedEvent : public Event {
    public:
        KeyPressedEvent(int keycode, int repeatCount) 
            : m_KeyCode(keycode), m_RepeatCount(repeatCount) {}

        int GetKeyCode() const { return m_KeyCode; }
        int GetRepeatCount() const { return m_RepeatCount; }
        
        static EventType GetStaticType() { return EventType::KeyPressed; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyPressed"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryKeyboard; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << m_KeyCode << " (repeat: " << m_RepeatCount << ")";
            return ss.str();
        }

    private:
        int m_KeyCode;
        int m_RepeatCount;
    };

    class KeyReleasedEvent : public Event {
    public:
        KeyReleasedEvent(int keycode) : m_KeyCode(keycode) {}

        int GetKeyCode() const { return m_KeyCode; }

        static EventType GetStaticType() { return EventType::KeyReleased; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyReleased"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryKeyboard; }

    private:
        int m_KeyCode;
    };

    class KeyTypedEvent : public Event {
    public:
        KeyTypedEvent(int keycode) : m_KeyCode(keycode) {}

        int GetKeyCode() const { return m_KeyCode; }

        static EventType GetStaticType() { return EventType::KeyTyped; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "KeyTyped"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryKeyboard; }

    private:
        int m_KeyCode;
    };

    // --- MOUSE EVENTS ---
    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

        float GetX() const { return m_MouseX; }
        float GetY() const { return m_MouseY; }

        static EventType GetStaticType() { return EventType::MouseMoved; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseMoved"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
            return ss.str();
        }

    private:
        float m_MouseX, m_MouseY;
    };

    class MouseButtonPressedEvent : public Event {
    public:
        MouseButtonPressedEvent(int button) : m_Button(button) {}

        int GetMouseButton() const { return m_Button; }

        static EventType GetStaticType() { return EventType::MouseButtonPressed; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseButtonPressed"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

    private:
        int m_Button;
    };

    class MouseButtonReleasedEvent : public Event {
    public:
        MouseButtonReleasedEvent(int button) : m_Button(button) {}

        int GetMouseButton() const { return m_Button; }

        static EventType GetStaticType() { return EventType::MouseButtonReleased; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseButtonReleased"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

    private:
        int m_Button;
    };

    class MouseScrolledEvent : public Event {
    public:
        MouseScrolledEvent(float xOffset, float yOffset) 
            : m_XOffset(xOffset), m_YOffset(yOffset) {}

        float GetXOffset() const { return m_XOffset; }
        float GetYOffset() const { return m_YOffset; }

        static EventType GetStaticType() { return EventType::MouseScrolled; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "MouseScrolled"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategoryMouse; }

    private:
        float m_XOffset, m_YOffset;
    };

    // --- SPACEMOUSE EVENTS (6-DOF) ---
    class SpaceMouseMovedEvent : public Event {
    public:
        // Tx, Ty, Tz (Translation) | Rx, Ry, Rz (Rotation)
        SpaceMouseMovedEvent(float tx, float ty, float tz, float rx, float ry, float rz)
            : m_Tx(tx), m_Ty(ty), m_Tz(tz), m_Rx(rx), m_Ry(ry), m_Rz(rz) {}

        float GetTx() const { return m_Tx; }
        float GetTy() const { return m_Ty; }
        float GetTz() const { return m_Tz; }
        float GetRx() const { return m_Rx; }
        float GetRy() const { return m_Ry; }
        float GetRz() const { return m_Rz; }

        static EventType GetStaticType() { return EventType::SpaceMouseMoved; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "SpaceMouseMoved"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategorySpaceMouse; }

    private:
        float m_Tx, m_Ty, m_Tz, m_Rx, m_Ry, m_Rz;
    };

    class SpaceMouseButtonEvent : public Event {
    public:
        SpaceMouseButtonEvent(int button, bool pressed)
            : m_Button(button), m_Pressed(pressed) {}

        int GetButton() const { return m_Button; }
        bool IsPressed() const { return m_Pressed; }

        static EventType GetStaticType() { return EventType::SpaceMouseButton; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "SpaceMouseButton"; }
        int GetCategoryFlags() const override { return EventCategoryInput | EventCategorySpaceMouse; }

    private:
        int m_Button;
        bool m_Pressed;
    };

    class WindowLostFocusEvent : public Event {
    public:
        static EventType GetStaticType() { return EventType::WindowLostFocus; }
        EventType GetEventType() const override { return GetStaticType(); }
        const char* GetName() const override { return "WindowLostFocus"; }
        int GetCategoryFlags() const override { return EventCategoryInput; }
    };

} // namespace SZM
