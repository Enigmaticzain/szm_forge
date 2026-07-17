#pragma once

#include "../Geometry/SolidBody.hpp"
#include <memory>

namespace SZM {

    /**
     * @class ToleranceAnalysis
     * @brief Modifies nominal B-Rep solid bodies to simulate physical manufacturing deviations and tolerances.
     */
    class ToleranceAnalysis {
    public:
        ToleranceAnalysis() = default;
        ~ToleranceAnalysis() = default;

        /**
         * @brief Morphs a nominal solid body's vertices by applying random noise within a tolerance band.
         * @param body The B-Rep solid body to morph
         * @param toleranceBand The maximum deviation magnitude (+/- meters)
         * @return true if successful
         */
        bool ApplyManufacturingDeviation(Geometry::SolidBody& body, double toleranceBand);

        /**
         * @brief Validates if a morphed body is still watertight and physically sound
         */
        bool ValidateDeviatedBody(const Geometry::SolidBody& body) const;
    };

} // namespace SZM
