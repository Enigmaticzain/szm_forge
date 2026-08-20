#include "FastAPIBridge.hpp"
#include "UuidUtils.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>

#include "../WebViewer/ExportToThreeJS.hpp"
#include "../FEA/CalculiXPipeline.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include "../Knowledge/KnowledgeBaseService.hpp"
#include "../Geometry/Sketch/Sketch2D.hpp"
#include "../Geometry/operations/ExtrudeOp.hpp"
#include "../Geometry/operations/BooleanOp.hpp"
#include "../Geometry/BRepQueries.hpp"
#include "StructuralSolveService.hpp"
#include "TableProjectService.hpp"
#include "MachineConfig.hpp"
#include "../AI/ObservationSystem/ObservationSystem.hpp"

namespace SZM::WebAPI {

static std::unique_ptr<httplib::Server> g_Server;
static std::unique_ptr<std::thread> g_ServerThread;
static std::mutex g_ServerMutex;

// ============================================================================
// SimulationAPI Implementation
// ============================================================================

SimulationAPI::SimulationAPI(uint16_t port) : m_Port(port) {
    std::cout << "[SimulationAPI] Initialized on port " << port << "\n";
}

SimulationAPI::~SimulationAPI() {
    m_Shutdown = true;
    if (m_IsRunning) {
        Stop();
    }
    for (auto& thread : m_JobThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool SimulationAPI::Start() {
    if (m_IsRunning) {
        std::cout << "[SimulationAPI] Already running\n";
        return true;
    }

    try {
        m_IsRunning = true;
        m_QueuedJobs = 0;
        m_RunningJobs = 0;

        std::cout << "[SimulationAPI] Server started on port " << m_Port << "\n";
        std::cout << "[SimulationAPI] API Documentation: http://localhost:" << m_Port << "/docs\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[SimulationAPI] Failed to start: " << e.what() << "\n";
        m_IsRunning = false;
        return false;
    }
}

void SimulationAPI::Stop() {
    if (!m_IsRunning) {
        return;
    }

    try {
        m_Shutdown = true;
        m_IsRunning = false;
        for (auto& thread : m_JobThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        m_JobThreads.clear();
        std::cout << "[SimulationAPI] Server stopped\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[SimulationAPI] Error stopping server: " << e.what() << "\n";
    }
}

std::string SimulationAPI::SubmitJob(const JobRequest& request) {
    if (!m_IsRunning) {
        std::cerr << "[SimulationAPI] Server not running\n";
        return "";
    }

    try {
        const std::string jobId = MakeJobId();
        JobResult job;
        job.jobId = jobId;
        job.status = "pending";
        job.progress = 0.0f;
        job.resultJSON = "{}";

        {
            std::lock_guard<std::mutex> lock(m_JobMutex);
            m_Jobs[jobId] = job;
            m_QueuedJobs++;
        }

        std::cout << "[SimulationAPI] Job submitted: " << jobId << "\n";
        std::cout << "[SimulationAPI] Type: " << request.simulationType
                  << ", Component: " << request.componentId
                  << ", Priority: " << request.priority << "\n";

        m_JobThreads.emplace_back([this, jobId, request]() {
            {
                std::lock_guard<std::mutex> lock(m_JobMutex);
                auto& stored = m_Jobs[jobId];
                if (stored.status == "cancelled") {
                    return;
                }
                stored.status = "running";
                stored.progress = 0.0f;
                if (m_QueuedJobs > 0) {
                    m_QueuedJobs--;
                }
                m_RunningJobs++;
            }

            constexpr int steps = 6;
            for (int step = 0; step < steps && !m_Shutdown; ++step) {
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
                std::lock_guard<std::mutex> lock(m_JobMutex);
                auto& stored = m_Jobs[jobId];
                if (stored.status == "cancelled") {
                    break;
                }
                stored.progress = (step + 1) * (100.0f / steps);
            }

            std::lock_guard<std::mutex> lock(m_JobMutex);
            auto& stored = m_Jobs[jobId];
            if (stored.status == "cancelled") {
                stored.progress = 0.0f;
                stored.resultJSON = "{}";
            } else if (m_Shutdown) {
                stored.status = "failed";
                stored.errorMessage = "Simulation service shut down";
                stored.resultJSON = "{}";
            } else {
                stored.status = "completed";
                stored.progress = 100.0f;
                nlohmann::json output;
                output["jobId"] = jobId;
                output["componentId"] = request.componentId;
                output["simulationType"] = request.simulationType;
                output["status"] = "success";
                output["results"] = {
                    {"maxStress", 123.4},
                    {"maxTemperature", 87.2},
                    {"durationMs", 950}
                };
                stored.resultJSON = output.dump();
            }

            if (m_RunningJobs > 0) {
                m_RunningJobs--;
            }
        });

        return jobId;
    }
    catch (const std::exception& e) {
        std::cerr << "[SimulationAPI] Job submission failed: " << e.what() << "\n";
        return "";
    }
}

SimulationAPI::JobResult SimulationAPI::GetJobStatus(const std::string& jobId) {
    JobResult result;
    result.jobId = jobId;

    if (!m_IsRunning) {
        result.status = "error";
        result.errorMessage = "Server not running";
        return result;
    }

    try {
        std::lock_guard<std::mutex> lock(m_JobMutex);
        auto it = m_Jobs.find(jobId);
        if (it == m_Jobs.end()) {
            result.status = "error";
            result.errorMessage = "Job not found";
            return result;
        }
        return it->second;
    }
    catch (const std::exception& e) {
        result.status = "error";
        result.errorMessage = e.what();
        return result;
    }
}

bool SimulationAPI::CancelJob(const std::string& jobId) {
    if (!m_IsRunning) {
        return false;
    }

    try {
        std::lock_guard<std::mutex> lock(m_JobMutex);
        auto it = m_Jobs.find(jobId);
        if (it == m_Jobs.end()) {
            return false;
        }

        if (it->second.status == "pending") {
            if (m_QueuedJobs > 0) {
                m_QueuedJobs--;
            }
            it->second.status = "cancelled";
            it->second.progress = 0.0f;
            it->second.resultJSON = "{}";
            return true;
        }

        if (it->second.status == "running") {
            it->second.status = "cancelled";
            return true;
        }

        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "[SimulationAPI] Cancel failed: " << e.what() << "\n";
        return false;
    }
}

std::string SimulationAPI::GetJobResult(const std::string& jobId) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        std::lock_guard<std::mutex> lock(m_JobMutex);
        auto it = m_Jobs.find(jobId);
        if (it == m_Jobs.end()) {
            nlohmann::json error;
            error["error"] = "Job not found";
            return error.dump();
        }

        if (it->second.status != "completed") {
            nlohmann::json response;
            response["jobId"] = jobId;
            response["status"] = it->second.status;
            response["progress"] = it->second.progress;
            response["errorMessage"] = it->second.errorMessage;
            return response.dump();
        }

        return it->second.resultJSON;
    }
    catch (const std::exception& e) {
        std::cerr << "[SimulationAPI] Result retrieval failed: " << e.what() << "\n";
        nlohmann::json error;
        error["error"] = e.what();
        return error.dump();
    }
}

// ============================================================================
// AnalysisAPI Implementation
// ============================================================================

AnalysisAPI::AnalysisAPI(uint16_t port) : m_Port(port) {
    std::cout << "[AnalysisAPI] Initialized on port " << port << "\n";
}

bool AnalysisAPI::Start() {
    if (m_IsRunning) {
        std::cout << "[AnalysisAPI] Already running\n";
        return true;
    }

    try {
        m_IsRunning = true;
        std::cout << "[AnalysisAPI] Server started on port " << m_Port << "\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[AnalysisAPI] Failed to start: " << e.what() << "\n";
        m_IsRunning = false;
        return false;
    }
}

void AnalysisAPI::Stop() {
    if (!m_IsRunning) {
        return;
    }

    try {
        m_IsRunning = false;
        std::cout << "[AnalysisAPI] Server stopped\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[AnalysisAPI] Error stopping server: " << e.what() << "\n";
    }
}

std::string AnalysisAPI::GetComponentInfo(uint32_t componentId) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        std::stringstream ss;
        ss << "{\n";
        ss << "  \"componentId\": " << componentId << ",\n";
        ss << "  \"name\": \"Component_" << componentId << "\",\n";
        ss << "  \"type\": \"Part\",\n";
        ss << "  \"material\": \"Steel\",\n";
        ss << "  \"mass\": 2.5,\n";
        ss << "  \"volume\": 0.001\n";
        ss << "}\n";

        std::cout << "[AnalysisAPI] Component info requested: " << componentId << "\n";
        return ss.str();
    }
    catch (const std::exception& e) {
        std::cerr << "[AnalysisAPI] Component info failed: " << e.what() << "\n";
        return "";
    }
}

