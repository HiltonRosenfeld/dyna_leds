#!/bin/bash

###############################################################################
# Dynamic LED Strip Control - Setup Script
# 
# This script automates the installation of required dependencies:
# - Arduino CLI (via Homebrew)
# - ESP32 board support
# - FastLED library
# - HLK-LD245X library
#
# Usage: ./scripts/setup.sh
###############################################################################

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
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

# Check if running on macOS
check_macos() {
    if [[ "$OSTYPE" != "darwin"* ]]; then
        print_error "This script is designed for macOS"
        print_info "For other platforms, please install dependencies manually"
        exit 1
    fi
    print_success "Running on macOS"
}

# Check if Homebrew is installed
check_homebrew() {
    print_info "Checking for Homebrew..."
    if ! command -v brew &> /dev/null; then
        print_warning "Homebrew not found"
        print_info "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        print_success "Homebrew installed"
    else
        print_success "Homebrew is installed"
    fi
}

# Install Arduino CLI
install_arduino_cli() {
    print_info "Checking for Arduino CLI..."
    if ! command -v arduino-cli &> /dev/null; then
        print_warning "Arduino CLI not found"
        print_info "Installing Arduino CLI via Homebrew..."
        brew install arduino-cli
        print_success "Arduino CLI installed"
    else
        print_success "Arduino CLI is already installed"
        arduino-cli version
    fi
}

# Initialize Arduino CLI configuration
init_arduino_cli() {
    print_info "Initializing Arduino CLI configuration..."
    
    # Create config if it doesn't exist
    if [ ! -f ~/.arduino15/arduino-cli.yaml ]; then
        arduino-cli config init
        print_success "Arduino CLI configuration created"
    else
        print_success "Arduino CLI configuration exists"
    fi
}

# Install ESP32 board support
install_esp32_support() {
    print_info "Installing ESP32 board support..."
    
    # Update core index
    arduino-cli core update-index
    
    # Check if ESP32 is already installed
    if arduino-cli core list | grep -q "esp32:esp32"; then
        print_success "ESP32 board support is already installed"
    else
        print_info "Installing ESP32 core..."
        arduino-cli core install esp32:esp32
        print_success "ESP32 board support installed"
    fi
}

# Install FastLED library
install_fastled() {
    print_info "Installing FastLED library..."
    
    # Check if already installed
    if arduino-cli lib list | grep -q "FastLED"; then
        print_success "FastLED library is already installed"
    else
        print_info "Installing FastLED..."
        arduino-cli lib install FastLED
        print_success "FastLED library installed"
    fi
}

# Install HLK-LD245X library
install_hlk_library() {
    print_info "Installing HLK-LD245X library..."
    
    # Check if already installed
    if arduino-cli lib list | grep -q "HLK-LD245X"; then
        print_success "HLK-LD245X library is already installed"
    else
        print_info "Installing HLK-LD245X..."
        
        # Try to install via library manager
        if arduino-cli lib install HLK-LD245X 2>/dev/null; then
            print_success "HLK-LD245X library installed via library manager"
        else
            print_warning "Library not found in Arduino Library Manager"
            print_info "You may need to install it manually from:"
            print_info "https://github.com/Fiooodooor/HLK-LD245X"
            print_info ""
            print_info "Manual installation steps:"
            print_info "1. Download the library from GitHub"
            print_info "2. Extract to ~/Documents/Arduino/libraries/HLK-LD245X"
            print_info "3. Restart Arduino IDE"
        fi
    fi
}

# Verify installation
verify_installation() {
    print_info "Verifying installation..."
    
    local all_good=true
    
    # Check Arduino CLI
    if command -v arduino-cli &> /dev/null; then
        print_success "Arduino CLI: OK"
    else
        print_error "Arduino CLI: NOT FOUND"
        all_good=false
    fi
    
    # Check ESP32 support
    if arduino-cli core list | grep -q "esp32:esp32"; then
        print_success "ESP32 Support: OK"
    else
        print_error "ESP32 Support: NOT FOUND"
        all_good=false
    fi
    
    # Check FastLED
    if arduino-cli lib list | grep -q "FastLED"; then
        print_success "FastLED Library: OK"
    else
        print_error "FastLED Library: NOT FOUND"
        all_good=false
    fi
    
    # Check HLK-LD245X
    if arduino-cli lib list | grep -q "HLK-LD245X"; then
        print_success "HLK-LD245X Library: OK"
    else
        print_warning "HLK-LD245X Library: May need manual installation"
    fi
    
    if [ "$all_good" = true ]; then
        print_success "All dependencies verified!"
    else
        print_error "Some dependencies are missing"
        return 1
    fi
}

# Print next steps
print_next_steps() {
    echo ""
    print_header "Setup Complete!"
    echo ""
    echo "Next steps:"
    echo "1. Edit include/config.h to configure your hardware pins"
    echo "2. Run ./scripts/validate_config.sh to verify configuration"
    echo "3. Open src/dyna_leds.ino in Arduino IDE"
    echo "4. Select Board: ESP32 Dev Module"
    echo "5. Select your ESP32 port"
    echo "6. Upload to ESP32"
    echo ""
    echo "Or use Arduino CLI:"
    echo "  arduino-cli compile --fqbn esp32:esp32:esp32 src/dyna_leds.ino"
    echo "  arduino-cli upload -p /dev/cu.usbserial-* --fqbn esp32:esp32:esp32 src/dyna_leds.ino"
    echo ""
    print_info "For more information, see README.md"
}

# Main execution
main() {
    print_header "Dynamic LED Strip Control - Setup"
    echo ""
    
    check_macos
    check_homebrew
    install_arduino_cli
    init_arduino_cli
    install_esp32_support
    install_fastled
    install_hlk_library
    
    echo ""
    verify_installation
    
    print_next_steps
}

# Run main function
main

# Made with Bob
