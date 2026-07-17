#pragma once

#include <string>
#include <glm/glm.hpp>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @class ShaderProgram
 * @brief Manages OpenGL shader compilation and linking
 *
 * Features:
 *  - Vertex/Fragment shader compilation
 *  - Program linking
 *  - Uniform setting (float, vec3, mat4)
 *  - Use/bind operations
 */
class ShaderProgram {
public:
    /**
     * @brief Create shader program from source strings
     * @param vertexSrc Vertex shader source code
     * @param fragmentSrc Fragment shader source code
     */
    ShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
    
    /**
     * @brief Destroy shader program
     */
    ~ShaderProgram();
    
    // Prevent copying
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    
    /**
     * @brief Use this shader program
     */
    void Use() const;
    
    /**
     * @brief Get program ID
     * @return OpenGL program ID
     */
    uint32_t GetID() const { return m_ProgramID; }
    
    /**
     * @brief Set uniform float
     * @param name Uniform name
     * @param value Float value
     */
    void SetFloat(const std::string& name, float value) const;
    
    /**
     * @brief Set uniform vec3
     * @param name Uniform name
     * @param value Vector value
     */
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    
    /**
     * @brief Set uniform mat4
     * @param name Uniform name
     * @param value Matrix value
     */
    void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
    uint32_t m_ProgramID = 0;
    
    /**
     * @brief Compile shader from source
     * @param source Shader source code
     * @param type GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * @return Compiled shader ID
     */
    uint32_t CompileShader(const std::string& source, uint32_t type);
};

} // namespace SZM::Graphics
