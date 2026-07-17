#pragma once

#include "RenderMode.hpp"
#include "../ShaderProgram.hpp"
#include <memory>
#include <unordered_map>
#include <string>

namespace SZM::Graphics {

/**
 * @class ShaderManager
 * @brief Manages multiple shader programs for different rendering modes
 *
 * Features:
 *  - Shader caching and reuse
 *  - Multiple render mode support
 *  - Lighting model switching
 *  - Uniform management
 */
class ShaderManager {
public:
    /**
     * @brief Get singleton instance
     */
    static ShaderManager& GetInstance();

    /**\n     * @brief Initialize all shader programs
     */
    void Init();

    /**
     * @brief Shutdown and cleanup shaders
     */
    void Shutdown();

    /**
     * @brief Get shader for render mode
     * @param mode Render mode
     * @param lighting Lighting model
     * @return Shader program pointer
     */
    ShaderProgram* GetShader(RenderMode mode, LightingMode lighting);

    /**
     * @brief Get current active shader
     * @return Active shader program
     */
    ShaderProgram* GetActiveShader() const { return m_ActiveShader; }

    /**
     * @brief Set active render mode
     * @param mode Render mode
     * @param lighting Lighting model
     */
    void SetRenderMode(RenderMode mode, LightingMode lighting);

    /**
     * @brief Get current render mode
     */
    RenderMode GetRenderMode() const { return m_CurrentMode; }

    /**
     * @brief Get current lighting mode
     */
    LightingMode GetLightingMode() const { return m_CurrentLighting; }

private:
    ShaderManager() = default;
    ~ShaderManager() = default;

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    using ShaderKey = std::pair<RenderMode, LightingMode>;
    std::unordered_map<size_t, std::unique_ptr<ShaderProgram>> m_Shaders;
    ShaderProgram* m_ActiveShader = nullptr;
    RenderMode m_CurrentMode = RenderMode::Solid;
    LightingMode m_CurrentLighting = LightingMode::PBR;

    /**
     * @brief Create shader for mode combination
     */
    std::unique_ptr<ShaderProgram> CreateShader(RenderMode mode, LightingMode lighting);

    /**
     * @brief Get shader source for mode
     */
    std::string GetVertexShaderSource(RenderMode mode);
    std::string GetFragmentShaderSource(RenderMode mode, LightingMode lighting);

    /**
     * @brief Hash function for shader key
     */
    static size_t HashShaderKey(RenderMode mode, LightingMode lighting);
};

} // namespace SZM::Graphics
