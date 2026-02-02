# Bedienungsanleitung - PC Restarter

**Version:** 1.0  
**Stand:** Februar 2026

---

## Inhaltsverzeichnis

1. [Einleitung](#1-einleitung)
2. [Lieferumfang](#2-lieferumfang)
3. [Technische Daten](#3-technische-daten)
4. [Sicherheitshinweise](#4-sicherheitshinweise)
5. [Installation](#5-installation)
6. [Ersteinrichtung](#6-ersteinrichtung)
7. [Bedienung](#7-bedienung)
8. [Web-Interface](#8-web-interface)
9. [MQTT-Integration](#9-mqtt-integration)
10. [Fehlerbehebung](#10-fehlerbehebung)
11. [Wartung](#11-wartung)
12. [Entsorgung](#12-entsorgung)
13. [Kontakt & Support](#13-kontakt--support)

---

## 1. Einleitung

Vielen Dank, dass Sie sich für den PC Restarter entschieden haben!

Der PC Restarter ist ein intelligentes Gerät zur Fernsteuerung und Überwachung von Computern. Er ermöglicht:

- **Fernsteuerung:** Ein-/Ausschalten und Neustarten des PCs über WLAN
- **Statusüberwachung:** Echtzeit-Anzeige des PC-Betriebszustands
- **Smart-Home-Integration:** Steuerung über MQTT-kompatible Systeme (Home Assistant, etc.)
- **Lokale Kontrolle:** Funktioniert vollständig ohne Cloud-Anbindung

---

## 2. Lieferumfang

- 1x PC Restarter Gerät
- 1x USB-A Stromkabel
- 1x Verbindungskabel zum Mainboard (Power SW, Power LED)
- 1x Kurzanleitung
- 1x Sicherheitshinweise

---

## 3. Technische Daten

| Eigenschaft | Wert |
|-------------|------|
| Betriebsspannung | 5V DC (über USB) |
| Stromaufnahme | max. 150 mA |
| WLAN | 802.11 b/g/n (2,4 GHz) |
| Prozessor | ESP32-C3 |
| Abmessungen | XX x XX x XX mm |
| Gewicht | XX g |
| Betriebstemperatur | 0°C bis 40°C |
| Luftfeuchtigkeit | 10% - 80% (nicht kondensierend) |
| Schutzart | IP20 (nur Innenbereich) |

---

## 4. Sicherheitshinweise

⚠️ **WARNUNG:** Lesen Sie vor Inbetriebnahme alle Sicherheitshinweise!

- Nur für den Innenbereich geeignet
- Vor Feuchtigkeit und Nässe schützen
- Nicht öffnen – keine vom Benutzer wartbaren Teile im Inneren
- Nur mit dem mitgelieferten oder einem kompatiblen USB-Netzteil (5V/1A) betreiben
- Nicht in der Nähe von Wärmequellen aufstellen
- Bei Beschädigung nicht weiter verwenden

Vollständige Sicherheitshinweise siehe beiliegendes Dokument.

---

## 5. Installation

### 5.1 Vorbereitung

1. PC ausschalten und vom Stromnetz trennen
2. PC-Gehäuse öffnen
3. Mainboard-Anschlüsse identifizieren:
   - Power SW (Power-Taster)
   - Power LED (Power-LED)

### 5.2 Verkabelung

1. **Original-Kabel vom Gehäuse abziehen:**
   - Power SW Kabel vom Mainboard trennen
   - Power LED Kabel vom Mainboard trennen

2. **PC Restarter anschließen:**
   - PC Restarter zwischen Gehäuse-Taster und Mainboard schalten
   - Mainboard-seitige Stecker an das Mainboard anschließen
   - Gehäuse-seitige Stecker mit den Original-Kabeln verbinden

3. **Stromversorgung:**
   - USB-Kabel an einen dauerhaft aktiven USB-Port oder externes Netzteil anschließen

### 5.3 Montage

- Das Gerät kann mit doppelseitigem Klebeband im Gehäuse befestigt werden
- Auf ausreichende Luftzirkulation achten

---

## 6. Ersteinrichtung

### 6.1 Access Point Modus

Bei der ersten Inbetriebnahme startet das Gerät im Access Point Modus:

1. WLAN-Netzwerk suchen: `PC-Restarter-XXXXXX`
2. Mit dem Netzwerk verbinden (kein Passwort erforderlich)
3. Browser öffnen und `http://192.168.4.1` aufrufen
4. Setup-Assistent folgen

### 6.2 WLAN konfigurieren

1. Ihr WLAN-Netzwerk aus der Liste wählen
2. WLAN-Passwort eingeben
3. Optionale Einstellungen vornehmen
4. "Speichern" klicken

Das Gerät startet neu und verbindet sich mit Ihrem WLAN.

### 6.3 IP-Adresse finden

Nach erfolgreicher Verbindung können Sie die IP-Adresse finden über:

- Ihren Router (DHCP-Client-Liste)
- mDNS: `http://pc-restarter.local`

---

## 7. Bedienung

### 7.1 Physischer Taster

Der originale Gehäuse-Taster funktioniert weiterhin wie gewohnt.

### 7.2 Web-Interface

Über das Web-Interface können Sie:

- PC-Status einsehen (An/Aus)
- PC einschalten
- PC ausschalten (kurzer Druck)
- PC hart ausschalten (langer Druck)
- PC neustarten

### 7.3 MQTT

Bei konfiguriertem MQTT-Broker werden Status-Updates gesendet und Befehle empfangen.

---

## 8. Web-Interface

### 8.1 Dashboard

Das Dashboard zeigt:

- Aktueller PC-Status (LED-Anzeige)
- Schnellaktionen (Power, Reset)
- Systeminformationen

### 8.2 Einstellungen

- **WLAN:** Netzwerkeinstellungen ändern
- **MQTT:** Broker-Konfiguration
- **System:** Geräteinfo, Neustart, Werksreset

---

## 9. MQTT-Integration

### 9.1 Topics

| Topic | Beschreibung |
|-------|-------------|
| `restarter/status` | PC-Status (on/off) |
| `restarter/command` | Befehle senden |
| `restarter/available` | Online-Status |

### 9.2 Befehle

| Befehl | Aktion |
|--------|--------|
| `power_on` | PC einschalten |
| `power_off` | PC ausschalten |
| `power_force_off` | PC hart ausschalten |
| `reset` | PC neustarten |

### 9.3 Home Assistant

Beispiel-Konfiguration für Home Assistant:

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

## 10. Fehlerbehebung

| Problem | Lösung |
|---------|--------|
| Gerät nicht erreichbar | WLAN-Verbindung prüfen, Router neustarten |
| Access Point nicht sichtbar | Gerät stromlos machen, 10 Sekunden warten |
| PC-Status falsch | LED-Kabel prüfen, Polarität tauschen |
| MQTT verbindet nicht | Broker-Adresse und Zugangsdaten prüfen |

### Werksreset

Falls das Gerät nicht mehr erreichbar ist:

1. Gerät stromlos machen
2. Reset-Taster gedrückt halten
3. Strom wieder anschließen
4. 10 Sekunden halten
5. Gerät startet im Access Point Modus

---

## 11. Wartung

Das Gerät ist wartungsfrei. 

- Regelmäßig auf Firmware-Updates prüfen
- Bei Verschmutzung mit trockenem Tuch reinigen
- Nicht mit Wasser oder Reinigungsmitteln behandeln

---

## 12. Entsorgung

♻️ Dieses Produkt darf nicht über den Hausmüll entsorgt werden.

Gemäß WEEE-Richtlinie 2012/19/EU muss das Gerät fachgerecht entsorgt werden:

- Kommunale Sammelstellen für Elektro-Altgeräte
- Rückgabe an den Händler

---

## 13. Kontakt & Support

**Hersteller:**  
[Firmenname]  
[Adresse]  
[PLZ Ort]  
Deutschland

**E-Mail:** support@[domain].de  
**Web:** https://[domain].de

---

*© 2026 [Firmenname]. Alle Rechte vorbehalten.*
