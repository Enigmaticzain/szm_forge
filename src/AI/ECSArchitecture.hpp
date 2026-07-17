#pragma once

#include <entt/entt.hpp>
#include <cstdint>
#include <string>

namespace SZM::ECS {

    /**
     * @brief Entity Component System registry wrapper
     * Provides a flexible, data-driven architecture using EnTT
     * 
     * Components are POD (Plain Old Data) structures that hold entity data
     * Systems process entities with specific component combinations
     */
    
    // ===== Core Components =====
    
    struct Transform {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        float rx = 0.0f, ry = 0.0f, rz = 0.0f;  // rotation
        float sx = 1.0f, sy = 1.0f, sz = 1.0f;  // scale
    };

    struct Geometry {
        uint32_t meshId = 0;
        uint32_t vertexCount = 0;
        uint32_t triangleCount = 0;
        bool isDirty = false;
    };

    struct Material {
        std::string name;
        uint32_t materialId = 0;
        float roughness = 0.5f;
        float metallic = 0.0f;
        uint32_t colorRGB = 0xFFFFFF;
    };

    struct PhysicsBody {
        float mass = 1.0f;
        bool isStatic = false;
        float restitution = 0.3f;
        float friction = 0.5f;
        bool isDirty = false;
    };

    struct Constraint {
        uint32_t bodyA = 0;
        uint32_t bodyB = 0;
        uint8_t type = 0;  // fixed, hinge, slider, etc
        bool isActive = true;
    };

    struct FEAData {
        uint32_t elementCount = 0;
        uint32_t nodeCount = 0;
        bool hasStress = false;
        bool hasDisplacement = false;
        bool hasTemperature = false;
        bool isDirty = false;
    };

    struct Assembly {
        uint32_t parentId = 0;
        std::string name;
        uint32_t depth = 0;
    };

    struct Tags {
        bool isVisible = true;
        bool isSelectable = true;
        bool isSerialized = true;
        std::string category;  // "component", "subassembly", "virtual"
    };

    // ===== System Interface =====

    /**
     * @brief Base class for ECS systems
     */
    class System {
    public:
        virtual ~System() = default;
        virtual void Update(entt::registry& registry, float deltaTime) = 0;
        virtual const char* Name() const = 0;
    };

    /**
     * @brief ECS Manager - Centralizes registry and systems
     */
    class ECManager {
    public:
        ECManager() = default;
        ~ECManager() = default;

        /**
         * @brief Create a new entity
         */
        entt::entity CreateEntity() { return registry.create(); }

        /**
         * @brief Destroy entity and all components
         */
        void DestroyEntity(entt::entity entity) { registry.destroy(entity); }

        /**
         * @brief Add component to entity
         */
        template<typename Component, typename... Args>
        Component& AddComponent(entt::entity entity, Args&&... args) {
            return registry.emplace<Component>(entity, std::forward<Args>(args)...);
        }

        /**
         * @brief Get component from entity
         */
        template<typename Component>
        Component* GetComponent(entt::entity entity) {
            return registry.try_get<Component>(entity);
        }

        /**
         * @brief Check if entity has component
         */
        template<typename Component>
        bool HasComponent(entt::entity entity) const {
            return registry.has<Component>(entity);
        }

        /**
         * @brief Remove component from entity
         */
        template<typename Component>
        void RemoveComponent(entt::entity entity) {
            registry.remove<Component>(entity);
        }

        /**
         * @brief Register and update system
         */
        void RegisterSystem(System* system) {
            m_Systems.push_back(system);
        }

        /**
         * @brief Update all systems
         */
        void Update(float deltaTime) {
            for (auto* system : m_Systems) {
                system->Update(registry, deltaTime);
            }
        }

        /**
         * @brief Query entities with specific components
         */
        template<typename... Components>
        auto Query() {
            return registry.view<Components...>();
        }

        /**
         * @brief Get raw registry for advanced queries
         */
        entt::registry& GetRegistry() { return registry; }

    private:
        entt::registry registry;
        std::vector<System*> m_Systems;
    };

}
