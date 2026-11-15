import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import "./index.css";
import "./legacy/index.css";
import App from "./App";
import { ThemeProvider } from "./store/ThemeContext";
import { TooltipProvider } from "./store/TooltipContext";
import { BackendProvider } from "./store/BackendContext";
import { ProjectProvider } from "./store/ProjectContext";
import { ForgeStoreProvider } from "./store/ForgeStoreContext";
import { SessionEventProvider } from "./store/SessionEventContext";
import { AuthProvider } from "./store/AuthContext";

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <AuthProvider>
      <ForgeStoreProvider>
        <ThemeProvider>
          <TooltipProvider>
            <BackendProvider>
              <ProjectProvider>
                <SessionEventProvider>
                  <App />
                </SessionEventProvider>
              </ProjectProvider>
            </BackendProvider>
          </TooltipProvider>
        </ThemeProvider>
      </ForgeStoreProvider>
    </AuthProvider>
  </StrictMode>
);
