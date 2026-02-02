# Installationsanleitung - PC Restarter

**Detaillierte Schritt-für-Schritt-Anleitung zur Hardware-Installation**

---

## Inhaltsverzeichnis

1. [Voraussetzungen](#1-voraussetzungen)
2. [Benötigtes Werkzeug](#2-benötigtes-werkzeug)
3. [Sicherheitsvorbereitungen](#3-sicherheitsvorbereitungen)
4. [Mainboard-Anschlüsse identifizieren](#4-mainboard-anschlüsse-identifizieren)
5. [Verkabelung](#5-verkabelung)
6. [Stromversorgung](#6-stromversorgung)
7. [Montage](#7-montage)
8. [Funktionstest](#8-funktionstest)
9. [Fehlerbehebung](#9-fehlerbehebung)

---

## 1. Voraussetzungen

### Kompatible Systeme

✅ Desktop-PCs mit Standard-ATX/Micro-ATX/Mini-ITX Mainboard  
✅ Workstations  
✅ Server mit Standard-Frontpanel-Anschlüssen  

### Nicht kompatibel

❌ Laptops / Notebooks  
❌ All-in-One PCs  
❌ Macs  
❌ Systeme ohne Standard-Frontpanel-Anschlüsse  

### Systemanforderungen

- Freier interner USB-Anschluss ODER externes USB-Netzteil
- 2-Pin Power SW Anschluss am Mainboard
- 2-Pin Power LED Anschluss am Mainboard
- WLAN-Netzwerk (2,4 GHz)

---

## 2. Benötigtes Werkzeug

- Kreuzschraubendreher (für Gehäuse)
- Optional: Taschenlampe
- Optional: Antistatik-Armband
- Optional: Kabelbinder

---

## 3. Sicherheitsvorbereitungen

### ⚠️ WICHTIG: Vor Beginn der Installation

1. **PC vollständig herunterfahren** (nicht Standby!)
2. **Netzstecker ziehen**
3. **Ein-/Ausschalter am Netzteil auf "0" stellen** (falls vorhanden)
4. **Power-Taste am Gehäuse drücken** (Restladung abbauen)
5. **Mindestens 30 Sekunden warten**

### Antistatik-Maßnahmen

- Geerdeten Gegenstand berühren (Heizungsrohr, Wasserhahn)
- Nicht auf Teppich arbeiten
- Elektronik nur an den Rändern anfassen

---

## 4. Mainboard-Anschlüsse identifizieren

### Position der Frontpanel-Header

Die Frontpanel-Anschlüsse befinden sich meist:
- Am unteren Rand des Mainboards
- In der Nähe der Gehäuse-Unterseite
- Beschriftet als "F_PANEL", "JFP1", "PANEL" o.ä.

### Anschluss-Belegung (Standard)

```
Typische Anordnung:
┌─────────────────────────────┐
│  PWR_LED+  PWR_LED-  │  GND  │
│  PWR_SW+   PWR_SW-   │ RESET │
│  HDD_LED+  HDD_LED-  │  GND  │
└─────────────────────────────┘
```

### Wichtige Anschlüsse für PC Restarter

| Anschluss | Pins | Funktion |
|-----------|------|----------|
| **POWER SW** | 2 | Einschalttaster (potentialfrei) |
| **POWER LED** | 2 | Betriebsanzeige (+/- beachten!) |

### Mainboard-Handbuch

Im Zweifelsfall:
- Mainboard-Handbuch konsultieren
- Hersteller-Website besuchen
- Beschriftung auf dem Mainboard lesen

---

## 5. Verkabelung

### 5.1 Original-Kabel identifizieren

Vom Gehäuse kommen typischerweise diese Kabel:

```
Gehäuse-Frontpanel:
├── POWER SW (2-pin) ← Benötigt
├── POWER LED (2-pin oder 3-pin) ← Benötigt
├── RESET SW (2-pin) ← Optional
├── HDD LED (2-pin) ← Nicht benötigt
└── SPEAKER (4-pin) ← Nicht benötigt
```

### 5.2 Original-Kabel abziehen

1. **POWER SW** Stecker vorsichtig vom Mainboard abziehen
2. **POWER LED** Stecker vorsichtig vom Mainboard abziehen

💡 **Tipp:** Foto von der Original-Belegung machen!

### 5.3 PC Restarter Kabel-Übersicht

Der PC Restarter hat folgende Anschlüsse:

```
PC Restarter:
┌──────────────────────────────────────┐
│                                      │
│  [MB_PWR_SW]  [MB_PWR_LED]  [USB]   │  ← Zum Mainboard
│                                      │
│  [CASE_PWR_SW] [CASE_PWR_LED]       │  ← Vom Gehäuse
│                                      │
└──────────────────────────────────────┘
```

### 5.4 Anschlussreihenfolge

**Schritt 1: Mainboard-seitige Verbindung**

```
PC Restarter           Mainboard
─────────────          ─────────────
MB_PWR_SW    ───────►  POWER SW Header
MB_PWR_LED   ───────►  POWER LED Header
```

**Schritt 2: Gehäuse-seitige Verbindung**

```
Gehäuse-Kabel         PC Restarter
─────────────          ─────────────
POWER SW     ───────►  CASE_PWR_SW
POWER LED    ───────►  CASE_PWR_LED
```

### 5.5 Polarität beachten

**POWER SW:** Polarität egal (potentialfreier Kontakt)

**POWER LED:** Polarität wichtig!
- `+` an `+` (oft rot oder farbiges Kabel)
- `-` an `-` (oft weiß oder schwarzes Kabel)

Bei falscher Polarität: LED-Status wird invertiert angezeigt

---

## 6. Stromversorgung

### Option A: Interner USB-Anschluss (empfohlen)

Viele Mainboards haben interne USB-Header:

```
Interner USB 2.0 Header (9-pin):
┌─────────────────┐
│ VCC  D-  D+  GND │ NC
│ VCC  D-  D+  GND │
└─────────────────┘
```

**Vorteil:** Saubere Kabelführung, Gerät startet mit Standby-Strom

### Option B: Externes USB-Netzteil

Bei fehlendem internen Anschluss:

1. USB-Kabel nach außen führen
2. An externes 5V USB-Netzteil anschließen
3. Netzteil dauerhaft mit Strom versorgen

**Hinweis:** Gerät muss VOR dem PC mit Strom versorgt werden!

### Option C: Always-On USB-Port (Rückseite)

Manche Mainboards haben Always-On USB-Ports:
- Im BIOS aktivieren ("USB Power in S5" o.ä.)
- USB-Kabel durch PCI-Slot-Blende führen

---

## 7. Montage

### Befestigungsoptionen

**Option 1: Doppelseitiges Klebeband**
- Auf flache, saubere Oberfläche kleben
- Nicht auf Kabeln oder heißen Bauteilen

**Option 2: Kabelbinder**
- An Laufwerksschacht oder Kabelmanagement befestigen

**Option 3: Frei liegend**
- Für Testzwecke möglich
- Auf rutschfeste Unterlage achten

### Platzierungshinweise

✅ Gut belüftete Stelle  
✅ Entfernt von Wärmequellen (CPU, GPU)  
✅ Mit Status-LED sichtbar (optional)  
❌ Nicht auf Mainboard legen  
❌ Nicht in Kabeln verstecken  

---

## 8. Funktionstest

### 8.1 Visuelle Prüfung

- [ ] Alle Kabel richtig angeschlossen
- [ ] Keine losen Verbindungen
- [ ] USB-Kabel angeschlossen
- [ ] Keine Kurzschlüsse sichtbar

### 8.2 Erster Start

1. PC-Gehäuse schließen
2. Netzstecker anschließen
3. Netzteil einschalten

**PC Restarter sollte jetzt starten:**
- Status-LED leuchtet
- WLAN "PC-Restarter-XXXXXX" erscheint

### 8.3 Funktionen testen

| Test | Erwartetes Ergebnis |
|------|---------------------|
| Gehäuse-Taster drücken | PC startet/stoppt normal |
| Web-Interface: Power On | PC startet |
| Web-Interface: Status | Zeigt korrekten PC-Status |

### 8.4 Ersteinrichtung

Weiter mit der Software-Ersteinrichtung:
→ Siehe Bedienungsanleitung, Kapitel 6

---

## 9. Fehlerbehebung

### PC Restarter startet nicht

| Symptom | Mögliche Ursache | Lösung |
|---------|------------------|--------|
| Keine LED | Kein Strom | USB-Verbindung prüfen |
| Keine LED | USB-Port deaktiviert | Anderen Port / externes Netzteil |

### PC startet nicht

| Symptom | Mögliche Ursache | Lösung |
|---------|------------------|--------|
| Taster ohne Funktion | Falsche Verkabelung | Anschlüsse prüfen |
| Taster ohne Funktion | Defektes Kabel | Kabel tauschen |

### LED-Status falsch

| Symptom | Mögliche Ursache | Lösung |
|---------|------------------|--------|
| Immer "Aus" | Falsche Polarität | + und - tauschen |
| Immer "An" | Falsche Polarität | + und - tauschen |
| Flackert | Lose Verbindung | Stecker festdrücken |

### WLAN nicht sichtbar

| Symptom | Mögliche Ursache | Lösung |
|---------|------------------|--------|
| Kein WLAN | Gerät nicht gestartet | Stromversorgung prüfen |
| Kein WLAN | Bereits konfiguriert | Reset durchführen |

---

## Verkabelungsdiagramm

```
                          ┌─────────────────┐
                          │   PC RESTARTER  │
                          │                 │
    ┌──────────┐          │  ┌───┐  ┌───┐  │          ┌──────────┐
    │ GEHÄUSE  │          │  │PWR│  │LED│  │          │MAINBOARD │
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
                          │ USB-Strom    │
                          │ (5V)         │
                          └──────────────┘
```

---

## Support

Bei Problemen mit der Installation:

**E-Mail:** support@[domain].de  
**Web:** https://[domain].de/support

---

*Stand: Februar 2026*
