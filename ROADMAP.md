# Markteinführungs-Roadmap - Deutschland/EU

Roadmap zur Markteinführung des PC Restarter Geräts in Deutschland und der Europäischen Union.

---

## Inhaltsverzeichnis

1. [Regulatorische Konformität (Pflicht)](#1-regulatorische-konformität-pflicht)
2. [Hardware-Verbesserungen](#2-hardware-verbesserungen)
3. [Firmware-Verbesserungen](#3-firmware-verbesserungen)
4. [Sicherheitshärtung](#4-sicherheitshärtung)
5. [Fertigung & Lieferkette](#5-fertigung--lieferkette)
6. [Dokumentation & Lokalisierung](#6-dokumentation--lokalisierung)
7. [Geschäftliches & Rechtliches](#7-geschäftliches--rechtliches)
8. [Markteinführung](#8-markteinführung)

---

## 1. Regulatorische Konformität (Pflicht)

### CE-Kennzeichnung (Erforderlich für EU-Markt)

Die CE-Kennzeichnung ist für den Verkauf elektronischer Geräte in der EU verpflichtend.


| Richtlinie                    | Beschreibung                                          | Status                        |
| ------------------------------- | ------------------------------------------------------- | ------------------------------- |
| **EMV-Richtlinie 2014/30/EU** | Elektromagnetische Verträglichkeit                   | ⬜ Ausstehend                 |
| **RoHS 2011/65/EU**           | Beschränkung gefährlicher Stoffe                    | ⬜ Ausstehend                 |
| **RED 2014/53/EU**            | Funkanlagenrichtlinie (für WLAN)                     | ⬜ Ausstehend                 |
| **WEEE 2012/19/EU**           | Elektro- und Elektronik-Altgeräte                    | ⬜ Bei Behörden registrieren |

#### Maßnahmen:

- [ ] Benannte Stelle für Prüfungen auswählen (z.B. TÜV, DEKRA, VDE)
- [ ] EMV-Vorprüfung durchführen
- [ ] RED-Prüfung für WLAN-Modul durchführen
- [ ] Technische Dokumentation (TD) erstellen
- [ ] EU-Konformitätserklärung ausstellen
- [ ] CE-Kennzeichnung auf Produkt und Verpackung anbringen

#### Geschätzte Kosten: 3.000 € - 15.000 € (je nach Komplexität)

#### Geschätzte Dauer: 2-4 Monate

---

### Deutschland-spezifische Anforderungen


| Anforderung                           | Beschreibung                                                  | Status                  |
| --------------------------------------- | --------------------------------------------------------------- | ------------------------- |
| **WEEE-Registrierung**                | Registrierung bei stiftung ear (Elektro-Altgeräte Register)  | ⬜ Ausstehend           |
| **Verpackungsgesetz (VerpackG)**      | Registrierung bei LUCID, Verpackungslizenzierung              | ⬜ Ausstehend           |
| **Produktsicherheitsgesetz (ProdSG)** | Allgemeine Produktsicherheitskonformität                     | ⬜ Ausstehend           |


## 2. Hardware-Verbesserungen

### Sicherheit & Zuverlässigkeit


| Punkt                          | Priorität | Status                                                                     |
| -------------------------------- | ------------ | ---------------------------------------------------------------------------- |
| **ESD-Schutz**                 | Hoch       | ✅ D3V3X4B10LP-7 an USB-Leitungen                                          |
| **Überstromschutz**           | Hoch       | ⬜ Selbstrückstellende Sicherungen (PTC) am 5V-Eingang hinzufügen        |
| **Verpolungsschutz**           | Hoch       | ⬜ Schutzdiode an VBUS hinzufügen                                         |
| **Geeignetes Gehäuse**        | Hoch       | ⬜ Gehäuse mit IP20+ Schutzart entwerfen                                  |
| **Wärmemanagement**           | Mittel     | ✅ Kompakte Bauform, stromsparendes Design                                 |
| **Optokoppler-Isolation**      | Hoch       | ✅ 4x PC817C für LED-Erfassung + Tastensteuerung                          |
| **Relais für Last ausgelegt** | Hoch       | ✅ Verwendung von Optokopplern stattdessen (PC817C) - galvanische Trennung |

### Produktionsoptimierung


| Punkt                               | Priorität | Status                                            |
| ------------------------------------- | ------------ | --------------------------------------------------- |
| **Eigenes PCB-Design**              | Hoch       | ✅ Einzelplatinen-Lösung entworfen               |
| **Bauteilauswahl**                  | Hoch       | ✅ Standardbauteile ausgewählt                   |
| **Fertigungsgerechte Konstruktion** | Mittel     | ✅ SMT-Bestückung optimiert                      |
| **Testbarkeit**                     | Mittel     | ⬜ Testpunkte hinzufügen, ICT in Betracht ziehen |
| **Beschriftungsfläche**            | Mittel     | ✅ Platine hat Branding-Bereich                   |

### Empfohlene Zertifizierungen für Komponenten

- ESP32-C3-Modul: Vorzertifiziertes Modul verwenden (z.B. ESP32-C3-WROOM-02)
- Prüfen, dass Modul RED/FCC/IC-Zertifizierungen hat

---

## 3. Firmware-Verbesserungen

### Stabilität & Zuverlässigkeit


| Punkt                        | Priorität | Status                                            |
| ------------------------------ | ------------ | --------------------------------------------------- |
| **Watchdog-Timer**           | Hoch       | ⬜ Hardware-WDT implementieren                    |
| **Fehlerbehandlung**         | Hoch       | ⬜ Robuste Behandlung aller Fehler                |
| **Speicherleck-Prävention** | Hoch       | ⬜ Heap-Nutzung prüfen, Überwachung hinzufügen |
| **Power-on-Selbsttest**      | Mittel     | ⬜ Hardware beim Booten verifizieren              |
| **Logging-System**           | Mittel     | ⬜ Persistente Fehlerprotokollierung              |
| **Absturzbericht**           | Mittel     | ⬜ Absturzinfos in NVS speichern                  |

### OTA-Updates


| Punkt                       | Priorität | Status                                       |
| ----------------------------- | ------------ | ---------------------------------------------- |
| **Sichere OTA-Updates**     | Hoch       | ⬜ Signierte Firmware-Updates implementieren |
| **Rollback-Unterstützung** | Hoch       | ⬜ Dual-Partition mit Fallback               |
| **Update-Server**           | Mittel     | ⬜ Update-Infrastruktur einrichten           |
| **Versionsprüfung**        | Mittel     | ⬜ Regelmäßig nach Updates prüfen         |
| **Delta-Updates**           | Niedrig    | ⬜ Update-Größe reduzieren                 |

### Qualitätssicherung


| Punkt                 | Priorität | Status                                  |
| ----------------------- | ------------ | ----------------------------------------- |
| **Unit-Tests**        | Hoch       | ⬜ Kritische Funktionen testen          |
| **Integrationstests** | Mittel     | ⬜ API-Endpunkt-Tests                   |
| **Stresstests**       | Mittel     | ⬜ Langzeit-Stabilitätstests           |
| **Speicheranalyse**   | Mittel     | ✅ Heap-Statistiken in Web-UI angezeigt |

---

## 4. Sicherheitshärtung

### Netzwerksicherheit


| Punkt                        | Priorität | Status                                       |
| ------------------------------ | ------------ | ---------------------------------------------- |
| **HTTPS-Unterstützung**     | Hoch       | ⬜ TLS für Web-Interface                    |
| **WSS (Sicherer WebSocket)** | Hoch       | ⬜ TLS für WebSocket                        |
| **MQTT TLS**                 | Hoch       | ⬜ MQTTS implementieren                      |
| **Passwort-Hashing**         | Hoch       | ⬜ WLAN-Passwort nicht im Klartext speichern |
| **API-Authentifizierung**    | Hoch       | ⬜ Token-basierte Auth für API hinzufügen  |
| **Rate Limiting**            | Mittel     | ⬜ Brute-Force-Angriffe verhindern           |
| **CSRF-Schutz**              | Mittel     | ⬜ CSRF-Tokens zu Formularen hinzufügen     |

### Zugriffskontrolle


| Punkt                    | Priorität | Status                                          |
| -------------------------- | ------------ | ------------------------------------------------- |
| **Admin-Passwort**       | Hoch       | ⬜ Passwort für sensible Aktionen erforderlich |
| **Sitzungsverwaltung**   | Hoch       | ⬜ Ordentliche Sitzungen implementieren         |
| **AP-Modus-Passwort**    | Mittel     | ⬜ Setup-AP absichern                           |
| **Physische Sicherheit** | Mittel     | ⬜ JTAG in Produktion deaktivieren              |

### Datenschutz (DSGVO)


| Punkt                         | Priorität | Status                                       |
| ------------------------------- | ------------ | ---------------------------------------------- |
| **Datenminimierung**          | Hoch       | ✅ Speichert nur WLAN/MQTT-Zugangsdaten      |
| **Datenschutzerklärung**     | Hoch       | ⬜ Dokumentieren welche Daten erhoben werden |
| **Datenlöschung**            | Hoch       | ✅ Werksreset löscht alle NVS-Daten         |
| **Keine Cloud-Abhängigkeit** | Mittel     | ✅ Gerät funktioniert 100% lokal            |

---

## 5. Fertigung & Lieferkette


### Alle Kosten pro Stück


| Kostenposten                             | Geschätzte Kosten pro Stück            |
| ------------------------------------------ | -------------------------------------- |
| PCB + Bestückung                        | 5 €                                   |
| Prüfung                                 | 1 €                                   |
| Verpackung                               | 2 €                                   |
| Zertifizierung (amortisiert)             | 1 €                                   |
| Marketing / Vertrieb                     | 1 €                                   |
| Support                                  | 1 €                                   |
| Rücklagen / Gewährleistung             | 1 €                                   |
| **Gesamtkosten pro Stück**              | **13 €** (zzgl. anteilige Fixkosten)  |
| **+ Marge (30%)**                        | **3,90 €**                            |
| **+ Shopify-Gebühr (2%)**               | **0,34 €**                            |
| **Verkaufspreis (Shopify, inkl. Marge, ohne MwSt)** | **17,24 €**                 |
| **+ MwSt (19%)**                        | **3,28 €**                            |
| **Verkaufspreis (Shopify, inkl. MwSt)**  | **20,52 €**                           |


### Lager & Logistik


| Punkt                   | Priorität | Status                                  |
| ------------------------- | ------------ | ----------------------------------------- |
| **Verpackungsdesign**   | Mittel     | ⬜ Verkaufsfertige Verpackung gestalten |
| **Seriennummernsystem** | Hoch       | ⬜ Eindeutige Geräteidentifikation     |
| **Bestandsverwaltung**  | Mittel     | ⬜ Lagerbestände verfolgen             |

---

## 6. Dokumentation & Lokalisierung

### Erforderliche Dokumentation


| Dokument                    | Sprache | Status                                |
| ----------------------------- | --------- | --------------------------------------- |
| **Bedienungsanleitung**     | DE, EN  | ⬜ Professionelle Anleitung erstellen |
| **Kurzanleitung**           | DE, EN  | ⬜ Gedruckte Beilage                  |
| **Sicherheitshinweise**     | DE, EN  | ⬜ Erforderliche Warnungen            |
| **Konformitätserklärung** | DE, EN  | ⬜ Rechtliches Dokument               |
| **Technisches Datenblatt**  | EN      | ⬜ Für B2B-Kunden                    |
| **Installationsanleitung**  | DE, EN  | ⬜ Mit Diagrammen                     |

### Web-Interface-Lokalisierung


| Punkt                     | Priorität | Status                          |
| --------------------------- | ------------ | --------------------------------- |
| **Deutsche Übersetzung** | Hoch       | ⬜ Alle UI-Texte übersetzen    |
| **Englisch (Standard)**   | Hoch       | ✅ Aktuelle Implementierung     |
| **Sprachauswahl**         | Mittel     | ⬜ Sprachumschalter hinzufügen |
| **RTL-Unterstützung**    | Niedrig    | ⬜ Für zukünftige Märkte     |

### Support-Ressourcen


| Punkt                        | Priorität | Status                       |
| ------------------------------ | ------------ | ------------------------------ |
| **FAQ-Seite**                | Mittel     | ⬜ Häufige Fragen           |
| **Fehlerbehebungsanleitung** | Mittel     | ⬜ Häufige Probleme         |
| **Support-E-Mail/System**    | Hoch       | ⬜ Kundensupport-Kanal       |

---

## 7. Geschäftliches & Rechtliches

### Unternehmenseinrichtung


| Punkt                          | Priorität | Status                             |
| -------------------------------- | ------------ | ------------------------------------ |
| **Rechtsform**                 | Hoch       | ✅ GmbH, UG oder Einzelunternehmer |
| **Gewerbeanmeldung**           | Hoch       | ✅ Gewerbeanmeldung                |
| **Umsatzsteuer-Registrierung** | Hoch       | ✅ Umsatzsteuer-IdNr               |
| **Betriebsversicherung**       | Hoch       | ⬜ Produkthaftpflichtversicherung  |

### Rechtliche Dokumente


| Dokument                             | Priorität | Status                          |
| -------------------------------------- | ------------ | --------------------------------- |
| **Allgemeine Geschäftsbedingungen** | Hoch       | ⬜ AGB auf Deutsch              |
| **Datenschutzerklärung**            | Hoch       | ⬜ Datenschutzerklärung        |
| **Widerrufsbelehrung**               | Hoch       | ⬜ 14-tägiges Widerrufsrecht   |
| **Garantiebedingungen**              | Hoch       | ⬜ Mindestens 2 Jahre in der EU |
| **Impressum**                        | Hoch       | ⬜ Für Website erforderlich    |


### Versicherungen


| Art                     | Empfohlene Deckung   | Status   |
| ------------------------- | ---------------------- | --------- |
| **Produkthaftpflicht**  | 1-5 Millionen €     | ⬜        |
| **Betriebshaftpflicht** | 1-3 Millionen €     | ⬜        |


---

## 8. Markteinführung

### Vertriebskanäle


| Kanal                           | Priorität | Status                        |
| --------------------------------- | ------------ | ------------------------------- |
| **Eigene Website/Shop**         | Hoch       | ⬜ WooCommerce, Shopify, etc. |
| **Amazon.de**                   | Hoch       | ⬜ FBA oder Eigenversand      |
| **eBay.de**                     | Mittel     | ⬜ Zusätzliche Reichweite    |
| **B2B-Direktvertrieb**          | Mittel     | ⬜ IT-Dienstleister           |
| **Einzelhandel (Conrad, etc.)** | Niedrig    | ⬜ Erfordert Volumen          |


### Marketing


| Kanal                  | Priorität | Status                        |
| ------------------------ | ------------ | ------------------------------- |
| **Produkt-Website**    | Hoch       | ⬜ Landing Page               |
| **SEO**                | Hoch       | ⬜ Suchmaschinenoptimierung   |
| **Tech-Blogs/Reviews** | Mittel     | ⬜ Testgeräte versenden      |
| **Reddit/Foren**       | Mittel     | ⬜ r/homelab, r/homeassistant |
| **Social Media**       | Niedrig    | ⬜ Twitter/X, LinkedIn        |

---


## Budgetschätzung


| Kategorie                              | Geschätzte Kosten   |
| ---------------------------------------- | ---------------------- |
| CE-Zertifizierung                      | 5.000-15.000 €      |
| Rechtliches/Geschäftsaufbau           | 2.000-5.000 €       |
| Website/Marketing                      | 2.000-5.000 €       |
| Erste Produktionsserie (100 Einheiten) | 3.000-5.000 €       |
| Versicherung (jährlich)               | 500-2.000 €         |
| Registrierungen (WEEE, LUCID, etc.)    | 500-1.500 €         |
| Puffer (20%)                           | 2.500-6.500 €       |
| **Gesamte Anfangsinvestition**         | **15.000-40.000 €** |

---

*Zuletzt aktualisiert: Februar 2026*
