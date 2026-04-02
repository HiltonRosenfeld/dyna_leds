Planning prompt for Bob:

---

**Project: Dynamic LED Strip Control with Motion Sensing**

I want to build a system that dynamically controls LED strips based on people's movement detected by an HLK-LD2450 sensor, all connected to an ESP32 board.

**Hardware Setup:**
- ESP32 development board
- HLK-LD2450 sensor (UART connection)
- LED strip with 70 individually addressable LEDs (configurable count)

**Control Mapping:**
- `target.d` (distance) → Number of active LEDs (closer = fewer LEDs lit)
- `target.angle` → LED Hue value

**Technical Stack:**
- Arduino IDE for compilation and upload
- [FastLED library](https://github.com/FastLED/FastLED) for LED control
- [HLK-LD245X library](https://github.com/Fiooodooor/HLK-LD245X) for sensor communication
  - **Reference implementation**: [LD2450.ino example](https://github.com/Fiooodooor/HLK-LD245X/blob/main/examples/LD2450/LD2450.ino) demonstrates proper sensor initialization and configuration
- Development environment: macOS

**Required Deliverables:**
1. Configuration file defining:
   - UART GPIO pins (TX/RX for sensor)
   - LED strip data pin
   - LED count (default 70, configurable)
   - Distance-to-LED mapping parameters
   - Angle-to-Hue mapping parameters

2. Main Arduino sketch implementing:
   - Sensor initialization and UART communication
     - Follow the pattern from [LD2450.ino example](https://github.com/Fiooodooor/HLK-LD245X/blob/main/examples/LD2450/LD2450.ino)
     - Include configuration session in setup():
       - `beginConfigurationSession()`
       - `setMultiTargetTracking()` (optional)
       - `queryTargetTrackingMode()` (optional)
       - `queryFirmwareVersion()` (optional)
       - `queryMacAddress()` (optional)
       - `queryZoneFilter()` (optional)
       - `endConfigurationSession()`
   - LED strip initialization
   - Real-time target tracking
     - Use `update()` to read sensor data
     - Use `getNrValidTargets()` to check number of valid targets
     - Use `getTarget(index)` to iterate through targets
     - **Note**: The library does NOT provide `hasTargets()` or `getClosestTarget()` functions
     - To find the closest target, iterate through all targets and compare distances manually
   - Distance-to-LED-count mapping logic
   - Angle-to-Hue conversion
   - LED update loop

**Constraints:**
- Do not create any testing code or hardware setup
- Do not create deisgn documents or diagrams
- DO not create any bash automation scripts
- Keep code modular and well-documented
- Ensure easy configuration without code changes

**Implementation Plan Request:**
Please create a detailed step-by-step implementation plan that breaks down:
1. Project structure and file organization
2. Configuration file format and parameters
3. Core Arduino sketch architecture
4. Sensor integration approach
5. LED control logic implementation
6. Bash automation scripts needed
7. Compilation and upload workflow

Focus on a practical, working implementation with clear separation between configuration and code.