#include "RenderTarget.hpp"
#include <iostream>

// GLAD must be included before GLFW
#include <glad/gl.h>

namespace SZM::Graphics {

    RenderTarget::RenderTarget(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height) {
        Initialize();
    }

    RenderTarget::RenderTarget(RenderTarget&& other) noexcept {
        MoveFrom(std::move(other));
    }

    RenderTarget::~RenderTarget() {
        Release();
    }

    RenderTarget& RenderTarget::operator=(RenderTarget&& other) noexcept {
        if (this != &other) {
            Release();
            MoveFrom(std::move(other));
        }
        return *this;
    }

    void RenderTarget::Initialize() {
        // Create framebuffer
        glGenFramebuffers(1, &m_Framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

        // Create color texture
        glGenTextures(1, &m_Texture);
        glBindTexture(GL_TEXTURE_2D, m_Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);

        // Create depth renderbuffer
        glGenRenderbuffers(1, &m_Renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_Renderbuffer);

        // Check completeness
        m_IsValid = CheckCompleteness();

        // Unbind
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if (!m_IsValid) {
            std::cerr << "RenderTarget: Framebuffer is not complete!\n";
        }
    }

    bool RenderTarget::CheckCompleteness() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
        
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        bool complete = (status == GL_FRAMEBUFFER_COMPLETE);

        if (!complete) {
            std::cerr << "RenderTarget: Framebuffer incomplete - ";
            switch (status) {
                case GL_FRAMEBUFFER_UNDEFINED:
                    std::cerr << "UNDEFINED\n";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    std::cerr << "INCOMPLETE_ATTACHMENT\n";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    std::cerr << "INCOMPLETE_MISSING_ATTACHMENT\n";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    std::cerr << "INCOMPLETE_DRAW_BUFFER\n";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    std::cerr << "INCOMPLETE_READ_BUFFER\n";
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    std::cerr << "UNSUPPORTED\n";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    std::cerr << "INCOMPLETE_MULTISAMPLE\n";
                    break;
                default:
                    std::cerr << "UNKNOWN\n";
                    break;
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return complete;
    }

    void RenderTarget::Bind() {
        if (!m_IsValid) {
            std::cerr << "RenderTarget: Cannot bind invalid framebuffer\n";
            return;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
        glViewport(0, 0, m_Width, m_Height);
        m_IsBound = true;
    }

    void RenderTarget::Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_IsBound = false;
    }

    void RenderTarget::Release() {
        if (m_IsBound) {
            Unbind();
        }

        if (m_Framebuffer != 0) {
            glDeleteFramebuffers(1, &m_Framebuffer);
        }
        if (m_Texture != 0) {
            glDeleteTextures(1, &m_Texture);
        }
        if (m_Renderbuffer != 0) {
            glDeleteRenderbuffers(1, &m_Renderbuffer);
        }

        m_Framebuffer = 0;
        m_Texture = 0;
        m_Renderbuffer = 0;
        m_Width = 0;
        m_Height = 0;
        m_IsBound = false;
        m_IsValid = false;
    }

    void RenderTarget::MoveFrom(RenderTarget&& other) noexcept {
        m_Framebuffer = other.m_Framebuffer;
        m_Texture = other.m_Texture;
        m_Renderbuffer = other.m_Renderbuffer;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_IsBound = other.m_IsBound;
        m_IsValid = other.m_IsValid;

        other.m_Framebuffer = 0;
        other.m_Texture = 0;
        other.m_Renderbuffer = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_IsBound = false;
        other.m_IsValid = false;
    }

} // namespace SZM::Graphics
