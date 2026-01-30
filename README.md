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
| 10 | WiFi error LED out |

## Commands
```bash
pio run --target upload --upload-port COM3      # Firmware
pio run --target uploadfs --upload-port COM3    # Web UI
pio run --target erase --upload-port COM3       # Full reset
pio device monitor --port COM3 --baud 115200    # Serial
```

## API
| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/status` | Device status |
| GET | `/api/config` | Config (no passwords) |
| POST | `/api/config` | Save config + reboot |
| POST | `/api/action/power` | Pulse power |
| POST | `/api/action/reset` | Pulse reset |

### `/api/status` fields
- `pcState` (OFF/BOOTING/RUNNING/RESTARTING)
- `powerRelayActive`, `resetRelayActive`
- `temperature` (°C)
- `hddLastActiveSec` (-1 = never, else seconds since activity)
- `wifiConnected`, `apMode`, `ssid`, `ip`, `rssi`

## Setup
1. Upload firmware + filesystem
2. Connect to `Restarter-XXXXXX` AP
3. Open http://192.168.4.1 and save WiFi

## Notes
- WiFi config in NVS (survives uploads)
- AP mode only starts when no SSID is set
- Hard refresh (`Ctrl+Shift+R`) if old HTML shows
- See `openapi.yaml` for full API docs
