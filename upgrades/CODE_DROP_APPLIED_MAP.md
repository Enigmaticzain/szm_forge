# Code Drop Applied Map

## 1) Fully materialized multi-file drops
- `code 3` extracted into runtime modules: 60 files, 2680 lines
  - Root folders: `module_010_1`, `module_010_3`, `module_010_4`, `module_020_1`, `module_020_2`, `module_110_4`, `module_120_1`, `module_130_1`, `module_150_1`, `module_150_2`

## 2) Structured imports from raw `code` and `code2`
- Imported snapshots: 22 files, 5197 lines
- Location: `upgrades/code_drop_imports/`
- Summary file: `upgrades/CODE_DROP_IMPORT_SUMMARY.md`

## 3) Active engine integrations (compiled)
- `src/Geometry/SolidBody.cpp`
  - Added center-of-mass volume integration
  - Upgraded manifold check to support higher-genus closed solids
- `src/Graphics/MeshGenerator.cpp`
  - Upgraded segment-count formula to high-fidelity conservative mode
  - Added vertex deduplication and smooth normal accumulation
- `src/Physics/CollisionDetection.cpp`
  - Replaced empty support function with proxy convex support
  - Added EPA placeholder penetration from body pose
  - Added practical swept CCD test (time-of-impact + hit normal)
- `src/Physics/CollisionDetection.hpp`
  - Fixed simplex push-front shift logic (removed uninitialized access risk)
- `src/Assembly/Mates.hpp`
  - Added alignment target-dot handling in residuals for anti-aligned mates

## 4) Validation
- Build: `cmake --build build-full --parallel 4` (pass)
- Tests: `./build-full/SZM_TestRunner` (55 passed, 0 failed)