std::string AnalysisAPI::GetAssemblyTree() {
    if (!m_IsRunning) {
        return "";
    }

    try {
        std::string tree = "{\n"
                          "  \"root\": {\n"
                          "    \"name\": \"Assembly\",\n"
                          "    \"children\": []\n"
                          "  }\n"
                          "}\n";

        std::cout << "[AnalysisAPI] Assembly tree requested\n";
        return tree;
    }
    catch (const std::exception& e) {
        std::cerr << "[AnalysisAPI] Assembly tree failed: " << e.what() << "\n";
        return "";
    }
}

std::string AnalysisAPI::GetSimulationHistory(uint32_t componentId, uint32_t limit) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        std::stringstream ss;
        ss << "{\n";
        ss << "  \"componentId\": " << componentId << ",\n";
        ss << "  \"limit\": " << limit << ",\n";
        ss << "  \"history\": []\n";
        ss << "}\n";

        std::cout << "[AnalysisAPI] Simulation history requested for component " << componentId << "\n";
        return ss.str();
    }
    catch (const std::exception& e) {
        std::cerr << "[AnalysisAPI] History retrieval failed: " << e.what() << "\n";
        return "";
    }
}

std::string AnalysisAPI::GetPredictions(uint32_t componentId, const std::string& predictionType) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        std::stringstream ss;
        ss << "{\n";
        ss << "  \"componentId\": " << componentId << ",\n";
        ss << "  \"type\": \"" << predictionType << "\",\n";
        ss << "  \"predictions\": []\n";
        ss << "}\n";

        std::cout << "[AnalysisAPI] Predictions requested: " << predictionType << "\n";
        return ss.str();
    }
    catch (const std::exception& e) {
        std::cerr << "[AnalysisAPI] Predictions failed: " << e.what() << "\n";
        return "";
    }
}

// ============================================================================
// ExportAPI Implementation
// ============================================================================

ExportAPI::ExportAPI(uint16_t port) : m_Port(port) {
    std::cout << "[ExportAPI] Initialized on port " << port << "\n";
}

bool ExportAPI::Start() {
    if (m_IsRunning) {
        std::cout << "[ExportAPI] Already running\n";
        return true;
    }

    try {
        m_IsRunning = true;
        std::cout << "[ExportAPI] Server started on port " << m_Port << "\n";
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[ExportAPI] Failed to start: " << e.what() << "\n";
        m_IsRunning = false;
        return false;
    }
}

void ExportAPI::Stop() {
    if (!m_IsRunning) {
        return;
    }

    try {
        m_IsRunning = false;
        std::cout << "[ExportAPI] Server stopped\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[ExportAPI] Error stopping server: " << e.what() << "\n";
    }
}

static std::filesystem::path EnsureExportDirectory();

std::string ExportAPI::ExportComponentSTEP(uint32_t componentId) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        const auto exportPath = EnsureExportDirectory();
        const auto filePath = exportPath / ("component_" + std::to_string(componentId) + ".step");
        std::ofstream file(filePath);
        if (!file) {
            throw std::runtime_error("Failed to open STEP export file");
        }

        file << "ISO-10303-21;\n";
        file << "HEADER;\n";
        file << "FILE_DESCRIPTION(('SZM Forge STEP export'),'2;1');\n";
        file << "FILE_NAME('" << filePath.filename().string() << "','" << __DATE__ << "');\n";
        file << "FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));\n";
        file << "ENDSEC;\n";
        file << "DATA;\n";
        file << "#1 = CARTESIAN_POINT(('Component_" << componentId << "'));\n";
        file << "ENDSEC;\n";
        file << "END-ISO-10303-21;\n";
        file.close();

        std::cout << "[ExportAPI] STEP export written to " << filePath.string() << "\n";
        return filePath.string();
    }
    catch (const std::exception& e) {
        std::cerr << "[ExportAPI] STEP export failed: " << e.what() << "\n";
        return "";
    }
}

std::string ExportAPI::ExportResultsCSV(const std::string& jobId) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        const auto exportPath = EnsureExportDirectory();
        const auto filePath = exportPath / (jobId + "_results.csv");
        std::ofstream file(filePath);
        if (!file) {
            throw std::runtime_error("Failed to open CSV export file");
        }

        file << "metric,value\n";
        file << "maxStress,123.4\n";
        file << "maxTemperature,87.2\n";
        file << "durationMs,950\n";
        file.close();

        std::cout << "[ExportAPI] CSV export written to " << filePath.string() << "\n";
        return filePath.string();
    }
    catch (const std::exception& e) {
        std::cerr << "[ExportAPI] CSV export failed: " << e.what() << "\n";
        return "";
    }
}

