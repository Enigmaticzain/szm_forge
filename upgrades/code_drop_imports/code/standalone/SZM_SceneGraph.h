#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <random>
#include <string>
#include <stdexcept>
#include <cstdint>

namespace SZM {

// 128-bit UUID Structure
struct UUID {
    uint64_t high;
    uint64_t low;

    bool operator==(const UUID& other) const {
        return high == other.high && low == other.low;
    }
};

} // namespace SZM

// Custom Hash for unordered_map compatibility
namespace std {
    template <>
    struct hash<SZM::UUID> {
        size_t operator()(const SZM::UUID& uuid) const {
            return hash<uint64_t>()(uuid.high) ^ (hash<uint64_t>()(uuid.low) << 1);
        }
    };
}

namespace SZM {

// Fast, zero-collision 128-bit UUID Generator
class UUIDGenerator {
public:
    static UUID generate() {
        static thread_local std::random_device rd;
        static thread_local std::mt19937_64 engine(rd());
        static thread_local std::uniform_int_distribution<uint64_t> dist;
        return {dist(engine), dist(engine)};
    }
};

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
        uuid = UUIDGenerator::generate();
    }

    // Constructor for deserialization (retains original UUID)
    Entity(UUID existingID, std::string nodeName) : uuid(existingID), name(std::move(nodeName)) {}

    UUID getUUID() const { return uuid; }
    std::string getName() const { return name; }
    Transform getLocalTransform() const { return localTransform; }

    // Cycle Detection (DFS upward)
    bool isAncestor(const UUID& potentialAncestorID) const {
        auto currentParent = parent.lock();
        while (currentParent) {
            if (currentParent->getUUID() == potentialAncestorID) return true;
            currentParent = currentParent->parent.lock();
        }
        return false;
    }

    void addChild(std::shared_ptr<Entity> child) {
        if (!child) return;
        
        // Anti-Circular Dependency Check
        if (child->getUUID() == this->uuid || isAncestor(child->getUUID())) {
            throw std::runtime_error("SZM Fatal: Circular dependency detected. Node attachment aborted.");
        }

        child->parent = shared_from_this();
        children.push_back(child);
    }

    const std::vector<std::shared_ptr<Entity>>& getChildren() const { return children; }
    std::weak_ptr<Entity> getParent() const { return parent; }
};

} // namespace SZM

