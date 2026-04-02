#!/bin/bash

###############################################################################
# Dynamic LED Strip Control - Build Preparation Script
# 
# This script prepares the project for compilation by:
# - Verifying all required files exist
# - Checking library dependencies
# - Validating configuration
# - Generating a build report
#
# Usage: ./scripts/prepare_build.sh
###############################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Check required files
check_files() {
    print_info "Checking required files..."
    
    local all_good=true
    
    # Check main sketch
    if [ -f "src/dyna_leds.ino" ]; then
        print_success "Main sketch: src/dyna_leds.ino"
    else
        print_error "Main sketch not found: src/dyna_leds.ino"
        all_good=false
    fi
    
    # Check config file
    if [ -f "include/config.h" ]; then
        print_success "Configuration: include/config.h"
    else
        print_error "Configuration not found: include/config.h"
        all_good=false
    fi
    
    if [ "$all_good" = false ]; then
        return 1
    fi
}

# Check Arduino CLI
check_arduino_cli() {
    print_info "Checking Arduino CLI..."
    
    if command -v arduino-cli &> /dev/null; then
        local version=$(arduino-cli version | head -n1)
        print_success "Arduino CLI installed: $version"
        return 0
    else
        print_warning "Arduino CLI not found"
        print_info "Install with: brew install arduino-cli"
        print_info "Or run: ./scripts/setup.sh"
        return 1
    fi
}

# Check ESP32 board support
check_esp32_support() {
    print_info "Checking ESP32 board support..."
    
    if ! command -v arduino-cli &> /dev/null; then
        print_warning "Cannot check (Arduino CLI not installed)"
        return 1
    fi
    
    if arduino-cli core list | grep -q "esp32:esp32"; then
        local version=$(arduino-cli core list | grep "esp32:esp32" | awk '{print $2}')
        print_success "ESP32 support installed: $version"
        return 0
    else
        print_warning "ESP32 board support not installed"
        print_info "Install with: arduino-cli core install esp32:esp32"
        print_info "Or run: ./scripts/setup.sh"
        return 1
    fi
}

# Check required libraries
check_libraries() {
    print_info "Checking required libraries..."
    
    if ! command -v arduino-cli &> /dev/null; then
        print_warning "Cannot check (Arduino CLI not installed)"
        return 1
    fi
    
    local all_good=true
    
    # Check FastLED
    if arduino-cli lib list | grep -q "FastLED"; then
        local version=$(arduino-cli lib list | grep "FastLED" | awk '{print $2}')
        print_success "FastLED: $version"
    else
        print_error "FastLED library not installed"
        print_info "Install with: arduino-cli lib install FastLED"
        all_good=false
    fi
    
    # Check HLK-LD245X
    if arduino-cli lib list | grep -q "HLK-LD245X"; then
        local version=$(arduino-cli lib list | grep "HLK-LD245X" | awk '{print $2}')
        print_success "HLK-LD245X: $version"
    else
        print_warning "HLK-LD245X library not found"
        print_info "May need manual installation from:"
        print_info "https://github.com/Fiooodooor/HLK-LD245X"
    fi
    
    if [ "$all_good" = false ]; then
        return 1
    fi
}

# Validate configuration
validate_config() {
    print_info "Validating configuration..."
    
    if [ -f "scripts/validate_config.sh" ]; then
        if ./scripts/validate_config.sh > /dev/null 2>&1; then
            print_success "Configuration is valid"
            return 0
        else
            print_error "Configuration validation failed"
            print_info "Run: ./scripts/validate_config.sh for details"
            return 1
        fi
    else
        print_warning "Validation script not found"
        return 1
    fi
}

# Check for connected ESP32
check_esp32_connection() {
    print_info "Checking for connected ESP32..."
    
    # Look for common ESP32 USB serial devices
    local devices=$(ls /dev/cu.* 2>/dev/null | grep -E "(usbserial|SLAB|wchusbserial)" || true)
    
    if [ -n "$devices" ]; then
        print_success "Found potential ESP32 device(s):"
        echo "$devices" | while read device; do
            echo "  - $device"
        done
        return 0
    else
        print_warning "No ESP32 device detected"
        print_info "Connect your ESP32 and ensure drivers are installed"
        return 1
    fi
}

