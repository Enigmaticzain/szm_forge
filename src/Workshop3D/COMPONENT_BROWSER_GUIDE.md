# Workshop Component Browser — User Guide

## Overview

The Component Browser Panel provides an easy way to:
- ✅ View all components in the library
- ✅ Import new components from files
- ✅ Create primitive components
- ✅ Place components in the 3D scene
- ✅ Visualize and analyze components

---

## Getting Started

### Opening the Component Browser

The Component Browser is available as a panel in the Workshop3D interface:

```
Workshop → Component Browser
```

Or access it through the UI panel system.

---

## Features

### 1. Library Tab (📚 Library)

**View all components in your library**

- Search components by name
- See component properties (material, category, mass)
- Select components for placement
- View total component count

**How to use:**
1. Click the "📚 Library" tab
2. Use the search box to filter components
3. Click "Select" on any component to choose it
4. Go to "📍 Placement" tab to add it to the scene

---

### 2. Import Tab (📥 Import)

**Add new components to your library**

#### Option A: Import from File

1. Enter file path in "File Path" field
2. Click "Browse" to select file (or type path)
3. Click "Import STEP" or "Import GEO"
4. Component appears in Library tab

**Supported formats:**
- STEP (.step, .stp)
- GEO (.geo)

#### Option B: Create Primitive

1. Enter component name
2. Set size (width, height, depth)
3. Choose primitive type:
   - Box
   - Cylinder
   - Sphere
   - Cone
4. Click "Create Component"

**Example:**
```
Name: Engine Block
Size: 0.5 x 0.5 x 0.5
Type: Box
```

---

### 3. Placement Tab (📍 Placement)

**Add selected component to the 3D scene**

#### Manual Placement

1. Select component from Library tab
2. Set Position (X, Y, Z coordinates)
3. Set Rotation (degrees)
4. Set Scale (1.0 = normal size)
5. Click "Add to Scene"

**Example:**
```
Position: 0, 0, 0
Rotation: 0, 0, 0
Scale: 1.0
```

#### Auto Placement

1. Check "Auto Placement" checkbox
2. Click "Add to Scene"
3. Component automatically positioned to avoid overlap

**Tip:** Use auto placement for quick assembly building!

---

### 4. Visualization Tab (🎨 Visualization)

**Control how components are displayed**

#### Display Options

- ✅ Show Grid - Display reference grid
- ✅ Show Labels - Show component names
- ✅ Show Bounds - Show bounding boxes

#### Visualization Modes

- **Solid** - Normal 3D view
- **Wireframe** - Show edges only
- **Stress** - Color by stress analysis
- **Thermal** - Color by temperature
- **Displacement** - Color by deformation

#### Heatmap Range

For analysis modes (Stress, Thermal, Displacement):
- Set minimum value
- Set maximum value
- Colors scale between min and max

**Example:**
```
Mode: Stress
Min: 0 MPa
Max: 100 MPa
```

#### Scene Statistics

View current scene information:
- Components in Library
- Number of Assemblies
- Instances in Scene

---

## Common Workflows

### Workflow 1: Build an Assembly from Scratch

1. **Create components** (Import tab)
   - Import STEP files or create primitives
   
2. **View library** (Library tab)
   - Verify components are available
   
3. **Place components** (Placement tab)
   - Select each component
   - Use auto placement
   - Click "Add to Scene"
   
4. **Visualize** (Visualization tab)
   - Enable grid and labels
   - View final assembly

### Workflow 2: Analyze Stress

1. **Build assembly** (steps 1-3 above)

2. **Switch to Stress visualization** (Visualization tab)
   - Mode: Stress
   - Set heatmap range
   
3. **View results**
   - Red = high stress
   - Blue = low stress

### Workflow 3: Import and Modify

1. **Import component** (Import tab)
   - Browse and select STEP file
   - Click "Import STEP"
   
2. **View in library** (Library tab)
   - Search for imported component
   - Verify properties
   
3. **Place in scene** (Placement tab)
   - Select component
   - Adjust position/rotation
   - Add to scene

---

## Tips & Tricks

### Tip 1: Use Search Filter
- Type partial name to find components quickly
- Example: type "engine" to find all engine parts

### Tip 2: Auto Placement
- Saves time when building assemblies
- Automatically avoids component overlap
- Perfect for quick prototyping

### Tip 3: Visualization Modes
- Use Wireframe to see internal structure
- Use Stress to identify weak points
- Use Thermal to see heat distribution

### Tip 4: Heatmap Range
- Adjust min/max to focus on specific range
- Example: Set 0-50 to see only low stress areas

### Tip 5: Component Properties
- Check material and mass in Library tab
- Use this info for simulation setup

---

## Troubleshooting

### Problem: Components not showing in Library

**Solution:**
1. Check Import tab
2. Verify file path is correct
3. Try importing again
4. Check file format (STEP or GEO)

### Problem: Can't add component to scene

**Solution:**
1. Make sure component is selected (Library tab)
2. Check position values are reasonable
3. Try auto placement instead
4. Verify assembly exists

### Problem: Visualization not updating

**Solution:**
1. Check visualization mode is correct
2. Verify heatmap range is set
3. Try switching modes and back
4. Refresh scene

### Problem: Component appears too small/large

**Solution:**
1. Go to Placement tab
2. Adjust Scale value
3. Increase for larger, decrease for smaller
4. Click "Add to Scene" again

---

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Search | Ctrl+F |
| Import | Ctrl+I |
| Add to Scene | Ctrl+A |
| Refresh | F5 |

---

## FAQ

**Q: Can I import multiple components at once?**
A: Currently one at a time. Use the Import tab for each file.

**Q: What file formats are supported?**
A: STEP (.step, .stp) and GEO (.geo) formats.

**Q: Can I edit components after importing?**
A: Yes, use the Part Editor panel for modifications.

**Q: How do I delete a component?**
A: Select it in Library tab and press Delete.

**Q: Can I save my assembly?**
A: Yes, use File → Save Workshop.

**Q: How do I undo placement?**
A: Use Edit → Undo or Ctrl+Z.

---

## Best Practices

1. **Organize by category** - Use consistent naming
2. **Set materials correctly** - Affects simulation
3. **Use auto placement** - Faster assembly building
4. **Check bounds** - Avoid overlapping components
5. **Save frequently** - Don't lose your work

---

## Advanced Features

### Custom Primitives

Create custom shapes:
1. Import tab → Create Primitive
2. Set custom size values
3. Choose primitive type
4. Name appropriately

### Batch Import

To import multiple files:
1. Place all files in one folder
2. Use Import tab for each file
3. Or use command line tools

### Material Assignment

Set material for components:
1. Library tab → Select component
2. View material in card
3. Use Part Editor to change

---

## Support

For issues or questions:
- Check Troubleshooting section
- Review FAQ
- Consult Part Editor guide
- Check Workshop documentation

---

*SZM Forge Workshop Component Browser | v0.1.0*
