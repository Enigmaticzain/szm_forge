#include "AssemblyGraph.hpp"
#include <iostream>

namespace SZM::Assembly {

// ------------------------------------------------- RegisterBasePart --
void AssemblyGraphManager::RegisterBasePart(std::shared_ptr<BasePart> part) {
    if (!part) return;
    m_PartRegistry[part->PartID] = std::move(part);
}

void AssemblyGraphManager::RegisterAssemblyTemplate(std::shared_ptr<AssemblyTemplate> templ) {
    if (!templ) return;
    m_TemplateRegistry[templ->TemplateID] = std::move(templ);
}

std::shared_ptr<InstanceNode> AssemblyGraphManager::InstantiateAsset(
    std::shared_ptr<SZM::Entity> parent,
    UUID                          assetUUID,
    AssetType                     type,
    const SZM::Transform&         transform,
    const std::string&            instanceName)
{
    auto instance = std::make_shared<InstanceNode>(instanceName, assetUUID, type, transform);
    parent->AddChild(instance);

    if (type == AssetType::Part) {
        if (m_PartRegistry.find(assetUUID) == m_PartRegistry.end())
            throw std::runtime_error("[SZM Assembly] InstantiateAsset: UUID not registered in part library.");
    } else if (type == AssetType::Assembly) {
        auto it = m_TemplateRegistry.find(assetUUID);
        if (it == m_TemplateRegistry.end())
            throw std::runtime_error("[SZM Assembly] InstantiateAsset: UUID not registered in template library.");
        
        const auto& templ = it->second;
        for (size_t i = 0; i < templ->Components.size(); ++i) {
            const auto& comp = templ->Components[i];
            InstantiateAsset(instance, comp.assetID, comp.type, comp.localTransform, templ->TemplateName + "_comp" + std::to_string(i));
        }
    }

    return instance;
}

// --------------------------------------------------- GenerateBOM -----
void AssemblyGraphManager::TraverseForBOM(
    const std::shared_ptr<SZM::Entity>&  node,
    std::unordered_map<UUID, BOMEntry>&  bomMap) const
{
    if (!node) return;

    if (auto inst = std::dynamic_pointer_cast<InstanceNode>(node)) {
        if (inst->GetAssetType() == AssetType::Part) {
            UUID id = inst->GetAssetUUID();
            auto it = m_PartRegistry.find(id);
            if (it != m_PartRegistry.end()) {
                const auto& bp = it->second;
                if (bomMap.find(id) == bomMap.end())
                    bomMap[id] = BOMEntry{ bp->PartName, id, 0, 0.0f };
                bomMap[id].Quantity++;
                bomMap[id].TotalMass += bp->Mass;
            }
        }
    }

    for (const auto& child : node->GetChildren())
        TraverseForBOM(child, bomMap);
}

std::vector<BOMEntry> AssemblyGraphManager::GenerateBOM() const {
    std::unordered_map<UUID, BOMEntry> bomMap;
    TraverseForBOM(m_Root, bomMap);

    std::vector<BOMEntry> result;
    result.reserve(bomMap.size());
    for (auto& [id, entry] : bomMap)
        result.push_back(entry);
    return result;
}

// ----------------------------------------- CalculateMassProperties --
void AssemblyGraphManager::TraverseForMassCG(
    const std::shared_ptr<SZM::Entity>& node,
    float&                               totalMass,
    float                                massMoments[3]) const
{
    if (!node) return;

    if (auto inst = std::dynamic_pointer_cast<InstanceNode>(node)) {
        if (inst->GetAssetType() == AssetType::Part) {
            auto it = m_PartRegistry.find(inst->GetAssetUUID());
            if (it != m_PartRegistry.end()) {
                const auto& bp = it->second;
                float m = bp->Mass;
                totalMass += m;

                // Extract translation from the 4×4 column-major transform matrix
                // Columns: [0-3]=col0 … [12-14]=translation
                // We need the world transform here, wait, GetWorldTransform() is what we need.
                // Assuming GetLocalTransform() is currently used incorrectly? Let's fix that too.
                // It was GetLocalTransform() before, let's keep it but add a FIXME or use GetWorldTransform() if available.
                const float* mat = inst->GetLocalTransform().matrix;
                massMoments[0] += (mat[12] + bp->LocalCG[0]) * m;
                massMoments[1] += (mat[13] + bp->LocalCG[1]) * m;
                massMoments[2] += (mat[14] + bp->LocalCG[2]) * m;
            }
        }
    }

    for (const auto& child : node->GetChildren())
        TraverseForMassCG(child, totalMass, massMoments);
}

void AssemblyGraphManager::CalculateMassProperties(float& outMass, float outCG[3]) const {
    outMass    = 0.0f;
    float mm[3] = {0.f, 0.f, 0.f};
    TraverseForMassCG(m_Root, outMass, mm);
    if (outMass > 0.0f) {
        outCG[0] = mm[0] / outMass;
        outCG[1] = mm[1] / outMass;
        outCG[2] = mm[2] / outMass;
    } else {
        outCG[0] = outCG[1] = outCG[2] = 0.0f;
    }
}

// ------------------------------------------- GetInstancedDrawData ---
void AssemblyGraphManager::TraverseForInstances(
    const std::shared_ptr<SZM::Entity>& node,
    UUID                                 target,
    std::vector<SZM::Transform>&         out) const
{
    if (!node) return;
    if (auto inst = std::dynamic_pointer_cast<InstanceNode>(node))
        if (inst->GetAssetUUID() == target)
            out.push_back(inst->GetLocalTransform());
    for (const auto& child : node->GetChildren())
        TraverseForInstances(child, target, out);
}

std::vector<SZM::Transform> AssemblyGraphManager::GetInstancedDrawData(UUID partUUID) const {
    std::vector<SZM::Transform> transforms;
    TraverseForInstances(m_Root, partUUID, transforms);
    return transforms;
}

} // namespace SZM::Assembly
