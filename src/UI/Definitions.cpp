#include <iostream>

int main() {
    try {
        SZM::WindowConfig config;
        config.Title = "SZM Forge - Runtime Execution";
        config.VSync = false; // Set to false to test raw tick speed

        SZM::Application& app = SZM::Application::GetInstance();
        app.Init(config);
        app.Run();
        app.Shutdown();
    }
    catch (const std::exception& e) {
        std::cerr << "CRITICAL FAILURE: " << e.what() << '\n';
        return -1;
    }

    return 0;
}

// EventBus.h