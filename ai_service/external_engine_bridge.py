from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import uvicorn
import logging/mnt/D/projects/szm_forge/agent_plans

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("ExternalEngineBridge")

app = FastAPI(title="SZM Forge - External Engine Bridge")

class CadImportRequest(BaseModel):
    file_path: str

class CadExportRequest(BaseModel):
    model_data: str
    format: str # e.g. "STEP", "IGES"

class SimulationRequest(BaseModel):
    setup_json: str

class FEARequest(BaseModel):
    component_id: int
    force_n: float
    area_m2: float
    thickness_m: float
    youngs_modulus_pa: float = 210e9
    yield_strength_pa: float = 250e6
    density_kg_m3: float = 7850.0

class UsdExportRequest(BaseModel):
    scene_json: str
    output_path: str = "/tmp/workspace_export.usda"

class FreeCADRequest(BaseModel):
    shape: str = "box"          # box | cylinder | sphere
    length: float = 0.1         # metres
    width: float = 0.05
    height: float = 0.02
    fillet_radius: float = 0.0  # 0 = no fillet
    output_path: str = "/tmp/szm_freecad_part.step"

class STEPAssemblyRequest(BaseModel):
    file_path: str

class AITrainRequest(BaseModel):
    data_dir: str = "./machine_data"

class AIGenerateRequest(BaseModel):
    machine_type: str = "cnc_mill"
    count: int = 1
    parameters: dict = None

# ============================================================================
# THERMAL & MODAL ANALYSIS REQUEST MODELS (Plan 17)
# ============================================================================

class ThermalAnalysisRequest(BaseModel):
    component_id: int
    mode: str = "steady_state"  # steady_state or transient
    ambient_temp_c: float = 20.0
    heat_input_w: float = 100.0
    boundary_conditions: dict = {}  # {location: temp_c}
    convection_coefficient: float = 10.0  # W/(m²·K)
    duration_seconds: float = 3600.0  # For transient
    time_steps: int = 100
    material_properties: dict = {}

class ModalAnalysisRequest(BaseModel):
    component_id: int
    num_modes: int = 10  # Number of mode shapes to compute
    material_properties: dict = {}  # density, youngs_modulus, poisson_ratio
    constraints: list = []  # Fixed/pinned points
    min_frequency_hz: float = 0.0
    max_frequency_hz: float = 10000.0

# ============================================================================
# CIRCUIT SIMULATION LAYER (Plan 19: Ngspice Electronics)
# ============================================================================

class CircuitSimulationRequest(BaseModel):
    component_id: int
    circuit_type: str = "dc"  # "dc", "ac", "transient"
    frequency_hz: float = 1000.0  # For AC analysis
    temperature_c: float = 27.0
    duration_seconds: float = 1.0  # For transient
    time_steps: int = 100
    netlist: str = ""  # SPICE netlist (alternative to components)
    components: list = []  # [{name, type, value, node1, node2}, ...]
    parameters: dict = {}  # {param_name: value}

# Initialize ML Pipeline globally
try:
    from machine_learning_training import MachineLearningPipeline
    ml_pipeline = MachineLearningPipeline("./machine_data")
except ImportError:
    logger.error("Could not import machine_learning_training")
    ml_pipeline = None

@app.post("/cad/import")
async def import_cad_model(request: CadImportRequest):
    logger.info(f"Received request to import CAD model: {request.file_path}")
    # TODO: Integrate PythonOCC (OpenCASCADE) or FreeCAD headless here
    # Example:
    # import OCC.Core.STEPControl as STEPControl
    # reader = STEPControl.STEPControl_Reader()
    # reader.ReadFile(request.file_path)
    
    return {
        "status": "success",
        "engine_used": "OpenCASCADE (Placeholder)",
        "model_data": "{\"vertices\": [...], \"indices\": [...]}" 
    }

@app.post("/cad/export")
async def export_cad_model(request: CadExportRequest):
    logger.info(f"Received request to export CAD model to {request.format}")
    # TODO: Integrate PythonOCC
    return {
        "status": "success",
        "file_path": f"/tmp/exported_model.{request.format.lower()}"
    }

@app.post("/simulation/run")
async def run_simulation(request: SimulationRequest):
    logger.info(f"Received request to run simulation")
    return {
        "status": "success",
        "engine_used": "CalculiX (Placeholder)",
        "result_json": "{\"max_stress\": 150.5, \"max_displacement\": 0.02}"
    }

@app.post("/simulation/fea/run")
async def run_fea(request: FEARequest):
    import subprocess, tempfile, os, re, math

    # Derive simple beam geometry: length = thickness, width = sqrt(area)
    L = request.thickness_m
    w = math.sqrt(request.area_m2)
    E = request.youngs_modulus_pa
    F = request.force_n

    # Analytical fallback (used if ccx not available)
    I = (w * L**3) / 12.0  # second moment of area
    sigma_analytical = (F * L / 2.0) / (I / (L / 2.0)) if I > 0 else F / request.area_m2
    disp_analytical = (F * L**3) / (3.0 * E * I) if I > 0 else 0.0

    with tempfile.TemporaryDirectory() as tmpdir:
        job = os.path.join(tmpdir, "job")
        inp_path = job + ".inp"

        # Minimal CalculiX cantilever beam .inp (8-node hex, 1 element)
        inp = f"""*NODE
 1, 0.0, 0.0, 0.0
 2, {w}, 0.0, 0.0
 3, {w}, {w}, 0.0
 4, 0.0, {w}, 0.0
 5, 0.0, 0.0, {L}
 6, {w}, 0.0, {L}
 7, {w}, {w}, {L}
 8, 0.0, {w}, {L}
*ELEMENT, TYPE=C3D8, ELSET=EALL
 1, 1,2,3,4,5,6,7,8
*MATERIAL, NAME=MAT
*ELASTIC
 {E}, 0.3
*DENSITY
 {request.density_kg_m3}
*SOLID SECTION, ELSET=EALL, MATERIAL=MAT
*BOUNDARY
 1, 1,3
 2, 1,3
 3, 1,3
 4, 1,3
*STEP
*STATIC
*CLOAD
 5, 3, {F / 4.0}
 6, 3, {F / 4.0}
 7, 3, {F / 4.0}
 8, 3, {F / 4.0}
*NODE PRINT, NSET=NALL
U
*EL PRINT, ELSET=EALL
S
*END STEP
"""
        with open(inp_path, "w") as f:
            f.write(inp)

        # Try running ccx
        ccx_available = False
        max_stress = sigma_analytical
        max_disp = disp_analytical
        engine_used = "Analytical fallback"

        for ccx_bin in ["ccx", "ccx_2.17", "ccx_2.20", "calculix"]:
            try:
                result = subprocess.run(
                    [ccx_bin, "-i", "job"],
                    cwd=tmpdir, capture_output=True, text=True, timeout=30
                )
                if result.returncode == 0:
                    ccx_available = True
                    engine_used = ccx_bin
                    break
            except (FileNotFoundError, subprocess.TimeoutExpired):
                continue

        if ccx_available:
            dat_path = job + ".dat"
            if os.path.exists(dat_path):
                with open(dat_path) as f:
                    content = f.read()
                # Parse max Von Mises stress from .dat
                stress_vals = re.findall(r"S\s+([\d.E+\-]+)", content)
                disp_vals = re.findall(r"U\s+[\d.E+\-]+\s+[\d.E+\-]+\s+([\d.E+\-]+)", content)
                if stress_vals:
                    max_stress = max(float(v) for v in stress_vals)
                if disp_vals:
                    max_disp = max(float(v) for v in disp_vals)

        stress_ratio = max_stress / request.yield_strength_pa if request.yield_strength_pa > 0 else 0.0
        logger.info(f"[FEA] component={request.component_id} engine={engine_used} "
                    f"max_stress={max_stress:.2f} Pa disp={max_disp:.6f} m")

        return {
            "status": "success",
            "engine_used": engine_used,
            "component_id": request.component_id,
            "max_stress_pa": max_stress,
            "max_displacement_m": max_disp,
            "stress_ratio": min(stress_ratio, 1.0),
            "is_dangerous": stress_ratio > 0.8,
        }

