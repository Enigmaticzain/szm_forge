#include "Viewport.hpp"
#include "ComponentRenderer.hpp"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

namespace SZM::Graphics {

Viewport::Viewport(uint32_t width, uint32_t height)
    : m_RenderTarget(width, height), m_Width(width), m_Height(height) {
    m_Camera.SetAspectRatio(static_cast<float>(width), static_cast<float>(height));
}

Viewport::~Viewport() = default;

void Viewport::Resize(uint32_t width, uint32_t height) {
    m_Width = width;
    m_Height = height;
    m_RenderTarget = RenderTarget(width, height);
    m_Camera.SetAspectRatio(static_cast<float>(width), static_cast<float>(height));
}

void Viewport::Render(const SimulationEngine& engine, const Heatmap& heatmap) {
    m_RenderTarget.Bind();
    Clear();
    RenderComponents(engine, heatmap);
    m_RenderTarget.Unbind();
}

uint32_t Viewport::GetTextureID() const {
    return m_RenderTarget.GetTexture();
}

void Viewport::HandleMouseInput(float deltaX, float deltaY, int button) {
    if (button == 0) {  // Left mouse - orbit
        m_Camera.Orbit(deltaX * 0.01f, deltaY * 0.01f);
    } else if (button == 1) {  // Right mouse - pan
        m_Camera.Pan(-deltaX * 0.01f, deltaY * 0.01f);
    }
}

void Viewport::HandleScroll(float delta) {
    m_Camera.Zoom(delta > 0 ? 1.1f : 0.9f);
}

void Viewport::Clear() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void Viewport::RenderComponents(const SimulationEngine& engine, const Heatmap& heatmap) {
    const auto& components = engine.GetComponents();
    for (const auto& comp : components) {
        ComponentRenderer::RenderComponent(*comp, m_Camera, heatmap, m_RenderTarget);
    }
}

} // namespace SZM::Graphics
