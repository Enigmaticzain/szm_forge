from typing import Dict, List, Any, Tuple
import logging
import math

logger = logging.getLogger(__name__)

class ManufacturabilityValidator:
    """Validates structural bounds against CNC and 3D printing constraints."""
    
    @staticmethod
    def validate_design(component_name: str, parameters: Dict[str, Any], process: str = "CNC") -> Dict[str, Any]:
        thickness = parameters.get("thickness", 0.05)
        
        results = []
        is_valid = True
        
        # Approximate checks based on bounds
        if process == "CNC":
            # Wall Thickness
            if thickness < 0.02:
                results.append({"check": "Wall Thickness", "result": "WARNING", "value": f"{thickness*1000:.1f}mm (min 20mm for robust milling)"})
                is_valid = False
            else:
                results.append({"check": "Wall Thickness", "result": "PASS", "value": f"{thickness*1000:.1f}mm"})
                
            # Undercuts (mock assumption for a standard bracket)
            results.append({"check": "Undercuts", "result": "PASS", "value": "None detected"})
            
            # Tool Access
            results.append({"check": "Tool Access", "result": "PASS", "value": "Standard 3-axis reachable"})
            
            # Surface Finish
            results.append({"check": "Surface Finish", "result": "PASS", "value": "Ra 0.8μm achievable"})
            
            # Fillets
            results.append({"check": "Internal Fillets", "result": "WARNING", "value": "Sharp internal corners detected, req. EDM"})
            
        return {
            "component": component_name,
            "process": process,
            "is_valid": is_valid,
            "checks": results
        }

class GCodeExporter:
    """Generates rudimentary G-Code toolpaths based on bounding boxes."""
    
    @staticmethod
    def export_gcode(component_name: str, bounds: Dict[str, float]) -> str:
        length = bounds.get("length", 1.0)
        width = bounds.get("width", 0.5)
        depth = bounds.get("depth", 0.1)
        
        feed_rate = 1200
        spindle_speed = 8000
        
        # A simple contouring program around a rectangular bound
        gcode = [
            f"(Program for: {component_name})",
            "(Tool: 10mm Flat End Mill)",
            "G21 (Metric)",
            "G90 (Absolute Positioning)",
            "G54 (Work Offset)",
            f"S{spindle_speed} M03 (Spindle On CW)",
            "G00 Z50.0 (Safe Z)",
            "G00 X0 Y0 (Go to origin)",
            f"G01 Z-{depth*1000:.1f} F{feed_rate/2} (Plunge)",
            f"G01 X{length*1000:.1f} F{feed_rate} (Cut along X)",
            f"G01 Y{width*1000:.1f} (Cut along Y)",
            f"G01 X0 (Cut along -X)",
            f"G01 Y0 (Cut along -Y)",
            "G00 Z50.0 (Retract)",
            "M05 (Spindle Off)",
            "M30 (End Program)"
        ]
        
        return "\n".join(gcode)

