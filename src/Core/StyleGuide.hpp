#pragma once

/**
 * @file StyleGuide.hpp
 * @brief SZM Forge C++ Code Style and API Design Guide
 *
 * This file documents the coding standards, naming conventions, and API design
 * principles for the SZM Forge project. All contributors should follow these
 * guidelines to maintain consistency and code quality.
 *
 * **Quick Reference:**
 * - Classes: PascalCase (Application, PhysicsEngine)
 * - Functions: PascalCase (GetComponent, SetPosition)
 * - Constants: PascalCase or SCREAMING_SNAKE_CASE (defaulted to PascalCase)
 * - Member variables: m_PascalCase (m_Position, m_Mass)
 * - Local variables: camelCase (tempValue, componentId)
 * - Namespaces: lowercase with :: separator (SZM::Physics::)
 * - Enums: PascalCase values (LogLevel::Debug, KinematicState::Dynamic)
 *
 * @see https://google.github.io/styleguide/cppguide.html (reference)
 */

namespace SZM {

// ============================================================
//  1. NAMING CONVENTIONS
// ============================================================

/**
 * @section naming Naming Conventions
 *
 * ### Classes & Structs
 * Use PascalCase (UpperCamelCase) for class and struct names.
 * @code
 * class PhysicsEngine { };      // ✓ Good
 * class physicsEngine { };       // ✗ Bad
 * class PHYSICS_ENGINE { };      // ✗ Bad
 * @endcode
 *
 * ### Methods & Functions
 * Use PascalCase with verb-noun pattern for methods.
 * Getters: Get<Property> or simplified forms
 * Setters: Set<Property>
 * @code
 * void ApplyCentralForce(const PhysVec3& force);  // ✓ Good
 * void apply_central_force(...);                   // ✗ Bad
 * void UpdateWorldInertia(const PhysMat3& R);    // ✓ Good
 * @endcode
 *
 * ### Member Variables (Instance)
 * Prefix with m_ followed by PascalCase.
 * This distinguishes instance members from local variables at a glance.
 * @code
 * class RigidBody {
 *     float m_Mass;              // ✓ Good
 *     std::vector<...> m_Bodies; // ✓ Good
 *     float mass;                // ✗ Bad (ambiguous)
 * };
 * @endcode
 *
 * ### Static Variables
 * Prefix with s_ followed by PascalCase.
 * @code
 * class Logger {
 *     static std::mutex s_Mutex;  // ✓ Good
 *     static LogLevel s_MinLevel; // ✓ Good
 * };
 * @endcode
 *
 * ### Local Variables
 * Use camelCase (lowerCamelCase) for local scope variables.
 * Keep names descriptive but concise.
 * @code
 * float timeStep = 0.016f;                      // ✓ Good
 * int componentId = GetNextId();                // ✓ Good
 * float t = 0.016f;                            // ✗ Ambiguous, too short
 * @endcode
 *
 * ### Constants (Compile-time)
 * Use PascalCase with static constexpr:
 * @code
 * namespace Config::Physics {
 *     constexpr float StandardGravity = 9.81f;      // ✓ Good
 *     constexpr float STANDARD_GRAVITY = 9.81f;    // Acceptable but less common
 * }
 * @endcode
 *
 * ### Enumerations
 * Class name: PascalCase
 * Enum values: PascalCase (no underscore prefix)
 * @code
 * enum class LogLevel {
 *     Debug,      // ✓ Good
 *     Info,       // ✓ Good
 *     DEBUG,      // ✗ Bad (inconsistent)
 * };
 *
 * enum class KinematicState {
 *     Dynamic,    // ✓ Good
 *     Kinematic,  // ✓ Good
 *     Static      // ✓ Good
 * };
 * @endcode
 *
 * ### Namespaces
 * Use lowercase with :: hierarchical organization.
 * Match directory structure when possible.
 * @code
 * namespace SZM::Physics { }         // ✓ Good
 * namespace SZM::Graphics { }        // ✓ Good
 * namespace SZM::Materials { }       // ✓ Good
 * @endcode
 *
 * ### Typedefs & Type Aliases
 * Use PascalCase. If based on STL, document the underlying type.
 * @code
 * using EntityID = uint32_t;         // ✓ Good
 * using ComponentMap = std::unordered_map<uint32_t, ComponentPtr>; // ✓ Good
 * @endcode
 */

// ============================================================
//  2. FUNCTION & METHOD DESIGN
// ============================================================

/**
 * @section api_design API Design Principles
 *
 * ### Method Naming Patterns
 * Follow established patterns for common operations:
 * @code
 * // Accessors / Getters
 * Type GetProperty() const;          // Return by value
 * Type& GetProperty();               // Return mutable reference (use with caution)
 * const Type& GetProperty() const;   // Return const reference (preferred)
 * Type* FindEntity(const ID& id);    // May return nullptr
 * bool IsValid() const;              // Boolean query
 * bool HasComponent(const ID& id) const; // Check existence
 *
 * // Mutators / Setters
 * void SetProperty(const Type& value);
 * void UpdateProperty(const Type& value); // Implies calculation/validation
 * void AddComponent(const Ptr& comp);
 * void RemoveComponent(const ID& id);
 * void ClearAll();
 *
 * // Actions / Verbs
 * void Tick(float deltaTime);        // Frame advance
 * void Init();                        // Initialization
 * void Shutdown();                   // Cleanup
 * void Rebuild();                    // Reconstruct internal state
 * @endcode
 *
 * ### Parameter Passing
 * Follow these rules for function parameters:
 * @code
 * // Small types (< 8 bytes): pass by value
 * void SetPosition(float x, float y, float z);
 * void SetScale(float scale);
 *
 * // Larger types: pass by const reference
 * void SetPosition(const PhysVec3& pos);
 * void ApplyForce(const PhysVec3& force);
 * void LoadMesh(const std::vector<Triangle>& mesh);
 *
 * // Out parameters: pass by non-const pointer or reference
 * bool GetComponent(uint32_t id, SimulationComponent& outComponent);
 * SimulationComponent* GetComponent(uint32_t id);  // May return nullptr
 *
 * // String parameters: pass const reference
 * bool SetMaterial(const std::string& materialId);
 * void LoadScene(const std::string& filePath);
 * @endcode
 *
 * ### Const Correctness
 * Apply const consistently to achieve strong compile-time guarantees:
 * @code
 * class Component {
 * public:
 *     // Const accessors for query operations
 *     const std::string& GetName() const;
 *     uint32_t GetID() const;
 *     bool IsActive() const;
 *
 *     // Non-const methods for mutation
 *     void SetActive(bool active);
 *     void SetName(const std::string& name);
 *
 *     // Const member methods (query-only)
 *     bool Contains(const ID& id) const;
 *     size_t GetCount() const;
 * };
 * @endcode
 *
 * ### Return Values
 * Choose appropriate return strategies:
 * @code
 * // Option 1: Return by value (small types, NRVO friendly)
 * PhysVec3 ComputeForce() const;
 *
 * // Option 2: Return const reference (avoid copy, but limited lifetime)
 * const std::vector<Entity>& GetAllEntities() const;
 *
 * // Option 3: Return pointer (nullable, indicates ownership transfer)
 * Entity* FindEntity(const ID& id);  // May return nullptr
 *
 * // Option 4: Return optional (C++17+, clearer nullable intent)
 * std::optional<Material> FindMaterial(const std::string& id) const;
 *
 * // Option 5: Return bool + out parameter (legacy, less preferred)
 * bool GetComponent(uint32_t id, Component& outComponent) const;
 * @endcode
 *
 * ### Single Responsibility
 * Each method should have one, clear purpose:
 * @code
 * // ✓ Good: Single purpose
 * void ApplyForce(const PhysVec3& force);
 * void Integrate(float deltaTime);
 *
 * // ✗ Bad: Multiple purposes
 * void UpdatePhysics(float dt, const PhysVec3& force, const PhysVec3& torque, ...);
 * @endcode
 */

// ============================================================
//  3. CLASS DESIGN
// ============================================================

/**
 * @section class_design Class Design Patterns
 *
 * ### Header Order
 * Follow this order for class member organization:
 * @code
 * class MyClass {
 * public:
 *     // 1. Public nested types & enums
 *     enum class State { };
 *     using Iterator = ...;
 *
 *     // 2. Factory methods & static accessors
 *     static MyClass& GetInstance();
 *     static std::shared_ptr<MyClass> Create(...);
 *
 *     // 3. Lifecycle (constructor, destructor)
 *     explicit MyClass(int param);
 *     ~MyClass();
 *
 *     // 4. Deleted copy/move (if applicable)
 *     MyClass(const MyClass&) = delete;
 *     MyClass& operator=(const MyClass&) = delete;
 *
 *     // 5. Public const methods (queries)
 *     uint32_t GetID() const;
 *     bool IsValid() const;
 *
 *     // 6. Public non-const methods (mutations)
 *     void SetPosition(const PhysVec3& pos);
 *     void Tick(float dt);
 *
 *     // 7. Operators (if any)
 *     bool operator==(const MyClass& other) const;
 *
 * private:
 *     // 8. Member variables (m_ prefix)
 *     uint32_t m_ID;
 *     std::string m_Name;
 *
 *     // 9. Private methods
 *     void UpdateInternal();
 * };
 * @endcode
 *
 * ### Singleton Pattern (Use with Caution)
 * When a singleton is required (e.g., Logger, Physics engine):
 * @code
 * class Logger {
 * public:
 *     static Logger& GetInstance() {
 *         static Logger instance;
 *         return instance;
 *     }
 *
 *     Logger(const Logger&) = delete;
 *     Logger& operator=(const Logger&) = delete;
 *
 * private:
 *     Logger() = default;
 *     ~Logger() = default;
 * };
 * @endcode
 *
 * ### Friend Usage
 * Minimize friend declarations. Document why they're needed:
 * @code
 * class PhysicsEngine {
 *     friend class Collider;  // Collider needs access to internal body list
 * private:
 *     std::vector<std::shared_ptr<RigidBody>> m_Bodies;
 * };
 * @endcode
 */

// ============================================================
//  4. CODE FORMATTING & STYLE
// ============================================================

/**
 * @section formatting Code Formatting
 *
 * ### Indentation
 * Use 4 spaces per indent level (no tabs).
 * @code
 * void Example() {
 *     if (condition) {
 *         DoSomething();
 *     }
 * }
 * @endcode
 *
 * ### Brace Style
 * Use Allman style (braces on separate lines):
 * @code
 * void Function()
 * {
 *     if (condition)
 *     {
 *         Statement();
 *     }
 *     else
 *     {
 *         AlternateStatement();
 *     }
 * }
 * @endcode
 *
 * ### Line Length
 * Maximum 120 characters per line (for readability on typical editors).
 * Break long lines logically:
 * @code
 * auto body = RigidBody::CreateFromMesh(
 *     triangles,
 *     density,
 *     KinematicState::Dynamic
 * );
 * @endcode
 *
 * ### Comments
 * Use /// for Doxygen documentation and // for inline comments:
 * @code
 * /// @brief Calculate net force from applied forces.
 * /// @param mass Object mass (kg)
 * /// @return Net force vector (N)
 * PhysVec3 CalculateNetForce(float mass) const;
 *
 * // Clamp velocity to physical limits
 * velocity = std::clamp(velocity, -MaxVelocity, MaxVelocity);
 * @endcode
 */

// ============================================================
//  5. COMMON PATTERNS
// ============================================================

/**
 * @section patterns Common Programming Patterns
 *
 * ### Error Handling
 * Prefer exceptions for exceptional conditions; use return codes for expected failures:
 * @code
 * // Exceptional: throw
 * if (meshSize == 0) {
 *     SZM_THROW(ErrorCode::InvalidArgument, "Mesh cannot be empty", "Physics");
 * }
 *
 * // Expected: return optional or bool
 * std::optional<Material> FindMaterial(const std::string& id) const;
 * bool SetComponentMaterial(uint32_t id, const std::string& materialId);
 * @endcode
 *
 * ### Resource Management (RAII)
 * Always use smart pointers and RAII for resource management:
 * @code
 * // Good: RAII - automatic cleanup
 * {
 *     auto body = std::make_shared<RigidBody>(state);
 *     engine.AddBody(body);
 * }  // body ref count decremented automatically
 *
 * // Bad: Manual new/delete
 * RigidBody* body = new RigidBody(state);
 * engine.AddBody(body);
 * delete body;  // Easy to forget, causes memory leaks
 * @endcode
 *
 * ### Thread Safety Documentation
 * Always document thread safety guarantees:
 * @code
 * /// Thread-safe singleton. All methods use mutex protection.
 * class Logger { };
 *
 * /// NOT thread-safe. Must only be called from physics thread.
 * class PhysicsEngine { };
 * @endcode
 */

} // namespace SZM

/**
 * @defgroup StyleGuide Code Style Guide
 * @brief Comprehensive C++ coding standards for SZM Forge.
 *
 * This section provides detailed coding conventions to maintain consistency,
 * readability, and professionalism across the codebase. All code contributions
 * should adhere to these guidelines.
 *
 * @see naming Naming Conventions
 * @see api_design API Design Principles
 * @see class_design Class Design Patterns
 * @see formatting Code Formatting
 * @see patterns Common Programming Patterns
 */
