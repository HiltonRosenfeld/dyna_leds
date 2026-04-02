# DynaLEDs Web Configuration Guide

## Overview

Your DynaLEDs system now includes a powerful web-based configuration interface that allows you to adjust all parameters without recompiling or re-uploading code. This guide will help you get started.

## Features

✨ **Web-Based Configuration** - Configure all 20+ parameters via browser
🔄 **Persistent Storage** - Settings saved to non-volatile memory
📱 **Mobile Responsive** - Works on phones, tablets, and computers
🌐 **WiFi Connectivity** - Station mode or Access Point mode
🔧 **OTA Updates** - Upload new firmware wirelessly
🎨 **Live Preview** - See color changes in real-time
💾 **Export/Import** - Backup and restore configurations

---

## Quick Start

### 1. Install Required Libraries

Open Arduino IDE and install these libraries via **Sketch → Include Library → Manage Libraries**:

1. **ESPAsyncWebServer** by me-no-dev
2. **AsyncTCP** by me-no-dev
3. **ArduinoJson** by Benoit Blanchon (v6.x or later)

### 2. Upload the Code

1. Open `dyna_leds.ino` in Arduino IDE
2. Select your ESP32 board: **Tools → Board → ESP32 Dev Module**
3. Select the correct COM port: **Tools → Port**
4. Click **Upload** button

### 3. First-Time Setup

On first boot, the device creates a WiFi Access Point:

- **SSID:** `DynaLEDs-Setup`
- **Password:** `configure`

**Steps:**
1. Connect your phone/computer to the `DynaLEDs-Setup` WiFi network
2. Open a web browser and navigate to: `http://192.168.4.1`
3. You'll see the DynaLEDs configuration interface
4. Scroll to the **WiFi Configuration** section
5. Enter your home WiFi credentials
6. Click **Save Configuration**
7. Click **Restart Device**

### 4. Normal Operation

After restart, the device will connect to your WiFi network. Access the web interface at:

- **By mDNS:** `http://dyna-leds.local` (recommended)
- **By IP:** Check your router or Serial Monitor for the IP address

---

## Web Interface Guide

### 🎨 Main Interface Sections

#### 1. Hardware Pins
Configure GPIO pins for sensor and LED connections:
- **UART RX Pin** - ESP32 receives data from sensor (default: 16)
- **UART TX Pin** - ESP32 sends data to sensor (default: 17)
- **LED Data Pin** - WS2812B data line (default: 21)

⚠️ **Note:** Pin changes require device restart to take effect.

#### 2. LED Configuration
- **Number of LEDs** - Total LEDs in your strip (1-1000)
- **Brightness** - Global brightness level (0-255)
  - 0 = Off
  - 128 = 50% brightness (recommended for indoor use)
  - 255 = Maximum brightness

#### 3. Distance Mapping
Controls how distance affects LED count:
- **Min Distance** - Closest detection distance in meters
- **Max Distance** - Farthest detection distance in meters
- **Min LEDs** - LEDs lit when target is far away
- **Max LEDs** - LEDs lit when target is close

**Example:** With inverse mapping, a person close to the sensor lights fewer LEDs (spotlight effect).

#### 4. Angle Mapping
Controls how angle affects LED color:
- **Min Angle** - Left side angle in degrees (typically -60°)
- **Max Angle** - Right side angle in degrees (typically +60°)
- **Min Hue** - Color at minimum angle (0-255)
- **Max Hue** - Color at maximum angle (0-255)

**Hue Reference:**
- 0 = Red
- 32 = Orange
- 64 = Yellow
- 96 = Green
- 128 = Cyan
- 160 = Blue
- 192 = Purple
- 224 = Magenta
- 255 = Red (wraps around)

#### 5. Color Settings
- **Saturation** - Color intensity (0-255)
  - 0 = White/grayscale
  - 255 = Fully saturated colors
- **Value** - Brightness of the color (0-255)
  - Independent of global brightness
  - 255 = Maximum intensity

**Color Preview:** Shows a live preview of the color based on Min Hue, Saturation, and Value.

#### 6. Performance
- **Update Interval** - Time between LED updates in milliseconds (10-1000ms)
  - Lower = Faster response, higher CPU usage
  - 50ms = 20 FPS (recommended)
  - Higher = Slower response, lower CPU usage
- **Debug Output** - Enable/disable Serial Monitor debug messages

#### 7. Default Behavior (No Target)
What happens when no motion is detected:
- **All Off** - Turn off all LEDs
- **All On** - Keep all LEDs on with default color
- **Last State** - Maintain the last known state

**Default LED Count** - Number of LEDs for "All On" mode
**Default Hue** - Color for "All On" mode

#### 8. WiFi Configuration
- **WiFi SSID** - Your home WiFi network name
- **WiFi Password** - Your home WiFi password
- **AP SSID** - Access Point name (for fallback mode)
- **AP Password** - Access Point password (minimum 8 characters)

---

## Configuration Tips

### 💡 Best Practices

1. **Start with Defaults**
   - The system comes with sensible defaults
   - Make small adjustments and test
   - Use "Restore Defaults" if things go wrong

2. **Test Changes Incrementally**
   - Change one parameter at a time
   - Save and observe the effect
   - Adjust as needed

3. **Brightness Management**
   - Start with lower brightness (64-128)
   - High brightness increases power consumption and heat
   - Adjust based on ambient lighting

4. **Update Interval**
   - 50ms provides smooth motion tracking
   - Increase if you experience WiFi issues
   - Decrease for faster response (at cost of CPU usage)

5. **Distance Mapping**
   - Adjust based on your room size
   - Sensor can detect up to 6 meters
   - Set Max Distance to your room's longest dimension

