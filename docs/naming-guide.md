# Naming Guide – Device & Company

## Device name

**Current:** PC Restarter

**Guidelines:**
- Keep it short, memorable, and descriptive of function (remote power/restart for PCs).
- Use one consistent name in all docs, packaging, and software (no "PC Restarter" vs "PC Power" mix in branding).
- For variants later: add suffix (e.g. PC Restarter Pro, PC Restarter Lite) or model code (e.g. PCR-100).
- Avoid special characters; use ASCII for compatibility (manuals, labels, config UIs).

**Where it appears:** technical-datasheet, datenblatt, user manuals, safety docs, compliance docs, packaging. Use the same spelling everywhere.

---

## Company name

**Placeholders in this repo:** `[Firmenname]` (DE), `[Company Name]` (EN), `[TODO]` (impressum/compliance).

**Guidelines:**
- Legal name as in Handelsregister / company register (exact spelling).
- If brand ≠ legal name: use legal name in Impressum, AGB, Datenschutz, EU declarations; you can add “Brand X by Legal Name GmbH” on product pages.
- Include Rechtsform in DE (GmbH, UG, e.K. etc.).
- One canonical spelling: pick one and replace all placeholders project-wide (see list below).

**Replace in these files:**
- `docs/rechtlich/impressum.md` – Firmenname, Rechtsform, Anschrift, etc.
- `docs/rechtlich/agb.md` – [Firmenname]
- `docs/rechtlich/datenschutzerklaerung.md` – [Firmenname], Anbieter
- `docs/produkt/de/*.md` – [Firmenname] (Beilegezettel, Bedienungsanleitung, Sicherheitshinweise)
- `docs/produkt/en/*.md` – [Company Name] (user-manual, safety-instructions)
- `docs/compliance/technische-dokumentation.md` – [Firmenname], Hersteller
- `docs/compliance/eu-konformitaetserklaerung.md` – [Firmenname] / [Company Name]
- `docs/compliance/eu-declaration-of-conformity-en.md` – Company name, Name (responsible person)

**Tip:** Use search in `docs` for `[Firmenname]`, `[Company Name]`, `[TODO]` and replace with your real company name (and responsible person name where needed).

---

## Name inspiration

### Device (product) names

| Style | Examples |
|-------|----------|
| **Descriptive** | PC Restarter, Remote Reboot, Power Remote, WakeBox |
| **Short / tech** | Reboot, Restart, PowerNode, PowerHub, RebootOne |
| **Action** | RebootPro, PowerCycle, RemotePower, WakeUp |
| **Neutral / codename** | PCR-1, Node One, Unit R, R1 |
| **German-friendly** | PC-Neustart, Fernstart, Reboot-Box |

### Company / brand names

| Style | Examples |
|-------|----------|
| **Tech / product-led** | Restart Labs, PowerNode Systems, Reboot Tech |
| **Short / abstract** | Revo, Nodix, Relio, Restart GmbH |
| **Trust / solid** | Solid Power, Reliable Systems, SteadyTech |
| **Your name / personal** | [YourName] Elektronik, [YourName] Solutions |
| **Descriptive** | Fernsteuerung GmbH, PC-Steuerung, Remote Control Systems |

**Quick checks:** Google the name (free domain? no strong trademark in your class?). For DE: check Markenregister (DPMA) and Handelsregister for conflicts.
