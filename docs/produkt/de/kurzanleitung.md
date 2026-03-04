---
title: Kurzanleitung - PC Restarter
---

# Kurzanleitung - PC Restarter

---

## 📦 Lieferumfang prüfen

- ☐ PC Restarter Gerät
- ☐ USB-Stromkabel
- ☐ Mainboard-Verbindungskabel
- ☐ Diese Kurzanleitung

---

## ⚠️ Sicherheit

- Nur für den Innenbereich
- PC vor Installation ausschalten und vom Strom trennen
- Nur 5V USB-Stromversorgung verwenden

---

## 🔧 Installation (5 Minuten)

### Schritt 1: PC vorbereiten
```
PC ausschalten → Stromkabel ziehen → Gehäuse öffnen
```

### Schritt 2: Kabel am Mainboard finden
```
Power SW (2-pin) und Power LED (2-pin) identifizieren
```

### Schritt 3: PC Restarter anschließen

```
Gehäuse-Taster ←→ [PC Restarter] ←→ Mainboard
                        ↓
                    USB-Strom
```

1. Original Power SW & LED Kabel vom Mainboard abziehen
2. PC Restarter Kabel ans Mainboard anschließen
3. Original-Kabel an PC Restarter anschließen
4. USB-Kabel für Stromversorgung anschließen

### Schritt 4: Gehäuse schließen
```
Gerät befestigen → Gehäuse schließen → PC einschalten
```

---

## 📱 Ersteinrichtung (2 Minuten)

### 1. Mit WLAN verbinden
```
WLAN suchen: "PC-Restarter-XXXXXX"
Verbinden (kein Passwort)
```

### 2. Setup öffnen
```
Browser: http://192.168.4.1
```

### 3. Ihr WLAN konfigurieren
```
Netzwerk wählen → Passwort eingeben → Speichern
```

### 4. Fertig!
```
Gerät neu erreichbar unter:
http://pc-restarter.local
oder IP-Adresse aus Router
```

---

## 🎮 Bedienung

| Aktion | Web-Interface | MQTT |
|--------|---------------|------|
| Einschalten | 🔘 Power On | `power_on` |
| Ausschalten | 🔘 Power Off | `power_off` |
| Hart aus | 🔘 Force Off | `power_force_off` |
| Neustart | 🔘 Reset | `reset` |

---

## ❓ Probleme?

| Problem | Lösung |
|---------|--------|
| WLAN nicht gefunden | 10 Sek. stromlos machen |
| Falscher PC-Status | LED-Kabel Polarität tauschen |
| Werksreset nötig | Reset-Taste 10 Sek. beim Einschalten halten |

---

## 📞 Support

**Web:** https://[domain].de  
**E-Mail:** support@[domain].de

---

**Vollständige Anleitung:** https://[domain].de/docs

*CE ✓ | WEEE-Reg.-Nr.: DEXXXXX*
