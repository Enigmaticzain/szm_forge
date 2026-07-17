#pragma once

#include "UUID.hpp"
#include "UUIDGenerator.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <algorithm>

namespace SZM {

    // Placeholder for Matrix4x4 Transform
    struct Transform {
        float matrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }; // Identity
    };

    // Base Entity Node
    class Entity : public std::enable_shared_from_this<Entity> {
    private:
        UUID uuid;
        std::weak_ptr<Entity> parent;
        std::vector<std::shared_ptr<Entity>> children;
        
        Transform localTransform;
        Transform worldTransform;
        std::string name;

    public:
        Entity(std::string nodeName = "Node") : name(std::move(nodeName)) {
            uuid = UUIDGenerator::Generate();
        }

        // Constructor for deserialization (retains original UUID)
        Entity(UUID existingID, std::string nodeName) : uuid(existingID), name(std::move(nodeName)) {}

        virtual ~Entity() = default;

        UUID GetUUID() const { return uuid; }
        std::string GetName() const { return name; }
        Transform GetLocalTransform() const { return localTransform; }
        void SetLocalTransform(const Transform& transform) { localTransform = transform; }
        Transform GetWorldTransform() const { return worldTransform; }
        void SetWorldTransform(const Transform& transform) { worldTransform = transform; }

        // Cycle Detection (DFS upward)
        bool IsAncestor(const UUID& potentialAncestorID) const {
            auto currentParent = parent.lock();
            while (currentParent) {
                if (currentParent->GetUUID() == potentialAncestorID) return true;
                currentParent = currentParent->parent.lock();
            }
            return false;
        }

        void AddChild(std::shared_ptr<Entity> child) {
            if (!child) return;
            
            // Anti-Circular Dependency Check
            if (child->GetUUID() == this->uuid || IsAncestor(child->GetUUID())) {
                throw std::runtime_error("SZM Fatal: Circular dependency detected. Node attachment aborted.");
            }

            child->parent = shared_from_this();
            children.push_back(child);
        }

        const std::vector<std::shared_ptr<Entity>>& GetChildren() const { return children; }
        std::weak_ptr<Entity> GetParent() const { return parent; }

        // Remove child by UUID
        bool RemoveChild(const UUID& childID) {
            auto it = std::find_if(children.begin(), children.end(),
                [childID](const std::shared_ptr<Entity>& child) {
                    return child->GetUUID() == childID;
                });
            
            if (it != children.end()) {
                (*it)->parent.reset();
                children.erase(it);
                return true;
            }
            return false;
        }

        // Find child by name
        std::shared_ptr<Entity> FindChildByName(const std::string& childName) const {
            for (const auto& child : children) {
                if (child->GetName() == childName) {
                    return child;
                }
            }
            return nullptr;
        }
    };

} // namespace SZM
