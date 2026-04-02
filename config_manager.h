/**
 * Configuration Manager for DynaLEDs
 * 
 * Handles persistent storage and retrieval of all configuration parameters
 * using the ESP32 Preferences library (non-volatile storage).
 * 
 * Features:
 * - Load/save configuration from/to NVS (Non-Volatile Storage)
 * - Validation of all parameters
 * - Default values from config.h as fallback
 * - Configuration versioning for future migrations
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Preferences.h>
#include "config.h"

// Configuration version for future migrations
#define CONFIG_VERSION 1

// Preferences namespace
#define PREFS_NAMESPACE "dyna_leds"

// ============================================================================
// CONFIGURATION DATA STRUCTURE
// ============================================================================

/**
 * Complete configuration structure
 * Contains all parameters that can be modified via web interface
 */
struct ConfigData {
    // Configuration version
    uint8_t version;
    
    // Hardware pins
    uint8_t uartRxPin;
    uint8_t uartTxPin;
    uint8_t ledDataPin;
    
    // LED configuration
    uint16_t numLeds;
    uint8_t brightness;
    
    // Distance mapping
    float minDistance;
    float maxDistance;
    uint16_t minLeds;
    uint16_t maxLeds;
    
    // Angle mapping
    float minAngle;
    float maxAngle;
    uint8_t minHue;
    uint8_t maxHue;
    
    // Color settings
    uint8_t saturation;
    uint8_t value;
    
    // Performance
    uint16_t updateIntervalMs;
    bool debugEnabled;
    
    // Default behavior
    // 0 = ALL_OFF, 1 = ALL_ON, 2 = LAST_STATE
    uint8_t defaultBehavior;
    uint16_t defaultLedCount;
    uint8_t defaultHue;
    
    // WiFi credentials
    char wifiSsid[32];
    char wifiPassword[64];
    char apSsid[32];
    char apPassword[64];
};

// Global configuration instance
extern ConfigData globalConfig;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/**
 * Initialize configuration with defaults from config.h
 * 
 * @param config Configuration structure to initialize
 */
void initializeDefaults(ConfigData& config);

/**
 * Load configuration from Preferences
 * If no saved configuration exists, uses defaults from config.h
 * 
 * @param config Configuration structure to populate
 * @return true if loaded from storage, false if using defaults
 */
bool loadConfiguration(ConfigData& config);

/**
 * Save configuration to Preferences
 * Validates configuration before saving
 * 
 * @param config Configuration structure to save
 * @return true if saved successfully, false on error
 */
bool saveConfiguration(const ConfigData& config);

/**
 * Reset configuration to factory defaults
 * 
 * @param config Configuration structure to reset
 */
void resetToDefaults(ConfigData& config);

/**
 * Validate configuration parameters
 * Checks ranges and data integrity
 * 
 * @param config Configuration structure to validate
 * @return true if valid, false if any parameter is out of range
 */
bool validateConfiguration(const ConfigData& config);

/**
 * Clear all saved configuration from Preferences
 * Used for factory reset
 * 
 * @return true if cleared successfully
 */
bool clearConfiguration();

// ============================================================================
// IMPLEMENTATION
// ============================================================================

/**
 * Initialize configuration with defaults from config.h
 */
void initializeDefaults(ConfigData& config) {
    config.version = CONFIG_VERSION;
    
    // Hardware pins
    config.uartRxPin = UART_RX_PIN;
    config.uartTxPin = UART_TX_PIN;
    config.ledDataPin = LED_DATA_PIN;
    
    // LED configuration
    config.numLeds = NUM_LEDS;
    config.brightness = BRIGHTNESS;
    
    // Distance mapping
    config.minDistance = MIN_DISTANCE;
    config.maxDistance = MAX_DISTANCE;
    config.minLeds = MIN_LEDS;
    config.maxLeds = MAX_LEDS;
    
    // Angle mapping
    config.minAngle = MIN_ANGLE;
    config.maxAngle = MAX_ANGLE;
    config.minHue = MIN_HUE;
    config.maxHue = MAX_HUE;
    
    // Color settings
    config.saturation = SATURATION;
    config.value = VALUE;
    
    // Performance
    config.updateIntervalMs = UPDATE_INTERVAL_MS;
    config.debugEnabled = DEBUG_ENABLED;
    
    // Default behavior
    #ifdef DEFAULT_BEHAVIOR_ALL_OFF
        config.defaultBehavior = 0;
    #elif defined(DEFAULT_BEHAVIOR_ALL_ON)
        config.defaultBehavior = 1;
    #elif defined(DEFAULT_BEHAVIOR_LAST_STATE)
        config.defaultBehavior = 2;
    #else
        config.defaultBehavior = 0;
    #endif
    
    config.defaultLedCount = DEFAULT_LED_COUNT;
    config.defaultHue = DEFAULT_HUE;
    
    // WiFi credentials (defaults)
    strcpy(config.wifiSsid, "");
    strcpy(config.wifiPassword, "");
    strcpy(config.apSsid, "DynaLEDs-Setup");
    strcpy(config.apPassword, "configure");
}

