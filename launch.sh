#!/bin/bash

# ============================================================
#  SZM Forge Smart Launcher
#  Auto-rebuilds if source files changed
# ============================================================

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-desktop"
EXECUTABLE="${BUILD_DIR}/SZM_Forge"
TIMESTAMP_FILE="${BUILD_DIR}/.last_build_timestamp"

# Colors
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
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

# ============================================================
#  Check if Rebuild Needed
# ============================================================
needs_rebuild() {
    # If executable doesn't exist, rebuild
    if [ ! -f "$EXECUTABLE" ]; then
        log_warn "Executable not found"
        return 0
    fi
    
    # If no timestamp file, rebuild
    if [ ! -f "$TIMESTAMP_FILE" ]; then
        log_warn "No build timestamp found"
        return 0
    fi
    
    local last_build=$(cat "$TIMESTAMP_FILE")
    
    # Check if any source files are newer than last build
    find "${PROJECT_ROOT}/src" -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "CMakeLists.txt" \) -newer "$TIMESTAMP_FILE" | grep -q . && return 0
    
    # Check if CMakeLists_AutoBuild.txt changed
    [ "${PROJECT_ROOT}/CMakeLists_AutoBuild.txt" -nt "$TIMESTAMP_FILE" ] && return 0
    
    return 1
}

# ============================================================
#  Rebuild if Needed
# ============================================================
rebuild_if_needed() {
    if needs_rebuild; then
        log_info "Source files changed, rebuilding..."
        
        cd "$PROJECT_ROOT"
        bash build.sh incremental || {
            log_error "Rebuild failed"
            return 1
        }
        
        mkdir -p "$BUILD_DIR"
        touch "$TIMESTAMP_FILE"
        log_success "Rebuild complete"
    else
        log_info "No changes detected, using cached build"
    fi
}

# ============================================================
#  Launch Application
# ============================================================
launch_app() {
    if [ ! -f "$EXECUTABLE" ]; then
        log_error "Executable not found at $EXECUTABLE"
        log_info "Run: bash build.sh rebuild"
        return 1
    fi
    
    log_info "Launching SZM Forge..."
    log_info "Executable: $EXECUTABLE"
    
    # Launch in background and detach from terminal
    nohup "$EXECUTABLE" > /dev/null 2>&1 &
    
    log_success "SZM Forge launched (PID: $!)"
}

# ============================================================
#  Main
# ============================================================
main() {
    log_info "SZM Forge Smart Launcher"
    
    # First build?
    if [ ! -d "$BUILD_DIR" ]; then
        log_warn "First launch detected, building..."
        cd "$PROJECT_ROOT"
        bash build.sh rebuild || {
            log_error "Initial build failed"
            return 1
        }
        mkdir -p "$BUILD_DIR"
        touch "$TIMESTAMP_FILE"
    fi
    
    # Check for changes and rebuild if needed
    rebuild_if_needed || return 1
    
    # Launch
    launch_app
}

main "$@"
