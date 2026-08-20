#include "ExternalEngineManager.hpp"
#include <iostream>

namespace SZM::Core::Integration {

    ExternalEngineManager& ExternalEngineManager::GetInstance() {
        static ExternalEngineManager instance;
        return instance;
    }

    bool ExternalEngineManager::InitializeAll() {
        bool allSuccess = true;
        for (auto& [name, engine] : m_Engines) {
            std::cout << "[Integration] Initializing external engine: " << name << "\n";
            if (!engine->Initialize()) {
                std::cerr << "[Integration] Failed to initialize engine: " << name << "\n";
                allSuccess = false;
            }
        }
        return allSuccess;
    }

    void ExternalEngineManager::ShutdownAll() {
        for (auto& [name, engine] : m_Engines) {
            std::cout << "[Integration] Shutting down external engine: " << name << "\n";
            engine->Shutdown();
        }
    }

    void ExternalEngineManager::RegisterEngine(std::shared_ptr<IExternalEngine> engine) {
        if (engine) {
            std::cout << "[Integration] Registered engine: " << engine->GetName() << "\n";
            m_Engines[engine->GetName()] = engine;
        }
    }

    std::shared_ptr<IExternalEngine> ExternalEngineManager::GetEngine(const std::string& name) {
        auto it = m_Engines.find(name);
        if (it != m_Engines.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<ICADEngine>> ExternalEngineManager::GetCADEngines() {
        std::vector<std::shared_ptr<ICADEngine>> result;
        for (auto& [name, engine] : m_Engines) {
            if (engine->GetType() == EngineType::CAD) {
                if (auto cad = std::dynamic_pointer_cast<ICADEngine>(engine)) {
                    result.push_back(cad);
                }
            }
        }
        return result;
    }

    std::vector<std::shared_ptr<ISimulationEngine>> ExternalEngineManager::GetSimulationEngines() {
        std::vector<std::shared_ptr<ISimulationEngine>> result;
        for (auto& [name, engine] : m_Engines) {
            if (engine->GetType() == EngineType::Simulation) {
                if (auto sim = std::dynamic_pointer_cast<ISimulationEngine>(engine)) {
                    result.push_back(sim);
                }
            }
        }
        return result;
    }

} // namespace SZM::Core::Integration