/**
 * Load configuration from Preferences
 */
bool loadConfiguration(ConfigData& config) {
    Preferences prefs;
    
    // Try to open preferences in read-only mode
    if (!prefs.begin(PREFS_NAMESPACE, true)) {
        Serial.println("Failed to open Preferences");
        initializeDefaults(config);
        return false;
    }
    
    // Check if configuration exists
    if (!prefs.isKey("version")) {
        Serial.println("No saved configuration found, using defaults");
        prefs.end();
        initializeDefaults(config);
        return false;
    }
    
    // Load configuration version
    config.version = prefs.getUChar("version", CONFIG_VERSION);
    
    // Load hardware pins
    config.uartRxPin = prefs.getUChar("uartRxPin", UART_RX_PIN);
    config.uartTxPin = prefs.getUChar("uartTxPin", UART_TX_PIN);
    config.ledDataPin = prefs.getUChar("ledDataPin", LED_DATA_PIN);
    
    // Load LED configuration
    config.numLeds = prefs.getUShort("numLeds", NUM_LEDS);
    config.brightness = prefs.getUChar("brightness", BRIGHTNESS);
    
    // Load distance mapping
    config.minDistance = prefs.getFloat("minDistance", MIN_DISTANCE);
    config.maxDistance = prefs.getFloat("maxDistance", MAX_DISTANCE);
    config.minLeds = prefs.getUShort("minLeds", MIN_LEDS);
    config.maxLeds = prefs.getUShort("maxLeds", MAX_LEDS);
    
    // Load angle mapping
    config.minAngle = prefs.getFloat("minAngle", MIN_ANGLE);
    config.maxAngle = prefs.getFloat("maxAngle", MAX_ANGLE);
    config.minHue = prefs.getUChar("minHue", MIN_HUE);
    config.maxHue = prefs.getUChar("maxHue", MAX_HUE);
    
    // Load color settings
    config.saturation = prefs.getUChar("saturation", SATURATION);
    config.value = prefs.getUChar("value", VALUE);
    
    // Load performance
    config.updateIntervalMs = prefs.getUShort("updateInterval", UPDATE_INTERVAL_MS);
    config.debugEnabled = prefs.getBool("debugEnabled", DEBUG_ENABLED);
    
    // Load default behavior
    config.defaultBehavior = prefs.getUChar("defaultBehavior", 0);
    config.defaultLedCount = prefs.getUShort("defaultLedCount", DEFAULT_LED_COUNT);
    config.defaultHue = prefs.getUChar("defaultHue", DEFAULT_HUE);
    
    // Load WiFi credentials
    prefs.getString("wifiSsid", config.wifiSsid, sizeof(config.wifiSsid));
    prefs.getString("wifiPassword", config.wifiPassword, sizeof(config.wifiPassword));
    prefs.getString("apSsid", config.apSsid, sizeof(config.apSsid));
    prefs.getString("apPassword", config.apPassword, sizeof(config.apPassword));
    
    prefs.end();
    
    // Validate loaded configuration
    if (!validateConfiguration(config)) {
        Serial.println("Loaded configuration is invalid, using defaults");
        initializeDefaults(config);
        return false;
    }
    
    Serial.println("Configuration loaded successfully");
    return true;
}

/**
 * Save configuration to Preferences
 */
