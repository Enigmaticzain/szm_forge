#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <mutex>
#include <cstdint>

namespace SZM::Geometry { class SolidBody; }

namespace SZM::Assembly {

    // ----------------------------------------------------------------
    //  AABB — axis-aligned bounding box
    // ----------------------------------------------------------------
    struct AABB {
        double MinX, MinY, MinZ;
        double MaxX, MaxY, MaxZ;

        [[nodiscard]] bool Intersects(const AABB& o, double tol) const {
            return (MinX <= o.MaxX + tol && MaxX >= o.MinX - tol) &&
                   (MinY <= o.MaxY + tol && MaxY >= o.MinY - tol) &&
                   (MinZ <= o.MaxZ + tol && MaxZ >= o.MinZ - tol);
        }
    };

    enum class MaterialType : uint8_t {
        Metal,
        Plastic,
        Rubber_Compressible,  ///< Allowed to overlap Metal (simulates gasket compression)
        Fluid_Volume          ///< Conforms to container — overlaps expected
    };

    struct PartInstance {
        uint32_t                                     InstanceID;
        AABB                                         BoundingBox;
        MaterialType                                 Material;
        std::shared_ptr<SZM::Geometry::SolidBody>   Geometry;
    };

    struct ClashVolume {
        uint32_t                                     PartA_ID;
        uint32_t                                     PartB_ID;
        double                                       InterferenceVolume;  ///< [m³]
        std::shared_ptr<SZM::Geometry::SolidBody>   IntersectionMesh;    ///< Highlight geometry
    };

    // ----------------------------------------------------------------
    //  InterferenceChecker
    // ----------------------------------------------------------------
    class InterferenceChecker {
    public:
        InterferenceChecker() = default;

        /**
         * @brief Run a static clash-detection pass over all part instances.
         *
         * Pipeline per pair:
         *   1. Material interaction rule (escape fast for rubber/fluid)
         *   2. AABB broad phase (cheap math)
         *   3. Boolean Intersection narrow phase (hooks Module 120.2)
         *
         * Outer loop is parallelised with std::jthread (C++20).
         *
         * @param assemblyNodes  Flat list of all part instances to check
         * @param toleranceEpsilon  Overlap tolerance [m] (default 0.01 mm)
         */
        [[nodiscard]] std::vector<ClashVolume> RunStaticCheck(
            const std::vector<PartInstance>& assemblyNodes,
            double toleranceEpsilon = 1e-5
        );

    private:
        [[nodiscard]] bool IsOverlapAllowed(MaterialType a, MaterialType b) const;

        [[nodiscard]] std::optional<ClashVolume> NarrowPhaseBoolean(
            const PartInstance& a,
            const PartInstance& b
        );
    };

} // namespace SZM::Assembly
