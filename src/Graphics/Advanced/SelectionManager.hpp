#pragma once

#include <cstdint>
#include <vector>
#include <unordered_set>

namespace SZM::Graphics {

/**
 * @class SelectionManager
 * @brief Manages component and part selection for interactive visualization
 */
class SelectionManager {
public:
    static SelectionManager& GetInstance();

    /**
     * @brief Select a component by ID
     */
    void Select(uint32_t componentId);

    /**
     * @brief Deselect a component
     */
    void Deselect(uint32_t componentId);

    /**
     * @brief Toggle selection of a component
     */
    void ToggleSelect(uint32_t componentId);

    /**
     * @brief Clear all selections
     */
    void ClearSelection();

    /**
     * @brief Check if component is selected
     */
    bool IsSelected(uint32_t componentId) const;

    /**
     * @brief Get all selected component IDs
     */
    const std::unordered_set<uint32_t>& GetSelectedIds() const { return m_SelectedIds; }

    /**
     * @brief Get selection count
     */
    size_t GetSelectionCount() const { return m_SelectedIds.size(); }

    /**
     * @brief Enable multi-select mode
     */
    void SetMultiSelectMode(bool enabled) { m_MultiSelectMode = enabled; }

    /**
     * @brief Check if multi-select is enabled
     */
    bool IsMultiSelectMode() const { return m_MultiSelectMode; }

private:
    SelectionManager() = default;
    ~SelectionManager() = default;

    SelectionManager(const SelectionManager&) = delete;
    SelectionManager& operator=(const SelectionManager&) = delete;

    std::unordered_set<uint32_t> m_SelectedIds;
    bool m_MultiSelectMode = false;
};

} // namespace SZM::Graphics
