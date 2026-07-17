#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace SZM::Electrical {

    /**
     * @brief PlatformIO integration for embedded systems
     * Enables:
     * - Firmware compilation and deployment
     * - Hardware-in-the-loop testing
     * - Serial monitoring
     * - Library management
     */
    class PlatformIOBridge {
    public:
        struct BoardConfig {
            std::string boardType;      // "esp32", "arduino", etc
            std::string port;           // "/dev/ttyUSB0"
            uint32_t baudRate = 115200;
            std::string framework = "arduino";
        };

        PlatformIOBridge(const BoardConfig& config = {});
        ~PlatformIOBridge();

        /**
         * @brief Initialize board connection
         */
        bool Initialize();

        /**
         * @brief Build firmware
         */
        bool BuildFirmware(const std::string& projectPath);

        /**
         * @brief Upload firmware to device
         */
        bool UploadFirmware(const std::string& firmwarePath);

        /**
         * @brief Start serial monitoring
         */
        bool StartMonitoring(std::function<void(const std::string&)> callback);

        /**
         * @brief Stop serial monitoring
         */
        void StopMonitoring();

        /**
         * @brief Send command to device
         */
        bool SendCommand(const std::string& command);

        /**
         * @brief Get device info
         */
        std::string GetDeviceInfo();

        /**
         * @brief List available boards
         */
        static std::vector<std::string> GetAvailableBoards();

        /**
         * @brief List available ports
         */
        static std::vector<std::string> GetAvailablePorts();

    private:
        BoardConfig m_Config;
        bool m_IsConnected = false;
        void* m_MonitorHandle = nullptr;
    };

}
