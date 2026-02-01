# PC Restarter - ESP32-C3

Remote PC power/reset via web UI and MQTT (Home Assistant).

## Hardware
| GPIO | Function |
|------|----------|
| 0 | I2C0 SDA (TMP112) |
| 1 | I2C0 SCL (TMP112) |
| 4 | Power LED in (active low) |
| 5 | HDD LED in (active low) |
| 6 | Power relay out (active high) |
| 7 | Reset relay out (active high) |
| 9 | Factory reset button (active low, hold 5s) |
| 10 | Status LED out |

### Status LED (GPIO 10)
- OFF = WiFi connected
- Solid ON = WiFi connection failed
- Slow blink = AP mode
- Accelerating blink = Factory reset in progress

## Commands
```bash
pio run --target upload --upload-port COM3      # Firmware
pio run --target uploadfs --upload-port COM3    # Web UI
pio run --target erase --upload-port COM3       # Full reset
pio device monitor --port COM3 --baud 115200    # Serial
```

## Web Interface

### Dashboard (`index.html`)
- Live PC state, temperature, HDD activity
- Hold-to-activate power/reset/force-off buttons
- Timing settings configuration
- Factory reset to restart onboarding

### Onboarding Wizard (`onboarding.html`)
- Step-by-step setup: Welcome → WiFi → MQTT → Done
- WiFi network scanning with signal strength
- Optional MQTT configuration with TLS support
- Served automatically in AP mode

## API
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/status` | Device status |
| GET | `/api/config` | Config (no passwords) |
| POST | `/api/config` | Save config + reboot |
| POST | `/api/action/power` | Pulse power |
| POST | `/api/action/reset` | Pulse reset |
| POST | `/api/action/force-power` | Force off (11s hold) |
| GET | `/api/wifi/scan` | Scan WiFi networks |
| POST | `/api/factory-reset` | Clear config + restart AP |

### `/api/status` fields
- `pcState` (OFF/BOOTING/RUNNING/RESTARTING)
- `powerRelayActive`, `resetRelayActive`
- `temperature` (°C from TMP112)
- `hddLastActiveSec` (-1 = never, else seconds since activity)
- `wifiConnected`, `apMode`, `ssid`, `ip`, `rssi`
- `freeHeap`, `totalHeap`, `cpuLoad` (ESP32 stats)

## Setup
1. Upload firmware + filesystem
2. Connect to `Restarter-XXXXXX` WiFi AP
3. Onboarding wizard opens automatically
4. Configure WiFi and optional MQTT
5. Device restarts and connects to your network

## File Structure
```
data/
├── index.html          # Main dashboard
├── style.css           # Dashboard styles
├── script.js           # Dashboard logic
├── onboarding.html     # Setup wizard
├── onboarding.css      # Wizard styles
└── onboarding/         # Wizard components
    ├── setup-wizard.js
    ├── wifi-form.js
    ├── mqtt-form.js
    ├── step-indicator.js
    └── device-icon.js
```

## Notes
- WiFi config stored in NVS (survives firmware uploads)
- AP mode starts when no WiFi SSID configured
- Factory reset: hold GPIO 9 button for 5s, or use web UI, or POST `/api/factory-reset`
- Hard refresh (`Ctrl+Shift+R`) if old HTML cached
- See `MANUAL.md` for complete user documentation
- See `openapi.yaml` for full API specification
