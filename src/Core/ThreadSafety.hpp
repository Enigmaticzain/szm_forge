#pragma once

/**
 * @file ThreadSafety.hpp
 * @brief Thread-safety documentation and patterns for SZM Forge.
 *
 * This file documents thread-safety guarantees across the codebase and
 * provides common patterns for thread-safe implementation.
 *
 * **Thread Safety Levels:**
 * - NOT thread-safe: Must only be used from single thread (no synchronization)
 * - Thread-safe: Safe to call concurrently; internal synchronization provided
 * - Internally synchronized: Uses locks internally; caller doesn't need synchronization
 * - Lock-free: Uses atomic operations; no locks required
 *
 * **Every public class should document its thread-safety level.**
 *
 * @see ThreadSafeWrapper for thread-safe wrapper template
 */

#include <mutex>
#include <shared_mutex>
#include <memory>

namespace SZM {

/**
 * @defgroup ThreadSafety Thread Safety Patterns & Documentation
 * @brief Thread-safe implementation utilities and guidelines.
 *
 * These utilities help implement thread-safe code and document
 * concurrency requirements clearly.
 * @{
 */

/**
 * @section thread_safety_levels Thread Safety Levels
 *
 * ### Level 0: NOT Thread-Safe
 * - Multiple threads → Undefined behavior / data corruption
 * - Use: Single-threaded components (e.g., scene graph per thread)
 * - Example: Input system, Frame-local temporary storage
 * @code
 * /// NOT thread-safe. Must only be used from main thread.
 * class SceneGraph { };
 * @endcode
 *
 * ### Level 1: Thread-Safe (Internally Synchronized)
 * - Multiple threads → Safe operation (no synchronization at call site)
 * - Uses: Mutex or atomic operations internally
 * - Example: Logger, Material database, Event bus
 * @code
 * /// Thread-safe singleton. All methods are internally synchronized.
 * /// Safe to call from any thread without external locks.
 * class Logger { };
 * @endcode
 *
 * ### Level 2: Lock-Free Thread-Safe
 * - Multiple threads → Safe operation, no contention
 * - Uses: Atomic operations, lock-free data structures
 * - Example: Simple atomic counters, queues for high-frequency ops
 * @code
 * /// Lock-free thread-safe counter using std::atomic.
 * /// No mutex or spinning required.
 * class AtomicCounter { };
 * @endcode
 *
 * ### Level 3: Thread-Compatible (Caller Synchronized)
 * - Multiple threads → SAFE IF caller provides external synchronization
 * - Uses: No internal synchronization; caller responsible
 * - Example: Components passed between threads with external lock held
 * @code
 * /// Thread-compatible. Safe if all access is protected by external mutex.
 * /// Example: Component accessed only inside locked critical section.
 * @endcode
 */

/**
 * @class ThreadSafeWrapper
 * @brief Template for making non-thread-safe types thread-safe.
 *
 * Wraps an object with mutex protection, providing scoped access patterns
 * that ensure proper lock acquisition/release.
 *
 * **Usage:**
 * ```cpp
 * // Wrap a non-thread-safe container
 * ThreadSafeWrapper<std::vector<Entity>> entities;
 *
 * // Thread-safe access with auto-release
 * {
 *     auto guard = entities.Lock();
 *     guard->push_back(newEntity);
 *     // Lock automatically released on scope exit
 * }
 * ```
 *
 * @tparam T The type to wrap (typically non-thread-safe)
 *
 * **Thread Safety:** Entire wrapped object is protected by mutex.
 * Each Lock() call acquires exclusive ownership.
 *
 * @see EventBus for a production example
 */
template<typename T>
class ThreadSafeWrapper {
public:
    /**
     * @class Guard
     * @brief RAII lock guard for thread-safe access.
     *
     * Acquires mutex on construction, releases on destruction.
     * Provides transparent access to wrapped object.
     */
    class Guard {
    public:
        /// @brief Get mutable access to wrapped object
        T* operator->() { return m_Ptr; }

