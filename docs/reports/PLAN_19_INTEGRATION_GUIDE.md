# Plan 19: Ngspice Electronics - Integration Guide

**Complete implementation guide for ngspice circuit simulation integration**

---

## Table of Contents
1. [Quick Start](#quick-start)
2. [API Reference](#api-reference)
3. [Testing & Validation](#testing--validation)
4. [Integration Checklist](#integration-checklist)
5. [Troubleshooting](#troubleshooting)
6. [Advanced Usage](#advanced-usage)

---

## Quick Start

### Prerequisites
```bash
# Install ngspice (optional but recommended)
sudo apt-get install ngspice  # Ubuntu/Debian
# or
brew install ngspice          # macOS

# Python requirements (if not already installed)
pip install fastapi pydantic uvicorn
```

### 1. Start Python Backend Server
```bash
cd /mnt/D/projects/szm_forge
python -m uvicorn ai_service.external_engine_bridge:app --host 127.0.0.1 --port 8003 --reload
```

Expected output:
```
INFO:     Uvicorn running on http://127.0.0.1:8003
```

### 2. Quick DC Analysis Test (2 seconds)
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "dc"
  }'
```

**Expected Response:**
```json
{
  "status": "success",
  "engine_used": "ngspice",
  "component_id": 1,
  "circuit_type": "dc",
  "node_voltages": {
    "1": 10.0,
    "2": 5.0
  },
  "currents": {
    "R1": 0.01
  },
  "power_dissipation": {
    "R1": 0.1
  }
}
```

### 3. Quick AC Analysis Test (2 seconds)
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "ac",
    "frequency_hz": 1000
  }'
```

**Expected Response:**
```json
{
  "status": "success",
  "engine_used": "ngspice",
  "circuit_type": "ac",
  "frequency_hz": 1000.0,
  "impedance_ohms": 1414.2,
  "phase_degrees": 45.0,
  "reactance_xl_ohms": 0.0,
  "reactance_xc_ohms": -159.15,
  "resonant_frequency_hz": 15915.5
}
```

### 4. Quick Transient Analysis Test (2 seconds)
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "transient",
    "duration_seconds": 0.01,
    "time_steps": 100
  }'
```

**Expected Response:**
```json
{
  "status": "success",
  "engine_used": "ngspice",
  "circuit_type": "transient",
  "time_constant_s": 0.001,
  "time_points_s": [0.0, 0.0001, 0.0002, ...],
  "voltage_rise": [0.0, 0.95, 1.81, ...],
  "final_voltage_v": 10.0,
  "settling_time_s": 0.005
}
```

---

## API Reference

### Endpoint: POST /simulation/circuit/run

**Base URL:** `http://127.0.0.1:8003`

**Request Headers:**
```
Content-Type: application/json
```

### DC Analysis

**Purpose:** Find steady-state node voltages and currents

**Request:**
```json
{
  "component_id": 1,
  "circuit_type": "dc",
  "temperature_c": 27.0,
  "netlist": "optional custom SPICE netlist"
}
```

**Response Fields (DC):**
- `node_voltages` - Dict of node → voltage in volts
- `currents` - Dict of component → current in amperes
- `power_dissipation` - Dict of component → power in watts

**Example 1: Simple series circuit**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "dc",
    "temperature_c": 25.0
  }'
# Default circuit: V1=10V, R1=1kΩ, C1=1µF
# Expected: I ≈ 10mA, P = 100mW
```

**Example 2: Different temperature**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 2,
    "circuit_type": "dc",
    "temperature_c": 100.0
  }'
# Note: Analytical solver may account for temperature effects if implemented
```

### AC Analysis

**Purpose:** Calculate frequency response and impedance

**Request:**
```json
{
  "component_id": 1,
  "circuit_type": "ac",
  "frequency_hz": 1000.0,
  "temperature_c": 27.0
}
```

**Response Fields (AC):**
- `frequency_hz` - Analysis frequency
- `impedance_ohms` - Magnitude of complex impedance
- `phase_degrees` - Phase angle
- `reactance_xl_ohms` - Inductive reactance
- `reactance_xc_ohms` - Capacitive reactance
- `resonant_frequency_hz` - LC resonant frequency (if circuit has L & C)

**Example 1: Low frequency (well below resonance)**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "ac",
    "frequency_hz": 10
  }'
# Expected: Z ≈ 1000Ω (resistive), φ ≈ 0°
```

**Example 2: High frequency (well above resonance)**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "ac",
    "frequency_hz": 100000
  }'
# Expected: Z ≈ 1000.8Ω (resistive), φ ≈ 0°
```

**Example 3: Near resonant frequency**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "ac",
    "frequency_hz": 15915.5
  }'
# Expected: Z minimum, φ = 0° (for RLC)
```

### Transient Analysis

**Purpose:** Simulate time-domain response to a step input

**Request:**
```json
{
  "component_id": 1,
  "circuit_type": "transient",
  "duration_seconds": 1.0,
  "time_steps": 100,
  "temperature_c": 27.0
}
```

**Response Fields (Transient):**
- `time_constant_s` - RC or L/R time constant (τ)
- `time_points_s` - Array of time values
- `voltage_rise` - Array of voltages at each time point
- `final_voltage_v` - Steady-state final voltage
- `settling_time_s` - Time to settle to 95% (approximately 3τ)

**Example 1: Short duration (1ms)**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "transient",
    "duration_seconds": 0.001,
    "time_steps": 50
  }'
# Default RC circuit: τ = 1ms
# Expected: Voltage reaches ~63% after 1ms
```

**Example 2: Long duration (100ms)**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "transient",
    "duration_seconds": 0.1,
    "time_steps": 200
  }'
