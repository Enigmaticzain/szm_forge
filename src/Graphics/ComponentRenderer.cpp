#include "ComponentRenderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

// GLAD must be included before GLFW
#include <glad/gl.h>

namespace SZM::Graphics {

namespace {

struct MeshBuffers {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
};

GLuint g_ShaderProgram = 0;
GLint g_ModelLoc = -1;
GLint g_ViewLoc = -1;
GLint g_ProjLoc = -1;
GLint g_ColorLoc = -1;
MeshBuffers g_CubeMesh{};
MeshBuffers g_SphereMesh{};
std::array<float, 16> g_CurrentView = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
std::array<float, 16> g_CurrentProj = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024] = {};
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "[ComponentRenderer] Shader compile failed: " << log << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint BuildProgram() {
    static constexpr const char* kVertexShader = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 u_Model;
        uniform mat4 u_View;
        uniform mat4 u_Proj;

        void main() {
            gl_Position = u_Proj * u_View * u_Model * vec4(aPos, 1.0);
        }
    )";

    static constexpr const char* kFragmentShader = R"(
        #version 450 core
        out vec4 FragColor;
        uniform vec3 u_Color;

        void main() {
            FragColor = vec4(u_Color, 1.0);
        }
    )";

    const GLuint vs = CompileShader(GL_VERTEX_SHADER, kVertexShader);
    const GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024] = {};
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "[ComponentRenderer] Program link failed: " << log << "\n";
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

void CreateMesh(
    MeshBuffers& mesh,
    const std::vector<float>& vertices,
    const std::vector<uint32_t>& indices)
{
    mesh.indexCount = static_cast<GLsizei>(indices.size());
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)),
        indices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void DestroyMesh(MeshBuffers& mesh) {
    if (mesh.ebo) glDeleteBuffers(1, &mesh.ebo);
    if (mesh.vbo) glDeleteBuffers(1, &mesh.vbo);
    if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
    mesh = {};
}

std::vector<float> BuildCubeVertices() {
    return {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };
}

std::vector<uint32_t> BuildCubeIndices() {
    return {
        0, 1, 2, 2, 3, 0, // back
        4, 5, 6, 6, 7, 4, // front
        0, 4, 7, 7, 3, 0, // left
        1, 5, 6, 6, 2, 1, // right
        3, 2, 6, 6, 7, 3, // top
        0, 1, 5, 5, 4, 0  // bottom
    };
}

void BuildSphereMesh(std::vector<float>& outVertices, std::vector<uint32_t>& outIndices) {
    constexpr int kStacks = 16;
    constexpr int kSlices = 24;
    constexpr float kPi = 3.14159265358979323846f;
    outVertices.clear();
    outIndices.clear();
    outVertices.reserve(static_cast<size_t>((kStacks + 1) * (kSlices + 1) * 3));
    outIndices.reserve(static_cast<size_t>(kStacks * kSlices * 6));

    for (int stack = 0; stack <= kStacks; ++stack) {
        const float phi = kPi * static_cast<float>(stack) / static_cast<float>(kStacks);
        const float y = std::cos(phi);
        const float r = std::sin(phi);
        for (int slice = 0; slice <= kSlices; ++slice) {
            const float theta = 2.0f * kPi * static_cast<float>(slice) / static_cast<float>(kSlices);
            outVertices.push_back(r * std::cos(theta));
            outVertices.push_back(y);
            outVertices.push_back(r * std::sin(theta));
        }
    }

    const int stride = kSlices + 1;
    for (int stack = 0; stack < kStacks; ++stack) {
        for (int slice = 0; slice < kSlices; ++slice) {
            const uint32_t a = static_cast<uint32_t>(stack * stride + slice);
            const uint32_t b = static_cast<uint32_t>((stack + 1) * stride + slice);
            const uint32_t c = static_cast<uint32_t>(a + 1);
            const uint32_t d = static_cast<uint32_t>(b + 1);
            outIndices.push_back(a); outIndices.push_back(b); outIndices.push_back(c);
            outIndices.push_back(c); outIndices.push_back(b); outIndices.push_back(d);
        }
    }
}

std::array<float, 16> BuildModelMatrix(
    const Geometry::Vector3& pos,
    const Geometry::Vector3& scale)
{
    return {
        static_cast<float>(scale.x), 0.0f, 0.0f, 0.0f,
        0.0f, static_cast<float>(scale.y), 0.0f, 0.0f,
        0.0f, 0.0f, static_cast<float>(scale.z), 0.0f,
        static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z), 1.0f
    };
}

