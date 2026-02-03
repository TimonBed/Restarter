# PC Restarter - ESP32-C3

Remote PC power and reset control via web interface, Home Assistant, and enterprise monitoring.

## Features

- **Remote Control**: Power on/off, reset, force shutdown from anywhere
- **Smart Home**: MQTT with Home Assistant auto-discovery
- **Enterprise Monitoring**: Prometheus metrics + Grafana Loki logging
- **Secure by Default**: Unique passwords per device, rate limiting, CSRF protection
- **EU Compliant**: Designed for EU Cyber Resilience Act requirements

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
# Clone and upload
git clone <repo-url>
cd Restarter

pio run -t upload --upload-port COM3
pio run -t uploadfs --upload-port COM3
```

### 3. Configure WiFi

1. Power on the ESP32
2. Connect to WiFi: **Restarter-XXXXXX** (password shown on serial)
3. Setup wizard opens automatically
4. Enter WiFi + optional integrations
5. Device restarts and connects to your network

### 4. Access Dashboard

Open `http://restarter-XXXXXX.local` or use the IP from your router.

**Default credentials**: Username `admin`, password shown on first boot (unique per device).

---

## Integrations

| Integration | Type | Description |
|-------------|------|-------------|
| **REST API** | Pull | Full control via HTTP endpoints |
| **WebSocket** | Push | Real-time status updates |
| **MQTT** | Push | Home Assistant auto-discovery |
| **Prometheus** | Pull | Metrics at `/metrics` |
| **Loki** | Push | Centralized log shipping |

### Home Assistant (MQTT)

When MQTT is configured, the device auto-discovers:
- **Power Switch**: Toggle to press power button
- **Reset Button**: Press to trigger reset

```yaml
# Example automation
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

### Prometheus

Scrape metrics from `/metrics`:

```yaml
# prometheus.yml
scrape_configs:
  - job_name: 'restarter'
    static_configs:
      - targets: ['restarter-abc123.local:80']
```

**Available metrics**:
- `restarter_pc_power` - PC power state (0/1)
- `restarter_pc_state` - Detailed state (0=OFF, 1=BOOTING, 2=RUNNING)
- `restarter_temperature_celsius` - Internal temperature
- `restarter_wifi_rssi` - WiFi signal strength
- `restarter_heap_free_bytes` - Free memory
- `restarter_uptime_seconds` - Device uptime

### Grafana Loki

Configure Loki host in device settings. Logs are pushed automatically with labels:
- `job=restarter`
- `device=<device-id>`
- `hostname=<hostname>`

---

## Security

| Feature | Description |
|---------|-------------|
| **Unique Passwords** | AP and admin passwords generated per-device (EU CRA compliant) |
| **HTTP Basic Auth** | All sensitive endpoints protected |
| **Rate Limiting** | 5 failed attempts → 5 minute lockout |
| **CSRF Protection** | Token required for all POST requests |
| **Password Obfuscation** | Credentials XOR-obfuscated in NVS |
| **MQTT TLS** | Optional TLS for MQTT connections |

---

## REST API

All POST endpoints require authentication and CSRF token (except in AP mode).

| Method | Endpoint | Auth | Description |
|--------|----------|------|-------------|
| GET | `/api/status` | No | Current device status + CSRF token |
| GET | `/api/config` | Yes | Configuration (passwords hidden) |
| POST | `/api/config` | Yes | Save configuration (restarts device) |
| POST | `/api/action/power` | Yes | Press power button |
| POST | `/api/action/reset` | Yes | Press reset button |
| POST | `/api/action/force-power` | Yes | Force shutdown (11s hold) |
| GET | `/api/wifi/scan` | No | Scan WiFi networks |
| POST | `/api/factory-reset` | Yes | Clear config, restart in AP mode |
| GET | `/metrics` | No | Prometheus metrics |

**WebSocket**: `ws://<device-ip>/ws` for real-time status updates.

See `openapi.yaml` for full API specification.

---

## Status LED (GPIO 10)

| LED State | Meaning |
|-----------|---------|
| OFF | WiFi connected |
| Solid ON | WiFi connection failed |
| Slow blink (1s) | AP mode (setup) |
| Fast blink (accelerating) | Factory reset button held |

---

## Factory Reset

Three ways to reset:
1. **Hardware**: Hold GPIO 9 button for 5 seconds
2. **Web UI**: Dashboard → Settings → Factory Reset
3. **API**: `POST /api/factory-reset` (with auth + CSRF)

---

## Project Structure

```
Restarter/
├── src/
│   ├── main.cpp            # Entry point, watchdog, health monitoring
│   ├── PCController.cpp    # PC power/reset control logic
│   ├── TempSensor.cpp      # TMP112 temperature sensor
│   ├── Networking.cpp      # WiFi, NVS config storage
│   ├── WebInterface.cpp    # Web server, REST API, auth, CSRF
│   ├── FactoryReset.cpp    # Hardware reset button handler
│   └── integrations/       # External service integrations
│       ├── MqttHandler.cpp     # MQTT + Home Assistant discovery
│       ├── MetricsHandler.cpp  # Prometheus /metrics endpoint
│       └── LokiHandler.cpp     # Grafana Loki log shipping
│
├── include/
│   ├── Config.h            # Hardware pins, timing defaults
│   ├── Constants.h         # Data structures (StoredConfig, RuntimeState)
│   ├── PCController.h      # PC controller class
│   ├── TempSensor.h        # Temperature sensor class
│   └── integrations/       # Integration headers
│       ├── MqttHandler.h
│       ├── MetricsHandler.h
│       └── LokiHandler.h
│
├── data/                   # Web UI (LittleFS)
│   ├── index.html          # Dashboard
│   ├── onboarding.html     # Setup wizard
│   └── ...
│
├── docs/                   # Documentation
│   ├── produkt/            # User manuals (DE/EN)
│   ├── technisch/          # Technical datasheets
│   ├── rechtlich/          # Legal documents
│   └── compliance/         # EU conformity docs
│
├── platformio.ini          # Build configuration
├── openapi.yaml            # REST API specification
├── ROADMAP.md              # Development roadmap
└── README.md               # This file
```

---

## Development

```bash
pio run                    # Build firmware
pio run -t upload          # Upload firmware
pio run -t uploadfs        # Upload web files
pio device monitor         # Serial monitor
pio run -t erase           # Full flash erase
```

### Adding Features

1. **New API endpoint**: Add to `WebInterface.cpp`
2. **New config setting**: Add to `StoredConfig` in `Config.h`, update `Networking.cpp`
3. **New integration**:
   - Create `include/integrations/XxxHandler.h` with function declarations
   - Create `src/integrations/XxxHandler.cpp` with implementation
   - Add `#include "integrations/XxxHandler.h"` to `main.cpp`
   - Call `XxxHandler_setup()` in `setup()` and `XxxHandler_loop()` in `loop()`

---

## License

MIT
