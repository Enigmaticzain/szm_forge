#include "FEA/CalculiXPipeline.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace {

struct Options {
    fs::path    Root;
    fs::path    StageDirectory;
    std::string ExampleName;
    bool        ListOnly = false;
};

void PrintUsage() {
    std::cout
        << "SZM_Code8Pipeline\n"
        << "Usage:\n"
        << "  SZM_Code8Pipeline [--root <path>] [--list]\n"
        << "  SZM_Code8Pipeline [--root <path>] --example <name> [--stage <dir>]\n";
}

Options ParseArguments(int argc, char** argv) {
    Options options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--root" && i + 1 < argc) {
            options.Root = argv[++i];
            continue;
        }
        if (arg == "--example" && i + 1 < argc) {
            options.ExampleName = argv[++i];
            continue;
        }
        if (arg == "--stage" && i + 1 < argc) {
            options.StageDirectory = argv[++i];
            continue;
        }
        if (arg == "--list") {
            options.ListOnly = true;
            continue;
        }
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            std::exit(0);
        }

        throw std::runtime_error("Unknown argument: " + arg);
    }

    return options;
}

void PrintTool(const SZM::FEA::ToolAvailability& tool) {
    std::cout << "  - " << tool.Name << ": "
              << (tool.Available ? tool.Executable : std::string("missing")) << '\n';
}

void PrintSummary(
    const fs::path& root,
    const std::vector<SZM::FEA::CalculiXExample>& examples,
    const SZM::FEA::CalculiXToolchainStatus& toolchain)
{
    std::cout << "Code 8 root: " << root << '\n';
    std::cout << "Examples indexed: " << examples.size() << '\n';
    std::cout << "Toolchain:\n";
    PrintTool(toolchain.Python);
    PrintTool(toolchain.CGX);
    PrintTool(toolchain.CCX);
    PrintTool(toolchain.Gmsh);
    PrintTool(toolchain.Gnuplot);
    PrintTool(toolchain.TetGen);
}

void PrintExample(const SZM::FEA::CalculiXExample& example) {
    std::cout << "\nExample: " << example.Name << '\n';
    std::cout << "Category: " << example.Category << '\n';
    std::cout << "Directory: " << example.Directory << '\n';
    if (!example.Summary.empty()) {
        std::cout << "Summary: " << example.Summary << '\n';
    }
    if (!example.TestScriptPath.empty()) {
        std::cout << "Test script: " << example.TestScriptPath.filename() << '\n';
    }
    std::cout << "Inputs: " << example.SolverInputFiles.size() << " .inp, "
              << example.CGXScripts.size() << " CGX scripts, "
              << example.GmshInputFiles.size() << " .geo files\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Options options = ParseArguments(argc, argv);

        const fs::path root = options.Root.empty()
            ? SZM::FEA::CalculiXPipeline::DiscoverCode8Root(fs::current_path())
            : fs::absolute(options.Root);

        if (root.empty()) {
            std::cerr << "Unable to locate code 8. Use --root or set SZM_CODE8_DIR.\n";
            return 1;
        }

        const auto toolchain = SZM::FEA::CalculiXPipeline::ProbeToolchain();
        const auto examples = SZM::FEA::CalculiXPipeline::CatalogExamples(root);

        PrintSummary(root, examples, toolchain);

        if (options.ExampleName.empty()) {
            std::cout << "\nCatalog:\n";
            for (const auto& example : examples) {
                std::cout << "  - " << example.Name;
                if (!example.Summary.empty()) {
                    std::cout << " :: " << example.Summary;
                }
                std::cout << '\n';
            }
            return 0;
        }

        const auto example = SZM::FEA::CalculiXPipeline::FindExample(root, options.ExampleName);
        if (!example.has_value()) {
            std::cerr << "\nExample not found or ambiguous: " << options.ExampleName << '\n';
            return 1;
        }

        PrintExample(*example);

        if (!options.StageDirectory.empty()) {
            const auto manifest = SZM::FEA::CalculiXPipeline::StageExampleWorkspace(
                root,
                *example,
                options.StageDirectory
            );

            std::cout << "\nWorkspace staged at: " << manifest.WorkspaceRoot << '\n';
            std::cout << "Manifest: " << manifest.ManifestPath << '\n';
            std::cout << "Execution plan:\n";
            for (const auto& command : manifest.ExecutionPlan) {
                std::cout << "  " << command << '\n';
            }
            return 0;
        }

        const auto commands = SZM::FEA::CalculiXPipeline::BuildExecutionPlan(*example, toolchain);
        std::cout << "Execution plan:\n";
        for (const auto& command : commands) {
            std::cout << "  " << command << '\n';
        }
        return 0;

    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