        /// @brief Get const access to wrapped object
        const T* operator->() const { return m_Ptr; }

        /// @brief Dereference to mutable object
        T& operator*() { return *m_Ptr; }

        /// @brief Dereference to const object
        const T& operator*() const { return *m_Ptr; }

    private:
        friend class ThreadSafeWrapper;

        Guard(T* ptr, std::unique_lock<std::mutex>&& lock)
            : m_Ptr(ptr), m_Lock(std::move(lock)) {}

        T* m_Ptr;
        std::unique_lock<std::mutex> m_Lock;
    };

    /**
     * @brief Create thread-safe wrapper around an object.
     *
     * Owns the object; it's copied or moved during construction.
     *
     * @param obj Object to wrap
     */
    explicit ThreadSafeWrapper(T obj = T())
        : m_Object(std::move(obj)) {}

    /**
     * @brief Acquire exclusive lock and access object.
     *
     * Returns a Guard that holds the lock and provides access.
     * Lock is released when Guard goes out of scope.
     *
     * **IMPORTANT:** Must be used in scoped context:
     * ```cpp
     * {
     *     auto guard = wrapper.Lock();
     *     use(*guard);
     * }  // Lock released here
     * ```
     *
     * @return Guard that holds the lock
     *
     * @warning Holding multiple locks risks deadlock; use with caution
     */
    Guard Lock() {
        return Guard(
            &m_Object,
            std::unique_lock<std::mutex>(m_Mutex)
        );
    }

    /**
     * @brief Const version - acquire read-only access.
     *
     * @return Const Guard with lock held
     */
    Guard LockConst() const {
        return Guard(
            const_cast<T*>(&m_Object),
            std::unique_lock<std::mutex>(m_Mutex)
        );
    }

private:
    T m_Object;
    mutable std::mutex m_Mutex;
};

/**
 * @class ReadWriteWrapper
 * @brief Thread-safe wrapper with shared read locks and exclusive write locks.
 *
 * Allows multiple concurrent readers but exclusive writer access.
 * More efficient than mutex when reads greatly outnumber writes.
 *
 * **Usage:**
 * ```cpp
 * ReadWriteWrapper<Database> db;
 *
 * // Multiple threads can read concurrently
 * {
 *     auto reader = db.LockRead();
 *     auto value = reader->Query(...);
 * }
 *
 * // Only one thread can write at a time
 * {
 *     auto writer = db.LockWrite();
 *     writer->Update(...);
 * }
 * ```
 *
 * @tparam T The type to wrap
 *
 * **Performance:** Better than mutex for read-heavy workloads.
 *
 * @see ThreadSafeWrapper for simpler exclusive-lock version
 */
template<typename T>
class ReadWriteWrapper {
public:
    class ReadGuard {
    public:
        const T* operator->() const { return m_Ptr; }
        const T& operator*() const { return *m_Ptr; }

    private:
        friend class ReadWriteWrapper;
        ReadGuard(const T* ptr, std::shared_lock<std::shared_mutex>&& lock)
            : m_Ptr(ptr), m_Lock(std::move(lock)) {}

        const T* m_Ptr;
        std::shared_lock<std::shared_mutex> m_Lock;
    };

    class WriteGuard {
    public:
        T* operator->() { return m_Ptr; }
        const T* operator->() const { return m_Ptr; }
        T& operator*() { return *m_Ptr; }
        const T& operator*() const { return *m_Ptr; }

    private:
        friend class ReadWriteWrapper;
        WriteGuard(T* ptr, std::unique_lock<std::shared_mutex>&& lock)
            : m_Ptr(ptr), m_Lock(std::move(lock)) {}

        T* m_Ptr;
        std::unique_lock<std::shared_mutex> m_Lock;
    };

