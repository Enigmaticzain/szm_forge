# Settings Tab — Component Browser

## Overview

The Settings tab (⚙️) allows you to customize how the Component Browser works and behaves.

---

## Settings Options

### Auto-Save Options

**Enable Auto-Save**
- When enabled: Components are automatically saved when imported
- When disabled: You must manually save components
- Default: ON
- Use case: Quick workflow without manual saves

### Grid Settings

**Enable Snap to Grid**
- When enabled: Components snap to grid when placed
- When disabled: Free placement anywhere
- Default: OFF
- Use case: Organized, aligned component placement

**Grid Size**
- Controls the grid spacing
- Range: 0.01 to 1.0 units
- Default: 0.1 units
- Use case: Fine-tune alignment precision

### Default Material

**Material Selection**
- Steel (default)
- Aluminum
- Titanium
- Composite

- Applied to new primitive components
- Can be changed per component
- Affects simulation properties

### Display Settings

**Show Component Info**
- When enabled: Shows material and mass in library
- When disabled: Shows only component name
- Default: ON
- Use case: Quick reference of component properties

**Enable Physics Preview**
- When enabled: Shows real-time physics simulation
- When disabled: Static display only
- Default: OFF
- Use case: Preview how components will behave

---

## Common Workflows

### Workflow 1: Organized Assembly Building

1. Go to Settings tab
2. Enable "Snap to Grid"
3. Set Grid Size to 0.1
4. Go to Placement tab
5. Place components - they'll align automatically

### Workflow 2: Quick Component Import

1. Go to Settings tab
2. Enable "Auto-Save"
3. Go to Import tab
4. Import components - they save automatically
5. No need to manually save

### Workflow 3: Physics Analysis

1. Go to Settings tab
2. Enable "Physics Preview"
3. Go to Visualization tab
4. Select "Stress" or "Thermal" mode
5. See real-time physics preview

### Workflow 4: Material Assignment

1. Go to Settings tab
2. Select default material (e.g., Aluminum)
3. Go to Import tab
4. Create new primitives
5. They'll use selected material

---

## Reset to Defaults

**Reset Button**
- Restores all settings to factory defaults
- Useful if settings are misconfigured
- Cannot be undone (but you can reconfigure)

**Default Values:**
- Auto-Save: ON
- Snap to Grid: OFF
- Grid Size: 0.1
- Default Material: Steel
- Show Component Info: ON
- Physics Preview: OFF

---

## Tips & Tricks

### Tip 1: Use Grid for Precision
- Enable snap to grid for organized assemblies
- Adjust grid size based on component size
- Smaller grid = more precise placement

### Tip 2: Auto-Save Workflow
- Enable auto-save for quick imports
- Saves time when building large assemblies
- Disable if you want to review before saving

### Tip 3: Material Consistency
- Set default material before creating primitives
- Ensures consistent material properties
- Can override per component if needed

### Tip 4: Physics Preview
- Enable for real-time feedback
- Useful for stress/thermal analysis
- Disable if performance is slow

---

## Settings Persistence

**Current Session:**
- Settings apply immediately
- Affect all new components

**Between Sessions:**
- Settings are saved automatically
- Restored when you restart
- Use "Reset to Defaults" to clear

---

## Troubleshooting

### Problem: Components not snapping to grid

**Solution:**
1. Check "Enable Snap to Grid" is ON
2. Verify Grid Size is reasonable (0.1 is typical)
3. Try resetting to defaults

### Problem: Auto-Save not working

**Solution:**
1. Check "Enable Auto-Save" is ON
2. Verify you have write permissions
3. Check disk space available

### Problem: Physics preview too slow

**Solution:**
1. Disable "Enable Physics Preview"
2. Use static visualization instead
3. Check system performance

### Problem: Wrong material on new components

**Solution:**
1. Go to Settings tab
2. Select correct material
3. Create new components
4. Or change material per component

---

## FAQ

**Q: Can I change settings per component?**
A: Settings tab sets defaults. Individual components can be modified in Library tab.

**Q: Do settings save between sessions?**
A: Yes, settings are automatically saved and restored.

**Q: Can I have different grid sizes?**
A: Yes, adjust Grid Size slider to any value between 0.01 and 1.0.

**Q: What happens if I disable Auto-Save?**
A: Components won't save automatically. You'll need to manually save them.

**Q: Does Physics Preview affect performance?**
A: Yes, it can slow down the viewport. Disable if needed.

**Q: Can I reset individual settings?**
A: No, "Reset to Defaults" resets all settings at once.

---

## Best Practices

1. **Start with defaults** - They work well for most cases
2. **Enable snap to grid** - For organized assemblies
3. **Use auto-save** - For faster workflow
4. **Set material first** - Before creating primitives
5. **Test physics preview** - To verify behavior

---

## Settings Summary

| Setting | Default | Range | Purpose |
|---------|---------|-------|---------|
| Auto-Save | ON | ON/OFF | Auto-save components |
| Snap to Grid | OFF | ON/OFF | Align to grid |
| Grid Size | 0.1 | 0.01-1.0 | Grid spacing |
| Default Material | Steel | 4 options | Material for primitives |
| Show Component Info | ON | ON/OFF | Display properties |
| Physics Preview | OFF | ON/OFF | Real-time physics |

---

*SZM Forge Component Browser Settings | v0.1.0*
