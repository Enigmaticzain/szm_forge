#include "EnhancedRenderer.hpp"
#include "../GlmInterop.hpp"
#include "../ShaderProgram.hpp"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace SZM::Graphics {

static GLuint g_SolidShader = 0;
static GLuint g_WireframeShader = 0;
static GLuint g_NormalShader = 0;

EnhancedRenderer& EnhancedRenderer::GetInstance() {
    static EnhancedRenderer instance;
    return instance;
}

void EnhancedRenderer::Init() {
    SetupShaders();
}

void EnhancedRenderer::Shutdown() {
    if (g_SolidShader) glDeleteProgram(g_SolidShader);
    if (g_WireframeShader) glDeleteProgram(g_WireframeShader);
    if (g_NormalShader) glDeleteProgram(g_NormalShader);
}

void EnhancedRenderer::SetupShaders() {
    // Solid shader
    const char* solidVert = R"(
        #version 450 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec3 aColor;

        uniform mat4 u_Model;
        uniform mat4 u_View;
        uniform mat4 u_Proj;

        out vec3 vNormal;
        out vec3 vColor;
        out vec3 vWorldPos;

        void main() {
            vWorldPos = (u_Model * vec4(aPos, 1.0)).xyz;
            vNormal = normalize(mat3(transpose(inverse(u_Model))) * aNormal);
            vColor = aColor;
            gl_Position = u_Proj * u_View * vec4(vWorldPos, 1.0);
        }
    )";

    const char* solidFrag = R"(
        #version 450 core
        in vec3 vNormal;
        in vec3 vColor;
        in vec3 vWorldPos;

        uniform vec3 u_LightPos;
        uniform vec3 u_ViewPos;
        uniform int u_LightingMode;

        out vec4 FragColor;

        void main() {
            vec3 norm = normalize(vNormal);
            vec3 lightDir = normalize(u_LightPos - vWorldPos);
            vec3 viewDir = normalize(u_ViewPos - vWorldPos);

            float diff = max(dot(norm, lightDir), 0.0);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

            vec3 ambient = vColor * 0.2;
            vec3 diffuse = vColor * diff * 0.7;
            vec3 specular = vec3(1.0) * spec * 0.5;

            vec3 result = ambient + diffuse + specular;
            FragColor = vec4(result, 1.0);
        }
    )";

    // Compile solid shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &solidVert, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &solidFrag, nullptr);
    glCompileShader(fs);

    g_SolidShader = glCreateProgram();
    glAttachShader(g_SolidShader, vs);
    glAttachShader(g_SolidShader, fs);
    glLinkProgram(g_SolidShader);

    glDeleteShader(vs);
    glDeleteShader(fs);

    std::cout << "[EnhancedRenderer] Shaders initialized\n";
}

void EnhancedRenderer::RenderMesh(
    const AdvancedMesh& mesh,
    const glm::mat4& model,
    const glm::vec3& color,
    const Camera& camera,
    RenderTarget& target
) {
    if (!target.IsBound()) {
        target.Bind();
    }

    const glm::mat4 viewMatrix = ToGlmMat4(camera.GetViewMatrix());
    const glm::mat4 projectionMatrix = ToGlmMat4(camera.GetProjectionMatrix());

    glUseProgram(g_SolidShader);

    glUniformMatrix4fv(glGetUniformLocation(g_SolidShader, "u_Model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(g_SolidShader, "u_View"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(g_SolidShader, "u_Proj"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniform3fv(glGetUniformLocation(g_SolidShader, "u_LightPos"), 1, glm::value_ptr(m_LightPos));
    glUniform3fv(glGetUniformLocation(g_SolidShader, "u_ViewPos"), 1, glm::value_ptr(m_ViewPos));

    glEnable(GL_DEPTH_TEST);
    mesh.Draw(m_RenderMode);
}

void EnhancedRenderer::RenderMeshWithWireframe(
    const AdvancedMesh& mesh,
    const glm::mat4& model,
    const glm::vec3& color,
    const glm::vec3& wireframeColor,
    const Camera& camera,
    RenderTarget& target
) {
    const glm::mat4 viewMatrix = ToGlmMat4(camera.GetViewMatrix());
    const glm::mat4 projectionMatrix = ToGlmMat4(camera.GetProjectionMatrix());

    RenderMesh(mesh, model, color, camera, target);

    glUseProgram(g_WireframeShader);
    glUniformMatrix4fv(glGetUniformLocation(g_WireframeShader, "u_Model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(g_WireframeShader, "u_View"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(g_WireframeShader, "u_Proj"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    mesh.DrawWireframeOverlay(1.5f);
}

void EnhancedRenderer::UpdateUniforms(const Camera& camera, const glm::vec3& color) {
    // Implemented in RenderMesh
}

} // namespace SZM::Graphics
