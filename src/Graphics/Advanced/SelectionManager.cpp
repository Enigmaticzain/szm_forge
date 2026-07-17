#include "SelectionManager.hpp"

namespace SZM::Graphics {

SelectionManager& SelectionManager::GetInstance() {
    static SelectionManager instance;
    return instance;
}

void SelectionManager::Select(uint32_t componentId) {
    if (!m_MultiSelectMode) {
        m_SelectedIds.clear();
    }
    m_SelectedIds.insert(componentId);
}

void SelectionManager::Deselect(uint32_t componentId) {
    m_SelectedIds.erase(componentId);
}

void SelectionManager::ToggleSelect(uint32_t componentId) {
    if (IsSelected(componentId)) {
        Deselect(componentId);
    } else {
        Select(componentId);
    }
}

void SelectionManager::ClearSelection() {
    m_SelectedIds.clear();
}

bool SelectionManager::IsSelected(uint32_t componentId) const {
    return m_SelectedIds.find(componentId) != m_SelectedIds.end();
}

} // namespace SZM::Graphics
