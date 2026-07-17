#pragma once
#include "SZM_SceneGraph.h"

namespace SZM {

class SceneGraphManager {
private:
    std::shared_ptr<Entity> rootNode;
    std::unordered_map<UUID, std::shared_ptr<Entity>> uuidRegistry;

    // Helper to register nodes into the lookup map
    void registerNode(std::shared_ptr<Entity> node) {
        uuidRegistry[node->getUUID()] = node;
        for (const auto& child : node->getChildren()) {
            registerNode(child);
        }
    }

public:
    SceneGraphManager() {
        rootNode = std::make_shared<Entity>("RootWorkspace");
        uuidRegistry[rootNode->getUUID()] = rootNode;
    }

    std::shared_ptr<Entity> getRoot() const { return rootNode; }

    // ---------------------------------------------------------
    // SERIALIZATION (Memory to .szm format)
    // ---------------------------------------------------------
    
    // Abstracted representation of the BSON/Protobuf payload
    struct SerializedNode {
        UUID id;
        UUID parentId; // The "swizzled" pointer
        std::string name;
        Transform localTransform;
    };

    std::vector<SerializedNode> serializeGraph() const {
        std::vector<SerializedNode> flattenedGraph;
        
        // DFS Traversal to flatten the DAG
        auto traverseAndFlatten = [&](auto& self, const std::shared_ptr<Entity>& node) -> void {
            if (!node) return;
            
            SerializedNode snode;
            snode.id = node->getUUID();
            snode.name = node->getName();
            snode.localTransform = node->getLocalTransform();
            
            if (auto p = node->getParent().lock()) {
                snode.parentId = p->getUUID(); // Convert Memory Pointer -> UUID
            } else {
                snode.parentId = {0, 0}; // Root has no parent
            }
            
            flattenedGraph.push_back(snode);

            for (const auto& child : node->getChildren()) {
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
    void deserializeGraph(const std::vector<SerializedNode>& diskData) {
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
                    parentNode->addChild(currentNode);
                } else {
                    // Handle "Ghost Nodes" or corrupted hierarchies
                    std::cerr << "SZM Warning: Parent UUID missing. Re-parenting " 
                              << sNode.name << " to Root." << std::endl;
                    if (rootNode && currentNode != rootNode) {
                        rootNode->addChild(currentNode);
                    }
                }
            } else {
                rootNode = currentNode; // This is the root
            }
        }
    }
};

} // namespace SZM

# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(SZM_Forge_DataCore VERSION 1.0.0 LANGUAGES CXX)

# Enforce C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform specific optimizations and strict warnings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /O2 /fp:fast")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -O3 -ffast-math -pthread")
endif()

# Core Data Library
add_library(DataCore_SceneGraph STATIC
    src/SZM_SceneGraph.h
    src/SZM_SceneGraphManager.h
    # Add implementation .cpp files here as they grow
)

# Includes (e.g., placeholder for nlohmann/json or protobuf later)
target_include_directories(DataCore_SceneGraph PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Test Executable
add_executable(SZM_TestRunner tests/main.cpp)
target_link_libraries(SZM_TestRunner PRIVATE DataCore_SceneGraph)

