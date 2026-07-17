# External Projects (`code4` ... `code8`)

This folder gives clean, no-space entry points to the external source trees.

- `bullet3` -> `../code 4`
- `sofa` -> `../code 5`
- `simpylc` -> `../code 6`
- `freecad` -> `../code 7`
- `calculix_examples` -> `../code 8`

## Quick usage

### Bullet (`code 4`)
```bash
cmake -S external_projects/bullet3 -B external_projects/bullet3/build
cmake --build external_projects/bullet3/build --parallel
```

### SOFA (`code 5`)
```bash
cmake -S external_projects/sofa -B external_projects/sofa/build
cmake --build external_projects/sofa/build --parallel
```

### SimPyLC (`code 6`)
```bash
pip3 install -e external_projects/simpylc
```

### FreeCAD (`code 7`)
Use upstream build guide in `external_projects/freecad/README.md`.

### CalculiX examples (`code 8`)
```bash
python3 external_projects/calculix_examples/run_all.py
```

