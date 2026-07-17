#pragma once

#include "../ModernPanelBase.hpp"
#include <string>
#include <map>

namespace SZM {

/**
 * @class ModernPropertiesPanel
 * @brief Modern properties inspector with collapsible sections
 */
class ModernPropertiesPanel : public ModernPanelBase {
public:
    ModernPropertiesPanel() = default;
    ~ModernPropertiesPanel() override = default;

    const char* GetName() const override { return "Modern Properties"; }
    std::string GetTitle() const override { return "Properties Inspector"; }
    const char* GetIcon() const override { return "⚙️"; }

protected:
    void DrawContent() override;
    void DrawHeader() override;

private:
    void DrawTransformSection();
    void DrawMaterialSection();
    void DrawPhysicsSection();
    void DrawAdvancedSection();

    std::map<std::string, bool> m_SectionExpanded = {
        {"Transform", true},
        {"Material", true},
        {"Physics", false},
        {"Advanced", false}
    };
};

} // namespace SZM
