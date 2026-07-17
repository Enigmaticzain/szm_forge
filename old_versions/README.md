# SZM Forge — Archived UI Versions

This folder keeps **superseded** user-interface code after the **new ui/** React app became the primary desktop experience (May 2026).

## Current (active) UI

| Location | Description |
|----------|-------------|
| `../new ui/` | **Primary** — Vite + React 19, Theme 1 (Classic) + Theme 2 (Modern), 3D viewport, tooltips, codes/ integration help |
| `../src/Workshop3D/` | C++ workshop backend (parts, assemblies, simulation bridge) — still used by the engine |
| `../src/UI/` | ImGui fallback when `SZM_NATIVE_WORKSHOP_UI=1` or React build fails |

## Archived here

| Folder | What it was |
|--------|-------------|
| `react-app_webviewer_legacy/` | Earlier React dashboard under `src/WebViewer/react-app` (replaced by `new ui/`) |
| `imgui_native_workshop_manifest.md` | Index of native ImGui workshop panels (still in `src/` for fallback only) |

## Build flags

```bash
# Default full desktop — new React UI (both themes)
cmake -B build -DSZM_BUILD_FULL_DESKTOP=ON -DSZM_USE_REACT_WEBVIEW=ON

# Force legacy ImGui workshop
SZM_NATIVE_WORKSHOP_UI=1 ./build/SZM_Forge
```

## Dev server

```bash
cd "new ui" && npm run dev
# http://localhost:3000 — loaded by desktop webview when production build is missing
```

## C++ API bridge (new)

When `./build/SZM_Forge` runs in desktop mode, the React UI polls:

- `GET http://127.0.0.1:8000/api/status`
- `GET http://127.0.0.1:8000/api/simulation/status`

Status bar shows **BACKEND LIVE** when connected; **DEMO MODE** uses mock data only.
