#pragma once

/**
 * @file ConfigConstants.hpp
 * @brief Centralized configuration constants for the SZM Forge engine.
 *
 * This file contains all tunable parameters, constants, and configuration values
 * used throughout the engine. Centralizing these values improves maintainability,
 * enables easy tuning, and prevents magic numbers scattered throughout the codebase.
 *
 * **Organization:**
 * - Physics simulation parameters
 * - Graphics rendering settings
 * - UI/UX dimensions and timing
 * - Material database defaults
 * - Error tolerance thresholds
 * - Logging configuration
 *
 * @see Individual module headers for module-specific documentation
 */

namespace SZM::Config {

// ============================================================
//  PHYSICS ENGINE PARAMETERS
// ============================================================

/// @defgroup PhysicsConfig Physics Engine Configuration
/// @{

namespace Physics {
    /// Standard Earth gravitational acceleration (m/s²)
    constexpr float StandardGravity = 9.81f;

    /// Default gravity vector for new simulations [m/s²]
    constexpr float DefaultGravityX = 0.0f;
    constexpr float DefaultGravityY = -9.81f;
    constexpr float DefaultGravityZ = 0.0f;

    /// Physics timestep for stable integration
    /// Recommended range: 0.01s (100 Hz) to 0.033s (30 Hz)
    /// Default: 0.0166s ≈ 60 Hz
    constexpr float DefaultTimestep = 0.0166f;

    /// Maximum safe timestep for integration stability
    constexpr float MaxTimestep = 0.05f;

    /// Angular velocity damping factor per frame
    /// Applied as: ω *= DampingFactor each integration step
    /// Range: 0.0 (infinite damping) to 1.0 (no damping)
    constexpr float AngularVelocityDamping = 0.999f;

    /// Minimum volume threshold for mesh-to-body conversion (m³)
    /// Meshes with smaller volume are considered degenerate
    constexpr float MinVolumeThreshold = 1e-6f;

    /// Matrix singularity threshold for inversion
    /// Matrices with |determinant| < this are considered singular
    constexpr float MatrixSingularityThreshold = 1e-6f;

    /// Tetrahedron volume formula divisor: V = det / 6
    constexpr float TetrahedronVolumeDivisor = 6.0f;

    /// Tetrahedron center of gravity formula divisor: CoG = sum / 4
    constexpr float TetrahedronCGDivisor = 4.0f;

    /// Inertia integration constant: I_x2 = (det/60) * ...
    constexpr float InertiaIntegral60Divisor = 60.0f;

    /// Inertia integration constant: I_xy = (det/120) * ...
    constexpr float InertiaIntegral120Divisor = 120.0f;

    /// GJK collision detection epsilon (minimum distance)
    constexpr float GJKEpsilon = 1e-6f;

    /// Maximum iterations for EPA (Expanding Polytope Algorithm)
    constexpr int EPAMaxIterations = 100;

    /// Constraint solver tolerance (Newton-Raphson convergence)
    constexpr float ConstraintSolverTolerance = 1e-4f;

    /// Maximum constraint solver iterations per frame
    constexpr int ConstraintSolverMaxIterations = 20;
}

/// @}

// ============================================================
//  THERMAL SIMULATION PARAMETERS
// ============================================================

/// @defgroup ThermalConfig Thermal Simulation Configuration
/// @{

namespace Thermal {
    /// Ambient temperature (room temperature) in Kelvin
    constexpr float AmbientTemperatureK = 293.15f;  // ~20°C

    /// Maximum safe operating temperature in Kelvin
    constexpr float MaxOperatingTemperatureK = 373.15f;  // ~100°C

    /// Heat dissipation rate (W/K)
    /// Higher values = faster cooling
    constexpr float HeatDissipationRate = 0.1f;

    /// Thermal time constant (seconds)
    /// Lower = faster response to heat input
    constexpr float ThermalTimeConstant = 10.0f;

    /// Stefan-Boltzmann constant (W/(m²·K⁴))
    /// Used for radiation heat transfer
    constexpr float StefanBoltzmannConstant = 5.67e-8f;

    /// Emissivity of typical steel (dimensionless, 0-1)
    constexpr float DefaultEmissivity = 0.85f;
}

/// @}

// ============================================================
//  GRAPHICS RENDERING PARAMETERS
// ============================================================

/// @defgroup GraphicsConfig Graphics Rendering Configuration
/// @{

namespace Graphics {
    /// Default window width (pixels)
    constexpr int DefaultWindowWidth = 1280;

    /// Default window height (pixels)
    constexpr int DefaultWindowHeight = 720;

    /// Default field of view (degrees)
    constexpr float DefaultFOV = 45.0f;

    /// Near clipping plane distance
    constexpr float NearClip = 0.01f;

    /// Far clipping plane distance
    constexpr float FarClip = 10000.0f;

    /// Maximum simultaneous point lights
    constexpr int MaxPointLights = 32;

    /// Maximum simultaneous directional lights
    constexpr int MaxDirectionalLights = 4;

    /// Shadow map resolution (pixels)
    constexpr int ShadowMapResolution = 2048;

    /// Ambient occlusion radius (meters)
    constexpr float AO_Radius = 0.5f;

    /// Anti-aliasing sample count (MSAA)
    constexpr int AAMultiplier = 4;

    /// Default clear color (RGBA, 0-1)
    constexpr float ClearColorR = 0.1f;
    constexpr float ClearColorG = 0.1f;
    constexpr float ClearColorB = 0.1f;
    constexpr float ClearColorA = 1.0f;

