#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace SZM::Physics {

    /**
     * @brief Bullet Physics integration
     * High-performance rigid body dynamics with:
     * - Multi-body simulation
     * - Soft body physics
     * - Constraint solving (fixed, hinge, slider)
     * - Collision detection (GJK+EPA)
     * - GPU acceleration (optional)
     */
    class BulletPhysicsBackend {
    public:
        struct SimulationConfig {
            float gravity = -9.81f;
            float timeStep = 1.0f / 60.0f;
            uint32_t maxSubSteps = 1;
            float linearDamping = 0.04f;
            float angularDamping = 0.1f;
            bool enableGPU = false;
            uint32_t GPUDeviceId = 0;
        };

        BulletPhysicsBackend(const SimulationConfig& config = {});
        ~BulletPhysicsBackend();

        /**
         * @brief Initialize physics world
         */
        bool Initialize();

        /**
         * @brief Step simulation
         */
        bool StepSimulation(float deltaTime);

        /**
         * @brief Create rigid body
         */
        uint32_t CreateRigidBody(uint32_t shapeId, float mass, 
                                float posX, float posY, float posZ);

        /**
         * @brief Apply force to body
         */
        void ApplyForce(uint32_t bodyId, 
                       float forceX, float forceY, float forceZ,
                       float pointX, float pointY, float pointZ);

        /**
         * @brief Apply impulse
         */
        void ApplyImpulse(uint32_t bodyId,
                         float impulseX, float impulseY, float impulseZ,
                         float pointX, float pointY, float pointZ);

        /**
         * @brief Get body transform
         */
        bool GetTransform(uint32_t bodyId,
                         float& posX, float& posY, float& posZ,
                         float& rotX, float& rotY, float& rotZ, float& rotW);

        /**
         * @brief Set body velocity
         */
        void SetLinearVelocity(uint32_t bodyId, float vx, float vy, float vz);

        /**
         * @brief Create constraint
         */
        uint32_t CreateConstraint(uint32_t bodyA, uint32_t bodyB,
                                 const std::string& constraintType);

        /**
         * @brief Raycast
         */
        bool Raycast(float startX, float startY, float startZ,
                    float endX, float endY, float endZ,
                    uint32_t& hitBodyId, float& hitDistance);

        /**
         * @brief Get collision data
         */
        struct CollisionInfo {
            uint32_t bodyA, bodyB;
            float normalX, normalY, normalZ;
            float penetration;
            float posX, posY, posZ;
        };

        std::vector<CollisionInfo> GetCollisions();

        /**
         * @brief Enable/disable GPU acceleration
         */
        bool SetGPUAcceleration(bool enabled, uint32_t deviceId = 0);

    private:
        SimulationConfig m_Config;
        void* m_World = nullptr;  // Bullet world handle
    };

}
