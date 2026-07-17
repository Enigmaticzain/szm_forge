#include "SpaceMouseDriver.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace SZM {

    namespace {

    bool ParseStateString(const std::string& text, SpaceMouseState& outState) {
        std::stringstream ss(text);
        std::string token;
        std::vector<float> values;
        while (std::getline(ss, token, ',')) {
            try {
                values.push_back(std::stof(token));
            } catch (...) {
                return false;
            }
        }
        if (values.size() < 6) {
            return false;
        }

        outState.tx = values[0];
        outState.ty = values[1];
        outState.tz = values[2];
        outState.rx = values[3];
        outState.ry = values[4];
        outState.rz = values[5];
        outState.buttons = values.size() >= 7
            ? static_cast<uint32_t>(std::max(0.0f, values[6]))
            : 0u;
        return true;
    }

    float ClampUnit(float v) {
        return std::max(-1.0f, std::min(1.0f, v));
    }

    } // namespace

    bool SpaceMouseDriver::s_IsConnected = false;
    float SpaceMouseDriver::s_Sensitivity = 1.0f;
    SpaceMouseState SpaceMouseDriver::s_CurrentState = {};

    void SpaceMouseDriver::Init() {
        // Hardware HID integration is platform-specific. For reproducible testing
        // we provide a simulation path via environment variables:
        //   SZM_SPACEMOUSE_CONNECTED=1
        //   SZM_SPACEMOUSE_SIM=tx,ty,tz,rx,ry,rz[,buttons]
        s_CurrentState = {};
        s_IsConnected = false;

        if (const char* connected = std::getenv("SZM_SPACEMOUSE_CONNECTED")) {
            if (std::string(connected) == "1") {
                s_IsConnected = true;
            }
        }

        if (const char* state = std::getenv("SZM_SPACEMOUSE_SIM")) {
            SpaceMouseState parsed{};
            if (ParseStateString(state, parsed)) {
                s_CurrentState = parsed;
                s_IsConnected = true;
            }
        }

        std::cout << "SZM SpaceMouseDriver: Initialized ("
                  << (s_IsConnected ? "simulated/connected" : "disconnected")
                  << ").\n";
    }

    void SpaceMouseDriver::Shutdown() {
        s_IsConnected = false;
        s_CurrentState = {};
        std::cout << "SZM SpaceMouseDriver: Shutdown complete.\n";
    }

    SpaceMouseState SpaceMouseDriver::GetState() {
        SpaceMouseState scaled = s_CurrentState;
        scaled.tx = ClampUnit(scaled.tx * s_Sensitivity);
        scaled.ty = ClampUnit(scaled.ty * s_Sensitivity);
        scaled.tz = ClampUnit(scaled.tz * s_Sensitivity);
        scaled.rx = ClampUnit(scaled.rx * s_Sensitivity);
        scaled.ry = ClampUnit(scaled.ry * s_Sensitivity);
        scaled.rz = ClampUnit(scaled.rz * s_Sensitivity);
        return scaled;
    }

    bool SpaceMouseDriver::IsConnected() {
        return s_IsConnected;
    }

    void SpaceMouseDriver::SetSensitivity(float scale) {
        s_Sensitivity = (scale < 0.0f) ? 0.0f : (scale > 1.0f) ? 1.0f : scale;
    }

    float SpaceMouseDriver::GetSensitivity() {
        return s_Sensitivity;
    }

} // namespace SZM
