#pragma once

#include "MathTypes.hpp"
#include <vector>
#include <cmath>
#include <stdexcept>

namespace SZM::Geometry {

    struct Face {
        std::vector<Vector3> Vertices;
        Vector3 Normal;
    };

    struct SolidBody {
        std::vector<Vector3> Vertices;
        std::vector<std::vector<size_t>> Faces;
        double Volume = 0.0;
    };

    struct ExtrudeParams {
        double Distance;
        Vector3 Direction;
        double DraftAngleRad = 0.0;
    };

    namespace Operations {
        [[nodiscard]] SolidBody LinearExtrude(const Face& profile, const ExtrudeParams& params);
    }

} // namespace SZM::Geometry
