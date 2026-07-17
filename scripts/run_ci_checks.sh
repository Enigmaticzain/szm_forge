#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${SZM_CI_BUILD_DIR:-$ROOT_DIR/build-ci-lightweight}"

echo "[1/4] Building and testing the lightweight C++ demo"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=OFF
cmake --build "$BUILD_DIR" --parallel
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "[2/4] Running backend integration tests"
npm --prefix "$ROOT_DIR/webui/backend" test -- --runInBand

echo "[3/4] Building the web dashboard"
npm --prefix "$ROOT_DIR/webui" run build

echo "[4/4] Integration checks completed"
