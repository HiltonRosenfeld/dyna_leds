# Required Libraries for DynaLEDs Web Configuration

## Installation Instructions

### Using Arduino IDE

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for and install each library listed below

---

## Required Libraries

### 1. ESPAsyncWebServer
- **Author:** me-no-dev
- **Purpose:** Asynchronous web server for non-blocking HTTP handling
- **Repository:** https://github.com/me-no-dev/ESPAsyncWebServer
- **Installation:** Search "ESP Async WebServer" in Library Manager

### 2. AsyncTCP
- **Author:** me-no-dev
- **Purpose:** Required dependency for ESPAsyncWebServer
- **Repository:** https://github.com/me-no-dev/AsyncTCP
- **Installation:** Search "AsyncTCP" in Library Manager

### 3. ArduinoJson
- **Author:** Benoit Blanchon
- **Version:** 6.x or later (NOT version 5.x)
- **Purpose:** JSON parsing and serialization for configuration API
- **Repository:** https://github.com/bblanchon/ArduinoJson
- **Installation:** Search "ArduinoJson" in Library Manager

---

## Already Included Libraries

These libraries are part of the ESP32 Arduino core and require no separate installation:

### 4. Preferences
- **Purpose:** Non-volatile storage for configuration persistence
- **Included in:** ESP32 Arduino Core

### 5. WiFi
- **Purpose:** WiFi connectivity (Station and AP modes)
- **Included in:** ESP32 Arduino Core

### 6. ESPmDNS
- **Purpose:** mDNS responder for http://dyna-leds.local access
- **Included in:** ESP32 Arduino Core

### 7. ArduinoOTA
- **Purpose:** Over-the-air firmware updates
- **Included in:** ESP32 Arduino Core

---

## Existing Project Libraries

These libraries were already required by the original project:

### 8. FastLED
- **Purpose:** WS2812B LED strip control
- **Repository:** https://github.com/FastLED/FastLED
- **Installation:** Search "FastLED" in Library Manager

### 9. HLK-LD245X
- **Purpose:** HLK-LD2450 sensor communication
- **Repository:** https://github.com/Fiooodooor/HLK-LD245X
- **Installation:** Manual installation or search in Library Manager

---

## Installation Verification

After installing all libraries, verify by:

1. Open `dyna_leds.ino` in Arduino IDE
2. Click **Verify** (checkmark button)
3. Compilation should succeed without errors

If you see errors like "No such file or directory":
- The library is not installed
- The library version is incorrect
- The library path is not recognized

---

## Troubleshooting

### Library Not Found
**Problem:** `fatal error: ESPAsyncWebServer.h: No such file or directory`

**Solution:**
1. Restart Arduino IDE
2. Re-install the library
3. Check that ESP32 board package is installed
4. Verify library is in the correct folder

### Version Conflicts
**Problem:** Compilation errors related to ArduinoJson

**Solution:**
- Ensure ArduinoJson version 6.x is installed (NOT 5.x)
- Uninstall old versions via Library Manager
- Install latest version 6.x

### ESP32 Board Not Found
**Problem:** ESP32 boards not available in Tools → Board menu

**Solution:**
1. Go to **File → Preferences**
2. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search "ESP32" and install "esp32 by Espressif Systems"

---

## Library Versions Tested

This implementation has been tested with:

- **ESPAsyncWebServer:** Latest version
- **AsyncTCP:** Latest version
- **ArduinoJson:** 6.21.0 or later
- **FastLED:** 3.5.0 or later
- **ESP32 Arduino Core:** 2.0.0 or later

---

## Manual Installation (Alternative)

If Library Manager installation fails, you can install manually:

1. Download the library ZIP from GitHub
2. Go to **Sketch → Include Library → Add .ZIP Library**
3. Select the downloaded ZIP file
4. Restart Arduino IDE

---

## Quick Check Script

To verify all libraries are installed, create a new sketch and paste:

```cpp
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <LD2450.hpp>

void setup() {
  Serial.begin(115200);
  Serial.println("All libraries found!");
}

void loop() {}
```

If this compiles successfully, all required libraries are installed correctly.

---

**Ready to upload? See WEB_CONFIG_GUIDE.md for usage instructions!**