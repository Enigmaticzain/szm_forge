#!/usr/bin/env bash
# SZM Forge engineering stack installer (Groups 2–18)
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REQ_DIR="$ROOT/ai_service/requirements"
PIP="${PIP:-pip3}"
TARGET_GROUPS=()
INSTALL_GPL=false
INSTALL_ML=false
INSTALL_DEV=false
INSTALL_SYSTEM=false

usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  --group N       Install requirement group N (2-18), repeatable
  --all           Install all Python groups (2-18)
  --gpl           Include GPL extras (gmsh, pygmsh, pygmo)
  --ml            Include heavy ML extras (deepxde, jax, neuraloperator)
  --dev           Include dev/test tools
  --system        Install apt system packages (CalculiX, OpenFOAM, etc.)
  -h, --help      Show this help

Examples:
  $(basename "$0") --group 2
  $(basename "$0") --all --gpl --dev
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --group) TARGET_GROUPS+=("$2"); shift 2 ;;
    --all) for i in $(seq 2 18); do TARGET_GROUPS+=("$i"); done; shift ;;
    --gpl) INSTALL_GPL=true; shift ;;
    --ml) INSTALL_ML=true; shift ;;
    --dev) INSTALL_DEV=true; shift ;;
    --system) INSTALL_SYSTEM=true; shift ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1"; usage; exit 1 ;;
  esac
done

if [[ ${#TARGET_GROUPS[@]} -eq 0 ]]; then
  TARGET_GROUPS=(2)
fi

group_file() {
  case "$1" in
    2) echo "group-02-cad.txt" ;;
    3) echo "group-03-mesh.txt" ;;
    4) echo "group-04-fea.txt" ;;
    5) echo "group-05-cfd.txt" ;;
    6) echo "group-06-physics-ml.txt" ;;
    7) echo "group-07-optimization.txt" ;;
    8) echo "group-08-materials.txt" ;;
    9) echo "group-09-iot.txt" ;;
    10) echo "group-10-robotics.txt" ;;
    11) echo "group-11-manufacturing.txt" ;;
    12) echo "group-12-viz.txt" ;;
    13) echo "group-13-kg.txt" ;;
    14) echo "group-14-web.txt" ;;
    15) echo "group-15-pipeline.txt" ;;
    16) echo "group-16-devops.txt" ;;
    17) echo "group-17-testing.txt" ;;
    18) echo "group-18-utils.txt" ;;
    *) echo "" ;;
  esac
}

echo "==> SZM Forge stack install"
INSTALLED=()
for g in "${TARGET_GROUPS[@]}"; do
  f="$(group_file "$g")"
  if [[ -z "$f" ]]; then
    echo "WARN: unknown group $g"
    continue
  fi
  path="$REQ_DIR/$f"
  if [[ -f "$path" ]]; then
    echo "==> pip install -r $path"
    "$PIP" install -r "$path"
    INSTALLED+=("$g")
  fi
done

if $INSTALL_GPL; then
  echo "==> WARNING: Installing GPL-licensed Python packages"
  "$PIP" install -r "$REQ_DIR/extras-gpl.txt"
fi

if $INSTALL_ML; then
  echo "==> Installing ML extras (may be large)"
  "$PIP" install -r "$REQ_DIR/extras-ml.txt" || echo "WARN: some ML extras failed"
fi

if $INSTALL_DEV; then
  "$PIP" install -r "$REQ_DIR/extras-dev.txt"
fi

if $INSTALL_SYSTEM; then
  if command -v apt-get >/dev/null 2>&1; then
    echo "==> Installing system packages (requires sudo)"
    sudo apt-get update
    sudo apt-get install -y \
      calculix-ccx code-aster openfoam elmerfem-csc \
      mosquitto linuxcnc cura-engine \
      docker.io docker-compose-v2 postgresql || true
  else
    echo "WARN: apt-get not found; skip --system on this OS"
  fi
fi

echo "==> Installed groups: ${INSTALLED[*]:-none}"
echo "==> Run: python3 $ROOT/scripts/verify_stack.py"
