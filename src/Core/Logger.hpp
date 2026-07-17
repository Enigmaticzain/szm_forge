#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <memory>
#include <source_location>

namespace SZM {

/**
 * @defgroup Logging Logger & Diagnostics
 * @brief Thread-safe centralized logging system with multiple severity levels.
 *
 * Supports console and file output with automatic timestamps, source location,
 * and severity classification. Compatible with IDE debuggers and log analysis tools.
 * @{
 */

/**
 * @enum LogLevel
 * @brief Severity classification for logged messages.
 *
 * Used to filter and categorize messages. Typical production deployments
 * use INFO level; development uses DEBUG; shipped binaries use WARNING+.
 */
enum class LogLevel {
    Debug = 0,      ///< Detailed diagnostic information (disabled in release builds)
    Info = 1,       ///< General informational messages (normal operation)
    Warning = 2,    ///< Warning conditions requiring attention (but operation continues)
    Error = 3,      ///< Recoverable error conditions (feature may not work correctly)
    Critical = 4,   ///< Unrecoverable critical failure (immediate action needed)
};

/**
 * @class Logger
 * @brief Thread-safe centralized logging system.
 *
 * Singleton that manages all logging output across the application. Features:
 * - Multiple severity levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
 * - Automatic timestamps and source location (file:line:function)
 * - Thread-safe output via mutex
 * - Dual output (console + file)
 * - Configurable minimum log level
 * - ANSI color codes for terminal (auto-detected)
 *
 * **Thread Safety:** All methods are thread-safe via std::lock_guard.
 *
 * **Usage:**
 * ```cpp
 * auto& logger = Logger::GetInstance();
 * logger.SetLogFile("simulation.log");
 * logger.SetMinimumLevel(LogLevel::Info);
 *
 * logger.Info("Physics engine initialized");
 * logger.Warn("Gravity disabled for space simulation");
 * logger.Error("Failed to load mesh: file not found");
 * ```
 *
 * @see LogLevel for severity classification
 */
class Logger {
public:
    /**
     * @brief Get the singleton logger instance.
     *
     * @return Reference to global Logger
     */
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Set the minimum log level.
     *
     * Messages with lower severity are filtered out.
     * Default: LogLevel::Info (development mode)
     *
     * @param level Minimum severity to log
     *
     * **Typical settings:**
     * - DEBUG mode: LogLevel::Debug
     * - RELEASE mode: LogLevel::Warning
     * - SHIPPING: LogLevel::Error
     *
     * @see LogLevel
     */
    void SetMinimumLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_MinimumLevel = level;
    }

