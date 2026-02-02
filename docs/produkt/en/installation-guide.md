# Installation Guide - PC Restarter

**Detailed step-by-step hardware installation instructions**

---

## Table of Contents

1. [Requirements](#1-requirements)
2. [Required Tools](#2-required-tools)
3. [Safety Preparations](#3-safety-preparations)
4. [Identifying Motherboard Connectors](#4-identifying-motherboard-connectors)
5. [Wiring](#5-wiring)
6. [Power Supply](#6-power-supply)
7. [Mounting](#7-mounting)
8. [Function Test](#8-function-test)
9. [Troubleshooting](#9-troubleshooting)

---

## 1. Requirements

### Compatible Systems

✅ Desktop PCs with standard ATX/Micro-ATX/Mini-ITX motherboard  
✅ Workstations  
✅ Servers with standard front panel connectors  

### Not Compatible

❌ Laptops / Notebooks  
❌ All-in-One PCs  
❌ Macs  
❌ Systems without standard front panel connectors  

### System Requirements

- Free internal USB port OR external USB power supply
- 2-pin Power SW connector on motherboard
- 2-pin Power LED connector on motherboard
- WiFi network (2.4 GHz)

---

## 2. Required Tools

- Phillips screwdriver (for case)
- Optional: Flashlight
- Optional: Anti-static wrist strap
- Optional: Cable ties

---

## 3. Safety Preparations

### ⚠️ IMPORTANT: Before Starting Installation

1. **Completely shut down PC** (not standby!)
2. **Unplug power cable**
3. **Set power supply switch to "0"** (if available)
4. **Press case power button** (discharge residual power)
5. **Wait at least 30 seconds**

### Anti-Static Measures

- Touch a grounded object (radiator pipe, water faucet)
- Do not work on carpet
- Handle electronics by edges only

---

## 4. Identifying Motherboard Connectors

### Front Panel Header Location

Front panel connectors are usually located:
- At the bottom edge of the motherboard
- Near the bottom of the case
- Labeled as "F_PANEL", "JFP1", "PANEL", etc.

### Connector Pinout (Standard)

```
Typical Layout:
┌─────────────────────────────┐
│  PWR_LED+  PWR_LED-  │  GND  │
│  PWR_SW+   PWR_SW-   │ RESET │
│  HDD_LED+  HDD_LED-  │  GND  │
└─────────────────────────────┘
```

### Important Connectors for PC Restarter

| Connector | Pins | Function |
|-----------|------|----------|
| **POWER SW** | 2 | Power button (potential-free) |
| **POWER LED** | 2 | Power indicator (+/- matters!) |

### Motherboard Manual

When in doubt:
- Consult motherboard manual
- Visit manufacturer website
- Read markings on motherboard

---

## 5. Wiring

### 5.1 Identify Original Cables

These cables typically come from the case:

```
Case Front Panel:
├── POWER SW (2-pin) ← Required
├── POWER LED (2-pin or 3-pin) ← Required
├── RESET SW (2-pin) ← Optional
├── HDD LED (2-pin) ← Not needed
└── SPEAKER (4-pin) ← Not needed
```

### 5.2 Disconnect Original Cables

1. Carefully remove **POWER SW** plug from motherboard
2. Carefully remove **POWER LED** plug from motherboard

💡 **Tip:** Take a photo of the original layout!

### 5.3 PC Restarter Cable Overview

The PC Restarter has these connectors:

```
PC Restarter:
┌──────────────────────────────────────┐
│                                      │
│  [MB_PWR_SW]  [MB_PWR_LED]  [USB]   │  ← To Motherboard
│                                      │
│  [CASE_PWR_SW] [CASE_PWR_LED]       │  ← From Case
│                                      │
└──────────────────────────────────────┘
```

### 5.4 Connection Order

**Step 1: Motherboard-side Connection**

```
PC Restarter           Motherboard
─────────────          ─────────────
MB_PWR_SW    ───────►  POWER SW Header
MB_PWR_LED   ───────►  POWER LED Header
```

**Step 2: Case-side Connection**

```
Case Cables           PC Restarter
─────────────          ─────────────
POWER SW     ───────►  CASE_PWR_SW
POWER LED    ───────►  CASE_PWR_LED
```

### 5.5 Observe Polarity

**POWER SW:** Polarity doesn't matter (potential-free contact)

**POWER LED:** Polarity matters!
- `+` to `+` (often red or colored wire)
- `-` to `-` (often white or black wire)

With wrong polarity: LED status will be displayed inverted

---

## 6. Power Supply

### Option A: Internal USB Port (Recommended)

Many motherboards have internal USB headers:

```
Internal USB 2.0 Header (9-pin):
┌─────────────────┐
│ VCC  D-  D+  GND │ NC
│ VCC  D-  D+  GND │
└─────────────────┘
```

**Advantage:** Clean cable routing, device starts with standby power

### Option B: External USB Power Supply

If no internal port available:

1. Route USB cable to outside
2. Connect to external 5V USB power supply
3. Keep power supply permanently powered

**Note:** Device must be powered BEFORE the PC!

### Option C: Always-On USB Port (Rear Panel)

Some motherboards have Always-On USB ports:
- Enable in BIOS ("USB Power in S5" or similar)
- Route USB cable through PCI slot cover

---

## 7. Mounting

### Mounting Options

**Option 1: Double-sided Tape**
- Attach to flat, clean surface
- Not on cables or hot components

**Option 2: Cable Ties**
- Attach to drive bay or cable management

**Option 3: Free-standing**
- Possible for testing purposes
- Use non-slip surface

### Placement Guidelines

✅ Well-ventilated location  
✅ Away from heat sources (CPU, GPU)  
✅ Status LED visible (optional)  
❌ Do not place on motherboard  
❌ Do not hide in cables  

---

## 8. Function Test

### 8.1 Visual Inspection

- [ ] All cables properly connected
- [ ] No loose connections
- [ ] USB cable connected
- [ ] No visible short circuits

### 8.2 First Start

1. Close PC case
2. Connect power cable
3. Turn on power supply

**PC Restarter should now start:**
- Status LED lights up
- WiFi "PC-Restarter-XXXXXX" appears

### 8.3 Test Functions

| Test | Expected Result |
|------|-----------------|
| Press case button | PC starts/stops normally |
| Web Interface: Power On | PC starts |
| Web Interface: Status | Shows correct PC status |

### 8.4 Initial Setup

Continue with software setup:
→ See User Manual, Chapter 6

---

## 9. Troubleshooting

### PC Restarter Doesn't Start

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No LED | No power | Check USB connection |
| No LED | USB port disabled | Try different port / external power |

### PC Doesn't Start

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Button no function | Wrong wiring | Check connections |
| Button no function | Defective cable | Replace cable |

### LED Status Wrong

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Always "Off" | Wrong polarity | Swap + and - |
| Always "On" | Wrong polarity | Swap + and - |
| Flickering | Loose connection | Secure plug |

### WiFi Not Visible

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No WiFi | Device not started | Check power supply |
| No WiFi | Already configured | Perform reset |

---

## Wiring Diagram

```
                          ┌─────────────────┐
                          │   PC RESTARTER  │
                          │                 │
    ┌──────────┐          │  ┌───┐  ┌───┐  │          ┌──────────┐
    │   CASE   │          │  │PWR│  │LED│  │          │MOTHERBOARD│
    │          │          │  │SW │  │   │  │          │          │
    │ [PWR SW]─┼──────────┼──┤   │  │   │──┼──────────┼─[PWR SW] │
    │          │          │  └───┘  └───┘  │          │          │
    │[PWR LED]─┼──────────┼────────────────┼──────────┼─[PWR LED]│
    │          │          │                │          │          │
    └──────────┘          │     [USB]      │          └──────────┘
                          │       │        │
                          └───────┼────────┘
                                  │
                                  ▼
                          ┌──────────────┐
                          │  USB Power   │
                          │    (5V)      │
                          └──────────────┘
```

---

## Support

For installation problems:

**Email:** support@[domain].de  
**Web:** https://[domain].de/support

---

*Date: February 2026*
