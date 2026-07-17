#include "Application.hpp"
#include "EventBus.hpp"
#include "../Input/InputManager.hpp"
#include "../Input/Event.hpp"
#include "../Input/InputEvents.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include "../Physics/PhysicsEngine.hpp"
#include "../Graphics/GraphicsContext.hpp"
#include "../WebViewer/ReactUIManager.hpp"
#include "../UI/panels/ConsolePanel.hpp"
#include "../UI/panels/SceneOutliner.hpp"
#include "../UI/panels/PropertiesPanel.hpp"
#include "../UI/panels/SimulationPanel.hpp"
#include "../UI/panels/SimulationVisualizerPanel.hpp"
#include "../UI/panels/ComponentBuilderPanel.hpp"
#include "../UI/panels/ToolbarPanel.hpp"
#include "../UI/panels/DashboardPanel.hpp"
#include "../Workshop3D/WorkshopManager.hpp"
#include "../Workshop3D/WorkshopPanels.hpp"
#include "../Workshop3D/ComponentBrowserPanel.hpp"
#include "../Workshop3D/ComponentWeightTablePanel.hpp"
#include "../UI/UITooltipHelper.hpp"
#include "../WebAPI/FastAPIBridge.hpp"
#include "../WebAPI/TableProjectService.hpp"
#include "../AI/AIEngine.hpp"
#include "../Thermal/ThermalSolver.hpp"
#include "../Electrical/CircuitSolver.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#define WEBVIEW_GTK
#define WEBVIEW_IMPLEMENTATION
#include "../WebViewer/webview.h"

