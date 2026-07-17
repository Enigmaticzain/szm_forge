#pragma once

#include "FEADataTypes.hpp"
#include <string>

namespace SZM::FEA {

    /**
     * @class VolumetricMesher
     * @brief Wraps the Gmsh API to produce a tetrahedral FEA mesh from a
     *        B-Rep / STEP boundary representation file.
     *
     * Supported element types:
     *   - Tet4  (linear,    4-node,  faster but lower accuracy)
     *   - Tet10 (quadratic, 10-node, higher accuracy for curved surfaces)
     *
     * Edge-Case 9 — Micro-Edge Healing:
     *   Edges shorter than `microEdgeTolerance` are merged before meshing
     *   to prevent degenerate sliver elements.
     *
     * External dependency: libgmsh (find_package(gmsh) in CMake)
     *
     * @note Mesh quality is enforced via Netgen optimiser (Gmsh built-in).
     */
    class VolumetricMesher {
    public:
        /**
         * @brief Generate a tetrahedral mesh from a CAD boundary file.
         *
         * @param geometryFilePath   Path to .brep / .step / .iges file
         * @param globalElementSize  Target mesh size [m]
         * @param useQuadratic       false = Tet4, true = Tet10
         * @param microEdgeTolerance Minimum edge length before healing [m]
         * @return Populated FEAMesh ready for LinearStaticSolver
         */
        [[nodiscard]] static FEAMesh GenerateTetrahedralMesh(
            const std::string& geometryFilePath,
            double             globalElementSize,
            bool               useQuadratic       = false,
            double             microEdgeTolerance = 1e-6
        );
    };

} // namespace SZM::FEA
