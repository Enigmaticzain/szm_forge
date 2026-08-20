#include "SelectionManager.hpp"

namespace SZM::Graphics {

SelectionManager& SelectionManager::GetInstance() {
    static SelectionManager instance;
    return instance;
}

void SelectionManager::Select(uint32_t componentId) {
    if (!m_MultiSelectMode) m_SelectedIds.clear();
    m_SelectedIds.insert(componentId);
}

void SelectionManager::Deselect(uint32_t componentId) {
    m_SelectedIds.erase(componentId);
}

void SelectionManager::ToggleSelect(uint32_t componentId) {
    if (IsSelected(componentId)) Deselect(componentId);
    else Select(componentId);
}

void SelectionManager::ClearSelection() {
    m_SelectedIds.clear();
}

bool SelectionManager::IsSelected(uint32_t componentId) const {
    return m_SelectedIds.count(componentId) > 0;
}

void SelectionManager::SelectFace(uint32_t componentId, uint32_t faceIndex) {
    if (!m_MultiSelectMode) m_BRepSelections.clear();
    m_BRepSelections.push_back({componentId, BRepSelectionMode::Face, faceIndex});
    m_ActiveMode = BRepSelectionMode::Face;
    m_SelectedIds.insert(componentId);
}

void SelectionManager::SelectEdge(uint32_t componentId, uint32_t edgeIndex) {
    if (!m_MultiSelectMode) m_BRepSelections.clear();
    m_BRepSelections.push_back({componentId, BRepSelectionMode::Edge, edgeIndex});
    m_ActiveMode = BRepSelectionMode::Edge;
    m_SelectedIds.insert(componentId);
}

void SelectionManager::SelectVertex(uint32_t componentId, uint32_t vertexIndex) {
    if (!m_MultiSelectMode) m_BRepSelections.clear();
    m_BRepSelections.push_back({componentId, BRepSelectionMode::Vertex, vertexIndex});
    m_ActiveMode = BRepSelectionMode::Vertex;
    m_SelectedIds.insert(componentId);
}

void SelectionManager::ClearBRepSelection() {
    m_BRepSelections.clear();
    m_ActiveMode = BRepSelectionMode::None;
}

} // namespace SZM::Graphics