    /**
     * @brief Get current minimum log level.
     *
     * @return Currently configured minimum level
     */
    LogLevel GetMinimumLevel() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_MinimumLevel;
    }

    /**
     * @brief Enable or disable file output.
     *
     * If enabled, all messages are also written to the log file.
     * File is created/appended on first log message.
     *
     * @param filepath Path to log file (e.g., "logs/app.log")
     *
     * @throws std::ios_base::failure if file cannot be opened
     */
    void SetLogFile(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_LogFilePath = filepath;
        m_UseFileOutput = true;
    }

    /**
     * @brief Disable file logging.
     */
    void DisableFileOutput() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_UseFileOutput = false;
        if (m_LogFile && m_LogFile->is_open()) {
            m_LogFile->close();
        }
    }

    /**
     * @brief Enable/disable console output.
     *
     * @param enabled true for console output, false to suppress
     * Default: true (enabled)
     */
    void SetConsoleOutput(bool enabled) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_UseConsoleOutput = enabled;
    }

    /**
     * @brief Enable/disable ANSI color codes in console output.
     *
     * Color codes are auto-disabled on non-terminal outputs.
     * Manual override with this method if needed.
     *
     * @param enabled true to enable ANSI colors
     */
    void SetColorOutput(bool enabled) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_UseColorOutput = enabled;
    }

    // ---- Logging Methods (Thread-Safe) ----

    /**
     * @brief Log a debug-level message (development diagnostics).
     *
     * @param message Log message (auto-formatted with timestamp/location)
     * @param module Module/subsystem name (e.g., "Physics", "Graphics")
     * @param location Source location (captured automatically)
     */
    void Debug(
        const std::string& message,
        const std::string& module = "Core",
        const std::source_location& location = std::source_location::current()
    ) {
        Log(LogLevel::Debug, message, module, location);
    }

    /**
     * @brief Log an info-level message (normal operation events).
     *
     * @param message Log message
     * @param module Module/subsystem name
     * @param location Source location (captured automatically)
     */
    void Info(
        const std::string& message,
        const std::string& module = "Core",
        const std::source_location& location = std::source_location::current()
    ) {
        Log(LogLevel::Info, message, module, location);
    }

    /**
     * @brief Log a warning-level message (attention required but operation continues).
     *
     * @param message Log message
     * @param module Module/subsystem name
     * @param location Source location (captured automatically)
     */
    void Warn(
        const std::string& message,
        const std::string& module = "Core",
        const std::source_location& location = std::source_location::current()
    ) {
        Log(LogLevel::Warning, message, module, location);
    }

    /**
     * @brief Log an error-level message (recoverable error).
     *
     * @param message Log message
     * @param module Module/subsystem name
     * @param location Source location (captured automatically)
     */
    void Error(
        const std::string& message,
        const std::string& module = "Core",
        const std::source_location& location = std::source_location::current()
    ) {
        Log(LogLevel::Error, message, module, location);
    }

    /**
     * @brief Log a critical-level message (unrecoverable failure).
     *
     * @param message Log message
     * @param module Module/subsystem name
     * @param location Source location (captured automatically)
     */
    void Critical(
        const std::string& message,
        const std::string& module = "Core",
        const std::source_location& location = std::source_location::current()
    ) {
        Log(LogLevel::Critical, message, module, location);
    }

    /**
     * @brief Generic logging method (internal use; prefer typed methods above).
     *
     * @param level Message severity level
     * @param message Log message
     * @param module Module/subsystem name
     * @param location Source code location
     *
     * @see Debug(), Info(), Warn(), Error(), Critical()
     */
    void Log(
        LogLevel level,
        const std::string& message,
        const std::string& module = "Core",
        const std::source_location& location = std::source_location::current()
    );

private:
    Logger() = default;
    ~Logger() {
        if (m_LogFile && m_LogFile->is_open()) {
            m_LogFile->close();
        }
    }

    std::string GetLevelString(LogLevel level) const;
    std::string GetColorCode(LogLevel level) const;
    std::string GetTimestamp() const;

    mutable std::mutex m_Mutex;
    LogLevel m_MinimumLevel = LogLevel::Info;
    bool m_UseConsoleOutput = true;
    bool m_UseFileOutput = false;
    bool m_UseColorOutput = true;
    std::string m_LogFilePath;
    std::unique_ptr<std::ofstream> m_LogFile;
};

/**
 * @def SZM_LOG_DEBUG
 * @brief Macro for debug-level logging with automatic location.
 *
 * ```cpp
 * SZM_LOG_DEBUG("Component created", "Physics");
 * ```
 */
#define SZM_LOG_DEBUG(msg, module) \
    SZM::Logger::GetInstance().Debug((msg), (module))

/**
 * @def SZM_LOG_INFO
 * @brief Macro for info-level logging with automatic location.
 */
#define SZM_LOG_INFO(msg, module) \
    SZM::Logger::GetInstance().Info((msg), (module))

/**
 * @def SZM_LOG_WARN
 * @brief Macro for warning-level logging with automatic location.
 */
#define SZM_LOG_WARN(msg, module) \
    SZM::Logger::GetInstance().Warn((msg), (module))

/**
 * @def SZM_LOG_ERROR
 * @brief Macro for error-level logging with automatic location.
 */
#define SZM_LOG_ERROR(msg, module) \
    SZM::Logger::GetInstance().Error((msg), (module))

/**
 * @def SZM_LOG_CRITICAL
 * @brief Macro for critical-level logging with automatic location.
 */
#define SZM_LOG_CRITICAL(msg, module) \
    SZM::Logger::GetInstance().Critical((msg), (module))

/** @} */ // end Logging group

} // namespace SZM