bool saveConfiguration(const ConfigData& config) {
    // Validate before saving
    if (!validateConfiguration(config)) {
        Serial.println("Configuration validation failed, not saving");
        return false;
    }
    
    Preferences prefs;
    
    // Open preferences in read-write mode
    if (!prefs.begin(PREFS_NAMESPACE, false)) {
        Serial.println("Failed to open Preferences for writing");
        return false;
    }
    
    // Save configuration version
    prefs.putUChar("version", config.version);
    
    // Save hardware pins
    prefs.putUChar("uartRxPin", config.uartRxPin);
    prefs.putUChar("uartTxPin", config.uartTxPin);
    prefs.putUChar("ledDataPin", config.ledDataPin);
    
    // Save LED configuration
    prefs.putUShort("numLeds", config.numLeds);
    prefs.putUChar("brightness", config.brightness);
    
    // Save distance mapping
    prefs.putFloat("minDistance", config.minDistance);
    prefs.putFloat("maxDistance", config.maxDistance);
    prefs.putUShort("minLeds", config.minLeds);
    prefs.putUShort("maxLeds", config.maxLeds);
    
    // Save angle mapping
    prefs.putFloat("minAngle", config.minAngle);
    prefs.putFloat("maxAngle", config.maxAngle);
    prefs.putUChar("minHue", config.minHue);
    prefs.putUChar("maxHue", config.maxHue);
    
    // Save color settings
    prefs.putUChar("saturation", config.saturation);
    prefs.putUChar("value", config.value);
    
    // Save performance
    prefs.putUShort("updateInterval", config.updateIntervalMs);
    prefs.putBool("debugEnabled", config.debugEnabled);
    
    // Save default behavior
    prefs.putUChar("defaultBehavior", config.defaultBehavior);
    prefs.putUShort("defaultLedCount", config.defaultLedCount);
    prefs.putUChar("defaultHue", config.defaultHue);
    
    // Save WiFi credentials
    prefs.putString("wifiSsid", config.wifiSsid);
    prefs.putString("wifiPassword", config.wifiPassword);
    prefs.putString("apSsid", config.apSsid);
    prefs.putString("apPassword", config.apPassword);
    
    prefs.end();
    
    Serial.println("Configuration saved successfully");
    return true;
}

/**
 * Reset configuration to factory defaults
 */
void resetToDefaults(ConfigData& config) {
    Serial.println("Resetting configuration to defaults");
    initializeDefaults(config);
    saveConfiguration(config);
}

/**
 * Validate configuration parameters
 */
bool validateConfiguration(const ConfigData& config) {
    // Validate GPIO pins (ESP32 valid range: 0-39, but some are restricted)
    if (config.uartRxPin > 39 || config.uartTxPin > 39 || config.ledDataPin > 39) {
        Serial.println("Invalid GPIO pin number");
        return false;
    }
    
    // Validate LED count (reasonable range: 1-1000)
    if (config.numLeds < 1 || config.numLeds > 1000) {
        Serial.println("Invalid LED count");
        return false;
    }
    
    // Validate brightness (0-255)
    if (config.brightness > 255) {
        Serial.println("Invalid brightness");
        return false;
    }
    
    // Validate distance range
    if (config.minDistance < 0 || config.maxDistance < 0 || 
        config.minDistance >= config.maxDistance) {
        Serial.println("Invalid distance range");
        return false;
    }
    
    // Validate LED range
    if (config.minLeds < 0 || config.maxLeds > config.numLeds || 
        config.minLeds >= config.maxLeds) {
        Serial.println("Invalid LED range");
        return false;
    }
    
    // Validate angle range
    if (config.minAngle >= config.maxAngle) {
        Serial.println("Invalid angle range");
        return false;
    }
    
    // Validate hue range (0-255)
    if (config.minHue > 255 || config.maxHue > 255) {
        Serial.println("Invalid hue range");
        return false;
    }
    
    // Validate color settings (0-255)
    if (config.saturation > 255 || config.value > 255) {
        Serial.println("Invalid color settings");
        return false;
    }
    
    // Validate update interval (reasonable range: 10-1000ms)
    if (config.updateIntervalMs < 10 || config.updateIntervalMs > 1000) {
        Serial.println("Invalid update interval");
        return false;
    }
    
    // Validate default behavior (0-2)
    if (config.defaultBehavior > 2) {
        Serial.println("Invalid default behavior");
        return false;
    }
    
    // Validate default LED count
    if (config.defaultLedCount > config.numLeds) {
        Serial.println("Invalid default LED count");
        return false;
    }
    
    // Validate default hue (0-255)
    if (config.defaultHue > 255) {
        Serial.println("Invalid default hue");
        return false;
    }
    
    return true;
}

/**
 * Clear all saved configuration
 */
bool clearConfiguration() {
    Preferences prefs;
    
    if (!prefs.begin(PREFS_NAMESPACE, false)) {
        Serial.println("Failed to open Preferences for clearing");
        return false;
    }
    
    bool result = prefs.clear();
    prefs.end();
    
    if (result) {
        Serial.println("Configuration cleared successfully");
    } else {
        Serial.println("Failed to clear configuration");
    }
    
    return result;
}

#endif // CONFIG_MANAGER_H

// Made with Bob
