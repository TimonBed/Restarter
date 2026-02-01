# PC Restarter - ESP32-C3

Remote PC power and reset control via web interface and Home Assistant.

## What It Does

The PC Restarter allows you to remotely control your PC's power and reset buttons from anywhere:
- **Turn PC on/off** from your phone or browser
- **Force shutdown** frozen PCs (11-second hold)
- **Integrate with Home Assistant** via MQTT auto-discovery
- **Monitor PC state** (off, booting, running)
- **Track temperature** with TMP112 sensor

## Quick Start

### 1. Hardware Setup

Connect the ESP32-C3 to your PC's front panel header:

| ESP32 GPIO | Function | Connect To |
|------------|----------|------------|
| 0 | I2C SDA | TMP112 temperature sensor |
| 1 | I2C SCL | TMP112 temperature sensor |
| 4 | Power LED | PC power LED (via optocoupler) |
| 5 | HDD LED | PC HDD LED (via optocoupler) |
| 6 | Power Relay | PC power switch header |
| 7 | Reset Relay | PC reset switch header |
| 9 | Factory Reset | Push button to GND (hold 5s) |
| 10 | Status LED | Indicates WiFi/reset status |

### 2. Upload Firmware

```bash
# Clone the repository
git clone <repo-url>
cd Restarter_Code

# Upload firmware
pio run -t upload --upload-port COM3

# Upload web interface
pio run -t uploadfs --upload-port COM3
```

### 3. Configure WiFi

1. Power on the ESP32
2. Connect to WiFi network: **Restarter-XXXXXX**
3. A setup wizard opens automatically
4. Enter your WiFi credentials
5. (Optional) Configure MQTT for Home Assistant
6. Device restarts and connects to your network

### 4. Access the Dashboard

Open `http://restarter-XXXXXX.local` or use the IP address from your router.

## Status LED (GPIO 10)

| LED State | Meaning |
|-----------|---------|
| OFF | WiFi connected |
| Solid ON | WiFi connection failed |
| Slow blink (1s) | AP mode (setup) |
| Fast blink (accelerating) | Factory reset button held |

## Factory Reset

Three ways to reset to factory settings:
1. **Hardware**: Hold GPIO 9 button for 5 seconds
2. **Web UI**: Dashboard → Device Settings → Factory Reset
3. **API**: `POST /api/factory-reset`

## Project Structure

```
Restarter_Code/
├── src/                    # C++ source files
│   ├── main.cpp            # Entry point, setup/loop
│   ├── PCController.cpp    # PC power/reset control logic
│   ├── TempSensor.cpp      # TMP112 temperature sensor driver
│   ├── Networking.cpp      # WiFi and configuration storage
│   ├── WebInterface.cpp    # Web server and REST API
│   └── MqttHandler.cpp     # MQTT and Home Assistant
│
├── include/                # C++ header files
│   ├── Config.h            # Hardware pins and timing defaults
│   ├── Constants.h         # Data structures (PCState, StoredConfig)
│   ├── PCController.h      # PC controller class definition
│   └── TempSensor.h        # Temperature sensor class definition
│
├── data/                   # Web UI files (uploaded to LittleFS)
│   ├── index.html          # Main dashboard
│   ├── style.css           # Dashboard styles
│   ├── script.js           # Dashboard JavaScript
│   ├── onboarding.html     # Setup wizard
│   ├── onboarding.css      # Wizard styles
│   └── onboarding/         # Wizard JavaScript modules
│       ├── setup-wizard.js # Main wizard controller
│       ├── wifi-form.js    # WiFi configuration step
│       ├── mqtt-form.js    # MQTT configuration step
│       ├── step-indicator.js # Progress indicator
│       └── device-icon.js  # Animated device icon
│
├── platformio.ini          # Build configuration
├── openapi.yaml            # REST API specification
├── README.md               # This file
└── MANUAL.md               # Complete user manual
```

## REST API

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/status` | Current device status |
| GET | `/api/config` | Configuration (passwords hidden) |
| POST | `/api/config` | Save configuration (restarts device) |
| POST | `/api/action/power` | Press power button |
| POST | `/api/action/reset` | Press reset button |
| POST | `/api/action/force-power` | Force shutdown (11s) |
| GET | `/api/wifi/scan` | Scan WiFi networks |
| POST | `/api/factory-reset` | Clear config, restart in AP mode |

WebSocket: `ws://<device-ip>/ws` for real-time status updates.

## Home Assistant

When MQTT is configured, the device auto-discovers in Home Assistant:
- **Power Switch**: Toggle to press power button
- **Reset Button**: Press to trigger reset

### Example Automation

```yaml
automation:
  - alias: "Wake PC when arriving home"
    trigger:
      - platform: zone
        entity_id: person.you
        zone: zone.home
        event: enter
    action:
      - service: switch.turn_on
        entity_id: switch.restarter_XXXXXX_power
```

## Development

### Commands

```bash
# Build firmware
pio run

# Upload firmware
pio run -t upload

# Upload web files
pio run -t uploadfs

# Serial monitor
pio device monitor

# Erase flash (full factory reset)
pio run -t erase
```

### Adding Features

1. **New API endpoint**: Add to `src/WebInterface.cpp`
2. **New config setting**: Add to `StoredConfig` in `include/Constants.h`, then update `Networking.cpp` to save/load it
3. **New hardware**: Add GPIO pins to `include/Config.h`

## Documentation

- `README.md` - Quick start guide (this file)
- `MANUAL.md` - Complete user manual with hardware diagrams
- `openapi.yaml` - Full REST API specification

## License

MIT
