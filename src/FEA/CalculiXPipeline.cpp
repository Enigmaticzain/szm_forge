#include "CalculiXPipeline.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {

std::string TrimCopy(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string ToLowerCopy(const std::string& value) {
    std::string lowered = value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lowered;
}

bool HasExtension(const fs::path& path, const std::initializer_list<const char*> extensions) {
    const std::string ext = ToLowerCopy(path.extension().string());
    for (const char* candidate : extensions) {
        if (ext == candidate) {
            return true;
        }
    }
    return false;
}

bool IsExampleCoreFile(const fs::path& path) {
    const std::string filename = ToLowerCopy(path.filename().string());
    return filename == "test.py" ||
           HasExtension(path, { ".inp", ".fbd", ".fbl", ".sm", ".geo", ".step", ".stp", ".stl", ".obj" });
}

bool DirectoryContainsExampleCoreFiles(const fs::path& directory) {
    std::error_code error;
    if (!fs::exists(directory, error) || !fs::is_directory(directory, error)) {
        return false;
    }

    for (const auto& entry : fs::directory_iterator(directory, error)) {
        if (error) {
            return false;
        }

        if (entry.is_regular_file(error) && IsExampleCoreFile(entry.path())) {
            return true;
        }
    }

    return false;
}

fs::path NormalizeExampleDirectory(const fs::path& path) {
    return path.filename() == "Refs" ? path.parent_path() : path;
}

std::string FindSummaryLine(const fs::path& readmePath) {
    std::ifstream input(readmePath);
    if (!input.is_open()) {
        return {};
    }

    std::string line;
    bool skippedHeading = false;
    while (std::getline(input, line)) {
        const std::string trimmed = TrimCopy(line);
        if (trimmed.empty()) {
            continue;
        }

        if (!skippedHeading && trimmed.front() == '#') {
            skippedHeading = true;
            continue;
        }

        if (trimmed.front() == '#' || trimmed.front() == '[' || trimmed.front() == '!' || trimmed.front() == '|') {
            continue;
        }

        return trimmed;
    }

    return {};
}

fs::path FindReadmePath(const fs::path& directory) {
    for (const auto& candidate : { "README.md", "Readme.md", "README.txt", "Readme.txt" }) {
        const fs::path readme = directory / candidate;
        std::error_code error;
        if (fs::exists(readme, error) && fs::is_regular_file(readme, error)) {
            return readme;
        }
    }
    return {};
}

fs::path ChoosePreviewImage(const fs::path& directory) {
    fs::path bestPath;
    int bestScore = -1;
    std::error_code error;

    for (const auto& entry : fs::recursive_directory_iterator(directory, error)) {
        if (error) {
            break;
        }

        if (!entry.is_regular_file(error) || !HasExtension(entry.path(), { ".png", ".jpg", ".jpeg", ".gif" })) {
            continue;
        }

        const std::string filename = ToLowerCopy(entry.path().filename().string());
        int score = 1;
        if (filename.find("preview") != std::string::npos) score += 6;
        if (filename.find("contact") != std::string::npos) score += 5;
        if (filename.find("mesh") != std::string::npos) score += 4;
        if (filename.find("def") != std::string::npos || filename.find("deformed") != std::string::npos) score += 4;
        if (filename.find("exp") != std::string::npos || filename.find("se") != std::string::npos) score += 3;
        if (entry.path().parent_path().filename() == "Refs") score += 3;

        if (score > bestScore || (score == bestScore && entry.path().string() < bestPath.string())) {
            bestScore = score;
            bestPath = entry.path();
        }
    }

    return bestPath;
}

bool IsPathSearchSeparator(char ch) {
#ifdef _WIN32
    return ch == ';';
#else
    return ch == ':';
#endif
}

std::vector<std::string> SplitPathEnvironment(const std::string& pathValue) {
    std::vector<std::string> entries;
    std::string current;
    for (const char ch : pathValue) {
        if (IsPathSearchSeparator(ch)) {
            if (!current.empty()) {
                entries.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(ch);
    }

    if (!current.empty()) {
        entries.push_back(current);
    }

    return entries;
}

std::vector<std::string> CandidateExecutableNames(const std::string& baseName) {
#ifdef _WIN32
    if (fs::path(baseName).has_extension()) {
        return { baseName };
    }

    return {
        baseName,
        baseName + ".exe",
        baseName + ".bat",
        baseName + ".cmd"
    };
#else
    return { baseName };
#endif
}

bool IsProbablyExecutable(const fs::path& candidate) {
    std::error_code error;
    if (!fs::exists(candidate, error) || !fs::is_regular_file(candidate, error)) {
        return false;
    }

#ifndef _WIN32
    const auto permissions = fs::status(candidate, error).permissions();
    if (error) {
        return false;
    }

    using perms = fs::perms;
    return (permissions & perms::owner_exec) != perms::none ||
           (permissions & perms::group_exec) != perms::none ||
           (permissions & perms::others_exec) != perms::none;
#else
    return true;
#endif
}

std::string FindExecutableOnPath(const std::vector<std::string>& toolNames) {
    const char* rawPath = std::getenv("PATH");
    const std::string pathValue = rawPath != nullptr ? rawPath : "";
    const std::vector<std::string> searchRoots = SplitPathEnvironment(pathValue);

    for (const auto& toolName : toolNames) {
        const fs::path direct(toolName);
        if (direct.has_parent_path() && IsProbablyExecutable(direct)) {
            return fs::absolute(direct).string();
        }

        for (const auto& root : searchRoots) {
            for (const auto& candidateName : CandidateExecutableNames(toolName)) {
                const fs::path candidate = fs::path(root) / candidateName;
                if (IsProbablyExecutable(candidate)) {
                    return candidate.string();
                }
            }
        }
    }

    return {};
}

SZM::FEA::ToolAvailability ProbeTool(const std::string& displayName, const std::vector<std::string>& toolNames) {
    SZM::FEA::ToolAvailability tool;
    tool.Name = displayName;
    tool.Executable = FindExecutableOnPath(toolNames);
    tool.Available = !tool.Executable.empty();
    if (!tool.Available && !toolNames.empty()) {
        tool.Executable = toolNames.front();
    }
    return tool;
}

std::string NormalizeExampleName(const std::string& value) {
    std::string normalized = ToLowerCopy(value);
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    while (!normalized.empty() && normalized.front() == '/') {
        normalized.erase(normalized.begin());
    }
    while (!normalized.empty() && normalized.back() == '/') {
        normalized.pop_back();
    }
    return normalized;
}

std::string SanitizeDirectoryName(const std::string& value) {
    std::string sanitized;
    sanitized.reserve(value.size());

    for (const char ch : value) {
        if (std::isalnum(static_cast<unsigned char>(ch)) != 0) {
            sanitized.push_back(ch);
        } else if (ch == '/' || ch == '\\' || ch == ' ' || ch == '-' || ch == '.') {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = "code8_workspace";
    }

    return sanitized;
}

std::string QuoteShell(const std::string& value) {
    std::string quoted;
    quoted.reserve(value.size() + 2);
    quoted.push_back('"');
    for (const char ch : value) {
        if (ch == '"' || ch == '\\') {
            quoted.push_back('\\');
        }
        quoted.push_back(ch);
    }
    quoted.push_back('"');
    return quoted;
}

void CopyPathRecursive(
    const fs::path& source,
    const fs::path& destination,
    std::vector<fs::path>& copiedRoots)
{
    std::error_code error;
    if (!fs::exists(source, error)) {
        return;
    }

    fs::create_directories(destination.parent_path(), error);
    if (error) {
        throw std::runtime_error("Failed to create parent directory for " + destination.string());
    }

    fs::copy(
        source,
        destination,
        fs::copy_options::recursive | fs::copy_options::overwrite_existing,
        error
    );
    if (error) {
        throw std::runtime_error("Failed to copy " + source.string() + " to " + destination.string());
    }

    copiedRoots.push_back(destination);
}

void WriteManifestFile(
    const fs::path& manifestPath,
    const fs::path& code8Root,
    const SZM::FEA::CalculiXExample& example,
    const SZM::FEA::CalculiXToolchainStatus& toolchain,
    const std::vector<std::string>& executionPlan)
{
    std::ofstream output(manifestPath);
    if (!output.is_open()) {
        throw std::runtime_error("Failed to write workspace manifest " + manifestPath.string());
    }

    output << "example=" << example.Name << '\n';
    output << "category=" << example.Category << '\n';
    output << "source_root=" << code8Root.string() << '\n';
    output << "source_directory=" << example.Directory.string() << '\n';
    output << "summary=" << example.Summary << '\n';
    output << "tool_python=" << (toolchain.Python.Available ? toolchain.Python.Executable : "missing") << '\n';
    output << "tool_cgx=" << (toolchain.CGX.Available ? toolchain.CGX.Executable : "missing") << '\n';
    output << "tool_ccx=" << (toolchain.CCX.Available ? toolchain.CCX.Executable : "missing") << '\n';
    output << "tool_gmsh=" << (toolchain.Gmsh.Available ? toolchain.Gmsh.Executable : "missing") << '\n';
    output << "tool_gnuplot=" << (toolchain.Gnuplot.Available ? toolchain.Gnuplot.Executable : "missing") << '\n';
    output << "tool_tetgen=" << (toolchain.TetGen.Available ? toolchain.TetGen.Executable : "missing") << '\n';
    output << '\n' << "[execution_plan]" << '\n';
    for (const auto& command : executionPlan) {
        output << command << '\n';
    }
}

SZM::FEA::CalculiXExample BuildExampleRecord(
    const fs::path& code8Root,
    const fs::path& exampleDirectory)
{
    SZM::FEA::CalculiXExample example;
    example.Directory = exampleDirectory;
    example.Name = fs::relative(exampleDirectory, code8Root).generic_string();

    const auto relative = fs::relative(exampleDirectory, code8Root);
    if (relative.begin() != relative.end()) {
        example.Category = relative.begin()->string();
    } else {
        example.Category = "code8";
    }

    example.ReadmePath = FindReadmePath(exampleDirectory);
    example.PreviewPath = ChoosePreviewImage(exampleDirectory);
    example.Summary = !example.ReadmePath.empty() ? FindSummaryLine(example.ReadmePath) : std::string();

    std::error_code error;
    for (const auto& entry : fs::directory_iterator(exampleDirectory, error)) {
        if (error || !entry.is_regular_file(error)) {
            continue;
        }

        const fs::path path = entry.path();
        const std::string filename = ToLowerCopy(path.filename().string());

        if (filename == "test.py") {
            example.TestScriptPath = path;
            continue;
        }

        if (HasExtension(path, { ".inp" })) {
            example.SolverInputFiles.push_back(path);
            continue;
        }

        if (HasExtension(path, { ".fbd", ".fbl", ".sm" })) {
            example.CGXScripts.push_back(path);
            if (filename.find("post") != std::string::npos) {
                example.UsesPostProcessing = true;
            }
            continue;
        }

        if (HasExtension(path, { ".geo" })) {
            example.GmshInputFiles.push_back(path);
            continue;
        }

        if (HasExtension(path, { ".py" })) {
            example.HelperPythonFiles.push_back(path);
        }
    }

    if (!example.TestScriptPath.empty()) {
        std::ifstream input(example.TestScriptPath);
        std::stringstream buffer;
        buffer << input.rdbuf();
        const std::string script = ToLowerCopy(buffer.str());
        example.UsesParametricPreprocessor = script.find("param.py") != std::string::npos;
        example.UsesMonitoring = script.find("monitor.py") != std::string::npos;
        example.UsesPostProcessing = example.UsesPostProcessing || script.find("post.fbd") != std::string::npos;
    } else {
        example.UsesParametricPreprocessor = std::any_of(
            example.CGXScripts.begin(),
            example.CGXScripts.end(),
            [](const fs::path& path) {
                return ToLowerCopy(path.filename().string()).rfind("par.", 0U) == 0U;
            }
        );
    }

    std::sort(example.SolverInputFiles.begin(), example.SolverInputFiles.end());
    std::sort(example.CGXScripts.begin(), example.CGXScripts.end());
    std::sort(example.GmshInputFiles.begin(), example.GmshInputFiles.end());
    std::sort(example.HelperPythonFiles.begin(), example.HelperPythonFiles.end());

    return example;
}

} // namespace

namespace SZM::FEA {

    fs::path CalculiXPipeline::DiscoverCode8Root(const fs::path& startSearchDirectory) {
        if (const char* configured = std::getenv("SZM_CODE8_DIR")) {
            const fs::path configuredPath(configured);
            std::error_code error;
            if (fs::exists(configuredPath, error) && fs::is_directory(configuredPath, error)) {
                return fs::absolute(configuredPath);
            }
        }

        std::vector<fs::path> bases;
        bases.push_back(startSearchDirectory);

        fs::path cursor = startSearchDirectory;
        for (int i = 0; i < 6 && !cursor.empty(); ++i) {
            cursor = cursor.parent_path();
            if (!cursor.empty()) {
                bases.push_back(cursor);
            }
        }

        for (const auto& base : bases) {
            for (const auto& candidate : {
                base / "codes" / "code 8",
                base / "code 8"
            }) {
                std::error_code error;
                if (fs::exists(candidate, error) && fs::is_directory(candidate, error)) {
                    return fs::absolute(candidate);
                }
            }
        }

        return {};
    }

    std::vector<CalculiXExample> CalculiXPipeline::CatalogExamples(const fs::path& code8Root) {
        std::vector<CalculiXExample> catalog;
        std::error_code error;
        if (!fs::exists(code8Root, error) || !fs::is_directory(code8Root, error)) {
            return catalog;
        }

        std::set<fs::path> exampleDirectories;

        for (const auto& entry : fs::recursive_directory_iterator(code8Root, error)) {
            if (error || !entry.is_regular_file(error)) {
                continue;
            }

            const fs::path relative = fs::relative(entry.path(), code8Root, error);
            if (error || relative.empty()) {
                continue;
            }

            if (relative.begin()->string() == "Scripts") {
                continue;
            }

            const fs::path parent = NormalizeExampleDirectory(entry.path().parent_path());
            const fs::path relativeParent = fs::relative(parent, code8Root, error);
            if (error || relativeParent.empty()) {
                continue;
            }

            if (relativeParent.begin()->string() == "Scripts") {
                continue;
            }

            if (IsExampleCoreFile(entry.path()) && DirectoryContainsExampleCoreFiles(parent)) {
                exampleDirectories.insert(parent);
            }
        }

        for (const auto& directory : exampleDirectories) {
            catalog.push_back(BuildExampleRecord(code8Root, directory));
        }

        std::sort(catalog.begin(), catalog.end(), [](const CalculiXExample& lhs, const CalculiXExample& rhs) {
            return lhs.Name < rhs.Name;
        });

        return catalog;
    }

    std::optional<CalculiXExample> CalculiXPipeline::FindExample(
        const fs::path& code8Root,
        const std::string& exampleName)
    {
        const std::string normalizedQuery = NormalizeExampleName(exampleName);
        const auto catalog = CatalogExamples(code8Root);

        for (const auto& example : catalog) {
            if (NormalizeExampleName(example.Name) == normalizedQuery) {
                return example;
            }
        }

        std::optional<CalculiXExample> fallback;
        for (const auto& example : catalog) {
            if (NormalizeExampleName(example.Directory.filename().string()) == normalizedQuery) {
                if (fallback.has_value()) {
                    return std::nullopt;
                }
                fallback = example;
            }
        }

        return fallback;
    }

    CalculiXToolchainStatus CalculiXPipeline::ProbeToolchain() {
        CalculiXToolchainStatus status;
        status.Python = ProbeTool("python", { "python3", "python" });
        status.CGX = ProbeTool("cgx", { "cgx" });
        status.CCX = ProbeTool("ccx", { "ccx" });
        status.Gmsh = ProbeTool("gmsh", { "gmsh" });
        status.Gnuplot = ProbeTool("gnuplot", { "gnuplot" });
        status.TetGen = ProbeTool("tetgen", { "tetgen" });
        return status;
    }

    std::vector<std::string> CalculiXPipeline::BuildExecutionPlan(
        const CalculiXExample& example,
        const CalculiXToolchainStatus& toolchain,
        const fs::path& workspaceRoot)
    {
        std::vector<std::string> commands;
        const fs::path exampleDirectory = workspaceRoot.empty()
            ? example.Directory
            : workspaceRoot / fs::path(example.Name);

        if (example.HasTestScript()) {
            const std::string python = toolchain.Python.Executable.empty()
                ? std::string("python3")
                : toolchain.Python.Executable;
            commands.push_back(
                "cd " + QuoteShell(exampleDirectory.string()) +
                " && " + QuoteShell(python) + " test.py"
            );
            return commands;
        }

        if (!example.GmshInputFiles.empty()) {
            const fs::path firstGeo = example.GmshInputFiles.front().filename();
            commands.push_back(
                "cd " + QuoteShell(exampleDirectory.string()) +
                " && " + QuoteShell(toolchain.Gmsh.Executable.empty() ? std::string("gmsh") : toolchain.Gmsh.Executable) +
                " " + QuoteShell(firstGeo.string())
            );
        }

        if (!example.CGXScripts.empty()) {
            const fs::path firstScript = example.CGXScripts.front().filename();
            commands.push_back(
                "cd " + QuoteShell(exampleDirectory.string()) +
                " && " + QuoteShell(toolchain.CGX.Executable.empty() ? std::string("cgx") : toolchain.CGX.Executable) +
                " -b " + QuoteShell(firstScript.string())
            );
        }

        if (!example.SolverInputFiles.empty()) {
            const fs::path firstInput = example.SolverInputFiles.front();
            commands.push_back(
                "cd " + QuoteShell(exampleDirectory.string()) +
                " && " + QuoteShell(toolchain.CCX.Executable.empty() ? std::string("ccx") : toolchain.CCX.Executable) +
                " " + QuoteShell(firstInput.stem().string())
            );
        }

        return commands;
    }

    CalculiXWorkspaceManifest CalculiXPipeline::StageExampleWorkspace(
        const fs::path& code8Root,
        const CalculiXExample& example,
        const fs::path& stagingDirectory)
    {
        std::error_code error;
        if (!fs::exists(code8Root, error) || !fs::is_directory(code8Root, error)) {
            throw std::runtime_error("code 8 root does not exist: " + code8Root.string());
        }
        if (!fs::exists(example.Directory, error) || !fs::is_directory(example.Directory, error)) {
            throw std::runtime_error("Example directory does not exist: " + example.Directory.string());
        }

        CalculiXWorkspaceManifest manifest;
        manifest.WorkspaceRoot = fs::absolute(stagingDirectory) / SanitizeDirectoryName(example.Name);
        manifest.ExampleWorkspace = manifest.WorkspaceRoot / fs::path(example.Name);
        manifest.ManifestPath = manifest.WorkspaceRoot / "SZM_Code8Pipeline.txt";

        fs::create_directories(manifest.WorkspaceRoot, error);
        if (error) {
            throw std::runtime_error("Failed to create staging directory: " + manifest.WorkspaceRoot.string());
        }

        for (const auto& rootFile : { "README.md", "Setup.md", "run_all.py", "testall.py", "LICENSE" }) {
            const fs::path source = code8Root / rootFile;
            if (fs::exists(source, error) && !error) {
                CopyPathRecursive(source, manifest.WorkspaceRoot / rootFile, manifest.CopiedRoots);
            }
        }

        CopyPathRecursive(code8Root / "Scripts", manifest.WorkspaceRoot / "Scripts", manifest.CopiedRoots);
        CopyPathRecursive(example.Directory, manifest.ExampleWorkspace, manifest.CopiedRoots);

        const auto toolchain = ProbeToolchain();
        manifest.ExecutionPlan = BuildExecutionPlan(example, toolchain, manifest.WorkspaceRoot);
        WriteManifestFile(manifest.ManifestPath, code8Root, example, toolchain, manifest.ExecutionPlan);

        return manifest;
    }

} // namespace SZM::FEA
