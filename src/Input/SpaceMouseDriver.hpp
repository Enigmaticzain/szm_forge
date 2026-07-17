#pragma once

#include <cstdint>
#include <array>

namespace SZM {

    struct SpaceMouseState {
        float tx, ty, tz;  // Translation
        float rx, ry, rz;  // Rotation
        uint32_t buttons;  // Bitmask for button states
    };

    class SpaceMouseDriver {
    public:
        static void Init();
        static void Shutdown();
        
        // Poll current state
        static SpaceMouseState GetState();
        
        // Check if device is connected
        static bool IsConnected();
        
        // Sensitivity scaling (0.0 - 1.0)
        static void SetSensitivity(float scale);
        static float GetSensitivity();

    private:
        static bool s_IsConnected;
        static float s_Sensitivity;
        static SpaceMouseState s_CurrentState;
    };

} // namespace SZM
