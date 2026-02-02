# Technische Dokumentation (TD)
# Technical Documentation File (TDF)

**Gemäß / According to:** RED 2014/53/EU, EMV 2014/30/EU, RoHS 2011/65/EU

---

## Dokumentenübersicht / Document Overview

| Nr. | Dokument | Status |
|----|----------|--------|
| 1 | Allgemeine Produktbeschreibung | ⬜ |
| 2 | Design- und Fertigungszeichnungen | ⬜ |
| 3 | Schaltpläne | ⬜ |
| 4 | Stückliste (BOM) | ⬜ |
| 5 | Funktionsbeschreibung | ⬜ |
| 6 | Risikoanalyse | ⬜ |
| 7 | Angewandte Normen | ⬜ |
| 8 | Prüfberichte | ⬜ |
| 9 | Benutzerinformationen | ⬜ |
| 10 | EU-Konformitätserklärung | ⬜ |

---

## 1. Allgemeine Produktbeschreibung

### 1.1 Produktidentifikation

| Feld | Wert |
|------|------|
| Produktname | PC Restarter |
| Modell/Typ | PCR-001 |
| Hersteller | [Firmenname] |
| Adresse | [Adresse] |
| Herstellungsdatum | [Datum] |
| Hardware-Version | 1.0 |
| Firmware-Version | 1.0.0 |

### 1.2 Bestimmungsgemäße Verwendung

Der PC Restarter ist ein WiFi-fähiges Gerät zur Fernsteuerung des Ein-/Ausschalters von Desktop-Computern. Es wird zwischen dem Gehäuse-Taster und dem Mainboard installiert und ermöglicht:

- Fernsteuerung über Web-Interface
- Statusüberwachung der Power-LED
- Integration in Smart-Home-Systeme via MQTT
- Lokalen Betrieb ohne Cloud-Anbindung

### 1.3 Zielgruppe

- Privatanwender mit technischen Grundkenntnissen
- IT-Administratoren
- Smart-Home-Enthusiasten

### 1.4 Einsatzumgebung

- Innenräume
- Trockene Umgebung
- Temperatur: 0°C bis 40°C
- Luftfeuchtigkeit: 10-80% (nicht kondensierend)

---

## 2. Design- und Fertigungszeichnungen

### 2.1 Mechanische Zeichnungen

*[Hier PCB-Layout und Gehäusezeichnungen einfügen]*

- Leiterplattenabmessungen: XX x XX mm
- Höhe bestückt: XX mm
- Befestigungsbohrungen: Keine / [Position]

### 2.2 Leiterplattenlayout

*[Hier Gerber-Dateien referenzieren]*

- Anzahl Layer: 2
- Lagenaufbau: Top - FR4 - Bottom
- Kupferdicke: 35 µm
- Mindestleiterbahnbreite: 0.2 mm
- Mindestabstand: 0.2 mm

---

## 3. Schaltpläne

### 3.1 Blockschaltbild

