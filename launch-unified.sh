#!/bin/bash
# SZM Forge Unified Launch Script

cd "$(dirname "$0")"

# Start Python AI bridge in background if not running
if ! pgrep -f "ai_service/external_engine_bridge.py" > /dev/null; then
    echo "Starting AI Bridge on port 8003..."
    source venv/bin/activate
    python ai_service/external_engine_bridge.py &
    AI_PID=$!
    deactivate
fi

./build-final/SZM_Forge

# Cleanup AI bridge on exit
if [ ! -z "$AI_PID" ]; then
    kill $AI_PID
fi