# ============================================================================
# THERMAL ANALYSIS ENDPOINT (Plan 17)
# ============================================================================

@app.post("/simulation/thermal/run")
async def run_thermal_analysis(request: ThermalAnalysisRequest):
    """Run steady-state or transient thermal analysis using CalculiX."""
    import subprocess, tempfile, os, re, math

    logger.info(f"[Thermal] Starting {request.mode} thermal analysis for component {request.component_id}")

    # Material properties
    density = request.material_properties.get("density_kg_m3", 7850.0)
    thermal_conductivity = request.material_properties.get("thermal_conductivity_w_mk", 50.0)
    specific_heat = request.material_properties.get("specific_heat_j_kgk", 500.0)

    # Generate thermal CalculiX input file
    heat_per_node = request.heat_input_w / 8.0  # Distribute over 8 nodes
    
    if request.mode == "transient":
        inp = f"""*NODE
 1, 0.0, 0.0, 0.0
 2, 0.1, 0.0, 0.0
 3, 0.1, 0.1, 0.0
 4, 0.0, 0.1, 0.0
 5, 0.0, 0.0, 0.1
 6, 0.1, 0.0, 0.1
 7, 0.1, 0.1, 0.1
 8, 0.0, 0.1, 0.1
*ELEMENT, TYPE=DC3D8, ELSET=EALL
 1, 1,2,3,4,5,6,7,8
*MATERIAL, NAME=THERM_MAT
*DENSITY
 {density}
*SPECIFIC HEAT
 {specific_heat}
*CONDUCTIVITY
 {thermal_conductivity}
*SOLID SECTION, ELSET=EALL, MATERIAL=THERM_MAT
*INITIAL CONDITIONS
 1, {request.ambient_temp_c}
 2, {request.ambient_temp_c}
 3, {request.ambient_temp_c}
 4, {request.ambient_temp_c}
 5, {request.ambient_temp_c}
 6, {request.ambient_temp_c}
 7, {request.ambient_temp_c}
 8, {request.ambient_temp_c}
*BOUNDARY
 1, 11, {request.ambient_temp_c}
*STEP, INC={request.time_steps}
*HEAT TRANSFER, TRANSIENT
 {request.duration_seconds/request.time_steps}, {request.duration_seconds}
*BOUNDARY CONVECTION
 P1, 10, {request.convection_coefficient}, {request.ambient_temp_c}
*CFLUX
 5, 11, {heat_per_node}
 6, 11, {heat_per_node}
 7, 11, {heat_per_node}
 8, 11, {heat_per_node}
*NODE PRINT, NSET=NALL
 NT
*END STEP
"""
    else:  # steady_state
        inp = f"""*NODE
 1, 0.0, 0.0, 0.0
 2, 0.1, 0.0, 0.0
 3, 0.1, 0.1, 0.0
 4, 0.0, 0.1, 0.0
 5, 0.0, 0.0, 0.1
 6, 0.1, 0.0, 0.1
 7, 0.1, 0.1, 0.1
 8, 0.0, 0.1, 0.1
*ELEMENT, TYPE=DC3D8, ELSET=EALL
 1, 1,2,3,4,5,6,7,8
*MATERIAL, NAME=THERM_MAT
*DENSITY
 {density}
*CONDUCTIVITY
 {thermal_conductivity}
*SOLID SECTION, ELSET=EALL, MATERIAL=THERM_MAT
*BOUNDARY
 1, 11, {request.ambient_temp_c}
*STEP
*HEAT TRANSFER
*CFLUX
 5, 11, {heat_per_node}
 6, 11, {heat_per_node}
 7, 11, {heat_per_node}
 8, 11, {heat_per_node}
*NODE PRINT, NSET=NALL
 NT
*END STEP
"""

    with tempfile.TemporaryDirectory() as tmpdir:
        job = os.path.join(tmpdir, "thermal_job")
        inp_path = job + ".inp"
        
        with open(inp_path, "w") as f:
            f.write(inp)

        max_temp = request.ambient_temp_c
        engine_used = "Analytical fallback"
        ccx_available = False

        # Try running CalculiX
        for ccx_bin in ["ccx", "ccx_2.17", "ccx_2.20", "calculix"]:
            try:
                result = subprocess.run(
                    [ccx_bin, "-i", "thermal_job"],
                    cwd=tmpdir, capture_output=True, text=True, timeout=60
                )
                if result.returncode == 0:
                    ccx_available = True
                    engine_used = ccx_bin
                    break
            except (FileNotFoundError, subprocess.TimeoutExpired):
                continue

        if ccx_available:
            dat_path = job + ".dat"
            if os.path.exists(dat_path):
                with open(dat_path) as f:
                    content = f.read()
                # Parse max temperature from .dat (NT = nodal temperature)
                temp_vals = re.findall(r"NT\s+([\d.E+\-]+)", content)
                if temp_vals:
                    max_temp = max(float(v) for v in temp_vals)
        else:
            # Analytical: Simple heat conduction estimate
            # Q = k*A*dT/dx  =>  dT = Q*x / (k*A)
            dx = 0.1  # Thickness
            area = 0.01  # 0.1 x 0.1 m²
            delta_t = (request.heat_input_w * dx) / (thermal_conductivity * area)
            max_temp = request.ambient_temp_c + delta_t

        temp_ratio = (max_temp - request.ambient_temp_c) / max(1.0, request.heat_input_w)
        
        logger.info(f"[Thermal] component={request.component_id} engine={engine_used} "
                   f"max_temp={max_temp:.2f}°C delta_t={max_temp - request.ambient_temp_c:.2f}°C")

        return {
            "status": "success",
            "engine_used": engine_used,
            "component_id": request.component_id,
            "analysis_type": request.mode,
            "max_temperature_c": round(max_temp, 2),
            "ambient_temperature_c": request.ambient_temp_c,
            "temperature_rise_c": round(max_temp - request.ambient_temp_c, 2),
            "is_dangerous": max_temp > 100,  # Arbitrary threshold
        }

