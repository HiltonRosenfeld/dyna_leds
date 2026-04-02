# Dynamic LED Strip Control - Implementation Plan

## Overview
This plan details the implementation of a motion-reactive LED strip system using an ESP32, HLK-LD2450 sensor, and WS2812B LED strips.

## 1. Project Structure

```
dyna-leds/
├── src/
│   └── dyna_leds.ino          # Main Arduino sketch
├── include/
│   └── config.h               # Configuration header file
├── scripts/
│   ├── setup.sh               # Project setup and library installation
│   ├── validate_config.sh     # Configuration validation
│   └── prepare_build.sh       # Build preparation
├── docs/
│   └── WIRING.md              # Hardware wiring guide
├── README.md                  # Project documentation
├── TODO.md                    # Implementation checklist
└── .gitignore                 # Git ignore file
```

## 2. Configuration File Format

**File:** `include/config.h`

The configuration will be a C++ header file that can be directly included in the Arduino sketch. This approach:
- Eliminates runtime parsing overhead
- Provides compile-time validation
- Allows easy customization without modifying core code
- Works seamlessly with Arduino IDE

**Configuration Parameters:**

```cpp
// Hardware Pin Configuration
#define UART_TX_PIN 17        // ESP32 TX pin for sensor
#define UART_RX_PIN 16        // ESP32 RX pin for sensor
#define LED_DATA_PIN 5        // Data pin for LED strip

// LED Strip Configuration
#define NUM_LEDS 70           // Number of LEDs in strip
#define LED_TYPE WS2812B      // LED strip type
#define COLOR_ORDER GRB       // Color order for LED strip
#define BRIGHTNESS 128        // Global brightness (0-255)

// Sensor Configuration
#define SENSOR_BAUD_RATE 256000  // HLK-LD2450 baud rate

// Distance-to-LED Mapping
#define MIN_DISTANCE 0.0      // Minimum distance in meters
#define MAX_DISTANCE 6.0      // Maximum distance in meters
#define MIN_LEDS 5            // Minimum LEDs lit (at max distance)
#define MAX_LEDS 70           // Maximum LEDs lit (at min distance)

// Angle-to-Hue Mapping
#define MIN_ANGLE -60.0       // Minimum angle in degrees
#define MAX_ANGLE 60.0        // Maximum angle in degrees
#define MIN_HUE 0             // Hue at min angle (0-255)
#define MAX_HUE 255           // Hue at max angle (0-255)

// Performance Settings
#define UPDATE_INTERVAL_MS 50 // LED update interval (20 FPS)
#define SATURATION 255        // HSV saturation (0-255)
#define VALUE 255             // HSV value/brightness (0-255)
```

## 3. Core Arduino Sketch Architecture

**File:** `src/dyna_leds.ino`

### Architecture Components:

```
┌─────────────────────────────────────┐
│         Main Loop (50ms)            │
├─────────────────────────────────────┤
│  1. Read Sensor Data                │
│     └─> HLK-LD245X::update()        │
│                                     │
│  2. Process Target Data             │
│     ├─> Get closest target          │
│     ├─> Extract distance & angle    │
│     └─> Validate data               │
│                                     │
│  3. Map to LED Parameters           │
│     ├─> Distance → LED Count        │
│     └─> Angle → Hue Value           │
│                                     │
│  4. Update LED Strip                │
│     ├─> Set active LEDs (HSV)       │
│     ├─> Clear inactive LEDs         │
│     └─> FastLED.show()              │
└─────────────────────────────────────┘
```

### Key Functions:

1. **`setup()`**
   - Initialize Serial for debugging
   - Initialize UART for sensor communication
   - Initialize HLK-LD2450 sensor
   - Initialize FastLED library
   - Set initial LED state

2. **`loop()`**
   - Update sensor readings
   - Process target data
   - Map sensor values to LED parameters
   - Update LED strip
   - Implement timing control

