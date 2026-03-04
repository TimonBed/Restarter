---
title: Quick Start Guide - PC Restarter
---

# Quick Start Guide - PC Restarter

---

## 📦 Check Package Contents

- ☐ PC Restarter device
- ☐ USB power cable
- ☐ Motherboard connection cable
- ☐ This quick start guide

---

## ⚠️ Safety

- Indoor use only
- Shut down PC and unplug before installation
- Use only 5V USB power supply

---

## 🔧 Installation (5 Minutes)

### Step 1: Prepare PC
```
Shut down PC → Unplug power → Open case
```

### Step 2: Find Motherboard Headers
```
Identify Power SW (2-pin) and Power LED (2-pin)
```

### Step 3: Connect PC Restarter

```
Case Button ←→ [PC Restarter] ←→ Motherboard
                      ↓
                  USB Power
```

1. Disconnect original Power SW & LED cables from motherboard
2. Connect PC Restarter cables to motherboard
3. Connect original cables to PC Restarter
4. Connect USB cable for power

### Step 4: Close Case
```
Mount device → Close case → Power on PC
```

---

## 📱 Initial Setup (2 Minutes)

### 1. Connect to WiFi
```
Find WiFi: "PC-Restarter-XXXXXX"
Connect (no password)
```

### 2. Open Setup
```
Browser: http://192.168.4.1
```

### 3. Configure Your WiFi
```
Select network → Enter password → Save
```

### 4. Done!
```
Device now reachable at:
http://pc-restarter.local
or IP address from router
```

---

## 🎮 Operation

| Action | Web Interface | MQTT |
|--------|---------------|------|
| Power On | 🔘 Power On | `power_on` |
| Power Off | 🔘 Power Off | `power_off` |
| Force Off | 🔘 Force Off | `power_force_off` |
| Restart | 🔘 Reset | `reset` |

---

## ❓ Problems?

| Issue | Solution |
|-------|----------|
| WiFi not found | Power cycle for 10 sec |
| Wrong PC status | Swap LED cable polarity |
| Factory reset needed | Hold reset button 10 sec during power-on |

---

## 📞 Support

**Web:** https://[domain].de  
**Email:** support@[domain].de

---

**Full Manual:** https://[domain].de/docs

*CE ✓ | WEEE Reg. No.: DEXXXXX*
