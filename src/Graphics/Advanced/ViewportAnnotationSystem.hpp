#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @enum AnnotationType
 * @brief Types of viewport annotations
 */
enum class AnnotationType {
    Label,
    Measurement,
    Arrow,
    Circle,
    Text
};

/**
 * @struct Annotation
 * @brief Viewport annotation data
 */
struct Annotation {
    uint32_t id;
    AnnotationType type;
    glm::vec3 position;
    glm::vec3 color = glm::vec3(1.0f);
    std::string text;
    bool visible = true;
};

/**
 * @class ViewportAnnotationSystem
 * @brief Manages viewport annotations and labels
 */
class ViewportAnnotationSystem {
public:
    static ViewportAnnotationSystem& GetInstance();

    /**
     * @brief Add annotation
     */
    uint32_t AddAnnotation(const Annotation& annotation);

    /**
     * @brief Remove annotation by ID
     */
    void RemoveAnnotation(uint32_t id);

    /**
     * @brief Get annotation by ID
     */
    Annotation* GetAnnotation(uint32_t id);

    /**
     * @brief Clear all annotations
     */
    void ClearAnnotations();

    /**
     * @brief Get all annotations
     */
    const std::vector<Annotation>& GetAnnotations() const { return m_Annotations; }

    /**
     * @brief Add label at position
     */
    uint32_t AddLabel(const glm::vec3& pos, const std::string& text);

    /**
     * @brief Add measurement between two points
     */
    uint32_t AddMeasurement(const glm::vec3& start, const glm::vec3& end, const std::string& label);

    /**
     * @brief Add arrow annotation
     */
    uint32_t AddArrow(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color);

    /**
     * @brief Toggle annotation visibility
     */
    void SetAnnotationVisibility(uint32_t id, bool visible);

    /**
     * @brief Render all annotations
     */
    void Render();

private:
    ViewportAnnotationSystem() = default;
    ~ViewportAnnotationSystem() = default;

    ViewportAnnotationSystem(const ViewportAnnotationSystem&) = delete;
    ViewportAnnotationSystem& operator=(const ViewportAnnotationSystem&) = delete;

    std::vector<Annotation> m_Annotations;
    uint32_t m_NextId = 1;

    void RenderLabel(const Annotation& annotation);
    void RenderMeasurement(const Annotation& annotation);
    void RenderArrow(const Annotation& annotation);
};

} // namespace SZM::Graphics
