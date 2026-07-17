#pragma once

#include "../MathTypes.hpp"
#include "../SolidBody.hpp"
#include "../Sketch/SketchProfile.hpp"
#include <vector>
#include <stdexcept>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  Profile is now SketchProfile in Sketch/SketchProfile.hpp
    // ----------------------------------------------------------------

    // ----------------------------------------------------------------
    //  ExtrudeParams
    // ----------------------------------------------------------------
    struct ExtrudeParams {
        double  Distance      = 0.0;    ///< Extrusion depth (metres)
        Vector3 Direction     = {};     ///< Defaults to Profile2D::Normal if zero
        double  DraftAngleRad = 0.0;    ///< Draft / taper angle ϕ (radians)
    };

    // ----------------------------------------------------------------
    //  Operations namespace
    // ----------------------------------------------------------------
    namespace Operations {

        /**
         * @brief Linearly extrudes a 2-D profile into a SolidBody.
         *
         * Supports:
         *   - Uniform extrusion along an arbitrary direction
         *   - Radial taper (draft angle) for moulding / casting geometry
         *
         * @throws std::invalid_argument  if the profile has no valid loops
         */
        [[nodiscard]] SolidBody LinearExtrude(const SketchProfile& profile,
                                               const ExtrudeParams& params);

    } // namespace Operations

} // namespace SZM::Geometry
