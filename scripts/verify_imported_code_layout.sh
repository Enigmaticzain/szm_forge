#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root_dir"

echo "[check] validating core placement from code/code2 ..."
required_cpp_paths=(
  "src/Core/Application.hpp"
  "src/Input/InputManager.cpp"
  "src/Graphics/GraphicsContext.hpp"
  "src/UI/UIManager.cpp"
  "src/Data/SceneGraph.hpp"
  "src/Math/ConstraintSolver.cpp"
  "src/Geometry/SolidBody.cpp"
  "src/Materials/MaterialDatabase.cpp"
  "src/Assembly/KinematicSolver.cpp"
  "src/Physics/CollisionDetection.cpp"
  "src/FEA/LinearSolver.cpp"
)

for path in "${required_cpp_paths[@]}"; do
  [[ -f "$path" ]] || { echo "[error] missing: $path"; exit 1; }
done

echo "[check] validating extracted TypeScript layout from code 3 ..."
while IFS= read -r marker; do
  rel_path="$(sed -E 's/^===== FILE: (.*) =====$/\1/' <<<"$marker")"
  rel_path="${rel_path#/}"
  [[ -f "$rel_path" ]] || { echo "[error] missing: $rel_path"; exit 1; }
done < <(grep '^===== FILE: ' "code 3")

echo "[check] validating external project links for code4..code8 ..."
declare -A links=(
  ["external_projects/bullet3"]="code 4"
  ["external_projects/sofa"]="code 5"
  ["external_projects/simpylc"]="code 6"
  ["external_projects/freecad"]="code 7"
  ["external_projects/calculix_examples"]="code 8"
)

for link in "${!links[@]}"; do
  [[ -L "$link" ]] || { echo "[error] not a symlink: $link"; exit 1; }
  target="$(readlink "$link")"
  [[ "$target" == "../${links[$link]}" ]] || {
    echo "[error] $link points to '$target' (expected '../${links[$link]}')"
    exit 1
  }
done

echo "[ok] imported code layout is valid."
