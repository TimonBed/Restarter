#import "@preview/codetastic:0.2.2": qrcode

// 1. Color Palette (Modern Tech Blue & Safety Red)
#let color-primary = rgb("#0056b3") 
#let color-accent = rgb("#eef4fb")
#let color-text = rgb("#333333")
#let color-alert = rgb("#d32f2f")
#let color-warning = rgb("#f57c00")
#let color-success = rgb("#388e3c")

// 2. Document Setup
#set page(
  paper: "a5",
  margin: (top: 1.5cm, bottom: 1.5cm, left: 1.2cm, right: 1.2cm),
  numbering: "1 / 1",
  header: context {
    if counter(page).get().first() > 1 [
      #set text(size: 8pt, fill: gray)
      PC Restarter - Installation & Bedienung
      #h(1fr)
      #line(length: 100%, stroke: 0.5pt + gray)
    ]
  },
  footer: [
    #set text(size: 7pt, fill: gray)
    #line(length: 100%, stroke: 0.5pt + gray)
    #grid(
      columns: (1fr, auto),
      align: (left, right),
      [© 2026 Timon Bedynek | Rev 3.0],
      [Seite #context counter(page).display("1 von 1")]
    )
  ]
)

#set text(font: ("Roboto", "Arial", "sans-serif"), size: 9pt, fill: color-text, lang: "de")
#set par(leading: 0.9em, justify: true)

// 3. Custom Heading Styles
#show heading.where(level: 1): it => {
  pagebreak(weak: true)
  set text(size: 16pt, weight: "bold", fill: color-primary)
  block(below: 0.8em, it.body)
  line(length: 100%, stroke: 1pt + color-primary)
}

#show heading.where(level: 2): it => {
  set text(size: 11pt, weight: "bold", fill: color-primary)
  pad(top: 0.5em, bottom: 0.3em, it.body)
}

#show heading.where(level: 3): it => {
  set text(size: 10pt, weight: "semibold", fill: color-text)
  it.body
}

// 4. Utility Functions (Components)

// Alert Box for Warnings
#let alert-box(title: "ACHTUNG", icon: "⚠️", color: color-alert, body) = {
  block(
    fill: color.lighten(90%),
    stroke: (left: 3pt + color),
    inset: 8pt,
    radius: 2pt,
    width: 100%,
    [
      #text(fill: color, weight: "bold")[#icon #title] \
      #v(0.2em)
      #body
    ]
  )
}

// Data Tables with Zebra Striping
#let spec-table(data) = {
  table(
    columns: (35%, 1fr),
    inset: 5pt,
    stroke: none,
    fill: (x, y) => if calc.odd(y) { color-accent } else { white },
    align: left,
    ..data
  )
}

// Checkmark/Cross Lists
#let status-list(items, type: "check") = {
  let icon = if type == "check" { text(fill: color-success)[✅] } else { text(fill: color-alert)[❌] }
  grid(
    columns: (1.5em, 1fr),
    row-gutter: 0.7em,
    ..items.map(it => (icon, it)).flatten()
  )
}

// --- DOCUMENT CONTENT ---

// --- CONFIGURATION ---
#let color-primary = rgb("#2563eb")
#let color-accent = rgb("#eff6ff")
#let color-warning = rgb("#f59e0b")
#let color-success = rgb("#10b981")

#set page(
  margin: (x: 1.5cm, y: 1.5cm),
)
#set text(font: "Segoe UI", size: 10pt) // Modern sans-serif

// --- COMPONENTS ---
#let card(body, fill: white, stroke: none) = {
  block(fill: fill, stroke: stroke, radius: 6pt, inset: 12pt, width: 100%, body)
}

// --- CONTENT ---

// 1. HEADER (Kompakt)
#align(center)[
  #box(fill: color-accent, radius: 50%, inset: 8pt)[
    #image("../../../images/logo.svg", width: 40pt)
  ]
  
  #v(0.4em)
  #text(size: 19pt, weight: "black", fill: color-primary)[PC Restarter] \
  #text(size: 10pt, weight: "medium", fill: gray.darken(40%))[Netzwerkfähiges Steuergerät]
  
  #v(0.3em)
  #box(fill: luma(240), inset: (x: 6pt, y: 2pt), radius: 3pt)[
    #text(size: 7pt, weight: "bold", fill: gray.darken(50%))[VERSION 3.0 | STAND: FEBRUAR 2026]
  ]
]

// kein extra Vertikalabstand mehr vor Hero-Text (direkt dahinter)

// 2. HERO DESCRIPTION (kompakter)
#align(center)[
  #block(width: 85%)[
    #text(size: 8pt, fill: luma(120))[
      **WLAN-Fernsteuerung** für Desktop-PCs – lokal & sicher, ohne Cloud.
    ]
  ]
]


