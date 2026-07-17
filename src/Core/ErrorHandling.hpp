#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <source_location>

namespace SZM {

/**
 * @defgroup ErrorHandling Error Handling & Diagnostics
 * @brief Centralized error reporting, logging, and diagnostics utilities.
 *
 * This module provides industry-standard error handling patterns:
 * - Structured exceptions with context and source location
 * - Debug assertions with custom messages
 * - Error codes for recoverable errors
 * - Logging framework integration
 * @{
 */

/**
 * @enum ErrorCode
 * @brief Standard error classification codes.
 *
 * Used for categorizing errors across all modules. Enables consistent
 * error handling and reporting.
 */
enum class ErrorCode {
    Success = 0,              ///< Operation completed successfully
    Unknown = -1,             ///< Unknown or unclassified error
    InvalidArgument = 100,    ///< Argument validation failed
    InvalidState = 101,       ///< Operation invalid for current state
    OutOfMemory = 102,        ///< Memory allocation failed
    NotImplemented = 103,     ///< Feature not yet implemented
    FileNotFound = 104,       ///< File or resource not found
    PermissionDenied = 105,   ///< Insufficient permissions
    TimeoutExpired = 106,     ///< Operation timed out
    ResourceExhausted = 107,  ///< Resource limit exceeded
    DataCorrupted = 108,      ///< Data integrity check failed
    PreconditionFailed = 109, ///< Precondition violated
};

/**
 * @struct ErrorContext
 * @brief Rich error context information for diagnostics.
 *
 * Contains location, category, and optional additional details.
 * Captured automatically with modern compiler support.
 */
struct ErrorContext {
    ErrorCode code;                                      ///< Error category
    std::string message;                                 ///< Human-readable message
    std::string module;                                  ///< Source module name
    std::source_location location;                       ///< Source code location
    
    /** @brief Format context as readable string */
    std::string ToString() const {
        std::ostringstream oss;
        oss << "[" << module << "] " << message << "\n"
            << "    at " << location.file_name() << ":" 
            << location.line() << ":" << location.column()
            << " in " << location.function_name();
        return oss.str();
    }
};

/**
 * @class RuntimeException
 * @brief Rich exception class with context information.
 *
 * Provides structured exception handling with:
 * - Error codes for categorization
 * - Source location information
 * - Module context
 * - Nested exception support
 *
 * **Usage:**
 * ```cpp
 * if (value < 0) {
 *     throw RuntimeException(
 *         ErrorCode::InvalidArgument,
 *         "Value must be non-negative",
 *         "Physics"
 *     );
 * }
 * ```
 */
class RuntimeException : public std::runtime_error {
public:
    /**
     * @brief Construct exception with rich context.
     *
     * @param code Error classification code
     * @param message Human-readable error description
     * @param module Source module name (e.g., "Physics", "Graphics")
     * @param location Source code location (captured automatically)
     */
    RuntimeException(
        ErrorCode code,
        const std::string& message,
        const std::string& module = "Unknown",
        const std::source_location& location = std::source_location::current()
    ) : std::runtime_error(message), m_Context{code, message, module, location} {}

    /**
     * @brief Get error context.
     *
     * @return Const reference to error context
     */
    const ErrorContext& GetContext() const noexcept { return m_Context; }

    /**
     * @brief Get error code.
     *
     * @return Error code enumeration
     */
    ErrorCode GetCode() const noexcept { return m_Context.code; }

    /**
     * @brief Get formatted diagnostic string.
     *
     * @return Multi-line formatted error information
     */
    std::string GetDiagnostics() const noexcept {
        return m_Context.ToString();
    }

private:
    ErrorContext m_Context;
};

/**
 * @class LogicException
 * @brief Exception for logic errors and violated contracts.
 *
 * Used for precondition/postcondition violations and invariant failures.
 * Typically indicates a programming error rather than environmental issue.
 */
class LogicException : public std::logic_error {
public:
    /**
     * @brief Construct logic exception with context.
     *
     * @param condition The condition that was violated
     * @param message Explanation of the violation
     * @param module Source module name
     * @param location Source code location
     */
    LogicException(
        const std::string& condition,
        const std::string& message,
        const std::string& module = "Unknown",
        const std::source_location& location = std::source_location::current()
    ) : std::logic_error(message),
        m_Condition(condition),
        m_Context{ErrorCode::PreconditionFailed, message, module, location} {}

    /**
     * @brief Get the violated condition expression.
     *
     * @return Condition string
     */
    const std::string& GetCondition() const noexcept { return m_Condition; }

    /**
     * @brief Get error context.
     *
     * @return Const reference to error context
     */
    const ErrorContext& GetContext() const noexcept { return m_Context; }

private:
    std::string m_Condition;
    ErrorContext m_Context;
};

/**
 * @def SZM_ASSERT
 * @brief Debug-time assertion with message.
 *
 * Enabled only in debug builds. Aborts if condition is false.
 * In release builds, expands to nothing.
 *
 * **Usage:**
 * ```cpp
 * SZM_ASSERT(ptr != nullptr, "Pointer cannot be null");
 * SZM_ASSERT(density > 0, "Density must be positive");
 * ```
 *
 * @param condition Condition to check (true = pass)
 * @param message Error message if condition fails
 */
#ifdef NDEBUG
    #define SZM_ASSERT(condition, message) ((void)0)
#else
    #define SZM_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                throw LogicException(#condition, message, __func__); \
            } \
        } while (false)
#endif

/**
 * @def SZM_CHECK
 * @brief Runtime check that always executes.
 *
 * Unlike SZM_ASSERT, this check always runs in both debug and release builds.
 * Use for critical runtime invariants and preconditions.
 *
 * **Usage:**
 * ```cpp
 * SZM_CHECK(meshSize > 0, "Mesh cannot be empty", "Physics");
 * SZM_CHECK(density > 0.0f, "Density must be positive", "Materials");
 * ```
 *
 * @param condition Condition to check (true = pass)
 * @param message Error message if condition fails
 * @param module Module name for context
 * @throws RuntimeException if condition fails
 */
#define SZM_CHECK(condition, message, module) \
    do { \
        if (!(condition)) { \
            throw RuntimeException(ErrorCode::PreconditionFailed, message, module); \
        } \
    } while (false)

/**
 * @def SZM_THROW
 * @brief Throw a RuntimeException with context.
 *
 * Convenience macro for throwing exceptions with automatic location capture.
 *
 * **Usage:**
 * ```cpp
 * SZM_THROW(ErrorCode::FileNotFound, "Could not open config.json", "Config");
 * SZM_THROW(ErrorCode::InvalidArgument, "Value out of range", "Math");
 * ```
 *
 * @param code Error code enumeration
 * @param message Human-readable message
 * @param module Module name for context
 */
#define SZM_THROW(code, message, module) \
    throw RuntimeException(code, message, module)

/** @} */ // end ErrorHandling group

} // namespace SZM
