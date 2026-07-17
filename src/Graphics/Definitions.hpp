#include "UIManager.h"
// #include <imgui.h> // Uncomment when ImGui is linked
// #include <imgui_internal.h>

UIManager::UIManager() {}
UIManager::~UIManager() {}

void UIManager::Initialize() {
    // ImGui Initialization logic goes here
}

void UIManager::RegisterPanel(std::shared_ptr<IPanel> panel) {
    m_Panels.push_back(panel);
}

void UIManager::RenderUI() {
    // Main docking logic and panel rendering loop goes here
}

void UIManager::BuildDefaultDockspace(unsigned int dockspace_id) {
    // Default docking splits logic goes here
}
""",

    "SZM_Forge_UI/src/panels/SceneOutliner.h": """// SZM_Forge_UI/src/panels/SceneOutliner.h

// ===== NEW ADDITIONS FROM CODE BUNDLES =====

#include "MeshGenerator.hpp"

// ===== NEW ADDITIONS FROM CODE BUNDLES =====

#include "../../Graphics/SceneGraph.h" // Inherits Entity and UUID from Module 020.1

// ===== NEW ADDITIONS FROM CODE BUNDLES =====

namespace SZM::Assembly {

void AssemblyGraphManager::registerBasePart(const std::shared_ptr<BasePart>& part) {
    if (!part) return;
    partRegistry[part->partID] = part;
}

std::shared_ptr<InstanceNode> AssemblyGraphManager::instantiatePart(
    std::shared_ptr<SZM::Entity> parentNode, 
    UUID partUUID, 
    const Transform& transform,
    const std::string& instanceName) 
{
    if (partRegistry.find(partUUID) == partRegistry.end()) {
        throw std::runtime_error("SZM Fatal: Attempted to instantiate an unregistered Part UUID.");
    }

    auto instance = std::make_shared<InstanceNode>(instanceName, partUUID, transform);
    
    // Cycle detection is inherently handled by Module 020.1's addChild() method.
    // If the user attempts to instance an assembly into itself, addChild will throw.
    parentNode->addChild(instance);

    return instance;
}

void AssemblyGraphManager::traverseForBOM(const std::shared_ptr<SZM::Entity>& node, std::unordered_map<UUID, BOMEntry>& bomMap) const {
    if (!node) return;

    // RTTI check to see if the Entity is an InstanceNode
    if (auto instanceNode = std::dynamic_pointer_cast<InstanceNode>(node)) {
        UUID partID = instanceNode->getBasePartUUID();
        
        if (partRegistry.count(partID)) {
            auto basePart = partRegistry.at(partID);
            
            if (bomMap.find(partID) == bomMap.end()) {
                bomMap[partID] = BOMEntry{basePart->partName, partID, 0, 0.0f};
            }
            
            bomMap[partID].quantity += 1;
            bomMap[partID].totalMass += basePart->mass;
        }
    }

    for (const auto& child : node->getChildren()) {
        traverseForBOM(child, bomMap);
    }
}

std::vector<BOMEntry> AssemblyGraphManager::generateBOM() const {
    std::unordered_map<UUID, BOMEntry> bomMap;
    traverseForBOM(activeAssemblyRoot, bomMap);

    std::vector<BOMEntry> bomList;
    bomList.reserve(bomMap.size());
    for (const auto& [id, entry] : bomMap) {
        bomList.push_back(entry);
    }
    return bomList;
}

void AssemblyGraphManager::traverseForMassCG(const std::shared_ptr<SZM::Entity>& node, float& outTotalMass, Vector3& outMassMoments) const {
    if (!node) return;

    if (auto instanceNode = std::dynamic_pointer_cast<InstanceNode>(node)) {
        UUID partID = instanceNode->getBasePartUUID();
        
        if (partRegistry.count(partID)) {
            auto basePart = partRegistry.at(partID);
            
            float m = basePart->mass;
            outTotalMass += m;

            // In a full implementation, we multiply the basePart->localCG by the instanceNode's 
            // getWorldTransform() to get the global CG position.
            // For this abstraction, we assume a simplified extraction of the position from the matrix.
            Transform worldTx = instanceNode->getLocalTransform(); // Placeholder for getWorldTransform()
            
            Vector3 globalPos = {
                worldTx.matrix[12] + basePart->localCG.x, 
                worldTx.matrix[13] + basePart->localCG.y, 
                worldTx.matrix[14] + basePart->localCG.z
            };

            // Summing (m_i * r_i)
            outMassMoments = outMassMoments + (globalPos * m);
        }
    }

    for (const auto& child : node->getChildren()) {
        traverseForMassCG(child, outTotalMass, outMassMoments);
    }
}

void AssemblyGraphManager::calculateMassProperties(float& outTotalMass, Vector3& outCG) const {
    outTotalMass = 0.0f;
    Vector3 massMoments = {0.0f, 0.0f, 0.0f};

    traverseForMassCG(activeAssemblyRoot, outTotalMass, massMoments);

    if (outTotalMass > 0.0f) {
        outCG = massMoments / outTotalMass;
    } else {
        outCG = {0.0f, 0.0f, 0.0f};
    }
}

std::vector<Transform> AssemblyGraphManager::getInstancedDrawData(UUID partUUID) const {
    std::vector<Transform> instanceTransforms;

    // DFS Traversal to collect all transforms for a specific base part.
    // This array is passed directly to the Graphics Hardware Context (Module 010.4)
    auto traverseAndCollect = [&](auto& self, const std::shared_ptr<SZM::Entity>& node) -> void {
        if (!node) return;

        if (auto instanceNode = std::dynamic_pointer_cast<InstanceNode>(node)) {
            if (instanceNode->getBasePartUUID() == partUUID) {
                // In production, this must be the computed World Transform
                instanceTransforms.push_back(instanceNode->getLocalTransform()); 
            }
        }

        for (const auto& child : node->getChildren()) {
            self(self, child);
        }
    };

    traverseAndCollect(traverseAndCollect, activeAssemblyRoot);
    return instanceTransforms;
}

} // namespace SZM::Assembly
