#pragma once

#include <cstdint>

namespace SZM::Geometry {

    enum class ConstraintType {
        Coincident,
        Horizontal,
        Vertical,
        Tangent,
        Parallel,
        Perpendicular,
        Concentric,
        EqualLength,
        EqualRadius,
        Distance,
        Angle,
        Fixed
    };

    // ----------------------------------------------------------------
    //  SketchConstraint — Relationship between sketch entities
    // ----------------------------------------------------------------
    struct SketchConstraint {
        uint32_t       ID;
        ConstraintType Type;

        // Up to two entities involved in the constraint
        uint32_t Entity1ID = 0;
        uint32_t Entity2ID = 0;

        // Sub-elements (e.g., specific endpoints: 0=Start, 1=End, 2=Center)
        int SubEntity1 = -1;
        int SubEntity2 = -1;

        // Optional dimension driving this constraint (for Distance/Angle)
        uint32_t DimensionID = 0;
    };

} // namespace SZM::Geometry
