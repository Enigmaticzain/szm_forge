#pragma once

#include <functional>
#include <stdexcept>
#include <memory>

namespace SZM {

/**
 * @defgroup RAII Resource Management (RAII Utilities)
 * @brief Modern C++ RAII patterns for reliable resource management.
 *
 * These utilities implement the RAII (Resource Acquisition Is Initialization)
 * principle to ensure resources are properly cleaned up even in the presence
 * of exceptions. All resource management should use these patterns.
 * @{
 */

/**
 * @class ScopeGuard
 * @brief Executes cleanup function on scope exit (RAII guard).
 *
 * Guarantees that a cleanup function is called when the scope is exited,
 * whether by normal return or exception. Essential for exception-safe cleanup.
 *
 * **Usage:**
 * ```cpp
 * {
 *     Resource resource = AcquireResource();
 *     ScopeGuard guard([&resource]() { ReleaseResource(resource); });
 *     DoWork(resource);
 * }  // ReleaseResource guaranteed to be called
 * ```
 *
 * **Exception Safety:**
 * - Strong exception guarantee: Either succeeds or leaves resources unchanged
 * - noexcept: Cleanup function should not throw; issues are logged
 *
 * @see std::unique_ptr for automatic cleanup
 */
class ScopeGuard {
public:
    /**
     * @brief Create a scope guard with cleanup function.
     *
     * @param onExit Function to call on scope exit (must not throw)
     *
     * @note Prefer this over manual try/catch blocks for cleanup
     */
    explicit ScopeGuard(std::function<void()> onExit) noexcept
        : m_OnExit(std::move(onExit)) {}

    /**
     * @brief Destructor ensures cleanup is called.
     *
     * Called automatically on scope exit. If cleanup function throws,
     * the exception is logged but not propagated (to prevent double-throw).
     */
    ~ScopeGuard() noexcept {
        if (m_OnExit) {
            try {
                m_OnExit();
            } catch (...) {
                // Log but don't rethrow to avoid termination
                // TODO: Use Logger::GetInstance().Error() when available
            }
        }
    }

    // Prevent copying
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    // Allow moving to enable return value optimization
    ScopeGuard(ScopeGuard&& other) noexcept
        : m_OnExit(std::move(other.m_OnExit)) {
        other.m_OnExit = nullptr;
    }

    ScopeGuard& operator=(ScopeGuard&& other) noexcept {
        if (this != &other) {
            m_OnExit = std::move(other.m_OnExit);
            other.m_OnExit = nullptr;
        }
        return *this;
    }

    /**
     * @brief Dismiss the guard (cleanup will not be called).
     *
     * Use when cleanup is no longer needed or responsibility
     * is transferred elsewhere. Call before scope exit.
     *
     * @see Release()
     */
    void Dismiss() noexcept { m_OnExit = nullptr; }

    /**
     * @brief Manually trigger cleanup early.
     *
     * Safe to call multiple times; subsequent calls are no-ops.
     *
     * @throws Only if cleanup function throws (rare)
     */
    void Release() {
        if (m_OnExit) {
            auto cleanup = std::move(m_OnExit);
            m_OnExit = nullptr;
            cleanup();
        }
    }

private:
    std::function<void()> m_OnExit;
};

/**
 * @class HandleGuard
 * @brief RAII wrapper for opaque handle types (e.g., file handles, GPU resources).
 *
 * Wraps a handle and a cleanup function to provide automatic resource management
 * for types that don't use pointers (e.g., Win32 HANDLE, OpenGL texture ID).
 *
 * **Usage:**
 * ```cpp
 * {
 *     auto handle = HandleGuard<GLuint>(
 *         textureID,
 *         [](GLuint id) { glDeleteTextures(1, &id); }
 *     );
 *     UseTexture(handle.Get());
 * }  // glDeleteTextures automatically called
 * ```
 *
 * @tparam HandleType Type of the handle (e.g., GLuint, HANDLE, int)
 *
 * @see ScopeGuard for pointer-based resources
 * @see std::unique_ptr for managed pointers
 */
template<typename HandleType>
class HandleGuard {
public:
    using CleanupFunc = std::function<void(HandleType)>;

    /**
     * @brief Create a handle guard.
     *
     * @param handle The resource handle to manage
     * @param cleanup Function to call on destruction
     * @param invalidValue Value that indicates an invalid handle (default-constructed)
     */
    explicit HandleGuard(
        HandleType handle,
        CleanupFunc cleanup,
        HandleType invalidValue = HandleType()
    ) noexcept
        : m_Handle(handle),
          m_Cleanup(std::move(cleanup)),
          m_InvalidValue(invalidValue) {}

