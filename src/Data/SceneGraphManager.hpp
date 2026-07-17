#pragma once

#include "SceneGraph.hpp"
#include <vector>

namespace SZM {

    class SceneGraphManager {
    private:
        std::shared_ptr<Entity> rootNode;
        std::unordered_map<UUID, std::shared_ptr<Entity>> uuidRegistry;

        // Helper to register nodes into the lookup map
        void RegisterNode(std::shared_ptr<Entity> node) {
            uuidRegistry[node->GetUUID()] = node;
            for (const auto& child : node->GetChildren()) {
                RegisterNode(child);
            }
        }

    public:
        SceneGraphManager() {
            rootNode = std::make_shared<Entity>("RootWorkspace");
            uuidRegistry[rootNode->GetUUID()] = rootNode;
        }

        std::shared_ptr<Entity> GetRoot() const { return rootNode; }

        // Backward-compatible API used by older tests/integration scripts.
        bool AddEntity(const std::shared_ptr<Entity>& entity) {
            return AddEntity(entity, rootNode ? rootNode->GetUUID() : UUID{0, 0});
        }

        bool AddEntity(const std::shared_ptr<Entity>& entity, const UUID& parentId) {
            if (!entity) return false;
            auto parent = FindByUUID(parentId);
            if (!parent) return false;
            parent->AddChild(entity);
            RegisterNode(entity);
            return true;
        }

        // Find entity by UUID
        std::shared_ptr<Entity> FindByUUID(const UUID& id) const {
            auto it = uuidRegistry.find(id);
            if (it != uuidRegistry.end()) {
                return it->second;
            }
            return nullptr;
        }

        // Find entity by name (DFS)
        std::shared_ptr<Entity> FindByName(const std::string& name) const {
            auto search = [&](auto& self, const std::shared_ptr<Entity>& node) -> std::shared_ptr<Entity> {
                if (!node) return nullptr;
                if (node->GetName() == name) return node;
                
                for (const auto& child : node->GetChildren()) {
                    auto result = self(self, child);
                    if (result) return result;
                }
                return nullptr;
            };
            
            return search(search, rootNode);
        }

        // ---------------------------------------------------------
        // SERIALIZATION (Memory to .szm format)
        // ---------------------------------------------------------
        
        struct SerializedNode {
            UUID id;
            UUID parentId; // The "swizzled" pointer
            std::string name;
            Transform localTransform;
        };

        std::vector<SerializedNode> SerializeGraph() const {
            std::vector<SerializedNode> flattenedGraph;
            
            // DFS Traversal to flatten the DAG
            auto traverseAndFlatten = [&](auto& self, const std::shared_ptr<Entity>& node) -> void {
                if (!node) return;
                
                SerializedNode snode;
                snode.id = node->GetUUID();
                snode.name = node->GetName();
                snode.localTransform = node->GetLocalTransform();
                
                if (auto p = node->GetParent().lock()) {
                    snode.parentId = p->GetUUID(); // Convert Memory Pointer -> UUID
                } else {
                    snode.parentId = {0, 0}; // Root has no parent
                }
                
                flattenedGraph.push_back(snode);

                for (const auto& child : node->GetChildren()) {
                    self(self, child);
                }
            };

            traverseAndFlatten(traverseAndFlatten, rootNode);
            return flattenedGraph; 
            // Note: flattenedGraph would then be encoded to BSON/Protobuf and written to disk
        }

        // ---------------------------------------------------------
        // DESERIALIZATION (Disk .szm to Memory)
        // ---------------------------------------------------------
        void DeserializeGraph(const std::vector<SerializedNode>& diskData) {
            uuidRegistry.clear();
            rootNode.reset();

            // Pass 1: Instantiate all nodes with their original UUIDs
            for (const auto& sNode : diskData) {
                auto entity = std::make_shared<Entity>(sNode.id, sNode.name);
                // In a full implementation, apply sNode.localTransform here
                uuidRegistry[sNode.id] = entity;
            }

            // Pass 2: Re-link pointers using the UUID Registry (Swizzling resolution)
            for (const auto& sNode : diskData) {
                auto currentNode = uuidRegistry[sNode.id];
                
                // Re-attach to parent
                if (sNode.parentId.high != 0 || sNode.parentId.low != 0) {
                    auto parentNode = uuidRegistry[sNode.parentId];
                    if (parentNode) {
                        parentNode->AddChild(currentNode);
                    } else {
                        // Handle "Ghost Nodes" or corrupted hierarchies
                        std::cerr << "SZM Warning: Parent UUID missing. Re-parenting " 
                                  << sNode.name << " to Root." << std::endl;
                        if (rootNode && currentNode != rootNode) {
                            rootNode->AddChild(currentNode);
                        }
                    }
                } else {
                    rootNode = currentNode; // This is the root
                }
            }
        }

        std::vector<SerializedNode> Serialize() const {
            return SerializeGraph();
        }

        void Deserialize(const std::vector<SerializedNode>& diskData) {
            DeserializeGraph(diskData);
        }

        // Get all entities (for iteration)
        const std::unordered_map<UUID, std::shared_ptr<Entity>>& GetAllEntities() const {
            return uuidRegistry;
        }

        // Get entity count
        size_t GetEntityCount() const {
            return uuidRegistry.size();
        }

        // Clear entire graph
        void Clear() {
            uuidRegistry.clear();
            rootNode = std::make_shared<Entity>("RootWorkspace");
            uuidRegistry[rootNode->GetUUID()] = rootNode;
        }
    };

} // namespace SZM