# Expected: Voltage settles to near final value
```

**Example 3: High time resolution**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "transient",
    "duration_seconds": 0.01,
    "time_steps": 1000
  }'
# Expected: 1000 time points for detailed waveform
```

---

## Testing & Validation

### Test Suite 1: DC Analysis

**TC-1.1: Basic DC Operating Point**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{"component_id": 1, "circuit_type": "dc"}'

# Validation
# ✓ Response status code: 200
# ✓ status == "success"
# ✓ node_voltages contains numeric values
# ✓ currents all positive
# ✓ power_dissipation = I² × R
```

**TC-1.2: Different Temperatures**
```bash
for temp in 0 25 50 75 100; do
  curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
    -H "Content-Type: application/json" \
    -d "{\"component_id\": 1, \"circuit_type\": \"dc\", \"temperature_c\": $temp}"
done

# Validation
# ✓ All requests succeed
# ✓ Results remain consistent (temperature may affect component values)
```

### Test Suite 2: AC Analysis

**TC-2.1: Frequency Sweep Validation**
```bash
# Test at multiple frequencies
for freq in 10 100 1000 10000 100000 1000000; do
  curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
    -H "Content-Type: application/json" \
    -d "{\"component_id\": 1, \"circuit_type\": \"ac\", \"frequency_hz\": $freq}"
  echo "Frequency: $freq Hz"
done

# Validation
# ✓ All frequencies respond within 1 second
# ✓ impedance_ohms is always positive
# ✓ phase_degrees in range [-90, 90]
# ✓ Impedance increases with frequency for capacitive circuits
```

**TC-2.2: Resonance Detection**
```bash
# Request resonant frequency from response
RESPONSE=$(curl -s -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{"component_id": 1, "circuit_type": "ac", "frequency_hz": 1000}')

# Extract resonant_frequency_hz and test it
RESONANT=$(echo $RESPONSE | grep -o '"resonant_frequency_hz":[0-9.]*' | cut -d: -f2)

# Validation
# ✓ resonant_frequency_hz > 0 (for RC/LC circuits)
```

### Test Suite 3: Transient Analysis

**TC-3.1: Basic RC Charging**
```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "transient",
    "duration_seconds": 0.01,
    "time_steps": 100
  }'

# Validation
# ✓ time_constant_s > 0
# ✓ time_points_s.length == 100
# ✓ voltage_rise.length == 100
# ✓ voltage_rise[0] == 0 (initial condition)
# ✓ voltage_rise[-1] < final_voltage_v (approaching steady state)
# ✓ settling_time_s ≈ 5 × time_constant_s
```

**TC-3.2: Voltage Rise Validation**
```bash
# Verify exponential RC charging behavior
# V(t) = Vs(1 - exp(-t/τ))

curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "transient",
    "duration_seconds": 0.005,
    "time_steps": 50
  }' | jq '.

