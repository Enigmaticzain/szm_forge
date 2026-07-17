#pragma once

#include "../Sketch/Sketch2D.hpp"
#include "../SolidBody.hpp"
#include <vector>

namespace SZM::Geometry::Operations {

    /**
     * @class LoftOp
     * @brief Generates a 3D solid body by lofting between two or more 2D profiles.
     */
    class LoftOp {
    public:
        /**
         * @brief Lofts multiple sketch profiles into a continuous solid body.
         * @param profiles A list of closed 2D profiles defined at different Z heights or planes.
         * @return A generated SolidBody bridging the profiles.
         */
        static SolidBody LoftProfiles(const std::vector<SketchProfile>& profiles);
    };

} // namespace SZM::Geometry::Operations
