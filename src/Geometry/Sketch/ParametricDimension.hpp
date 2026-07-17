#pragma once

#include <string>

namespace SZM::Geometry {

    // ----------------------------------------------------------------
    //  ParametricDimension — A named dimension that drives geometry
    // ----------------------------------------------------------------
    struct ParametricDimension {
        uint32_t    ID;
        std::string Name;
        double      Value;       ///< Current solved value
        std::string Expression;  ///< Optional formula (e.g. "Width / 2")
        bool        IsDriven;    ///< True if this is a reference dimension
    };

} // namespace SZM::Geometry
