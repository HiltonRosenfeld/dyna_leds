/**
 * Web Server for DynaLEDs Configuration
 * 
 * Provides a web-based interface for configuring all system parameters.
 * Uses ESPAsyncWebServer for non-blocking operation.
 * All HTML/CSS/JS is embedded as PROGMEM constants.
 * 
 * Features:
 * - Responsive web interface
 * - JSON API for configuration
 * - WiFi configuration
 * - System status monitoring
 * - Device restart capability
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "config_manager.h"

// Forward declarations
void initWebServer(AsyncWebServer& server, ConfigData& config);
void initWiFi(ConfigData& config);

// ============================================================================
// EMBEDDED WEB INTERFACE (HTML + CSS + JavaScript)
// ============================================================================

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>DynaLEDs Configuration</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }
        
        .container {
            max-width: 900px;
            margin: 0 auto;
            background: white;
            border-radius: 12px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        
        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        
        header h1 {
            font-size: 2em;
            margin-bottom: 10px;
        }
        
        #status {
            display: inline-block;
            padding: 5px 15px;
            background: rgba(255,255,255,0.2);
            border-radius: 20px;
            font-size: 0.9em;
        }
        
        main {
            padding: 30px;
        }
        
        section {
            margin-bottom: 30px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        
        section h2 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 1.3em;
        }
        
        .form-group {
            margin-bottom: 15px;
        }
        
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: 500;
            color: #555;
        }
        
        input[type="number"],
        input[type="text"],
        input[type="password"],
        select {
            width: 100%;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 6px;
            font-size: 1em;
            transition: border-color 0.3s;
        }
        
        input:focus,
        select:focus {
            outline: none;
            border-color: #667eea;
        }
        
        .input-group {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
        }
        
        .color-preview {
            width: 100%;
            height: 50px;
            border-radius: 6px;
            margin-top: 10px;
            border: 2px solid #ddd;
        }
        
        .actions {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
            justify-content: center;
            margin-top: 30px;
        }
        
        button {
            padding: 12px 30px;
            font-size: 1em;
            font-weight: 600;
            border: none;
            border-radius: 6px;
            cursor: pointer;
            transition: all 0.3s;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        }
        
        #save {
            background: #28a745;
            color: white;
        }
        
        #save:hover {
            background: #218838;
        }
        
        #reset {
            background: #ffc107;
            color: #333;
        }
        
        #reset:hover {
            background: #e0a800;
        }
        
        #restart {
            background: #dc3545;
            color: white;
        }
        
        #restart:hover {
            background: #c82333;
        }
        
        .notification {
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 15px 25px;
            border-radius: 6px;
            color: white;
            font-weight: 500;
            box-shadow: 0 5px 15px rgba(0,0,0,0.3);
            animation: slideIn 0.3s ease-out;
            z-index: 1000;
        }
        
        @keyframes slideIn {
            from {
                transform: translateX(400px);
                opacity: 0;
            }
            to {
                transform: translateX(0);
                opacity: 1;
            }
        }
        
        .notification.success {
            background: #28a745;
        }
        
        .notification.error {
            background: #dc3545;
        }
        
        .notification.info {
            background: #17a2b8;
        }
        
        .loading {
            display: inline-block;
            width: 20px;
            height: 20px;
            border: 3px solid rgba(255,255,255,0.3);
            border-radius: 50%;
            border-top-color: white;
            animation: spin 1s linear infinite;
        }
        
        @keyframes spin {
            to { transform: rotate(360deg); }
        }
        
        @media (max-width: 768px) {
            .input-group {
                grid-template-columns: 1fr;
            }
            
            .actions {
                flex-direction: column;
            }
            
            button {
                width: 100%;
            }
        }
        
        .help-text {
            font-size: 0.85em;
            color: #666;
            margin-top: 5px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🎨 DynaLEDs Configuration</h1>
            <div id="status">Loading...</div>
        </header>
        
        <main>
            <section id="hardware">
                <h2>⚙️ Hardware Pins</h2>
                <div class="input-group">
                    <div class="form-group">
                        <label for="uartRxPin">UART RX Pin (GPIO)</label>
                        <input type="number" id="uartRxPin" min="0" max="39" required>
                        <div class="help-text">ESP32 RX from sensor TX</div>
                    </div>
                    <div class="form-group">
                        <label for="uartTxPin">UART TX Pin (GPIO)</label>
                        <input type="number" id="uartTxPin" min="0" max="39" required>
                        <div class="help-text">ESP32 TX to sensor RX</div>
                    </div>
                </div>
                <div class="form-group">
                    <label for="ledDataPin">LED Data Pin (GPIO)</label>
                    <input type="number" id="ledDataPin" min="0" max="39" required>
                    <div class="help-text">WS2812B data input pin</div>
                </div>
            </section>
            
            <section id="leds">
                <h2>💡 LED Configuration</h2>
                <div class="input-group">
                    <div class="form-group">
                        <label for="numLeds">Number of LEDs</label>
                        <input type="number" id="numLeds" min="1" max="1000" required>
                        <div class="help-text">Total LEDs in strip</div>
                    </div>
                    <div class="form-group">
                        <label for="brightness">Brightness (0-255)</label>
                        <input type="number" id="brightness" min="0" max="255" required>
                        <div class="help-text">Global brightness level</div>
                    </div>
                </div>
            </section>
            
            <section id="distance">
                <h2>📏 Distance Mapping</h2>
                <div class="input-group">
                    <div class="form-group">
                        <label for="minDistance">Min Distance (m)</label>
                        <input type="number" id="minDistance" min="0" max="10" step="0.1" required>
                    </div>
                    <div class="form-group">
                        <label for="maxDistance">Max Distance (m)</label>
                        <input type="number" id="maxDistance" min="0" max="10" step="0.1" required>
                    </div>
                </div>
                <div class="input-group">
                    <div class="form-group">
                        <label for="minLeds">Min LEDs (far)</label>
                        <input type="number" id="minLeds" min="0" max="1000" required>
                        <div class="help-text">LEDs lit when far away</div>
                    </div>
                    <div class="form-group">
                        <label for="maxLeds">Max LEDs (close)</label>
                        <input type="number" id="maxLeds" min="0" max="1000" required>
                        <div class="help-text">LEDs lit when close</div>
                    </div>
                </div>
            </section>
            
            <section id="angle">
                <h2>🎯 Angle Mapping</h2>
                <div class="input-group">
                    <div class="form-group">
                        <label for="minAngle">Min Angle (°)</label>
                        <input type="number" id="minAngle" min="-180" max="180" step="1" required>
                        <div class="help-text">Left side angle</div>
                    </div>
                    <div class="form-group">
                        <label for="maxAngle">Max Angle (°)</label>
                        <input type="number" id="maxAngle" min="-180" max="180" step="1" required>
                        <div class="help-text">Right side angle</div>
                    </div>
                </div>
                <div class="input-group">
                    <div class="form-group">
                        <label for="minHue">Min Hue (0-255)</label>
                        <input type="number" id="minHue" min="0" max="255" required>
                        <div class="help-text">Color at min angle</div>
                    </div>
                    <div class="form-group">
                        <label for="maxHue">Max Hue (0-255)</label>
                        <input type="number" id="maxHue" min="0" max="255" required>
                        <div class="help-text">Color at max angle</div>
                    </div>
                </div>
            </section>
            
            <section id="color">
                <h2>🎨 Color Settings</h2>
                <div class="input-group">
                    <div class="form-group">
                        <label for="saturation">Saturation (0-255)</label>
                        <input type="number" id="saturation" min="0" max="255" required>
                        <div class="help-text">0=white, 255=full color</div>
                    </div>
                    <div class="form-group">
                        <label for="value">Value/Brightness (0-255)</label>
                        <input type="number" id="value" min="0" max="255" required>
                        <div class="help-text">Color intensity</div>
                    </div>
                </div>
                <div class="form-group">
                    <label>Color Preview</label>
                    <div id="colorPreview" class="color-preview"></div>
                </div>
            </section>
            
            <section id="performance">
                <h2>⚡ Performance</h2>
                <div class="input-group">
                    <div class="form-group">
                        <label for="updateIntervalMs">Update Interval (ms)</label>
                        <input type="number" id="updateIntervalMs" min="10" max="1000" required>
                        <div class="help-text">Lower = faster updates</div>
                    </div>
                    <div class="form-group">
                        <label for="debugEnabled">Debug Output</label>
                        <select id="debugEnabled">
                            <option value="true">Enabled</option>
                            <option value="false">Disabled</option>
                        </select>
                        <div class="help-text">Serial debug messages</div>
                    </div>
                </div>
            </section>
            
            <section id="default">
                <h2>🔄 Default Behavior (No Target)</h2>
                <div class="form-group">
                    <label for="defaultBehavior">Behavior Mode</label>
                    <select id="defaultBehavior">
                        <option value="0">All Off</option>
                        <option value="1">All On</option>
                        <option value="2">Last State</option>
                    </select>
                </div>
                <div class="input-group">
                    <div class="form-group">
                        <label for="defaultLedCount">Default LED Count</label>
                        <input type="number" id="defaultLedCount" min="0" max="1000" required>
                        <div class="help-text">For "All On" mode</div>
                    </div>
                    <div class="form-group">
                        <label for="defaultHue">Default Hue (0-255)</label>
                        <input type="number" id="defaultHue" min="0" max="255" required>
                        <div class="help-text">For "All On" mode</div>
                    </div>
                </div>
            </section>
            
            <section id="wifi">
                <h2>📡 WiFi Configuration</h2>
                <div class="form-group">
                    <label for="wifiSsid">WiFi SSID</label>
                    <input type="text" id="wifiSsid" maxlength="31" placeholder="Your WiFi network name">
                    <div class="help-text">Leave empty for AP mode only</div>
                </div>
                <div class="form-group">
                    <label for="wifiPassword">WiFi Password</label>
                    <input type="password" id="wifiPassword" maxlength="63" placeholder="Your WiFi password">
                </div>
                <div class="input-group">
                    <div class="form-group">
                        <label for="apSsid">AP SSID</label>
                        <input type="text" id="apSsid" maxlength="31" required>
                        <div class="help-text">Access Point name</div>
                    </div>
                    <div class="form-group">
                        <label for="apPassword">AP Password</label>
                        <input type="password" id="apPassword" maxlength="63" required>
                        <div class="help-text">Min 8 characters</div>
                    </div>
                </div>
            </section>
            
            <section class="actions">
                <button id="save">💾 Save Configuration</button>
                <button id="reset">🔄 Restore Defaults</button>
                <button id="restart">🔌 Restart Device</button>
            </section>
        </main>
    </div>
    
    <script>
        // Load configuration on page load
        window.addEventListener('DOMContentLoaded', loadConfig);
        
        // Button event listeners
        document.getElementById('save').addEventListener('click', saveConfig);
        document.getElementById('reset').addEventListener('click', resetConfig);
        document.getElementById('restart').addEventListener('click', restartDevice);
        
        // Color preview update
        document.getElementById('minHue').addEventListener('input', updateColorPreview);
        document.getElementById('saturation').addEventListener('input', updateColorPreview);
        document.getElementById('value').addEventListener('input', updateColorPreview);
        
        async function loadConfig() {
            try {
                showNotification('Loading configuration...', 'info');
                const response = await fetch('/config');
                if (!response.ok) throw new Error('Failed to load configuration');
                
                const config = await response.json();
                populateForm(config);
                updateColorPreview();
                showNotification('Configuration loaded successfully', 'success');
                document.getElementById('status').textContent = 'Connected';
            } catch (error) {
                showNotification('Error loading configuration: ' + error.message, 'error');
                document.getElementById('status').textContent = 'Error';
            }
        }
        
        function populateForm(config) {
            // Hardware
            document.getElementById('uartRxPin').value = config.hardware.uartRxPin;
            document.getElementById('uartTxPin').value = config.hardware.uartTxPin;
            document.getElementById('ledDataPin').value = config.hardware.ledDataPin;
            
            // LEDs
            document.getElementById('numLeds').value = config.leds.numLeds;
            document.getElementById('brightness').value = config.leds.brightness;
            
            // Distance
            document.getElementById('minDistance').value = config.distance.min;
            document.getElementById('maxDistance').value = config.distance.max;
            document.getElementById('minLeds').value = config.distance.minLeds;
            document.getElementById('maxLeds').value = config.distance.maxLeds;
            
            // Angle
            document.getElementById('minAngle').value = config.angle.min;
            document.getElementById('maxAngle').value = config.angle.max;
            document.getElementById('minHue').value = config.angle.minHue;
            document.getElementById('maxHue').value = config.angle.maxHue;
            
            // Color
            document.getElementById('saturation').value = config.color.saturation;
            document.getElementById('value').value = config.color.value;
            
            // Performance
            document.getElementById('updateIntervalMs').value = config.performance.updateInterval;
            document.getElementById('debugEnabled').value = config.performance.debugEnabled.toString();
            
            // Default behavior
            document.getElementById('defaultBehavior').value = config.defaultBehavior.mode;
            document.getElementById('defaultLedCount').value = config.defaultBehavior.ledCount;
            document.getElementById('defaultHue').value = config.defaultBehavior.hue;
            
            // WiFi
            document.getElementById('wifiSsid').value = config.wifi.ssid || '';
            document.getElementById('wifiPassword').value = config.wifi.password || '';
            document.getElementById('apSsid').value = config.wifi.apSsid;
            document.getElementById('apPassword').value = config.wifi.apPassword;
        }
        
        function collectFormData() {
            return {
                hardware: {
                    uartRxPin: parseInt(document.getElementById('uartRxPin').value),
                    uartTxPin: parseInt(document.getElementById('uartTxPin').value),
                    ledDataPin: parseInt(document.getElementById('ledDataPin').value)
                },
                leds: {
                    numLeds: parseInt(document.getElementById('numLeds').value),
                    brightness: parseInt(document.getElementById('brightness').value)
                },
                distance: {
                    min: parseFloat(document.getElementById('minDistance').value),
                    max: parseFloat(document.getElementById('maxDistance').value),
                    minLeds: parseInt(document.getElementById('minLeds').value),
                    maxLeds: parseInt(document.getElementById('maxLeds').value)
                },
                angle: {
                    min: parseFloat(document.getElementById('minAngle').value),
                    max: parseFloat(document.getElementById('maxAngle').value),
                    minHue: parseInt(document.getElementById('minHue').value),
                    maxHue: parseInt(document.getElementById('maxHue').value)
                },
                color: {
                    saturation: parseInt(document.getElementById('saturation').value),
                    value: parseInt(document.getElementById('value').value)
                },
                performance: {
                    updateInterval: parseInt(document.getElementById('updateIntervalMs').value),
                    debugEnabled: document.getElementById('debugEnabled').value === 'true'
                },
                defaultBehavior: {
                    mode: parseInt(document.getElementById('defaultBehavior').value),
                    ledCount: parseInt(document.getElementById('defaultLedCount').value),
                    hue: parseInt(document.getElementById('defaultHue').value)
                },
                wifi: {
                    ssid: document.getElementById('wifiSsid').value,
                    password: document.getElementById('wifiPassword').value,
                    apSsid: document.getElementById('apSsid').value,
                    apPassword: document.getElementById('apPassword').value
                }
            };
        }
        
        async function saveConfig() {
            try {
                const config = collectFormData();
                
                if (!validateConfig(config)) {
                    return;
                }
                
                showNotification('Saving configuration...', 'info');
                
                const response = await fetch('/config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify(config)
                });
                
                if (!response.ok) {
                    const error = await response.text();
                    throw new Error(error || 'Failed to save configuration');
                }
                
                showNotification('Configuration saved! Restart device to apply changes.', 'success');
            } catch (error) {
                showNotification('Error saving configuration: ' + error.message, 'error');
            }
        }
        
        async function resetConfig() {
            if (!confirm('Reset all settings to factory defaults?')) return;
            
            try {
                showNotification('Resetting to defaults...', 'info');
                
                const response = await fetch('/reset', {method: 'POST'});
                if (!response.ok) throw new Error('Failed to reset configuration');
                
                showNotification('Configuration reset! Reloading...', 'success');
                setTimeout(() => location.reload(), 2000);
            } catch (error) {
                showNotification('Error resetting configuration: ' + error.message, 'error');
            }
        }
        
        async function restartDevice() {
            if (!confirm('Restart the device? This will apply saved configuration.')) return;
            
            try {
                showNotification('Restarting device...', 'info');
                
                await fetch('/restart', {method: 'POST'});
                
                showNotification('Device restarting... Please wait 10 seconds.', 'success');
                setTimeout(() => {
                    showNotification('Reconnecting...', 'info');
                    location.reload();
                }, 10000);
            } catch (error) {
                showNotification('Device is restarting...', 'info');
            }
        }
        
        function validateConfig(config) {
            // Validate distance range
            if (config.distance.min >= config.distance.max) {
                showNotification('Min distance must be less than max distance', 'error');
                return false;
            }
            
            // Validate LED range
            if (config.distance.minLeds >= config.distance.maxLeds) {
                showNotification('Min LEDs must be less than max LEDs', 'error');
                return false;
            }
            
            // Validate angle range
            if (config.angle.min >= config.angle.max) {
                showNotification('Min angle must be less than max angle', 'error');
                return false;
            }
            
            // Validate AP password length
            if (config.wifi.apPassword.length < 8) {
                showNotification('AP password must be at least 8 characters', 'error');
                return false;
            }
            
            return true;
        }
        
        function updateColorPreview() {
            const hue = parseInt(document.getElementById('minHue').value) || 0;
            const sat = parseInt(document.getElementById('saturation').value) || 255;
            const val = parseInt(document.getElementById('value').value) || 255;
            
            // Convert HSV to RGB for preview
            const rgb = hsvToRgb(hue / 255, sat / 255, val / 255);
            document.getElementById('colorPreview').style.backgroundColor = 
                `rgb(${rgb[0]}, ${rgb[1]}, ${rgb[2]})`;
        }
        
        function hsvToRgb(h, s, v) {
            let r, g, b;
            const i = Math.floor(h * 6);
            const f = h * 6 - i;
            const p = v * (1 - s);
            const q = v * (1 - f * s);
            const t = v * (1 - (1 - f) * s);
            
            switch (i % 6) {
                case 0: r = v; g = t; b = p; break;
                case 1: r = q; g = v; b = p; break;
                case 2: r = p; g = v; b = t; break;
                case 3: r = p; g = q; b = v; break;
                case 4: r = t; g = p; b = v; break;
                case 5: r = v; g = p; b = q; break;
            }
            
            return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
        }
        
        function showNotification(message, type) {
            // Remove existing notifications
            const existing = document.querySelectorAll('.notification');
            existing.forEach(n => n.remove());
            
            // Create new notification
            const notification = document.createElement('div');
            notification.className = `notification ${type}`;
            notification.textContent = message;
            document.body.appendChild(notification);
            
            // Auto-remove after 5 seconds
            setTimeout(() => notification.remove(), 5000);
        }
    </script>
</body>
</html>
)rawliteral";

// ============================================================================
// WEB SERVER FUNCTIONS
// ============================================================================

/**
 * Initialize WiFi connection
 * Tries to connect to saved WiFi network, falls back to AP mode
 */
