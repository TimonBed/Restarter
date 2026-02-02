# User Manual - PC Restarter

**Version:** 1.0  
**Date:** February 2026

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Package Contents](#2-package-contents)
3. [Technical Specifications](#3-technical-specifications)
4. [Safety Instructions](#4-safety-instructions)
5. [Installation](#5-installation)
6. [Initial Setup](#6-initial-setup)
7. [Operation](#7-operation)
8. [Web Interface](#8-web-interface)
9. [MQTT Integration](#9-mqtt-integration)
10. [Troubleshooting](#10-troubleshooting)
11. [Maintenance](#11-maintenance)
12. [Disposal](#12-disposal)
13. [Contact & Support](#13-contact--support)

---

## 1. Introduction

Thank you for choosing the PC Restarter!

The PC Restarter is an intelligent device for remote control and monitoring of computers. It enables:

- **Remote Control:** Power on/off and restart your PC via WiFi
- **Status Monitoring:** Real-time display of PC operating status
- **Smart Home Integration:** Control via MQTT-compatible systems (Home Assistant, etc.)
- **Local Control:** Works 100% without cloud connectivity

---

## 2. Package Contents

- 1x PC Restarter device
- 1x USB-A power cable
- 1x Motherboard connection cable (Power SW, Power LED)
- 1x Quick start guide
- 1x Safety instructions

---

## 3. Technical Specifications

| Property | Value |
|----------|-------|
| Operating Voltage | 5V DC (via USB) |
| Power Consumption | max. 150 mA |
| WiFi | 802.11 b/g/n (2.4 GHz) |
| Processor | ESP32-C3 |
| Dimensions | XX x XX x XX mm |
| Weight | XX g |
| Operating Temperature | 0°C to 40°C (32°F to 104°F) |
| Humidity | 10% - 80% (non-condensing) |
| Protection Rating | IP20 (indoor use only) |

---

## 4. Safety Instructions

⚠️ **WARNING:** Read all safety instructions before use!

- Indoor use only
- Protect from moisture and liquids
- Do not open – no user-serviceable parts inside
- Only operate with the included or compatible USB power supply (5V/1A)
- Do not place near heat sources
- Do not use if damaged

See enclosed safety document for complete instructions.

---

## 5. Installation

### 5.1 Preparation

1. Shut down and unplug the PC
2. Open the PC case
3. Identify motherboard connectors:
   - Power SW (Power button)
   - Power LED (Power indicator)

### 5.2 Wiring

1. **Disconnect original cables from case:**
   - Remove Power SW cable from motherboard
   - Remove Power LED cable from motherboard

2. **Connect PC Restarter:**
   - Connect PC Restarter between case button and motherboard
   - Connect motherboard-side plugs to the motherboard
   - Connect case-side plugs to original cables

3. **Power Supply:**
   - Connect USB cable to an always-on USB port or external power supply

### 5.3 Mounting

- Device can be attached with double-sided tape inside the case
- Ensure adequate air circulation

---

## 6. Initial Setup

### 6.1 Access Point Mode

On first power-up, the device starts in Access Point mode:

1. Search for WiFi network: `PC-Restarter-XXXXXX`
2. Connect to the network (no password required)
3. Open browser and navigate to `http://192.168.4.1`
4. Follow the setup wizard

### 6.2 Configure WiFi

1. Select your WiFi network from the list
2. Enter WiFi password
3. Configure optional settings
4. Click "Save"

The device will restart and connect to your WiFi.

### 6.3 Finding the IP Address

After successful connection, find the IP address via:

- Your router (DHCP client list)
- mDNS: `http://pc-restarter.local`

---

## 7. Operation

### 7.1 Physical Button

The original case button continues to work as usual.

### 7.2 Web Interface

Via the web interface you can:

- View PC status (On/Off)
- Power on PC
- Power off PC (short press)
- Force power off PC (long press)
- Restart PC

### 7.3 MQTT

With MQTT broker configured, status updates are sent and commands received.

---

## 8. Web Interface

### 8.1 Dashboard

The dashboard shows:

- Current PC status (LED indicator)
- Quick actions (Power, Reset)
- System information

### 8.2 Settings

- **WiFi:** Change network settings
- **MQTT:** Broker configuration
- **System:** Device info, restart, factory reset

---

## 9. MQTT Integration

### 9.1 Topics

| Topic | Description |
|-------|-------------|
| `restarter/status` | PC status (on/off) |
| `restarter/command` | Send commands |
| `restarter/available` | Online status |

### 9.2 Commands

| Command | Action |
|---------|--------|
| `power_on` | Turn PC on |
| `power_off` | Turn PC off |
| `power_force_off` | Force PC off |
| `reset` | Restart PC |

### 9.3 Home Assistant

Example configuration for Home Assistant:

```yaml
switch:
  - platform: mqtt
    name: "PC Power"
    state_topic: "restarter/status"
    command_topic: "restarter/command"
    payload_on: "power_on"
    payload_off: "power_off"
    state_on: "on"
    state_off: "off"
```

---

## 10. Troubleshooting

| Problem | Solution |
|---------|----------|
| Device not reachable | Check WiFi connection, restart router |
| Access point not visible | Power cycle device, wait 10 seconds |
| Wrong PC status | Check LED cable, swap polarity |
| MQTT not connecting | Verify broker address and credentials |

### Factory Reset

If the device is not reachable:

1. Disconnect power
2. Hold reset button
3. Reconnect power
4. Hold for 10 seconds
5. Device starts in Access Point mode

---

## 11. Maintenance

The device is maintenance-free.

- Regularly check for firmware updates
- Clean with dry cloth if dusty
- Do not use water or cleaning agents

---

## 12. Disposal

♻️ This product must not be disposed of with household waste.

According to WEEE Directive 2012/19/EU, the device must be properly disposed of:

- Municipal collection points for electronic waste
- Return to retailer

---

## 13. Contact & Support

**Manufacturer:**  
[Company Name]  
[Address]  
[Postal Code City]  
Germany

**Email:** support@[domain].de  
**Web:** https://[domain].de

---

*© 2026 [Company Name]. All rights reserved.*
