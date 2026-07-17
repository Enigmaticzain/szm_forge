#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <iostream>

namespace SZM::Physics {

    /**
     * @defgroup PhysMath Physics Math Primitives
     * @brief Low-level vector and matrix types for physics simulation.
     *
     * This module provides fundamental math primitives optimized for rigid-body dynamics.
     * These types are designed to be SIMD-expandable for future performance optimization.
     * All operations maintain mathematical correctness for 3D physics simulation.
     * @{
     */

    /**
     * @struct PhysVec3
     * @brief 3D float vector for physics calculations.
     *
     * Supports standard vector operations: addition, subtraction, scaling, dot product,
     * and cross product. All operations are const-correct and branch-free where possible.
     *
     * @invariant All components are finite floats during normal operation.
     */
    struct PhysVec3 {
        float x; ///< X component
        float y; ///< Y component
        float z; ///< Z component

        /** @brief Vector addition */
        PhysVec3 operator+(const PhysVec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
        
        /** @brief Vector subtraction */
        PhysVec3 operator-(const PhysVec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
        
        /** @brief Scalar multiplication (right operand) */
        PhysVec3 operator*(float s)           const { return {x*s,   y*s,   z*s  }; }
        
        /** @brief Scalar division (right operand) */
        PhysVec3 operator/(float s)           const { return {x/s,   y/s,   z/s  }; }
        
        /** @brief Dot product (scalar triple product result)
         *  @param o The other vector
         *  @return Dot product: x*o.x + y*o.y + z*o.z
         */
        float dot  (const PhysVec3& o) const { return x*o.x + y*o.y + z*o.z; }
        
        /** @brief Cross product
         *  @param o The other vector
         *  @return Cross product vector perpendicular to both operands
         */
        PhysVec3 cross(const PhysVec3& o) const {
            return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x};
        }
    };

    /**
     * @struct PhysMat3
     * @brief 3x3 float matrix for rigid-body transformations.
     *
     * Represents rotation matrices and inertia tensors. Stored in row-major order.
     * Supports matrix-matrix multiplication, matrix-vector transformation, transpose,
     * and inverse operations. Used for storing inertia tensors and rotation matrices.
     *
     * @invariant Rotation matrices are orthogonal; inverse equals transpose.
     * @invariant Inertia tensors are symmetric (I_ij = I_ji).
     */
    struct PhysMat3 {
        float m[3][3] = {{1,0,0},{0,1,0},{0,0,1}}; ///< Row-major 3x3 matrix (identity by default)
        
        /** @brief Matrix-matrix multiplication
         *  @param o Right operand matrix
         *  @return Product matrix (this × o)
         */
        PhysMat3 operator*(const PhysMat3& o) const;
        
        /** @brief Matrix-vector multiplication
         *  @param v Right operand vector
         *  @return Transformed vector (this × v)
         */
        PhysVec3 operator*(const PhysVec3& v) const;
        
        /** @brief Matrix transpose
         *  @return Transposed matrix (M^T)
         */
        PhysMat3 Transpose() const;
        
        /** @brief Matrix inverse using Cramer's rule
         *  @return Inverse matrix M^-1; returns zero matrix if determinant < 1e-6
         *  @warning Returns zero matrix on singular matrices; callers should verify
         */
        PhysMat3 Inverse()   const;
    };

    /**
     * @enum KinematicState
     * @brief Classification of rigid body motion characteristics.
     *
     * Determines which physics equations are applied during integration.
     * Static and kinematic bodies are not integrated; only Dynamic bodies
     * respond to applied forces and torques.
     */
    enum class KinematicState {
        Dynamic,    ///< Full rigid-body dynamics: forces and torques affect motion
        Kinematic,  ///< Motor-driven or prescribed motion: infinite effective mass
        Static      ///< Immobile anchor: infinite mass and inertia
    };

    /**
     * @struct Triangle
     * @brief Triangle mesh primitive for rigid body initialization.
     *
     * Represents a single triangle extracted from a Boundary Representation (B-Rep)
     * solid mesh. Used with RigidBody::CreateFromMesh() to compute physical properties
     * (mass, center of mass, inertia tensor) via signed-tetrahedron integration.
     *
     * @see RigidBody::CreateFromMesh()
     */
    struct Triangle {
        PhysVec3 v0; ///< First vertex
        PhysVec3 v1; ///< Second vertex
        PhysVec3 v2; ///< Third vertex
    };

    /** @} */ // end PhysMath group

