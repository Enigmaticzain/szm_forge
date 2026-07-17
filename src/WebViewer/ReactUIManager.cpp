#include "ReactUIManager.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <chrono>

namespace SZM::WebViewer {

bool ReactUIManager::Initialize(const std::string& uiPath) {
    try {
        m_UIPath = uiPath;
        
        // Check if React app exists
        std::string packageJsonPath = m_UIPath + "/package.json";
        if (!std::filesystem::exists(packageJsonPath)) {
            std::cerr << "[ReactUIManager] React app not found at " << uiPath << "\n";
            return false;
        }

        // Check if node_modules exists, if not run npm install
        std::string nodeModulesPath = m_UIPath + "/node_modules";
        if (!std::filesystem::exists(nodeModulesPath)) {
            std::cout << "[ReactUIManager] Installing dependencies...\n";
            
            // Run npm install
            std::string installCmd = "cd \"" + m_UIPath + "\" && npm install";
            int installResult = system(installCmd.c_str());
            
            if (installResult != 0) {
                std::cerr << "[ReactUIManager] Failed to install dependencies\n";
                return false;
            }
        }

        m_BuildPath = m_UIPath + "/dist";
        m_Available = true;
        
        std::cout << "[ReactUIManager] Initialized successfully\n";
        std::cout << "[ReactUIManager] UI path: " << m_UIPath << "\n";
        std::cout << "[ReactUIManager] Build path: " << m_BuildPath << "\n";
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[ReactUIManager] Initialization failed: " << e.what() << "\n";
        return false;
    }
}

bool ReactUIManager::Build() {
    if (!m_Available) {
        std::cerr << "[ReactUIManager] Not initialized\n";
        return false;
    }

    try {
        std::string indexHtmlPath = m_BuildPath + "/index.html";
        if (std::filesystem::exists(indexHtmlPath)) {
            std::cout << "[ReactUIManager] Build already exists. Skipping build step for faster startup.\n";
            return true;
        }

        std::cout << "[ReactUIManager] Building React UI...\n";
        
        // Run npm run build
        std::string buildCmd = "cd \"" + m_UIPath + "\" && npm run build";
        int buildResult = system(buildCmd.c_str());
        
        if (buildResult != 0) {
            std::cerr << "[ReactUIManager] Build failed\n";
            return false;
        }

        // Verify build output
        if (!std::filesystem::exists(indexHtmlPath)) {
            std::cerr << "[ReactUIManager] Build output not found\n";
            return false;
        }

        std::cout << "[ReactUIManager] Build successful\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[ReactUIManager] Build failed: " << e.what() << "\n";
        return false;
    }
}

bool ReactUIManager::StartDevServer() {
    if (!m_Available) {
        std::cerr << "[ReactUIManager] Not initialized\n";
        return false;
    }

    if (m_DevServerRunning) {
        std::cout << "[ReactUIManager] Dev server already running\n";
        return true;
    }

    try {
        std::cout << "[ReactUIManager] Starting dev server...\n";
        
        // Fork and run npm run dev in background
        pid_t pid = fork();
        
        if (pid == 0) {
            // Child process
            std::string devCmd = "cd \"" + m_UIPath + "\" && npm run dev";
            execl("/bin/bash", "bash", "-c", devCmd.c_str(), nullptr);
            exit(1);
        } else if (pid > 0) {
            std::cout << "[ReactUIManager] Waiting for Vite dev server at " << m_UIURL << "...\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
            m_DevServerRunning = true;
            std::cout << "[ReactUIManager] Dev server started on " << m_UIURL << "\n";
            return true;
        } else {
            std::cerr << "[ReactUIManager] Failed to fork process\n";
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ReactUIManager] Failed to start dev server: " << e.what() << "\n";
        return false;
    }
}

void ReactUIManager::StopDevServer() {
    if (!m_DevServerRunning) {
        return;
    }

    try {
        // Kill the dev server process
        system("pkill -f 'vite' || true");
        m_DevServerRunning = false;
        std::cout << "[ReactUIManager] Dev server stopped\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[ReactUIManager] Error stopping dev server: " << e.what() << "\n";
    }
}

ReactUIManager& ReactUIManager::GetInstance() {
    static ReactUIManager instance;
    return instance;
}

} // namespace SZM::WebViewer