3. **`mapDistanceToLEDCount(float distance)`**
   - Input: Distance in meters (0.0 - 6.0)
   - Output: Number of LEDs to light (5 - 70)
   - Logic: Inverse linear mapping (closer = fewer LEDs)
   - Formula: `LEDs = MAX_LEDS - ((distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE)) * (MAX_LEDS - MIN_LEDS)`

4. **`mapAngleToHue(float angle)`**
   - Input: Angle in degrees (-60 to +60)
   - Output: Hue value (0 - 255)
   - Logic: Linear mapping across hue spectrum
   - Formula: `Hue = MIN_HUE + ((angle - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE)) * (MAX_HUE - MIN_HUE)`

5. **`updateLEDs(int ledCount, uint8_t hue)`**
   - Set first `ledCount` LEDs to HSV color
   - Clear remaining LEDs
   - Call `FastLED.show()`

## 4. Sensor Integration Approach

### HLK-LD2450 Integration:

```cpp
#include <HLK_LD245X.h>

// Create sensor instance
HLK_LD245X sensor;

// In setup():
Serial2.begin(SENSOR_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
sensor.begin(&Serial2);

// In loop():
sensor.update();
if (sensor.hasTargets()) {
    Target* target = sensor.getClosestTarget();
    float distance = target->d;  // Distance in mm, convert to meters
    float angle = target->angle; // Angle in degrees
}
```

### Target Selection Strategy:
- Use closest target for LED control
- Fallback to default state if no targets detected
- Implement smoothing to prevent flickering (optional enhancement)

## 5. LED Control Logic Implementation

### FastLED Integration:

```cpp
#include <FastLED.h>

CRGB leds[NUM_LEDS];

// In setup():
FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
FastLED.setBrightness(BRIGHTNESS);

// In loop():
void updateLEDs(int ledCount, uint8_t hue) {
    // Set active LEDs
    for (int i = 0; i < ledCount && i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue, SATURATION, VALUE);
    }
    
    // Clear inactive LEDs
    for (int i = ledCount; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }
    
    FastLED.show();
}
```

### Mapping Functions:

```cpp
int mapDistanceToLEDCount(float distanceMM) {
    // Convert mm to meters
    float distance = distanceMM / 1000.0;
    
    // Clamp to valid range
    distance = constrain(distance, MIN_DISTANCE, MAX_DISTANCE);
    
    // Inverse mapping: closer = fewer LEDs
    float ratio = (distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE);
    int ledCount = MAX_LEDS - (int)(ratio * (MAX_LEDS - MIN_LEDS));
    
    return constrain(ledCount, MIN_LEDS, MAX_LEDS);
}

uint8_t mapAngleToHue(float angle) {
    // Clamp to valid range
    angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);
    
    // Linear mapping to hue spectrum
    float ratio = (angle - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE);
    uint8_t hue = MIN_HUE + (uint8_t)(ratio * (MAX_HUE - MIN_HUE));
    
    return hue;
}
```

## 6. Bash Automation Scripts

### 6.1 Setup Script (`scripts/setup.sh`)

**Purpose:** Install Arduino CLI and required libraries

**Features:**
- Detect macOS environment
- Install Arduino CLI via Homebrew (if not present)
- Install ESP32 board support
- Install FastLED library
- Install HLK-LD245X library
- Create project directories

**Usage:** `./scripts/setup.sh`

### 6.2 Configuration Validation (`scripts/validate_config.sh`)

**Purpose:** Validate configuration parameters

**Checks:**
- Pin assignments don't conflict
- LED count is positive
- Distance ranges are valid
- Angle ranges are valid
- Hue ranges are valid (0-255)
- Brightness is valid (0-255)

**Usage:** `./scripts/validate_config.sh`

### 6.3 Build Preparation (`scripts/prepare_build.sh`)

**Purpose:** Prepare project for Arduino IDE compilation

**Features:**
- Verify all required files exist
- Check library dependencies
- Validate configuration
- Generate build report
- Copy files to Arduino sketchbook (optional)

