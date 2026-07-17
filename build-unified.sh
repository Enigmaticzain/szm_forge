#!/bin/bash

# ============================================================
#  SZM Forge Unified Build Script
#  Merges all build variants into a single final build
# ============================================================

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-final"
INSTALL_DIR="${PROJECT_ROOT}/install"
EXECUTABLE="${BUILD_DIR}/SZM_Forge"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_section() {
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
}

# ============================================================
#  Check Dependencies
# ============================================================
check_dependencies() {
    log_section "Checking Dependencies"
    
    local missing=0
    
    if ! command -v cmake &> /dev/null; then
        log_error "CMake not found. Install with: sudo apt install cmake"
        missing=1
    fi
    
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        log_error "C++ compiler not found. Install with: sudo apt install build-essential"
        missing=1
    fi
    
    if ! command -v node &> /dev/null || ! command -v npm &> /dev/null; then
        log_warn "Node.js/npm not found. React UI will be skipped."
    fi
    
    if ! pkg-config --exists glfw3 2>/dev/null; then
        log_warn "GLFW3 not found. Install with: sudo apt install libglfw3-dev"
    fi
    
    if ! pkg-config --exists eigen3 2>/dev/null; then
        log_warn "Eigen3 not found. Install with: sudo apt install libeigen3-dev"
    fi
    
    if [ $missing -eq 1 ]; then
        log_error "Critical dependencies missing. Please install them and try again."
        return 1
    fi
    
    log_success "All dependencies found"
    return 0
}

# ============================================================
#  Clean All Build Directories
# ============================================================
clean_all_builds() {
    log_section "Cleaning All Build Directories"
    
    local builds=(
        "build"
        "build-full"
        "build-desktop"
        "build-check"
        "build-ci-lightweight"
        "build-code8-lite"
        "build-code8-full"
        "build-bullet-check"
        "build-bullet-local"
        "build-full-check"
        "build-asan"
        "legendary-ai-build"
    )
    
    for build_dir in "${builds[@]}"; do
        if [ -d "${PROJECT_ROOT}/${build_dir}" ]; then
            log_info "Cleaning ${build_dir}..."
            rm -rf "${PROJECT_ROOT}/${build_dir}"
        fi
    done
    
    log_success "All build directories cleaned"
}

# ============================================================
#  Configure Unified Build
# ============================================================
configure_unified_build() {
    log_section "Configuring Unified Build"
    
    mkdir -p "$BUILD_DIR"
    
    log_info "Enabling all features:"
    log_info "  - Full desktop mode (GLFW, Eigen, Graphics)"
    log_info "  - Native 3D workshop UI (ImGui + Workshop3D; default)"
    log_info "  - React webview optional (set -DSZM_USE_REACT_WEBVIEW=ON to enable)"
    log_info "  - Three.js integration"
    log_info "  - EnTT ECS"
    log_info "  - FastAPI server"
    log_info "  - Bullet physics (if available)"
    log_info "  - AI module"
    log_info "  - FEA simulation"
    log_info "  - Thermal engine"
    log_info "  - Electrical engine"
    
    cmake -S "$PROJECT_ROOT" \
          -B "$BUILD_DIR" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
          -DSZM_BUILD_FULL_DESKTOP=ON \
          -DSZM_BUILD_WEBUI=ON \
          -DSZM_USE_REACT_WEBVIEW=ON \
          -DSZM_ENABLE_THREEJS_INTEGRATION=ON \
          -DSZM_ENABLE_ENTT_ECS=ON \
          -DSZM_ENABLE_FASTAPI_SERVER=ON \
          -DSZM_ENABLE_BULLET_BACKEND=OFF \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 2>&1 | tee "$BUILD_DIR/cmake_config.log"
    
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        log_error "CMake configuration failed"
        return 1
    fi
    
    log_success "CMake configuration complete"
    return 0
}