== Wichtige Sicherheitshinweise

// Kompakter Sicherheitshinweis-Block (weniger Höhe)
#block(
  width: 100%,
  stroke: (left: 3pt + color-warning),
  fill: color-warning.lighten(96%),
  inset: (x: 10pt, y: 6pt),
  radius: 3pt,
  [
    #grid(
      columns: (auto, 1fr),
      gutter: 0.7em,
      align: horizon,
      text(size: 14pt)[⚠️],
      [
        #text(weight: "bold", fill: color-warning.darken(30%), size: 8pt)[WICHTIGER SICHERHEITSHINWEIS]
        #v(0.13em)
        #text(size: 7.6pt)[
          - *Nur für trockene Innenräume geeignet.*
          - *Lebensgefahr durch Stromschlag.* PC vor Installation komplett vom Stromnetz trennen!
          - *Kinder:* Gerät außerhalb deren Reichweite lagern.
          - Nur durch qualifiziertes Personal montieren.
          - Detaillierte Warnhinweise beachten.
        ]
      ]
    )
  ]
)

== Bestimmungsgemäßer Gebrauch
Der PC Restarter dient der *Fernsteuerung und Überwachung von Desktop-PCs* über Mainboard-Anschlüsse.
Er ist ausschließlich konzipiert für:
- Desktop-PCs mit Standard-ATX-Frontpanel-Anschlüssen.
- Private und gewerbliche Nutzung in Innenräumen.

*Jede andere Verwendung gilt als nicht bestimmungsgemäß* und kann zu Schäden an Hardware oder Datenverlust führen.

== Nicht bestimmungsgemäßer Gebrauch & Elektrische Sicherheit

#grid(
  columns: (1fr, 1fr),
  gutter: 0.7em,
  [
    #set text(size: 7pt)
    #status-list((
      [Einsatz in Laptops, Notebooks oder Macs],
      [Betrieb im Außenbereich oder Feuchträumen],
      [Eigenmächtige Umbauten oder Öffnen des Geräts],
      [Betrieb mit Spannungen > 5 V DC]
    ), type: "cross")
  ],
  [
    #set text(size: 7pt)
    #status-list((
      [Spannung: *5 V DC* (USB)],
      [Max. Strom: *1 A*],
      [Hitzequellen vermeiden],
      [Nicht abdecken (Überhitzung)],
      [Keine defekten Kabel nutzen]
    ))
  ]
)

#text(size: 7pt, style: "italic", fill: gray)[
  *Haftungsausschluss:* Der Hersteller übernimmt keine Haftung für Schäden an Hardware, Software oder Daten, die durch unsachgemäße Installation entstehen.
]

// --- PAGE 2: MANUAL (Rückseite) ---

= Installation & Bedienung

== 1. Produktbeschreibung
Der *PC Restarter* ist ein ESP32-basiertes Steuergerät zum Ein-, Aus- und Neustarten von Desktop-PCs über WLAN – *lokal und ohne Cloud*.

== 2. Technische Daten & Lieferumfang
#grid(
  columns: (1fr, 1fr),
  gutter: 1em,
  [
    *Technische Daten*
    #spec-table((
      [Spannung], [5V DC (USB)],
      [WLAN], [802.11 b/g/n],
      [Chip], [ESP32-C3],
      [Temp.], [0°C – 40°C],
    ))
  ],
  [
    *Lieferumfang*
    
    #list(
      [1× PC Restarter Gerät],
      [1× USB-A Stromkabel],
      [1× Anleitung],
    )
  ]
)

== 3. Sicherheitsvorbereitungen
#alert-box(title: "Vor der Installation", color: color-warning)[
  1. PC herunterfahren & *Netzstecker ziehen*.
  2. Netzteil-Schalter auf "0".
  3. Power-Taste 5 Sek. drücken (Reststrom entladen).
  4. Gehäuse öffnen & *Dich selbst erden*.
]

== 4. Anschluss & Verkabelung
Das Gerät wird *zwischen* Gehäuse-Taster und Mainboard geschaltet ("Man-in-the-Middle"). 

#table(
  columns: (auto, 1fr),
  inset: 5pt,
  fill: (_, row) => if row == 0 { color-primary } else { white },
  stroke: 0.5pt + gray,
  align: left,
  text(fill: white, weight: "bold")[Anschluss], text(fill: white, weight: "bold")[Beschreibung],
  // Escape the underscores here:
  [*MB\_PWR\_SW*], [Zum Mainboard Power-Switch Header],
  [*MB\_PWR\_LED*], [Zum Mainboard Power-LED Header (+/- beachten!)],
  [*CASE\_...*], [Verbindung zu den Gehäuse-Kabeln],
)
// --- PAGE 3: SETUP wrap ---
#text(size: 8pt, fill: gray)[Umblättern → Rückseite: Einrichtung & weitere Hinweise]
#pagebreak()

