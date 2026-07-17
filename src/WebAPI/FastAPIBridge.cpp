#include "FastAPIBridge.hpp"
#include "UuidUtils.hpp"
#include <iostream>
#include <sstream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <memory>
#include <mutex>
#include <map>

#include "../WebViewer/ExportToThreeJS.hpp"
#include "../FEA/CalculiXPipeline.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include "StructuralSolveService.hpp"
#include "TableProjectService.hpp"

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
    if (m_IsRunning) {
        Stop();
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
        m_IsRunning = false;
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

        m_QueuedJobs++;

        std::cout << "[SimulationAPI] Job submitted: " << jobId << "\n";
        std::cout << "[SimulationAPI] Type: " << request.simulationType
                  << ", Component: " << request.componentId
                  << ", Priority: " << request.priority << "\n";

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
        // Placeholder: In real implementation, query job database
        result.status = "completed";
        result.progress = 100.0f;
        result.resultJSON = "{\"status\":\"success\"}";

        std::cout << "[SimulationAPI] Status query for job: " << jobId << "\n";
        return result;
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
        std::cout << "[SimulationAPI] Cancelling job: " << jobId << "\n";
        if (m_QueuedJobs > 0) m_QueuedJobs--;
        return true;
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
        std::cout << "[SimulationAPI] Retrieving result for job: " << jobId << "\n";
        return "{\"jobId\":\"" + jobId + "\",\"status\":\"completed\"}";
    }
    catch (const std::exception& e) {
        std::cerr << "[SimulationAPI] Result retrieval failed: " << e.what() << "\n";
        return "";
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

std::string ExportAPI::ExportComponentSTEP(uint32_t componentId) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        std::stringstream ss;
        ss << "component_" << componentId << ".step";
        std::cout << "[ExportAPI] Exporting component " << componentId << " to STEP\n";
        return ss.str();
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
        std::stringstream ss;
        ss << jobId << "_results.csv";
        std::cout << "[ExportAPI] Exporting results for job " << jobId << " to CSV\n";
        return ss.str();
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
        std::cout << "[ExportAPI] Exporting scene to Three.js\n";
        return "scene.json";
    }
    catch (const std::exception& e) {
        std::cerr << "[ExportAPI] Three.js export failed: " << e.what() << "\n";
        return "";
    }
}

std::string ExportAPI::ExportReportPDF(const std::string& jobId) {
    if (!m_IsRunning) {
        return "";
    }

    try {
        std::stringstream ss;
        ss << jobId << "_report.pdf";
        std::cout << "[ExportAPI] Exporting report for job " << jobId << " to PDF\n";
        return ss.str();
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

void APIManager::PushComponentSnapshot(const std::string& name,
                                        float stress, float temperature,
                                        float stressRatio, float tempRatio,
                                        double simTime) {
    std::lock_guard<std::mutex> lock(g_Snapshot.mu);
    g_Snapshot.simTime = simTime;
    // Upsert component
    for (auto& c : g_Snapshot.components) {
        if (c.name == name) {
            c.stress = stress;
            c.temperature = temperature;
            c.stressRatio = stressRatio;
            c.tempRatio = tempRatio;
            c.status = (stressRatio > 0.9f || tempRatio > 0.9f) ? "FAIL" : "SAFE";
            return;
        }
    }
    g_Snapshot.components.push_back({name, stress, temperature, stressRatio, tempRatio,
                                     (stressRatio > 0.9f || tempRatio > 0.9f) ? "FAIL" : "SAFE"});
}

bool APIManager::Initialize(uint16_t basePort) {
    std::lock_guard<std::mutex> lock(g_ServerMutex);
    try {
        std::cout << "[APIManager] Initializing embedded HTTP server...\n";

        if (!g_Server) {
            g_Server = std::make_unique<httplib::Server>();

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
                res.set_content(handler(body).dump(2), "application/json");
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

            // ── Engineering materials (codes/ MaterialDatabase) ───────
            g_Server->Get("/api/materials", [](const httplib::Request&, httplib::Response& res) {
                auto& db = Materials::MaterialDatabase::GetInstance();
                nlohmann::json mats = nlohmann::json::array();
                for (const auto& m : db.GetAllMaterials()) {
                    mats.push_back({
                        {"id", m.id},
                        {"name", m.name},
                        {"youngsModulus_GPa", m.youngsModulus},
                        {"yieldStrength_MPa", m.yieldStrength},
                        {"density_kg_m3", m.density},
                    });
                }
                res.set_content(nlohmann::json{{"ok", true}, {"materials", mats}}.dump(2),
                                "application/json");
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

            // ── GET /api/scene.json (WebViewer) ──────────────────────
            g_Server->Get("/api/scene.json", [](const httplib::Request&, httplib::Response& res) {
                std::lock_guard<std::mutex> lock(g_Snapshot.mu);
                nlohmann::json scene = SZM::WebViewer::ThreeJSExporter::ExportScene(g_Snapshot.components);
                res.set_content(scene.dump(2), "application/json");
            });

            // ── Serve React UI ───────────────────────────────────────
            g_Server->set_mount_point("/", "new ui/dist");

            // Start server in background thread
            g_ServerThread = std::make_unique<std::thread>([basePort]() {
                std::cout << "[APIManager] HTTP Server listening on port " << basePort << "\n";
                g_Server->listen("0.0.0.0", basePort);
            });

            // Give the listener a moment to bind
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
            std::cout << "  GET  /api/code8/catalog\n";
            std::cout << "  GET  /api/materials\n";
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
    return "http://localhost:8000/docs";
}

} // namespace SZM::WebAPI
