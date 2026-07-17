#pragma once

#include <imgui.h>
#include <string>

namespace SZM::UITooltip {

/** Small (?) icon; shows tooltip when hovered. Place after a label on the same line. */
void HelpMarker(const char* description);

/** Tooltip on the preceding ImGui item (uses ImGuiHoveredFlags_ForTooltip). */
void Hover(const char* description);

/** Label text plus optional HelpMarker on the same line. */
void Label(const char* label, const char* description);

bool Button(const char* label, const char* description, const ImVec2& size = ImVec2(0, 0));
bool ButtonPrimary(const char* label, const char* description, const ImVec2& size = ImVec2(0, 0));

bool Checkbox(const char* label, bool* value, const char* description);
bool SliderFloat(const char* label, float* value, float min, float max, const char* description, const char* format = "%.2f");
bool DragFloat3(const char* label, float values[3], float speed, const char* description, float min = 0.0f, float max = 0.0f, const char* format = "%.2f");
bool InputText(const char* label, char* buffer, std::size_t bufferSize, const char* description, ImGuiInputTextFlags flags = 0);
bool Combo(const char* label, int* currentItem, const char* const items[], int itemCount, const char* description);

/** Muted one-line hint block at top of a panel section. */
void SectionHint(const char* text);

/** Register default hover delay for tooltips (slightly faster than ImGui default). */
void ApplyTooltipStyle();

} // namespace SZM::UITooltip
