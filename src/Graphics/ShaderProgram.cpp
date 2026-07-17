#include "ShaderProgram.hpp"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace SZM::Graphics {

ShaderProgram::ShaderProgram(const std::string& vertexSrc, const std::string& fragmentSrc) {
    uint32_t vertexID = CompileShader(vertexSrc, GL_VERTEX_SHADER);
    uint32_t fragmentID = CompileShader(fragmentSrc, GL_FRAGMENT_SHADER);
    
    m_ProgramID = glCreateProgram();
    glAttachShader(m_ProgramID, vertexID);
    glAttachShader(m_ProgramID, fragmentID);
    glLinkProgram(m_ProgramID);
    
    int success;
    char infoLog[512];
    glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ProgramID, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexID);
    glDeleteShader(fragmentID);
}

ShaderProgram::~ShaderProgram() {
    if (m_ProgramID) {
        glDeleteProgram(m_ProgramID);
    }
}

void ShaderProgram::Use() const {
    glUseProgram(m_ProgramID);
}

void ShaderProgram::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_ProgramID, name.c_str()), value);
}

void ShaderProgram::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_ProgramID, name.c_str()), 1, glm::value_ptr(value));
}

void ShaderProgram::SetMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_ProgramID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

uint32_t ShaderProgram::CompileShader(const std::string& source, uint32_t type) {
    uint32_t shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    
    return shader;
}

} // namespace SZM::Graphics
