#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace SZM::FEA {

    struct ToolAvailability {
        std::string Name;
        std::string Executable;
        bool        Available = false;
    };

    struct CalculiXToolchainStatus {
        ToolAvailability Python;
        ToolAvailability CGX;
        ToolAvailability CCX;
        ToolAvailability Gmsh;
        ToolAvailability Gnuplot;
        ToolAvailability TetGen;

        [[nodiscard]] bool HasCoreSolverStack() const {
            return Python.Available && CGX.Available && CCX.Available;
        }
    };

    struct CalculiXExample {
        std::string                 Name;
        std::string                 Category;
        std::filesystem::path       Directory;
        std::filesystem::path       ReadmePath;
        std::filesystem::path       PreviewPath;
        std::filesystem::path       TestScriptPath;
        std::string                 Summary;
        std::vector<std::filesystem::path> SolverInputFiles;
        std::vector<std::filesystem::path> CGXScripts;
        std::vector<std::filesystem::path> GmshInputFiles;
        std::vector<std::filesystem::path> HelperPythonFiles;
        bool UsesParametricPreprocessor = false;
        bool UsesMonitoring = false;
        bool UsesPostProcessing = false;

        [[nodiscard]] bool HasTestScript() const {
            return !TestScriptPath.empty();
        }
    };

    struct CalculiXWorkspaceManifest {
        std::filesystem::path             WorkspaceRoot;
        std::filesystem::path             ExampleWorkspace;
        std::filesystem::path             ManifestPath;
        std::vector<std::filesystem::path> CopiedRoots;
        std::vector<std::string>           ExecutionPlan;
    };

    class CalculiXPipeline {
    public:
        [[nodiscard]] static std::filesystem::path DiscoverCode8Root(
            const std::filesystem::path& startSearchDirectory = std::filesystem::current_path()
        );

        [[nodiscard]] static std::vector<CalculiXExample> CatalogExamples(
            const std::filesystem::path& code8Root
        );

        [[nodiscard]] static std::optional<CalculiXExample> FindExample(
            const std::filesystem::path& code8Root,
            const std::string& exampleName
        );

        [[nodiscard]] static CalculiXToolchainStatus ProbeToolchain();

        [[nodiscard]] static std::vector<std::string> BuildExecutionPlan(
            const CalculiXExample& example,
            const CalculiXToolchainStatus& toolchain,
            const std::filesystem::path& workspaceRoot = {}
        );

        [[nodiscard]] static CalculiXWorkspaceManifest StageExampleWorkspace(
            const std::filesystem::path& code8Root,
            const CalculiXExample& example,
            const std::filesystem::path& stagingDirectory
        );
    };

} // namespace SZM::FEA