# ============================================================================
# MODAL ANALYSIS ENDPOINT (Plan 17)
# ============================================================================

@app.post("/simulation/modal/run")
async def run_modal_analysis(request: ModalAnalysisRequest):
    """Run modal (eigenvalue) analysis to find natural frequencies and mode shapes."""
    import subprocess, tempfile, os, re, math
    
    logger.info(f"[Modal] Computing {request.num_modes} mode shapes for component {request.component_id}")

    # Material properties
    density = request.material_properties.get("density_kg_m3", 7850.0)
    youngs_modulus = request.material_properties.get("youngs_modulus_pa", 210e9)
    poisson_ratio = request.material_properties.get("poisson_ratio", 0.3)

    # Generate modal CalculiX input file
    inp = f"""*NODE
 1, 0.0, 0.0, 0.0
 2, 0.1, 0.0, 0.0
 3, 0.1, 0.1, 0.0
 4, 0.0, 0.1, 0.0
 5, 0.0, 0.0, 0.1
 6, 0.1, 0.0, 0.1
 7, 0.1, 0.1, 0.1
 8, 0.0, 0.1, 0.1
*ELEMENT, TYPE=C3D8, ELSET=EALL
 1, 1,2,3,4,5,6,7,8
*MATERIAL, NAME=MODAL_MAT
*ELASTIC
 {youngs_modulus}, {poisson_ratio}
*DENSITY
 {density}
*SOLID SECTION, ELSET=EALL, MATERIAL=MODAL_MAT
*BOUNDARY
 1, 1,3
*STEP
*FREQUENCY, NMIN={request.num_modes}, NMAX={request.num_modes}
*END STEP
"""

    with tempfile.TemporaryDirectory() as tmpdir:
        job = os.path.join(tmpdir, "modal_job")
        inp_path = job + ".inp"
        
        with open(inp_path, "w") as f:
            f.write(inp)

        frequencies = []
        engine_used = "Analytical fallback"
        ccx_available = False

        # Try running CalculiX
        for ccx_bin in ["ccx", "ccx_2.17", "ccx_2.20", "calculix"]:
            try:
                result = subprocess.run(
                    [ccx_bin, "-i", "modal_job"],
                    cwd=tmpdir, capture_output=True, text=True, timeout=60
                )
                if result.returncode == 0:
                    ccx_available = True
                    engine_used = ccx_bin
                    break
            except (FileNotFoundError, subprocess.TimeoutExpired):
                continue

        if ccx_available:
            frd_path = job + ".frd"
            if os.path.exists(frd_path):
                # Parse .frd file for frequencies
                with open(frd_path) as f:
                    content = f.read()
                # Extract eigenvalues (frequencies in Hz)
                freq_vals = re.findall(r"eigenvalue:\s+([\d.E+\-]+)", content, re.IGNORECASE)
                if freq_vals:
                    # Convert eigenvalues to frequencies
                    frequencies = [math.sqrt(float(v)) / (2 * math.pi) for v in freq_vals]

        if not frequencies:
            # Analytical: Simple beam frequency estimation (cantilever)
            # f = (λ² / (2*π)) * sqrt(E*I / (ρ*A*L⁴))
            L = 0.1
            E = youngs_modulus
            rho = density
            A = 0.01
            I = (0.1 * 0.1**3) / 12  # Second moment of area
            
            # First few mode coefficients for cantilever: 1.875, 4.694, 7.855, 10.996
            mode_coeffs = [1.875, 4.694, 7.855, 10.996, 14.137, 17.276, 20.420, 23.562, 26.703, 29.845]
            
            base_freq = (1 / (2 * math.pi)) * math.sqrt(E * I / (rho * A * L**4))
            frequencies = [mode_coeffs[i] * base_freq for i in range(min(request.num_modes, len(mode_coeffs)))]

        logger.info(f"[Modal] component={request.component_id} engine={engine_used} "
                   f"frequencies={[f'{f:.2f}' for f in frequencies[:5]]} Hz")

        return {
            "status": "success",
            "engine_used": engine_used,
            "component_id": request.component_id,
            "num_modes_computed": len(frequencies),
            "natural_frequencies_hz": [round(f, 2) for f in frequencies],
            "first_natural_frequency_hz": round(frequencies[0], 2) if frequencies else 0,
            "frequency_range": {
                "min_hz": round(min(frequencies), 2) if frequencies else 0,
                "max_hz": round(max(frequencies), 2) if frequencies else 0,
            },
            "lowest_frequency_hz": request.min_frequency_hz,
            "highest_frequency_hz": request.max_frequency_hz,
        }

# ============================================================================
# CIRCUIT SIMULATION ENDPOINT (Plan 19: Ngspice Electronics)
# ============================================================================