    /**
     * @class RigidBody
     * @brief Represents a 3D rigid body in physics simulation.
     *
     * Models a single rigid body with full 6-DOF (6 degrees of freedom) dynamics:
     * - 3 translational (position, linear velocity)
     * - 3 rotational (orientation, angular velocity)
     *
     * RigidBody integrates forces/torques to compute linear and angular accelerations,
     * then integrates velocities to update position and orientation. Supports three
     * kinematic states (Dynamic, Kinematic, Static) for flexible simulation control.
     *
     * **Construction Methods:**
     * - Default constructor: Creates body with state, zero mass
     * - CreateFromMesh(): Factory that computes physics properties from triangle mesh
     *
     * **Force Application:**
     * Apply forces before each Tick(). Accumulated forces are cleared after integration.
     * - ApplyCentralForce(): Add linear force
     * - ApplyTorque(): Add rotational torque
     *
     * **Integration Method:**
     * Semi-implicit Euler integration (velocity Verlet variant):
     * 1. Compute acceleration: a = F · m^-1
     * 2. Update velocity: v = v + a · dt
     * 3. Update position: x = x + v · dt
     * 4. Clear accumulated forces
     *
     * **Mathematical Details:**
     * - Inertia tensor stored in both local (body) and world (global) coordinates
     * - World inertia updated each frame: I_world^-1 = R · I_local^-1 · R^T
     * - Angular momentum: L = I · ω (integrated via torque)
     * - Linear momentum: p = m · v (integrated via force)
     *
     * **Thread Safety:** Not thread-safe. Tick() should only be called from physics thread.
     *
     * @see KinematicState for motion classification
     * @see Triangle for mesh input format
     * @see PhysicsEngine for integration into simulation loop
     * @invariant m_Mass >= 0; m_InvMass = 1/m_Mass for Dynamic/Kinematic bodies
     * @invariant m_State determines which physics equations apply
     * @invariant All velocities are updated each frame by Integrate()
     */
    class RigidBody {
    public:
        /**
         * @brief Construct a rigid body with the given kinematic state.
         *
         * Creates an uninitialized body (zero mass, zero position). Use CreateFromMesh()
         * for bodies with computed physics properties, or manually set properties after.
         *
         * @param state Classification: Dynamic (full dynamics), Kinematic (infinite mass),
         *              or Static (immobile). Default: Dynamic.
         * @see CreateFromMesh()
         */
        explicit RigidBody(KinematicState state = KinematicState::Dynamic);

        /**
         * @brief Factory method: Create a rigid body from a triangle mesh.
         *
         * Computes mass, center of mass, and inertia tensor using signed-tetrahedron
         * integration (Mirtich / Eberly polynomial method). This method is the standard
         * way to create physically accurate bodies from CAD geometry.
         *
         * **Algorithm:**
         * 1. Decompose mesh into tetrahedra with origin
         * 2. Integrate volume and inertia using polynomial formulas
         * 3. Compute CoM as volume-weighted centroid
         * 4. Invert inertia tensor for dynamics
         *
         * @param mesh Closed triangle mesh (CCW winding order assumed)
         * @param density Material density in kg/m³
         * @param state Kinematic classification (default: Dynamic)
         * @return Shared pointer to initialized RigidBody
         *
         * @throws std::exception if mesh is empty
         * @precondition mesh should form a closed manifold for accurate results
         * @postcondition Returned body ready for simulation (masses, inertia computed)
         *
         * @see Triangle structure for mesh format
         */
        [[nodiscard]] static std::shared_ptr<RigidBody> CreateFromMesh(
            const std::vector<Triangle>& mesh,
            float                        density,
            KinematicState               state = KinematicState::Dynamic
        );

        /**
         * @brief Update world-space inertia tensor before each Tick().
         *
         * Transforms the local (body-space) inertia tensor to world coordinates
         * using the current rotation matrix. Required for accurate angular dynamics
         * when the body is rotating.
         *
         * **Formula:** I_world^-1 = R · I_local^-1 · R^T
         *
         * @param rotationMatrix Current body-to-world rotation matrix (orthogonal)
         *
         * @note Called automatically by physics engine; exposed for custom integrators
         * @precondition rotationMatrix must be orthogonal (R^T · R = I)
         */
        void UpdateWorldInertia(const PhysMat3& rotationMatrix);

        /**
         * @brief Accumulate a linear force to be applied this frame.
         *
         * Forces accumulate and are applied during Integrate(). Multiple calls
         * add to the accumulator. Accumulated forces are cleared after integration.
         *
         * **Effect:** Modifies acceleration a = F · m^-1
         *
         * @param force Force vector in newtons (N)
         * @note Static/Kinematic bodies ignore this
         * @see ApplyTorque() for rotational force
         */
        inline void ApplyCentralForce(const PhysVec3& force) {
            m_ForceAccumulator = m_ForceAccumulator + force;
        }

        /**
         * @brief Accumulate a torque (rotational force) to be applied this frame.
         *
         * Torques accumulate and are applied during Integrate(). Multiple calls
         * add to the accumulator. Accumulated torques are cleared after integration.
         *
         * **Effect:** Modifies angular acceleration α = τ · I^-1
         *
         * @param torque Torque vector in newton-meters (N·m)
         * @note Static/Kinematic bodies ignore this
         * @see ApplyCentralForce() for linear force
         */
        inline void ApplyTorque(const PhysVec3& torque) {
            m_TorqueAccumulator = m_TorqueAccumulator + torque;
        }

