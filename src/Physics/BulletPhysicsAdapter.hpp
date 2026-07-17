#pragma once

#include "RigidBody.hpp"

#include <memory>
#include <cstddef>
#include <vector>

namespace SZM::Physics {

/**
 * @class BulletPhysicsAdapter
 * @brief Optional adapter that mirrors SZM rigid bodies into a Bullet world.
 *
 * Built only when `SZM_ENABLE_BULLET_BACKEND=ON` and Bullet targets are available.
 */
class BulletPhysicsAdapter {
public:
    struct CollisionContact {
        int handleA = -1;
        int handleB = -1;
        PhysVec3 pointWorld = {0.0f, 0.0f, 0.0f};
        PhysVec3 normalWorld = {0.0f, 1.0f, 0.0f};
        float separation = 0.0f;
    };

    BulletPhysicsAdapter();
    ~BulletPhysicsAdapter();

    bool Initialize(const PhysVec3& gravity = {0.0f, -9.81f, 0.0f});
    void Shutdown();
    void SetGravity(const PhysVec3& gravity);

    int RegisterBody(
        std::shared_ptr<RigidBody> body,
        const PhysVec3& halfExtents = {0.5f, 0.5f, 0.5f}
    );
    bool UnregisterBody(int handle);
    bool ApplyCentralForce(int handle, const PhysVec3& force);
    void Step(float dtSeconds, int maxSubSteps = 1);
    void SyncFromBullet();
    std::vector<CollisionContact> CollectContacts() const;

    std::size_t GetBodyCount() const { return m_Entries.size(); }
    bool IsInitialized() const { return m_Initialized; }

private:
    static constexpr int kGroundHandle = 0;

    struct Entry {
        int handle = -1;
        std::shared_ptr<RigidBody> body;
        PhysVec3 halfExtents = {0.5f, 0.5f, 0.5f};
        void* collisionShape = nullptr;
        void* motionState = nullptr;
        void* bulletBody = nullptr;
    };

    Entry* FindEntry(int handle);
    const Entry* FindEntry(int handle) const;
    int ResolveHandle(const void* bulletBody) const;

    std::vector<Entry> m_Entries;
    bool m_Initialized = false;
    int m_NextHandle = 1;

    void* m_Config = nullptr;
    void* m_Dispatcher = nullptr;
    void* m_Broadphase = nullptr;
    void* m_Solver = nullptr;
    void* m_World = nullptr;
    void* m_GroundShape = nullptr;
    void* m_GroundMotionState = nullptr;
    void* m_GroundBody = nullptr;
};

} // namespace SZM::Physics
