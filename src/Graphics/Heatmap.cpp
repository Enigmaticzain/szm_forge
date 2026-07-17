#include "Heatmap.hpp"
#include <algorithm>

namespace SZM::Graphics {

    Heatmap::Heatmap(Mode mode) : m_Mode(mode) {}

    Geometry::Vector3 Heatmap::Lerp(const Geometry::Vector3& c1, const Geometry::Vector3& c2, float t) {
        t = std::max(0.0f, std::min(1.0f, t));
        return Geometry::Vector3(
            c1.x + (c2.x - c1.x) * t,
            c1.y + (c2.y - c1.y) * t,
            c1.z + (c2.z - c1.z) * t
        );
    }

    Geometry::Vector3 Heatmap::GetStressColor(float ratio) const {
        // Clamp ratio to 0-1
        ratio = std::max(0.0f, std::min(1.0f, ratio));

        // Blue (0%) → Yellow (50%) → Red (100%)
        if (ratio < 0.5f) {
            // Blue to Yellow
            float t = ratio * 2.0f;  // 0-1
            Geometry::Vector3 blue(0.0f, 0.0f, 1.0f);
            Geometry::Vector3 yellow(1.0f, 1.0f, 0.0f);
            return Lerp(blue, yellow, t);
        } else {
            // Yellow to Red
            float t = (ratio - 0.5f) * 2.0f;  // 0-1
            Geometry::Vector3 yellow(1.0f, 1.0f, 0.0f);
            Geometry::Vector3 red(1.0f, 0.0f, 0.0f);
            return Lerp(yellow, red, t);
        }
    }

    Geometry::Vector3 Heatmap::GetTemperatureColor(float ratio) const {
        // Clamp ratio to 0-1
        ratio = std::max(0.0f, std::min(1.0f, ratio));

        // Blue (20°C) → Purple (60°C) → Red (100°C)
        if (ratio < 0.5f) {
            // Blue to Purple
            float t = ratio * 2.0f;  // 0-1
            Geometry::Vector3 blue(0.0f, 0.0f, 1.0f);
            Geometry::Vector3 purple(0.5f, 0.0f, 1.0f);
            return Lerp(blue, purple, t);
        } else {
            // Purple to Red
            float t = (ratio - 0.5f) * 2.0f;  // 0-1
            Geometry::Vector3 purple(0.5f, 0.0f, 1.0f);
            Geometry::Vector3 red(1.0f, 0.0f, 0.0f);
            return Lerp(purple, red, t);
        }
    }

    Geometry::Vector3 Heatmap::GetDangerColor(float ratio) const {
        // Clamp ratio to 0-1
        ratio = std::max(0.0f, std::min(1.0f, ratio));

        // Green (safe) → Red (dangerous)
        Geometry::Vector3 green(0.0f, 1.0f, 0.0f);
        Geometry::Vector3 red(1.0f, 0.0f, 0.0f);
        return Lerp(green, red, ratio);
    }

    Geometry::Vector3 Heatmap::GetColor(float ratio) const {
        switch (m_Mode) {
            case Mode::Stress:
                return GetStressColor(ratio);
            case Mode::Temperature:
                return GetTemperatureColor(ratio);
            case Mode::DangerIndicator:
                return GetDangerColor(ratio);
            case Mode::None:
            default:
                return Geometry::Vector3(1.0f, 1.0f, 1.0f);  // White
        }
    }

} // namespace SZM::Graphics