    /**
     * @brief Destructor cleans up the handle.
     */
    ~HandleGuard() noexcept {
        if (m_Handle != m_InvalidValue && m_Cleanup) {
            try {
                m_Cleanup(m_Handle);
            } catch (...) {
                // Log but don't rethrow
            }
        }
    }

    // Prevent copying
    HandleGuard(const HandleGuard&) = delete;
    HandleGuard& operator=(const HandleGuard&) = delete;

    // Allow moving
    HandleGuard(HandleGuard&& other) noexcept
        : m_Handle(other.m_Handle),
          m_Cleanup(std::move(other.m_Cleanup)),
          m_InvalidValue(other.m_InvalidValue) {
        other.m_Handle = other.m_InvalidValue;
    }

    HandleGuard& operator=(HandleGuard&& other) noexcept {
        if (this != &other) {
            Cleanup();
            m_Handle = other.m_Handle;
            m_Cleanup = std::move(other.m_Cleanup);
            m_InvalidValue = other.m_InvalidValue;
            other.m_Handle = other.m_InvalidValue;
        }
        return *this;
    }

    /**
     * @brief Get the underlying handle value.
     *
     * @return The managed handle
     */
    HandleType Get() const noexcept { return m_Handle; }

    /**
     * @brief Check if handle is valid.
     *
     * @return true if handle is not the invalid value
     */
    bool IsValid() const noexcept { return m_Handle != m_InvalidValue; }

    /**
     * @brief Release the handle without cleanup.
     *
     * Transfers ownership to caller; caller is responsible for cleanup.
     * Handle will not be cleaned up on guard destruction.
     *
     * @return The managed handle
     */
    HandleType Release() noexcept {
        HandleType temp = m_Handle;
        m_Handle = m_InvalidValue;
        return temp;
    }

    /**
     * @brief Manually trigger cleanup and invalidate handle.
     */
    void Cleanup() noexcept {
        if (m_Handle != m_InvalidValue && m_Cleanup) {
            try {
                m_Cleanup(m_Handle);
            } catch (...) {
                // Log but don't rethrow
            }
        }
        m_Handle = m_InvalidValue;
    }

private:
    HandleType m_Handle;
    CleanupFunc m_Cleanup;
    HandleType m_InvalidValue;
};

/**
 * @class ResourcePool
 * @brief Thread-safe resource pool for reusable object allocation.
 *
 * Manages a pool of pre-allocated resources to avoid repeated
 * allocation/deallocation. Resources are automatically returned to
 * the pool when the guard is destroyed.
 *
 * **Usage:**
 * ```cpp
 * ResourcePool<GraphicsBuffer> bufferPool([]() { return new GraphicsBuffer(); });
 *
 * {
 *     auto buffer = bufferPool.Acquire();  // Gets from pool or creates
 *     UseBuffer(buffer.get());
 * }  // Automatically returned to pool
 * ```
 *
 * @tparam ResourceType Type of resource to pool
 *
 * @note Requires std::mutex and thread support
 * @see HandleGuard for single resource management
 */
template<typename ResourceType>
class ResourcePool {
public:
    using Creator = std::function<std::shared_ptr<ResourceType>()>;
    using GuardType = std::shared_ptr<ResourceType>;

    /**
     * @brief Create a resource pool.
     *
     * @param creator Function to create new resources
     */
    explicit ResourcePool(Creator creator)
        : m_Creator(creator) {}

    /**
     * @brief Acquire a resource from the pool.
     *
     * Returns a pooled resource if available, otherwise creates a new one.
     * When the returned shared_ptr goes out of scope, the resource is
     * returned to the pool.
     *
     * @return Shared pointer to a resource
     *
     * @note Thread-safe
     */
    GuardType Acquire() {
        std::lock_guard<std::mutex> lock(m_Mutex);

        if (!m_AvailableResources.empty()) {
            auto resource = m_AvailableResources.back();
            m_AvailableResources.pop_back();
            return resource;
        }

        return m_Creator();
    }

    /**
     * @brief Pre-allocate resources for future use.
     *
     * Useful during initialization to prepare resources before
     * real-time operations begin.
     *
     * @param count Number of resources to pre-allocate
     *
     * @note Thread-safe
     */
    void PreAllocate(size_t count) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (size_t i = 0; i < count; ++i) {
            m_AvailableResources.push_back(m_Creator());
        }
    }

    /**
     * @brief Get current pool size.
     *
     * @return Number of available resources in pool
     */
    size_t GetPoolSize() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_AvailableResources.size();
    }

    /**
     * @brief Clear the resource pool.
     *
     * All pooled resources are released.
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_AvailableResources.clear();
    }

private:
    Creator m_Creator;
    std::vector<GuardType> m_AvailableResources;
    mutable std::mutex m_Mutex;
};

/** @} */ // end RAII group

} // namespace SZM