void DrawMesh(const MeshBuffers& mesh, const std::array<float, 16>& model, const Geometry::Vector3& color) {
    if (!g_ShaderProgram || !mesh.vao || mesh.indexCount == 0) {
        return;
    }

    glUseProgram(g_ShaderProgram);
    glUniformMatrix4fv(g_ModelLoc, 1, GL_FALSE, model.data());
    glUniformMatrix4fv(g_ViewLoc, 1, GL_FALSE, g_CurrentView.data());
    glUniformMatrix4fv(g_ProjLoc, 1, GL_FALSE, g_CurrentProj.data());
    glUniform3f(g_ColorLoc, static_cast<float>(color.x), static_cast<float>(color.y), static_cast<float>(color.z));

    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void SetCameraMatrices(const Camera& camera) {
    const Matrix4x4 view = camera.GetViewMatrix();
    const Matrix4x4 proj = camera.GetProjectionMatrix();
    std::copy(std::begin(view.m), std::end(view.m), g_CurrentView.begin());
    std::copy(std::begin(proj.m), std::end(proj.m), g_CurrentProj.begin());
}

} // namespace

    void ComponentRenderer::Init() {
        SetupShaders();
    }

    void ComponentRenderer::Shutdown() {
        DestroyMesh(g_CubeMesh);
        DestroyMesh(g_SphereMesh);
        if (g_ShaderProgram) {
            glDeleteProgram(g_ShaderProgram);
            g_ShaderProgram = 0;
        }
    }

    void ComponentRenderer::SetupShaders() {
        if (g_ShaderProgram) {
            return;
        }

        g_ShaderProgram = BuildProgram();
        if (!g_ShaderProgram) {
            return;
        }

        g_ModelLoc = glGetUniformLocation(g_ShaderProgram, "u_Model");
        g_ViewLoc = glGetUniformLocation(g_ShaderProgram, "u_View");
        g_ProjLoc = glGetUniformLocation(g_ShaderProgram, "u_Proj");
        g_ColorLoc = glGetUniformLocation(g_ShaderProgram, "u_Color");

        CreateMesh(g_CubeMesh, BuildCubeVertices(), BuildCubeIndices());

        std::vector<float> sphereVertices;
        std::vector<uint32_t> sphereIndices;
        BuildSphereMesh(sphereVertices, sphereIndices);
        CreateMesh(g_SphereMesh, sphereVertices, sphereIndices);
    }

    Geometry::Vector3 ComponentRenderer::GetComponentColor(
        const SimulationComponent& comp,
        const Heatmap& heatmap
    ) {
        float ratio = 0.0f;

        switch (heatmap.GetMode()) {
            case Heatmap::Mode::Stress:
                ratio = comp.stress;
                break;
            case Heatmap::Mode::Temperature:
                ratio = (comp.temperature - 293.15f) / 80.0f;  // 20°C to 100°C
                ratio = std::max(0.0f, std::min(1.0f, ratio));
                break;
            case Heatmap::Mode::DangerIndicator:
                ratio = comp.isDangerous ? 1.0f : 0.0f;
                break;
            case Heatmap::Mode::None:
            default:
                return Geometry::Vector3(0.8f, 0.8f, 0.8f);  // Gray
        }

        return heatmap.GetColor(ratio);
    }

    void ComponentRenderer::RenderComponent(
        const SimulationComponent& comp,
        const Camera& camera,
        const Heatmap& heatmap,
        RenderTarget& target
    ) {
        if (!target.IsBound()) {
            target.Bind();
        }
        SetCameraMatrices(camera);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Get component color based on heatmap
        Geometry::Vector3 color = GetComponentColor(comp, heatmap);

        // Render component as cube
        // Position: use component ID as offset for now
        Geometry::Vector3 pos(
            static_cast<float>(comp.id % 10) * 2.0f,
            static_cast<float>(comp.id / 10) * 2.0f,
            0.0f
        );

        // Scale based on area
        float scale = std::sqrt(comp.area) * 0.5f;
        Geometry::Vector3 scaleVec(scale, scale, scale);

        if (comp.name.find("Bolt") != std::string::npos ||
            comp.name.find("Pin") != std::string::npos) {
            RenderSphere(pos, scale * 0.55f, color);
        } else {
            RenderCube(pos, scaleVec, color);
        }
    }

    void ComponentRenderer::RenderAssembly(
        const Assembly::AssemblyGraphManager& manager,
        const Camera& camera,
        const Heatmap& heatmap,
        RenderTarget& target
    ) {
        if (!target.IsBound()) {
            target.Bind();
        }
        SetCameraMatrices(camera);

        // Clear framebuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        const auto bom = manager.GenerateBOM();
        for (size_t i = 0; i < bom.size(); ++i) {
            const float t = bom.empty()
                ? 0.0f
                : static_cast<float>(i) / static_cast<float>(std::max<size_t>(bom.size() - 1, 1));
            const Geometry::Vector3 color = heatmap.GetColor(t);
            const double base = 0.5 + std::sqrt(static_cast<double>(std::max<uint32_t>(1u, bom[i].Quantity))) * 0.18;
            const Geometry::Vector3 pos(-1.5 + static_cast<double>(i) * 1.3, 0.0, 0.0);
            RenderCube(pos, Geometry::Vector3(base, base, base), color);
        }
    }

    void ComponentRenderer::RenderCube(
        const Geometry::Vector3& pos,
        const Geometry::Vector3& scale,
        const Geometry::Vector3& color
    ) {
        DrawMesh(g_CubeMesh, BuildModelMatrix(pos, scale), color);
    }

    void ComponentRenderer::RenderSphere(
        const Geometry::Vector3& pos,
        float radius,
        const Geometry::Vector3& color
    ) {
        const Geometry::Vector3 scale(radius * 2.0, radius * 2.0, radius * 2.0);
        DrawMesh(g_SphereMesh, BuildModelMatrix(pos, scale), color);
    }

} // namespace SZM::Graphics
