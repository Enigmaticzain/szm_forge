namespace SZM {

    struct WindowConfig {
        int Width = 1280;
        int Height = 720;
        std::string Title = "SZM Forge - Core";
        bool VSync = true;
    };

    class Application {
    public:
        // Singleton Access
        static Application& GetInstance() {
            static Application instance;
            return instance;
        }

        // Delete copy/move constructors to strictly enforce Singleton
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void Init(const WindowConfig& config);
        void Run();
        void Shutdown();

        // Getters
        GLFWwindow* GetNativeWindow() const { return m_WindowHandle; }
        double GetDeltaTime() const { return m_DeltaTime; }

    private:
        Application() = default;
        ~Application() = default;

        // Callbacks
        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    private:
        GLFWwindow* m_WindowHandle = nullptr;
        bool m_IsRunning = false;
        
        // High-Resolution Timing
        double m_DeltaTime = 0.0;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime;
    };

} // namespace SZM
