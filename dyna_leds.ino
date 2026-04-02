/**
 * Dynamic LED Strip Control with Motion Sensing
 *
 * This sketch controls WS2812B LED strips based on motion detected by
 * an HLK-LD2450 sensor. The system maps:
 * - Target distance → Number of active LEDs (closer = fewer LEDs)
 * - Target angle → LED Hue value (color)
 *
 * Hardware:
 * - ESP32 development board
 * - HLK-LD2450 sensor (UART connection)
 * - WS2812B LED strip
 *
 * Libraries Required:
 * - FastLED (https://github.com/FastLED/FastLED)
 * - HLK-LD245X (https://github.com/Fiooodooor/HLK-LD245X)
 * - ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
 * - AsyncTCP (https://github.com/me-no-dev/AsyncTCP)
 * - ArduinoJson (https://github.com/bblanchon/ArduinoJson)
 *
 * Configuration:
 * - Access web interface at device IP address or http://dyna-leds.local
 * - Default AP: SSID "DynaLEDs-Setup", Password "configure"
 * - config.h provides default values as fallback
 */

#include <HardwareSerial.h>
#include <FastLED.h>
#include <LD2450.hpp>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "config_manager.h"
#include "web_server.h"

// Use the esphome::ld245x namespace
using namespace esphome::ld245x;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Configuration instance
ConfigData globalConfig;

// LED array (will be dynamically sized based on config)
CRGB* leds = nullptr;

// Sensor instance
HardwareSerial radarSerial(2);
LD2450 sensor;

// Web server instance
AsyncWebServer server(80);

// Timing control
unsigned long lastUpdateTime = 0;

// Last valid readings (for smoothing or fallback)
float lastValidDistance = 0.0;
float lastValidAngle = 0.0;
int lastLEDCount = 0;
uint8_t lastHue = 128;

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup() {
  // Initialize Serial first (always on for web server feedback)
  Serial.begin(115200);
  delay(1000);
  Serial.println("=================================");
  Serial.println("Dynamic LED Strip Control");
  Serial.println("Web Configuration Enabled");
  Serial.println("=================================");
  Serial.println();

  // Load configuration from Preferences
  Serial.println("Loading configuration...");
  loadConfiguration(globalConfig);
  
  // Initialize last valid readings from config
  lastValidDistance = globalConfig.maxDistance;
  lastLEDCount = globalConfig.minLeds;

  // Initialize WiFi
  initWiFi(globalConfig);

  // Initialize OTA updates
  initOTA();

  // Initialize web server
  initWebServer(server, globalConfig);

  // Initialize LED strip
  initializeLEDs();

  // Initialize sensor
  initializeSensor();

  Serial.println();
  Serial.println("Setup complete!");
  Serial.println("Waiting for targets...");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Web interface: http://");
    Serial.println(WiFi.localIP());
    Serial.println("or http://dyna-leds.local");
  } else {
    Serial.print("Web interface: http://");
    Serial.println(WiFi.softAPIP());
  }
  Serial.println();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();

  // Check if it's time to update
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime < globalConfig.updateIntervalMs) {
    return;
  }
  lastUpdateTime = currentTime;

  // Update sensor readings
  sensor.update();

  // Process sensor data and update LEDs
  // Check if there are valid targets using the correct API
  if (sensor.getNrValidTargets() > 0) {
    processTargetData();
  } else {
    handleNoTarget();
  }
}

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * Initialize OTA (Over-The-Air) updates
 */