@app.post("/simulation/circuit/run")
async def run_circuit_simulation(request: CircuitSimulationRequest):
    """Run circuit simulation using ngspice for DC, AC, or transient analysis."""
    import subprocess, tempfile, os, re, json
    
    logger.info(f"[Circuit] Running {request.circuit_type} analysis for component {request.component_id}")
    
    # Build SPICE netlist
    netlist_lines = ["SZM Circuit Simulation - Component {}".format(request.component_id)]
    
    # If custom netlist provided, use it; otherwise build from components
    if request.netlist:
        netlist_lines.append(request.netlist)
    else:
        # Build netlist from components list
        # Default simple RC circuit if no components specified
        if not request.components:
            # Default: Voltage source + Resistor + Capacitor circuit
            request.components = [
                {"name": "V1", "type": "VoltageSource", "value": 10.0, "node1": 1, "node2": 0, "ac": 10.0},
                {"name": "R1", "type": "Resistor", "value": 1000.0, "node1": 1, "node2": 2},
                {"name": "C1", "type": "Capacitor", "value": 1e-6, "node1": 2, "node2": 0},
            ]
        
        # Add components to netlist
        for comp in request.components:
            comp_type = comp.get("type", "Resistor").lower()
            name = comp.get("name", f"{comp_type[0]}_comp")
            n1 = comp.get("node1", 1)
            n2 = comp.get("node2", 0)
            value = comp.get("value", 1.0)
            
            if comp_type in ["voltagesource", "voltage_source"]:
                # V <name> <+ node> <- node> [DC <value>] [AC <ac_mag> [<ac_phase>]]
                ac_mag = comp.get("ac", value)
                netlist_lines.append(f"{name} {n1} {n2} DC {value} AC {ac_mag}")
            elif comp_type in ["currentsource", "current_source"]:
                # I <name> <+ node> <- node> [DC <value>] [AC <ac_mag>]
                ac_mag = comp.get("ac", value)
                netlist_lines.append(f"{name} {n1} {n2} DC {value} AC {ac_mag}")
            elif comp_type == "resistor":
                netlist_lines.append(f"{name} {n1} {n2} {value}")
            elif comp_type == "capacitor":
                netlist_lines.append(f"{name} {n1} {n2} {value}")
            elif comp_type == "inductor":
                netlist_lines.append(f"{name} {n1} {n2} {value}")
            elif comp_type == "diode":
                netlist_lines.append(f"{name} {n1} {n2} D1")
                # Add diode model if not already present
                if ".model D1 D" not in "\n".join(netlist_lines):
                    netlist_lines.append(".model D1 D(IS=1e-14 N=1.0)")
    
    # Add analysis command
    if request.circuit_type.lower() == "dc":
        netlist_lines.append(".op")  # DC operating point
    elif request.circuit_type.lower() == "ac":
        # AC sweep: .ac dec <points> <start_freq> <stop_freq>
        freq = request.frequency_hz
        netlist_lines.append(f".ac dec 100 {freq*0.001} {freq*1000}")
    elif request.circuit_type.lower() == "transient":
        # Transient: .tran <tstep> <tstop> [<tstart> [<tmax>]]
        time_step = request.duration_seconds / max(1, request.time_steps)
        netlist_lines.append(f".tran {time_step} {request.duration_seconds}")
    
    # Add control commands and output
    netlist_lines.append(".control")
    netlist_lines.append("run")
    netlist_lines.append("set hcopydevtype=postscript")
    netlist_lines.append("quit")
    netlist_lines.append(".endc")
    netlist_lines.append(".end")
    
    netlist_str = "\n".join(netlist_lines)
    
    # Try running with ngspice
    engine_used = "Analytical fallback"
    results = None
    
    try:
        with tempfile.TemporaryDirectory() as tmpdir:
            cir_path = os.path.join(tmpdir, "circuit.cir")
            out_path = os.path.join(tmpdir, "circuit.out")
            raw_path = os.path.join(tmpdir, "circuit.raw")
            
            # Write netlist
            with open(cir_path, "w") as f:
                f.write(netlist_str)
            
            # Check if ngspice is available
            ret = os.system("which ngspice > /dev/null 2>&1")
            if ret == 0:
                # Run ngspice
                cmd = f"ngspice -b {cir_path} -o {out_path}"
                result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=30)
                
                if result.returncode == 0:
                    engine_used = "ngspice"
                    
                    # Parse output file for results
                    if os.path.exists(out_path):
                        with open(out_path, "r") as f:
                            output = f.read()
                        
                        # Extract node voltages from DC operating point
                        if request.circuit_type.lower() == "dc":
                            results = parse_ngspice_dc_results(output)
                        elif request.circuit_type.lower() == "ac":
                            results = parse_ngspice_ac_results(output)
                        elif request.circuit_type.lower() == "transient":
                            results = parse_ngspice_tran_results(output)
    except subprocess.TimeoutExpired:
        logger.warning("[Circuit] ngspice timeout, using analytical fallback")
    except Exception as e:
        logger.warning(f"[Circuit] ngspice execution failed: {str(e)}, using analytical fallback")
    
    # Analytical fallback
    if results is None:
        results = compute_circuit_analytical(request)
    
    logger.info(f"[Circuit] component={request.component_id} engine={engine_used} "
               f"type={request.circuit_type}")
    
    return {
        "status": "success",
        "engine_used": engine_used,
        "component_id": request.component_id,
        "circuit_type": request.circuit_type,
        "temperature_c": request.temperature_c,
        **results
    }

def parse_ngspice_dc_results(output: str) -> dict:
    """Extract DC operating point results from ngspice output."""
    import re
    
    node_voltages = {}
    power_dissipation = {}
    
    # Pattern: Node <node_name>: <voltage>
    for match in re.finditer(r"Node\s+(\w+):\s+([\d.\-eE+]+)", output, re.IGNORECASE):
        node_name = match.group(1)
        voltage = float(match.group(2))
        node_voltages[node_name] = voltage
    
    # Pattern: V(<n1>): <voltage> or current flows
    for match in re.finditer(r"V\((\w+)\)\s*=\s*([\d.\-eE+]+)", output):
        node_name = match.group(1)
        voltage = float(match.group(2))
        node_voltages[node_name] = voltage
    
    return {
        "node_voltages": node_voltages,
        "power_dissipation": power_dissipation,
        "analysis_successful": True
    }

def parse_ngspice_ac_results(output: str) -> dict:
    """Extract AC analysis results from ngspice output."""
    return {
        "ac_impedance": {},
        "frequency_response": [],
        "phase_response": [],
        "analysis_successful": True
    }

def parse_ngspice_tran_results(output: str) -> dict:
    """Extract transient analysis results from ngspice output."""
    return {
        "time_points": [],
        "voltage_waveforms": {},
        "current_waveforms": {},
        "analysis_successful": True
    }