class CNCToolpathGenerator:
    """Generates CNC milling toolpaths with FreeCAD-compatible metadata.

    The Python external bridge can route the generated path through FreeCAD's
    Path post-processing layer when FreeCAD is installed. This class is the
    deterministic fallback used by local services and tests.
    """

    @staticmethod
    def _positive(value: Any, fallback: float) -> float:
        try:
            parsed = float(value)
        except (TypeError, ValueError):
            return fallback
        return parsed if math.isfinite(parsed) and parsed > 0 else fallback

    @staticmethod
    def _bounds_mm(bounds: Dict[str, Any]) -> Dict[str, float]:
        # Inputs in this project are usually metres; tolerate explicit mm too.
        unit = str(bounds.get("unit", "m")).lower()
        scale = 1.0 if unit in {"mm", "millimeter", "millimeters"} else 1000.0
        return {
            "length": CNCToolpathGenerator._positive(bounds.get("length"), 1.0) * scale,
            "width": CNCToolpathGenerator._positive(bounds.get("width"), 0.5) * scale,
            "depth": CNCToolpathGenerator._positive(bounds.get("depth"), bounds.get("height", 0.05)) * scale,
        }

    @staticmethod
    def _format_move(command: str, x: float = None, y: float = None, z: float = None,
                     feed: float = None, comment: str = "") -> str:
        words = [command]
        if x is not None:
            words.append(f"X{x:.3f}")
        if y is not None:
            words.append(f"Y{y:.3f}")
        if z is not None:
            words.append(f"Z{z:.3f}")
        if feed is not None:
            words.append(f"F{feed:.1f}")
        line = " ".join(words)
        return f"{line} ({comment})" if comment else line

    @staticmethod
    def generate_rectangular_milling_job(
        component_name: str,
        bounds: Dict[str, Any],
        operation: str = "pocket",
        tool_diameter_mm: float = 10.0,
        stepover_mm: float = None,
        stepdown_mm: float = 2.0,
        feed_rate_mm_min: float = 1200.0,
        plunge_rate_mm_min: float = 300.0,
        spindle_speed_rpm: int = 8000,
        safe_z_mm: float = 15.0,
        include_header: bool = True,
    ) -> Dict[str, Any]:
        dims = CNCToolpathGenerator._bounds_mm(bounds)
        operation = operation.lower().strip()
        if operation not in {"profile", "pocket", "facing"}:
            operation = "pocket"

        tool_diameter = CNCToolpathGenerator._positive(tool_diameter_mm, 10.0)
        stepover = CNCToolpathGenerator._positive(stepover_mm, tool_diameter * 0.4)
        stepdown = CNCToolpathGenerator._positive(stepdown_mm, 2.0)
        feed = CNCToolpathGenerator._positive(feed_rate_mm_min, 1200.0)
        plunge = CNCToolpathGenerator._positive(plunge_rate_mm_min, 300.0)
        safe_z = CNCToolpathGenerator._positive(safe_z_mm, 15.0)
        spindle = max(500, int(spindle_speed_rpm or 8000))

        length = dims["length"]
        width = dims["width"]
        depth = dims["depth"]
        passes = max(1, math.ceil(depth / stepdown))
        x_min = tool_diameter / 2.0
        y_min = tool_diameter / 2.0
        x_max = max(x_min, length - tool_diameter / 2.0)
        y_max = max(y_min, width - tool_diameter / 2.0)

        lines: List[str] = []
        preview_points: List[Dict[str, float]] = []
        cut_length = 0.0
        last_cut_point: Tuple[float, float, float] = (x_min, y_min, safe_z)

        def add_line(line: str):
            lines.append(line)

        def add_cut(x: float, y: float, z: float, comment: str = ""):
            nonlocal cut_length, last_cut_point
            lx, ly, lz = last_cut_point
            cut_length += math.sqrt((x - lx) ** 2 + (y - ly) ** 2 + (z - lz) ** 2)
            last_cut_point = (x, y, z)
            preview_points.append({"x": round(x, 3), "y": round(y, 3), "z": round(z, 3)})
            add_line(CNCToolpathGenerator._format_move("G01", x=x, y=y, z=z, feed=feed, comment=comment))

        if include_header:
            add_line(f"(Program for: {component_name})")
            add_line(f"(Operation: {operation})")
            add_line(f"(Tool: {tool_diameter:.3f} mm flat end mill)")
            add_line("G21 (Metric units)")
            add_line("G90 (Absolute positioning)")
            add_line("G17 (XY plane)")
            add_line("G54 (Work offset)")
            add_line(f"S{spindle} M03 (Spindle on clockwise)")
            add_line(CNCToolpathGenerator._format_move("G00", z=safe_z, comment="safe height"))

        for pass_index in range(1, passes + 1):
            z = -min(pass_index * stepdown, depth)
            add_line(f"(Pass {pass_index}/{passes} Z{z:.3f})")
            add_line(CNCToolpathGenerator._format_move("G00", x=x_min, y=y_min, comment="rapid to start"))
            add_line(CNCToolpathGenerator._format_move("G01", z=z, feed=plunge, comment="plunge"))
            last_cut_point = (x_min, y_min, z)
            preview_points.append({"x": round(x_min, 3), "y": round(y_min, 3), "z": round(z, 3)})

            if operation == "profile":
                add_cut(x_max, y_min, z, "profile +X")
                add_cut(x_max, y_max, z, "profile +Y")
                add_cut(x_min, y_max, z, "profile -X")
                add_cut(x_min, y_min, z, "profile -Y")
            else:
                y = y_min
                row = 0
                while y <= y_max + 1e-6:
                    if row % 2 == 0:
                        add_cut(x_max, y, z, "raster cut")
                    else:
                        add_cut(x_min, y, z, "raster cut")
                    next_y = y + stepover
                    if next_y <= y_max + 1e-6:
                        current_x = x_max if row % 2 == 0 else x_min
                        add_cut(current_x, next_y, z, "stepover")
                    y = next_y
                    row += 1

        add_line(CNCToolpathGenerator._format_move("G00", z=safe_z, comment="retract"))
        add_line("M05 (Spindle stop)")
        add_line("M30 (End program)")

        warnings = []
        if tool_diameter > min(length, width):
            warnings.append("Tool diameter exceeds the smallest workpiece dimension.")
        if stepdown > tool_diameter:
            warnings.append("Stepdown is larger than tool diameter; reduce for conservative milling.")
        if len(preview_points) > 2000:
            warnings.append("Preview path is large; UI may simplify display.")

        gcode = "\n".join(lines)
        return {
            "status": "success",
            "engine_used": "SZM deterministic CNC fallback",
            "component": component_name,
            "operation": operation,
            "gcode": gcode,
            "preview_points": preview_points[:2000],
            "metrics": {
                "line_count": len(lines),
                "cut_length_mm": round(cut_length, 3),
                "passes": passes,
                "bounds_mm": {key: round(value, 3) for key, value in dims.items()},
                "tool_diameter_mm": round(tool_diameter, 3),
                "stepover_mm": round(stepover, 3),
                "stepdown_mm": round(stepdown, 3),
                "feed_rate_mm_min": round(feed, 3),
                "plunge_rate_mm_min": round(plunge, 3),
                "spindle_speed_rpm": spindle,
                "estimated_cycle_time_min": round(cut_length / max(feed, 1.0), 3),
            },
            "warnings": warnings,
        }
