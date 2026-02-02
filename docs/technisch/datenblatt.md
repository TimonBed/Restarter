# Technical Data Sheet - PC Restarter

**Product:** PC Restarter  
**Version:** 1.0  
**Date:** February 2026

---

## Product Overview

The PC Restarter is a WiFi-enabled remote power control device for desktop computers. It interfaces with standard ATX motherboard front panel headers to provide remote power control and status monitoring capabilities.

---

## Technical Specifications

### Electrical Characteristics

| Parameter | Min | Typical | Max | Unit |
|-----------|-----|---------|-----|------|
| Operating Voltage | 4.5 | 5.0 | 5.5 | V DC |
| Operating Current | - | 80 | 150 | mA |
| Power Consumption | - | 0.4 | 0.75 | W |
| Power Button Output | - | - | 100 | mA |
| LED Input Voltage | 1.8 | 3.3 | 5.0 | V |

### Processor & Memory

| Parameter | Specification |
|-----------|---------------|
| Microcontroller | ESP32-C3 (RISC-V) |
| Clock Speed | 160 MHz |
| Flash Memory | 4 MB |
| SRAM | 400 KB |
| RTC Memory | 8 KB |

### Wireless Communication

| Parameter | Specification |
|-----------|---------------|
| WiFi Standard | IEEE 802.11 b/g/n |
| Frequency Band | 2.4 GHz (2400 - 2484 MHz) |
| Channels | 1 - 13 |
| Max TX Power | +20 dBm |
| Antenna | PCB integrated |
| Security | WPA/WPA2/WPA3 |

### Interfaces

| Interface | Type | Specification |
|-----------|------|---------------|
| Power Input | USB-A | 5V DC, 1A max |
| Power SW (MB) | 2-pin Header | To motherboard |
| Power LED (MB) | 2-pin Header | From motherboard |
| Power SW (Case) | 2-pin Header | From case button |
| Power LED (Case) | 2-pin Header | To case LED |

### Physical Dimensions

| Parameter | Value | Unit |
|-----------|-------|------|
| Length | XX | mm |
| Width | XX | mm |
| Height | XX | mm |
| Weight | XX | g |
| PCB Thickness | 1.6 | mm |

### Environmental Specifications

| Parameter | Min | Max | Unit |
|-----------|-----|-----|------|
| Operating Temperature | 0 | +40 | °C |
| Storage Temperature | -20 | +60 | °C |
| Relative Humidity | 10 | 80 | % RH |
| Altitude | - | 2000 | m |

**Note:** Non-condensing environment required.

---

## Functional Description

### Power Button Control

The device provides galvanically isolated power button control via optocoupler (PC817C):

- Switch type: Normally open (NO)
- Isolation voltage: 5000 V RMS
- Response time: < 100 µs
- Contact rating: 50 mA @ 80 V

### Power LED Sensing

LED status detection via optocoupler with configurable threshold:

- Input range: 1.8V - 5V
- Sensing current: < 5 mA
- Detection accuracy: > 99%
- Update rate: 100 ms

### Communication Protocols

| Protocol | Description |
|----------|-------------|
| HTTP/REST | Web interface and API |
| WebSocket | Real-time status updates |
| MQTT | Home automation integration |
| mDNS | Zero-configuration networking |

---

## Software Features

### Web Interface

- Responsive HTML5/CSS3 design
- Real-time status via WebSocket
- Configuration wizard
- OTA firmware updates

### MQTT Integration

| Topic | Direction | Payload |
|-------|-----------|---------|
| `restarter/status` | Publish | `on` / `off` |
| `restarter/command` | Subscribe | `power_on`, `power_off`, `reset` |
| `restarter/available` | Publish | `online` / `offline` |

### API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/status` | GET | Get current status |
| `/api/power` | POST | Power control |
| `/api/config` | GET/POST | Configuration |
| `/api/system` | GET | System information |

---

## Compliance & Certifications

### EU Compliance

| Directive | Standard | Status |
|-----------|----------|--------|
| EMC 2014/30/EU | EN 55032, EN 55035 | ✓ |
| RED 2014/53/EU | EN 300 328, EN 301 489 | ✓ |
| RoHS 2011/65/EU | EN 50581 | ✓ |
| WEEE 2012/19/EU | - | ✓ |

### Wireless Certifications

| Region | Certification |
|--------|---------------|
| EU | CE (RED) |
| WiFi Module | Pre-certified (ESP32-C3-WROOM-02) |

---

## Pin Configuration

### Motherboard Header (J1)

```
┌─────┬─────┐
│  1  │  2  │  Power SW (to MB)
├─────┼─────┤
│  3  │  4  │  Power LED (from MB)
└─────┴─────┘
```

### Case Header (J2)

```
┌─────┬─────┐
│  1  │  2  │  Power SW (from case)
├─────┼─────┤
│  3  │  4  │  Power LED (to case)
└─────┴─────┘
```

---

## Block Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        PC RESTARTER                          │
│                                                              │
│  ┌──────────┐    ┌──────────┐    ┌──────────────────────┐  │
│  │   USB    │───►│   5V     │───►│      ESP32-C3        │  │
│  │  Input   │    │ Regulator│    │                      │  │
│  └──────────┘    └──────────┘    │  ┌────────────────┐  │  │
│                                   │  │   WiFi Radio   │  │  │
│  ┌──────────┐    ┌──────────┐    │  │   (2.4 GHz)    │  │  │
│  │  Power   │◄───│ Opto-    │◄───│  └────────────────┘  │  │
│  │  SW Out  │    │ coupler  │    │                      │  │
│  └──────────┘    └──────────┘    │  ┌────────────────┐  │  │
│                                   │  │   GPIO Control │  │  │
│  ┌──────────┐    ┌──────────┐    │  └────────────────┘  │  │
│  │  Power   │───►│ Opto-    │───►│                      │  │
│  │  LED In  │    │ coupler  │    └──────────────────────┘  │
│  └──────────┘    └──────────┘                               │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Reliability Data

| Parameter | Value |
|-----------|-------|
| MTBF | > 50,000 hours |
| Operating Life | > 5 years |
| Power Cycles | > 100,000 |
| Flash Write Endurance | 100,000 cycles |

---

## Ordering Information

| Part Number | Description |
|-------------|-------------|
| PCR-001 | PC Restarter, complete kit |
| PCR-001-PCB | PCB only (for OEM) |

---

## Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | Feb 2026 | Initial release |

---

## Contact

**Technical Support:**  
Email: support@[domain].de  
Web: https://[domain].de

**Sales:**  
Email: sales@[domain].de

---

*© 2026 [Company Name]. Specifications subject to change without notice.*
