# Planning Prompt for Bob: ESP32 Web Configuration Server

## Project Context
The application [`dyna_leds.ino`](dyna_leds.ino) dynamically controls WS2812B LED strips based on motion detected by an HLK-LD2450 sensor connected to an ESP32. Currently, all configuration parameters are hardcoded in [`config.h`](config.h:1-211).

## Objective
Implement a web server on the ESP32 that allows runtime configuration of all parameters currently defined in [`config.h`](config.h:1-211) without requiring code recompilation or re-upload.

## Technical Requirements

### Core Libraries to Use
- **ESPAsyncWebServer**: Asynchronous web server for non-blocking HTTP handling
- **AsyncTCP**: Required dependency for ESPAsyncWebServer
- **LittleFS**: File system for storing HTML/CSS/JS files
- **Preferences**: Non-volatile storage for persisting configuration across reboots
- **ArduinoJson**: JSON parsing for configuration data

### Configuration Parameters to Expose
From [`config.h`](config.h:1-211), make the following configurable via web interface:

**Hardware Pins:**
- [`UART_RX_PIN`](config.h:21) (GPIO 16)
- [`UART_TX_PIN`](config.h:22) (GPIO 17)
- [`LED_DATA_PIN`](config.h:28) (GPIO 21)

**LED Configuration:**
- [`NUM_LEDS`](config.h:38) (70)
- [`BRIGHTNESS`](config.h:53) (0-255)

**Distance Mapping:**
- [`MIN_DISTANCE`](config.h:76) (0.0m)
- [`MAX_DISTANCE`](config.h:77) (6.0m)
- [`MIN_LEDS`](config.h:86) (5)
- [`MAX_LEDS`](config.h:87) (70)

**Angle Mapping:**
- [`MIN_ANGLE`](config.h:99) (-60°)
- [`MAX_ANGLE`](config.h:100) (60°)
- [`MIN_HUE`](config.h:111) (0)
- [`MAX_HUE`](config.h:112) (255)

**Color Settings:**
- [`SATURATION`](config.h:123) (0-255)
- [`VALUE`](config.h:130) (0-255)

**Performance:**
- [`UPDATE_INTERVAL_MS`](config.h:143) (50ms)
- [`DEBUG_ENABLED`](config.h:156) (boolean)

**Default Behavior:**
- Default behavior mode (ALL_OFF/ALL_ON/LAST_STATE)
- [`DEFAULT_LED_COUNT`](config.h:172)
- [`DEFAULT_HUE`](config.h:173)

**WiFi Configuration:**
- SSID (for Station mode)
- Password (for Station mode)
- AP SSID (for Access Point mode)
- AP Password (for Access Point mode)

## Implementation Plan

### Phase 1: Project Structure Setup
1. Create `data/` directory for LittleFS files
2. Create `data/index.html` - Main configuration web page
3. Create `data/style.css` - Styling for web interface
4. Create `data/script.js` - Client-side JavaScript for form handling
5. Create new file `web_server.h` - Web server initialization and route handlers
6. Create new file `config_manager.h` - Configuration loading/saving with Preferences library

### Phase 2: Configuration Management System
1. Implement configuration structure to hold all parameters
2. Create functions to load configuration from Preferences on boot
3. Create functions to save configuration to Preferences
4. Create functions to apply configuration to running system
5. Implement default configuration fallback
6. Add configuration validation logic

### Phase 3: Web Server Implementation
1. Initialize LittleFS in [`setup()`](dyna_leds.ino:54-76)
2. Initialize WiFi (AP mode for initial setup, Station mode for normal operation)
3. Initialize ESPAsyncWebServer
4. Implement HTTP GET route for serving configuration page
5. Implement HTTP GET route for retrieving current configuration as JSON
6. Implement HTTP POST route for receiving and saving new configuration
7. Implement HTTP POST route for WiFi configuration
8. Add route for system restart after configuration changes