std::string ExportAPI::ExportSceneThreeJS() {
    if (!m_IsRunning) {
        return "";
    }

    try {
        const auto exportPath = EnsureExportDirectory();
        const auto filePath = exportPath / "scene.json";
        std::ofstream file(filePath);
        if (!file) {
            throw std::runtime_error("Failed to open Three.js export file");
        }

        std::vector<SZM::WebAPI::SimulationSnapshot::ComponentSnap> empty_components;
        auto scene = SZM::WebViewer::ThreeJSExporter::ExportScene(empty_components);
        file << scene.dump(2);
        file.close();

        std::cout << "[ExportAPI] Scene export written to " << filePath.string() << "\n";
        return filePath.string();
    } catch (const std::exception& e) {
        std::cerr << "[ExportAPI] Three.js export failed: " << e.what() << "\n";
        return "";
    }
}

std::string ExportAPI::ExportSceneUSD(const std::string& outputPath) {
    auto* scene = SimulationEngine::GetInstance().GetScene();
    if (!scene) {
        return R"({"error": "No Scene Graph available to export"})";
    }

    // Build Scene JSON payload
    nlohmann::json entitiesArray = nlohmann::json::array();
    auto entities = scene->View<SZM::SceneGraph::TransformComponent>();
    
    for (auto e : entities) {
        std::string name = "Entity_" + std::to_string(static_cast<uint32_t>(e));
        if (scene->HasComponent<SZM::SceneGraph::TagComponent>(e)) {
            name = scene->GetComponent<SZM::SceneGraph::TagComponent>(e).name;
        }
        
        const auto& transform = scene->GetComponent<SZM::SceneGraph::TransformComponent>(e);
        
        entitiesArray.push_back({
            {"id", static_cast<uint32_t>(e)},
            {"name", name},
            {"transform", {
                {"position", {transform.position.x, transform.position.y, transform.position.z}},
                {"rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z}},
                {"scale", {transform.scale.x, transform.scale.y, transform.scale.z}}
            }}
        });
    }

    nlohmann::json sceneJson = {
        {"entities", entitiesArray}
    };

    nlohmann::json requestJson = {
        {"scene_json", sceneJson.dump()},
        {"output_path", outputPath}
    };

    try {
        httplib::Client cli("127.0.0.1", 8003); // Python ExternalEngineBridge
        cli.set_connection_timeout(5);
        
        auto res = cli.Post("/usd/export", requestJson.dump(), "application/json");
        
        if (res && res->status == 200) {
            std::cout << "[ExportAPI] USD export successful\n";
            return res->body;
        } else {
            return R"({"status": "error", "error": "Bridge unreachable or failed"})";
        }
    } catch (const std::exception& e) {
        return std::string(R"({"status": "error", "error": ")") + e.what() + "\"}";
    }
}

std::string ExportAPI::ExportReportPDF(const std::string& jobId) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        const auto exportPath = EnsureExportDirectory();
        const auto filePath = exportPath / (jobId + "_report.pdf");
        std::ofstream file(filePath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open PDF export file");
        }

        file << "%PDF-1.4\n";
        file << "%\xE2\xE3\xCF\xD3\n";
        file << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
        file << "2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n";
        file << "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Contents 4 0 R >>\nendobj\n";
        file << "4 0 obj\n<< /Length 44 >>\nstream\nBT /F1 24 Tf 72 720 Td (Report for " << jobId << ") Tj ET\nendstream\nendobj\n";
        file << "xref\n0 5\n0000000000 65535 f \n0000000010 00000 n \n0000000060 00000 n \n0000000110 00000 n \n0000000170 00000 n \ntrailer\n<< /Size 5 /Root 1 0 R >>\nstartxref\n250\n%%EOF\n";
        file.close();

        std::cout << "[ExportAPI] PDF export written to " << filePath.string() << "\n";
        return filePath.string();
    }
    catch (const std::exception& e) {
        std::cerr << "[ExportAPI] PDF export failed: " << e.what() << "\n";
        return "";
    }
}

// ============================================================================
// APIManager Implementation
// ============================================================================

APIManager& APIManager::GetInstance() {
    static APIManager instance;
    return instance;
}

// ============================================================================
// Shared telemetry snapshot (written by simulation, read by HTTP handlers)
// ============================================================================
static SimulationSnapshot g_Snapshot;

static std::filesystem::path EnsureExportDirectory() {
    std::filesystem::path exportPath = std::filesystem::current_path() / "exports";
    std::error_code ec;
    std::filesystem::create_directories(exportPath, ec);
    return exportPath;
}

void APIManager::PushComponentSnapshot(const std::string& name,
                                        float stress, float temperature,
                                        float stressRatio, float tempRatio,
                                        double simTime) {
    std::lock_guard<std::mutex> lock(g_Snapshot.mu);
    g_Snapshot.simTime = simTime;

    const auto it = g_Snapshot.indexByName.find(name);
    if (it != g_Snapshot.indexByName.end()) {
        auto& component = g_Snapshot.components[it->second];
        component.stress = stress;
        component.temperature = temperature;
        component.stressRatio = stressRatio;
        component.tempRatio = tempRatio;
        component.status = (stressRatio > 0.9f || tempRatio > 0.9f) ? "FAIL" : "SAFE";
        return;
    }

    g_Snapshot.components.push_back({name, stress, temperature, stressRatio, tempRatio,
                                     (stressRatio > 0.9f || tempRatio > 0.9f) ? "FAIL" : "SAFE"});
    g_Snapshot.indexByName[name] = g_Snapshot.components.size() - 1;
}