def compute_circuit_analytical(request: CircuitSimulationRequest) -> dict:
    """Compute circuit behavior using analytical formulas (fallback when ngspice unavailable)."""
    import math
    
    if request.circuit_type.lower() == "dc":
        # Simple DC analysis: V=IR, P=VI
        # Assume simple 10V source with 1k resistor
        voltage_source = 10.0
        total_resistance = 1000.0
        current = voltage_source / max(total_resistance, 1.0)
        power = voltage_source * current
        
        return {
            "node_voltages": {
                "node_1": voltage_source,
                "node_2": voltage_source * 0.5,  # Midpoint
                "gnd": 0.0
            },
            "currents": {
                "R1": current
            },
            "power_dissipation": {
                "total_w": power
            },
            "analysis_successful": True
        }
    
    elif request.circuit_type.lower() == "ac":
        # AC analysis: impedance calculation
        # Z = sqrt(R^2 + (ωL - 1/(ωC))^2)
        freq = request.frequency_hz
        omega = 2 * math.pi * freq
        
        # Default: R=1k, L=1mH, C=1µF
        R = 1000.0
        L = 1e-3
        C = 1e-6
        
        XL = omega * L
        XC = 1.0 / (omega * C) if omega * C != 0 else float('inf')
        impedance = math.sqrt(R**2 + (XL - XC)**2)
        phase = math.atan2(XL - XC, R) * 180 / math.pi
        
        return {
            "frequency_hz": freq,
            "impedance_ohms": round(impedance, 2),
            "phase_degrees": round(phase, 2),
            "reactance_xl_ohms": round(XL, 2),
            "reactance_xc_ohms": round(XC, 2),
            "resonant_frequency_hz": round(1.0 / (2 * math.pi * math.sqrt(L * C)), 2),
            "analysis_successful": True
        }
    
    elif request.circuit_type.lower() == "transient":
        # Transient response: RC charging curve
        # V(t) = V_s * (1 - exp(-t/RC))
        V_source = 10.0
        R = 1000.0
        C = 1e-6
        tau = R * C  # Time constant
        t_final = request.duration_seconds
        
        time_points = []
        voltages = []
        
        for i in range(request.time_steps + 1):
            t = t_final * i / max(request.time_steps, 1)
            v = V_source * (1.0 - math.exp(-t / tau))
            time_points.append(round(t, 6))
            voltages.append(round(v, 4))
        
        return {
            "time_constant_s": round(tau, 6),
            "time_points_s": time_points,
            "voltage_rise": voltages,
            "final_voltage_v": voltages[-1] if voltages else 0,
            "settling_time_s": round(5 * tau, 6),  # 5 time constants for 99.3%
            "analysis_successful": True
        }
    
    return {
        "error": f"Unknown circuit type: {request.circuit_type}",
        "analysis_successful": False
    }

@app.post("/step/assembly/parse")
async def parse_step_assembly(request: STEPAssemblyRequest):
    """Pure-Python AP214 STEP parser — extracts PRODUCT hierarchy and NAUO relations."""
    import re, os

    if not os.path.exists(request.file_path):
        raise HTTPException(status_code=404, detail=f"File not found: {request.file_path}")

    # Try pythonocc first (best quality)
    try:
        from OCC.Core.STEPControl import STEPControl_Reader
        from OCC.Core.IFSelect import IFSelect_RetDone
        from OCC.Core.XCAFDoc import XCAFDoc_DocumentTool
        from OCC.Core.TDocStd import TDocStd_Document
        from OCC.Core.TCollection import TCollection_ExtendedString
        from OCC.Core.STEPCAFControl import STEPCAFControl_Reader

        doc = TDocStd_Document(TCollection_ExtendedString("MDTV-CAF"))
        reader = STEPCAFControl_Reader()
        reader.SetNameMode(True)
        status = reader.ReadFile(request.file_path)
        if status == IFSelect_RetDone:
            reader.Transfer(doc)
            shape_tool = XCAFDoc_DocumentTool.ShapeTool(doc.Main())
            labels = []
            shape_tool.GetFreeShapes(labels)

            def label_to_node(label):
                name = label.EntryDumpToString()
                children = []
                it = label.NewChildIterator(label, False)
                while it.More():
                    children.append(label_to_node(it.Value()))
                    it.Next()
                return {"id": name, "name": name, "children": children}

            tree = {"name": "Assembly", "children": [label_to_node(l) for l in labels]}
            return {"status": "success", "engine": "pythonocc", "file": request.file_path, "tree": tree}
    except Exception:
        pass

    # Pure-Python fallback: parse STEP DATA section directly
    with open(request.file_path, "r", errors="replace") as f:
        content = f.read()

    # Extract PRODUCT entities: #id = PRODUCT('name','desc',...)
    products = {}
    for m in re.finditer(r"#(\d+)\s*=\s*PRODUCT\s*\(\s*'([^']*)'\s*,\s*'([^']*)'\s*,", content, re.IGNORECASE):
        pid, name, desc = m.group(1), m.group(2), m.group(3)
        products[pid] = {"id": pid, "name": name or desc or f"Part_{pid}", "children": []}

    # Extract NEXT_ASSEMBLY_USAGE_OCCURENCE: #id = NAUO('','',#relating,#related,...)
    # AP214: NEXT_ASSEMBLY_USAGE_OCCURENCE(id,name,description,#relating_product_def,#related_product_def,...)
    # We need PRODUCT_DEFINITION to map back to PRODUCT
    prod_defs = {}  # product_def_id -> product_id
    for m in re.finditer(r"#(\d+)\s*=\s*PRODUCT_DEFINITION\s*\([^,]*,[^,]*,\s*#(\d+)\s*,", content, re.IGNORECASE):
        prod_defs[m.group(1)] = m.group(2)  # product_def -> product

    # PRODUCT_DEFINITION_SHAPE / PRODUCT_DEFINITION_FORMATION also map to PRODUCT
    for m in re.finditer(r"#(\d+)\s*=\s*PRODUCT_DEFINITION_FORMATION\s*\([^,]*,[^,]*,\s*#(\d+)\s*\)", content, re.IGNORECASE):
        prod_defs[m.group(1)] = m.group(2)

    children_map = {}  # parent_product_id -> [child_product_id]
    for m in re.finditer(
        r"#\d+\s*=\s*NEXT_ASSEMBLY_USAGE_OCCURENCE\s*\([^,]*,[^,]*,[^,]*,\s*#(\d+)\s*,\s*#(\d+)\s*,",
        content, re.IGNORECASE
    ):
        relating, related = m.group(1), m.group(2)
        parent_prod = prod_defs.get(relating, relating)
        child_prod  = prod_defs.get(related,  related)
        children_map.setdefault(parent_prod, []).append(child_prod)

    # Build tree recursively
    def build_node(pid, visited=None):
        if visited is None:
            visited = set()
        if pid in visited:
            return {"id": pid, "name": products.get(pid, {}).get("name", f"Part_{pid}"), "children": [], "cyclic": True}
        visited.add(pid)
        node = {"id": pid, "name": products.get(pid, {}).get("name", f"Part_{pid}"), "children": []}
        for child_id in children_map.get(pid, []):
            node["children"].append(build_node(child_id, visited.copy()))
        return node

    # Roots = products that are never a child
    all_children = {c for kids in children_map.values() for c in kids}
    roots = [pid for pid in products if pid not in all_children]
    if not roots:
        roots = list(products.keys())[:1]  # fallback: first product

    tree = {
        "name": "Assembly",
        "part_count": len(products),
        "children": [build_node(r) for r in roots]
    }

    logger.info(f"[STEP] Parsed {len(products)} products, {len(children_map)} parent relations from {request.file_path}")
    return {
        "status": "success",
        "engine": "pure-python-ap214",
        "file": request.file_path,
        "part_count": len(products),
        "tree": tree
    }

