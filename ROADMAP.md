# Commercial Release Roadmap - Germany/EU

Roadmap for bringing the PC Restarter device to market in Germany and the European Union.

---

## Table of Contents

1. [Regulatory Compliance (Mandatory)](#1-regulatory-compliance-mandatory)
2. [Hardware Improvements](#2-hardware-improvements)
3. [Firmware Improvements](#3-firmware-improvements)
4. [Security Hardening](#4-security-hardening)
5. [Manufacturing & Supply Chain](#5-manufacturing--supply-chain)
6. [Documentation & Localization](#6-documentation--localization)
7. [Business & Legal](#7-business--legal)
8. [Go-to-Market](#8-go-to-market)

---

## 1. Regulatory Compliance (Mandatory)

### CE Marking (Required for EU Market)

The CE mark is mandatory for selling electronic devices in the EU.

| Directive | Description | Status |
|-----------|-------------|--------|
| **EMC Directive 2014/30/EU** | Electromagnetic compatibility | ⬜ Pending |
| **LVD 2014/35/EU** | Low Voltage Directive (if >50V AC or >75V DC) | ⬜ Check if applicable |
| **RoHS 2011/65/EU** | Restriction of Hazardous Substances | ⬜ Pending |
| **RED 2014/53/EU** | Radio Equipment Directive (for WiFi) | ⬜ Pending |
| **WEEE 2012/19/EU** | Waste Electrical and Electronic Equipment | ⬜ Register with authorities |

#### Action Items:
- [ ] Select a Notified Body for testing (e.g., TÜV, DEKRA, VDE)
- [ ] Conduct EMC pre-compliance testing
- [ ] Conduct RED testing for WiFi module
- [ ] Create Technical Documentation File (TDF)
- [ ] Issue EU Declaration of Conformity (DoC)
- [ ] Apply CE marking to product and packaging

#### Estimated Cost: €3,000 - €15,000 (depending on complexity)
#### Estimated Time: 2-4 months

---

### German-Specific Requirements

| Requirement | Description | Status |
|-------------|-------------|--------|
| **WEEE Registration** | Register with stiftung ear (elektro-altgeräte register) | ⬜ Pending |
| **Packaging Act (VerpackG)** | Register with LUCID, license packaging | ⬜ Pending |
| **Battery Act (BattG)** | If device contains battery, register with stiftung ear | ⬜ Check if applicable |
| **Product Safety Act (ProdSG)** | General product safety compliance | ⬜ Pending |

#### Action Items:
- [ ] Register at [LUCID](https://lucid.verpackungsregister.org/) for packaging
- [ ] Register at [stiftung ear](https://www.stiftung-ear.de/) for WEEE
- [ ] Calculate annual packaging volumes for licensing
- [ ] Choose a dual system provider (e.g., Grüner Punkt, Landbell)

---

## 2. Hardware Improvements

### Safety & Reliability

| Item | Priority | Status |
|------|----------|--------|
| **ESD Protection** | High | ✅ D3V3X4B10LP-7 on USB lines |
| **Overcurrent Protection** | High | ⬜ Add resettable fuses (PTC) on 5V input |
| **Reverse Polarity Protection** | High | ⬜ Add protection diode on VBUS |
| **Proper Enclosure** | High | ⬜ Design IP20+ rated enclosure |
| **Strain Relief** | Medium | ⬜ Cable strain relief for connections |
| **Thermal Management** | Medium | ✅ Small form factor, low power design |
| **Optocoupler Isolation** | High | ✅ 4x PC817C for LED sensing + button control |
| **Relay Rated for Load** | High | ✅ Using optocouplers instead (PC817C) - galvanic isolation |

### Production Optimization

| Item | Priority | Status |
|------|----------|--------|
| **Custom PCB Design** | High | ✅ Single-board solution designed |
| **Component Selection** | High | ✅ Standard components selected |
| **Design for Manufacturability** | Medium | ✅ SMT assembly optimized |
| **Testability** | Medium | ⬜ Add test points, consider ICT |
| **Labeling Space** | Medium | ✅ Board has branding area |

### Recommended Certifications for Components

- ESP32-C3 module: Use pre-certified module (e.g., ESP32-C3-WROOM-02)
- Verify module has RED/FCC/IC certifications

---

## 3. Firmware Improvements

### Stability & Reliability

| Item | Priority | Status |
|------|----------|--------|
| **Watchdog Timer** | High | ⬜ Implement hardware WDT |
| **Error Recovery** | High | ⬜ Graceful handling of all failures |
| **Memory Leak Prevention** | High | ⬜ Audit heap usage, add monitoring |
| **Power-on Self Test** | Medium | ⬜ Verify hardware on boot |
| **Logging System** | Medium | ⬜ Persistent error logging |
| **Crash Reporting** | Medium | ⬜ Store crash info in NVS |

### OTA Updates

| Item | Priority | Status |
|------|----------|--------|
| **Secure OTA** | High | ⬜ Implement signed firmware updates |
| **Rollback Support** | High | ⬜ Dual partition with fallback |
| **Update Server** | Medium | ⬜ Set up update infrastructure |
| **Version Checking** | Medium | ⬜ Check for updates periodically |
| **Delta Updates** | Low | ⬜ Reduce update size |

### Quality Assurance

| Item | Priority | Status |
|------|----------|--------|
| **Unit Tests** | High | ⬜ Test critical functions |
| **Integration Tests** | Medium | ⬜ API endpoint testing |
| **Stress Testing** | Medium | ⬜ Long-running stability tests |
| **Memory Analysis** | Medium | ✅ Heap stats shown in web UI |

---

## 4. Security Hardening

### Network Security

| Item | Priority | Status |
|------|----------|--------|
| **HTTPS Support** | High | ⬜ TLS for web interface |
| **WSS (Secure WebSocket)** | High | ⬜ TLS for WebSocket |
| **MQTT TLS** | High | ⬜ Implement MQTTS |
| **Password Hashing** | High | ⬜ Don't store plaintext WiFi password |
| **API Authentication** | High | ⬜ Add token-based auth for API |
| **Rate Limiting** | Medium | ⬜ Prevent brute force attacks |
| **CSRF Protection** | Medium | ⬜ Add CSRF tokens to forms |

### Access Control

| Item | Priority | Status |
|------|----------|--------|
| **Admin Password** | High | ⬜ Require password for sensitive actions |
| **Session Management** | High | ⬜ Implement proper sessions |
| **AP Mode Password** | Medium | ⬜ Secure the setup AP |
| **Physical Security** | Medium | ⬜ Disable JTAG in production |

### Data Protection (GDPR)

| Item | Priority | Status |
|------|----------|--------|
| **Data Minimization** | High | ✅ Only stores WiFi/MQTT credentials |
| **Privacy Policy** | High | ⬜ Document what data is collected |
| **Data Deletion** | High | ✅ Factory reset clears all NVS data |
| **No Cloud Dependency** | Medium | ✅ Device works 100% locally |

---

## 5. Manufacturing & Supply Chain

### Production Planning

| Item | Priority | Status |
|------|----------|--------|
| **Bill of Materials (BOM)** | High | ⬜ Finalize component list |
| **Supplier Selection** | High | ⬜ 2+ sources per critical component |
| **Contract Manufacturer** | High | ⬜ Select EMS partner |
| **Quality Control Process** | High | ⬜ Define QC procedures |
| **Production Testing** | High | ⬜ Define test fixtures/procedures |

### Cost Analysis

| Cost Category | Estimated Range |
|---------------|-----------------|
| PCB + Assembly | €5-15 per unit |
| Components | €10-25 per unit |
| Enclosure | €2-8 per unit |
| Testing | €1-3 per unit |
| Packaging | €1-3 per unit |
| Certification (amortized) | €0.50-2 per unit |
| **Total BOM Cost** | **€20-55 per unit** |

### Inventory & Logistics

| Item | Priority | Status |
|------|----------|--------|
| **Warehouse Partner** | Medium | ⬜ Select fulfillment provider |
| **Packaging Design** | Medium | ⬜ Design retail-ready packaging |
| **Serial Number System** | High | ⬜ Unique device identification |
| **Inventory Management** | Medium | ⬜ Track stock levels |

---

## 6. Documentation & Localization

### Required Documentation

| Document | Language | Status |
|----------|----------|--------|
| **User Manual** | DE, EN | ⬜ Create professional manual |
| **Quick Start Guide** | DE, EN | ⬜ Printed insert |
| **Safety Instructions** | DE, EN | ⬜ Required warnings |
| **Declaration of Conformity** | DE, EN | ⬜ Legal document |
| **Technical Data Sheet** | EN | ⬜ For B2B customers |
| **Installation Guide** | DE, EN | ⬜ With diagrams |

### Web Interface Localization

| Item | Priority | Status |
|------|----------|--------|
| **German Translation** | High | ⬜ Translate all UI text |
| **English (Default)** | High | ✅ Current implementation |
| **Language Selection** | Medium | ⬜ Add language switcher |
| **RTL Support** | Low | ⬜ For future markets |

### Support Resources

| Item | Priority | Status |
|------|----------|--------|
| **FAQ Page** | Medium | ⬜ Common questions |
| **Troubleshooting Guide** | Medium | ⬜ Common issues |
| **Video Tutorials** | Low | ⬜ Installation walkthrough |
| **Support Email/System** | High | ⬜ Customer support channel |

---

## 7. Business & Legal

### Company Setup

| Item | Priority | Status |
|------|----------|--------|
| **Legal Entity** | High | ✅ GmbH, UG, or Einzelunternehmer |
| **Trade Registration** | High | ✅ Gewerbeanmeldung |
| **VAT Registration** | High | ✅ Umsatzsteuer-IdNr |
| **Business Insurance** | High | ⬜ Product liability insurance |

### Legal Documents

| Document | Priority | Status |
|----------|----------|--------|
| **Terms of Service** | High | ⬜ AGB in German |
| **Privacy Policy** | High | ⬜ Datenschutzerklärung |
| **Return Policy** | High | ⬜ 14-day right of withdrawal |
| **Warranty Terms** | High | ⬜ Minimum 2 years in EU |
| **Impressum** | High | ⬜ Required for website |

### Intellectual Property

| Item | Priority | Status |
|------|----------|--------|
| **Trademark Search** | High | ⬜ Check "Restarter" availability |
| **Trademark Registration** | Medium | ⬜ DPMA or EUIPO |
| **Patent Consideration** | Low | ⬜ Evaluate if applicable |
| **Open Source Compliance** | High | ⬜ Audit library licenses |

### Insurance

| Type | Recommended Coverage |
|------|---------------------|
| **Product Liability**   | €1-5 million      | **Expected Cost (Germany, Startup):** €400-900/year |
| **Business Liability**  | €1-3 million      | **Expected Cost (Germany, Startup):** €120-350/year   |
| **Cyber Insurance**     | €500k-1 million   | **Expected Cost (Germany, Startup):** €200-450/year   |

---

## 8. Go-to-Market

### Sales Channels

| Channel | Priority | Status |
|---------|----------|--------|
| **Own Website/Shop** | High | ⬜ WooCommerce, Shopify, etc. |
| **Amazon.de** | High | ⬜ FBA or merchant fulfilled |
| **eBay.de** | Medium | ⬜ Additional reach |
| **B2B Direct Sales** | Medium | ⬜ IT service providers |
| **Retail (Conrad, etc.)** | Low | ⬜ Requires volume |

### Pricing Strategy

| Cost Component            | Amount      |
|--------------------------|-------------|
| Manufacturing Cost        | €5          |
| Certification (amortized) | €2          |
| Shipping/Fulfillment      | €2          |
| Returns Reserve (5%)      | €0.50       |
| Marketing (15%)           | €1.50       |
| Margin (30-40%)           | €4-6        |
| **Suggested Retail Price**| **€15-18**  |

### Marketing

| Channel | Priority | Status |
|---------|----------|--------|
| **Product Website** | High | ⬜ Landing page |
| **SEO** | High | ⬜ Organic search optimization |
| **Tech Blogs/Reviews** | Medium | ⬜ Send review units |
| **YouTube** | Medium | ⬜ Demo videos |
| **Reddit/Forums** | Medium | ⬜ r/homelab, r/homeassistant |
| **Social Media** | Low | ⬜ Twitter/X, LinkedIn |

---

## Timeline Overview

| Phase | Duration | Key Milestones |
|-------|----------|----------------|
| **Phase 1: Preparation** | 1-2 months | Hardware finalization, security audit |
| **Phase 2: Certification** | 2-4 months | CE testing, documentation |
| **Phase 3: Manufacturing Setup** | 1-2 months | EMS partner, first batch |
| **Phase 4: Business Setup** | 1 month | Legal entity, registrations |
| **Phase 5: Soft Launch** | 1 month | Limited release, feedback |
| **Phase 6: Full Launch** | Ongoing | Marketing, sales channels |
| **Total Estimated Time** | **6-10 months** | |

---

## Budget Estimate

| Category | Estimated Cost |
|----------|---------------|
| CE Certification | €5,000-15,000 |
| Legal/Business Setup | €2,000-5,000 |
| Website/Marketing | €2,000-5,000 |
| First Production Run (100 units) | €3,000-5,000 |
| Insurance (annual) | €500-2,000 |
| Registrations (WEEE, LUCID, etc.) | €500-1,500 |
| Contingency (20%) | €2,500-6,500 |
| **Total Initial Investment** | **€15,000-40,000** |

---

## Resources & Links

### Certification & Testing
- [TÜV Rheinland](https://www.tuv.com/germany/de/)
- [DEKRA](https://www.dekra.de/)
- [VDE Testing](https://www.vde.com/de)

### Registrations
- [LUCID Packaging Register](https://lucid.verpackungsregister.org/)
- [stiftung ear (WEEE)](https://www.stiftung-ear.de/)
- [DPMA Trademark Office](https://www.dpma.de/)

### Standards
- EN 55032 (EMC emissions)
- EN 55035 (EMC immunity)
- EN 62368-1 (Safety)
- ETSI EN 301 489 (RED EMC)
- ETSI EN 300 328 (WiFi 2.4GHz)

### Business Resources
- [IHK (Chamber of Commerce)](https://www.ihk.de/)
- [Handwerkskammer](https://www.hwk.de/)
- [Gründerplattform](https://gruenderplattform.de/)

---

*Last Updated: February 2026*
