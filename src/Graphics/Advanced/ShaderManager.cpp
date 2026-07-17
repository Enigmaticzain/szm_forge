#include "ShaderManager.hpp"
#include <iostream>

namespace SZM::Graphics {

ShaderManager& ShaderManager::GetInstance() {
    static ShaderManager instance;
    return instance;
}

void ShaderManager::Init() {
    // Pre-compile common shader combinations
    for (int m = 0; m < 6; ++m) {
        for (int l = 0; l < 3; ++l) {
            auto mode = static_cast<RenderMode>(m);
            auto lighting = static_cast<LightingMode>(l);
            auto shader = CreateShader(mode, lighting);
            if (shader) {
                size_t key = HashShaderKey(mode, lighting);
                m_Shaders[key] = std::move(shader);
            }
        }
    }
    std::cout << "[ShaderManager] Initialized " << m_Shaders.size() << " shader programs\n";
}

void ShaderManager::Shutdown() {
    m_Shaders.clear();
    m_ActiveShader = nullptr;
}

ShaderProgram* ShaderManager::GetShader(RenderMode mode, LightingMode lighting) {
    size_t key = HashShaderKey(mode, lighting);
    auto it = m_Shaders.find(key);
    if (it != m_Shaders.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ShaderManager::SetRenderMode(RenderMode mode, LightingMode lighting) {
    m_CurrentMode = mode;
    m_CurrentLighting = lighting;
    m_ActiveShader = GetShader(mode, lighting);
    if (m_ActiveShader) {
        m_ActiveShader->Use();
    }
}

size_t ShaderManager::HashShaderKey(RenderMode mode, LightingMode lighting) {
    return (static_cast<size_t>(mode) << 8) | static_cast<size_t>(lighting);
}

std::unique_ptr<ShaderProgram> ShaderManager::CreateShader(RenderMode mode, LightingMode lighting) {
    try {
        std::string vertSrc = GetVertexShaderSource(mode);
        std::string fragSrc = GetFragmentShaderSource(mode, lighting);
        return std::make_unique<ShaderProgram>(vertSrc, fragSrc);
    } catch (const std::exception& e) {
        std::cerr << "[ShaderManager] Failed to create shader: " << e.what() << "\n";
        return nullptr;
    }
}

std::string ShaderManager::GetVertexShaderSource(RenderMode mode) {
    switch (mode) {
        case RenderMode::NormalMap:
            return R"(
                #version 450 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aNormal;
                layout (location = 2) in vec3 aTangent;

                uniform mat4 u_Model;
                uniform mat4 u_View;
                uniform mat4 u_Proj;

                out VS_OUT {
                    vec3 Normal;
                    vec3 Tangent;
                    vec3 Bitangent;
                    vec3 WorldPos;
                } vs_out;

                void main() {
                    vs_out.WorldPos = (u_Model * vec4(aPos, 1.0)).xyz;
                    vs_out.Normal = normalize(mat3(transpose(inverse(u_Model))) * aNormal);
                    vs_out.Tangent = normalize(mat3(u_Model) * aTangent);
                    vs_out.Bitangent = cross(vs_out.Normal, vs_out.Tangent);
                    gl_Position = u_Proj * u_View * vec4(vs_out.WorldPos, 1.0);
                }
            )";

        case RenderMode::DepthMap:
            return R"(
                #version 450 core
                layout (location = 0) in vec3 aPos;

                uniform mat4 u_Model;
                uniform mat4 u_View;
                uniform mat4 u_Proj;

                void main() {
                    gl_Position = u_Proj * u_View * u_Model * vec4(aPos, 1.0);
                }
            )";

        default: // Solid, Wireframe, WireframeShaded, FlatShaded
            return R"(
                #version 450 core
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aNormal;

                uniform mat4 u_Model;
                uniform mat4 u_View;
                uniform mat4 u_Proj;

                out VS_OUT {
                    vec3 Normal;
                    vec3 WorldPos;
                    vec3 FragPos;
                } vs_out;

                void main() {
                    vs_out.WorldPos = (u_Model * vec4(aPos, 1.0)).xyz;
                    vs_out.Normal = normalize(mat3(transpose(inverse(u_Model))) * aNormal);
                    vs_out.FragPos = aPos;
                    gl_Position = u_Proj * u_View * vec4(vs_out.WorldPos, 1.0);
                }
            )";
    }
}