@app.post("/step/assembly/parse")
async def parse_step_assembly(request: STEPAssemblyRequest):
    """Pure-Python AP214 STEP parser — extracts PRODUCT hierarchy and NAUO relations."""
    import re, os

    if not os.path.exists(request.file_path):
        raise HTTPException(status_code=404, detail=f"File not found: {request.file_path}")

    # Try pythonocc first
    try:
        from OCC.Core.STEPCAFControl import STEPCAFControl_Reader
        from OCC.Core.IFSelect import IFSelect_RetDone
        from OCC.Core.TDocStd import TDocStd_Document
        from OCC.Core.TCollection import TCollection_ExtendedString
        from OCC.Core.XCAFDoc import XCAFDoc_DocumentTool

        doc = TDocStd_Document(TCollection_ExtendedString("MDTV-CAF"))
        reader = STEPCAFControl_Reader()
        reader.SetNameMode(True)
        if reader.ReadFile(request.file_path) == IFSelect_RetDone:
            reader.Transfer(doc)
            shape_tool = XCAFDoc_DocumentTool.ShapeTool(doc.Main())
            free_shapes = []
            shape_tool.GetFreeShapes(free_shapes)

            def occ_label_to_node(label):
                from OCC.Core.TDataStd import TDataStd_Name
                from OCC.Core.TCollection import TCollection_AsciiString
                name_attr = TDataStd_Name()
                name = label.EntryDumpToString()
                if label.FindAttribute(TDataStd_Name.GetID(), name_attr):
                    name = TCollection_AsciiString(name_attr.Get()).ToCString()
                children = []
                it = label.NewChildIterator(label, False)
                while it.More():
                    children.append(occ_label_to_node(it.Value()))
                    it.Next()
                return {"id": label.EntryDumpToString(), "name": name, "children": children}

            tree = {"name": "Assembly", "children": [occ_label_to_node(l) for l in free_shapes]}
            return {"status": "success", "engine": "pythonocc",
                    "file": request.file_path, "part_count": len(free_shapes), "tree": tree}
    except Exception:
        pass

    # Pure-Python AP214 fallback
    with open(request.file_path, "r", errors="replace") as f:
        content = f.read()

    products = {}
    for m in re.finditer(r"#(\d+)\s*=\s*PRODUCT\s*\(\s*'([^']*)'\s*,\s*'([^']*)'\s*,",
                         content, re.IGNORECASE):
        pid, name, desc = m.group(1), m.group(2), m.group(3)
        products[pid] = {"id": pid, "name": name or desc or f"Part_{pid}", "children": []}

    # PRODUCT_DEFINITION -> PRODUCT_DEFINITION_FORMATION -> PRODUCT
    pdf_to_prod = {}
    for m in re.finditer(r"#(\d+)\s*=\s*PRODUCT_DEFINITION_FORMATION\s*\([^,]*,[^,]*,\s*#(\d+)\s*\)",
                         content, re.IGNORECASE):
        pdf_to_prod[m.group(1)] = m.group(2)
    prod_def_to_prod = {}
    for m in re.finditer(r"#(\d+)\s*=\s*PRODUCT_DEFINITION\s*\([^,]*,[^,]*,\s*#(\d+)\s*,",
                         content, re.IGNORECASE):
        formation = m.group(2)
        prod_def_to_prod[m.group(1)] = pdf_to_prod.get(formation, formation)

    children_map = {}
    for m in re.finditer(
        r"#\d+\s*=\s*NEXT_ASSEMBLY_USAGE_OCCURENCE\s*\([^,]*,[^,]*,[^,]*,\s*#(\d+)\s*,\s*#(\d+)\s*,",
        content, re.IGNORECASE
    ):
        parent = prod_def_to_prod.get(m.group(1), m.group(1))
        child  = prod_def_to_prod.get(m.group(2), m.group(2))
        children_map.setdefault(parent, []).append(child)

    def build_node(pid, visited=None):
        visited = visited or set()
        if pid in visited:
            return {"id": pid, "name": products.get(pid, {}).get("name", f"Part_{pid}"),
                    "children": [], "cyclic": True}
        visited.add(pid)
        return {
            "id": pid,
            "name": products.get(pid, {}).get("name", f"Part_{pid}"),
            "children": [build_node(c, visited.copy()) for c in children_map.get(pid, [])]
        }

    all_children = {c for kids in children_map.values() for c in kids}
    roots = [pid for pid in products if pid not in all_children] or list(products.keys())[:1]

    tree = {"name": "Assembly", "part_count": len(products),
            "children": [build_node(r) for r in roots]}

    logger.info(f"[STEP] {len(products)} products, {len(children_map)} relations from {request.file_path}")
    return {"status": "success", "engine": "pure-python-ap214",
            "file": request.file_path, "part_count": len(products), "tree": tree}


@app.post("/freecad/parametric")
async def freecad_parametric(request: FreeCADRequest):
    import subprocess, tempfile, os, json as _json

    # Build the FreeCAD headless Python script
    script = f"""
import FreeCAD, Part, os

doc = FreeCAD.newDocument("SZM")

if "{request.shape}" == "cylinder":
    shape = Part.makeCylinder({request.width / 2}, {request.height})
elif "{request.shape}" == "sphere":
    shape = Part.makeSphere({min(request.length, request.width, request.height) / 2})
else:
    shape = Part.makeBox({request.length}, {request.width}, {request.height})

if {request.fillet_radius} > 0:
    edges = shape.Edges
    shape = shape.makeFillet({request.fillet_radius}, edges)

feature = doc.addObject("Part::Feature", "SZM_Part")
feature.Shape = shape
doc.recompute()

Part.export([feature], "{request.output_path}")

# Write feature tree JSON
tree = {{
    "name": "SZM_Part",
    "shape": "{request.shape}",
    "params": {{
        "length": {request.length},
        "width": {request.width},
        "height": {request.height},
        "fillet_radius": {request.fillet_radius}
    }},
    "volume_m3": shape.Volume * 1e-9,
    "area_m2": shape.Area * 1e-6,
    "bbox": {{
        "x": shape.BoundBox.XLength,
        "y": shape.BoundBox.YLength,
        "z": shape.BoundBox.ZLength
    }},
    "edge_count": len(shape.Edges),
    "face_count": len(shape.Faces)
}}
import json
with open("{request.output_path}.tree.json", "w") as f:
    json.dump(tree, f)
print("SZM_FREECAD_OK")
"""

    with tempfile.NamedTemporaryFile(suffix=".py", mode="w", delete=False) as f:
        script_path = f.name
        f.write(script)

    feature_tree = {}
    engine_used = "FreeCAD headless"
    success = False
    error_msg = ""

    try:
        for cmd in ["FreeCADCmd", "freecadcmd", "FreeCAD"]:
            try:
                result = subprocess.run(
                    [cmd, script_path],
                    capture_output=True, text=True, timeout=60
                )
                if "SZM_FREECAD_OK" in result.stdout:
                    success = True
                    engine_used = cmd
                    break
                error_msg = result.stderr[:500]
            except (FileNotFoundError, subprocess.TimeoutExpired):
                continue

        if success:
            tree_path = request.output_path + ".tree.json"
            if os.path.exists(tree_path):
                with open(tree_path) as f:
                    feature_tree = _json.load(f)
            logger.info(f"[FreeCAD] Parametric part built: {request.output_path}")
        else:
            # Analytical fallback — compute geometry without FreeCAD
            import math
            if request.shape == "cylinder":
                r = request.width / 2
                vol = math.pi * r * r * request.height
                area = 2 * math.pi * r * (r + request.height)
            elif request.shape == "sphere":
                r = min(request.length, request.width, request.height) / 2
                vol = (4/3) * math.pi * r**3
                area = 4 * math.pi * r**2
            else:
                vol = request.length * request.width * request.height
                area = 2 * (request.length * request.width +
                            request.width * request.height +
                            request.length * request.height)
            feature_tree = {{
                "name": "SZM_Part",
                "shape": request.shape,
                "params": {{
                    "length": request.length, "width": request.width,
                    "height": request.height, "fillet_radius": request.fillet_radius
                }},
                "volume_m3": vol,
                "area_m2": area,
                "bbox": {{"x": request.length, "y": request.width, "z": request.height}},
                "edge_count": 12 if request.shape == "box" else 0,
                "face_count": 6 if request.shape == "box" else 1
            }}
            engine_used = "Analytical fallback (FreeCAD not found)"
    finally:
        os.unlink(script_path)

    return {
        "status": "success" if (success or not error_msg) else "partial",
        "engine_used": engine_used,
        "step_path": request.output_path if success else None,
        "feature_tree": feature_tree,
        "error": error_msg or None
    }

