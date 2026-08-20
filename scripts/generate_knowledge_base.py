#!/usr/bin/env python3
"""Generate data/knowledge/*.json from engineering reference data."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "data" / "knowledge"
OUT.mkdir(parents=True, exist_ok=True)

SCHEMA = "1.0.0"


def mat(
    id_, name, category, E, nu, yield_m, uts, rho, alpha=0.0, k=0.0, tags=None, notes=""
):
    return {
        "id": id_,
        "name": name,
        "category": category,
        "tags": tags or [],
        "youngsModulus_GPa": E,
        "poissonsRatio": nu,
        "yieldStrength_MPa": yield_m,
        "ultimateStrength_MPa": uts,
        "density_kg_m3": rho,
        "thermalExpansion_1_K": alpha,
        "thermalConductivity_W_mK": k,
        "notes": notes,
        "source": "Machine Creation KB Part 1",
    }


materials = [
    # Preserve existing simulation IDs
    mat("MAT-STEEL-STRUCT", "Structural Steel (AISI 1020)", "Ferrous / Low Carbon Steel",
        200, 0.30, 250, 400, 7850, 1.2e-5, 50, ["steel", "AISI 1020", "weldable"]),
    mat("MAT-AL-6061-T6", "6061-T6 Aluminum", "Non-Ferrous / Aluminum",
        68.9, 0.33, 276, 310, 2700, 2.32e-5, 167, ["aluminum", "6061", "machinable"]),
    mat("MAT-TI-6AL4V", "Ti-6Al-4V (Grade 5)", "Non-Ferrous / Titanium",
        113.8, 0.342, 880, 950, 4430, 8.6e-6, 6.7, ["titanium", "aerospace", "biocompatible"]),
    mat("MAT-CU-C110", "C110 Copper", "Non-Ferrous / Copper",
        117, 0.34, 69, 220, 8960, 1.68e-5, 385, ["copper", "electrical"]),
    # Ferrous
    mat("MAT-IRON-PURE", "Pure Iron", "Ferrous / Pure Iron", 211, 0.29, 540, 540, 7874,
        11.8e-6, 80, ["iron", "BCC"]),
    mat("MAT-CAST-GRAY", "Gray Cast Iron", "Ferrous / Cast Iron", 110, 0.26, 200, 350, 7200,
        10.8e-6, 50, ["cast iron", "gray", "damping"]),
    mat("MAT-CAST-DUCTILE", "Ductile/Nodular Cast Iron", "Ferrous / Cast Iron", 169, 0.28, 550, 900, 7100,
        11.0e-6, 36, ["cast iron", "ductile"]),
    mat("MAT-STEEL-1010", "AISI 1010 Low Carbon Steel", "Ferrous / Low Carbon Steel",
        200, 0.30, 205, 365, 7830, 12.2e-6, 49.8, ["AISI 1010", "soft", "weldable"]),
    mat("MAT-STEEL-1045", "AISI 1045 Medium Carbon Steel", "Ferrous / Medium Carbon Steel",
        200, 0.29, 530, 630, 7850, 11.7e-6, 49.8, ["AISI 1045", "shafts", "gears"]),
    mat("MAT-STEEL-1080", "AISI 1080 High Carbon Steel", "Ferrous / High Carbon Steel",
        200, 0.29, 800, 1000, 7850, 11.0e-6, 49.0, ["AISI 1080", "springs"]),
    mat("MAT-STEEL-4130", "4130 Chromoly", "Ferrous / Alloy Steel", 205, 0.29, 460, 670, 7850,
        12.3e-6, 42.7, ["chromoly", "4130", "aerospace"]),
    mat("MAT-STEEL-4140", "4140 Alloy Steel", "Ferrous / Alloy Steel", 205, 0.29, 655, 1020, 7850,
        12.3e-6, 42.7, ["4140", "gears", "shafts"]),
    mat("MAT-STEEL-4340", "4340 Alloy Steel", "Ferrous / Alloy Steel", 205, 0.29, 710, 1080, 7850,
        12.3e-6, 44.5, ["4340", "high strength"]),
    mat("MAT-STEEL-52100", "52100 Bearing Steel", "Ferrous / Tool Steel", 210, 0.30, 1500, 1900, 7810,
        11.5e-6, 46, ["bearing steel", "52100"]),
    mat("MAT-SS-304", "304 Stainless Steel", "Ferrous / Stainless", 193, 0.29, 215, 505, 8000,
        17.3e-6, 16.2, ["304", "18Cr-8Ni", "corrosion"]),
    mat("MAT-SS-316", "316 Stainless Steel", "Ferrous / Stainless", 193, 0.29, 290, 580, 8000,
        16.0e-6, 16.3, ["316", "marine", "Mo"]),
    mat("MAT-SS-17-4PH", "17-4 PH Stainless", "Ferrous / Stainless", 196, 0.27, 1170, 1310, 7800,
        10.8e-6, 17.9, ["17-4PH", "aerospace"]),
    mat("MAT-TOOL-H13", "H13 Hot Work Tool Steel", "Ferrous / Tool Steel", 210, 0.30, 1200, 1380, 7800,
        11.5e-6, 28, ["H13", "dies"]),
    mat("MAT-TOOL-D2", "D2 Cold Work Tool Steel", "Ferrous / Tool Steel", 210, 0.30, 1400, 1520, 7700,
        11.0e-6, 20, ["D2", "punches"]),
    # Aluminum
    mat("MAT-AL-2024-T3", "2024-T3 Aluminum", "Non-Ferrous / Aluminum", 73.1, 0.33, 345, 483, 2780,
        23.2e-6, 121, ["2024", "Al-Cu", "aerospace"]),
    mat("MAT-AL-5083", "5083 Aluminum (Marine)", "Non-Ferrous / Aluminum", 70, 0.33, 145, 315, 2660,
        23.8e-6, 117, ["5083", "marine"]),
    mat("MAT-AL-7075-T6", "7075-T6 Aluminum", "Non-Ferrous / Aluminum", 71.7, 0.33, 503, 572, 2810,
        23.6e-6, 130, ["7075", "aerospace"]),
    mat("MAT-AL-6063-T5", "6063-T5 Aluminum", "Non-Ferrous / Aluminum", 68.9, 0.33, 145, 190, 2700,
        23.4e-6, 201, ["6063", "extrusions"]),
    # Titanium & others
    mat("MAT-TI-GRADE4", "Titanium Grade 4", "Non-Ferrous / Titanium", 105, 0.34, 480, 550, 4506,
        8.6e-6, 7.2, ["titanium", "pure"]),
    mat("MAT-BRASS-7030", "70/30 Brass", "Non-Ferrous / Copper Alloy", 110, 0.34, 124, 320, 8520,
        19.9e-6, 120, ["brass", "Cu-Zn"]),
    mat("MAT-BRONZE-AL", "Aluminum Bronze", "Non-Ferrous / Copper Alloy", 110, 0.34, 250, 655, 7640,
        16.3e-6, 59, ["bronze", "bearings"]),
    mat("MAT-BECU", "Beryllium Copper", "Non-Ferrous / Copper Alloy", 131, 0.30, 965, 1380, 8250,
        17.0e-6, 105, ["beryllium copper", "non-sparking"]),
    mat("MAT-IN718", "Inconel 718", "Non-Ferrous / Nickel Superalloy", 200, 0.29, 1100, 1375, 8190,
        13.0e-6, 11.4, ["Inconel", "718", "aerospace"]),
    mat("MAT-IN625", "Inconel 625", "Non-Ferrous / Nickel Superalloy", 205, 0.31, 690, 965, 8440,
        12.8e-6, 9.8, ["Inconel", "625", "1000C service"]),
    mat("MAT-HAST-C276", "Hastelloy C-276", "Non-Ferrous / Nickel Superalloy", 205, 0.31, 380, 760, 8890,
        11.2e-6, 10.2, ["Hastelloy", "chemical"]),
    mat("MAT-MG-AZ31B", "AZ31B Magnesium", "Non-Ferrous / Magnesium", 45, 0.35, 200, 260, 1738,
        26.0e-6, 96, ["magnesium", "lightweight"]),
    mat("MAT-W-PURE", "Tungsten", "Non-Ferrous / Refractory", 411, 0.28, 750, 980, 19300,
        4.5e-6, 173, ["tungsten", "cutting tools"]),
    mat("MAT-CO-STELLITE6", "Stellite 6 Cobalt Alloy", "Non-Ferrous / Cobalt", 240, 0.30, 450, 950, 8400,
        14.0e-6, 15, ["Stellite", "wear"]),
    # Polymers
    mat("MAT-ABS", "ABS", "Polymer / Thermoplastic", 2.3, 0.35, 40, 50, 1050,
        90e-6, 0.17, ["ABS", "impact resistant"]),
    mat("MAT-PA66", "Nylon PA66", "Polymer / Thermoplastic", 3.0, 0.39, 70, 82, 1140,
        80e-6, 0.25, ["nylon", "PA66", "gears"]),
    mat("MAT-PEEK", "PEEK", "Polymer / High Performance", 3.6, 0.40, 91, 100, 1320,
        47e-6, 0.25, ["PEEK", "250C service"]),
    mat("MAT-PC", "Polycarbonate", "Polymer / Thermoplastic", 2.3, 0.37, 62, 70, 1200,
        65e-6, 0.20, ["polycarbonate", "optical"]),
    mat("MAT-POM", "POM (Delrin/Acetal)", "Polymer / Thermoplastic", 3.0, 0.35, 65, 70, 1410,
        110e-6, 0.31, ["POM", "Delrin", "gears"]),
    mat("MAT-PTFE", "PTFE (Teflon)", "Polymer / Thermoplastic", 0.5, 0.46, 12, 25, 2200,
        135e-6, 0.25, ["PTFE", "low friction"]),
    mat("MAT-UHMWPE", "UHMWPE", "Polymer / Thermoplastic", 0.8, 0.46, 20, 40, 930,
        200e-6, 0.42, ["UHMWPE", "wear", "armor"]),
    mat("MAT-PP", "Polypropylene", "Polymer / Thermoplastic", 1.5, 0.42, 32, 40, 905,
        100e-6, 0.22, ["polypropylene", "hinges"]),
    # Composites
    mat("MAT-CFRP-UD", "CFRP Unidirectional", "Composite / Fiber Reinforced", 135, 0.30, 1500, 1500, 1600,
        -0.5e-6, 5, ["CFRP", "carbon fiber", "T300"]),
    mat("MAT-GFRP", "GFRP (E-Glass)", "Composite / Fiber Reinforced", 45, 0.25, 200, 400, 1850,
        6e-6, 0.35, ["fiberglass", "GFRP"]),
    mat("MAT-KEVLAR49", "Kevlar 49 Aramid", "Composite / Fiber Reinforced", 70, 0.36, 3000, 3000, 1440,
        -4e-6, 0.04, ["Kevlar", "ballistic"]),
    # Ceramics
    mat("MAT-AL2O3-96", "Alumina 96%", "Ceramic / Technical", 370, 0.22, 310, 310, 3950,
        8.0e-6, 24, ["alumina", "Al2O3", "wear"]),
    mat("MAT-SIC", "Silicon Carbide", "Ceramic / Technical", 410, 0.17, 410, 550, 3210,
        4.0e-6, 120, ["SiC", "1650C"]),
    mat("MAT-SI3N4", "Silicon Nitride", "Ceramic / Technical", 310, 0.24, 580, 1000, 3200,
        3.2e-6, 30, ["Si3N4", "bearings"]),
    mat("MAT-ZRO2", "Toughened Zirconia", "Ceramic / Technical", 200, 0.31, 900, 1200, 6030,
        10.5e-6, 2, ["zirconia", "dental"]),
]

(OUT / "materials.json").write_text(
    json.dumps({"schemaVersion": SCHEMA, "domain": "materials", "entries": materials}, indent=2)
)

equations = [
    {"id": "EQ-EQUILIBRIUM", "name": "Static Equilibrium", "domain": "statics",
     "expression": "ΣF=0, ΣM=0", "variables": [], "notes": "Fundamental statics"},
    {"id": "EQ-NORMAL-STRESS", "name": "Normal Stress", "domain": "statics",
     "expression": "σ = F/A", "variables": [{"symbol": "σ", "unit": "Pa"}, {"symbol": "F", "unit": "N"}, {"symbol": "A", "unit": "m²"}]},
    {"id": "EQ-HOOKE", "name": "Hooke's Law", "domain": "statics",
     "expression": "σ = E·ε", "variables": [{"symbol": "E", "unit": "Pa"}, {"symbol": "ε", "unit": "-"}]},
    {"id": "EQ-VON-MISES", "name": "Von Mises Criterion", "domain": "statics",
     "expression": "σ_vm = √[σx²-σxσy+σy²+3τxy²]", "variables": [], "notes": "Failure when σ_vm ≥ σ_yield"},
    {"id": "EQ-BEAM-CANTILEVER", "name": "Cantilever Point Load Deflection", "domain": "statics",
     "expression": "δmax = FL³/(3EI)", "variables": [{"symbol": "F", "unit": "N"}, {"symbol": "L", "unit": "m"}, {"symbol": "E", "unit": "Pa"}, {"symbol": "I", "unit": "m⁴"}]},
    {"id": "EQ-EULER-BUCKLING", "name": "Euler Column Buckling", "domain": "statics",
     "expression": "Pcr = π²EI/(KL)²", "variables": [{"symbol": "K", "unit": "-", "notes": "effective length factor"}]},
    {"id": "EQ-TORSION", "name": "Torsion Shear Stress", "domain": "statics",
     "expression": "τ = Tr/J", "variables": []},
    {"id": "EQ-NEWTON2", "name": "Newton's Second Law", "domain": "dynamics",
     "expression": "F = ma", "variables": []},
    {"id": "EQ-KINETIC-ENERGY", "name": "Kinetic Energy", "domain": "dynamics",
     "expression": "KE = ½mv²", "variables": []},
    {"id": "EQ-NATURAL-FREQ", "name": "Natural Frequency (SDOF)", "domain": "dynamics",
     "expression": "ω_n = √(k/m)", "variables": [{"symbol": "ω_n", "unit": "rad/s"}]},
    {"id": "EQ-GEAR-RATIO", "name": "Gear Ratio", "domain": "dynamics",
     "expression": "i = N₂/N₁ = ω₁/ω₂ = T₂/T₁", "variables": []},
    {"id": "EQ-BERNOULLI", "name": "Bernoulli Equation", "domain": "fluids",
     "expression": "p + ½ρv² + ρgh = constant", "variables": []},
    {"id": "EQ-REYNOLDS", "name": "Reynolds Number", "domain": "fluids",
     "expression": "Re = ρvL/μ", "variables": [], "notes": "Re<2300 laminar, Re>4000 turbulent"},
    {"id": "EQ-DARCY", "name": "Darcy-Weisbach Head Loss", "domain": "fluids",
     "expression": "hf = f·L·v²/(D·2g)", "variables": []},
    {"id": "EQ-FOURIER", "name": "Fourier Conduction", "domain": "thermo",
     "expression": "Q = k·A·ΔT/L", "variables": [{"symbol": "k", "unit": "W/(m·K)"}]},
    {"id": "EQ-THERMAL-STRESS", "name": "Thermal Stress", "domain": "thermo",
     "expression": "σ_thermal = E·α·ΔT", "variables": []},
    {"id": "EQ-OHM", "name": "Ohm's Law", "domain": "electromagnetism",
     "expression": "V = IR", "variables": []},
    {"id": "EQ-MOTOR-TORQUE", "name": "DC Motor Torque", "domain": "electromagnetism",
     "expression": "τ = K_t·I", "variables": []},
    {"id": "EQ-PID", "name": "PID Control", "domain": "control",
     "expression": "u(t) = Kp·e + Ki·∫e dt + Kd·de/dt", "variables": []},
    {"id": "EQ-BEARING-L10", "name": "Bearing L10 Life", "domain": "machine_elements",
     "expression": "L10 = (C/P)^p × 10⁶ rev", "variables": [{"symbol": "p", "unit": "-", "notes": "3 ball, 10/3 roller"}]},
    {"id": "EQ-SPRING-RATE", "name": "Compression Spring Rate", "domain": "machine_elements",
     "expression": "k = Gd⁴/(8D³Na)", "variables": []},
]
(OUT / "equations.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "equations", "entries": equations}, indent=2))

manufacturing = [
    {"id": "MFG-TURNING", "name": "CNC Turning", "category": "Machining", "process": "turning",
     "parameters": {"vc_steel_carbide_m_min": "150-300", "feed_rough_mm_rev": "0.3-1.5"},
     "notes": "v=πDN/1000, MRR=v·f·ap"},
    {"id": "MFG-MILLING", "name": "CNC Milling", "category": "Machining", "process": "milling",
     "parameters": {"fz_al_mm": "0.05-0.25", "fz_steel_mm": "0.02-0.15"}},
    {"id": "MFG-DRILLING", "name": "Drilling", "category": "Machining", "process": "drilling",
     "parameters": {"feed_mm_rev": "0.05-0.5"}},
    {"id": "MFG-GRINDING", "name": "Grinding", "category": "Machining", "process": "grinding",
     "parameters": {"Ra_um": "0.025-1.6"}},
    {"id": "MFG-SAND-CAST", "name": "Sand Casting", "category": "Casting", "tolerance_mm": "±0.5-1.5", "Ra_um": "6.3-25"},
    {"id": "MFG-DIE-CAST", "name": "Die Casting", "category": "Casting", "tolerance_mm": "±0.025-0.075"},
    {"id": "MFG-INVESTMENT", "name": "Investment Casting", "category": "Casting", "tolerance_mm": "±0.1-0.25"},
    {"id": "MFG-FORGING", "name": "Forging", "category": "Forming", "notes": "Hot/warm/cold; F=Yf·A·(1+0.4μD/h)"},
    {"id": "MFG-EXTRUSION", "name": "Extrusion", "category": "Forming", "notes": "Al ratio up to 40:1"},
    {"id": "MFG-GMAW", "name": "GMAW (MIG)", "category": "Welding", "filler": "ER70S-6"},
    {"id": "MFG-GTAW", "name": "GTAW (TIG)", "category": "Welding", "notes": "Precision thin sections"},
    {"id": "MFG-FDM", "name": "FDM 3D Printing", "category": "Additive", "layer_mm": "0.05-0.4", "tolerance_mm": "±0.2-0.5"},
    {"id": "MFG-SLA", "name": "SLA/DLP", "category": "Additive", "layer_mm": "0.025-0.1"},
    {"id": "MFG-DMLS", "name": "DMLS/SLM Metal AM", "category": "Additive", "materials": ["316L", "Ti64", "IN718"]},
]
(OUT / "manufacturing.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "manufacturing", "entries": manufacturing}, indent=2))

machine_elements = [
    {"id": "ME-BOLT-M8-8.8", "name": "Metric Bolt M8 Grade 8.8", "category": "Fastener", "grade": "8.8", "Sp_MPa": 600, "Su_MPa": 830},
    {"id": "ME-BOLT-M12-12.9", "name": "Metric Bolt M12 Grade 12.9", "category": "Fastener", "grade": "12.9", "Sp_MPa": 970},
    {"id": "ME-BALL-SCREW", "name": "Ball Screw", "category": "Power Transmission", "efficiency": 0.90},
    {"id": "ME-BRG-6200", "name": "Deep Groove Ball Bearing (6200 series)", "category": "Bearing", "type": "deep_groove_ball"},
    {"id": "ME-BRG-7200", "name": "Angular Contact Bearing (7200)", "category": "Bearing", "type": "angular_contact"},
    {"id": "ME-BRG-TAPER", "name": "Tapered Roller Bearing (30000)", "category": "Bearing", "type": "tapered_roller"},
    {"id": "ME-GEAR-SPUR", "name": "Spur Gear", "category": "Gear", "pressure_angle_deg": 20, "module_standard": "ISO"},
    {"id": "ME-GEAR-HELICAL", "name": "Helical Gear", "category": "Gear", "helix_angle_deg": "15-30"},
    {"id": "ME-GEAR-WORM", "name": "Worm Gear", "category": "Gear", "ratio_range": "5:1 to 100:1"},
    {"id": "ME-SPRING-COMP", "name": "Compression Spring", "category": "Spring", "formula": "k=Gd⁴/(8D³Na)"},
    {"id": "ME-KEY-PARALLEL", "name": "Parallel Key", "category": "Shaft Connection", "notes": "b≈d/4"},
]
(OUT / "machine_elements.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "machine_elements", "entries": machine_elements}, indent=2))

machine_types = [
    {"id": "MT-CNC-LATHE", "name": "CNC Lathe", "category": "Manufacturing", "axes": 2, "applications": ["turning", "boring"]},
    {"id": "MT-VMC", "name": "Vertical Machining Center", "category": "Manufacturing", "axes": 3},
    {"id": "MT-5AXIS", "name": "5-Axis Machining Center", "category": "Manufacturing", "axes": 5},
    {"id": "MT-ROBOT-6DOF", "name": "6-DOF Articulated Robot", "category": "Robotics", "brands": ["KUKA", "ABB", "FANUC"]},
    {"id": "MT-SCARA", "name": "SCARA Robot", "category": "Robotics", "applications": ["assembly"]},
    {"id": "MT-DELTA", "name": "Delta Parallel Robot", "category": "Robotics", "applications": ["pick_and_place"]},
    {"id": "MT-HYDRAULIC-PRESS", "name": "Hydraulic Press", "category": "Forming", "pressure_bar": "350-700"},
    {"id": "MT-STEAM-TURBINE", "name": "Steam Turbine", "category": "Power Generation", "power_MW": "1-1800"},
    {"id": "MT-GAS-TURBINE", "name": "Gas Turbine", "category": "Power Generation", "efficiency_pct": "35-65"},
    {"id": "MT-WIND", "name": "Wind Turbine", "category": "Power Generation", "notes": "Betz limit 59.3%"},
    {"id": "MT-CMM", "name": "Coordinate Measuring Machine", "category": "Precision", "accuracy_um": "0.5-5"},
]
(OUT / "machine_types.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "machine_types", "entries": machine_types}, indent=2))

robotics = [
    {"id": "ROB-DH", "name": "Denavit-Hartenberg Kinematics", "category": "Kinematics", "notes": "Forward/inverse kinematics via DH parameters"},
    {"id": "ROB-JACOBIAN", "name": "Jacobian Matrix", "category": "Kinematics", "expression": "ẋ = J(θ)·θ̇"},
    {"id": "ROB-NEWTON-EULER", "name": "Newton-Euler Dynamics", "category": "Dynamics", "expression": "M(θ)θ̈ + C(θ,θ̇)θ̇ + G(θ) = τ"},
    {"id": "ROB-UR5", "name": "Universal Robots UR5", "category": "Cobot", "payload_kg": 5, "reach_mm": 850},
    {"id": "ROB-ENCODER", "name": "Rotary Encoder", "category": "Sensor", "resolution_PPR": "100-131072"},
    {"id": "ROB-LIDAR", "name": "LiDAR", "category": "Sensor", "range_m": "0.1-200"},
]
(OUT / "robotics.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "robotics", "entries": robotics}, indent=2))

control_systems = [
    {"id": "CTL-PID", "name": "PID Controller", "category": "Control", "tuning": "Ziegler-Nichols: Kp=0.6Ku, Ti=Tu/2, Td=Tu/8"},
    {"id": "CTL-CASCADE", "name": "Cascade Control", "category": "Control", "notes": "Inner loop 5-10× faster than outer"},
    {"id": "CTL-SERVO", "name": "Servo Drive Loops", "category": "Motion", "loops": ["current", "velocity", "position"]},
    {"id": "CTL-VFD", "name": "AC Variable Frequency Drive", "category": "Drive", "freq_Hz": "0-400+"},
    {"id": "CTL-PLC-IEC", "name": "IEC 61131-3 PLC Languages", "category": "PLC", "languages": ["LD", "FBD", "ST", "SFC", "IL"]},
    {"id": "CTL-PROFINET", "name": "PROFINET", "category": "Fieldbus", "ecosystem": "Siemens"},
    {"id": "CTL-ETHERCAT", "name": "EtherCAT", "category": "Fieldbus", "notes": "Ultra-fast industrial Ethernet"},
]
(OUT / "control_systems.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "control_systems", "entries": control_systems}, indent=2))

treatments = [
    {"id": "HT-ANNEAL", "name": "Annealing", "category": "Heat Treatment", "notes": "Soften, relieve stress"},
    {"id": "HT-QT", "name": "Quench & Temper", "category": "Heat Treatment", "notes": "1045 Q&T HRC 22-58"},
    {"id": "HT-CARBURIZE", "name": "Gas Carburizing", "category": "Case Hardening", "case_depth_mm": "0.5-2.5"},
    {"id": "HT-NITRIDE", "name": "Nitriding", "category": "Case Hardening", "temp_C": "550-570"},
    {"id": "ST-TIN-PVD", "name": "TiN PVD Coating", "category": "Surface", "hardness_HV": 2400},
    {"id": "ST-DLC", "name": "DLC Coating", "category": "Surface", "hardness_HV": "3000-8000"},
    {"id": "ST-ANODIZE-III", "name": "Hard Anodize Type III", "category": "Surface", "thickness_um": "50-75"},
]
(OUT / "treatments.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "treatments", "entries": treatments}, indent=2))

standards = [
    {"id": "STD-ISO286", "name": "ISO 286 Tolerances & Fits", "category": "Dimensional", "examples": ["H7/g6 sliding", "H7/k6 transition", "H7/p6 press"]},
    {"id": "STD-ISO1302", "name": "ISO 1302 Surface Finish", "category": "Surface", "parameters": ["Ra", "Rz", "Rmax"]},
    {"id": "STD-AGMA", "name": "AGMA Gear Standards", "category": "Machine Elements"},
    {"id": "STD-ASME-BPV", "name": "ASME Boiler & Pressure Vessel Code", "category": "Pressure Systems"},
    {"id": "STD-ISO281", "name": "ISO 281 Bearing Life", "category": "Machine Elements"},
    {"id": "STD-IEC61131", "name": "IEC 61131-3 PLC", "category": "Automation"},
]
(OUT / "standards.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "standards", "entries": standards}, indent=2))

cad_cam = [
    {"id": "CAD-SW", "name": "SOLIDWORKS", "category": "CAD", "formats": [".SLDPRT", ".SLDASM", ".SLDDRW"]},
    {"id": "CAD-NX", "name": "Siemens NX", "category": "CAD", "notes": "Integrated CAM"},
    {"id": "CAD-F360", "name": "Fusion 360", "category": "CAD", "notes": "Cloud CAM + FEA"},
    {"id": "CAD-STEP", "name": "STEP (.stp)", "category": "Exchange", "standard": "ISO 10303"},
    {"id": "CAM-MASTERCAM", "name": "Mastercam", "category": "CAM", "notes": "Industry standard"},
    {"id": "CAM-5AXIS", "name": "5-Axis Simultaneous Milling", "category": "CAM", "strategies": ["swarf", "flank"]},
    {"id": "FEA-ANSYS", "name": "ANSYS Mechanical", "category": "FEA"},
    {"id": "GCODE-G81", "name": "G81 Drilling Cycle", "category": "CNC", "code": "G81"},
]
(OUT / "cad_cam.json").write_text(json.dumps({"schemaVersion": SCHEMA, "domain": "cad_cam", "entries": cad_cam}, indent=2))

index = {
    "schemaVersion": SCHEMA,
    "title": "SZM Forge Machine Creation Knowledge Base",
    "domains": [
        {"id": "materials", "name": "Materials Science", "file": "materials.json", "entryCount": len(materials)},
        {"id": "equations", "name": "Physics Equations", "file": "equations.json", "entryCount": len(equations)},
        {"id": "manufacturing", "name": "Manufacturing Processes", "file": "manufacturing.json", "entryCount": len(manufacturing)},
        {"id": "machine_elements", "name": "Machine Elements", "file": "machine_elements.json", "entryCount": len(machine_elements)},
        {"id": "machine_types", "name": "Machine Types", "file": "machine_types.json", "entryCount": len(machine_types)},
        {"id": "robotics", "name": "Robotics", "file": "robotics.json", "entryCount": len(robotics)},
        {"id": "control_systems", "name": "Control Systems", "file": "control_systems.json", "entryCount": len(control_systems)},
        {"id": "treatments", "name": "Heat & Surface Treatments", "file": "treatments.json", "entryCount": len(treatments)},
        {"id": "standards", "name": "Standards & Codes", "file": "standards.json", "entryCount": len(standards)},
        {"id": "cad_cam", "name": "CAD/CAM/FEA", "file": "cad_cam.json", "entryCount": len(cad_cam)},
    ],
}
(OUT / "index.json").write_text(json.dumps(index, indent=2))
print(f"Generated {len(index['domains'])} domain files in {OUT}")
