#pragma once

#include "../Physics/RigidBody.hpp"
#if defined(SZM_HAS_BULLET)
#include "../Physics/BulletPhysicsAdapter.hpp"
#endif

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace SZM {

class Phase5PhysicsIntegration {
public:
    void Initialize();
    void Shutdown();

    // Creates/removes rigid bodies as components appear/disappear,
    // then applies simulation forces to those bodies.
    void PrePhysicsStep(double deltaTime);

    // Pushes physics-derived state (position/velocity) back to simulation components.
    void PostPhysicsStep();

    [[nodiscard]] std::size_t GetTrackedBodyCount() const { return m_Bodies.size(); }
    [[nodiscard]] std::size_t GetLastCollisionCount() const { return m_LastCollisionCount; }

private:
    struct TrackedBody {
        uint32_t componentId = 0;
        std::shared_ptr<Physics::RigidBody> body;
        Physics::PhysVec3 halfExtents = {0.5f, 0.5f, 0.5f};
        int bulletHandle = -1;
    };

    std::shared_ptr<Physics::RigidBody> CreateBodyFromComponent(
        uint32_t componentId,
        Physics::PhysVec3* outHalfExtents = nullptr
    );
    void SyncBodySetWithSimulation();

    static float EstimateComponentVolumeM3(float areaM2);
    static Physics::PhysVec3 ComputeSpawnPosition(uint32_t componentId);

private:
    std::unordered_map<uint32_t, TrackedBody> m_Bodies;
    bool m_Initialized = false;
    bool m_UseBullet = false;
    double m_LastDeltaTime = 1.0 / 60.0;
    std::size_t m_LastCollisionCount = 0;
#if defined(SZM_HAS_BULLET)
    Physics::BulletPhysicsAdapter m_BulletAdapter;
#endif
};

} // namespace SZM
