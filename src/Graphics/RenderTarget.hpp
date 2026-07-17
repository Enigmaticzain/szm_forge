#pragma once

#include <cstdint>

namespace SZM::Graphics {

/**
 * @class RenderTarget
 * @brief Framebuffer abstraction for render-to-texture operations
 *
 * Features:
 *  - OpenGL framebuffer creation and management
 *  - Texture attachment for rendering
 *  - Renderbuffer for depth
 *  - Bind/unbind operations
 *  - Error checking
 */
class RenderTarget {
public:
    /**
     * @brief Create a render target with specified dimensions
     * @param width Texture width in pixels
     * @param height Texture height in pixels
     */
    RenderTarget(uint32_t width, uint32_t height);
    RenderTarget(RenderTarget&& other) noexcept;
    
    /**
     * @brief Destroy render target and free GPU resources
     */
    ~RenderTarget();
    
    // Prevent copying
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;
    RenderTarget& operator=(RenderTarget&& other) noexcept;
    
    /**
     * @brief Bind framebuffer for rendering
     */
    void Bind();
    
    /**
     * @brief Unbind framebuffer (restore default framebuffer)
     */
    void Unbind();
    
    /**
     * @brief Get OpenGL texture ID
     * @return Texture ID for use in ImGui::Image()
     */
    uint32_t GetTexture() const { return m_Texture; }
    
    /**
     * @brief Get OpenGL framebuffer ID
     * @return Framebuffer ID
     */
    uint32_t GetFramebuffer() const { return m_Framebuffer; }
    
    /**
     * @brief Get texture width
     * @return Width in pixels
     */
    uint32_t GetWidth() const { return m_Width; }
    
    /**
     * @brief Get texture height
     * @return Height in pixels
     */
    uint32_t GetHeight() const { return m_Height; }
    
    /**
     * @brief Check if framebuffer is currently bound
     * @return True if bound, false otherwise
     */
    bool IsBound() const { return m_IsBound; }
    
    /**
     * @brief Check if framebuffer is valid
     * @return True if framebuffer is complete and valid
     */
    bool IsValid() const { return m_IsValid; }

private:
    uint32_t m_Framebuffer = 0;      ///< OpenGL framebuffer object
    uint32_t m_Texture = 0;          ///< Color attachment texture
    uint32_t m_Renderbuffer = 0;     ///< Depth attachment renderbuffer
    uint32_t m_Width = 0;            ///< Texture width
    uint32_t m_Height = 0;           ///< Texture height
    bool m_IsBound = false;          ///< Bind state
    bool m_IsValid = false;          ///< Validity flag
    
    /**
     * @brief Initialize framebuffer and attachments
     */
    void Initialize();
    
    /**
     * @brief Check framebuffer completeness
     * @return True if framebuffer is complete
     */
    bool CheckCompleteness();
    void Release();
    void MoveFrom(RenderTarget&& other) noexcept;
};

} // namespace SZM::Graphics
