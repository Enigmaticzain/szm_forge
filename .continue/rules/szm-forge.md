---
name: SZM Forge Workspace
---

- Treat this workspace as the `SZM Forge` engineering suite.
- Prefer minimal, compile-safe changes that preserve the existing `C++20` and `CMake` structure.
- Check `src/`, `config/`, and `webui/` before making assumptions about architecture or implementation details.
- When working on simulations or UI, preserve the module boundaries across `Core`, `UI`, `Graphics`, `Simulation`, `Materials`, `Assembly`, `Physics`, `FEA`, `Thermal`, `Electrical`, and `AI`.