# ============================================================
#  Build Project
# ============================================================
build_project() {
    log_section "Building SZM Forge (Unified)"
    
    cd "$BUILD_DIR"
    
    local num_jobs=$(nproc)
    log_info "Using $num_jobs parallel jobs..."
    
    cmake --build . --parallel "$num_jobs" 2>&1 | tee build.log
    
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        log_error "Build failed"
        return 1
    fi
    
    log_success "Build complete"
    return 0
}

# ============================================================
#  Create Desktop Shortcut
# ============================================================
create_desktop_shortcut() {
    log_section "Creating Desktop Shortcut"
    
    local desktop_dir="${HOME}/Desktop"
    local desktop_file="${desktop_dir}/SZM_Forge.desktop"
    
    if [ ! -d "$desktop_dir" ]; then
        log_warn "Desktop directory not found at $desktop_dir"
        desktop_dir="${HOME}/.local/share/applications"
        desktop_file="${desktop_dir}/SZM_Forge.desktop"
        mkdir -p "$desktop_dir"
    fi
    
    # Find the executable
    if [ ! -f "$EXECUTABLE" ]; then
        log_error "Executable not found at $EXECUTABLE"
        return 1
    fi
    
    # Create desktop file
    cat > "$desktop_file" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=SZM Forge
Comment=Physics-Driven Engineering Platform - Unified Build
Exec=${EXECUTABLE}
Path=${PROJECT_ROOT}
Terminal=false
StartupWMClass=SZM_Forge
Categories=Engineering;Science;Development;
StartupNotify=true
Icon=${PROJECT_ROOT}/assets/icon.png
EOF
    
    chmod +x "$desktop_file"
    chmod +x "$EXECUTABLE"
    
    log_success "Desktop shortcut created at $desktop_file"
    
    # Also copy to applications menu
    mkdir -p "${HOME}/.local/share/applications"
    cp "$desktop_file" "${HOME}/.local/share/applications/"
    
    log_success "Application added to system menu"
    return 0
}

# ============================================================
#  Create Launch Script
# ============================================================
create_launch_script() {
    log_section "Creating Launch Script"
    
    local launch_script="${PROJECT_ROOT}/launch-unified.sh"
    
    cat > "$launch_script" << 'EOF'
#!/bin/bash
# SZM Forge Unified Launch Script

cd "$(dirname "$0")"
./build-final/SZM_Forge
EOF
    
    chmod +x "$launch_script"
    log_success "Launch script created at $launch_script"
}

# ============================================================
#  Main Script Logic
# ============================================================
main() {
    log_section "SZM Forge Unified Build System"
    log_info "Project Root: $PROJECT_ROOT"
    log_info "Build Directory: $BUILD_DIR"
    log_info "Executable: $EXECUTABLE"
    echo ""
    
    # Parse arguments
    case "${1:-build}" in
        clean)
            clean_all_builds
            ;;
        rebuild)
            clean_all_builds
            check_dependencies || exit 1
            configure_unified_build || exit 1
            build_project || exit 1
            create_desktop_shortcut || exit 1
            create_launch_script
            log_success "Full rebuild complete!"
            ;;
        quick|*)
            check_dependencies || exit 1
            
            if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
                log_info "First build detected, configuring..."
                configure_unified_build || exit 1
            else
                log_info "Incremental build..."
            fi
            
            build_project || exit 1
            create_desktop_shortcut || exit 1
            create_launch_script
            log_success "Build complete!"
            ;;
    esac
    
    echo ""
    log_section "Build Summary"
    log_info "Executable: $EXECUTABLE"
    log_info "Desktop Shortcut: ${HOME}/Desktop/SZM_Forge.desktop"
    log_info "Launch Script: ${PROJECT_ROOT}/launch-unified.sh"
    echo ""
    log_success "To launch SZM Forge:"
    echo "  - Double-click the desktop icon, OR"
    echo "  - Run: ${PROJECT_ROOT}/launch-unified.sh"
    echo "  - Or find it in your applications menu"
}

main "$@"
