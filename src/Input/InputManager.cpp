#include "InputManager.hpp"
#include <algorithm>

namespace SZM {

    std::array<bool, 512> InputManager::s_KeyStates = {};
    std::array<bool, 8> InputManager::s_MouseButtonStates = {};
    float InputManager::s_MouseX = 0.0f;
    float InputManager::s_MouseY = 0.0f;

    void InputManager::Init() {
        ClearStateOnFocusLost();
    }

    bool InputManager::IsKeyPressed(int keycode) {
        if (keycode >= 0 && keycode < static_cast<int>(s_KeyStates.size()))
            return s_KeyStates[keycode];
        return false;
    }

    bool InputManager::IsMouseButtonPressed(int button) {
        if (button >= 0 && button < static_cast<int>(s_MouseButtonStates.size()))
            return s_MouseButtonStates[button];
        return false;
    }

    std::pair<float, float> InputManager::GetMousePosition() {
        return {s_MouseX, s_MouseY};
    }

    void InputManager::UpdateKeyState(int keycode, bool pressed) {
        if (keycode >= 0 && keycode < static_cast<int>(s_KeyStates.size())) {
            s_KeyStates[keycode] = pressed;
        }
    }

    void InputManager::UpdateMouseState(int button, bool pressed) {
        if (button >= 0 && button < static_cast<int>(s_MouseButtonStates.size())) {
            s_MouseButtonStates[button] = pressed;
        }
    }

    void InputManager::UpdateMousePosition(float x, float y) {
        s_MouseX = x;
        s_MouseY = y;
    }

    // Resolves Edge Case: Alt-Tabbing away causes keys to get "stuck"
    void InputManager::ClearStateOnFocusLost() {
        std::fill(s_KeyStates.begin(), s_KeyStates.end(), false);
        std::fill(s_MouseButtonStates.begin(), s_MouseButtonStates.end(), false);
    }

} // namespace SZM