std::string ShaderManager::GetFragmentShaderSource(RenderMode mode, LightingMode lighting) {
    // Wireframe mode
    if (mode == RenderMode::Wireframe) {
        return R"(
            #version 450 core
            out vec4 FragColor;

            void main() {
                FragColor = vec4(1.0, 1.0, 1.0, 1.0);
            }
        )";
    }

    // Normal map visualization
    if (mode == RenderMode::NormalMap) {
        return R"(
            #version 450 core
            in VS_OUT {
                vec3 Normal;
                vec3 Tangent;
                vec3 Bitangent;
                vec3 WorldPos;
            } fs_in;

            out vec4 FragColor;

            void main() {
                vec3 normal = normalize(fs_in.Normal);
                FragColor = vec4(normal * 0.5 + 0.5, 1.0);
            }
        )";
    }

    // Depth map visualization
    if (mode == RenderMode::DepthMap) {
        return R"(
            #version 450 core
            out vec4 FragColor;

            void main() {
                float depth = gl_FragCoord.z;
                FragColor = vec4(vec3(depth), 1.0);
            }
        )";
    }

    // Flat shaded
    if (mode == RenderMode::FlatShaded) {
        return R"(
            #version 450 core
            in VS_OUT {
                vec3 Normal;
                vec3 WorldPos;
                vec3 FragPos;
            } fs_in;

            uniform vec3 u_Color;
            uniform vec3 u_LightPos;
            uniform vec3 u_ViewPos;

            out vec4 FragColor;

            void main() {
                vec3 norm = normalize(fs_in.Normal);
                vec3 lightDir = normalize(u_LightPos - fs_in.WorldPos);
                float diff = max(dot(norm, lightDir), 0.0);
                vec3 color = u_Color * (0.3 + 0.7 * diff);
                FragColor = vec4(color, 1.0);
            }
        )";
    }

    // Solid/WireframeShaded with lighting
    if (lighting == LightingMode::Phong) {
        return R"(
            #version 450 core
            in VS_OUT {
                vec3 Normal;
                vec3 WorldPos;
                vec3 FragPos;
            } fs_in;

            uniform vec3 u_Color;
            uniform vec3 u_LightPos;
            uniform vec3 u_ViewPos;
            uniform float u_Shininess;

            out vec4 FragColor;

            void main() {
                vec3 norm = normalize(fs_in.Normal);
                vec3 lightDir = normalize(u_LightPos - fs_in.WorldPos);
                vec3 viewDir = normalize(u_ViewPos - fs_in.WorldPos);
                vec3 reflectDir = reflect(-lightDir, norm);

                float diff = max(dot(norm, lightDir), 0.0);
                float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess);

                vec3 ambient = u_Color * 0.2;
                vec3 diffuse = u_Color * diff * 0.7;
                vec3 specular = vec3(1.0) * spec * 0.5;

                vec3 result = ambient + diffuse + specular;
                FragColor = vec4(result, 1.0);
            }
        )";
    }

    // Default: PBR or Solid
    return R"(
        #version 450 core
        in VS_OUT {
            vec3 Normal;
            vec3 WorldPos;
            vec3 FragPos;
        } fs_in;

        uniform vec3 u_Color;
        uniform vec3 u_LightPos;
        uniform vec3 u_ViewPos;

        out vec4 FragColor;

        void main() {
            vec3 norm = normalize(fs_in.Normal);
            vec3 lightDir = normalize(u_LightPos - fs_in.WorldPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 color = u_Color * (0.3 + 0.7 * diff);
            FragColor = vec4(color, 1.0);
        }
    )";
}

} // namespace SZM::Graphics
