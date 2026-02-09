<img src="../../images/logo.svg" alt="Logo" width="120" />

# PC Restarter  
## Sicherheitshinweise, Installation & Bedienungsanleitung  
### Vollständige Fassung (Vorder- und Rückseite)

---

| **Produktname** | PC Restarter |
|-----------------|--------------|
| **Produktart** | Netzwerkfähiges Steuergerät für Desktop-PCs |
| **Version** | 1.0 |
| **Stand** | Februar 2026 |  

**Layout-Hinweis:**  
Beidseitig auf DIN A5 oder DIN A4 drucken, mittig falten.

---

## VORDERSEITE  
# Wichtige Sicherheitshinweise

---

## ⚠️ Allgemeine Sicherheitshinweise

- **Nur für den Einsatz in trockenen Innenräumen geeignet.**
- **Vor Installation oder Wartung den PC vollständig vom Stromnetz trennen.**
- **Gefahr von Kurzschluss, Stromschlag oder Geräteschäden bei unsachgemäßer Installation.**
- **Kein Spielzeug – außerhalb der Reichweite von Kindern aufbewahren.**

---

## Bestimmungsgemäßer Gebrauch

Der PC Restarter ist ein elektronisches Steuergerät zur **Fernsteuerung und Statusüberwachung von Desktop-PCs** über den Power-Switch- und Power-LED-Anschluss des Mainboards.

Das Gerät ist ausschließlich bestimmt für:
- Desktop-PCs mit Standard-Frontpanel-Anschlüssen  
- Private und gewerbliche Nutzung in Innenräumen  

**Eine andere Verwendung gilt als nicht bestimmungsgemäß** und kann zu Sachschäden, Datenverlust oder Sicherheitsrisiken führen.

---

## Nicht bestimmungsgemäßer Gebrauch

❌ Einsatz in Laptops, Notebooks, All-in-One-PCs oder Macs  
❌ Betrieb im Außenbereich oder in feuchter Umgebung  
❌ Eigenmächtige Umbauten oder Öffnen des Geräts  
❌ Betrieb mit Spannungen > 5 V DC  

---

## Elektrische Sicherheit

- Nur mit **5 V DC** betreiben (USB-Netzteil oder USB-Port)
- Maximalstrom: **1 A**
- Keine beschädigten Kabel oder Stecker verwenden
- Gerät nicht in der Nähe von Hitzequellen betreiben
- Gerät nicht abdecken (Überhitzungsgefahr)

---

## Haftungsausschluss

Der Einbau und Betrieb erfolgen auf eigene Verantwortung.  
Der Hersteller übernimmt **keine Haftung für Schäden an Hardware, Software oder Daten**, die durch unsachgemäße Installation, Fehlbedienung oder nicht bestimmungsgemäßen Gebrauch entstehen.

---

## RÜCKSEITE  
# Installation & Bedienungsanleitung

---

## 1. Produktbeschreibung

Der **PC Restarter** ist ein netzwerkfähiges Steuergerät auf Basis eines ESP32-Controllers.  
Er ermöglicht das **Ein-, Aus- und Neustarten eines Desktop-PCs**, sowie die **Statusüberwachung** über WLAN – vollständig **lokal und ohne Cloud-Zwang**.

Funktionen:
- WLAN-basierte Fernsteuerung
- Echtzeit-Status über Power-LED
- Web-Interface
- MQTT-Integration (z. B. Home Assistant)
- Weiterverwendung des Gehäuse-Power-Tas­ters

---

## 2. Lieferumfang

| Menge | Artikel |
|-------|---------|
| 1× | PC Restarter Gerät |
| 1× | USB-A Stromkabel |
| 1× | Mainboard-Anschlusskabel (Power SW / Power LED) |
| 1× | Kurzanleitung |
| 1× | Sicherheitshinweise |  

---

## 3. Technische Daten

| Eigenschaft | Wert |
|------------|------|
| Betriebsspannung | 5 V DC (USB) |
| Stromaufnahme | max. 150 mA |
| WLAN | IEEE 802.11 b/g/n (2,4 GHz) |
| Prozessor | ESP32-C3 |
| Schutzart | IP20 |
| Betriebstemperatur | 0 °C – 40 °C |
| Luftfeuchtigkeit | 10 – 80 % (nicht kondensierend) |
| Abmessungen | XX × XX × XX mm |
| Gewicht | XX g |

---

## 4. Kompatibilität

