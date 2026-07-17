#pragma once

#include "../Sketch/Sketch2D.hpp"
#include "../SolidBody.hpp"
#include <vector>

namespace SZM::Geometry::Operations {

    /**
     * @class SweepOp
     * @brief Generates a 3D solid body by sweeping a 2D profile along a 3D path.
     */
    class SweepOp {
    public:
        /**
         * @brief Sweeps a sketch profile along a polyline path.
         * @param profile The closed 2D profile to sweep.
         * @param path The 3D polyline defining the sweep trajectory.
         * @return A generated SolidBody representing the swept volume.
         */
        static SolidBody SweepProfileAlongPath(const SketchProfile& profile, const std::vector<Vector3>& path);
    };

} // namespace SZM::Geometry::Operations
