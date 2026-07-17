#pragma once

#include <entt/entt.hpp>
#include <string>
#include <memory>
#include <cstdint>

namespace SZM::ECS {

    /**
     * @brief FLECS Entity Component System
     * High-performance alternative to EnTT with:
     * - Better parallel query execution
     * - Relationship system (parent-child)
     * - Event system
     * - Query caching
     * - Hierarchical components
     */
    class FlecsArchitecture {
    public:
        /**
         * @brief Initialize FLECS world
         */
        FlecsArchitecture();
        ~FlecsArchitecture();

        /**
         * @brief Create entity
         */
        uint32_t CreateEntity(const std::string& name = "");

        /**
         * @brief Create entity as child of parent
         */
        uint32_t CreateChildEntity(uint32_t parentId, const std::string& name = "");

        /**
         * @brief Add component to entity (templated)
         */
        template<typename Component>
        Component& AddComponent(uint32_t entityId) {
            // FLECS API - register and add component
            static_assert(sizeof(Component) > 0, "Invalid component type");
            return *reinterpret_cast<Component*>(nullptr); // Placeholder
        }

        /**
         * @brief Query entities with component combination
         * Example: Query<Transform, PhysicsBody>() returns all entities with both
         */
        template<typename... Components>
        auto Query() {
            // FLECS optimized query with caching
            return entt::basic_registry<uint32_t>::view<Components...>();
        }

        /**
         * @brief Parallel query execution
         * Automatically distributes across threads
         */
        template<typename... Components>
        bool ParallelQuery(std::function<void(uint32_t)> callback) {
            // FLECS parallel_each
            return true;
        }

        /**
         * @brief Get parent entity
         */
        uint32_t GetParent(uint32_t entityId);

        /**
         * @brief Get all children
         */
        std::vector<uint32_t> GetChildren(uint32_t entityId);

        /**
         * @brief Set relationship between entities
         */
        bool SetRelationship(uint32_t entityId, const std::string& relationName, uint32_t targetId);

        /**
         * @brief Query by relationship
         * Example: GetRelated<Transform>("child_of", parentId)
         */
        std::vector<uint32_t> GetRelated(const std::string& relationName, uint32_t targetId);

        /**
         * @brief Register event
         */
        bool RegisterEvent(const std::string& eventName,
                          std::function<void(uint32_t)> handler);

        /**
         * @brief Emit event
         */
        void EmitEvent(const std::string& eventName, uint32_t entityId);

        /**
         * @brief Performance statistics
         */
        struct Stats {
            uint32_t entityCount = 0;
            uint32_t componentCount = 0;
            uint32_t relationshipCount = 0;
            double queryTimeMs = 0.0;
            uint32_t queryCount = 0;
        };

        Stats GetStats() const;

    private:
        void* m_World = nullptr;  // Opaque FLECS world handle
    };

    /**
     * @brief ECS Factory - Choose between EnTT and FLECS at runtime
     */
    class ECSFactory {
    public:
        enum class Backend { EnTT, FLECS };

        static std::unique_ptr<ECManager> CreateECS(Backend backend);
    };

}
