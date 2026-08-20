#pragma once

#include <cstdint>
#include <vector>
#include <unordered_set>

namespace SZM::Graphics {

enum class BRepSelectionMode { None, Face, Edge, Vertex };

struct BRepSelection {
    uint32_t componentId = UINT32_MAX;
    BRepSelectionMode mode = BRepSelectionMode::None;
    uint32_t elementIndex = UINT32_MAX; ///< face/edge/vertex index within the body
};

/**
 * @class SelectionManager
 * @brief Manages component and BRep sub-element selection
 */
class SelectionManager {
public:
    static SelectionManager& GetInstance();

    // Component-level selection
    void Select(uint32_t componentId);
    void Deselect(uint32_t componentId);
    void ToggleSelect(uint32_t componentId);
    void ClearSelection();
    bool IsSelected(uint32_t componentId) const;
    const std::unordered_set<uint32_t>& GetSelectedIds() const { return m_SelectedIds; }
    size_t GetSelectionCount() const { return m_SelectedIds.size(); }
    void SetMultiSelectMode(bool enabled) { m_MultiSelectMode = enabled; }
    bool IsMultiSelectMode() const { return m_MultiSelectMode; }

    // BRep sub-element selection
    void SelectFace(uint32_t componentId, uint32_t faceIndex);
    void SelectEdge(uint32_t componentId, uint32_t edgeIndex);
    void SelectVertex(uint32_t componentId, uint32_t vertexIndex);
    void ClearBRepSelection();

    const std::vector<BRepSelection>& GetBRepSelections() const { return m_BRepSelections; }
    BRepSelectionMode GetActiveMode() const { return m_ActiveMode; }
    void SetActiveMode(BRepSelectionMode mode) { m_ActiveMode = mode; }

private:
    SelectionManager() = default;
    ~SelectionManager() = default;
    SelectionManager(const SelectionManager&) = delete;
    SelectionManager& operator=(const SelectionManager&) = delete;

    std::unordered_set<uint32_t>  m_SelectedIds;
    std::vector<BRepSelection>    m_BRepSelections;
    BRepSelectionMode             m_ActiveMode = BRepSelectionMode::None;
    bool                          m_MultiSelectMode = false;
};

} // namespace SZM::Graphics