void initOTA() {
  // Set OTA hostname
  ArduinoOTA.setHostname("dyna-leds");
  
  // Set OTA password (optional but recommended)
  ArduinoOTA.setPassword("update");
  
  // OTA callbacks
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    
    // Turn off all LEDs during update
    FastLED.clear();
    FastLED.show();
    
    Serial.println("Start updating " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update Complete!");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int percent = (progress / (total / 100));
    Serial.printf("Progress: %u%%\r", percent);
    
    // Show progress on LED strip (first LED indicates progress)
    if (leds != nullptr && globalConfig.numLeds > 0) {
      int progressLeds = (percent * globalConfig.numLeds) / 100;
      for (int i = 0; i < progressLeds; i++) {
        leds[i] = CRGB::Blue;
      }
      FastLED.show();
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
    
    // Flash red on error
    if (leds != nullptr && globalConfig.numLeds > 0) {
      for (int i = 0; i < globalConfig.numLeds; i++) {
        leds[i] = CRGB::Red;
      }
      FastLED.show();
    }
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA updates enabled");
  Serial.println("  Hostname: dyna-leds");
  Serial.println("  Password: update");
}

/**
 * Initialize FastLED library and LED strip
 */
void initializeLEDs() {
  if (globalConfig.debugEnabled) {
    Serial.println("Initializing LED strip...");
    Serial.print("  Type: WS2812B");
    Serial.print("  Count: ");
    Serial.println(globalConfig.numLeds);
    Serial.print("  Data Pin: GPIO");
    Serial.println(globalConfig.ledDataPin);
    Serial.print("  Brightness: ");
    Serial.println(globalConfig.brightness);
  }

  // Allocate LED array dynamically based on configuration
  if (leds != nullptr) {
    delete[] leds;
  }
  leds = new CRGB[globalConfig.numLeds];

  // Add LED strip to FastLED
  // Note: LED_DATA_PIN is now from config, but FastLED requires compile-time constant
  // We'll use the config value but this requires the pin to match config.h initially
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, globalConfig.numLeds);

  // Set global brightness
  FastLED.setBrightness(globalConfig.brightness);

  // Clear all LEDs initially
  FastLED.clear();
  FastLED.show();

  if (globalConfig.debugEnabled) {
    Serial.println("  Status: OK");
    Serial.println();
  }
}

/**
 * Initialize HLK-LD2450 sensor via UART
 */
void initializeSensor() {
  if (globalConfig.debugEnabled) {
    Serial.println("Initializing HLK-LD2450 sensor...");
    Serial.print("  TX Pin: GPIO");
    Serial.println(globalConfig.uartTxPin);
    Serial.print("  RX Pin: GPIO");
    Serial.println(globalConfig.uartRxPin);
    Serial.print("  Baud Rate: ");
    Serial.println(SENSOR_BAUD_RATE);
  }

  // Initialize UART for sensor communication
  radarSerial.begin(SENSOR_BAUD_RATE, SERIAL_8N1, globalConfig.uartRxPin, globalConfig.uartTxPin);

  // Initialize sensor library
  sensor.begin(radarSerial);

  if (globalConfig.debugEnabled) {
    Serial.println("  Configuring sensor...");
  }

  // Configuration session (following LD2450.ino example)
  sensor.beginConfigurationSession();

  // Optional: Set multi-target tracking mode
  //sensor.setMultiTargetTracking();
  sensor.setSingleTargetTracking();

  // Optional: Query sensor information
  if (globalConfig.debugEnabled) {
    sensor.queryTargetTrackingMode();
    sensor.queryFirmwareVersion();
    sensor.queryMacAddress();
    sensor.queryZoneFilter();
  }

  sensor.endConfigurationSession();

  if (globalConfig.debugEnabled) {
    Serial.println("  Status: OK");
    Serial.println();
  }
}

// ============================================================================
// TARGET PROCESSING FUNCTIONS
// ============================================================================

/**
 * Process target data and update LEDs
 */
void processTargetData() {
  // Get number of valid targets
  int numTargets = sensor.getNrValidTargets();

  if (numTargets == 0) {
    handleNoTarget();
    return;
  }

  /*
  // Find the closest target by iterating through all targets
  RadarTarget closestTarget;
  float minDistance = MAX_DISTANCE * 1000.0;  // Convert to millimeters

  for (int i = 0; i < numTargets; i++) {
    auto target = sensor.getTarget(i);
    //if (target != nullptr && target->d < minDistance) {
    if (target.d < minDistance) {
      minDistance = target.d;
      closestTarget = target;
    }
  }
  */
  auto closestTarget = sensor.getTarget(0);
  Serial.println(closestTarget.format().c_str());

  // Extract distance and angle from closest target
  float distanceMM = closestTarget.d;  // Distance in millimeters
  float angle = closestTarget.angle;   // Angle in degrees

  // Convert distance to meters
  float distanceM = distanceMM / 1000.0;

  // Store as last valid readings
  lastValidDistance = distanceM;
  lastValidAngle = angle;

  // Map sensor values to LED parameters
  int ledCount = mapDistanceToLEDCount(distanceM);
  uint8_t hue = mapAngleToHue(angle);

  // Store for potential reuse
  lastLEDCount = ledCount;
  lastHue = hue;

  // Update LED strip
  updateLEDs(ledCount, hue);

  // Debug output
  printTargetInfo(distanceM, angle, ledCount, hue);
}

/**
 * Handle case when no target is detected
 */
void handleNoTarget() {
  // Handle based on configured default behavior
  switch (globalConfig.defaultBehavior) {
    case 0: // ALL_OFF
      updateLEDs(0, 0);
      break;
    case 1: // ALL_ON
      updateLEDs(globalConfig.defaultLedCount, globalConfig.defaultHue);
      break;
    case 2: // LAST_STATE
      // Keep last state - do nothing
      break;
  }

  if (globalConfig.debugEnabled) {
    static unsigned long lastNoTargetPrint = 0;
    if (millis() - lastNoTargetPrint > 2000) {
      Serial.println("No targets detected");
      lastNoTargetPrint = millis();
    }
  }
}

// ============================================================================
// MAPPING FUNCTIONS
// ============================================================================

/**
 * Map distance to LED count
 * Inverse mapping: closer distance = fewer LEDs (spotlight effect)
 * 
 * @param distance Distance in meters
 * @return Number of LEDs to light up
 */
int mapDistanceToLEDCount(float distance) {
  // Clamp distance to valid range
  distance = constrain(distance, globalConfig.minDistance, globalConfig.maxDistance);

  // Calculate ratio (0.0 at min distance, 1.0 at max distance)
  float ratio = (distance - globalConfig.minDistance) / (globalConfig.maxDistance - globalConfig.minDistance);

  // Inverse mapping: closer = more LEDs
  // We want maxLeds at minDistance and minLeds at maxDistance
  int ledCount = globalConfig.maxLeds - (int)(ratio * (globalConfig.maxLeds - globalConfig.minLeds));

  // Ensure result is within valid range
  return constrain(ledCount, globalConfig.minLeds, globalConfig.maxLeds);
}

/**
 * Map angle to hue value
 * Linear mapping across the hue spectrum
 * 
 * @param angle Angle in degrees
 * @return Hue value (0-255)
 */
uint8_t mapAngleToHue(float angle) {
  // Clamp angle to valid range
  angle = constrain(angle, globalConfig.minAngle, globalConfig.maxAngle);

  // Calculate ratio (0.0 at min angle, 1.0 at max angle)
  float ratio = (angle - globalConfig.minAngle) / (globalConfig.maxAngle - globalConfig.minAngle);

  // Linear mapping to hue spectrum
  uint8_t hue = globalConfig.minHue + (uint8_t)(ratio * (globalConfig.maxHue - globalConfig.minHue));

  return hue;
}

// ============================================================================
// LED UPDATE FUNCTIONS
// ============================================================================

/**
 * Update LED strip with specified count and hue
 * 
 * @param ledCount Number of LEDs to light up
 * @param hue Hue value for the LEDs
 */
void updateLEDs(int ledCount, uint8_t hue) {
  // Ensure ledCount is within valid range
  ledCount = constrain(ledCount, 0, globalConfig.numLeds);

  // Set active LEDs to the specified color
  for (int i = 0; i < ledCount; i++) {
    leds[i] = CHSV(hue, globalConfig.saturation, globalConfig.value);
  }

  // Turn off remaining LEDs
  for (int i = ledCount; i < globalConfig.numLeds; i++) {
    leds[i] = CRGB::Black;
  }

  // Update the physical LED strip
  FastLED.show();
}

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

/**
 * Print target information to Serial Monitor
 */
void printTargetInfo(float distance, float angle, int ledCount, uint8_t hue) {
  if (!globalConfig.debugEnabled) return;
  
  static unsigned long lastPrint = 0;

  // Limit print rate to avoid flooding serial output
  if (millis() - lastPrint < 500) {
    return;
  }
  lastPrint = millis();

  Serial.println("--- Target Detected ---");
  Serial.print("Distance: ");
  Serial.print(distance, 2);
  Serial.println(" m");

  Serial.print("Angle: ");
  Serial.print(angle, 1);
  Serial.println(" °");

  Serial.print("LED Count: ");
  Serial.print(ledCount);
  Serial.print(" / ");
  Serial.println(globalConfig.numLeds);

  Serial.print("Hue: ");
  Serial.print(hue);
  Serial.print(" (");
  printColorName(hue);
  Serial.println(")");

  Serial.println();
}

/**
 * Print approximate color name for a hue value
 */
void printColorName(uint8_t hue) {
  if (hue < 16 || hue >= 240) {
    Serial.print("Red");
  } else if (hue < 48) {
    Serial.print("Orange");
  } else if (hue < 80) {
    Serial.print("Yellow");
  } else if (hue < 112) {
    Serial.print("Green");
  } else if (hue < 144) {
    Serial.print("Cyan");
  } else if (hue < 176) {
    Serial.print("Blue");
  } else if (hue < 208) {
    Serial.print("Purple");
  } else {
    Serial.print("Magenta");
  }
}

// ============================================================================
// END OF SKETCH
// ============================================================================

// Made with Bob
