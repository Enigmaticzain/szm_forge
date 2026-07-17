#include "Logger.hpp"
#include <iostream>
#include <filesystem>

namespace SZM {

std::string Logger::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::GetLevelString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARN";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT";
        default:                 return "UNKNOWN";
    }
}

std::string Logger::GetColorCode(LogLevel level) const {
    if (!m_UseColorOutput) return "";
    
    // ANSI color codes
    switch (level) {
        case LogLevel::Debug:    return "\033[36m";  // Cyan
        case LogLevel::Info:     return "\033[32m";  // Green
        case LogLevel::Warning:  return "\033[33m";  // Yellow
        case LogLevel::Error:    return "\033[31m";  // Red
        case LogLevel::Critical: return "\033[1;31m"; // Bold Red
        default:                 return "\033[0m";   // Reset
    }
}

void Logger::Log(
    LogLevel level,
    const std::string& message,
    const std::string& module,
    const std::source_location& location)
{
    // Filter by minimum level
    if (level < m_MinimumLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);

    // Format the log message
    std::ostringstream oss;
    oss << GetTimestamp() << " [" << GetLevelString(level) << "] ["
        << module << "] " << message << " (" << location.file_name()
        << ":" << location.line() << " in " << location.function_name() << ")";

    std::string formatted_msg = oss.str();

    // Console output
    if (m_UseConsoleOutput) {
        std::string color = GetColorCode(level);
        std::string reset = m_UseColorOutput ? "\033[0m" : "";
        std::cout << color << formatted_msg << reset << "\n";
    }

    // File output
    if (m_UseFileOutput) {
        if (!m_LogFile || !m_LogFile->is_open()) {
            // Lazy initialization of log file
            std::filesystem::path log_path(m_LogFilePath);
            std::filesystem::create_directories(log_path.parent_path());
            m_LogFile = std::make_unique<std::ofstream>(
                m_LogFilePath, std::ios::app);
            if (!m_LogFile->is_open()) {
                std::cerr << "[Logger] Failed to open log file: " << m_LogFilePath << "\n";
                m_UseFileOutput = false;
                return;
            }
        }

        *m_LogFile << formatted_msg << "\n";
        m_LogFile->flush();
    }
}

} // namespace SZM
