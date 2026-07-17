#pragma once

#include "../Geometry/MathTypes.hpp"

namespace SZM::Graphics {

/**
 * @class Heatmap
 * @brief Color mapping for stress, temperature, and danger visualization
 *
 * Features:
 *  - Stress heatmap (blue → yellow → red)
 *  - Temperature heatmap (blue → purple → red)
 *  - Danger indicator (green ↔ red)
 *  - Smooth color transitions
 */
class Heatmap {
public:
    /**
     * @enum Mode
     * @brief Heatmap visualization mode
     */
    enum class Mode {
        Stress,           ///< Stress ratio visualization
        Temperature,      ///< Temperature visualization
        DangerIndicator,  ///< Danger state (safe/dangerous)
        None              ///< No heatmap
    };

    /**
     * @brief Create heatmap with specified mode
     * @param mode Visualization mode
     */
    explicit Heatmap(Mode mode = Mode::Stress);

    /**
     * @brief Get color for a given ratio
     * @param ratio Value from 0.0 to 1.0
     * @return RGB color vector
     */
    Geometry::Vector3 GetColor(float ratio) const;

    /**
     * @brief Set heatmap mode
     * @param mode New visualization mode
     */
    void SetMode(Mode mode) { m_Mode = mode; }

    /**
     * @brief Get current heatmap mode
     * @return Current mode
     */
    Mode GetMode() const { return m_Mode; }

private:
    Mode m_Mode;

    /**
     * @brief Get stress heatmap color
     * @param ratio Stress ratio (0-1)
     * @return RGB color (blue → yellow → red)
     */
    Geometry::Vector3 GetStressColor(float ratio) const;

    /**
     * @brief Get temperature heatmap color
     * @param ratio Temperature ratio (0-1)
     * @return RGB color (blue → purple → red)
     */
    Geometry::Vector3 GetTemperatureColor(float ratio) const;

    /**
     * @brief Get danger indicator color
     * @param ratio Danger ratio (0-1)
     * @return RGB color (green → red)
     */
    Geometry::Vector3 GetDangerColor(float ratio) const;

    /**
     * @brief Interpolate between two colors
     * @param c1 First color
     * @param c2 Second color
     * @param t Interpolation factor (0-1)
     * @return Interpolated color
     */
    static Geometry::Vector3 Lerp(const Geometry::Vector3& c1, const Geometry::Vector3& c2, float t);
};

} // namespace SZM::Graphics
