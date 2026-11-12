#include "AssemblyGraph.hpp"
#include <iostream>

namespace SZM::Assembly {

namespace {

SZM::Transform MultiplyTransform(const SZM::Transform& a, const SZM::Transform& b) {
    SZM::Transform out{};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.matrix[k * 4 + row] * b.matrix[col * 4 + k];
            }
            out.matrix[col * 4 + row] = sum;
        }
    }
    return out;
}

} // namespace

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
    std::unordered_set<UUID> activeTemplates;
    return InstantiateAssetImpl(std::move(parent), assetUUID, type, transform, instanceName, activeTemplates);
}

std::shared_ptr<InstanceNode> AssemblyGraphManager::InstantiateAssetImpl(
    std::shared_ptr<SZM::Entity> parent,
    UUID                          assetUUID,
    AssetType                     type,
    const SZM::Transform&         transform,
    const std::string&            instanceName,
    std::unordered_set<UUID>&     activeTemplates)
{
    if (!parent) {
        throw std::invalid_argument("[SZM Assembly] InstantiateAsset: parent node must not be null.");
    }

    auto instance = std::make_shared<InstanceNode>(instanceName, assetUUID, type, transform);
    parent->AddChild(instance);

    if (type == AssetType::Part) {
        if (m_PartRegistry.find(assetUUID) == m_PartRegistry.end())
            throw std::runtime_error("[SZM Assembly] InstantiateAsset: UUID not registered in part library.");
    } else if (type == AssetType::Assembly) {
        if (!activeTemplates.insert(assetUUID).second) {
            throw std::runtime_error("[SZM Assembly] InstantiateAsset: cyclic assembly template detected.");
        }

        auto it = m_TemplateRegistry.find(assetUUID);
        if (it == m_TemplateRegistry.end()) {
            activeTemplates.erase(assetUUID);
            throw std::runtime_error("[SZM Assembly] InstantiateAsset: UUID not registered in template library.");
        }

        const auto& templ = it->second;
        try {
            for (size_t i = 0; i < templ->Components.size(); ++i) {
                const auto& comp = templ->Components[i];
                InstantiateAssetImpl(
                    instance,
                    comp.assetID,
                    comp.type,
                    comp.localTransform,
                    templ->TemplateName + "_comp" + std::to_string(i),
                    activeTemplates);
            }
        } catch (...) {
            activeTemplates.erase(assetUUID);
            throw;
        }
        activeTemplates.erase(assetUUID);
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
    const SZM::Transform&               parentWorld,
    float&                              totalMass,
    float                               massMoments[3]) const
{
    if (!node) return;

    const SZM::Transform nodeWorld = MultiplyTransform(parentWorld, node->GetLocalTransform());

    if (auto inst = std::dynamic_pointer_cast<InstanceNode>(node)) {
        if (inst->GetAssetType() == AssetType::Part) {
            auto it = m_PartRegistry.find(inst->GetAssetUUID());
            if (it != m_PartRegistry.end()) {
                const auto& bp = it->second;
                float m = bp->Mass;
                totalMass += m;

                const float* mat = nodeWorld.matrix;
                const float wx = mat[0] * bp->LocalCG[0] + mat[4] * bp->LocalCG[1] + mat[8]  * bp->LocalCG[2] + mat[12];
                const float wy = mat[1] * bp->LocalCG[0] + mat[5] * bp->LocalCG[1] + mat[9]  * bp->LocalCG[2] + mat[13];
                const float wz = mat[2] * bp->LocalCG[0] + mat[6] * bp->LocalCG[1] + mat[10] * bp->LocalCG[2] + mat[14];
                massMoments[0] += wx * m;
                massMoments[1] += wy * m;
                massMoments[2] += wz * m;
            }
        }
    }

    for (const auto& child : node->GetChildren())
        TraverseForMassCG(child, nodeWorld, totalMass, massMoments);
}

void AssemblyGraphManager::CalculateMassProperties(float& outMass, float outCG[3]) const {
    outMass    = 0.0f;
    float mm[3] = {0.f, 0.f, 0.f};
    const SZM::Transform identity{};
    TraverseForMassCG(m_Root, identity, outMass, mm);
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
    const SZM::Transform&               parentWorld,
    UUID                                target,
    std::vector<SZM::Transform>&        out) const
{
    if (!node) return;
    const SZM::Transform nodeWorld = MultiplyTransform(parentWorld, node->GetLocalTransform());
    if (auto inst = std::dynamic_pointer_cast<InstanceNode>(node))
        if (inst->GetAssetUUID() == target)
            out.push_back(nodeWorld);
    for (const auto& child : node->GetChildren())
        TraverseForInstances(child, nodeWorld, target, out);
}

std::vector<SZM::Transform> AssemblyGraphManager::GetInstancedDrawData(UUID partUUID) const {
    std::vector<SZM::Transform> transforms;
    const SZM::Transform identity{};
    TraverseForInstances(m_Root, identity, partUUID, transforms);
    return transforms;
}

} // namespace SZM::Assembly