    /// PI constant (used in shaders and transforms)
    constexpr float Pi = 3.14159265358979323846f;

    /// Maximum texture resolution supported
    constexpr int MaxTextureResolution = 4096;

    /// Startup splash screen duration (seconds)
    constexpr double StartupSplashDuration = 5.5;
}

/// @}

// ============================================================
//  UI/UX PARAMETERS
// ============================================================

/// @defgroup UIConfig User Interface Configuration
/// @{

namespace UI {
    /// ImGui default font size (pixels)
    constexpr float FontSize = 16.0f;

    /// Panel header height
    constexpr float PanelHeaderHeight = 24.0f;

    /// Standard button height
    constexpr float ButtonHeight = 32.0f;

    /// Standard button width
    constexpr float ButtonWidth = 80.0f;

    /// Default padding/spacing (pixels)
    constexpr float Padding = 4.0f;

    /// Standard margin between UI elements
    constexpr float Margin = 8.0f;

    /// Toolbar height
    constexpr float ToolbarHeight = 40.0f;

    /// Status bar height
    constexpr float StatusBarHeight = 24.0f;

    /// Gizmo handle size (screen pixels)
    constexpr float GizmoHandleSize = 10.0f;

    /// Gizmo axis length
    constexpr float GizmoAxisLength = 1.0f;

    /// Property panel width (when docked)
    constexpr float PropertyPanelDefaultWidth = 300.0f;

    /// Console panel default height
    constexpr float ConsolePanelDefaultHeight = 200.0f;

    /// Maximum log messages in console view
    constexpr int ConsoleMaxMessages = 10000;

    /// Default frame rate for playback
    constexpr float DefaultPlaybackFrameRate = 60.0f;

    /// Animation lerp speed (0-1, higher = faster)
    constexpr float AnimationLerpSpeed = 0.1f;
}

/// @}

// ============================================================
//  MATERIAL DATABASE DEFAULTS
// ============================================================

/// @defgroup MaterialConfig Material Database Configuration
/// @{

namespace Materials {
    /// Default Young's Modulus for unknown materials (GPa)
    constexpr double DefaultYoungsModulus = 200.0;  // Typical steel

    /// Default Poisson's ratio for unknown materials
    constexpr double DefaultPoissonsRatio = 0.3;    // Typical for metals

    /// Default density for unknown materials (kg/m³)
    constexpr double DefaultDensity = 7850.0;       // Structural steel

    /// Default yield strength (MPa)
    constexpr double DefaultYieldStrength = 250.0;  // Mild steel

    /// Default ultimate tensile strength (MPa)
    constexpr double DefaultUltimateStrength = 400.0;

    /// Default thermal conductivity (W/(m·K))
    constexpr double DefaultThermalConductivity = 50.0;

    /// Default thermal expansion coefficient (1/K)
    constexpr double DefaultThermalExpansion = 1.2e-5;
}

/// @}

// ============================================================
//  LOGGING CONFIGURATION
// ============================================================

/// @defgroup LoggingConfig Logging Configuration
/// @{

namespace Logging {
    /// Default minimum log level
    /// 0=DEBUG, 1=INFO, 2=WARNING, 3=ERROR, 4=CRITICAL
    constexpr int DefaultLogLevel = 1;  // INFO

    /// Default log file path (relative to executable)
    constexpr const char* DefaultLogFile = "logs/szm_forge.log";

    /// Enable console output by default
    constexpr bool DefaultConsoleOutput = true;

    /// Enable file output by default
    constexpr bool DefaultFileOutput = false;

    /// Enable color codes in console output
    constexpr bool DefaultColorOutput = true;

    /// Maximum log file size before rotation (bytes)
    constexpr size_t MaxLogFileSize = 10 * 1024 * 1024;  // 10 MB
}

/// @}

// ============================================================
//  SIMULATION ENGINE PARAMETERS
// ============================================================

/// @defgroup SimulationConfig Simulation Engine Configuration
/// @{

namespace Simulation {
    /// Maximum number of components per simulation
    constexpr int MaxComponents = 1000;

    /// Stress ratio threshold for danger state
    constexpr float DangerStressThreshold = 0.8f;

    /// Temperature ratio threshold for danger state
    constexpr float DangerTempThreshold = 0.8f;

    /// Component update frequency (Hz)
    constexpr float UpdateFrequency = 60.0f;

    /// Default component area (m²)
    constexpr float DefaultComponentArea = 0.01f;

    /// Default component thickness (m)
    constexpr float DefaultComponentThickness = 0.01f;
}

/// @}

// ============================================================
//  VALIDATION RANGES
// ============================================================

/// @defgroup ValidationConfig Validation & Error Thresholds
/// @{

namespace Validation {
    /// Minimum allowed density (kg/m³)
    constexpr float MinDensity = 0.1f;

    /// Maximum allowed density (kg/m³)
    constexpr float MaxDensity = 25000.0f;

    /// Minimum allowed Young's modulus (GPa)
    constexpr float MinYoungsModulus = 0.1f;

    /// Maximum allowed Young's modulus (GPa)
    constexpr float MaxYoungsModulus = 1000.0f;

    /// Valid Poisson's ratio range: (-1, 0.5)
    constexpr float MinPoissonsRatio = -0.99f;
    constexpr float MaxPoissonsRatio = 0.49f;

    /// Float comparison epsilon
    constexpr float FloatEpsilon = 1e-6f;

    /// Double comparison epsilon
    constexpr double DoubleEpsilon = 1e-12;
}

/// @}

} // namespace SZM::Config