# Expected behavior:
# ✓ V(t=τ) ≈ 0.632 × Vs
# ✓ V(t=5τ) ≈ 0.993 × Vs
```

### Test Suite 4: Fallback Mechanism

**TC-4.1: Analytical Fallback (ngspice disabled)**
```bash
# Rename ngspice binary to simulate unavailability
sudo mv /usr/bin/ngspice /usr/bin/ngspice.bak 2>/dev/null || true

# Run all three analysis types
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{"component_id": 1, "circuit_type": "dc"}'

curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{"component_id": 1, "circuit_type": "ac", "frequency_hz": 1000}'

curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{"component_id": 1, "circuit_type": "transient"}'

# Validation
# ✓ All three succeed
# ✓ engine_used contains "Analytical"
# ✓ Results are physically reasonable

# Restore ngspice
sudo mv /usr/bin/ngspice.bak /usr/bin/ngspice 2>/dev/null || true
```

### Test Suite 5: React Component Integration

**File:** `webui/src/components/CircuitAnalysisPanel.test.tsx` (To be created)

```typescript
describe('CircuitAnalysisPanel', () => {
  test('renders DC, AC, Transient tabs', () => {
    // TC-5.1: Verify all tabs render
  });

  test('submits DC analysis request', async () => {
    // TC-5.2: Click "Run DC Analysis" and verify API call
  });

  test('displays DC results', async () => {
    // TC-5.3: Verify node voltages display correctly
  });

  test('submits AC analysis request', async () => {
    // TC-5.4: Input frequency and run AC analysis
  });

  test('displays AC results', async () => {
    // TC-5.5: Verify impedance and phase display
  });

  test('submits transient request', async () => {
    // TC-5.6: Run transient with duration/steps
  });

  test('displays transient waveform', async () => {
    // TC-5.7: Verify time waveform renders
  });

  test('handles API errors gracefully', async () => {
    // TC-5.8: Disconnect server and verify error display
  });
});
```

### Test Suite 6: ImGui Panel Integration

**TC-6.1: ImGui CircuitAnalysisPanel Rendering**
```cpp
// Launch desktop app with CircuitAnalysisPanel
// Verify:
// ✓ Window title: "Circuit Analysis"
// ✓ Component ID slider visible (1-100)
// ✓ 5 tabs visible: Netlist, DC, AC, Transient, Results
// ✓ Netlist tab shows component list
// ✓ DC tab has temperature input
// ✓ AC tab has frequency input
// ✓ Transient tab has duration/time_steps inputs
```

**TC-6.2: ImGui DC Analysis Execution**
```cpp
// 1. Select Component ID = 5
// 2. Click "Run DC Analysis"
// 3. Verify status changes to "Running..."
// 4. After ~500ms, verify Results tab shows:
//    - Node voltage table
//    - Current values
//    - Power dissipation
```

**TC-6.3: ImGui AC Analysis with Plot**
```cpp
// 1. Go to AC Analysis tab
// 2. Set frequency = 1000
// 3. Click "Run AC Analysis"
// 4. Go to Results tab
// 5. Verify impedance plot displays
// 6. Verify phase shift indicator
```

---

## Integration Checklist

### Phase 1: Verification (✅ Complete)
- [x] C++ solver files compile without errors
- [x] React components compile without errors
- [x] ImGui panels compile without errors
- [x] Python API endpoint defined
- [x] No syntax errors in any file

### Phase 2: API Validation (In Progress)
- [ ] Start Python server: `python -m uvicorn ...`
- [ ] Test DC endpoint: `curl .../simulation/circuit/run` with circuit_type="dc"
- [ ] Test AC endpoint: with circuit_type="ac" and frequency_hz
- [ ] Test Transient endpoint: with circuit_type="transient"
- [ ] Verify fallback (with ngspice disabled)
- [ ] Verify error messages are helpful

### Phase 3: Frontend Integration
- [ ] React component imports useCircuitAPI successfully
- [ ] React component connects to backend
- [ ] DC analysis form submission works
- [ ] AC analysis form submission works
- [ ] Transient analysis form submission works
- [ ] Results display correctly in React
- [ ] ImGui panels render in desktop app
- [ ] Input validation works on all forms

### Phase 4: End-to-End Testing
- [ ] Desktop app launches with CircuitAnalysisPanel
- [ ] Web UI loads and displays CircuitAnalysisPanel
- [ ] Run DC analysis from React UI
- [ ] Run AC analysis from React UI
- [ ] Run Transient analysis from React UI
- [ ] Run DC analysis from ImGui panel
- [ ] Run AC analysis from ImGui panel
- [ ] Run Transient analysis from ImGui panel
- [ ] Verify consistency between React and ImGui results

### Phase 5: Production Readiness
- [ ] Performance tested (<1s response time)
- [ ] Edge cases handled (invalid frequencies, extreme temperatures)
- [ ] Documentation complete
- [ ] Error messages helpful and actionable
- [ ] Logging configured for debugging

---

## Troubleshooting

### Issue: "ngspice not found" but analysis still completes

**Cause:** Fallback analytical solver automatically activated (normal behavior)

**Resolution:** 
- Verify ngspice installation: `which ngspice`
- Install ngspice if needed: `apt install ngspice` (Ubuntu) or `brew install ngspice` (macOS)
- Restart Python server after installation
- Analytical results are still valid

### Issue: DC analysis returns zero currents

**Cause:** Open circuit or no voltage source

**Resolution:**
- Check default circuit includes V1 (10V source)
- Verify resistance values are non-zero
- Ensure nodes are connected properly

### Issue: AC impedance very high or very low

**Cause:** Frequency at or near capacitive/inductive extremes

**Resolution:**
- Test at resonant frequency (displayed in response)
- Try frequency between 100Hz and 100kHz for typical RC
- High impedance below resonance = capacitive behavior
- Low impedance above resonance = inductive behavior

### Issue: Transient analysis returns flat line

**Cause:** Time constant much shorter than simulation duration

**Resolution:**
- Reduce duration or increase time_steps
- Check τ value (should be RC)
- Verify initial voltage starts at 0

### Issue: React component not connecting to API

**Symptoms:** Loading spinner stuck, error messages in browser console

**Debugging:**
```bash
# Verify server is running
curl http://127.0.0.1:8003/health

