#pragma once

#include <string>
#include <memory>

namespace SZM::WebViewer {

    /**
     * @brief Manages the React UI build and serving
     */
    class ReactUIManager {
    public:
        static ReactUIManager& GetInstance();

        /**
         * @brief Initialize React UI manager
         * @param uiPath Path to React app source
         * @return true if successful
         */
        bool Initialize(const std::string& uiPath);

        /**
         * @brief Build React UI for production
         * @return true if build successful
         */
        bool Build();

        /**
         * @brief Start development server
         * @return true if server started
         */
        bool StartDevServer();

        /**
         * @brief Stop development server
         */
        void StopDevServer();

        /**
         * @brief Get path to built UI
         */
        std::string GetBuildPath() const { return m_BuildPath; }

        /**
         * @brief Get React UI URL
         */
        std::string GetUIURL() const { return m_UIURL; }

        /**
         * @brief Check if React UI is available
         */
        bool IsAvailable() const { return m_Available; }

    private:
        ReactUIManager() = default;
        ~ReactUIManager() = default;

        std::string m_UIPath;
        std::string m_BuildPath = "dist";
        std::string m_UIURL = "http://localhost:5173";
        bool m_Available = false;
        bool m_DevServerRunning = false;
    };

} // namespace SZM::WebViewer
