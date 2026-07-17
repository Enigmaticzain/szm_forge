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

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <ForgeStoreProvider>
      <ThemeProvider>
        <TooltipProvider>
          <BackendProvider>
            <ProjectProvider>
              <App />
            </ProjectProvider>
          </BackendProvider>
        </TooltipProvider>
      </ThemeProvider>
    </ForgeStoreProvider>
  </StrictMode>
);
