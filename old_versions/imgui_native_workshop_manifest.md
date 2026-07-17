# Native ImGui Workshop UI (fallback)

Still compiled from `src/` when React webview is disabled. Archived here as documentation only.

## Panels

- `src/UI/UIManager.cpp` — dockspace, workspaces, codes/ sidebar
- `src/Workshop3D/WorkshopPanels.cpp` — Library, Assembly, Viewport, Simulation
- `src/Workshop3D/ComponentBrowserPanel.cpp` — tabbed browser
- `src/UI/UITooltipHelper.cpp` — C++ tooltips
- `src/UI/WorkshopTooltips.hpp` — tooltip strings

## Enable fallback

```bash
cmake -B build -DSZM_BUILD_FULL_DESKTOP=ON -DSZM_USE_REACT_WEBVIEW=OFF
export SZM_NATIVE_WORKSHOP_UI=1
./build/SZM_Forge
```