void initWiFi(ConfigData& config) {
    Serial.println("Initializing WiFi...");
    
    // Try to connect to saved WiFi if credentials exist
    if (strlen(config.wifiSsid) > 0) {
        Serial.print("Connecting to WiFi: ");
        Serial.println(config.wifiSsid);
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.wifiSsid, config.wifiPassword);
        
        // Wait for connection (timeout 10 seconds)
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            
            // Start mDNS
            if (MDNS.begin("dyna-leds")) {
                Serial.println("mDNS started: http://dyna-leds.local");
            }
            return;
        } else {
            Serial.println("WiFi connection failed, starting AP mode");
        }
    }
    
    // Fall back to AP mode
    Serial.println("Starting Access Point mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.apSsid, config.apPassword);
    
    Serial.print("AP SSID: ");
    Serial.println(config.apSsid);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

/**
 * Initialize web server with all routes
 */
void initWebServer(AsyncWebServer& server, ConfigData& config) {
    Serial.println("Initializing web server...");
    
    // Serve main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", HTML_PAGE);
    });
    
    // Get current configuration as JSON
    server.on("/config", HTTP_GET, [&config](AsyncWebServerRequest *request) {
        StaticJsonDocument<1024> doc;
        
        // Hardware
        JsonObject hardware = doc.createNestedObject("hardware");
        hardware["uartRxPin"] = config.uartRxPin;
        hardware["uartTxPin"] = config.uartTxPin;
        hardware["ledDataPin"] = config.ledDataPin;
        
        // LEDs
        JsonObject leds = doc.createNestedObject("leds");
        leds["numLeds"] = config.numLeds;
        leds["brightness"] = config.brightness;
        
        // Distance
        JsonObject distance = doc.createNestedObject("distance");
        distance["min"] = config.minDistance;
        distance["max"] = config.maxDistance;
        distance["minLeds"] = config.minLeds;
        distance["maxLeds"] = config.maxLeds;
        
        // Angle
        JsonObject angle = doc.createNestedObject("angle");
        angle["min"] = config.minAngle;
        angle["max"] = config.maxAngle;
        angle["minHue"] = config.minHue;
        angle["maxHue"] = config.maxHue;
        
        // Color
        JsonObject color = doc.createNestedObject("color");
        color["saturation"] = config.saturation;
        color["value"] = config.value;
        
        // Performance
        JsonObject performance = doc.createNestedObject("performance");
        performance["updateInterval"] = config.updateIntervalMs;
        performance["debugEnabled"] = config.debugEnabled;
        
        // Default behavior
        JsonObject defaultBehavior = doc.createNestedObject("defaultBehavior");
        defaultBehavior["mode"] = config.defaultBehavior;
        defaultBehavior["ledCount"] = config.defaultLedCount;
        defaultBehavior["hue"] = config.defaultHue;
        
        // WiFi
        JsonObject wifi = doc.createNestedObject("wifi");
        wifi["ssid"] = config.wifiSsid;
        wifi["password"] = config.wifiPassword;
        wifi["apSsid"] = config.apSsid;
        wifi["apPassword"] = config.apPassword;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Save configuration
    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
        [&config](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, data, len);
            
            if (error) {
                request->send(400, "text/plain", "Invalid JSON");
                return;
            }
            
            // Update configuration
            config.uartRxPin = doc["hardware"]["uartRxPin"];
            config.uartTxPin = doc["hardware"]["uartTxPin"];
            config.ledDataPin = doc["hardware"]["ledDataPin"];
            
            config.numLeds = doc["leds"]["numLeds"];
            config.brightness = doc["leds"]["brightness"];
            
            config.minDistance = doc["distance"]["min"];
            config.maxDistance = doc["distance"]["max"];
            config.minLeds = doc["distance"]["minLeds"];
            config.maxLeds = doc["distance"]["maxLeds"];
            
            config.minAngle = doc["angle"]["min"];
            config.maxAngle = doc["angle"]["max"];
            config.minHue = doc["angle"]["minHue"];
            config.maxHue = doc["angle"]["maxHue"];
            
            config.saturation = doc["color"]["saturation"];
            config.value = doc["color"]["value"];
            
            config.updateIntervalMs = doc["performance"]["updateInterval"];
            config.debugEnabled = doc["performance"]["debugEnabled"];
            
            config.defaultBehavior = doc["defaultBehavior"]["mode"];
            config.defaultLedCount = doc["defaultBehavior"]["ledCount"];
            config.defaultHue = doc["defaultBehavior"]["hue"];
            
            strlcpy(config.wifiSsid, doc["wifi"]["ssid"] | "", sizeof(config.wifiSsid));
            strlcpy(config.wifiPassword, doc["wifi"]["password"] | "", sizeof(config.wifiPassword));
            strlcpy(config.apSsid, doc["wifi"]["apSsid"] | "DynaLEDs-Setup", sizeof(config.apSsid));
            strlcpy(config.apPassword, doc["wifi"]["apPassword"] | "configure", sizeof(config.apPassword));
            
            // Save to Preferences
            if (saveConfiguration(config)) {
                request->send(200, "text/plain", "Configuration saved");
            } else {
                request->send(500, "text/plain", "Failed to save configuration");
            }
        }
    );
    
    // Reset to defaults
    server.on("/reset", HTTP_POST, [&config](AsyncWebServerRequest *request) {
        resetToDefaults(config);
        request->send(200, "text/plain", "Configuration reset to defaults");
    });
    
    // Restart device
    server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Restarting...");
        delay(1000);
        ESP.restart();
    });
    
    // System status
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<256> doc;
        doc["uptime"] = millis() / 1000;
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        doc["ipAddress"] = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Start server
    server.begin();
    Serial.println("Web server started");
}

#endif // WEB_SERVER_H

// Made with Bob
