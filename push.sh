#!/bin/bash
git config --global user.email "bot@szm-forge.local"
git config --global user.name "SZM Forge Bot"
git add "new ui/src/components/CircuitDesigner.tsx"
git add "new ui/src/components/ProjectHierarchy.tsx"
git add "new ui/src/store/ProjectContext.tsx"
git add "new ui/src/workshop/furnitureEngine.ts"
git add "new ui/demo.cjs"
git add "new ui/package.json"
git add "new ui/package-lock.json"
git commit -m "feat: Integrated custom motor builder and Project Hierarchy syncing"
git push