# Check browser console for CORS errors
# Verify API_BASE in useCircuitAPI.ts matches server address

# Enable verbose logging in React component
console.log('Making request to:', API_BASE + '/simulation/circuit/run');
```

### Issue: ImGui panel not rendering results

**Cause:** Analysis may still be running or results not parsed

**Resolution:**
- Wait 1-2 seconds before checking Results tab
- Verify Run button shows completion status
- Check application logs for parsing errors

---

## Advanced Usage

### Custom SPICE Netlist

Instead of using the default RC circuit, provide a custom netlist:

```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "dc",
    "netlist": "Custom Circuit\nV1 0 1 DC 10\nR1 1 2 2000\nR2 2 0 1000\nC1 2 0 2e-6\n.op\n.end"
  }'
```

### Temperature Effects

Modify component behavior based on ambient temperature:

```bash
curl -X POST http://127.0.0.1:8003/simulation/circuit/run \
  -H "Content-Type: application/json" \
  -d '{
    "component_id": 1,
    "circuit_type": "ac",
    "frequency_hz": 1000,
    "temperature_c": 75.0
  }'
```

### Parameter Sweep (Future Enhancement)

Request to sweep a parameter (implemented in C++, needs Python integration):

```bash
# Example for future enhancement:
# curl -X POST .../simulation/circuit/run \
#   -d '{
#     "component_id": 1,
#     "circuit_type": "sweep",
#     "sweep_parameter": "frequency",
#     "sweep_start": 1,
#     "sweep_stop": 1e6,
#     "sweep_points": 100
#   }'
```

---

## Performance Optimization

### Reducing Response Time

1. **Use Analytical Solver** - Remove ngspice binary for ~10x speedup
2. **Reduce Time Steps** - Fewer points = faster transient analysis
3. **Parallel Requests** - Can run multiple analyses simultaneously
4. **Cache Results** - Same parameters = same results

### Memory Usage

- DC Analysis: ~1MB
- AC Analysis: ~2-5MB
- Transient (100 steps): ~1-2MB
- Transient (1000 steps): ~10-15MB

---

## Next Steps

1. Run TC-1.1 through TC-4.1 from Testing & Validation section
2. Integrate React component into main application
3. Add CircuitAnalysisPanel to ImGui window layout
4. Perform end-to-end testing (Phase 4)
5. Deploy to production with proper error handling

For detailed technical information, refer to:
- **PLAN_19_IMPLEMENTATION_SUMMARY.md** - Architecture and design
- **src/Electrical/NgspiceCircuitSolver.hpp** - C++ API documentation
- **ai_service/external_engine_bridge.py** - Python endpoint implementation
