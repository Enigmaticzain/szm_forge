#pragma once

#include "RigidBody.hpp"
#include <vector>
#include <array>
#include <memory>

namespace SZM::Physics {

    // ----------------------------------------------------------------
    //  Contact data
    // ----------------------------------------------------------------
    struct ContactPoint {
        PhysVec3 localPointA;
        PhysVec3 localPointB;
        PhysVec3 worldPoint;
        PhysVec3 normal;       ///< Points from B → A
        float    penetration = 0.0f;

        // Warm-starting accumulators (transferred from previous frame)
        float    normalImpulseAccum       = 0.0f;
        float    tangentImpulseAccum[2]   = {0.0f, 0.0f};
        uint32_t featureIdA = 0;
        uint32_t featureIdB = 0;

        bool Matches(const ContactPoint& old, float threshold = 0.01f) const {
            PhysVec3 d = { worldPoint.x - old.worldPoint.x,
                           worldPoint.y - old.worldPoint.y,
                           worldPoint.z - old.worldPoint.z };
            return (d.x*d.x + d.y*d.y + d.z*d.z) < threshold*threshold;
        }
    };

    struct ContactManifold {
        std::shared_ptr<RigidBody>  BodyA;
        std::shared_ptr<RigidBody>  BodyB;
        std::vector<ContactPoint>   Contacts;
        bool                        IsPersistent = false;
    };

    /// GJK working simplex (max 4 points = tetrahedron)
    struct Simplex {
        std::array<PhysVec3, 4> Points;
        int Size = 0;
        void PushFront(PhysVec3 p) {
            for (int i = std::min(Size, 3); i > 0; --i) {
                Points[static_cast<size_t>(i)] = Points[static_cast<size_t>(i - 1)];
            }
            Points[0] = p;
            Size = std::min(Size + 1, 4);
        }
    };

    // ----------------------------------------------------------------
    //  CollisionDetection
    // ----------------------------------------------------------------
    class CollisionDetection {
    public:
        /// Primary entry point — GJK broad then EPA narrow
        static bool GenerateManifold(
            std::shared_ptr<RigidBody> bodyA,
            std::shared_ptr<RigidBody> bodyB,
            ContactManifold&           outManifold
        );

        /// CCD sweep test to prevent tunnelling (Edge-Case 9)
        static bool SweepTestCCD(
            std::shared_ptr<RigidBody> bodyA, std::shared_ptr<RigidBody> bodyB,
            PhysVec3 velA, PhysVec3 velB, float dt,
            float& outTOI, PhysVec3& outHitNormal
        );

        /// Warm-starting: merge new manifold against cached previous frame
        static void MergeManifolds(
            std::vector<ContactManifold>& activeCache,
            ContactManifold&              newManifold
        );

    private:
        static PhysVec3 GetFurthestPoint(const RigidBody& body, PhysVec3 dir);
        static PhysVec3 Support(const RigidBody& a, const RigidBody& b, PhysVec3 dir);
        static bool     ExecuteGJK(const RigidBody& a, const RigidBody& b, Simplex& out);
        static bool     HandleSimplex(Simplex& s, PhysVec3& dir);
        static ContactPoint ExecuteEPA(const RigidBody& a, const RigidBody& b,
                                       const Simplex& gjkSimplex);
    };

} // namespace SZM::Physics