namespace {

constexpr float kPi = 3.14159265358979323846f;

float Clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float SmoothStep(float edge0, float edge1, float x) {
    if (edge0 >= edge1) {
        return x >= edge1 ? 1.0f : 0.0f;
    }
    const float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

bool ReadPPMToken(std::ifstream& input, std::string& outToken) {
    outToken.clear();
    while (input >> outToken) {
        if (!outToken.empty() && outToken[0] == '#') {
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        return true;
    }
    return false;
}

} // namespace

namespace SZM {

Application::Application() = default;
Application::~Application() = default;

// ------------------------------------------------------------------ Init ---
void Application::Init(const WindowConfig& config) {
    if (!glfwInit()) {
        throw std::runtime_error("SZM_FATAL: Failed to initialize GLFW.");
    }

    // DPI awareness across monitors
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    m_UseVulkan = config.UseVulkan;
    if (m_UseVulkan) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    } else {
        // Forward-compatible OpenGL core profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // MSAA 4× for CAD line smoothing
        glfwWindowHint(GLFW_SAMPLES, 4);
    }
    
    // Remove the OS title bar to save vertical space
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    m_WindowHandle = glfwCreateWindow(
        config.Width, config.Height,
        config.Title.c_str(),
        nullptr, nullptr
    );

    if (!m_WindowHandle) {
        glfwTerminate();
        throw std::runtime_error("SZM_FATAL: OS denied window creation.");
    }

    if (!m_UseVulkan) {
        glfwMakeContextCurrent(m_WindowHandle);
        if (!gladLoadGL(glfwGetProcAddress)) {
            glfwDestroyWindow(m_WindowHandle);
            m_WindowHandle = nullptr;
            glfwTerminate();
            throw std::runtime_error("SZM_FATAL: Failed to load OpenGL symbols via GLAD.");
        }
        glfwSwapInterval(config.VSync ? 1 : 0);
    }

    // Register this instance as the GLFW user pointer
    glfwSetWindowUserPointer(m_WindowHandle, this);

    // --- Graphics Context (Module 010.4) ---------------------------------
    Graphics::ContextConfig graphicsConfig{};
    graphicsConfig.PreferredAPI = m_UseVulkan
        ? Graphics::API::Vulkan
        : Graphics::API::OpenGL;
    graphicsConfig.MSAASamples = 4;
    graphicsConfig.DepthBits = 24;
    graphicsConfig.DoubleBuffering = true;

    m_GraphicsContext = Graphics::ContextFactory::Create(m_WindowHandle, graphicsConfig);
    m_GraphicsContext->Init();

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(m_WindowHandle, &framebufferWidth, &framebufferHeight);
    if (framebufferWidth > 0 && framebufferHeight > 0) {
        m_GraphicsContext->OnResize(
            static_cast<uint32_t>(framebufferWidth),
            static_cast<uint32_t>(framebufferHeight)
        );
    }

    // --- Wire OS callbacks to our event system -------------------------
    glfwSetFramebufferSizeCallback(m_WindowHandle, FramebufferResizeCallback);
    glfwSetKeyCallback(m_WindowHandle, KeyCallback);
    glfwSetCursorPosCallback(m_WindowHandle, MouseMoveCallback);
    glfwSetWindowFocusCallback(m_WindowHandle, FocusCallback);

    // --- Initialise sub-systems ----------------------------------------
    InputManager::Init();
    SimulationEngine::GetInstance().Init();
    Physics::PhysicsEngine::GetInstance().Init();
    m_Phase5PhysicsIntegration.Initialize();
    Workshop3D::WorkshopManager::GetInstance().Init();

    WebAPI::APIManager::GetInstance().Initialize(8000);
    AI::AIEngine::GetInstance().Init();
    Thermal::ThermalSolver::GetInstance().Init();
    Electrical::CircuitSolver::GetInstance().Init();


    // --- New UI (new ui/) — Theme 1 Classic + Theme 2 Modern with 3D visualization ---
#if defined(SZM_USE_REACT_WEBVIEW)
    std::cout << "[SZM Forge] Initializing new React UI (new ui/)...\n";
    WebViewer::ReactUIManager& reactUI = WebViewer::ReactUIManager::GetInstance();
    std::string reactUIPath = config.ThemePath.empty() ? std::string(SZM_REACT_UI_PATH) : config.ThemePath;

    if (reactUI.Initialize(reactUIPath)) {
        if (reactUI.Build()) {
            std::cout << "[SZM Forge] React UI built successfully\n";
            m_UseWebview = true;
            m_WebviewUrl = "http://127.0.0.1:8000/";
        } else {
            std::cout << "[SZM Forge] React UI build failed, starting dev server...\n";
            if (reactUI.StartDevServer()) {
                m_UseWebview = true;
                m_WebviewUrl = reactUI.GetUIURL();
            }
        }
    } else {
        std::cout << "[SZM Forge] React UI initialization failed, using native workshop UI\n";
    }
#else
    std::cout << "[SZM Forge] Native 3D workshop UI enabled (ImGui + Workshop3D).\n";
#endif

    if (const char* forceNative = std::getenv("SZM_NATIVE_WORKSHOP_UI")) {
        if (forceNative[0] == '1' || forceNative[0] == 'y' || forceNative[0] == 'Y') {
            m_UseWebview = false;
            std::cout << "[SZM Forge] SZM_NATIVE_WORKSHOP_UI set — using native workshop UI.\n";
        }
    }

    if (m_UseWebview) {
        glfwHideWindow(m_WindowHandle);
    } else {
        // --- Native ImGui UI with full 3D workshop -----------------------------------
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        UITooltip::ApplyTooltipStyle();
        m_UIManager.Initialize();

        auto consolePanel = std::make_shared<ConsolePanel>();
        auto sceneOutliner = std::make_shared<SceneOutliner>();
        auto propertiesPanel = std::make_shared<PropertiesPanel>();
        auto simulationPanel = std::make_shared<SimulationPanel>();
        auto simulationVisualizer = std::make_shared<SimulationVisualizerPanel>();
        auto componentBuilder = std::make_shared<ComponentBuilderPanel>();
        auto toolbarPanel = std::make_shared<ToolbarPanel>();
        auto dashboardPanel = std::make_shared<DashboardPanel>();
        auto componentBrowser = std::make_shared<Workshop3D::ComponentBrowserPanel>();
        auto componentWeightTable = std::make_shared<Workshop3D::ComponentWeightTablePanel>();
        auto workshopLibrary = std::make_shared<Workshop3D::PartLibraryPanel>();
        auto partEditor = std::make_shared<Workshop3D::PartEditorPanel>();
        auto workshopAssembly = std::make_shared<Workshop3D::AssemblyPanel>();
        auto workshopViewport = std::make_shared<Workshop3D::WorkshopViewportPanel>();
        auto workshopSimulation = std::make_shared<Workshop3D::SimulationSetupPanel>();

        m_UIManager.RegisterPanel(consolePanel);
        m_UIManager.RegisterPanel(sceneOutliner);
        m_UIManager.RegisterPanel(propertiesPanel);
        m_UIManager.RegisterPanel(simulationPanel);
        m_UIManager.RegisterPanel(simulationVisualizer);
        m_UIManager.RegisterPanel(componentBuilder);
        m_UIManager.RegisterPanel(toolbarPanel);
        m_UIManager.RegisterPanel(dashboardPanel);
        m_UIManager.RegisterPanel(componentBrowser);
        m_UIManager.RegisterPanel(componentWeightTable);
        m_UIManager.RegisterPanel(workshopLibrary);
        m_UIManager.RegisterPanel(partEditor);
        m_UIManager.RegisterPanel(workshopAssembly);
        m_UIManager.RegisterPanel(workshopViewport);
        m_UIManager.RegisterPanel(workshopSimulation);

        m_UIManager.ApplyWorkspacePreset(WorkspacePreset::Build);

        ImGui_ImplGlfw_InitForOpenGL(m_WindowHandle, true);
        ImGui_ImplOpenGL3_Init("#version 450 core");
        m_ImGuiInitialized = true;
    }

    m_ShowStartupSplash = true;
    m_StartupSplashElapsed = 0.0;
    if (!m_UseVulkan && !m_UseWebview) {
        LoadStartupLogoTexture();
    }

    m_IsRunning     = true;
    m_LastFrameTime = std::chrono::high_resolution_clock::now();

    std::cout << "[SZM Forge] Window initialised ("
              << config.Width << "×" << config.Height << ").\n";
}

// ------------------------------------------------------------------ Run ----
void Application::Run() {
    int    frameCount     = 0;
    double timeAccumulator = 0.0;
    m_LastFrameTime = std::chrono::high_resolution_clock::now();
    m_StartupSplashElapsed = 0.0;

    if (m_UseWebview) {
        std::cout << "[SZM Forge] Launching Webview: " << m_WebviewUrl << "\n";
        
        struct webview w;
        memset(&w, 0, sizeof(w));
        w.title = "SZM Forge";
        w.url = m_WebviewUrl.c_str();
        w.width = 1440;
        w.height = 900;
        w.resizable = 1;
        w.debug = 1;
        webview_init(&w);
        
        std::atomic<bool> physicsRunning{true};
        std::thread physicsThread([this, &physicsRunning]() {
            while (physicsRunning) {
                auto now = std::chrono::high_resolution_clock::now();
                m_DeltaTime = std::chrono::duration<double>(now - m_LastFrameTime).count();
                m_LastFrameTime = now;
                
                TickSystems();
                
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        });
        
        while (webview_loop(&w, 1) == 0) {
            // Loop runs until window is closed
        }
        webview_exit(&w);
        
        physicsRunning = false;
        physicsThread.join();
        return;
    }

    while (!glfwWindowShouldClose(m_WindowHandle) && m_IsRunning) {
        // 1. Delta-Time
        auto now = std::chrono::high_resolution_clock::now();
        m_DeltaTime     = std::chrono::duration<double>(now - m_LastFrameTime).count();
        m_LastFrameTime = now;

        // 2. OS Events
        glfwPollEvents();

        // 3. System Tick (Physics 200-Series, CEA AI, etc. hook here)
        TickSystems();

        // 4. ImGui + Render (OpenGL path)
        if (!m_UseVulkan && m_ImGuiInitialized) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (m_ShowStartupSplash) {
                m_StartupSplashElapsed += m_DeltaTime;
                if (m_StartupSplashElapsed < m_StartupSplashDuration) {
                    RenderStartupSplash();
                } else {
                    m_ShowStartupSplash = false;
                }
            }
            if (!m_ShowStartupSplash) {
                m_UIManager.RenderUI();
            }
            ImGui::Render();

            int display_w = 0;
            int display_h = 0;
            glfwGetFramebufferSize(m_WindowHandle, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
        }

        if (m_GraphicsContext) {
            m_GraphicsContext->SwapBuffers();
        } else if (!m_UseVulkan) {
            glfwSwapBuffers(m_WindowHandle);
        }

        // 6. FPS Diagnostic
        ++frameCount;
        timeAccumulator += m_DeltaTime;
        if (timeAccumulator >= 1.0) {
            std::cout << "[SZM Core] FPS: " << frameCount
                      << " | dt: " << m_DeltaTime << "s\n";
            frameCount      = 0;
            timeAccumulator = 0.0;
        }
    }
}

// --------------------------------------------------------------- Shutdown --
void Application::Shutdown() {
    m_Phase5PhysicsIntegration.Shutdown();
    Workshop3D::WorkshopManager::GetInstance().Shutdown();
    WebAPI::APIManager::GetInstance().Shutdown();

    // --- Shutdown React UI ---------------------------------------------
    WebViewer::ReactUIManager& reactUI = WebViewer::ReactUIManager::GetInstance();
    if (reactUI.IsAvailable()) {
        reactUI.StopDevServer();
    }

    if (!m_UseVulkan) {
        ReleaseStartupLogoTexture();
    }

    if (m_ImGuiInitialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_ImGuiInitialized = false;
    }

    m_GraphicsContext.reset();

    if (m_WindowHandle) {
        glfwDestroyWindow(m_WindowHandle);
        m_WindowHandle = nullptr;
    }
    glfwTerminate();
    std::cout << "[SZM Forge] Terminated safely.\n";
}

// --------------------------------------------------------- TickSystems ----
void Application::TickSystems() {
    SimulationEngine::GetInstance().Tick(m_DeltaTime);
    m_Phase5PhysicsIntegration.PrePhysicsStep(m_DeltaTime);
    Physics::PhysicsEngine::GetInstance().Tick(m_DeltaTime);
    m_Phase5PhysicsIntegration.PostPhysicsStep();

    auto& engine = SimulationEngine::GetInstance();
    static double apiSimTime = 0.0;
    apiSimTime += m_DeltaTime;
    for (const auto& compPtr : engine.GetComponents()) {
        if (!compPtr) {
            continue;
        }
        const auto& comp = *compPtr;
        WebAPI::APIManager::GetInstance().PushComponentSnapshot(
            comp.name,
            comp.stress / 1e6f,
            comp.temperature - 273.15f,
            comp.stressRatio,
            comp.tempRatio,
            apiSimTime
        );
    }
    Thermal::ThermalSolver::GetInstance().SolveTransient(m_DeltaTime);
    Electrical::CircuitSolver::GetInstance().SolveTransient(m_DeltaTime);
    // TODO (AI):         CEAAssistant::Update();
}

bool Application::LoadStartupLogoTexture() {
    ReleaseStartupLogoTexture();

    const std::vector<std::filesystem::path> candidates = {
        "assets/splash/szm_forge_logo.ppm",
        "../assets/splash/szm_forge_logo.ppm",
        "../../assets/splash/szm_forge_logo.ppm"
    };

    std::filesystem::path logoPath;
    for (const auto& candidate : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(candidate, ec)) {
            logoPath = candidate;
            break;
        }
    }

    if (logoPath.empty()) {
        std::cerr << "[SZM Splash] Logo image not found (PPM expected at assets/splash/szm_forge_logo.ppm).\n";
        return false;
    }

    std::ifstream input(logoPath, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "[SZM Splash] Failed to open logo file: " << logoPath << '\n';
        return false;
    }

    std::string magic;
    std::string widthToken;
    std::string heightToken;
    std::string maxToken;
    if (!ReadPPMToken(input, magic) ||
        !ReadPPMToken(input, widthToken) ||
        !ReadPPMToken(input, heightToken) ||
        !ReadPPMToken(input, maxToken)) {
        std::cerr << "[SZM Splash] Invalid PPM header in " << logoPath << '\n';
        return false;
    }

    if (magic != "P6") {
        std::cerr << "[SZM Splash] Unsupported image format (expected P6 PPM): " << logoPath << '\n';
        return false;
    }

    int width = 0;
    int height = 0;
    int maxValue = 0;
    try {
        width = std::stoi(widthToken);
        height = std::stoi(heightToken);
        maxValue = std::stoi(maxToken);
    } catch (const std::exception&) {
        std::cerr << "[SZM Splash] Corrupted numeric fields in PPM header: " << logoPath << '\n';
        return false;
    }

    if (width <= 0 || height <= 0 || maxValue != 255) {
        std::cerr << "[SZM Splash] Invalid PPM dimensions or depth in " << logoPath << '\n';
        return false;
    }

    const int separator = input.peek();
    if (separator == std::char_traits<char>::eof() ||
        !std::isspace(static_cast<unsigned char>(separator))) {
        std::cerr << "[SZM Splash] Missing binary payload separator in " << logoPath << '\n';
        return false;
    }
    input.get();
    if (separator == '\r' && input.peek() == '\n') {
        input.get();
    }

    const std::size_t bytesToRead = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 3U;
    std::vector<unsigned char> pixels(bytesToRead);
    input.read(reinterpret_cast<char*>(pixels.data()), static_cast<std::streamsize>(bytesToRead));
    if (input.gcount() != static_cast<std::streamsize>(bytesToRead)) {
        std::cerr << "[SZM Splash] Unexpected end of file while reading " << logoPath << '\n';
        return false;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (texture == 0) {
        std::cerr << "[SZM Splash] OpenGL texture allocation failed.\n";
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        pixels.data()
    );
    glBindTexture(GL_TEXTURE_2D, 0);

    if (glGetError() != GL_NO_ERROR) {
        glDeleteTextures(1, &texture);
        std::cerr << "[SZM Splash] OpenGL upload failed for logo texture.\n";
        return false;
    }

    m_StartupLogoTexture = texture;
    m_StartupLogoWidth = width;
    m_StartupLogoHeight = height;
    std::cout << "[SZM Splash] Loaded startup logo (" << width << "x" << height << ").\n";
    return true;
}

void Application::ReleaseStartupLogoTexture() {
    if (m_StartupLogoTexture != 0) {
        glDeleteTextures(1, &m_StartupLogoTexture);
        m_StartupLogoTexture = 0;
    }
    m_StartupLogoWidth = 0;
    m_StartupLogoHeight = 0;
}

void Application::RenderStartupSplash() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (!viewport) {
        return;
    }

    const ImVec2 viewportPos = viewport->Pos;
    const ImVec2 viewportSize = viewport->Size;
    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) {
        return;
    }

    const float progress = Clamp01(static_cast<float>(m_StartupSplashElapsed / m_StartupSplashDuration));
    const float fadeOut = SmoothStep(0.76f, 1.0f, progress);
    const float alpha = 1.0f - fadeOut;
    if (alpha <= 0.0f) {
        return;
    }

    ImDrawList* drawList = ImGui::GetForegroundDrawList(viewport);
    const ImVec2 viewportMax(viewportPos.x + viewportSize.x, viewportPos.y + viewportSize.y);
    const ImVec2 center(viewportPos.x + viewportSize.x * 0.5f, viewportPos.y + viewportSize.y * 0.5f);
    const float elapsed = static_cast<float>(m_StartupSplashElapsed);

    // Base matte background and a warm core glow to match the mechanical logo.
    drawList->AddRectFilled(
        viewportPos,
        viewportMax,
        ImGui::GetColorU32(ImVec4(0.02f, 0.02f, 0.03f, 0.98f * alpha))
    );

    const float atmosphereRadius = std::min(viewportSize.x, viewportSize.y) * 0.66f;
    for (int i = 0; i < 7; ++i) {
        const float t = static_cast<float>(i) / 6.0f;
        const float ringRadius = atmosphereRadius * (1.0f - t * 0.12f);
        const float ringAlpha = (0.12f - t * 0.012f) * alpha;
        drawList->AddCircleFilled(
            center,
            ringRadius,
            ImGui::GetColorU32(ImVec4(0.95f, 0.64f, 0.22f, ringAlpha)),
            120
        );
    }

    const float outerRadius = std::min(viewportSize.x, viewportSize.y) * 0.31f;
    const float outerInner = outerRadius - 18.0f;
    const float outerSpin = elapsed * 1.25f;
    for (int tooth = 0; tooth < 56; ++tooth) {
        const float toothAngle = outerSpin + (static_cast<float>(tooth) * (2.0f * kPi / 56.0f));
        const float toothWeight = (tooth % 4 == 0) ? 2.5f : 1.4f;
        const float toothAlpha = (tooth % 4 == 0) ? 0.45f : 0.22f;
        const ImVec2 innerPoint(
            center.x + std::cos(toothAngle) * outerInner,
            center.y + std::sin(toothAngle) * outerInner
        );
        const ImVec2 outerPoint(
            center.x + std::cos(toothAngle) * outerRadius,
            center.y + std::sin(toothAngle) * outerRadius
        );
        drawList->AddLine(
            innerPoint,
            outerPoint,
            ImGui::GetColorU32(ImVec4(0.90f, 0.72f, 0.33f, toothAlpha * alpha)),
            toothWeight
        );
    }
    drawList->AddCircle(
        center,
        outerRadius,
        ImGui::GetColorU32(ImVec4(0.78f, 0.62f, 0.30f, 0.52f * alpha)),
        160,
        2.0f
    );
    drawList->AddCircle(
        center,
        outerInner - 4.0f,
        ImGui::GetColorU32(ImVec4(0.50f, 0.41f, 0.24f, 0.40f * alpha)),
        128,
        1.0f
    );

    const float innerRadius = outerRadius * 0.70f;
    const float innerSpin = -elapsed * 1.95f;
    for (int segment = 0; segment < 24; ++segment) {
        const float segmentStart = innerSpin + static_cast<float>(segment) * (2.0f * kPi / 24.0f);
        const float segmentEnd = segmentStart + 0.14f;
        const ImVec2 p0(
            center.x + std::cos(segmentStart) * (innerRadius - 7.0f),
            center.y + std::sin(segmentStart) * (innerRadius - 7.0f)
        );
        const ImVec2 p1(
            center.x + std::cos(segmentEnd) * innerRadius,
            center.y + std::sin(segmentEnd) * innerRadius
        );
        drawList->AddLine(
            p0,
            p1,
            ImGui::GetColorU32(ImVec4(0.62f, 0.50f, 0.28f, 0.36f * alpha)),
            2.0f
        );
    }

    const float sparkSweep = std::fmod(elapsed * 0.85f, 1.0f);
    for (int spark = 0; spark < 3; ++spark) {
        const float sparkAngle = (sparkSweep + (static_cast<float>(spark) / 3.0f)) * 2.0f * kPi;
        const ImVec2 sparkCenter(
            center.x + std::cos(sparkAngle) * (innerRadius + 9.0f),
            center.y + std::sin(sparkAngle) * (innerRadius + 9.0f)
        );
        drawList->AddCircleFilled(
            sparkCenter,
            3.3f,
            ImGui::GetColorU32(ImVec4(1.0f, 0.80f, 0.32f, 0.90f * alpha)),
            16
        );
    }

    const float logoReveal = SmoothStep(0.0f, 0.22f, progress);
    const float logoPulse = 1.0f + 0.018f * std::sin(elapsed * 7.3f);
    float logoAspect = 1.0f;
    if (m_StartupLogoTexture != 0 && m_StartupLogoWidth > 0 && m_StartupLogoHeight > 0) {
        logoAspect = static_cast<float>(m_StartupLogoHeight) / static_cast<float>(m_StartupLogoWidth);
    }
    const float logoBaseWidth = std::min(viewportSize.x * 0.44f, 640.0f);
    const float logoWidth = logoBaseWidth * (0.80f + 0.20f * logoReveal) * logoPulse;
    const float logoHeight = logoWidth * logoAspect;
    const float logoAlpha = logoReveal * alpha;

    drawList->AddCircleFilled(
        center,
        std::max(logoWidth, logoHeight) * 0.44f,
        ImGui::GetColorU32(ImVec4(0.98f, 0.68f, 0.18f, 0.24f * logoAlpha)),
        120
    );

    if (m_StartupLogoTexture != 0) {
        const ImVec2 logoMin(center.x - logoWidth * 0.5f, center.y - logoHeight * 0.5f);
        const ImVec2 logoMax(center.x + logoWidth * 0.5f, center.y + logoHeight * 0.5f);
        drawList->AddImage(
            static_cast<ImTextureID>(static_cast<ImU64>(m_StartupLogoTexture)),
            logoMin,
            logoMax,
            ImVec2(0.0f, 0.0f),
            ImVec2(1.0f, 1.0f),
            ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, logoAlpha))
        );
    } else {
        const char* fallback = "SZM FORGE";
        const ImVec2 textSize = ImGui::CalcTextSize(fallback);
        drawList->AddText(
            ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f),
            ImGui::GetColorU32(ImVec4(0.96f, 0.77f, 0.33f, logoAlpha)),
            fallback
        );
    }

    const float textAlpha = SmoothStep(0.18f, 0.50f, progress) * alpha;
    if (textAlpha > 0.01f) {
        const int dots = static_cast<int>(elapsed * 3.0f) % 4;
        std::string status = "Booting mechanical core";
        status.append(static_cast<std::size_t>(dots), '.');

        const ImVec2 statusSize = ImGui::CalcTextSize(status.c_str());
        const ImVec2 statusPos(
            center.x - statusSize.x * 0.5f,
            center.y + (logoHeight * 0.56f) + 24.0f
        );
        drawList->AddText(
            statusPos,
            ImGui::GetColorU32(ImVec4(0.92f, 0.80f, 0.56f, textAlpha)),
            status.c_str()
        );

        const float progressBarFill = SmoothStep(0.0f, 0.92f, progress);
        const float barWidth = std::min(viewportSize.x * 0.38f, 420.0f);
        const ImVec2 barMin(center.x - barWidth * 0.5f, statusPos.y + statusSize.y + 16.0f);
        const ImVec2 barMax(barMin.x + barWidth, barMin.y + 8.0f);
        const ImVec2 fillMax(barMin.x + barWidth * progressBarFill, barMax.y);

        drawList->AddRectFilled(
            barMin,
            barMax,
            ImGui::GetColorU32(ImVec4(0.14f, 0.14f, 0.16f, 0.85f * textAlpha)),
            3.0f
        );
        drawList->AddRectFilled(
            barMin,
            fillMax,
            ImGui::GetColorU32(ImVec4(0.95f, 0.67f, 0.20f, 0.95f * textAlpha)),
            3.0f
        );
    }

}

// ------------------------------------------------------- GLFW Callbacks ---
void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) return; // Minimised — pause rendering
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app && app->m_GraphicsContext) {
        app->m_GraphicsContext->OnResize(
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        );
    }
}

void Application::KeyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    (void)window;
    bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
    InputManager::UpdateKeyState(key, pressed);

    // Publish via EventBus
    KeyPressedEvent evt(key, action == GLFW_REPEAT ? 1 : 0);
    SZM::EventBus::GetInstance().Publish("KeyPressed", evt);
}

void Application::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
    (void)window;
    InputManager::UpdateMousePosition(static_cast<float>(xpos),
                                      static_cast<float>(ypos));
}

void Application::FocusCallback(GLFWwindow* window, int focused) {
    (void)window;
    if (!focused) {
        InputManager::ClearStateOnFocusLost(); // Edge-Case 11: sticky keys on Alt-Tab
    }
}

} // namespace SZM
