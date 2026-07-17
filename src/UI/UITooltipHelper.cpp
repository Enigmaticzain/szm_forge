#include "UITooltipHelper.hpp"
#include "UIStyleHelper.hpp"

namespace SZM::UITooltip {

void ApplyTooltipStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.HoverDelayShort = 0.15f;
    style.HoverDelayNormal = 0.35f;
    style.HoverFlagsForTooltipMouse =
        ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_AllowWhenDisabled;
}

void HelpMarker(const char* description) {
    if (!description || description[0] == '\0') {
        return;
    }

    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
        ImGui::SetTooltip("%s", description);
    }
}

void Hover(const char* description) {
    if (description && description[0] != '\0') {
        ImGui::SetItemTooltip("%s", description);
    }
}

void Label(const char* label, const char* description) {
    ImGui::TextUnformatted(label);
    if (description && description[0] != '\0') {
        HelpMarker(description);
    }
}

bool Button(const char* label, const char* description, const ImVec2& size) {
    const bool pressed = ImGui::Button(label, size);
    Hover(description);
    return pressed;
}

bool ButtonPrimary(const char* label, const char* description, const ImVec2& size) {
    const bool pressed = UIStyleHelper::ModernButtonPrimary(label, size);
    Hover(description);
    return pressed;
}

bool Checkbox(const char* label, bool* value, const char* description) {
    const bool changed = ImGui::Checkbox(label, value);
    Hover(description);
    return changed;
}

bool SliderFloat(
    const char* label,
    float* value,
    float min,
    float max,
    const char* description,
    const char* format
) {
    const bool changed = ImGui::SliderFloat(label, value, min, max, format);
    Hover(description);
    return changed;
}

bool DragFloat3(
    const char* label,
    float values[3],
    float speed,
    const char* description,
    float min,
    float max,
    const char* format
) {
    const bool changed = (min < max)
        ? ImGui::DragFloat3(label, values, speed, min, max, format)
        : ImGui::DragFloat3(label, values, speed, 0.0f, 0.0f, format);
    Hover(description);
    return changed;
}

bool InputText(
    const char* label,
    char* buffer,
    std::size_t bufferSize,
    const char* description,
    ImGuiInputTextFlags flags
) {
    const bool changed = ImGui::InputText(label, buffer, bufferSize, flags);
    Hover(description);
    return changed;
}

bool Combo(
    const char* label,
    int* currentItem,
    const char* const items[],
    int itemCount,
    const char* description
) {
    const bool changed = ImGui::Combo(label, currentItem, items, itemCount);
    Hover(description);
    return changed;
}

void SectionHint(const char* text) {
    if (!text || text[0] == '\0') {
        return;
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

} // namespace SZM::UITooltip