    explicit ReadWriteWrapper(T obj = T())
        : m_Object(std::move(obj)) {}

    /// Acquire shared read lock (multiple readers allowed)
    ReadGuard LockRead() const {
        return ReadGuard(
            &m_Object,
            std::shared_lock<std::shared_mutex>(m_Mutex)
        );
    }

    /// Acquire exclusive write lock (single writer, blocks readers)
    WriteGuard LockWrite() {
        return WriteGuard(
            &m_Object,
            std::unique_lock<std::shared_mutex>(m_Mutex)
        );
    }

private:
    T m_Object;
    mutable std::shared_mutex m_Mutex;
};

/**
 * @section thread_safety_checklist Thread Safety Checklist
 *
 * When implementing thread-safe code, ensure:
 *
 * 1. **Document Thread Safety Level**
 *    - Add comment above class: /// Thread-safe: Internally synchronized
 *    - Specify if NOT thread-safe: /// NOT thread-safe. Single-threaded only.
 *
 * 2. **Use Appropriate Synchronization**
 *    - Shared data → mutex or atomic
 *    - Multiple readers, few writers → std::shared_mutex
 *    - Frequent concurrent access → Consider lock-free structures
 *
 * 3. **Use RAII Locks**
 *    - Never manual lock/unlock
 *    - Use std::lock_guard or std::scoped_lock
 *    - Never hold locks across blocking operations
 *
 * 4. **Prevent Deadlock**
 *    - Always acquire locks in consistent order
 *    - Never hold multiple locks unless necessary
 *    - Use std::scoped_lock for multiple locks
 *
 * 5. **Data Race Prevention**
 *    - All shared data must be protected
 *    - Use thread sanitizer: -fsanitize=thread (GCC/Clang)
 *    - Use static analysis: clang-analyzer, cppcheck
 *
 * 6. **Test Concurrency**
 *    - Test with ThreadSanitizer enabled
 *    - Stress test with multiple threads
 *    - Use tools like Helgrind (Valgrind)
 *
 * **Example Thread-Safe Class:**
 * @code
 * /// Thread-safe message queue. All operations are internally synchronized.
 * class MessageQueue {
 * public:
 *     /// Thread-safe. Safe to call from any thread.
 *     void Enqueue(const Message& msg) {
 *         std::lock_guard<std::mutex> lock(m_Mutex);
 *         m_Queue.push(msg);
 *     }
 *
 *     /// Thread-safe. Returns nullopt if queue is empty.
 *     [[nodiscard]] std::optional<Message> TryDequeue() {
 *         std::lock_guard<std::mutex> lock(m_Mutex);
 *         if (m_Queue.empty()) return std::nullopt;
 *         auto msg = m_Queue.front();
 *         m_Queue.pop();
 *         return msg;
 *     }
 *
 * private:
 *     std::queue<Message> m_Queue;
 *     mutable std::mutex m_Mutex;
 * };
 * @endcode
 */

/** @} */ // end ThreadSafety group

} // namespace SZM

/**
 * @section existing_thread_safety Existing Thread Safety Status
 *
 * ### Thread-Safe (Internally Synchronized):
 * - Logger: Uses mutex for console/file output
 * - EventBus: Uses mutex for listener map
 * - MaterialDatabase: Uses std::shared_mutex for read-heavy lookups
 * - ResourcePool: Uses mutex for resource management
 *
 * ### NOT Thread-Safe (Single-Threaded):
 * - PhysicsEngine: Designed for physics thread only
 * - SimulationEngine: Frame-local, single-threaded
 * - InputManager: Platform-specific, typically single-threaded
 * - Application: Main thread only
 * - SceneGraph: Scene-local, typically single-threaded
 *
 * ### Notes:
 * - UI systems (ImGui) are typically NOT thread-safe; render from single thread
 * - Physics updates must be called from consistent thread
 * - Consider thread pools for parallel physics/simulation if needed
 */