| Status | Systeme |
|--------|---------|
| ✅ **Unterstützt** | Desktop-PCs mit ATX / Micro-ATX / Mini-ITX |
| ✅ **Unterstützt** | Systeme mit Standard-Frontpanel-Header |
| ❌ **Nicht unterstützt** | Laptops / Notebooks |
| ❌ **Nicht unterstützt** | All-in-One-PCs |
| ❌ **Nicht unterstützt** | Apple Macs |
| ❌ **Nicht unterstützt** | Systeme ohne Power-SW-Header |

---

## 5. Sicherheitsvorbereitungen

**Vor der Installation unbedingt durchführen:**

1. PC vollständig herunterfahren  
2. Netzstecker ziehen  
3. Netzteil-Schalter auf „0“ stellen  
4. Power-Taste 5 Sekunden drücken  
5. Mindestens 30 Sekunden warten  

Antistatik-Hinweise:
- Geerdeten Gegenstand berühren
- Keine Arbeiten auf Teppichboden
- Elektronik nur an den Rändern anfassen

---

## 6. Anschluss & Verkabelung

Der PC Restarter wird **zwischen Gehäuse-Taster und Mainboard** geschaltet.

| Anschluss | Bezeichnung | Beschreibung |
|-----------|-------------|--------------|
| **MB_PWR_SW** | Mainboard Power-Switch | Anschluss am Mainboard |
| **MB_PWR_LED** | Mainboard Power-LED | Anschluss am Mainboard (Polarität beachten!) |
| **CASE_PWR_SW** | Gehäuse-Power-Taster | Anschluss am Gehäuse-Taster |
| **CASE_PWR_LED** | Gehäuse-Power-LED | Anschluss am Gehäuse-LED (Polarität beachten!) |

**Hinweis:**  
Beim Power-Switch ist die Polarität egal.  
Bei der Power-LED **muss + und − korrekt angeschlossen sein**.

---

## 7. Stromversorgung

| Option | Beschreibung | Vorteile |
|--------|--------------|----------|
| **A (empfohlen)** | Interner USB-Header | Saubere Kabelführung, Versorgung auch im Standby |
| **B** | Externes USB-Netzteil | 5 V / 1 A, Dauerstrom erforderlich |

---

## 8. Ersteinrichtung

| Schritt | Aktion |
|---------|--------|
| 1 | WLAN „**PC-Restarter-XXXXXX**" verbinden |
| 2 | Browser öffnen: `http://192.168.4.1` |
| 3 | WLAN auswählen, Passwort eingeben |
| 4 | Einstellungen speichern |

**Danach erreichbar über:**
- Router-IP oder
- `http://pc-restarter.local`

---

## 9. Bedienung

| Methode | Funktionen |
|---------|------------|
| **Gehäuse-Taster** | Funktioniert wie gewohnt |
| **Web-Interface** | Ein / Aus / Neustart / Status |
| **MQTT** | Fernsteuerung & Integration in Smart-Home-Systeme |

---

## 10. Wartung & Reinigung

- Gerät ist wartungsfrei
- Reinigung nur mit trockenem Tuch
- Keine Flüssigkeiten oder Reinigungsmittel

---

## 11. Entsorgung

<img src="../../../images/WEEE_2.svg" alt="WEEE Symbol" width="40" />

**Nicht im Hausmüll entsorgen!**  

Dieses Gerät unterliegt der **WEEE-Richtlinie 2012/19/EU**.  
Entsorgung über kommunale Sammelstellen oder Rückgabe beim Händler.

---

## 12. Konformität & Rechtliches

<img src="../../../images/CE.svg" alt="CE Marking" width="40" />

### EU-Konformitätserklärung

Dieses Produkt entspricht den einschlägigen EU-Richtlinien:

| Richtlinie | Beschreibung |
|------------|--------------|
| 2014/53/EU | RED – Funkanlagenrichtlinie |
| 2011/65/EU | RoHS |
| 2014/30/EU | EMV |

Die vollständige EU-Konformitätserklärung ist erhältlich unter:
`https://[domain].de/conformity`

---

## 13. Herstellerangaben

**Hersteller / Inverkehrbringer:**  
[Firmenname]  
[Straße, Hausnummer]  
[PLZ Ort]  
Deutschland  

**E-Mail:** support@[domain].de  
**Web:** https://[domain].de  

---

## Fußzeile

<img src="../../../images/CE.svg" alt="CE Marking" width="30" /> <img src="../../../images/WEEE_2.svg" alt="WEEE Symbol" width="30" /> IP20  

© 2026 [Firmenname] – Alle Rechte vorbehalten  
Stand: Februar 2026
