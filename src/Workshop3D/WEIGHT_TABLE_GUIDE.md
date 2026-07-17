# ⚖️ Component Weight Table — User Guide

## Overview

The Component Weight Table Panel displays component weights in an easy-to-read table format in the 3D workspace.

**4 Tabs:**
1. **📚 Library** - All components in library with weights
2. **🎬 Scene** - Components currently in 3D scene
3. **📊 Statistics** - Weight analysis and distribution
4. **💾 Export** - Export weights to CSV file

---

## Library Tab (📚)

### View All Component Weights

**Features:**
- Table with all components and their weights
- Sort by name or weight
- Filter heavy components
- Total and average weight display

### How to Use

1. Click "📚 Library" tab
2. View component weights in table
3. Use "Refresh" button to update
4. Sort by name or weight
5. Filter heavy components if needed

### Sorting Options

- **Name** - Alphabetical order
- **Weight (Low→High)** - Lightest first
- **Weight (High→Low)** - Heaviest first

### Filter Heavy Components

1. Check "Show Only Heavy Components"
2. Set threshold (e.g., 10 kg)
3. Only components heavier than threshold shown

### Table Columns

| Column | Description |
|--------|-------------|
| Component Name | Name of the component |
| Weight (kg) | Weight in kilograms |
| Percentage | Percentage of total weight |

---

## Scene Tab (🎬)

### View Scene Component Weights

**Features:**
- Table of components in 3D scene
- Same sorting options as Library
- Total scene weight
- Real-time updates

### How to Use

1. Click "🎬 Scene" tab
2. View components currently in scene
3. See total weight of assembly
4. Sort and analyze as needed

### Scene Information

- **Total Scene Weight** - Sum of all components in scene
- **Component Count** - Number of components
- **Average Weight** - Average per component

---

## Statistics Tab (📊)

### Weight Analysis

**Library Statistics:**
- Total components
- Total weight
- Average weight
- Heaviest component
- Lightest component

**Scene Statistics:**
- Components in scene
- Total scene weight
- Average scene component weight

**Weight Distribution:**
- Scene vs Library percentage
- Helps understand assembly composition

### How to Use

1. Click "📊 Statistics" tab
2. Review library statistics
3. Check scene statistics
4. Analyze weight distribution

### Example Statistics

```
Library Statistics:
- Total Components: 15
- Total Weight: 250.5 kg
- Average Weight: 16.7 kg
- Heaviest: Engine Block (85.2 kg)
- Lightest: Fastener (0.05 kg)

Scene Statistics:
- Components in Scene: 8
- Total Scene Weight: 145.3 kg
- Average Scene Component: 18.2 kg

Weight Distribution:
- Scene vs Library: 57.9%
```

---

## Export Tab (💾)

### Export Weights to CSV

**Features:**
- Export to CSV format
- Compatible with Excel/Sheets
- Includes percentages
- Easy data analysis

### How to Use

1. Click "💾 Export" tab
2. Enter file path (e.g., `/path/to/weights.csv`)
3. Click "Export Library Weights" or "Export Scene Weights"
4. Open CSV in Excel or spreadsheet app

### CSV Format

```
Component Name,Weight (kg),Percentage
Engine Block,85.2,34.0
Transmission,45.3,18.1
Frame,32.1,12.8
...
TOTAL,250.5,100.0
```

### Using Exported Data

1. **In Excel:**
   - Open CSV file
   - Create charts
   - Analyze data
   - Print reports

2. **In Google Sheets:**
   - Upload CSV
   - Create visualizations
   - Share with team

3. **In Python:**
   - Read CSV with pandas
   - Perform analysis
   - Generate reports

---

## Common Workflows

### Workflow 1: Check Component Weights

1. Go to "📚 Library" tab
2. View all component weights
3. Sort by weight (heaviest first)
4. Identify heavy components

### Workflow 2: Analyze Scene Assembly

1. Add components to scene
2. Go to "🎬 Scene" tab
3. View total scene weight
4. Check weight distribution

### Workflow 3: Generate Weight Report

1. Go to "📊 Statistics" tab
2. Review all statistics
3. Go to "💾 Export" tab
4. Export to CSV
5. Open in Excel
6. Create charts and report

### Workflow 4: Find Heaviest Components

1. Go to "📚 Library" tab
2. Sort by "Weight (High→Low)"
3. View heaviest components first
4. Identify optimization opportunities

---

## Tips & Tricks

### Tip 1: Use Sorting
- Sort by weight to find heavy components
- Helps identify optimization opportunities
- Useful for weight reduction

### Tip 2: Filter Heavy Components
- Set threshold to find components above certain weight
- Example: Show only components > 50 kg
- Useful for focusing on major components

### Tip 3: Export for Analysis
- Export to CSV for detailed analysis
- Create charts in Excel
- Share with team members

### Tip 4: Monitor Scene Weight
- Check scene weight regularly
- Ensure assembly meets weight targets
- Identify unbalanced designs

### Tip 5: Compare Library vs Scene
- Use Statistics tab to compare
- See what percentage of library is in scene
- Helps understand assembly composition

---

## Troubleshooting

### Problem: Weights showing as 0

**Solution:**
1. Ensure components have mass assigned
2. Check material properties
3. Refresh the table
4. Verify component geometry

### Problem: Export not working

**Solution:**
1. Check file path is valid
2. Ensure write permissions
3. Try different file path
4. Check disk space

### Problem: Scene weights not updating

**Solution:**
1. Click "Refresh" button
2. Add/remove components
3. Check active assembly
4. Verify components in scene

### Problem: Can't see all components

**Solution:**
1. Scroll down in table
2. Disable "Show Only Heavy Components" filter
3. Adjust window size
4. Use Export to see all data

---

## FAQ

**Q: How are weights calculated?**
A: Weights are calculated from component mass property, which is based on material density and volume.

**Q: Can I edit component weights?**
A: Edit weights in Component Browser Library tab or Part Editor.

**Q: What if component has no weight?**
A: Assign material and verify geometry in Part Editor.

**Q: Can I export scene weights?**
A: Yes, use "Export Scene Weights" button in Export tab.

**Q: How do I use exported CSV?**
A: Open in Excel, Google Sheets, or any spreadsheet application.

**Q: Can I sort by multiple columns?**
A: Currently supports single-column sorting. Use Export for advanced analysis.

**Q: What's the weight unit?**
A: All weights are in kilograms (kg).

**Q: Can I see weight per instance?**
A: Scene tab shows all instances with their weights.

---

## Best Practices

1. **Keep weights updated** - Ensure all components have correct mass
2. **Use sorting** - Sort by weight to find optimization opportunities
3. **Export regularly** - Export data for documentation
4. **Monitor assembly** - Check scene weight as you build
5. **Compare metrics** - Use Statistics tab to analyze

---

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Refresh | F5 |
| Export | Ctrl+E |
| Sort | Click column header |

---

## Integration with Other Panels

**Component Browser Panel:**
- Import components
- View in Library tab
- Weights shown in Weight Table

**3D Workspace:**
- Add components to scene
- Weights updated in Scene tab
- Real-time weight tracking

**Statistics Tab:**
- Analyze weight distribution
- Compare library vs scene
- Generate reports

---

*SZM Forge Component Weight Table | v0.1.0*
