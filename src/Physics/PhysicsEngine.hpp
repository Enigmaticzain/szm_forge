#pragma once

#include "RigidBody.hpp"
#include <vector>
#include <memory>

namespace SZM::Physics {

/**
 * @class PhysicsEngine
 * @brief Central physics simulation manager using rigid-body dynamics.
 *
 * The PhysicsEngine is a Singleton that manages all rigid bodies in the simulation,
 * advances time via Tick(), and coordinates force and torque application with integration.
 *
 * **Responsibilities:**
 * - Body management: Add/remove bodies, track all active bodies
 * - Time stepping: Advance all bodies by deltaTime each frame
 * - Gravity: Apply universal gravity to all Dynamic bodies
 * - Integration coordination: Call Integrate() on each body each frame
 *
 * **Basic Usage:**
 * ```cpp
 * auto& engine = PhysicsEngine::GetInstance();
 * engine.Init();
 *
 * // Create body from mesh
 * auto body = RigidBody::CreateFromMesh(triangles, density);
 * engine.AddBody(body);
 *
 * // Game loop
 * while (running) {
 *     body->ApplyCentralForce(force);
 *     engine.Tick(deltaTime);
 * }
 * ```
 *
 * **Thread Safety:**
 * The PhysicsEngine itself is NOT thread-safe. All bodies must be added/removed
 * and Tick() must be called from a single thread (typically the physics thread).
 *
 * **Integration Method:**
 * Uses semi-implicit Euler for each body (see RigidBody::Integrate).
 * Global gravity is applied each frame via ApplyCentralForce.
 *
 * @see RigidBody for individual body dynamics
 * @see KinematicState for body classification
 * @invariant All stored bodies are non-null shared pointers
 * @invariant Gravity vector is constant during simulation (setter changes it)
 */
class PhysicsEngine {
public:
    /**
     * @brief Get the singleton instance of the PhysicsEngine.
     *
     * Returns a reference to the global PhysicsEngine. Multiple calls return
     * the same instance. Destroyed at program exit.
     *
     * @return Reference to the single PhysicsEngine instance
     *
     * @see Meyers' Singleton pattern (thread-safe in C++11)
     */
    static PhysicsEngine& GetInstance() {
        static PhysicsEngine instance;
        return instance;
    }

    /**
     * @brief Initialize the physics engine.
     *
     * Prepares internal state. Currently a no-op but reserved for future
     * initialization logic (e.g., memory pools, broad-phase structures).
     *
     * @see Shutdown()
     */
    void Init();
    
    /**
     * @brief Advance all bodies in the simulation by deltaTime.
     *
     * **Simulation Loop (per call):**
     * 1. Apply gravity to all Dynamic bodies: F_gravity = m · g
     * 2. Call Integrate(deltaTime) on each body
     * 3. Clear accumulated forces
     *
     * This is the main entry point for physics simulation. Call once per
     * game frame, typically after all forces have been applied.
     *
     * @param deltaTime Time step in seconds. Should be constant for stability.
     *                  Typical values: 0.016s (60 Hz), 0.033s (30 Hz)
     *
     * @precondition Bodies have had forces applied via ApplyCentralForce/ApplyTorque
     * @postcondition All body velocities and positions updated
     * @warning Large deltaTime values (> 0.05s) may cause stability issues
     *
     * @see RigidBody::Integrate()
     * @see RigidBody::ApplyCentralForce()
     */
    void Tick(double deltaTime);

    /**
     * @brief Register a rigid body to be managed by this engine.
     *
     * The body will be updated each Tick() and gravity will be applied.
     * Bodies can be added at any time. Ownership is shared with caller.
     *
     * @param body Non-null shared pointer to a RigidBody
     * @precondition body is not nullptr
     * @postcondition body is now integrated each Tick()
     *
     * @see RemoveBody()
     */
    void AddBody(std::shared_ptr<RigidBody> body);
    
    /**
     * @brief Unregister a rigid body from this engine.
     *
     * The body will no longer be updated each Tick(). The engine releases
     * its reference; the body is destroyed only if no other references exist.
     *
     * @param body Shared pointer to the body to remove
     * @postcondition body is no longer integrated; engine reference released
     *
     * @see AddBody()
     */
    void RemoveBody(std::shared_ptr<RigidBody> body);

    /**
     * @brief Set the universal gravitational acceleration.
     *
     * Applied to all Dynamic bodies each Tick() as:
     *     F_gravity = m · gravity_vector
     *
     * Default is Earth gravity: (0, -9.81, 0) m/s²
     * Set to (0, 0, 0) to disable gravity (e.g., space simulation).
     *
     * @param gravity 3D acceleration vector in m/s²
     *
     * @see GetGravity()
     */
    void SetGravity(const PhysVec3& gravity) { m_Gravity = gravity; }
    
    /**
     * @brief Get the current gravitational acceleration.
     *
     * @return Gravity vector in m/s²
     *
     * @see SetGravity()
     */
    PhysVec3 GetGravity() const              { return m_Gravity; }

private:
    /**
     * @brief Private constructor (Singleton pattern).
     *
     * Initializes gravity to Earth standard (0, -9.81, 0).
     * Bodies list is empty; call AddBody() to populate.
     */
    PhysicsEngine() = default;
    
    /**
     * @brief Private destructor (Singleton pattern).
     */
    ~PhysicsEngine() = default;

    // Delete copy and move semantics to enforce Singleton
    PhysicsEngine(const PhysicsEngine&) = delete;
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;

    std::vector<std::shared_ptr<RigidBody>> m_Bodies;  ///< All managed rigid bodies
    PhysVec3 m_Gravity = {0.0f, -9.81f, 0.0f};         ///< Gravity acceleration (m/s²)
};

} // namespace SZM::Physics