### Phase 4: Web Interface Design
1. Create responsive HTML form with sections for each configuration category
2. Add input validation (min/max ranges, data types)
3. Implement real-time preview of color settings (hue visualization)
4. Add "Save Configuration" button with confirmation
5. Add "Restore Defaults" button
6. Add "Restart Device" button
7. Display current configuration values on page load
8. Show connection status and IP address

### Phase 5: Integration with Main Application
1. Modify [`dyna_leds.ino`](dyna_leds.ino:1-413) to load configuration from Preferences instead of [`config.h`](config.h:1-211)
2. Update [`setup()`](dyna_leds.ino:54-76) to initialize web server
3. Ensure web server runs asynchronously without blocking [`loop()`](dyna_leds.ino:82-100)
4. Add configuration reload capability without full restart
5. Maintain backward compatibility with [`config.h`](config.h:1-211) as defaults

### Phase 6: WiFi Provisioning
1. Implement dual-mode WiFi (AP + Station)
2. On first boot or failed connection, start in AP mode
3. Provide captive portal for initial WiFi setup
4. Store WiFi credentials in Preferences
5. Auto-connect to saved WiFi on subsequent boots
6. Add fallback to AP mode if connection fails

### Phase 7: Advanced Features
1. Add OTA (Over-The-Air) firmware update capability
2. Implement configuration export/import (JSON file download/upload)
3. Add real-time LED preview on web interface
4. Implement configuration presets/profiles
5. Add system information page (uptime, memory, sensor status)
6. Implement mDNS for easy access (e.g., http://dyna-leds.local)

## File Structure After Implementation
```
dyna_leds/
├── dyna_leds.ino          # Main application (modified)
├── config.h               # Default configuration values (kept as fallback)
├── config_manager.h       # NEW: Configuration persistence
├── web_server.h           # NEW: Web server setup and routes
├── data/                  # NEW: LittleFS files
│   ├── index.html        # Configuration web page
│   ├── style.css         # Styling
│   └── script.js         # Client-side logic
└── README.md
```

## Key Implementation Details

### Configuration Storage Strategy
- Use Preferences library with namespace "dyna_leds"
- Store each parameter with descriptive key
- Implement version checking for configuration migration
- Provide atomic save operations

### Web Server Routes
- `GET /` - Serve main configuration page
- `GET /config` - Return current configuration as JSON
- `POST /config` - Save new configuration
- `POST /wifi` - Configure WiFi credentials
- `POST /restart` - Restart ESP32
- `GET /status` - System status information

### Security Considerations
- Implement basic authentication for web interface
- Store WiFi password encrypted in Preferences
- Add CORS headers for API endpoints
- Implement rate limiting for configuration changes

### Error Handling
- Validate all input ranges before saving
- Provide meaningful error messages to user
- Implement rollback on invalid configuration
- Log errors to Serial for debugging

## Success Criteria
1. Web interface accessible via ESP32 IP address
2. All configuration parameters editable through web UI
3. Configuration persists across power cycles
4. Changes apply immediately without code upload
5. System remains stable during configuration changes
6. LED behavior updates reflect new configuration
7. WiFi credentials stored securely
8. Fallback to AP mode if WiFi connection fails

## Testing Checklist
- [ ] Web server starts successfully in AP mode
- [ ] Web server connects to existing WiFi network
- [ ] Configuration page loads correctly
- [ ] All form fields display current values
- [ ] Configuration saves successfully
- [ ] Configuration persists after restart
- [ ] LED behavior changes reflect new settings
- [ ] Invalid inputs are rejected with clear errors
- [ ] Default configuration can be restored
- [ ] System remains responsive during web access
- [ ] Multiple clients can access simultaneously
- [ ] OTA updates work correctly (if implemented)

---

## Important
- Do not create any tests or unit tests
- write the imlplementation plan to a file, and update the file with progress after each Phase.

**Note:** This plan focuses exclusively on code implementation. No documentation files should be created. All changes should maintain the existing functionality of [`dyna_leds.ino`](dyna_leds.ino:1-413) while adding web-based configuration capability.