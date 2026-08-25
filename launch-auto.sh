#!/bin/bash

# ============================================================
#  SZM Forge Smart Launcher (Auto-Update)
#  Detects source changes and rebuilds before launch
# ============================================================

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build-full"
EXECUTABLE="${BUILD_DIR}/SZM_Forge"
TIMESTAMP_FILE="${BUILD_DIR}/.last_launch_check"

BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[✓]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[!]${NC} $1"; }
log_error() { echo -e "${RED}[✗]${NC} $1"; }

# ============================================================
#  Check for Source Changes
# ============================================================
needs_rebuild() {
    [ ! -f "$EXECUTABLE" ] && return 0
    [ ! -f "$TIMESTAMP_FILE" ] && return 0
    
    # Check if any source files changed since last check
    find "${PROJECT_ROOT}/src" -type f \( -name "*.cpp" -o -name "*.hpp" \) -newer "$TIMESTAMP_FILE" 2>/dev/null | grep -q . && return 0
    
    return 1
}

# ============================================================
#  Rebuild
# ============================================================
rebuild() {
    log_info "Rebuilding SZM Forge..."
    
    cd "$BUILD_DIR" || {
        log_error "Build directory not found"
        return 1
    }
    
    cmake --build . --parallel $(nproc) 2>&1 | tail -20
    
    if [ ${PIPESTATUS[0]} -eq 0 ]; then
        log_success "Rebuild complete"
        mkdir -p "$BUILD_DIR"
        touch "$TIMESTAMP_FILE"
        return 0
    else
        log_error "Rebuild failed"
        return 1
    fi
}

# ============================================================
#  Main
# ============================================================
main() {
    log_info "SZM Forge Launcher"
    
    # First launch?
    if [ ! -d "$BUILD_DIR" ]; then
        log_error "Build directory not found at $BUILD_DIR"
        log_info "Run: cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON"
        log_info "Then: cmake --build build-full --parallel"
        return 1
    fi
    
    # Check for changes
    if needs_rebuild; then
        log_warn "Source files changed"
        rebuild || return 1
    else
        log_info "No changes detected"
    fi
    
    # Launch
    if [ ! -f "$EXECUTABLE" ]; then
        log_error "Executable not found at $EXECUTABLE"
        return 1
    fi
    
    log_success "Launching SZM Forge..."
    
    # Start Python AI bridge in background if not running
    if ! pgrep -f "ai_service/external_engine_bridge.py" > /dev/null; then
        log_info "Starting AI Bridge on port 8003..."
        source venv/bin/activate
        nohup python ai_service/external_engine_bridge.py > ai_bridge.log 2>&1 &
        AI_PID=$!
        deactivate
    fi

    # Run SZM_Forge in foreground so the script blocks
    "$EXECUTABLE"
    
    # Cleanup AI bridge on exit
    if [ ! -z "$AI_PID" ]; then
        log_info "Cleaning up AI Bridge..."
        kill $AI_PID || true
    fi
}

main "$@"
