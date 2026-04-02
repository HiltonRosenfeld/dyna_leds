# Hardware Wiring Guide

This guide provides detailed instructions for connecting the HLK-LD2450 sensor and WS2812B LED strip to your ESP32 development board.

## Components Required

- **ESP32 Development Board** (any variant with UART and GPIO)
- **HLK-LD2450 Sensor** (24GHz mmWave radar sensor)
- **WS2812B LED Strip** (70 LEDs or configurable count)
- **5V Power Supply** (adequate amperage for LED strip)
- **Jumper Wires** (male-to-female recommended)
- **Breadboard** (optional, for prototyping)

## Power Requirements

### LED Strip Power Calculation
Each WS2812B LED can draw up to 60mA at full brightness (white color):
- 70 LEDs × 60mA = 4.2A maximum
- Typical usage (colored, medium brightness): ~2-3A

**Recommended Power Supply:**
- 5V, 5A power supply for 70 LEDs
- Use a dedicated power supply for the LED strip (not USB power)

### ESP32 Power
- Can be powered via USB (500mA-1A)
- Or from the same 5V supply as LEDs (using voltage regulator if needed)

## Pin Connections

### Default Pin Configuration
These are the default pins defined in `include/config.h`. You can change them by editing the configuration file.

```
ESP32 Pin    →    Component
─────────────────────────────
GPIO 17      →    HLK-LD2450 RX
GPIO 16      →    HLK-LD2450 TX
GPIO 5       →    WS2812B Data In
GND          →    Common Ground
5V/VIN       →    Power (if needed)
```

## Detailed Wiring Diagrams

### 1. HLK-LD2450 Sensor Connection

```
┌─────────────────────┐
│   HLK-LD2450        │
│                     │
│  VCC  ──────────────┼──→ 5V (or 3.3V)
│  GND  ──────────────┼──→ GND
│  TX   ──────────────┼──→ ESP32 GPIO 16 (RX)
│  RX   ──────────────┼──→ ESP32 GPIO 17 (TX)
│                     │
└─────────────────────┘

Note: TX on sensor connects to RX on ESP32, and vice versa
```

