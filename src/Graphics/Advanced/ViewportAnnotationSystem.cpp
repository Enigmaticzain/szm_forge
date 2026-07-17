#include "ViewportAnnotationSystem.hpp"
#include <GL/gl.h>
#include <algorithm>

namespace SZM::Graphics {

ViewportAnnotationSystem& ViewportAnnotationSystem::GetInstance() {
    static ViewportAnnotationSystem instance;
    return instance;
}

uint32_t ViewportAnnotationSystem::AddAnnotation(const Annotation& annotation) {
    Annotation ann = annotation;
    ann.id = m_NextId++;
    m_Annotations.push_back(ann);
    return ann.id;
}

void ViewportAnnotationSystem::RemoveAnnotation(uint32_t id) {
    auto it = std::find_if(m_Annotations.begin(), m_Annotations.end(),
        [id](const Annotation& a) { return a.id == id; });
    if (it != m_Annotations.end()) {
        m_Annotations.erase(it);
    }
}

Annotation* ViewportAnnotationSystem::GetAnnotation(uint32_t id) {
    auto it = std::find_if(m_Annotations.begin(), m_Annotations.end(),
        [id](const Annotation& a) { return a.id == id; });
    return it != m_Annotations.end() ? &(*it) : nullptr;
}

void ViewportAnnotationSystem::ClearAnnotations() {
    m_Annotations.clear();
}

uint32_t ViewportAnnotationSystem::AddLabel(const glm::vec3& pos, const std::string& text) {
    Annotation ann;
    ann.type = AnnotationType::Label;
    ann.position = pos;
    ann.text = text;
    return AddAnnotation(ann);
}

uint32_t ViewportAnnotationSystem::AddMeasurement(const glm::vec3& start, const glm::vec3& end, const std::string& label) {
    Annotation ann;
    ann.type = AnnotationType::Measurement;
    ann.position = (start + end) * 0.5f;
    ann.text = label;
    return AddAnnotation(ann);
}

uint32_t ViewportAnnotationSystem::AddArrow(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color) {
    Annotation ann;
    ann.type = AnnotationType::Arrow;
    ann.position = start;
    ann.color = color;
    return AddAnnotation(ann);
}

void ViewportAnnotationSystem::SetAnnotationVisibility(uint32_t id, bool visible) {
    auto ann = GetAnnotation(id);
    if (ann) {
        ann->visible = visible;
    }
}

void ViewportAnnotationSystem::Render() {
    glDisable(GL_DEPTH_TEST);

    for (const auto& annotation : m_Annotations) {
        if (!annotation.visible) continue;

        switch (annotation.type) {
            case AnnotationType::Label:
                RenderLabel(annotation);
                break;
            case AnnotationType::Measurement:
                RenderMeasurement(annotation);
                break;
            case AnnotationType::Arrow:
                RenderArrow(annotation);
                break;
            default:
                break;
        }
    }

    glEnable(GL_DEPTH_TEST);
}

void ViewportAnnotationSystem::RenderLabel(const Annotation& annotation) {
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    glColor3f(annotation.color.x, annotation.color.y, annotation.color.z);
    glVertex3f(annotation.position.x, annotation.position.y, annotation.position.z);
    glEnd();
    glPointSize(1.0f);
}

void ViewportAnnotationSystem::RenderMeasurement(const Annotation& annotation) {
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(annotation.color.x, annotation.color.y, annotation.color.z);
    glVertex3f(annotation.position.x - 0.1f, annotation.position.y, annotation.position.z);
    glVertex3f(annotation.position.x + 0.1f, annotation.position.y, annotation.position.z);
    glEnd();
    glLineWidth(1.0f);
}

void ViewportAnnotationSystem::RenderArrow(const Annotation& annotation) {
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(annotation.color.x, annotation.color.y, annotation.color.z);
    glVertex3f(annotation.position.x, annotation.position.y, annotation.position.z);
    glVertex3f(annotation.position.x + 0.2f, annotation.position.y, annotation.position.z);
    glEnd();
    glLineWidth(1.0f);
}

} // namespace SZM::Graphics
