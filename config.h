/**
 * Dynamic LED Strip Control - Configuration File
 * 
 * This file contains all hardware and behavior configuration parameters.
 * Modify these values to customize the system without changing the main code.
 * 
 * Hardware: ESP32 + HLK-LD2450 + WS2812B LED Strip
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// HARDWARE PIN CONFIGURATION
// ============================================================================

/**
 * UART Pins for HLK-LD2450 Sensor
 * Note: ESP32 TX connects to sensor RX, ESP32 RX connects to sensor TX
 */
#define UART_RX_PIN 16        // ESP32 RX pin (from sensor TX)
#define UART_TX_PIN 17        // ESP32 TX pin (to sensor RX)

/**
 * LED Strip Data Pin
 * Connect to the data input (DIN) of the WS2812B LED strip
 */
#define LED_DATA_PIN 21        // GPIO pin for LED strip data

// ============================================================================
// LED STRIP CONFIGURATION
// ============================================================================

/**
 * Number of LEDs in the strip
 * Adjust this to match your actual LED count
 */
#define NUM_LEDS 70

/**
 * LED Strip Type and Color Order
 * Common types: WS2812B, WS2811, SK6812
 * Common orders: GRB, RGB, GRBW
 */
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

/**
 * Global Brightness (0-255)
 * Lower values reduce power consumption and heat
 * Recommended: 128 for indoor use, 255 for maximum brightness
 */
#define BRIGHTNESS 128

// ============================================================================
// SENSOR CONFIGURATION
// ============================================================================

/**
 * HLK-LD2450 Baud Rate
 * Default: 256000 (do not change unless sensor is reconfigured)
 */
#define SENSOR_BAUD_RATE 256000

// ============================================================================
// DISTANCE TO LED COUNT MAPPING
// ============================================================================

/**
 * Distance Range (in meters)
 * MIN_DISTANCE: Closest detection distance
 * MAX_DISTANCE: Farthest detection distance
 * 
 * The HLK-LD2450 can detect up to ~6 meters
 */
#define MIN_DISTANCE 0.0      // Minimum distance in meters
#define MAX_DISTANCE 6.0      // Maximum distance in meters

/**
 * LED Count Range
 * MIN_LEDS: Number of LEDs lit at maximum distance (far away)
 * MAX_LEDS: Number of LEDs lit at minimum distance (very close)
 * 
 * Inverse mapping: closer target = fewer LEDs (spotlight effect)
 */
#define MIN_LEDS 5            // LEDs lit when target is far
#define MAX_LEDS 70           // LEDs lit when target is close

// ============================================================================
// ANGLE TO HUE MAPPING
// ============================================================================

/**
 * Angle Range (in degrees)
 * The HLK-LD2450 typically detects angles from -60° to +60°
 * Negative angles: left side
 * Positive angles: right side
 */
#define MIN_ANGLE -60.0       // Minimum angle in degrees (left)
#define MAX_ANGLE 60.0        // Maximum angle in degrees (right)

/**
 * Hue Range (0-255)
 * HSV color model: 0=Red, 64=Yellow, 128=Cyan, 192=Blue, 255=Red
 * 
 * Examples:
 * - Full spectrum: MIN_HUE=0, MAX_HUE=255
 * - Warm colors: MIN_HUE=0, MAX_HUE=64 (red to yellow)
 * - Cool colors: MIN_HUE=128, MAX_HUE=192 (cyan to blue)
 */
#define MIN_HUE 0             // Hue at minimum angle
#define MAX_HUE 255           // Hue at maximum angle

// ============================================================================
// COLOR CONFIGURATION
// ============================================================================

/**
 * HSV Saturation (0-255)
 * 0 = White/grayscale
 * 255 = Fully saturated colors
 */
#define SATURATION 255

/**
 * HSV Value/Brightness (0-255)
 * This is separate from global BRIGHTNESS
 * 255 = Maximum intensity for the chosen hue
 */
#define VALUE 255

// ============================================================================
// PERFORMANCE SETTINGS
// ============================================================================

/**
 * LED Update Interval (in milliseconds)
 * Lower values = faster updates but higher CPU usage
 * Higher values = slower updates but lower CPU usage
 * 
 * Recommended: 50ms (20 FPS) for smooth visual response
 */
#define UPDATE_INTERVAL_MS 50

/**
 * Serial Debug Baud Rate
 * Used for debugging output to Serial Monitor
 */
#define DEBUG_BAUD_RATE 115200

/**
 * Enable Debug Output
 * Set to true to print sensor data to Serial Monitor
 * Set to false to disable debug output (slightly better performance)
 */
#define DEBUG_ENABLED true

// ============================================================================
// DEFAULT BEHAVIOR (NO TARGET DETECTED)
// ============================================================================

/**
 * Default LED State when no target is detected
 * Options:
 * - ALL_OFF: Turn off all LEDs
 * - ALL_ON: Keep all LEDs on with default color
 * - LAST_STATE: Maintain last known state
 */
#define DEFAULT_BEHAVIOR_ALL_OFF

#ifdef DEFAULT_BEHAVIOR_ALL_OFF
  #define DEFAULT_LED_COUNT 0
  #define DEFAULT_HUE 0
#endif

#ifdef DEFAULT_BEHAVIOR_ALL_ON
  #define DEFAULT_LED_COUNT NUM_LEDS
  #define DEFAULT_HUE 128      // Cyan
#endif

#ifdef DEFAULT_BEHAVIOR_LAST_STATE
  // Will use last valid readings
#endif

// ============================================================================
// ADVANCED SETTINGS (OPTIONAL)
// ============================================================================

/**
 * Smoothing Factor (0.0 - 1.0)
 * Higher values = more smoothing but slower response
 * Lower values = faster response but more jittery
 * Set to 1.0 to disable smoothing
 * 
 * Note: Smoothing is not implemented in the basic version
 * Uncomment to enable in future versions
 */
// #define SMOOTHING_FACTOR 0.8

/**
 * Minimum Target Confidence
 * Ignore targets below this confidence level
 * Range: 0-100
 * 
 * Note: Confidence filtering is not implemented in the basic version
 */
// #define MIN_CONFIDENCE 50

#endif // CONFIG_H

// Made with Bob