**Important Notes:**
- The HLK-LD2450 can operate on 5V or 3.3V (check your module's specifications)
- TX/RX pins are crossed: sensor TX → ESP32 RX, sensor RX → ESP32 TX
- Ensure proper voltage levels for UART communication

### 2. WS2812B LED Strip Connection

```
┌─────────────────────┐
│  WS2812B LED Strip  │
│                     │
│  5V   ──────────────┼──→ 5V Power Supply (+)
│  GND  ──────────────┼──→ Power Supply (-) AND ESP32 GND
│  DIN  ──────────────┼──→ ESP32 GPIO 5
│                     │
└─────────────────────┘

Power Supply (5V, 5A)
  (+) ──→ LED Strip 5V
  (-) ──→ LED Strip GND AND ESP32 GND
```

**Important Notes:**
- **Common Ground:** ESP32 GND and LED strip GND must be connected together
- **Separate Power:** Use a dedicated 5V power supply for the LED strip
- **Data Pin:** Only one wire needed for data (GPIO 5 → DIN)
- **Direction:** Ensure you connect to the input end of the LED strip (marked with arrows)

### 3. Complete System Wiring

```
                    ┌──────────────────┐
                    │   5V Power       │
                    │   Supply         │
                    │   (5A)           │
                    └────┬─────┬───────┘
                         │     │
                        5V    GND
                         │     │
    ┌────────────────────┼─────┼────────────────────┐
    │                    │     │                    │
    │  ┌─────────────────┴─────┴──────────┐        │
    │  │         WS2812B LED Strip        │        │
    │  │  (70 LEDs)                       │        │
    │  └──────────┬───────────────────────┘        │
    │             │ DIN                             │
    │             │                                 │
    │  ┌──────────┴──────────────────────┐         │
    │  │       ESP32 Dev Board           │         │
    │  │                                 │         │
    │  │  GPIO 5  ←──────────────────────┘         │
    │  │  GPIO 17 ──────────┐                      │
    │  │  GPIO 16 ←─────────┼──┐                   │
    │  │  GND ──────────────┼──┼───────────────────┘
    │  │  5V (optional) ────┘  │
    │  └───────────────────────┼───────────┐
    │                          │           │
    │  ┌───────────────────────┴───────────┴──┐
    │  │      HLK-LD2450 Sensor              │
    │  │                                     │
    │  │  RX  TX  GND  VCC                  │
    │  └─────────────────────────────────────┘
    │
    └─ Common Ground Connection
```

## Step-by-Step Wiring Instructions

### Step 1: Prepare Components
1. Gather all components and tools
2. Identify the input end of the LED strip (look for arrows)
3. Note the pin labels on your ESP32 board

### Step 2: Connect Power Supply
1. **DO NOT** connect power yet
2. Identify the positive (+) and negative (-) terminals
3. Prepare connections but leave unplugged

### Step 3: Connect LED Strip
1. Connect LED strip **GND** to power supply negative (-)
2. Connect LED strip **5V** to power supply positive (+)
3. Connect LED strip **GND** to ESP32 **GND** (common ground)
4. Connect LED strip **DIN** to ESP32 **GPIO 5** (or your configured pin)

### Step 4: Connect HLK-LD2450 Sensor
1. Connect sensor **GND** to ESP32 **GND**
2. Connect sensor **VCC** to ESP32 **5V** or **3.3V** (check sensor specs)
3. Connect sensor **TX** to ESP32 **GPIO 16** (RX pin)
4. Connect sensor **RX** to ESP32 **GPIO 17** (TX pin)

### Step 5: Connect ESP32
1. Connect ESP32 to computer via USB (for programming and debugging)
2. Or connect ESP32 **VIN** to 5V power supply (if not using USB)

### Step 6: Verify Connections
Before applying power, double-check:
- [ ] All GND connections are common
- [ ] LED strip has dedicated 5V power supply
- [ ] Sensor TX → ESP32 RX (pins are crossed)
- [ ] Sensor RX → ESP32 TX (pins are crossed)
- [ ] LED data pin matches configuration
- [ ] No short circuits

### Step 7: Power On
1. Connect USB to ESP32 (if using USB power)
2. Connect 5V power supply to LED strip
3. Observe initial LED state
4. Check Serial Monitor for debug output

## Troubleshooting

### LEDs Don't Light Up
- **Check power supply:** Ensure 5V supply is connected and adequate amperage
- **Check data pin:** Verify GPIO 5 (or configured pin) is connected to DIN
- **Check LED direction:** Ensure connected to input end (arrows point away)
- **Check common ground:** ESP32 GND must connect to LED strip GND
- **Check configuration:** Verify `LED_DATA_PIN` in `config.h`

### Sensor Not Responding
- **Check UART pins:** Verify TX/RX are crossed correctly
- **Check baud rate:** Should be 256000 (default for HLK-LD2450)
- **Check power:** Ensure sensor has adequate power (5V or 3.3V)
- **Check Serial Monitor:** Look for sensor initialization messages
- **Swap TX/RX:** Try swapping if still not working

### Flickering or Unstable LEDs
- **Add capacitor:** 1000µF capacitor across LED strip power (+ to 5V, - to GND)
- **Add resistor:** 330Ω resistor between ESP32 data pin and LED DIN
- **Check power supply:** Ensure adequate current capacity
- **Shorten wires:** Use shorter wires for data connection
- **Check ground:** Ensure solid common ground connection

### ESP32 Won't Program
- **Check USB cable:** Use a data cable (not charge-only)
- **Hold BOOT button:** Press and hold during upload
- **Check port:** Verify correct port selected in Arduino IDE
- **Install drivers:** Install CP210x or CH340 drivers if needed

## Safety Considerations

⚠️ **Important Safety Notes:**

1. **Power Supply Rating:** Ensure your power supply can handle the LED strip current
2. **Heat Management:** High brightness generates heat; ensure adequate ventilation
3. **Wire Gauge:** Use appropriate wire gauge for current (18-22 AWG recommended)
4. **Polarity:** Double-check polarity before connecting power
5. **Fusing:** Consider adding a fuse to protect against short circuits
6. **Insulation:** Ensure all connections are properly insulated

## Advanced Wiring Options

### Option 1: Level Shifter for Data Line
For improved reliability, use a 3.3V to 5V level shifter:
```
ESP32 GPIO 5 → Level Shifter (3.3V side)
Level Shifter (5V side) → LED Strip DIN
```

### Option 2: Multiple LED Strips
To control multiple strips:
```
ESP32 GPIO 5  → Strip 1 DIN
ESP32 GPIO 18 → Strip 2 DIN
ESP32 GPIO 19 → Strip 3 DIN
(Update config.h accordingly)
```

### Option 3: External Power for ESP32
If not using USB:
```
5V Power Supply → Buck Converter (5V to 3.3V) → ESP32 3.3V pin
5V Power Supply → ESP32 VIN pin (if board has voltage regulator)
```

## Testing Checklist

After wiring, verify:
- [ ] All connections are secure
- [ ] No exposed wires or short circuits
- [ ] Power supply voltage is correct (5V)
- [ ] Common ground is established
- [ ] LED strip direction is correct
- [ ] Sensor TX/RX are crossed
- [ ] Configuration matches physical wiring

## Pin Reference Table

| Component | Pin Name | ESP32 Default | Configurable | Notes |
|-----------|----------|---------------|--------------|-------|
| Sensor | TX | GPIO 16 | Yes | Connects to ESP32 RX |
| Sensor | RX | GPIO 17 | Yes | Connects to ESP32 TX |
| Sensor | VCC | 5V/3.3V | - | Check sensor specs |
| Sensor | GND | GND | - | Common ground |
| LED Strip | DIN | GPIO 5 | Yes | Data input |
| LED Strip | 5V | 5V Supply | - | Dedicated power |
| LED Strip | GND | GND | - | Common ground |

## Additional Resources

- **ESP32 Pinout:** https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
- **WS2812B Datasheet:** Search for "WS2812B datasheet" online
- **HLK-LD2450 Manual:** Check manufacturer's documentation
- **FastLED Wiki:** https://github.com/FastLED/FastLED/wiki

---

**Need Help?**
- Check the main README.md for software setup
- Review IMPLEMENTATION_PLAN.md for system architecture
- Run `./scripts/validate_config.sh` to verify configuration