# Generate build report
generate_report() {
    print_info "Generating build report..."
    
    local report_file="build_report.txt"
    
    {
        echo "Dynamic LED Strip Control - Build Report"
        echo "Generated: $(date)"
        echo ""
        echo "=== Project Files ==="
        echo "Main sketch: $([ -f src/dyna_leds.ino ] && echo "✓" || echo "✗")"
        echo "Configuration: $([ -f include/config.h ] && echo "✓" || echo "✗")"
        echo ""
        echo "=== Dependencies ==="
        if command -v arduino-cli &> /dev/null; then
            echo "Arduino CLI: ✓ $(arduino-cli version | head -n1)"
            echo "ESP32 Support: $(arduino-cli core list | grep -q "esp32:esp32" && echo "✓" || echo "✗")"
            echo "FastLED: $(arduino-cli lib list | grep -q "FastLED" && echo "✓" || echo "✗")"
            echo "HLK-LD245X: $(arduino-cli lib list | grep -q "HLK-LD245X" && echo "✓" || echo "✗")"
        else
            echo "Arduino CLI: ✗"
        fi
        echo ""
        echo "=== Configuration ==="
        if [ -f include/config.h ]; then
            echo "UART TX Pin: $(grep "^#define UART_TX_PIN" include/config.h | awk '{print $3}')"
            echo "UART RX Pin: $(grep "^#define UART_RX_PIN" include/config.h | awk '{print $3}')"
            echo "LED Data Pin: $(grep "^#define LED_DATA_PIN" include/config.h | awk '{print $3}')"
            echo "Number of LEDs: $(grep "^#define NUM_LEDS" include/config.h | awk '{print $3}')"
            echo "Brightness: $(grep "^#define BRIGHTNESS" include/config.h | awk '{print $3}')"
        fi
        echo ""
        echo "=== Connected Devices ==="
        ls /dev/cu.* 2>/dev/null | grep -E "(usbserial|SLAB|wchusbserial)" || echo "No ESP32 detected"
    } > "$report_file"
    
    print_success "Build report saved to: $report_file"
}

# Print compilation instructions
print_instructions() {
    echo ""
    print_header "Build Instructions"
    echo ""
    
    print_info "Option 1: Arduino IDE"
    echo "  1. Open src/dyna_leds.ino in Arduino IDE"
    echo "  2. Select Board: Tools → Board → ESP32 Dev Module"
    echo "  3. Select Port: Tools → Port → (your ESP32 port)"
    echo "  4. Click Upload button"
    echo ""
    
    print_info "Option 2: Arduino CLI"
    echo "  Compile:"
    echo "    arduino-cli compile --fqbn esp32:esp32:esp32 src/dyna_leds.ino"
    echo ""
    echo "  Upload (replace port with your device):"
    echo "    arduino-cli upload -p /dev/cu.usbserial-* --fqbn esp32:esp32:esp32 src/dyna_leds.ino"
    echo ""
    
    print_info "After Upload:"
    echo "  1. Open Serial Monitor (115200 baud)"
    echo "  2. Wave hand in front of sensor"
    echo "  3. Observe LED response and debug output"
}

# Print summary
print_summary() {
    echo ""
    print_header "Preparation Summary"
    echo ""
    
    local ready=true
    
    # Check critical items
    [ ! -f "src/dyna_leds.ino" ] && ready=false
    [ ! -f "include/config.h" ] && ready=false
    
    if [ "$ready" = true ]; then
        print_success "Project is ready for compilation!"
        echo ""
        print_info "Next steps:"
        echo "  1. Review build_report.txt"
        echo "  2. Follow build instructions above"
        echo "  3. Upload to ESP32"
    else
        print_error "Project is not ready for compilation"
        echo ""
        print_info "Please resolve the issues above"
    fi
}

# Main execution
main() {
    print_header "Build Preparation"
    echo ""
    
    check_files
    echo ""
    
    check_arduino_cli
    echo ""
    
    check_esp32_support
    echo ""
    
    check_libraries
    echo ""
    
    validate_config
    echo ""
    
    check_esp32_connection
    echo ""
    
    generate_report
    
    print_instructions
    print_summary
}

# Run main function
main

# Made with Bob