bool APIManager::Initialize(uint16_t basePort, const std::string& uiDistPath) {
    std::lock_guard<std::mutex> lock(g_ServerMutex);
    try {
        std::cout << "[APIManager] Initializing embedded HTTP server...\n";

        if (!g_Server) {
            g_Server = std::make_unique<httplib::Server>();
            auto& simApi = m_SimulationAPI;
            auto& analysisApi = m_AnalysisAPI;
            auto& exportApi = m_ExportAPI;
            simApi.Start();
            analysisApi.Start();
            exportApi.Start();

            Materials::MaterialDatabase::GetInstance().LoadStandardLibraries();
            Knowledge::KnowledgeBaseService::GetInstance().Load();

            // CORS — React dev server (:3000) and embedded webview call POST /api/*
            g_Server->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
                res.set_header("Access-Control-Allow-Origin", "*");
                res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept");
                if (req.method == "OPTIONS") {
                    res.status = 204;
                    return httplib::Server::HandlerResponse::Handled;
                }
                return httplib::Server::HandlerResponse::Unhandled;
            });

            // ── GET /api/status ──────────────────────────────────────
            g_Server->Get("/api/status", [](const httplib::Request&, httplib::Response& res) {
                nlohmann::json j;
                j["status"]  = "running";
                j["version"] = "0.3.0";
                j["engine"]  = "SZM Forge";
                j["modules"] = {"Simulation", "AI Predictions", "Digital Twin", "WebAPI"};
                res.set_content(j.dump(2), "application/json");
            });

            // ── GET /api/assembly ────────────────────────────────────
            g_Server->Get("/api/assembly", [](const httplib::Request&, httplib::Response& res) {
                res.set_content(TableProjectService::GetInstance().GetAssemblyTree().dump(2),
                                "application/json");
            });

            // ── GET /api/project ─────────────────────────────────────
            g_Server->Get("/api/project", [](const httplib::Request&, httplib::Response& res) {
                res.set_content(TableProjectService::GetInstance().GetProjectState().dump(2),
                                "application/json");
            });

            g_Server->Get("/api/project/limits", [](const httplib::Request&, httplib::Response& res) {
                res.set_content(TableProjectService::GetInstance().GetLimits().dump(2),
                                "application/json");
            });

            g_Server->Get("/api/ai/insights", [](const httplib::Request&, httplib::Response& res) {
                res.set_content(TableProjectService::GetInstance().GetAIInsights().dump(2),
                                "application/json");
            });

            auto postJson = [](const httplib::Request& req, httplib::Response& res,
                               const std::function<nlohmann::json(const nlohmann::json&)>& handler) {
                nlohmann::json body;
                try {
                    body = req.body.empty() ? nlohmann::json::object() : nlohmann::json::parse(req.body);
                } catch (...) {
                    res.status = 400;
                    res.set_content(R"({"ok":false,"error":"Invalid JSON"})", "application/json");
                    return;
                }

                try {
                    res.set_content(handler(body).dump(2), "application/json");
                } catch (const std::exception& e) {
                    res.status = 500;
                    nlohmann::json error;
                    error["ok"] = false;
                    error["error"] = e.what();
                    res.set_content(error.dump(2), "application/json");
                } catch (...) {
                    res.status = 500;
                    res.set_content(R"({"ok":false,"error":"Internal server error"})", "application/json");
                }
            };

            g_Server->Post("/api/project/new", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    const std::string name = body.value("name", "Table Workshop Project");
                    return TableProjectService::GetInstance().CreateNewProject(name);
                });
            });

            g_Server->Post("/api/project/table", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    const std::string mat = body.value("materialId", "MAT-STEEL-STRUCT");
                    const float loadKg = body.value("loadKg", 200.0f);
                    return TableProjectService::GetInstance().CreateTable(mat, loadKg);
                });
            });

            g_Server->Post("/api/project/furniture", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    const std::string type = body.value("type", "table");
                    const std::string mat = body.value("materialId", "MAT-STEEL-STRUCT");
                    const float loadKg = body.value("loadKg", 200.0f);
                    return TableProjectService::GetInstance().CreateFurniture(type, mat, loadKg);
                });
            });

            g_Server->Post("/api/project/load", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    return TableProjectService::GetInstance().SetLoad(body.value("loadKg", 200.0f));
                });
            });

            g_Server->Post("/api/project/limits", [&postJson](const httplib::Request& req, httplib::Response& res) {
                (void)req;
                postJson(req, res, [](const nlohmann::json&) {
                    return TableProjectService::GetInstance().RunLimitSweep();
                });
            });

            g_Server->Post("/api/ai/improve", [&postJson](const httplib::Request& req, httplib::Response& res) {
                (void)req;
                postJson(req, res, [](const nlohmann::json&) {
                    return TableProjectService::GetInstance().RunAIImprove();
                });
            });

            g_Server->Post("/api/ai/train", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    auto pythonRes = cli.Post("/ai/train", body.dump(), "application/json");
                    if (pythonRes && pythonRes->status == 200) {
                        return nlohmann::json::parse(pythonRes->body);
                    }
                    return nlohmann::json{{"ok", false}, {"error", "Failed to reach AI service"}};
                });
            });

            g_Server->Post("/api/ai/generate", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(30); // Generation might take longer
                    auto pythonRes = cli.Post("/ai/generate", body.dump(), "application/json");
                    if (pythonRes && pythonRes->status == 200) {
                        return nlohmann::json::parse(pythonRes->body);
                    }
                    return nlohmann::json{{"ok", false}, {"error", "Failed to reach AI service"}};
                });
            });

            g_Server->Post("/api/workshop/bootstrap", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    auto& svc = TableProjectService::GetInstance();
                    const std::string name = body.value("name", "Four-Leg Table Study");
                    const std::string mat = body.value("materialId", "MAT-STEEL-STRUCT");
                    const float loadKg = body.value("loadKg", 200.0f);
                    svc.CreateNewProject(name);
                    auto table = svc.CreateTable(mat, loadKg);
                    auto limits = svc.RunLimitSweep();
                    auto improved = svc.RunAIImprove();
                    return nlohmann::json{
                        {"ok", true},
                        {"project", svc.GetProjectState()},
                        {"table", table},
                        {"limits", limits},
                        {"ai", improved}
                    };
                });
            });

            // ── Structural kit (codes/ physics via MaterialDatabase) ──
            g_Server->Post("/api/structural/solve", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    return StructuralSolveService::Solve(body);
                });
            });

            // ── CalculiX FEA via Python bridge (port 8003) ───────────
            g_Server->Post("/api/simulation/fea/run", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    cli.set_read_timeout(60);
                    auto r = cli.Post("/simulation/fea/run", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "CalculiX bridge unreachable"}};
                });
            });

            // ── OpenFOAM CFD via Python bridge (port 8003) ───────────
            g_Server->Post("/api/simulation/cfd/run", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    cli.set_read_timeout(60);
                    auto r = cli.Post("/simulation/cfd/run", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "OpenFOAM bridge unreachable"}};
                });
            });

            // ── CuraEngine Slicer via Python bridge (port 8003) ──────
            g_Server->Post("/api/manufacturing/slicer/run", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    cli.set_read_timeout(60);
                    auto r = cli.Post("/manufacturing/slicer/run", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "Slicer bridge unreachable"}};
                });
            });

            // ── USD & MaterialX Export via Python bridge (port 8003) ──
            g_Server->Post("/api/export/usd", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    cli.set_read_timeout(60);
                    auto r = cli.Post("/export/usd", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "USD Export bridge unreachable"}};
                });
            });

            // ── OptiX Raytracing via Python bridge (port 8003) ──
            g_Server->Post("/api/render/optix", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    cli.set_read_timeout(60);
                    auto r = cli.Post("/render/optix", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "OptiX Render bridge unreachable"}};
                });
            });

            g_Server->Post("/api/auth/login", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    auto r = cli.Post("/auth/login", body.dump(), "application/json");
                    if (r && r->status == 200) return nlohmann::json::parse(r->body);
                    return nlohmann::json{{"error", "Auth bridge unreachable"}};
                });
            });
            g_Server->Post("/api/auth/register", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    auto r = cli.Post("/auth/register", body.dump(), "application/json");
                    if (r && r->status == 200) return nlohmann::json::parse(r->body);
                    return nlohmann::json{{"error", "Auth bridge unreachable"}};
                });
            });
            g_Server->Get("/api/auth/me", [](const httplib::Request& req, httplib::Response& res) {
                httplib::Client cli("127.0.0.1", 8003);
                auto r = cli.Get("/auth/me");
                if (r && r->status == 200) {
                    res.set_content(r->body, "application/json");
                } else {
                    res.set_content(R"({"error": "Auth bridge unreachable"})", "application/json");
                }
            });

            // ── Simulation Optimizer (Plan 37) ──
            g_Server->Post("/api/simulation/optimize", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_read_timeout(120);
                    auto r = cli.Post("/simulation/optimize", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "Optimization bridge unreachable"}};
                });
            });

            // ── AI Observation System (Plan 38) ──
            g_Server->Post("/api/ai/observe/start", [](const httplib::Request& req, httplib::Response& res) {
                SZM::AI::Observation::ObservationSystem::GetInstance().StartObserving();
                res.set_content(R"({"status": "observing"})", "application/json");
            });

            g_Server->Post("/api/ai/observe/stop", [](const httplib::Request& req, httplib::Response& res) {
                SZM::AI::Observation::ObservationSystem::GetInstance().StopObserving();
                res.set_content(R"({"status": "stopped"})", "application/json");
            });

            g_Server->Post("/api/ai/observe/action", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    try {
                        SZM::AI::Observation::UserAction action;
                        action.id = SZM::AI::Observation::ObservationSystem::GetInstance().GetStatistics()["total_actions"]; 
                        // ObservationSystem has GetStatistics(), we don't have direct access to generate action id, but it doesn't matter for the struct init
                        if (body.contains("type") && body["type"] == "MOUSE_CLICK") {
                            action.type = SZM::AI::Observation::ActionType::MOUSE_CLICK;
                            int x = body.value("x", 0);
                            int y = body.value("y", 0);
                            
                            SZM::AI::Observation::ActionContext ctx;
                            if (body.contains("context")) {
                                auto bCtx = body["context"];
                                ctx.focused_element_id = bCtx.value("element_id", "");
                                ctx.focused_element_type = bCtx.value("element_type", "");
                                ctx.focused_element_text = bCtx.value("element_text", "");
                            }
                            
                            if (SZM::AI::Observation::ObservationSystem::GetInstance().IsObserving()) {
                                SZM::AI::Observation::ObservationSystem::GetInstance().RecordMouseClick(x, y, 0, 1, ctx);
                            }
                        }
                        else if (body.contains("type") && body["type"] == "KEYBOARD_SHORTCUT") {
                            action.type = SZM::AI::Observation::ActionType::KEYBOARD_SHORTCUT;
                            SZM::AI::Observation::KeyCombination keys;
                            if (body.contains("keys")) {
                                for (auto& k : body["keys"]) keys.keys.push_back(k.get<std::string>());
                            }
                            SZM::AI::Observation::ActionContext ctx;
                            if (SZM::AI::Observation::ObservationSystem::GetInstance().IsObserving()) {
                                SZM::AI::Observation::ObservationSystem::GetInstance().RecordKeyboardShortcut(keys, ctx);
                            }
                        }
                        
                        return nlohmann::json{{"status", "recorded"}};
                    } catch (const std::exception& e) {
                        return nlohmann::json{{"status", "error"}, {"message", e.what()}};
                    }
                });
            });

            g_Server->Get("/api/ai/observe/stats", [](const httplib::Request& req, httplib::Response& res) {
                try {
                    auto stats = SZM::AI::Observation::ObservationSystem::GetInstance().GetStatistics();
                    res.set_content(stats.dump(), "application/json");
                } catch (const std::exception& e) {
                    res.set_content(R"({"error": "Failed to get stats"})", "application/json");
                }
            });

            // ── FreeCAD headless parametric modelling (port 8003) ────
            g_Server->Post("/api/freecad/parametric", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    cli.set_read_timeout(60);
                    auto r = cli.Post("/freecad/parametric", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "FreeCAD bridge unreachable"}};
                });
            });

            // ── STEP assembly hierarchy parser (port 8003) ──────────
            g_Server->Post("/api/step/assembly/parse", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    httplib::Client cli("127.0.0.1", 8003);
                    cli.set_connection_timeout(5);
                    cli.set_read_timeout(30);
                    auto r = cli.Post("/step/assembly/parse", body.dump(), "application/json");
                    if (r && r->status == 200) {
                        return nlohmann::json::parse(r->body);
                    }
                    return nlohmann::json{{"status", "error"}, {"error", "STEP bridge unreachable"}};
                });
            });

            // ── CSG Boolean operations ─────────────────────────────
            g_Server->Post("/api/csg/boolean", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    // Try Python bridge first (FreeCAD/pythonocc quality)
                    {
                        httplib::Client cli("127.0.0.1", 8003);
                        cli.set_connection_timeout(2);
                        cli.set_read_timeout(30);
                        auto r = cli.Post("/csg/boolean", body.dump(), "application/json");
                        if (r && r->status == 200) {
                            auto j = nlohmann::json::parse(r->body);
                            if (j.value("status", "") == "success") return j;
                        }
                    }

                    // C++ fallback
                    std::string opStr = body.value("operation", "union");
                    SZM::Geometry::BooleanOpType opType =
                        (opStr == "difference") ? SZM::Geometry::BooleanOpType::Difference :
                        (opStr == "intersect")  ? SZM::Geometry::BooleanOpType::Intersect  :
                                                   SZM::Geometry::BooleanOpType::Union;

                    auto buildBody = [](const nlohmann::json& mesh) {
                        auto sb = std::make_shared<SZM::Geometry::SolidBody>();
                        std::vector<SZM::Geometry::Vector3> verts;
                        for (const auto& v : mesh.value("vertices", nlohmann::json::array()))
                            verts.push_back({v[0],v[1],v[2]});
                        for (const auto& f : mesh.value("faces", nlohmann::json::array())) {
                            if (f.size() < 3) continue;
                            sb->AddFace({verts[f[0].get<int>()],
                                         verts[f[1].get<int>()],
                                         verts[f[2].get<int>()]});
                        }
                        sb->Stitch();
                        return sb;
                    };

                    auto target = buildBody(body.value("target", nlohmann::json::object()));
                    auto tool   = buildBody(body.value("tool",   nlohmann::json::object()));

                    auto result = SZM::Geometry::BooleanCSG::Execute(target, tool, opType);
                    if (!result)
                        return nlohmann::json{{"status","error"},{"error","CSG produced empty result"}};

                    // Serialize result mesh
                    nlohmann::json vArr = nlohmann::json::array();
                    for (const auto& v : result->GetVertices())
                        vArr.push_back({v.position.x, v.position.y, v.position.z});

                    nlohmann::json fArr = nlohmann::json::array();
                    for (uint32_t fi = 0; fi < result->GetFaceCount(); ++fi) {
                        auto fv = result->GetFaceVertices(fi);
                        nlohmann::json tri = nlohmann::json::array();
                        for (auto& p : fv) tri.push_back({p.x, p.y, p.z});
                        fArr.push_back(tri);
                    }

                    return nlohmann::json{
                        {"status",       "success"},
                        {"engine",       "cpp-brep"},
                        {"operation",    opStr},
                        {"vertex_count", result->GetVertexCount()},
                        {"face_count",   result->GetFaceCount()},
                        {"vertices",     vArr},
                        {"faces",        fArr}
                    };
                });
            });

            // ── BRep face/edge/vertex picking ───────────────────────
            g_Server->Post("/api/brep/pick", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    // Expect: {ray_origin:[x,y,z], ray_dir:[x,y,z], mode:"face"|"edge"|"vertex",
                    //          faces:[[v0,v1,v2],...], vertices:[[x,y,z],...]}
                    auto ro = body.value("ray_origin", nlohmann::json::array());
                    auto rd = body.value("ray_dir",    nlohmann::json::array());
                    if (ro.size() < 3 || rd.size() < 3)
                        return nlohmann::json{{"ok",false},{"error","ray_origin/ray_dir required"}};

                    SZM::Geometry::Vector3 rayOrigin{ro[0],ro[1],ro[2]};
                    SZM::Geometry::Vector3 rayDir{rd[0],rd[1],rd[2]};
                    rayDir = rayDir.Normalize();

                    // Build a transient SolidBody from the provided mesh data
                    SZM::Geometry::SolidBody body_;
                    if (body.contains("faces") && body.contains("vertices")) {
                        std::vector<SZM::Geometry::Vector3> verts;
                        for (const auto& v : body["vertices"])
                            verts.push_back({v[0],v[1],v[2]});
                        for (const auto& f : body["faces"]) {
                            if (f.size() < 3) continue;
                            std::vector<SZM::Geometry::Vector3> tri = {
                                verts[f[0].get<int>()],
                                verts[f[1].get<int>()],
                                verts[f[2].get<int>()]
                            };
                            body_.AddFace(tri);
                        }
                        body_.Stitch();
                    }

                    std::string mode = body.value("mode", "face");
                    SZM::Geometry::BRepQueries::RayHit hit;

                    if (mode == "edge")
                        hit = SZM::Geometry::BRepQueries::RaycastEdge(body_, rayOrigin, rayDir);
                    else if (mode == "vertex")
                        hit = SZM::Geometry::BRepQueries::RaycastVertex(body_, rayOrigin, rayDir);
                    else
                        hit = SZM::Geometry::BRepQueries::RaycastFace(body_, rayOrigin, rayDir);

                    if (hit.index == UINT32_MAX)
                        return nlohmann::json{{"ok",true},{"hit",false}};

                    return nlohmann::json{
                        {"ok",    true},
                        {"hit",   true},
                        {"mode",  mode},
                        {"index", hit.index},
                        {"distance", hit.distance},
                        {"point", {hit.point.x, hit.point.y, hit.point.z}}
                    };
                });
            });

            // ── 2D Sketch solve + extrude ───────────────────────────
            g_Server->Post("/api/sketch/solve", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    auto sketch = SZM::Geometry::Sketch2D::FromJSON(body);
                    bool ok = sketch.Solve();
                    auto result = sketch.ToJSON();
                    result["solved"] = ok;
                    result["dof"]    = sketch.GetDOFCount();
                    return result;
                });
            });

            g_Server->Post("/api/sketch/extrude", [&postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [](const nlohmann::json& body) {
                    auto sketch = SZM::Geometry::Sketch2D::FromJSON(
                        body.value("sketch", nlohmann::json::object()));
                    sketch.Solve();
                    auto profile = sketch.GenerateProfile();
                    SZM::Geometry::ExtrudeParams params;
                    params.Distance = body.value("distance", 0.01);
                    params.Direction = SZM::Geometry::Vector3{
                        profile.PlaneNormal.x,
                        profile.PlaneNormal.y,
                        profile.PlaneNormal.z
                    };
                    auto solid = SZM::Geometry::Operations::LinearExtrude(profile, params);
                    nlohmann::json out;
                    out["ok"]           = true;
                    out["vertex_count"] = solid.GetVertexCount();
                    out["face_count"]   = solid.GetFaceCount();
                    out["volume_m3"]    = solid.CalculateVolume();
                    return out;
                });
            });

            // ── Machine Configuration ─────────────────────────────────
            g_Server->Get("/api/machine/config", [](const httplib::Request& req, httplib::Response& res) {
                (void)req;
                auto id = req.has_param("id") ? req.get_param_value("id") : "default";
                auto cfg = SZM::WebAPI::MachineConfigRegistry::Get().Load(id);
                res.set_content(cfg.ToJSON().dump(), "application/json");
            });

            g_Server->Post("/api/machine/config", [](const httplib::Request& req, httplib::Response& res) {
                try {
                    auto body = nlohmann::json::parse(req.body);
                    auto cfg  = SZM::WebAPI::MachineConfig::FromJSON(body);
                    SZM::WebAPI::MachineConfigRegistry::Get().Save(cfg);
                    nlohmann::json out;
                    out["ok"]  = true;
                    out["id"]  = cfg.id;
                    res.set_content(out.dump(), "application/json");
                } catch (const std::exception& e) {
                    res.status = 400;
                    res.set_content(nlohmann::json{{"ok", false},{"error", e.what()}}.dump(), "application/json");
                }
            });

            // ── CalculiX examples catalog (codes/code 8) ─────────────
            g_Server->Get("/api/code8/catalog", [](const httplib::Request& req, httplib::Response& res) {
                (void)req;
                const auto root = SZM::FEA::CalculiXPipeline::DiscoverCode8Root();
                const auto toolchain = SZM::FEA::CalculiXPipeline::ProbeToolchain();

                nlohmann::json j;
                j["ok"] = !root.empty();
                j["root"] = root.string();
                j["toolchain"] = {
                    {"python", {{"name", toolchain.Python.Name}, {"available", toolchain.Python.Available}}},
                    {"cgx", {{"name", toolchain.CGX.Name}, {"available", toolchain.CGX.Available}}},
                    {"ccx", {{"name", toolchain.CCX.Name}, {"available", toolchain.CCX.Available}}},
                    {"gmsh", {{"name", toolchain.Gmsh.Name}, {"available", toolchain.Gmsh.Available}}},
                    {"coreSolverReady", toolchain.HasCoreSolverStack()},
                };

                nlohmann::json examples = nlohmann::json::array();
                if (!root.empty()) {
                    for (const auto& ex : SZM::FEA::CalculiXPipeline::CatalogExamples(root)) {
                        examples.push_back({
                            {"name", ex.Name},
                            {"category", ex.Category},
                            {"summary", ex.Summary},
                            {"directory", ex.Directory.string()},
                            {"hasTestScript", ex.HasTestScript()},
                            {"inpCount", ex.SolverInputFiles.size()},
                        });
                    }
                }
                j["examples"] = examples;
                j["exampleCount"] = examples.size();
                res.set_content(j.dump(2), "application/json");
            });

            // ── Engineering materials (MaterialDatabase) ───────────────
            g_Server->Get("/api/materials", [](const httplib::Request&, httplib::Response& res) {
                auto& db = Materials::MaterialDatabase::GetInstance();
                nlohmann::json mats = nlohmann::json::array();
                for (const auto& m : db.GetAllMaterials()) {
                    nlohmann::json row = {
                        {"id", m.id},
                        {"name", m.name},
                        {"youngsModulus_GPa", m.youngsModulus},
                        {"poissonsRatio", m.poissonsRatio},
                        {"yieldStrength_MPa", m.yieldStrength},
                        {"ultimateStrength_MPa", m.ultimateStrength},
                        {"density_kg_m3", m.density},
                        {"thermalExpansion_1_K", m.thermalExpansion},
                        {"thermalConductivity_W_mK", m.thermalConductivity},
                    };
                    if (!m.category.empty()) {
                        row["category"] = m.category;
                    }
                    if (!m.tags.empty()) {
                        row["tags"] = m.tags;
                    }
                    if (!m.notes.empty()) {
                        row["notes"] = m.notes;
                    }
                    mats.push_back(std::move(row));
                }
                res.set_content(nlohmann::json{{"ok", true}, {"materials", mats}}.dump(2),
                                "application/json");
            });

            // ── Knowledge base ───────────────────────────────────────
            g_Server->Get("/api/kb/domains", [](const httplib::Request&, httplib::Response& res) {
                auto& kb = Knowledge::KnowledgeBaseService::GetInstance();
                kb.Load();
                res.set_content(
                    nlohmann::json{{"ok", true}, {"domains", kb.ListDomains()}}.dump(2),
                    "application/json");
            });

            g_Server->Get("/api/kb/search", [](const httplib::Request& req, httplib::Response& res) {
                const std::string q      = req.has_param("q") ? req.get_param_value("q") : "";
                const std::string domain = req.has_param("domain") ? req.get_param_value("domain") : "";
                auto& kb = Knowledge::KnowledgeBaseService::GetInstance();
                kb.Load();
                res.set_content(kb.Search(q, domain).dump(2), "application/json");
            });

            g_Server->Get(R"(/api/kb/([^/]+))", [](const httplib::Request& req, httplib::Response& res) {
                if (req.matches.size() < 2) {
                    res.status = 400;
                    return;
                }
                const std::string domain = req.matches[1];
                auto& kb = Knowledge::KnowledgeBaseService::GetInstance();
                kb.Load();
                res.set_content(kb.GetDomain(domain).dump(2), "application/json");
            });

            g_Server->Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
                res.status = 204;
            });

            // ── GET /api/simulation/status ────────────────────────────
            g_Server->Get("/api/simulation/status", [](const httplib::Request&, httplib::Response& res) {
                std::lock_guard<std::mutex> lock(g_Snapshot.mu);
                nlohmann::json j;
                j["simTime"] = g_Snapshot.simTime;
                j["componentCount"] = g_Snapshot.components.size();

                nlohmann::json comps = nlohmann::json::array();
                for (const auto& c : g_Snapshot.components) {
                    comps.push_back({
                        {"name",        c.name},
                        {"stress_MPa",  c.stress},
                        {"temperature_C", c.temperature},
                        {"stressRatio", c.stressRatio},
                        {"tempRatio",   c.tempRatio},
                        {"status",      c.status}
                    });
                }
                j["components"] = comps;
                res.set_content(j.dump(2), "application/json");
            });

            g_Server->Post("/api/simulation/submit", [this, &postJson](const httplib::Request& req, httplib::Response& res) {
                postJson(req, res, [this](const nlohmann::json& body) {
                    SimulationAPI::JobRequest request;
                    request.componentId = body.value("componentId", 0u);
                    request.simulationType = body.value("simulationType", std::string("stress"));
                    request.priority = body.value("priority", std::string("normal"));

                    if (body.contains("parameters") && body["parameters"].is_array()) {
                        for (const auto& value : body["parameters"]) {
                            request.parameters.push_back(value.get<float>());
                        }
                    }

                    const std::string jobId = m_SimulationAPI.SubmitJob(request);
                    return nlohmann::json{{"ok", !jobId.empty()}, {"jobId", jobId}};
                });
            });

            g_Server->Get(R"(/api/simulation/job/(.*))", [this](const httplib::Request& req, httplib::Response& res) {
                const std::string jobId = req.matches.size() > 1 ? req.matches[1] : std::string();
                if (jobId.empty()) {
                    res.status = 400;
                    res.set_content(R"({"ok":false,"error":"Missing jobId"})", "application/json");
                    return;
                }
                const auto status = m_SimulationAPI.GetJobStatus(jobId);
                res.set_content(nlohmann::json{
                    {"ok", status.status != "error"},
                    {"jobId", status.jobId},
                    {"status", status.status},
                    {"progress", status.progress},
                    {"errorMessage", status.errorMessage}
                }.dump(2), "application/json");
            });

            g_Server->Post(R"(/api/simulation/job/(.*)/cancel)", [this](const httplib::Request& req, httplib::Response& res) {
                const std::string jobId = req.matches.size() > 1 ? req.matches[1] : std::string();
                if (jobId.empty()) {
                    res.status = 400;
                    res.set_content(R"({"ok":false,"error":"Missing jobId"})", "application/json");
                    return;
                }
                const bool cancelled = m_SimulationAPI.CancelJob(jobId);
                res.set_content(nlohmann::json{{"ok", cancelled}, {"jobId", jobId}}.dump(2), "application/json");
            });

            g_Server->Get(R"(/api/simulation/job/(.*)/result)", [this](const httplib::Request& req, httplib::Response& res) {
                const std::string jobId = req.matches.size() > 1 ? req.matches[1] : std::string();
                if (jobId.empty()) {
                    res.status = 400;
                    res.set_content(R"({"ok":false,"error":"Missing jobId"})", "application/json");
                    return;
                }
                const std::string resultPayload = m_SimulationAPI.GetJobResult(jobId);
                res.set_content(resultPayload, "application/json");
            });

            // ── GET /api/scene.json (WebViewer) ──────────────────────
            g_Server->Get("/api/scene.json", [](const httplib::Request&, httplib::Response& res) {
                std::lock_guard<std::mutex> lock(g_Snapshot.mu);
                nlohmann::json scene = SZM::WebViewer::ThreeJSExporter::ExportScene(g_Snapshot.components);
                res.set_content(scene.dump(2), "application/json");
            });

            g_Server->Get("/api/simulation/jobs", [&simApi](const httplib::Request& req, httplib::Response& res) {
                const std::string jobId = req.get_param_value("jobId");
                if (jobId.empty()) {
                    res.set_content(nlohmann::json{{"ok", true}, {"jobs", nlohmann::json::array()}}.dump(2), "application/json");
                    return;
                }

                auto status = simApi.GetJobStatus(jobId);
                res.set_content(nlohmann::json{{"ok", true}, {"job", {
                    {"jobId", status.jobId},
                    {"status", status.status},
                    {"progress", status.progress},
                    {"errorMessage", status.errorMessage}
                }}}.dump(2), "application/json");
            });

            g_Server->Post("/api/simulation/submit", [&simApi](const httplib::Request& req, httplib::Response& res) {
                try {
                    const auto body = req.body.empty() ? nlohmann::json::object() : nlohmann::json::parse(req.body);
                    SimulationAPI::JobRequest request;
                    request.componentId = body.value("componentId", 0U);
                    request.simulationType = body.value("simulationType", "stress");
                    request.priority = body.value("priority", "normal");
                    if (body.contains("parameters") && body["parameters"].is_array()) {
                        for (const auto& item : body["parameters"]) {
                            if (item.is_number()) {
                                request.parameters.push_back(item.get<float>());
                            }
                        }
                    }

                    const std::string jobId = simApi.SubmitJob(request);
                    if (jobId.empty()) {
                        res.status = 500;
                        res.set_content(R"({"ok":false,"error":"Unable to queue simulation job"})", "application/json");
                        return;
                    }

                    res.set_content(nlohmann::json{{"ok", true}, {"jobId", jobId}}.dump(2), "application/json");
                } catch (const std::exception& e) {
                    res.status = 400;
                    res.set_content(nlohmann::json{{"ok", false}, {"error", e.what()}}.dump(2), "application/json");
                }
            });

            g_Server->Post("/api/simulation/cancel", [&simApi](const httplib::Request& req, httplib::Response& res) {
                try {
                    const auto body = req.body.empty() ? nlohmann::json::object() : nlohmann::json::parse(req.body);
                    const std::string jobId = body.value("jobId", "");
                    const bool cancelled = simApi.CancelJob(jobId);
                    res.set_content(nlohmann::json{{"ok", cancelled}, {"cancelled", cancelled}}.dump(2), "application/json");
                } catch (const std::exception& e) {
                    res.status = 400;
                    res.set_content(nlohmann::json{{"ok", false}, {"error", e.what()}}.dump(2), "application/json");
                }
            });

            g_Server->Get("/api/simulation/result", [&simApi](const httplib::Request& req, httplib::Response& res) {
                const std::string jobId = req.get_param_value("jobId");
                const std::string result = simApi.GetJobResult(jobId);
                if (result.empty()) {
                    res.status = 404;
                    res.set_content(R"({"ok":false,"error":"No result available"})", "application/json");
                    return;
                }
                res.set_content(result, "application/json");
            });

            // ── Serve React UI ───────────────────────────────────────
            g_Server->set_mount_point("/", uiDistPath);
            g_Server->Get("/", [uiDistPath](const httplib::Request&, httplib::Response& res) {
                std::string indexPath = uiDistPath + "/index.html";
                std::ifstream f(indexPath, std::ios::binary);
                if (f) {
                    std::ostringstream ss;
                    ss << f.rdbuf();
                    res.set_content(ss.str(), "text/html");
                } else {
                    res.status = 404;
                    res.set_content("React UI not found. Please run 'npm run build' in 'new ui'.", "text/plain");
                }
            });

            // Start server in background thread and wait for a healthy response
            m_Port = basePort;
            g_ServerThread = std::make_unique<std::thread>([this]() {
                std::cout << "[APIManager] HTTP Server listening on port " << m_Port << "\n";
                g_Server->listen("0.0.0.0", m_Port);
            });

            const auto waitUntilReady = [&]() {
                using clock = std::chrono::steady_clock;
                auto deadline = clock::now() + std::chrono::seconds(3);
                while (clock::now() < deadline) {
                    httplib::Client client("127.0.0.1", m_Port);
                    client.set_connection_timeout(0, 250000);
                    client.set_read_timeout(0, 250000);
                    if (auto res = client.Get("/api/status"); res && res->status == 200) {
                        return true;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                return false;
            };

            if (!waitUntilReady()) {
                m_IsReady = false;
                std::cerr << "[APIManager] Server failed to respond on port " << m_Port << "\n";
                if (g_Server) {
                    g_Server->stop();
                }
                return false;
            }

            m_IsReady = true;
            std::cout << "[APIManager] Server started successfully\n";
            std::cout << "[APIManager] Endpoints:\n";
            std::cout << "  GET  /api/status\n";
            std::cout << "  GET  /api/assembly\n";
            std::cout << "  GET  /api/project\n";
            std::cout << "  POST /api/project/new\n";
            std::cout << "  POST /api/project/table\n";
            std::cout << "  POST /api/project/limits\n";
            std::cout << "  POST /api/ai/improve\n";
            std::cout << "  POST /api/workshop/bootstrap\n";
            std::cout << "  GET  /api/simulation/status\n";
            std::cout << "  GET  /api/scene.json\n";
            std::cout << "  POST /api/structural/solve\n";
            std::cout << "  POST /api/simulation/fea/run\n";
            std::cout << "  POST /api/freecad/parametric\n";
            std::cout << "  POST /api/step/assembly/parse\n";
            std::cout << "  POST /api/sketch/solve\n";
            std::cout << "  POST /api/sketch/extrude\n";
            std::cout << "  POST /api/brep/pick\n";
            std::cout << "  POST /api/csg/boolean\n";
            std::cout << "  GET  /api/machine/config\n";
            std::cout << "  POST /api/machine/config\n";
            std::cout << "  GET  /api/code8/catalog\n";
            std::cout << "  GET  /api/materials\n";
            std::cout << "  GET  /api/kb/domains\n";
            std::cout << "  GET  /api/kb/{domain}\n";
            std::cout << "  GET  /api/kb/search?q=\n";
            return true;
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "[APIManager] Initialization failed: " << e.what() << "\n";
        m_IsReady = false;
        return false;
    }
}

void APIManager::Shutdown() {
    std::lock_guard<std::mutex> lock(g_ServerMutex);
    try {
        if (g_Server) {
            std::cout << "[APIManager] Stopping HTTP server...\n";
            g_Server->stop();
            if (g_ServerThread && g_ServerThread->joinable()) {
                g_ServerThread->join();
            }
            g_Server.reset();
            g_ServerThread.reset();
        }
        m_IsReady = false;
        std::cout << "[APIManager] Server stopped\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[APIManager] Shutdown error: " << e.what() << "\n";
    }
}

std::string APIManager::GetDocumentationURL() const {
    return "http://localhost:" + std::to_string(m_Port) + "/docs";
}

} // namespace SZM::WebAPI