        /**
         * @brief Clear accumulated forces and torques (called automatically by Integrate).
         *
         * Resets force and torque accumulators to zero. Normally called automatically
         * after integration; only call directly if resetting mid-frame.
         *
         * @see Integrate()
         */
        inline void ClearForces() {
            m_ForceAccumulator = {0.0f, 0.0f, 0.0f};
            m_TorqueAccumulator = {0.0f, 0.0f, 0.0f};
        }

        /**
         * @brief Integrate forces/torques and advance body state.
         *
         * **Semi-implicit Euler Integration:**
         * - Skip if not Dynamic
         * - Compute acceleration: a = F · m^-1, α = τ · I^-1
         * - Update velocity: v += a · dt, ω += α · dt
         * - Update position: x += v · dt
         * - Apply damping to angular velocity (Config::AngularVelocityDamping)
         * - Clear force/torque accumulators
         *
         * @param dt Time step in seconds (should be constant for stability, ~0.016s for 60 Hz)
         *
         * @note Static and Kinematic bodies return early; only Dynamic bodies integrate
         * @precondition UpdateWorldInertia() should be called before this with current rotation
         * @postcondition Force/torque accumulators are cleared; body state updated
         *
         * @warning dt should be small (< 0.05s) and constant for stability
         * @see UpdateWorldInertia()
         */
        inline void Integrate(float dt) {
            if (m_State != KinematicState::Dynamic) return;
            
            // Semi-implicit Euler integration
            PhysVec3 linearAccel = m_ForceAccumulator * m_InvMass;
            m_LinearVelocity = m_LinearVelocity + linearAccel * dt;
            m_Position = m_Position + m_LinearVelocity * dt;
            
            // Angular dynamics in body/world inertia space.
            const PhysVec3 angularAccel = m_InvInertiaTensorWorld * m_TorqueAccumulator;
            m_AngularVelocity = m_AngularVelocity + angularAccel * dt;
            // Note: Damping factor from Config::Physics::AngularVelocityDamping
            m_AngularVelocity = m_AngularVelocity * 0.999f;  // Configurable constant
            
            ClearForces();
        }

        // --- Accessor Methods ----------------------------------------

        /** @brief Get mass in kilograms (constant; set only via CreateFromMesh or direct field) */
        float          GetMass()          const { return m_Mass;               }
        
        /** @brief Get local center of mass (body-space coordinates) */
        PhysVec3       GetCenterOfMass()  const { return m_CenterOfMassLocal;  }
        
        /** @brief Get local inertia tensor (body-space coordinates) */
        PhysMat3       GetInertiaTensor() const { return m_InertiaTensorLocal; }
        
        /** @brief Get kinematic state (Dynamic/Kinematic/Static) */
        KinematicState GetState()         const { return m_State;              }
        
        /** @brief Get inverse mass (1/m for Dynamic, 0 for Static/Kinematic) */
        float          GetInvMass()       const { return m_InvMass;            }
        
        /** @brief Get current linear velocity (m/s) */
        PhysVec3       GetLinearVelocity()  const { return m_LinearVelocity;   }
        
        /** @brief Get current angular velocity (rad/s) */
        PhysVec3       GetAngularVelocity() const { return m_AngularVelocity;  }

        // --- Mutator Methods -----------------------------------------

        /** @brief Set world-space position of center of mass */
        void     SetPosition(const PhysVec3& pos) { m_Position = pos; }
        
        /** @brief Set linear velocity (m/s) */
        void     SetLinearVelocity(const PhysVec3& vel) { m_LinearVelocity = vel; }
        
        /** @brief Set angular velocity (rad/s) */
        void     SetAngularVelocity(const PhysVec3& vel) { m_AngularVelocity = vel; }
        
        /** @brief Get world-space position of center of mass */
        PhysVec3 GetPosition() const              { return m_Position; }

    private:
        // Kinematic classification
        KinematicState m_State;
        
        // Mass properties
        float     m_Mass    = 0.0f;  ///< Total mass in kg
        float     m_InvMass = 0.0f;  ///< Precomputed 1/m for efficiency; 0 if immobile
        
        // Inertia and center of mass
        PhysVec3  m_CenterOfMassLocal  = {};  ///< Center of mass in body coordinates
        PhysVec3  m_CenterOfMassWorld  = {};  ///< Center of mass in world coordinates
        PhysMat3  m_InertiaTensorLocal;       ///< Inertia tensor I_local (constant)
        PhysMat3  m_InvInertiaTensorLocal;    ///< Precomputed inverse I_local^-1
        PhysMat3  m_InvInertiaTensorWorld;    ///< I_world^-1 (updated each frame)
        
        // Dynamics state
        PhysVec3  m_LinearVelocity  = {};     ///< Linear velocity (m/s)
        PhysVec3  m_AngularVelocity = {};     ///< Angular velocity (rad/s)

        // Position and accumulated forces
        PhysVec3  m_Position          = {};   ///< Center of mass in world space (m)
        PhysVec3  m_ForceAccumulator  = {};   ///< Sum of applied forces (N)
        PhysVec3  m_TorqueAccumulator = {};   ///< Sum of applied torques (N·m)
    };

} // namespace SZM::Physics