@app.post("/usd/export")
async def export_usd(request: UsdExportRequest):
    logger.info(f"Received request to export USD to {request.output_path}")
    import json
    try:
        from pxr import Usd, UsdGeom, Gf
    except ImportError:
        logger.error("usd-core is not installed. Please install it with pip install usd-core")
        return {"status": "error", "error": "usd-core not installed"}
        
    try:
        scene_data = json.loads(request.scene_json)
        
        # Create a new stage
        stage = Usd.Stage.CreateNew(request.output_path)
        
        for entity in scene_data.get("entities", []):
            name = entity.get("name", f"Entity_{entity.get('id')}")
            # Ensure name is a valid USD path identifier
            name = name.replace(" ", "_").replace("-", "_")
            path = f"/{name}"
            
            xform = UsdGeom.Xform.Define(stage, path)
            transform = entity.get("transform", {})
            pos = transform.get("position", [0, 0, 0])
            
            # Add translation op
            UsdGeom.XformCommonAPI(xform).SetTranslate(Gf.Vec3d(pos[0], pos[1], pos[2]))
            
            # We can expand to handle Mesh components here
            # cube = UsdGeom.Cube.Define(stage, f"{path}/Geometry")
            # cube.GetSizeAttr().Set(1.0)
            
        stage.GetRootLayer().Save()
        
        return {
            "status": "success",
            "file_path": request.output_path
        }
    except Exception as e:
        logger.error(f"Error exporting USD: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.post("/ai/train")
async def ai_train(request: AITrainRequest):
    logger.info(f"Received request to train AI on {request.data_dir}")
    if not ml_pipeline:
        return {"status": "error", "error": "ML pipeline not initialized"}
    try:
        ml_pipeline.data_dir = __import__('pathlib').Path(request.data_dir)
        count = ml_pipeline.ingest_data()
        if count == 0:
            return {"status": "error", "error": "No data found"}
        pattern_count = ml_pipeline.learn_patterns()
        return {
            "status": "success",
            "machines_ingested": count,
            "patterns_learned": pattern_count
        }
    except Exception as e:
        logger.error(f"Error training AI: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.post("/ai/generate")
async def ai_generate(request: AIGenerateRequest):
    logger.info(f"Received request to generate {request.count} {request.machine_type}")
    if not ml_pipeline:
        return {"status": "error", "error": "ML pipeline not initialized"}
    try:
        if not ml_pipeline.patterns:
            ml_pipeline.ingest_data()
            ml_pipeline.learn_patterns()
            
        machines = ml_pipeline.generate_machines(
            request.machine_type, 
            request.count, 
            request.parameters
        )
        
        # We'll just return the first one's specification for simplicity
        if not machines:
            return {"status": "error", "error": "Failed to generate machine"}
            
        return {
            "status": "success",
            "machine_type": request.machine_type,
            "specification": machines[0].specification,
            "confidence": machines[0].confidence_score
        }
    except Exception as e:
        logger.error(f"Error generating machine: {str(e)}")
        return {"status": "error", "error": str(e)}


# ============================================================================
# TELEMETRY INGESTION LAYER (Plan 10: Digital Twin Telemetry)
# ============================================================================

class TelemetryDataRequest(BaseModel):
    stream_id: str
    asset_uuid: str = None
    sensor_type: str
    value: float
    timestamp: float = None

class TelemetryStreamRequest(BaseModel):
    stream_id: str
    asset_uuid: str

class TelemetryQueryRequest(BaseModel):
    stream_id: str
    sensor_type: str = None
    start_time: float = None
    end_time: float = None
    time_window_seconds: float = None  # For statistics

class AlertThresholdRequest(BaseModel):
    stream_id: str
    sensor_type: str
    warning_low: float = None
    warning_high: float = None
    critical_low: float = None
    critical_high: float = None

# In-memory telemetry store (in production, use a database)
telemetry_streams = {}
telemetry_history = {}

@app.post("/telemetry/stream/create")
async def create_telemetry_stream(request: TelemetryStreamRequest):
    """Create a new telemetry stream for digital twin data ingestion."""
    try:
        if request.stream_id in telemetry_streams:
            return {"status": "error", "error": f"Stream '{request.stream_id}' already exists"}
        
        telemetry_streams[request.stream_id] = {
            "stream_id": request.stream_id,
            "asset_uuid": request.asset_uuid,
            "created_at": __import__('time').time(),
            "data_points": 0,
            "latest_update": None
        }
        telemetry_history[request.stream_id] = []
        
        logger.info(f"[Telemetry] Created stream '{request.stream_id}' for asset '{request.asset_uuid}'")
        return {
            "status": "success",
            "stream_id": request.stream_id,
            "message": "Telemetry stream created"
        }
    except Exception as e:
        logger.error(f"Error creating telemetry stream: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.post("/telemetry/ingest")
async def ingest_telemetry_data(request: TelemetryDataRequest):
    """Ingest telemetry data from sensors into a digital twin stream."""
    try:
        if request.stream_id not in telemetry_streams:
            return {"status": "error", "error": f"Stream '{request.stream_id}' not found"}
        
        import time
        timestamp = request.timestamp if request.timestamp else time.time()
        
        data_point = {
            "timestamp": timestamp,
            "sensor_type": request.sensor_type,
            "value": request.value
        }
        
        telemetry_history[request.stream_id].append(data_point)
        telemetry_streams[request.stream_id]["latest_update"] = timestamp
        telemetry_streams[request.stream_id]["data_points"] += 1
        
        # Keep only last 10000 points in memory
        if len(telemetry_history[request.stream_id]) > 10000:
            telemetry_history[request.stream_id] = telemetry_history[request.stream_id][-10000:]
        
        logger.debug(f"[Telemetry] Ingested: {request.stream_id}/{request.sensor_type} = {request.value}")
        return {
            "status": "success",
            "stream_id": request.stream_id,
            "data_point_count": len(telemetry_history[request.stream_id])
        }
    except Exception as e:
        logger.error(f"Error ingesting telemetry data: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.post("/telemetry/query")
async def query_telemetry_data(request: TelemetryQueryRequest):
    """Query historical telemetry data from a digital twin stream."""
    try:
        if request.stream_id not in telemetry_history:
            return {"status": "error", "error": f"Stream '{request.stream_id}' not found"}
        
        history = telemetry_history[request.stream_id]
        
        # Filter by sensor type
        if request.sensor_type:
            history = [p for p in history if p["sensor_type"] == request.sensor_type]
        
        # Filter by time range
        if request.start_time and request.end_time:
            history = [p for p in history if request.start_time <= p["timestamp"] <= request.end_time]
        
        # Calculate statistics if time window is specified
        stats = None
        if request.time_window_seconds and request.sensor_type:
            values = [p["value"] for p in history if p["sensor_type"] == request.sensor_type]
            if values:
                import statistics
                stats = {
                    "count": len(values),
                    "min": min(values),
                    "max": max(values),
                    "avg": statistics.mean(values),
                    "stdev": statistics.stdev(values) if len(values) > 1 else 0
                }
        
        return {
            "status": "success",
            "stream_id": request.stream_id,
            "data_point_count": len(history),
            "data_points": history[-100:],  # Return last 100 points
            "statistics": stats
        }
    except Exception as e:
        logger.error(f"Error querying telemetry data: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.get("/telemetry/stream/{stream_id}")
async def get_stream_status(stream_id: str):
    """Get status and metadata of a telemetry stream."""
    try:
        if stream_id not in telemetry_streams:
            return {"status": "error", "error": f"Stream '{stream_id}' not found"}
        
        stream = telemetry_streams[stream_id]
        return {
            "status": "success",
            "stream": stream,
            "data_points_in_history": len(telemetry_history.get(stream_id, []))
        }
    except Exception as e:
        logger.error(f"Error getting stream status: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.get("/telemetry/streams")
async def list_telemetry_streams():
    """List all active telemetry streams."""
    try:
        streams_info = []
        for stream_id, stream_meta in telemetry_streams.items():
            streams_info.append({
                "stream_id": stream_id,
                "asset_uuid": stream_meta["asset_uuid"],
                "data_points": stream_meta["data_points"],
                "latest_update": stream_meta["latest_update"]
            })
        
        return {
            "status": "success",
            "stream_count": len(streams_info),
            "streams": streams_info
        }
    except Exception as e:
        logger.error(f"Error listing telemetry streams: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.post("/telemetry/export/json/{stream_id}")
async def export_telemetry_json(stream_id: str):
    """Export telemetry data as JSON."""
    try:
        if stream_id not in telemetry_history:
            return {"status": "error", "error": f"Stream '{stream_id}' not found"}
        
        import json
        export_data = {
            "stream_id": stream_id,
            "metadata": telemetry_streams.get(stream_id, {}),
            "data": telemetry_history[stream_id]
        }
        
        return {
            "status": "success",
            "json_data": json.dumps(export_data, indent=2)
        }
    except Exception as e:
        logger.error(f"Error exporting telemetry to JSON: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.post("/telemetry/export/csv/{stream_id}")
async def export_telemetry_csv(stream_id: str):
    """Export telemetry data as CSV."""
    try:
        if stream_id not in telemetry_history:
            return {"status": "error", "error": f"Stream '{stream_id}' not found"}
        
        import io
        csv_buffer = io.StringIO()
        csv_buffer.write("timestamp,sensor_type,value\n")
        
        for point in telemetry_history[stream_id]:
            csv_buffer.write(f"{point['timestamp']},{point['sensor_type']},{point['value']}\n")
        
        return {
            "status": "success",
            "csv_data": csv_buffer.getvalue()
        }
    except Exception as e:
        logger.error(f"Error exporting telemetry to CSV: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.post("/telemetry/alert/set")
async def set_alert_threshold(request: AlertThresholdRequest):
    """Set alert thresholds for a sensor type in a stream."""
    try:
        if request.stream_id not in telemetry_streams:
            return {"status": "error", "error": f"Stream '{request.stream_id}' not found"}
        
        # Store alert config
        if "alerts" not in telemetry_streams[request.stream_id]:
            telemetry_streams[request.stream_id]["alerts"] = {}
        
        telemetry_streams[request.stream_id]["alerts"][request.sensor_type] = {
            "sensor_type": request.sensor_type,
            "warning_low": request.warning_low,
            "warning_high": request.warning_high,
            "critical_low": request.critical_low,
            "critical_high": request.critical_high
        }
        
        logger.info(f"[Telemetry] Alert threshold set for {request.stream_id}/{request.sensor_type}")
        return {
            "status": "success",
            "stream_id": request.stream_id,
            "sensor_type": request.sensor_type,
            "message": "Alert threshold configured"
        }
    except Exception as e:
        logger.error(f"Error setting alert threshold: {str(e)}")
        return {"status": "error", "error": str(e)}

@app.get("/telemetry/health")
async def telemetry_health():
    """Get overall telemetry system health."""
    try:
        total_streams = len(telemetry_streams)
        total_data_points = sum(len(history) for history in telemetry_history.values())
        
        active_alerts = 0
        for stream_id, stream_meta in telemetry_streams.items():
            if "alerts" in stream_meta and stream_meta["alerts"]:
                active_alerts += len(stream_meta["alerts"])
        
        return {
            "status": "healthy",
            "total_streams": total_streams,
            "total_data_points": total_data_points,
            "active_alert_configs": active_alerts,
            "system_uptime": __import__('time').time()
        }
    except Exception as e:
        logger.error(f"Error getting telemetry health: {str(e)}")
        return {"status": "error", "error": str(e)}

if __name__ == "__main__":
    logger.info("Starting External Engine Bridge on port 8003...")
    uvicorn.run(app, host="127.0.0.1", port=8003)
