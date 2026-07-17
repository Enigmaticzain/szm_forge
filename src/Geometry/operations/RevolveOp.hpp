#pragma once

#include "../MathTypes.hpp"
#include "../SolidBody.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  Profile used for revolution (shares Vector3 from MathTypes)
    // ----------------------------------------------------------------
    struct RevolveProfile {
        std::vector<Vector3> Vertices; ///< Ordered profile vertices
        Vector3              Normal;   ///< Plane normal (used for axis validation)
    };

    // ----------------------------------------------------------------
    //  RevolveOp
    // ----------------------------------------------------------------
    class RevolveOp {
    public:
        /**
         * @brief Revolves a 2-D profile around an arbitrary axis.
         *
         * @param profile           The 2-D cross-section to sweep
         * @param axisOrigin        A point on the revolution axis
         * @param axisVector        The axis direction (need not be unit)
         * @param sweepAngleDegrees Sweep angle [0, 360]
         * @param resolutionSteps   Angular tessellation steps
         * @return Newly generated SolidBody
         *
         * @throws std::invalid_argument  if the profile is empty
         * @throws std::runtime_error     if the profile crosses the axis
         *                                (would produce self-intersecting geometry)
         */
        [[nodiscard]] static SolidBody Execute(
            const RevolveProfile& profile,
            const Vector3&        axisOrigin,
            const Vector3&        axisVector,
            float                 sweepAngleDegrees,
            int                   resolutionSteps = 36
        );

    private:
        /// Rodrigues' rotation formula — rotates `point` around `axis` by `angleRad`
        static Vector3 RotateRodrigues(const Vector3& point,
                                        const Vector3& origin,
                                        const Vector3& axis,
                                        float          angleRad);

        /// Ensures no profile vertex crosses the axis of revolution
        static void ValidateProfile(const RevolveProfile& profile,
                                    const Vector3&         axisOrigin,
                                    const Vector3&         axisVector);
    };

} // namespace SZM::Geometry
