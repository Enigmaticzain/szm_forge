# Code Import Map (`code` ... `code8`)

This project now has a stable, non-destructive layout for the added sources.

## 1) `code` (C++ core bundle)

Integrated into the `src/` C++ module tree (Core/Input/Graphics/UI/Data/Math/Geometry/etc.), where it is compiled by `CMakeLists.txt`.

## 2) `code2` (advanced C++ bundle)

Integrated into advanced module implementations under `src/` (Geometry/Physics/Assembly/Materials/FEA/etc.), compiled through existing module CMake files.

## 3) `code 3` (TypeScript module bundle)

Extracted into:

- `module_010_1/`
- `module_010_3/`
- `module_010_4/`
- `module_020_1/`
- `module_020_2/`
- `module_110_4/`
- `module_120_1/`
- `module_130_1/`
- `module_150_1/`
- `module_150_2/`

## 4) `code 4` ... `code 8` (external projects)

Added stable execution paths in `external_projects/`:

- `external_projects/bullet3` -> `code 4`
- `external_projects/sofa` -> `code 5`
- `external_projects/simpylc` -> `code 6`
- `external_projects/freecad` -> `code 7`
- `external_projects/calculix_examples` -> `code 8`

These are symbolic links, so original source folders remain untouched.

## Verification

Run:

```bash
./scripts/verify_imported_code_layout.sh
```

