#!/bin/bash

###############################################################################
# Dynamic LED Strip Control - Configuration Validation Script
# 
# This script validates the configuration in include/config.h to ensure:
# - Pin assignments don't conflict
# - Numeric values are within valid ranges
# - Configuration is logically consistent
#
# Usage: ./scripts/validate_config.sh
###############################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

CONFIG_FILE="include/config.h"
ERRORS=0
WARNINGS=0

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ ERROR: $1${NC}"
    ((ERRORS++))
}

print_warning() {
    echo -e "${YELLOW}⚠ WARNING: $1${NC}"
    ((WARNINGS++))
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Extract value from config file
get_config_value() {
    local key=$1
    grep "^#define $key" "$CONFIG_FILE" | awk '{print $3}' | tr -d '\r'
}

# Check if config file exists
check_config_exists() {
    if [ ! -f "$CONFIG_FILE" ]; then
        print_error "Configuration file not found: $CONFIG_FILE"
        exit 1
    fi
    print_success "Configuration file found"
}

# Validate pin assignments
validate_pins() {
    print_info "Validating pin assignments..."
    
    local uart_tx=$(get_config_value "UART_TX_PIN")
    local uart_rx=$(get_config_value "UART_RX_PIN")
    local led_pin=$(get_config_value "LED_DATA_PIN")
    
    # Check if pins are defined
    if [ -z "$uart_tx" ] || [ -z "$uart_rx" ] || [ -z "$led_pin" ]; then
        print_error "One or more pins are not defined"
        return
    fi
    
    print_info "  UART TX Pin: GPIO$uart_tx"
    print_info "  UART RX Pin: GPIO$uart_rx"
    print_info "  LED Data Pin: GPIO$led_pin"
    
    # Check for pin conflicts
    if [ "$uart_tx" = "$uart_rx" ]; then
        print_error "UART TX and RX pins cannot be the same"
    fi
    
    if [ "$uart_tx" = "$led_pin" ]; then
        print_error "UART TX and LED data pins cannot be the same"
    fi
    
    if [ "$uart_rx" = "$led_pin" ]; then
        print_error "UART RX and LED data pins cannot be the same"
    fi
    
    # Check for reserved pins (ESP32 specific)
    local reserved_pins=(0 1 6 7 8 9 10 11)
    for pin in "${reserved_pins[@]}"; do
        if [ "$uart_tx" = "$pin" ] || [ "$uart_rx" = "$pin" ] || [ "$led_pin" = "$pin" ]; then
            print_warning "GPIO$pin is typically reserved on ESP32 (flash/boot)"
        fi
    done
    
    if [ $ERRORS -eq 0 ]; then
        print_success "Pin assignments are valid"
    fi
}

# Validate LED configuration
validate_led_config() {
    print_info "Validating LED configuration..."
    
    local num_leds=$(get_config_value "NUM_LEDS")
    local brightness=$(get_config_value "BRIGHTNESS")
    
    if [ -z "$num_leds" ] || [ -z "$brightness" ]; then
        print_error "LED configuration values missing"
        return
    fi
    
    print_info "  Number of LEDs: $num_leds"
    print_info "  Brightness: $brightness"
    
    # Validate NUM_LEDS
    if [ "$num_leds" -le 0 ]; then
        print_error "NUM_LEDS must be greater than 0"
    elif [ "$num_leds" -gt 1000 ]; then
        print_warning "NUM_LEDS is very high ($num_leds). Ensure adequate power supply."
    fi
    
    # Validate BRIGHTNESS
    if [ "$brightness" -lt 0 ] || [ "$brightness" -gt 255 ]; then
        print_error "BRIGHTNESS must be between 0 and 255"
    elif [ "$brightness" -gt 200 ]; then
        print_warning "High brightness ($brightness) may cause excessive heat and power draw"
    fi
    
    if [ $ERRORS -eq 0 ]; then
        print_success "LED configuration is valid"
    fi
}

# Validate distance mapping
validate_distance_mapping() {
    print_info "Validating distance mapping..."
    
    local min_dist=$(get_config_value "MIN_DISTANCE")
    local max_dist=$(get_config_value "MAX_DISTANCE")
    local min_leds=$(get_config_value "MIN_LEDS")
    local max_leds=$(get_config_value "MAX_LEDS")
    local num_leds=$(get_config_value "NUM_LEDS")
    
    if [ -z "$min_dist" ] || [ -z "$max_dist" ] || [ -z "$min_leds" ] || [ -z "$max_leds" ]; then
        print_error "Distance mapping values missing"
        return
    fi
    
    print_info "  Distance range: ${min_dist}m - ${max_dist}m"
    print_info "  LED count range: $min_leds - $max_leds"
    
    # Validate distance range
    if (( $(echo "$min_dist < 0" | bc -l) )); then
        print_error "MIN_DISTANCE cannot be negative"
    fi
    
    if (( $(echo "$max_dist <= $min_dist" | bc -l) )); then
        print_error "MAX_DISTANCE must be greater than MIN_DISTANCE"
    fi
    
    if (( $(echo "$max_dist > 10" | bc -l) )); then
        print_warning "MAX_DISTANCE ($max_dist m) exceeds typical sensor range (~6m)"
    fi
    
    # Validate LED count range
    if [ "$min_leds" -lt 0 ]; then
        print_error "MIN_LEDS cannot be negative"
    fi
    
    if [ "$max_leds" -gt "$num_leds" ]; then
        print_error "MAX_LEDS ($max_leds) cannot exceed NUM_LEDS ($num_leds)"
    fi
    
    if [ "$min_leds" -ge "$max_leds" ]; then
        print_error "MIN_LEDS must be less than MAX_LEDS"
    fi
    
    if [ $ERRORS -eq 0 ]; then
        print_success "Distance mapping is valid"
    fi
}

# Validate angle mapping
validate_angle_mapping() {
    print_info "Validating angle mapping..."
    
    local min_angle=$(get_config_value "MIN_ANGLE")
    local max_angle=$(get_config_value "MAX_ANGLE")
    local min_hue=$(get_config_value "MIN_HUE")
    local max_hue=$(get_config_value "MAX_HUE")
    
    if [ -z "$min_angle" ] || [ -z "$max_angle" ] || [ -z "$min_hue" ] || [ -z "$max_hue" ]; then
        print_error "Angle mapping values missing"
        return
    fi
    
    print_info "  Angle range: ${min_angle}° - ${max_angle}°"
    print_info "  Hue range: $min_hue - $max_hue"
    
    # Validate angle range
    if (( $(echo "$max_angle <= $min_angle" | bc -l) )); then
        print_error "MAX_ANGLE must be greater than MIN_ANGLE"
    fi
    
    if (( $(echo "$min_angle < -90" | bc -l) )) || (( $(echo "$max_angle > 90" | bc -l) )); then
        print_warning "Angle range exceeds typical sensor range (-60° to +60°)"
    fi
    
    # Validate hue range
    if [ "$min_hue" -lt 0 ] || [ "$min_hue" -gt 255 ]; then
        print_error "MIN_HUE must be between 0 and 255"
    fi
    
    if [ "$max_hue" -lt 0 ] || [ "$max_hue" -gt 255 ]; then
        print_error "MAX_HUE must be between 0 and 255"
    fi
    
    if [ $ERRORS -eq 0 ]; then
        print_success "Angle mapping is valid"
    fi
}

# Validate color configuration
validate_color_config() {
    print_info "Validating color configuration..."
    
    local saturation=$(get_config_value "SATURATION")
    local value=$(get_config_value "VALUE")
    
    if [ -z "$saturation" ] || [ -z "$value" ]; then
        print_error "Color configuration values missing"
        return
    fi
    
    print_info "  Saturation: $saturation"
    print_info "  Value: $value"
    
    # Validate saturation
    if [ "$saturation" -lt 0 ] || [ "$saturation" -gt 255 ]; then
        print_error "SATURATION must be between 0 and 255"
    fi
    
    # Validate value
    if [ "$value" -lt 0 ] || [ "$value" -gt 255 ]; then
        print_error "VALUE must be between 0 and 255"
    fi
    
    if [ $ERRORS -eq 0 ]; then
        print_success "Color configuration is valid"
    fi
}

# Validate performance settings
validate_performance() {
    print_info "Validating performance settings..."
    
    local update_interval=$(get_config_value "UPDATE_INTERVAL_MS")
    
    if [ -z "$update_interval" ]; then
        print_error "UPDATE_INTERVAL_MS not defined"
        return
    fi
    
    print_info "  Update interval: ${update_interval}ms"
    
    if [ "$update_interval" -le 0 ]; then
        print_error "UPDATE_INTERVAL_MS must be greater than 0"
    elif [ "$update_interval" -lt 20 ]; then
        print_warning "Very fast update rate (${update_interval}ms) may cause high CPU usage"
    elif [ "$update_interval" -gt 200 ]; then
        print_warning "Slow update rate (${update_interval}ms) may appear laggy"
    fi
    
    if [ $ERRORS -eq 0 ]; then
        print_success "Performance settings are valid"
    fi
}

# Print summary
print_summary() {
    echo ""
    print_header "Validation Summary"
    echo ""
    
    if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
        print_success "Configuration is valid with no issues!"
        echo ""
        print_info "You can proceed with compilation and upload"
        return 0
    elif [ $ERRORS -eq 0 ]; then
        print_success "Configuration is valid"
        echo -e "${YELLOW}Found $WARNINGS warning(s)${NC}"
        echo ""
        print_info "Warnings are non-critical but should be reviewed"
        return 0
    else
        echo -e "${RED}Found $ERRORS error(s) and $WARNINGS warning(s)${NC}"
        echo ""
        print_error "Please fix errors in $CONFIG_FILE before proceeding"
        return 1
    fi
}

# Main execution
main() {
    print_header "Configuration Validation"
    echo ""
    
    check_config_exists
    echo ""
    
    validate_pins
    echo ""
    
    validate_led_config
    echo ""
    
    validate_distance_mapping
    echo ""
    
    validate_angle_mapping
    echo ""
    
    validate_color_config
    echo ""
    
    validate_performance
    
    print_summary
}

# Run main function
main
exit $?

# Made with Bob
