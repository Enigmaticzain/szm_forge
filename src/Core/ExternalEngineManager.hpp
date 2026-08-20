#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>

namespace SZM::Core::Integration {

    /**
     * @brief Engine categories
     */
    enum class EngineType {
        CAD,        // E.g., OpenCASCADE, FreeCAD
        Simulation, // E.g., CalculiX, OpenFOAM
        CAM,        // E.g., PyCAM, LinuxCNC
        Rendering   // E.g., Omniverse/USD
    };

    /**
     * @brief Base interface for all external engine integrations
     */
    class IExternalEngine {
    public:
        virtual ~IExternalEngine() = default;

        virtual std::string GetName() const = 0;
        virtual EngineType GetType() const = 0;
        
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual bool IsAvailable() const = 0;
        
        // Execute a generic command string (JSON/CLI) on the external engine
        virtual std::string ExecuteCommand(const std::string& commandJson) = 0;
    };

    /**
     * @brief Specialized interface for CAD engines
     */
    class ICADEngine : public IExternalEngine {
    public:
        EngineType GetType() const override { return EngineType::CAD; }
        
        // e.g. import STEP, generate mesh, boolean operations
        virtual bool ImportModel(const std::string& filePath, std::string& outModelData) = 0;
        virtual bool ExportModel(const std::string& modelData, const std::string& filePath) = 0;
    };

    /**
     * @brief Specialized interface for Simulation engines
     */
    class ISimulationEngine : public IExternalEngine {
    public:
        EngineType GetType() const override { return EngineType::Simulation; }
        
        virtual bool RunSimulation(const std::string& setupJson, std::string& outResultJson) = 0;
        virtual float GetProgress() const = 0;
    };

    /**
     * @brief Manager class to hold and orchestrate external engine plugins
     */
    class ExternalEngineManager {
    public:
        static ExternalEngineManager& GetInstance();

        bool InitializeAll();
        void ShutdownAll();

        void RegisterEngine(std::shared_ptr<IExternalEngine> engine);
        std::shared_ptr<IExternalEngine> GetEngine(const std::string& name);
        
        std::vector<std::shared_ptr<ICADEngine>> GetCADEngines();
        std::vector<std::shared_ptr<ISimulationEngine>> GetSimulationEngines();

    private:
        ExternalEngineManager() = default;
        ~ExternalEngineManager() = default;

        std::unordered_map<std::string, std::shared_ptr<IExternalEngine>> m_Engines;
    };

} // namespace SZM::Core::Integration
