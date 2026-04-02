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
 * 
 * Configuration:
 * - Edit include/config.h to customize hardware pins and behavior
 */

#include <FastLED.h>
#include <LD2450.hpp>
#include "config.h"

// Use the esphome::ld245x namespace
using namespace esphome::ld245x;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// LED array
CRGB leds[NUM_LEDS];

// Sensor instance
LD2450 sensor;

// Timing control
unsigned long lastUpdateTime = 0;

// Last valid readings (for smoothing or fallback)
float lastValidDistance = MAX_DISTANCE;
float lastValidAngle = 0.0;
int lastLEDCount = MIN_LEDS;
uint8_t lastHue = 128;

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup() {
  // Initialize Serial for debugging
  #if DEBUG_ENABLED
    Serial.begin(DEBUG_BAUD_RATE);
    delay(1000);
    Serial.println("=================================");
    Serial.println("Dynamic LED Strip Control");
    Serial.println("=================================");
    Serial.println();
  #endif

  // Initialize LED strip
  initializeLEDs();

  // Initialize sensor
  initializeSensor();

  #if DEBUG_ENABLED
    Serial.println("Setup complete!");
    Serial.println("Waiting for targets...");
    Serial.println();
  #endif
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Check if it's time to update
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime < UPDATE_INTERVAL_MS) {
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
 * Initialize FastLED library and LED strip
 */
void initializeLEDs() {
  #if DEBUG_ENABLED
    Serial.println("Initializing LED strip...");
    Serial.print("  Type: ");
    Serial.println("WS2812B");
    Serial.print("  Count: ");
    Serial.println(NUM_LEDS);
    Serial.print("  Data Pin: GPIO");
    Serial.println(LED_DATA_PIN);
    Serial.print("  Brightness: ");
    Serial.println(BRIGHTNESS);
  #endif

  // Add LED strip to FastLED
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  
  // Set global brightness
  FastLED.setBrightness(BRIGHTNESS);
  
  // Clear all LEDs initially
  FastLED.clear();
  FastLED.show();

  #if DEBUG_ENABLED
    Serial.println("  Status: OK");
    Serial.println();
  #endif
}

/**
 * Initialize HLK-LD2450 sensor via UART
 */
void initializeSensor() {
  #if DEBUG_ENABLED
    Serial.println("Initializing HLK-LD2450 sensor...");
    Serial.print("  TX Pin: GPIO");
    Serial.println(UART_TX_PIN);
    Serial.print("  RX Pin: GPIO");
    Serial.println(UART_RX_PIN);
    Serial.print("  Baud Rate: ");
    Serial.println(SENSOR_BAUD_RATE);
  #endif

  // Initialize UART for sensor communication
  Serial2.begin(SENSOR_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
  // Initialize sensor library
  sensor.begin(&Serial2);

  #if DEBUG_ENABLED
    Serial.println("  Configuring sensor...");
  #endif

  // Configuration session (following LD2450.ino example)
  sensor.beginConfigurationSession();
  
  // Optional: Set multi-target tracking mode
  sensor.setMultiTargetTracking();
  
  // Optional: Query sensor information
  #if DEBUG_ENABLED
    sensor.queryTargetTrackingMode();
    sensor.queryFirmwareVersion();
    sensor.queryMacAddress();
    sensor.queryZoneFilter();
  #endif
  
  sensor.endConfigurationSession();

  #if DEBUG_ENABLED
    Serial.println("  Status: OK");
    Serial.println();
  #endif
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

  // Find the closest target by iterating through all targets
  Target* closestTarget = nullptr;
  float minDistance = MAX_DISTANCE * 1000.0; // Convert to millimeters
  
  for (int i = 0; i < numTargets; i++) {
    Target* target = sensor.getTarget(i);
    if (target != nullptr && target->d < minDistance) {
      minDistance = target->d;
      closestTarget = target;
    }
  }
  
  if (closestTarget == nullptr) {
    handleNoTarget();
    return;
  }

  // Extract distance and angle from closest target
  float distanceMM = closestTarget->d;      // Distance in millimeters
  float angle = closestTarget->angle;       // Angle in degrees

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
  #if DEBUG_ENABLED
    printTargetInfo(distanceM, angle, ledCount, hue);
  #endif
}

/**
 * Handle case when no target is detected
 */
void handleNoTarget() {
  #ifdef DEFAULT_BEHAVIOR_ALL_OFF
    updateLEDs(0, 0);
  #endif

  #ifdef DEFAULT_BEHAVIOR_ALL_ON
    updateLEDs(DEFAULT_LED_COUNT, DEFAULT_HUE);
  #endif

  #ifdef DEFAULT_BEHAVIOR_LAST_STATE
    // Keep last state - do nothing
  #endif

  #if DEBUG_ENABLED
    static unsigned long lastNoTargetPrint = 0;
    if (millis() - lastNoTargetPrint > 2000) {
      Serial.println("No targets detected");
      lastNoTargetPrint = millis();
    }
  #endif
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
  distance = constrain(distance, MIN_DISTANCE, MAX_DISTANCE);

  // Calculate ratio (0.0 at min distance, 1.0 at max distance)
  float ratio = (distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE);

  // Inverse mapping: closer = more LEDs
  // We want MAX_LEDS at MIN_DISTANCE and MIN_LEDS at MAX_DISTANCE
  int ledCount = MAX_LEDS - (int)(ratio * (MAX_LEDS - MIN_LEDS));

  // Ensure result is within valid range
  return constrain(ledCount, MIN_LEDS, MAX_LEDS);
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
  angle = constrain(angle, MIN_ANGLE, MAX_ANGLE);

  // Calculate ratio (0.0 at min angle, 1.0 at max angle)
  float ratio = (angle - MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE);

  // Linear mapping to hue spectrum
  uint8_t hue = MIN_HUE + (uint8_t)(ratio * (MAX_HUE - MIN_HUE));

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
  ledCount = constrain(ledCount, 0, NUM_LEDS);

  // Set active LEDs to the specified color
  for (int i = 0; i < ledCount; i++) {
    leds[i] = CHSV(hue, SATURATION, VALUE);
  }

  // Turn off remaining LEDs
  for (int i = ledCount; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  // Update the physical LED strip
  FastLED.show();
}

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

#if DEBUG_ENABLED
/**
 * Print target information to Serial Monitor
 */
void printTargetInfo(float distance, float angle, int ledCount, uint8_t hue) {
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
  Serial.println(NUM_LEDS);
  
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
#endif

// ============================================================================
// END OF SKETCH
// ============================================================================

// Made with Bob
