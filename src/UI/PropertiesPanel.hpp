#pragma once

#include "IPanel.hpp"
#include <string>

namespace SZM {

    class PropertiesPanel : public IPanel {
    public:
        PropertiesPanel();
        ~PropertiesPanel() = default;

        void Draw() override;
        std::string GetTitle() const override { return "Properties Inspector"; }

        void SetSelectedObject(const std::string& objectName);

    private:
        std::string m_SelectedObject;
        float m_Position[3] = {0.0f, 0.0f, 0.0f};
        float m_Rotation[3] = {0.0f, 0.0f, 0.0f};
        float m_Scale[3] = {1.0f, 1.0f, 1.0f};
    };

} // namespace SZM