```
┌─────────────────────────────────────────────────────────────┐
│                        PC RESTARTER                          │
│                                                              │
│  ┌──────────┐    ┌──────────┐    ┌──────────────────────┐  │
│  │   USB    │───►│   LDO    │───►│      ESP32-C3        │  │
│  │  5V In   │    │  3.3V    │    │     WROOM-02         │  │
│  └──────────┘    └──────────┘    │                      │  │
│       │                          │  ┌────────────────┐  │  │
│       ▼                          │  │   WiFi 2.4GHz  │  │  │
│  ┌──────────┐                    │  └────────────────┘  │  │
│  │   ESD    │                    │                      │  │
│  │ Schutz   │                    │  GPIO ──────────────┼──┼──►
│  └──────────┘                    └──────────────────────┘  │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              Optokoppler-Array (4x PC817C)            │   │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐  │   │
│  │  │ PWR_SW  │  │ PWR_LED │  │ PWR_SW  │  │ PWR_LED │  │   │
│  │  │  OUT    │  │   IN    │  │  PASS   │  │  PASS   │  │   │
│  │  └─────────┘  └─────────┘  └─────────┘  └─────────┘  │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 Detaillierter Schaltplan

*[Hier vollständigen Schaltplan als PDF referenzieren]*

**Referenz:** Schaltplan_PCR-001_v1.0.pdf

---

## 4. Stückliste (Bill of Materials)

| Pos | Bezeichnung | Wert | Gehäuse | Menge | Hersteller | Bestellnummer |
|-----|-------------|------|---------|-------|------------|---------------|
| U1 | ESP32-C3-WROOM-02 | - | Module | 1 | Espressif | ESP32-C3-WROOM-02 |
| U2 | LDO Regler | 3.3V | SOT-23-5 | 1 | [TBD] | [TBD] |
| U3-U6 | Optokoppler | PC817C | DIP-4 | 4 | Sharp | PC817C |
| U7 | ESD Schutz | D3V3X4B10LP | SOT-553 | 1 | Diodes Inc. | D3V3X4B10LP-7 |
| R1-R4 | Widerstand | 330Ω | 0402 | 4 | [TBD] | [TBD] |
| R5-R8 | Widerstand | 10kΩ | 0402 | 4 | [TBD] | [TBD] |
| C1-C4 | Kondensator | 100nF | 0402 | 4 | [TBD] | [TBD] |
| C5 | Kondensator | 10µF | 0805 | 1 | [TBD] | [TBD] |
| J1 | USB-A Stecker | - | THT | 1 | [TBD] | [TBD] |
| J2-J3 | Pin Header | 2x2 | 2.54mm | 2 | [TBD] | [TBD] |

---

## 5. Funktionsbeschreibung

### 5.1 Stromversorgung

- Eingang: 5V DC über USB-A
- Interne Spannung: 3.3V (LDO-geregelt)
- Stromaufnahme: typ. 80mA, max. 150mA

### 5.2 Mikrocontroller

Der ESP32-C3-WROOM-02 ist ein vorzertifiziertes Funkmodul mit:
- RISC-V Single-Core Prozessor @ 160MHz
- 400KB SRAM, 4MB Flash
- Integrierter WiFi 802.11 b/g/n
- CE/FCC/IC zertifiziert

### 5.3 Galvanische Trennung

Vier PC817C Optokoppler sorgen für galvanische Trennung:
- Power Button Ausgang (zum Mainboard)
- Power LED Eingang (vom Mainboard)
- Power Button Durchschleifung (vom Gehäuse)
- Power LED Durchschleifung (zum Gehäuse)

Isolation: 5000V RMS

### 5.4 ESD-Schutz

USB-Datenleitungen durch D3V3X4B10LP geschützt:
- ESD-Festigkeit: ±8kV Kontakt, ±15kV Luft

---

## 6. Risikoanalyse

### 6.1 Identifizierte Risiken

| Risiko | Schwere | Wahrscheinlichkeit | Maßnahme |
|--------|---------|---------------------|----------|
| Stromschlag | Hoch | Sehr gering | Niederspannung (5V), Isolation |
| Brand | Hoch | Sehr gering | Überstromschutz, Materialauswahl |
| EMV-Störungen | Mittel | Gering | EMV-gerechtes Design |
| Datenverlust (PC) | Mittel | Gering | Benutzerwarnung in Dokumentation |

### 6.2 Residualrisiken

Nach Anwendung aller Maßnahmen verbleiben keine unakzeptablen Risiken.

---

## 7. Angewandte Normen

### 7.1 EMV

| Norm | Titel | Anwendung |
|------|-------|-----------|
| EN 55032:2015+A1:2020 | Störaussendung | Vollständig |
| EN 55035:2017+A11:2020 | Störfestigkeit | Vollständig |

### 7.2 Funk (RED)

| Norm | Titel | Anwendung |
|------|-------|-----------|
| ETSI EN 300 328 V2.2.2 | 2.4 GHz Systeme | WiFi-Modul |
| ETSI EN 301 489-1 V2.2.3 | EMV für Funk | Allgemein |
| ETSI EN 301 489-17 V3.2.4 | EMV für Funk | Breitband |

### 7.3 Sicherheit

| Norm | Titel | Anwendung |
|------|-------|-----------|
| EN 62368-1:2020+A11:2020 | AV/IT Sicherheit | Vollständig |
| EN IEC 62311:2020 | EMF-Exposition | SAR nicht erforderlich (< 20mW) |

### 7.4 RoHS

| Norm | Titel | Anwendung |
|------|-------|-----------|
| EN 50581:2012 | RoHS Dokumentation | Lieferantenerklärungen |

---

## 8. Prüfberichte

### 8.1 Interne Prüfungen

| Prüfung | Datum | Ergebnis |
|---------|-------|----------|
| Funktionstest | [Datum] | Bestanden |
| ESD-Vortest | [Datum] | Bestanden |
| Temperaturtest | [Datum] | Bestanden |

### 8.2 Externe Prüfberichte

| Prüfbericht | Labor | Datum | Nummer |
|-------------|-------|-------|--------|
| EMV-Prüfung | [Labor] | [Datum] | [Nummer] |
| RED-Prüfung | [Labor] | [Datum] | [Nummer] |

*[Prüfberichte als Anlage beifügen]*

---

## 9. Benutzerinformationen

### 9.1 Mitgelieferte Dokumente

- Kurzanleitung (DE/EN)
- Sicherheitshinweise (DE/EN)

### 9.2 Online verfügbar

- Vollständige Bedienungsanleitung
- Installationsanleitung
- FAQ

---

## 10. EU-Konformitätserklärung

Siehe separates Dokument: `eu-konformitaetserklaerung.md`

---

## Änderungshistorie

| Version | Datum | Änderung | Autor |
|---------|-------|----------|-------|
| 1.0 | Feb 2026 | Erstausgabe | [Name] |

---

## Dokumentenaufbewahrung

Diese Technische Dokumentation wird aufbewahrt bei:

**[Firmenname]**  
[Adresse]  
Deutschland

Aufbewahrungsfrist: Mindestens 10 Jahre nach Inverkehrbringen des letzten Produkts.

---

*Dokumentnummer: TD-PCR-001-2026-01*
