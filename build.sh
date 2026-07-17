#!/bin/bash

# ============================================================
#  SZM Forge Auto-Build Script
#  Automatically detects new modules and rebuilds
# ============================================================

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-desktop"
INSTALL_DIR="${PROJECT_ROOT}/install"
DESKTOP_ICON="${HOME}/.local/share/applications/szm-forge.desktop"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# ============================================================
#  Check Dependencies
# ============================================================
check_dependencies() {
    log_info "Checking dependencies..."
    
    local missing=0
    
    if ! command -v cmake &> /dev/null; then
        log_error "CMake not found. Install with: sudo apt install cmake"
        missing=1
    fi
    
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        log_error "C++ compiler not found. Install with: sudo apt install build-essential"
        missing=1
    fi
    
    if ! pkg-config --exists glfw3 2>/dev/null; then
        log_warn "GLFW3 not found. Will attempt to build from source."
    fi
    
    if ! pkg-config --exists eigen3 2>/dev/null; then
        log_warn "Eigen3 not found. Install with: sudo apt install libeigen3-dev"
        missing=1
    fi
    
    if [ $missing -eq 1 ]; then
        log_error "Critical dependencies missing. Please install them and try again."
        return 1
    fi
    
    log_success "All dependencies found"
    return 0
}

# ============================================================
#  Scan for New Modules
# ============================================================
scan_modules() {
    log_info "Scanning for modules in src/..."
    
    local module_count=0
    for dir in "${PROJECT_ROOT}"/src/*/; do
        if [ -f "${dir}CMakeLists.txt" ]; then
            local module_name=$(basename "$dir")
            log_info "  Found: $module_name"
            ((module_count++))
        fi
    done
    
    log_success "Found $module_count modules"
}

# ============================================================
#  Configure Build
# ============================================================
configure_build() {
    log_info "Configuring CMake build..."
    
    mkdir -p "$BUILD_DIR"
    
    cmake -S "$PROJECT_ROOT" \
          -B "$BUILD_DIR" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
          -DSZM_BUILD_FULL_DESKTOP=ON \
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
    log_info "Building SZM Forge..."
    
    cd "$BUILD_DIR"
    
    local num_jobs=$(nproc)
    cmake --build . --parallel "$num_jobs" 2>&1 | tee build.log
    
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        log_error "Build failed"
        return 1
    fi
    
    log_success "Build complete"
    return 0
}

# ============================================================
#  Create Desktop Icon
# ============================================================
create_desktop_icon() {
    log_info "Creating desktop icon..."
    
    local executable="${BUILD_DIR}/SZM_Forge"
    local icon_path="${PROJECT_ROOT}/assets/icon.png"
    
    if [ ! -f "$executable" ]; then
        log_error "Executable not found at $executable"
        return 1
    fi
    
    mkdir -p "$(dirname "$DESKTOP_ICON")"
    
    cat > "$DESKTOP_ICON" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=SZM Forge
Comment=Physics-Driven Engineering Platform
Exec=${executable}
Icon=${icon_path}
Terminal=false
Categories=Engineering;Science;Development;
StartupNotify=true
EOF
    
    chmod +x "$DESKTOP_ICON"
    chmod +x "$executable"
    
    log_success "Desktop icon created at $DESKTOP_ICON"
    return 0
}

# ============================================================
#  Incremental Rebuild (for development)
# ============================================================
incremental_rebuild() {
    log_info "Performing incremental rebuild..."

    if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        log_warn "Build directory is missing or not configured yet. Running full configuration..."
        configure_build || return 1
    fi
    
    cd "$BUILD_DIR"
    
    # Check if CMakeLists.txt changed
    if [ "$PROJECT_ROOT/CMakeLists_AutoBuild.txt" -nt "$BUILD_DIR/CMakeCache.txt" ]; then
        log_warn "CMakeLists.txt changed, reconfiguring..."
        configure_build || return 1
    fi
    
    # Scan for new modules
    scan_modules
    
    # Rebuild
    build_project || return 1
    
    log_success "Incremental rebuild complete"
    return 0
}

# ============================================================
#  Clean Build
# ============================================================
clean_build() {
    log_info "Cleaning build directory..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        log_success "Build directory cleaned"
    fi
}

# ============================================================
#  Main Script Logic
# ============================================================
main() {
    log_info "SZM Forge Auto-Build System"
    log_info "Project Root: $PROJECT_ROOT"
    log_info "Build Directory: $BUILD_DIR"
    
    # Parse arguments
    case "${1:-build}" in
        clean)
            clean_build
            ;;
        rebuild)
            clean_build
            check_dependencies || exit 1
            scan_modules
            configure_build || exit 1
            build_project || exit 1
            create_desktop_icon || exit 1
            log_success "Full rebuild complete!"
            ;;
        incremental)
            check_dependencies || exit 1
            incremental_rebuild || exit 1
            ;;
        build|*)
            check_dependencies || exit 1
            
            if [ ! -d "$BUILD_DIR" ]; then
                log_info "First build detected, performing full configuration..."
                scan_modules
                configure_build || exit 1
            else
                log_info "Incremental build..."
                scan_modules
            fi
            
            build_project || exit 1
            create_desktop_icon || exit 1
            log_success "Build complete!"
            ;;
    esac
    
    log_info "Executable: ${BUILD_DIR}/SZM_Forge"
    log_info "To launch: ${BUILD_DIR}/SZM_Forge"
}

main "$@"