#let color-primary = rgb("#1a5fb4")

// Define the circle style once to keep it clean
#let step-circle(n) = box(
  circle(
    radius: 0.8em,
    fill: color-primary,
    align(center + horizon, text(fill: white, weight: "bold", str(n)))
  )
)

== 5. Ersteinrichtung

#grid(
  columns: (auto, 1fr),
  column-gutter: 1em,
  row-gutter: 0.8em,
  align: horizon, // <--- This centers the circle vertically against the text
  
  step-circle(1),
  [Gerät per USB mit Strom versorgen.],
  
  step-circle(2),
  [Verbinde dich mit dem WLAN: `PC-Restarter-XXXXXX` mit deinem Handy oder Laptop.],
  
  step-circle(3),
  [Die Einrichtungsseite öffnet sich automatisch. Wenn nicht, öffne im Browser: `http://192.168.4.1`],
  
  step-circle(4),
  [Wähle dein Heim-WLAN und speichere die Daten.]
)

#v(0.5em)
Danach erreichbar unter: `http://pc-restarter.local`
== 6. API & Integrationen

=== REST API
Der PC Restarter bietet eine REST-API für die Steuerung und Abfrage des Geräts.

*Basis-URL:* `http://pc-restarter.local/api`

*Endpunkte:*
#spec-table((
  [`GET /api/status`], [Gerätestatus abrufen],
  [`POST /api/power/on`], [PC einschalten],
  [`POST /api/power/off`], [PC ausschalten],
  [`POST /api/power/reset`], [PC neustarten],
  [`GET /api/info`], [Geräteinformationen],
))

*Beispiel:*
```bash
curl -X POST http://pc-restarter.local/api/power/on
```

=== MQTT
Das Gerät unterstützt MQTT für die Integration in Smart-Home-Systeme.

*Standard-Topics:*
#spec-table((
  [`pc-restarter/status`], [Gerätestatus (JSON)],
  [`pc-restarter/power/command`], [Befehle: `on`, `off`, `reset`],
  [`pc-restarter/power/state`], [Aktueller Zustand],
))

*Konfiguration:* Über Web-Interface unter *Einstellungen → MQTT*

=== Home Assistant

Das Gerät kann einfach in Home Assistant integriert werden.

#grid(
  columns: (1fr, 1fr),
  gutter: 1em,
  align: (left, center),
  [
    *Detaillierte Anleitung:*
    #v(0.3em)
    #text(size: 7.5pt)[
      Für eine ausführliche Schritt-für-Schritt Anleitung zur Integration in Home Assistant, scannen Sie den QR-Code oder besuchen Sie die Website.
    ]
  ],
  [
    #align(center)[
      #block(width: 3.5cm, height: 1.5cm)[
        #align(center)[
        #qrcode("https://google.com", size: 0.6mm)
        ]
      ]
      #text(size: 7pt, fill: gray)[Home Assistant Anleitung]
    ]
  ]
)

=== Prometheus
Metriken für Monitoring über Prometheus verfügbar.

*Endpoint:* `http://pc-restarter.local/metrics`

*Verfügbare Metriken:*
- `pc_restarter_uptime_seconds` - Betriebszeit
- `pc_restarter_power_state` - Stromzustand (0/1)
- `pc_restarter_wifi_rssi` - WLAN-Signalstärke
- `pc_restarter_temperature_celsius` - Temperatur

*Grafana Integration:* Prometheus als Datenquelle konfigurieren.

=== Loki
Logs können an Loki gesendet werden (optional).

*Konfiguration:* Über Web-Interface unter *Einstellungen → Logging*

*Log-Level:* ERROR, WARN, INFO, DEBUG

*Format:* JSON-Logs mit Timestamp und Level

== 7. Entsorgung & Konformität
#table(
  columns: (auto, 1fr),
  stroke: none,
  align: (center, top),
  [#image("../../../images/WEEE_2.svg", height: 7mm)], [*WEEE:* Nicht im Hausmüll entsorgen.],
  [#image("../../../images/CE.svg", height: 7mm)], [*CE:* Entspricht RED 2014/53/EU, RoHS 2011/65/EU. \
    Konformitätserklärung: `https://[domain].de/conformity`]
)

#line(length: 100%, stroke: 0.5pt + gray)

#text(size: 7.5pt)[
  *Hersteller:* Timon Bedynek, [Straße], [PLZ Ort], Deutschland | *Support:* support@[domain].de
]