**Usage:** `./scripts/prepare_build.sh`

## 7. Compilation and Upload Workflow

### Method 1: Arduino IDE (Recommended for beginners)

1. **Open Project:**
   ```bash
   open src/dyna_leds.ino
   ```

2. **Configure Arduino IDE:**
   - Board: "ESP32 Dev Module"
   - Upload Speed: 921600
   - Flash Frequency: 80MHz
   - Port: Select your ESP32 port

3. **Install Libraries:**
   - Sketch → Include Library → Manage Libraries
   - Search and install: "FastLED"
   - Search and install: "HLK-LD245X"

4. **Compile and Upload:**
   - Click "Verify" to compile
   - Click "Upload" to flash ESP32

### Method 2: Arduino CLI (Advanced)

1. **Compile:**
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32 src/dyna_leds.ino
   ```

2. **Upload:**
   ```bash
   arduino-cli upload -p /dev/cu.usbserial-* --fqbn esp32:esp32:esp32 src/dyna_leds.ino
   ```

### Method 3: PlatformIO (Alternative)

Create `platformio.ini`:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    fastled/FastLED@^3.6.0
    HLK-LD245X
monitor_speed = 115200
```

## 8. Configuration Workflow

### Initial Setup:
1. Run `./scripts/setup.sh` to install dependencies
2. Edit `include/config.h` with your GPIO pins
3. Adjust LED count and mapping parameters
4. Run `./scripts/validate_config.sh` to verify settings
5. Open `src/dyna_leds.ino` in Arduino IDE
6. Compile and upload to ESP32

### Customization:
- **Change LED count:** Modify `NUM_LEDS` in `config.h`
- **Adjust sensitivity:** Modify distance/angle ranges
- **Change colors:** Adjust `MIN_HUE` and `MAX_HUE`
- **Adjust brightness:** Modify `BRIGHTNESS` value
- **Change update rate:** Modify `UPDATE_INTERVAL_MS`

## 9. Testing Strategy (Minimal)

### Pre-Upload Verification:
1. Configuration validation (automated via script)
2. Compilation check (Arduino IDE verify)

### Post-Upload Testing:
1. **Serial Monitor Test:**
   - Open Serial Monitor (115200 baud)
   - Verify sensor data output
   - Check for initialization messages

2. **LED Functionality Test:**
   - Wave hand in front of sensor
   - Verify LEDs respond to distance
   - Verify color changes with angle

3. **Range Test:**
   - Test at minimum distance (close)
   - Test at maximum distance (6m)
   - Verify LED count changes appropriately

## 10. Error Handling

### Sensor Errors:
- No targets detected → Default state (all LEDs off or preset color)
- Invalid data → Use last valid reading
- Sensor initialization failure → Serial error message

### LED Errors:
- FastLED initialization → Serial error message
- Invalid LED count → Clamp to valid range

### Configuration Errors:
- Caught at compile time (invalid constants)
- Validation script catches logical errors

## 11. Future Enhancements (Optional)

- Smoothing/filtering for stable LED transitions
- Multiple target tracking
- Configurable LED patterns
- Web interface for configuration
- MQTT integration for remote control
- Data logging to SD card

## 12. Dependencies Summary

### Hardware:
- ESP32 development board
- HLK-LD2450 sensor
- WS2812B LED strip (70 LEDs)
- 5V power supply (adequate for LED strip)
- Jumper wires

### Software:
- Arduino IDE 2.x or Arduino CLI
- ESP32 board support package
- FastLED library (v3.6.0+)
- HLK-LD245X library
- Bash (macOS built-in)

### Development Tools:
- Homebrew (for Arduino CLI installation)
- Git (for version control)
- Text editor (for configuration)

---

## Next Steps

Once you approve this plan, I recommend switching to **Code mode** to implement:
1. Configuration header file
2. Main Arduino sketch
3. Bash automation scripts
4. Documentation files

Would you like me to proceed with implementation, or would you like to discuss any modifications to this plan?