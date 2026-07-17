#pragma once

#include <array>
#include <utility>

namespace SZM {

    class InputManager {
    public:
        static void Init();
        
        // Polling APIs for systems that prefer direct state checking over events
        static bool IsKeyPressed(int keycode);
        static bool IsMouseButtonPressed(int button);
        static std::pair<float, float> GetMousePosition();

        // State updaters (called by OS-level window callbacks)
        static void UpdateKeyState(int keycode, bool pressed);
        static void UpdateMouseState(int button, bool pressed);
        static void UpdateMousePosition(float x, float y);

        // Edge Case Handling: Called when App loses OS focus
        static void ClearStateOnFocusLost();

    private:
        // Internal state trackers (size depends on underlying API, e.g., GLFW_KEY_LAST is 348)
        static std::array<bool, 512> s_KeyStates;
        static std::array<bool, 8> s_MouseButtonStates;
        static float s_MouseX, s_MouseY;
    };

} // namespace SZM