### 🎯 Common Configurations

#### Configuration 1: Full Rainbow Spectrum
```
Min Angle: -60°
Max Angle: 60°
Min Hue: 0
Max Hue: 255
Saturation: 255
Value: 255
```

#### Configuration 2: Warm Colors Only
```
Min Angle: -60°
Max Angle: 60°
Min Hue: 0 (Red)
Max Hue: 64 (Yellow)
Saturation: 255
Value: 255
```

#### Configuration 3: Cool Colors Only
```
Min Angle: -60°
Max Angle: 60°
Min Hue: 128 (Cyan)
Max Hue: 192 (Blue)
Saturation: 255
Value: 255
```

#### Configuration 4: Subtle White Glow
```
Saturation: 0
Value: 128
Brightness: 64
```

---

## Advanced Features

### 🔄 OTA (Over-The-Air) Updates

Update firmware without USB cable:

1. Ensure device is connected to WiFi
2. In Arduino IDE, go to **Tools → Port**
3. Select **Network Port: dyna-leds at [IP address]**
4. Enter password: `update`
5. Click **Upload**
6. LEDs will show blue progress bar during update

**Visual Feedback:**
- Blue LEDs = Update in progress
- Red LEDs = Update error
- Normal operation resumes after successful update

### 📊 System Status

Access system information at: `http://dyna-leds.local/status`

Returns JSON with:
- Uptime in seconds
- Free heap memory
- WiFi connection status
- IP address

### 🔧 Factory Reset

**Method 1: Via Web Interface**
1. Click "Restore Defaults" button
2. Confirm the action
3. Click "Restart Device"

**Method 2: Via Serial Monitor**
1. Connect via USB
2. Open Serial Monitor (115200 baud)
3. Device will show current configuration on boot

**Method 3: Clear Preferences (Emergency)**
If web interface is inaccessible, add this to `setup()` temporarily:
```cpp
clearConfiguration();
```
Upload, then remove the line and upload again.

---

## Troubleshooting

### Problem: Can't Connect to Web Interface

**Solution 1: Check WiFi Connection**
- Open Serial Monitor (115200 baud)
- Look for IP address on boot
- Verify device connected to correct network

**Solution 2: Use AP Mode**
- Device falls back to AP mode if WiFi fails
- Connect to `DynaLEDs-Setup` network
- Access at `http://192.168.4.1`

**Solution 3: Use mDNS**
- Try `http://dyna-leds.local`
- Works on most devices (may not work on Android)

### Problem: Configuration Not Saving

**Check:**
- Validation errors in web interface
- Serial Monitor for error messages
- Ensure values are within valid ranges

**Solution:**
- Verify all required fields are filled
- Check that Min values < Max values
- Ensure AP password is at least 8 characters

### Problem: LEDs Not Responding

**Check:**
1. LED Data Pin matches your wiring
2. Number of LEDs matches your strip
3. Power supply is adequate
4. Sensor is connected and working

**Debug:**
- Enable Debug Output in web interface
- Check Serial Monitor for sensor data
- Verify sensor is detecting targets

### Problem: WiFi Keeps Disconnecting

**Solutions:**
- Increase Update Interval (reduces CPU load)
- Move device closer to router
- Check for WiFi interference
- Verify WiFi credentials are correct

### Problem: OTA Update Fails

**Solutions:**
- Ensure device is on same network as computer
- Check that password is correct: `update`
- Verify sufficient free memory (check /status)
- Try uploading via USB if OTA fails

---

## API Reference

For advanced users who want to integrate with other systems:

### GET /config
Returns current configuration as JSON.

**Example Response:**
```json
{
  "hardware": {
    "uartRxPin": 16,
    "uartTxPin": 17,
    "ledDataPin": 21
  },
  "leds": {
    "numLeds": 70,
    "brightness": 128
  },
  ...
}
```

### POST /config
Save new configuration. Send JSON body with configuration.

**Example Request:**
```bash
curl -X POST http://dyna-leds.local/config \
  -H "Content-Type: application/json" \
  -d '{"leds":{"brightness":200}}'
```

### POST /reset
Reset configuration to factory defaults.

### POST /restart
Restart the ESP32 device.

### GET /status
Get system status information.

---

## Security Notes

### Default Credentials
- **AP Password:** `configure`
- **OTA Password:** `update`

### Recommendations
1. Change AP password via web interface
2. Keep device on secure network
3. Consider adding web authentication (Phase 7 feature)
4. Don't expose device directly to internet

---

## Support & Resources

### Documentation
- `IMPLEMENTATION_PLAN.md` - Technical implementation details
- `config.h` - Default configuration values
- `config_manager.h` - Configuration management code
- `web_server.h` - Web server implementation

### Serial Monitor
Connect via USB and open Serial Monitor (115200 baud) to see:
- Boot messages
- Configuration loading status
- WiFi connection details
- IP address
- Sensor data (if debug enabled)
- Error messages

### Getting Help
1. Check Serial Monitor for error messages
2. Review this guide's Troubleshooting section
3. Verify all libraries are installed correctly
4. Check that ESP32 board package is up to date

---

## Changelog

### Version 1.0 (2026-04-02)
- ✅ Initial web configuration implementation
- ✅ All 20+ parameters configurable
- ✅ Persistent storage via Preferences
- ✅ Dual-mode WiFi (Station + AP)
- ✅ mDNS support (dyna-leds.local)
- ✅ OTA firmware updates
- ✅ Responsive web interface
- ✅ Real-time color preview
- ✅ Configuration validation
- ✅ System status API

---

**Enjoy your web-configurable DynaLEDs system! 🎨✨**