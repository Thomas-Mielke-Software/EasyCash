# Migrationsplan: ECTIFace (C++/MFC) → ECTEngine (C#/.NET)

## 1. Ist-Analyse

### 1.1 Aktuelle Architektur

Das Projekt EasyCash&Tax ist eine MFC-MDI-Applikation (Visual Studio 2022, Toolset v143, MultiByte-Zeichensatz), die als **EasyCT.exe** gebaut wird. Die Geschäftslogik lebt aktuell im Subprojekt **ECTIFace**, das als native C++-Bibliothek (`ECTIFace.lib` / `ECTIFace.dll`) gebaut und gegen die Hauptapplikation gelinkt wird.

```
┌──────────────────────────────────────────────────────┐
│  EasyCT.exe  (MFC-MDI-Applikation, C++/v143)         │
│  ┌────────────────────────────────────────────────┐  │
│  │  UI-Schicht (Views, Dialoge, MainFrm)          │  │
│  │  • CEasyCashView (Journal, Formulare, Druck)   │  │
│  │  • BuchenDlg, DauerbuchungenDlg, ...           │  │
│  │  • Navigation, PluginManager                   │  │
│  └──────────────┬─────────────────────────────────┘  │
│                 │ #include "ECTIFace\*.h"             │
│                 │ link: ECTIFace.lib                  │
│  ┌──────────────▼─────────────────────────────────┐  │
│  │  ECTIFace.dll/.lib  (Geschäftslogik, C++)      │  │
│  │  • CBetrag          (Währungsbeträge)           │  │
│  │  • CBuchung         (Einzelbuchung)             │  │
│  │  • CDauerbuchung    (Wiederkehrende Buchung)    │  │
│  │  • CEasyCashDoc     (MFC-Dokumentenklasse)      │  │
│  │  • AfAGenauigkeit   (Abschreibungslogik)        │  │
│  │  • Erweiterung-API  (Key/Value-Extensions)      │  │
│  │  • currency_to_int  (Währungskonvertierung)     │  │
│  └────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────┤
│  Plugin-Ökosystem:                                    │
│  • EasyCTX.ocx    (COM/ActiveX-Schnittstelle)        │
│  • EasyCTXP.dll   (Plugin-Proxy)                     │
│  • ECTElster, ECTDatev, ECTImport (Plugins)          │
└──────────────────────────────────────────────────────┘
```

### 1.2 Identifizierte Kernklassen in ECTIFace

| Klasse | Verantwortlichkeit | Bekannte Mitglieder/Aspekte |
|--------|-------------------|----------------------------|
| **CBetrag** | Währungsbeträge, Cent-genaue Arithmetik | `currency_to_int()`; ab >10.000 € Wechsel auf Fließkomma; kaufmännische Rundung |
| **CBuchung** | Einzelne Buchung im Journal | `Belegnummer` (CString), `Beschreibung` (CString), `Erweiterung` (Key/Value-Daten), `cBuchungsart` ('E'/'A'), Datum, Betrag, USt-Informationen, AfA-Dauer, Restwert, Split-Buchungs-Marker, Betrieb-Zuordnung |
| **CDauerbuchung** | Wiederkehrende Buchungen | Intervalle (monatlich, 2-monatlich, quartalsweise, …), Platzhalter ($M, $m, $q, $h, $J, $j, $2), "Ausgeführt bis"-Datum, Beschreibung, Belegnummer-Template |
| **CEasyCashDoc** | MFC-Dokumentenklasse (CDocument) | Buchungslisten (CObArray/CTypedPtrArray), Serialize(), Mandant/Betrieb-Verwaltung, Kontenlisten (Einnahmen/Ausgaben je 100 Slots), Bestandskonten, Einstellungen, Jahreswechsel-Logik |
| **AfAGenauigkeit** | Abschreibungsberechnung | Lineare AfA, Sonderabschreibungen, Buchwertberechnung, Abgänge |
| **Erweiterung-API** | Erweiterbare Metadaten pro Buchung | `GetErweiterungKey()`, `GetErweiterungKeyCS()` – Schlüssel z.B. "UrspruenglichesKonto", "UrspruenglicherNettobetrag", "UrspruenglichesAnschaffungsdatum", "SplitGegenbuchungOhneVorsteuerabzug" |

### 1.3 Abhängigkeiten und Kopplung

Die **enge Kopplung** zwischen ECTIFace und der UI-Schicht zeigt sich in mehreren Punkten:

- **CEasyCashDoc erbt von CDocument** – die MFC-Dokumentenklasse ist gleichzeitig Datencontainer und Persistenzschicht (Serialize/Deserialize über MFC-Archive).
- **CBuchung-Pointer werden direkt in View-Methoden** wie `DrawToDC_BestandskontenLine(CBuchung *p, ...)` übergeben.
- **CString-Nutzung** durchgängig – alle Felder nutzen MFC-CString statt std::string.
- **CObArray / CTypedPtrArray** – MFC-Collection-Klassen für Buchungslisten.
- **Plugin-Schnittstelle** (EasyCTX.ocx) greift auf ECTIFace.dll zu und leitet Daten an COM-Clients weiter.

---

## 2. Zielarchitektur: ECTEngine (.NET)

### 2.1 Architekturüberblick

```
┌──────────────────────────────────────────────────────┐
│  EasyCT.exe  (MFC-MDI, C++/CLI Bridge)                │
│  ┌────────────────────────────────────────────────┐  │
│  │  UI-Schicht (Views, Dialoge)                   │  │
│  │  (bleibt C++/MFC – wird NICHT migriert)        │  │
│  └──────────────┬─────────────────────────────────┘  │
│                 │                                     │
│  ┌──────────────▼─────────────────────────────────┐  │
│  │  ECTBridge.dll  (C++/CLI "Mixed-Mode")         │  │
│  │  • Dünne Adapter-Schicht                       │  │
│  │  • Übersetzt CString ↔ System::String          │  │
│  │  • Übersetzt native Pointer ↔ managed Refs     │  │
│  │  • Stellt CEasyCashDocBridge bereit             │  │
│  │    (erbt von CDocument, delegiert an Engine)    │  │
│  └──────────────┬─────────────────────────────────┘  │
│                 │ [.NET Interop]                      │
│  ┌──────────────▼─────────────────────────────────┐  │
│  │  ECTEngine.dll  (.NET Class Library, C#)       │  │
│  │  • Betrag         (decimal-basiert)             │  │
│  │  • Buchung        (POCO)                        │  │
│  │  • Dauerbuchung   (POCO)                        │  │
│  │  • BuchungsDocument (Geschäftslogik)            │  │
│  │  • AfACalculator  (Abschreibungslogik)          │  │
│  │  • ErweiterungStore (Dictionary-basiert)        │  │
│  │  • IBuchungsRepository (Persistenz-Interface)   │  │
│  └────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────┤
│  Plugin-Ökosystem (Phase 2):                          │
│  • Bestehende Plugins arbeiten weiter über            │
│    EasyCTX.ocx (COM-Bridge bleibt vorerst)            │
│  • Neue Plugins können ECTEngine direkt nutzen        │
└──────────────────────────────────────────────────────┘
```

### 2.2 Interop-Strategie: C++/CLI als Bridge

Für die Integration einer .NET-Assembly in eine native MFC-Applikation gibt es drei gängige Optionen:

| Strategie | Vorteile | Nachteile |
|-----------|----------|-----------|
| **COM-Interop** | Saubere Trennung, Plugin-kompatibel | Hoher Boilerplate, Registrierung nötig, COM-Type-Marshalling umständlich |
| **C++/CLI Mixed-Mode** | Direkte .NET-Aufrufe aus C++, kein COM-Overhead, schrittweise Migration möglich | Erfordert /clr pro Modul, etwas komplexere Build-Konfiguration |
| **Hosting der CLR** | Maximale Kontrolle | Sehr hoher Aufwand, fehleranfällig |

**Empfehlung: C++/CLI Mixed-Mode** (ECTBridge.dll). Gründe:

- Die MFC-App nutzt bereits v143 – C++/CLI wird von VS2022 vollständig unterstützt.
- Kein COM-Registrierungsaufwand (wichtig für portable Installation per XCOPY/Installer).
- Minimaler Marshalling-Overhead: CString ↔ String^, native Arrays ↔ managed Collections.
- Die bestehende CEasyCashDoc kann schrittweise delegieren: Methode für Methode kann Logik von C++ nach C# wandern, während die CDocument-Vererbung erhalten bleibt.

### 2.3 Build-Konfiguration

```
EasyCash.sln
├── EasyCash.vcxproj          (C++/MFC, native, wie bisher)
│   └── linkt gegen: ECTBridge.lib
├── ECTBridge.vcxproj         (C++/CLI, /clr, Mixed-Mode DLL)
│   └── Referenz auf: ECTEngine.csproj
├── ECTEngine.csproj          (C# Class Library, .NET Framework 4.8)
│   └── Keine MFC-/native Abhängigkeiten
└── ECTEngine.Tests.csproj    (C# Unit Tests, xUnit/NUnit)
```

**Warum .NET Framework 4.8 statt .NET 8?** C++/CLI mit /clr:mixed erfordert .NET Framework. Ein späterer Wechsel zu .NET 8 wäre über COM-Interop oder Hosting möglich, verkompliziert aber die Bridge. Da EasyCash eine Desktop-Win32-App ist, ist .NET Framework 4.8 (auf Windows vorinstalliert) die pragmatische Wahl.

---

## 3. Klassendesign ECTEngine (C#)

### 3.1 Betrag

**C++-Original (CBetrag : CObject):**
```
int Betrag;     // brutto, in Cents
int MWSt;       // in Prozent * 1000 (z.B. 19% = 19000, 7% = 7000)
long GetNetto();
```

**C#-Äquivalent:**
```csharp
namespace ECTEngine
{
    /// <summary>
    /// Ersetzt CBetrag. Nutzt decimal statt int-Cents.
    /// MWSt wird als decimal-Prozentwert gespeichert (z.B. 19.0m).
    /// </summary>
    public struct Betrag : IEquatable<Betrag>, IComparable<Betrag>
    {
        public decimal BruttoWert { get; }
        public decimal MwstProzent { get; }  // z.B. 19.0, 7.0, 0

        public Betrag(decimal brutto, decimal mwstProzent = 0m)
        {
            BruttoWert = decimal.Round(brutto, 2, MidpointRounding.AwayFromZero);
            MwstProzent = mwstProzent;
        }

        // Kompatibilität mit Legacy-Code (int Cents, int MWSt-Promille)
        public int InCent => (int)(BruttoWert * 100m);
        public int MwstPromille => (int)(MwstProzent * 1000m);

        public static Betrag AusCent(int cent, int mwstPromille = 0) =>
            new Betrag(cent / 100m, mwstPromille / 1000m);

        // Berechnungen
        public decimal NettoWert =>
            MwstProzent == 0m ? BruttoWert
            : decimal.Round(BruttoWert / (1m + MwstProzent / 100m), 2,
                            MidpointRounding.AwayFromZero);
        public decimal MwstBetrag => BruttoWert - NettoWert;

        // Arithmetik
        public static Betrag operator +(Betrag a, Betrag b) =>
            new Betrag(a.BruttoWert + b.BruttoWert, a.MwstProzent);
        public static Betrag operator -(Betrag a, Betrag b) =>
            new Betrag(a.BruttoWert - b.BruttoWert, a.MwstProzent);
        public static Betrag operator *(Betrag a, decimal faktor) =>
            new Betrag(a.BruttoWert * faktor, a.MwstProzent);
        public static Betrag operator -(Betrag a) =>
            new Betrag(-a.BruttoWert, a.MwstProzent);
        public static Betrag Null => new Betrag(0m);

        // Formatierung
        public string AlsWaehrung(string waehrung = "EUR") =>
            BruttoWert.ToString("N2", new System.Globalization.CultureInfo("de-DE"));
    }
}
```

**Designentscheidung:** `decimal` löst das Problem des CBetrag-Codes, der ab >10.000 € auf Fließkomma umschaltet. `decimal` bietet 28–29 signifikante Stellen und ist für Finanzarithmetik konzipiert.

### 3.2 Buchung

**C++-Original (CBuchung : CBetrag : CObject) — exaktes Feld-Mapping:**
```
Geerbte Felder von CBetrag:
  int Betrag;                    // brutto, in Cents
  int MWSt;                      // Prozent * 1000

Eigene Felder:
  int AbschreibungNr;            // AfA-Nummer
  int AbschreibungJahre;         // AfA-Dauer in Jahren
  CString Beschreibung;          // Buchungstext
  CTime Datum;                   // Buchungsdatum
  CString Konto;                 // E/Ü-Kontoname
  CBuchung *next;                // Linked-List-Zeiger
  CString Belegnummer;           // (seit v7)
  int AbschreibungRestwert;      // Restwert in Cents (seit v7)
  BOOL AbschreibungDegressiv;    // (seit v7)
  int AbschreibungSatz;          // Degressions-Prozentsatz (seit v7)
  CString Erweiterung;           // Plugin-Daten, Pipe-Format (seit v7)
  int AbschreibungGenauigkeit;   // 0..3 (seit v10)
  CString Bestandskonto;         // (seit v11)
  CString Betrieb;               // (seit v11)
```

**C#-Äquivalent:**
```csharp
namespace ECTEngine
{
    public enum Buchungsart { Einnahme = 0, Ausgabe = 1 }

    public enum AfaGenauigkeit
    {
        Ganzjahr = 0,
        Halbjahr = 1,
        Monatsgenau = 2,
        EntsprechendEinstellungen = 3
    }

    /// <summary>
    /// 1:1-Mapping der CBuchung-Klasse.
    /// POCO statt Linked List — die verkettete Liste wird
    /// in BuchungsDocument zu List<Buchung> aufgelöst.
    /// Erbt NICHT von Betrag; stattdessen Composition.
    /// </summary>
    public class Buchung
    {
        // --- aus CBetrag ---
        public Betrag BruttoBetrag { get; set; }
        // MWSt steckt jetzt im Betrag-Struct

        // --- Identität ---
        public Buchungsart Art { get; set; }  // war implizit durch Einnahmen/Ausgaben-Liste

        // --- Kernfelder ---
        public DateTime Datum { get; set; }
        public string Beschreibung { get; set; } = "";
        public string Konto { get; set; } = "";
        public string Belegnummer { get; set; } = "";

        // --- Abschreibung ---
        public int AfaNr { get; set; }
        public int AfaJahre { get; set; }
        public int AfaRestwertCent { get; set; }  // Cent-Wert (Legacy-Kompatibilität)
        public bool AfaDegressiv { get; set; }
        public int AfaSatz { get; set; }  // Prozentsatz bei degressiver AfA
        public AfaGenauigkeit AfaGenauigkeit { get; set; }
            = AfaGenauigkeit.EntsprechendEinstellungen;

        // --- Erweiterungen ---
        public string Bestandskonto { get; set; } = "";
        public string Betrieb { get; set; } = "";
        public ErweiterungStore Erweiterungen { get; set; } = new();

        // --- Berechnete Properties ---
        public decimal NettoWert => BruttoBetrag.NettoWert;
        public decimal MwstBetrag => BruttoBetrag.MwstBetrag;
        public bool IstNeutralesKonto => Konto.StartsWith("/");
        public bool HatAfA => AfaJahre > 0;
        public bool IstSplitBuchung =>
            Erweiterungen.Hat("EasyCash",
                              "SplitGegenbuchungOhneVorsteuerabzug");
    }
}
```

**Wichtige Design-Entscheidungen:**
- **Keine Vererbung** von Betrag – stattdessen Komposition. Das entkoppelt Buchung von der Wertdarstellung.
- **Keine Linked List** – die `*next`-Zeiger werden beim Einlesen in `List<Buchung>` aufgelöst. Die Bridge konvertiert die C++-Linked-Lists beim Deserialisieren.
- **Buchungsart als explizites Feld** – im Original war die Art implizit dadurch bestimmt, ob die Buchung in der `Einnahmen`- oder `Ausgaben`-Liste hing.

### 3.3 Dauerbuchung

**C++-Original (CDauerbuchung : CBetrag : CObject) — exaktes Feld-Mapping:**
```
Geerbte Felder von CBetrag:
  int Betrag;                    // brutto, in Cents
  int MWSt;                      // Prozent * 1000

Eigene Felder:
  CString Beschreibung;
  int Buchungstyp;               // 0=Einnahmen, 1=Ausgaben
  int Intervall;                 // 1=Monat, 2=Quartal, 3=Halbjahr,
                                 // 4=Jahr, 5=2Monate
  CTime VonDatum;
  CTime BisDatum;
  int Buchungstag;               // Tag im Monat für die Buchung
  CTime AktualisiertBisDatum;    // zuletzt ausgeführt bis
  CString Konto;
  CDauerbuchung *next;           // Linked-List-Zeiger
  CString Belegnummer;           // Template mit Platzhaltern (seit v7)
  CString Erweiterung;           // Plugin-Daten (seit v7)
  CString Bestandskonto;         // (seit v12)
  CString Betrieb;               // (seit v12)
```

**C#-Äquivalent:**
```csharp
namespace ECTEngine
{
    public enum Intervall
    {
        Monatlich = 1,
        Quartalsweise = 2,
        Halbjaehrlich = 3,
        Jaehrlich = 4,
        ZweiMonatlich = 5
    }

    public class Dauerbuchung
    {
        // --- aus CBetrag ---
        public Betrag BruttoBetrag { get; set; }

        // --- Kernfelder ---
        public Buchungsart Art { get; set; }
        public string Beschreibung { get; set; } = "";  // Template mit $M $m $q...
        public Intervall Intervall { get; set; }
        public DateTime VonDatum { get; set; }
        public DateTime BisDatum { get; set; }
        public int Buchungstag { get; set; }
        public DateTime AusgefuehrtBis { get; set; }
        public string Konto { get; set; } = "";
        public string Belegnummer { get; set; } = "";    // Template
        public string Bestandskonto { get; set; } = "";
        public string Betrieb { get; set; } = "";
        public ErweiterungStore Erweiterungen { get; set; } = new();

        /// <summary>
        /// Löst Platzhalter auf: $M $m $q $h $J $j $2
        /// </summary>
        public static string ResolvePlatzhalter(
            string template, DateTime stichtag)
        {
            return template
                .Replace("$M", stichtag.Month.ToString("D2"))
                .Replace("$m", stichtag.Month.ToString())
                .Replace("$q", ((stichtag.Month - 1) / 3 + 1).ToString())
                .Replace("$h", (stichtag.Month <= 6 ? 1 : 2).ToString())
                .Replace("$J", stichtag.Year.ToString())
                .Replace("$j", (stichtag.Year % 100).ToString("D2"))
                .Replace("$2", ((stichtag.Month - 1) / 2 + 1).ToString());
        }

        public Buchung ErzeugeBuchung(DateTime stichtag)
        {
            return new Buchung
            {
                Datum = stichtag,
                BruttoBetrag = BruttoBetrag,
                Beschreibung = ResolvePlatzhalter(Beschreibung, stichtag),
                Belegnummer = ResolvePlatzhalter(Belegnummer, stichtag),
                Art = Art,
                Konto = Konto,
                Bestandskonto = Bestandskonto,
                Betrieb = Betrieb
            };
        }

        public IEnumerable<DateTime> FaelligeTermine(DateTime bis)
        { /* Iterator über alle Termine nach AusgefuehrtBis bis 'bis' */ }
    }
}
```

### 3.4 ErweiterungStore

**Originalformat (aus dem Header-Kommentar):**
```
"DLLName1|Schlüssel1=Wert1|Schlüssel2=Wert2||DLLName2|Key=Val||"
```
Trennzeichen: `|` zwischen Key=Value-Paaren, `||` zwischen DLL-Namensräumen. Einschränkung: `|` darf nicht in Keys/Werten vorkommen, `=` nicht in Key-Namen.

**C#-Äquivalent mit kompatiblem Parser:**
```csharp
namespace ECTEngine
{
    /// <summary>
    /// Typsichere Alternative zu GetErweiterungKey/SetErweiterungKey.
    /// Kann das Pipe-Format lesen und schreiben.
    /// </summary>
    public class ErweiterungStore
    {
        private readonly Dictionary<(string Namespace, string Key), string> _data = new();

        public void Setze(string ns, string key, string wert) =>
            _data[(ns, key)] = wert;
        public string Hole(string ns, string key, string fallback = "") =>
            _data.TryGetValue((ns, key), out var v) ? v : fallback;
        public bool Hat(string ns, string key) =>
            _data.ContainsKey((ns, key));
        public void Entferne(string ns, string key) =>
            _data.Remove((ns, key));
        public IEnumerable<(string Ns, string Key, string Wert)> Alle() =>
            _data.Select(kv => (kv.Key.Namespace, kv.Key.Key, kv.Value));

        /// <summary>
        /// Parst das Legacy-Pipe-Format:
        /// "DLLName|Key1=Val1|Key2=Val2||DLLName2|Key=Val||"
        /// </summary>
        public static ErweiterungStore AusPipeFormat(string raw)
        {
            var store = new ErweiterungStore();
            if (string.IsNullOrEmpty(raw)) return store;

            // Aufteilen an "||" → Namensräume
            var namespaces = raw.Split(new[] { "||" },
                                       StringSplitOptions.RemoveEmptyEntries);
            foreach (var nsBlock in namespaces)
            {
                var parts = nsBlock.Split('|');
                if (parts.Length < 1) continue;
                string ns = parts[0]; // erstes Element = DLL-Name
                for (int i = 1; i < parts.Length; i++)
                {
                    int eq = parts[i].IndexOf('=');
                    if (eq > 0)
                    {
                        string key = parts[i].Substring(0, eq);
                        string val = parts[i].Substring(eq + 1);
                        store.Setze(ns, key, val);
                    }
                }
            }
            return store;
        }

        /// <summary>
        /// Serialisiert zurück ins Legacy-Pipe-Format.
        /// </summary>
        public string ZuPipeFormat()
        {
            var sb = new System.Text.StringBuilder();
            var grouped = _data.GroupBy(kv => kv.Key.Namespace);
            foreach (var g in grouped)
            {
                sb.Append(g.Key);
                foreach (var kv in g)
                    sb.Append('|').Append(kv.Key.Key)
                      .Append('=').Append(kv.Value);
                sb.Append("||");
            }
            return sb.ToString();
        }
    }
}
```

### 3.5 BuchungsDocument (Geschäftslogik-Kern)

**C++-Original (CEasyCashDoc : CDocument) — exaktes Feld-Mapping:**
```
CBuchung *Einnahmen;                    // Linked-List-Head
CBuchung *Ausgaben;                     // Linked-List-Head
int Buchungszaehler;
CDauerbuchung *Dauerbuchungen;          // Linked-List-Head
int nLaufendeBuchungsnummerFuerEinnahmen;
int nLaufendeBuchungsnummerFuerAusgaben;
int nJahr;
CString csWaehrung;
CString csUrspruenglicheWaehrung;
int AbschreibungGenauigkeit;            // 0..3
CString Erweiterung;                    // Plugin-Daten
int Version;                            // Dokumentversion (aktuell 13)
int nLaufendeBuchungsnummerFuerBank;    // seit v8
int nLaufendeBuchungsnummerFuerKasse;   // seit v8
int nNachfrageIntervall;               // Backup-Nachfrage (seit v13)
CTime ctNachfrageTermin;               // seit v13

Private:
  CStringArray m_csEinnahmenKonten/Feldzuweisungen;
  CStringArray m_csAusgabenKonten/Feldzuweisungen;
  long m_laFeldWerte[10000];            // Formular-Cache
  BOOL m_bFeldWertGueltig[10000];
```

**C#-Äquivalent:**
```csharp
namespace ECTEngine
{
    /// <summary>
    /// Enthält die Geschäftslogik, die aktuell in CEasyCashDoc steckt.
    /// NICHT von CDocument abgeleitet – die MFC-Vererbung bleibt
    /// in der C++/CLI-Bridge (CEasyCashDocBridge : CDocument).
    ///
    /// Die Linked Lists werden zu List<Buchung> aufgelöst.
    /// Einnahmen und Ausgaben werden in einer gemeinsamen Liste
    /// geführt, unterschieden durch Buchung.Art.
    /// </summary>
    public class BuchungsDocument
    {
        // --- Buchungslisten (ersetzt die Linked Lists) ---
        public List<Buchung> Buchungen { get; } = new();
        public List<Dauerbuchung> Dauerbuchungen { get; } = new();

        // --- Zähler ---
        public int Buchungszaehler { get; set; }
        public int LaufendeBelegnrEinnahmen { get; set; }
        public int LaufendeBelegnrAusgaben { get; set; }
        public int LaufendeBelegnrBank { get; set; }
        public int LaufendeBelegnrKasse { get; set; }

        // --- Dokument-Metadaten ---
        public int Jahr { get; set; }
        public string Waehrung { get; set; } = "EUR";
        public string UrspruenglicheWaehrung { get; set; } = "";
        public AfaGenauigkeit GlobaleAfaGenauigkeit { get; set; }
        public int DokumentVersion { get; set; } = 13;

        // --- Erweiterungen ---
        public ErweiterungStore Erweiterungen { get; set; } = new();

        // --- Backup ---
        public int BackupNachfrageIntervallTage { get; set; }
        public DateTime BackupNachfrageTermin { get; set; }

        // --- Komfort-Zugriffe ---
        public IEnumerable<Buchung> Einnahmen =>
            Buchungen.Where(b => b.Art == Buchungsart.Einnahme);
        public IEnumerable<Buchung> Ausgaben =>
            Buchungen.Where(b => b.Art == Buchungsart.Ausgabe);

        // --- Berechnungen (ersetzen EinnahmenSumme/AusgabenSumme) ---
        public int EinnahmenSumme(int monatsFilter = 0,
            string kontoFilter = "")
        {
            return Einnahmen
                .Where(b => monatsFilter == 0
                            || b.Datum.Month == monatsFilter)
                .Where(b => string.IsNullOrEmpty(kontoFilter)
                            || b.Konto == kontoFilter)
                .Sum(b => b.BruttoBetrag.InCent);
        }

        public int AusgabenSumme(int monatsFilter = 0,
            string kontoFilter = "") { /* analog */ }
        public int EinnahmenSummeNetto(int monatsFilter = 0,
            string kontoFilter = "") { /* netto */ }
        public int AusgabenSummeNetto(int monatsFilter = 0,
            string kontoFilter = "") { /* netto */ }

        // --- Sortierung ---
        public void Sort() =>
            Buchungen.Sort((a, b) => a.Datum.CompareTo(b.Datum));

        // --- Buchungsnummern ---
        public string NaechsteBelegnummer(Buchungsart art)
        {
            if (art == Buchungsart.Einnahme)
                return (++LaufendeBelegnrEinnahmen).ToString();
            else
                return (++LaufendeBelegnrAusgaben).ToString();
        }

        // --- Dauerbuchungen ---
        public List<Buchung> FuehreDauerbuchungenAus(DateTime stichtag)
        { /* ... */ }
        public List<string> PruefeDauerbuchungenVollstaendig()
        { /* Warnung für nicht ausgeführte Dauerbuchungen */ }

        // --- Jahreswechsel ---
        public BuchungsDocument Jahreswechsel(int neuesJahr)
        { /* Übernimmt Dauerbuchungen und AfAs ins neue Jahr */ }

        // --- Persistenz ---
        // Das Laden/Speichern geschieht in der C++/CLI Bridge,
        // die die MFC-CArchive-Logik nutzt und die Daten
        // in dieses Objekt überträgt.
    }
}
```

**Zentrale Architekturänderung:** Die beiden separaten Linked Lists (`Einnahmen`, `Ausgaben`) werden zu einer einzigen `List<Buchung>` zusammengeführt. Die Unterscheidung erfolgt über das Feld `Buchung.Art`. Das vereinfacht Iteration, Filterung und Sortierung erheblich und macht die `*next`-Zeiger-Verwaltung überflüssig.

### 3.6 AfACalculator

```csharp
namespace ECTEngine
{
    public class AfACalculator
    {
        /// <summary>
        /// Berechnet den jährlichen AfA-Betrag (linear).
        /// </summary>
        public Betrag JahresAfA(Betrag anschaffungskosten, int dauerJahre,
                                Betrag restwert)
        {
            if (dauerJahre <= 0) return Betrag.Null;
            return new Betrag((anschaffungskosten.Wert - restwert.Wert)
                              / dauerJahre);
        }

        /// <summary>
        /// Berechnet den Buchwert zu einem Stichtag.
        /// </summary>
        public Betrag Buchwert(Buchung afaBuchung, DateTime stichtag)
        { /* ... */ }

        /// <summary>
        /// Erzeugt AfA-Buchungen beim Jahreswechsel.
        /// Hängt die Jahreszahl an die Belegnummer an.
        /// </summary>
        public List<Buchung> ErzeugeJahreswechselAfAs(
            IEnumerable<Buchung> afaBuchungen, int altesJahr)
        { /* ... */ }
    }
}
```

---

## 4. C++/CLI Bridge-Schicht (ECTBridge)

### 4.1 Konzept

ECTBridge.dll wird als C++/CLI-Projekt mit `/clr` kompiliert und kann sowohl native C++-Typen als auch .NET-Managed-Typen nutzen. Sie übernimmt drei Aufgaben:

1. **Typ-Marshalling:** CString ↔ System::String ↔ C#-string, native Pointer ↔ managed References.
2. **CEasyCashDoc-Adaption:** Eine neue Klasse `CEasyCashDocBridge`, die von MFC-`CDocument` erbt (für das MDI-Framework) und intern an `ECTEngine::BuchungsDocument` delegiert.
3. **Legacy-Kompatibilität:** Stellt weiterhin die alten CBuchung-Pointer-Interfaces bereit, die die Views erwarten – konvertiert aber intern von/zu managed Buchung-Objekten.

### 4.2 Marshalling-Helfer

```cpp
// ECTBridge/Marshalling.h
#pragma once
#include <msclr/marshal_cppstd.h>

namespace ECTBridge {
    inline System::String^ ToManaged(const CString& cs) {
        return gcnew System::String((LPCTSTR)cs);
    }
    inline CString ToNative(System::String^ s) {
        return CString(msclr::interop::marshal_as<std::string>(s).c_str());
    }
}
```

### 4.3 CEasyCashDocBridge (Skelett)

```cpp
// ECTBridge/EasyCashDocBridge.h
#pragma once

#using "ECTEngine.dll"
#include <afxwin.h>

class CEasyCashDocBridge : public CDocument
{
    DECLARE_DYNCREATE(CEasyCashDocBridge)

    // Managed Handle auf die C#-Engine
    gcroot<ECTEngine::BuchungsDocument^> m_engine;

    // Legacy-Linked-Lists (werden beim Serialize befüllt/gelesen)
    CBuchung *m_pEinnahmen;
    CBuchung *m_pAusgaben;
    CDauerbuchung *m_pDauerbuchungen;

public:
    CEasyCashDocBridge();

    // CDocument Overrides
    virtual void Serialize(CArchive& ar) override;
    virtual BOOL OnNewDocument() override;

    // --- Konvertierung: Linked-List ↔ Managed List ---

    /// Nach dem Einlesen (ar >> ...): C++-Linked-List → C#-List<Buchung>
    void LinkedListToManaged()
    {
        m_engine->Buchungen->Clear();
        for (CBuchung *p = m_pEinnahmen; p; p = p->next)
            m_engine->Buchungen->Add(ConvertBuchung(p,
                ECTEngine::Buchungsart::Einnahme));
        for (CBuchung *p = m_pAusgaben; p; p = p->next)
            m_engine->Buchungen->Add(ConvertBuchung(p,
                ECTEngine::Buchungsart::Ausgabe));
        // analog für Dauerbuchungen...
        m_engine->Sort();
    }

    /// Vor dem Speichern: C#-List<Buchung> → C++-Linked-Lists
    void ManagedToLinkedList()
    {
        FreeLinkedList(m_pEinnahmen);
        FreeLinkedList(m_pAusgaben);
        m_pEinnahmen = m_pAusgaben = nullptr;

        for each (auto b in m_engine->Buchungen)
        {
            CBuchung *pNeu = ConvertToNative(b);
            if (b->Art == ECTEngine::Buchungsart::Einnahme)
                AppendToList(&m_pEinnahmen, pNeu);
            else
                AppendToList(&m_pAusgaben, pNeu);
        }
    }

    // Kompatibilitäts-API für bestehende Views
    // (gibt native Pointer zurück, die aus den Linked-Lists stammen)
    CBuchung* GetEinnahmen() { return m_pEinnahmen; }
    CBuchung* GetAusgaben()  { return m_pAusgaben; }
    int GetBuchungszaehler() { return m_engine->Buchungszaehler; }

    // Neue Engine-API (für schrittweise Migration der Views)
    ECTEngine::BuchungsDocument^ GetEngine() { return m_engine; }

private:
    ECTEngine::Buchung^ ConvertBuchung(CBuchung *p,
        ECTEngine::Buchungsart art);
    CBuchung* ConvertToNative(ECTEngine::Buchung^ b);
    void AppendToList(CBuchung **ppHead, CBuchung *pNeu);
    void FreeLinkedList(CBuchung *pHead);
};
```

**Ablauf beim Laden:** `Serialize(CArchive& ar)` liest die .eca-Datei mit dem existierenden nativen C++-Code (inkl. Versionsprüfung für Doc-Version 1–13). Danach ruft es `LinkedListToManaged()` auf, um die Daten in die C#-Engine zu übertragen.

**Ablauf beim Speichern:** `ManagedToLinkedList()` baut die nativen Linked Lists aus den managed Daten auf, dann schreibt der existierende Serialize-Code diese wie gehabt in die .eca-Datei.

---

## 5. Migrationsphasen

### Phase 1: Fundament (ca. 2–3 Wochen)

**Ziel:** ECTEngine.dll baut, Unit Tests laufen, Bridge kompiliert.

Schritte:
1. Neues C#-Projekt `ECTEngine.csproj` zur Solution hinzufügen (.NET Framework 4.8).
2. Klassen `Betrag`, `ErweiterungStore` in C# implementieren mit vollständiger Unit-Test-Abdeckung.
3. Neues C++/CLI-Projekt `ECTBridge.vcxproj` anlegen (`/clr`, referenziert ECTEngine).
4. Marshalling-Helfer implementieren und mit Smoke-Tests verifizieren.
5. Build-Reihenfolge in der Solution konfigurieren: ECTEngine → ECTBridge → EasyCash.
6. **Ergebnis:** Die bisherige ECTIFace.lib wird weiterhin gelinkt; ECTBridge existiert parallel, wird aber noch nicht produktiv genutzt.

### Phase 2: Kernklassen migrieren (ca. 3–4 Wochen)

**Ziel:** `Buchung`, `Dauerbuchung`, `BuchungsDocument`, `AfACalculator` in C# funktionsfähig.

Schritte:
1. `Buchung` und `Dauerbuchung` als C#-POCOs implementieren.
2. `BuchungsDocument` mit Buchungslisten und Berechnungslogik (Saldo, Konten-Filterung) implementieren.
3. `AfACalculator` mit linearer AfA, Buchwert-Berechnung und Jahreswechsel-Logik.
4. `IBuchungsRepository` Interface definieren.
5. `LegacyArchiveRepository` implementieren: liest/schreibt das bestehende MFC-CArchive-Binärformat, damit alte .ect-Dateien weiterhin geladen werden können.
6. **Umfangreiche Unit-Tests** mit realistischen Testdaten (verschiedene USt-Sätze, AfA-Dauern, Split-Buchungen, neutrale Konten, Dauerbuchungs-Platzhalter).
7. **Ergebnis:** Die gesamte Geschäftslogik existiert doppelt (C++ und C#), die C#-Version ist per Tests validiert.

### Phase 3: Bridge aktivieren (ca. 2–3 Wochen)

**Ziel:** EasyCT.exe nutzt intern ECTEngine statt ECTIFace für die Geschäftslogik.

Schritte:
1. `CEasyCashDocBridge` vollständig implementieren: Serialize() liest/schreibt über `LegacyArchiveRepository`, alle Businesslogik delegiert an `BuchungsDocument`.
2. Kompatibilitäts-Wrapper für CBuchung* bereitstellen (native Struct, die von Managed-Daten befüllt wird), damit bestehende Views ohne Änderung weiterarbeiten.
3. In `EasyCash.cpp` die Dokumentenklassen-Registrierung von CEasyCashDoc auf CEasyCashDocBridge umstellen.
4. **Parallelbetrieb testen:** Öffne bestehende .ect-Dateien, buche, prüfe Formulare, teste Dauerbuchungen, teste Jahreswechsel.
5. ECTIFace.lib aus den Linker-Abhängigkeiten des Hauptprojekts entfernen (ECTBridge.lib tritt an dessen Stelle).
6. **Ergebnis:** ECTIFace wird nicht mehr gelinkt. ECTEngine + ECTBridge übernehmen.

### Phase 4: Plugin-Kompatibilität (ca. 1–2 Wochen)

**Ziel:** Bestehende Plugins (ECTElster, ECTDatev, ECTImport) funktionieren weiterhin.

Schritte:
1. Prüfen, ob EasyCTX.ocx direkt auf ECTIFace.dll zugreift oder nur über die Exe. Falls direkt: COM-Wrapper in ECTBridge bereitstellen, der die alte DLL-Schnittstelle emuliert.
2. Sicherstellen, dass ECTIFace.dll weiterhin als leere Stub-DLL ausgeliefert wird (falls Plugins per LoadLibrary darauf zugreifen), die an ECTBridge weiterleitet.
3. Plugin-Tests mit ECTDatev (C#-Plugin, nutzt COM-Interop) und ECTImport durchführen.
4. **Ergebnis:** Kein Plugin bricht.

### Phase 5: Aufräumen und Zukunft (fortlaufend)

Schritte:
1. Schrittweise View-Methoden umstellen: statt `CBuchung*` die managed `Buchung`-Objekte über die Engine-API nutzen.
2. CEasyCashDoc-Header aus den Views entfernen, durch ECTBridge-Header ersetzen.
3. ECTIFace-Quellcode als deprecated markieren, aber im Repo belassen.
4. Langfristig: Neue Features nur noch in ECTEngine implementieren.

---

## 6. Risiken und Gegenmaßnahmen

| Risiko | Wahrscheinlichkeit | Gegenmaßnahme |
|--------|-------------------|---------------|
| **Linked-List → List-Konvertierung**: Die Views greifen über `pDoc->Einnahmen` und `for(p=...; p; p=p->next)` auf die Linked Lists zu. Die Bridge muss diese Zugriffsmuster weiterhin bedienen. | Hoch | Die Bridge hält parallel die nativen Linked Lists (für Views) und die managed List (für Engine). `LinkedListToManaged()` / `ManagedToLinkedList()` synchronisiert bei Laden/Speichern. Views werden schrittweise auf Engine-API umgestellt. |
| **Binärformat-Kompatibilität**: MFC CArchive serialisiert mit `DECLARE_SERIAL`-Versionierung. Das Dokument hat Version 13 mit Feldern, die in Version 7, 8, 10, 11, 12 und 13 dazukamen. Die Bridge muss alle Versionsweichen exakt nachbilden. | Hoch | `Serialize()` bleibt als nativer C++-Code in der Bridge. Kein Reverse-Engineering des Binärformats in C#. Daten werden nach dem Lesen in managed Objekte konvertiert und vor dem Schreiben zurückkonvertiert. |
| **/clr und MFC**: Mixed-Mode-DLLs mit MFC können zu subtilen Initialisierungsproblemen führen. | Mittel | `#pragma managed(push, off)` für MFC-Header. DllMain-Initialisierung beachten. Loader-Lock vermeiden. |
| **Performance**: Marshalling bei jedem Zugriff auf Buchungsdaten könnte bei großen Journalen (>10.000 Buchungen) spürbar werden. | Niedrig | Batch-Marshalling: Listen einmalig konvertieren, nicht pro Zelle. Die managed Buchungsliste cached halten. |
| **Plugin-Bruch**: Plugins könnten intern auf nicht-dokumentierte ECTIFace-Interna zugreifen. | Mittel | Stub-DLL bereitstellen. Plugin-Autoren frühzeitig informieren. |
| **Zwei-Code-Basen-Drift**: Während Phase 2 existiert die Logik doppelt. | Hoch | Phase 2 und 3 eng takten. Keine neuen Features in ECTIFace während der Migration. |

---

## 7. Noch benötigte Dateien

Die zentralen Dateien liegen vor:
- ✅ `ECTIFace/EasyCashDoc.h` – Klassendeklarationen, Felder, Versionshistorie
- ✅ `ECTIFace/EasyCashDoc.cpp` – Serialize-Protokoll, GetNetto()-Algorithmus, Jahreswechsel, Sortierung, Summenberechnung

Für die vollständige Implementierung wären ergänzend noch hilfreich:
1. **ECTIFace/ECTIFace.def** oder Projektdatei – um zu sehen, welche C-Funktionen (`currency_to_int`, `GetErweiterungKey`, etc.) als DLL-Exports deklariert sind und in der Bridge weiterhin exportiert werden müssen.
2. **EasyCTX/\*** – Die COM-Schnittstelle (OCX), um die Plugin-Kompatibilität in Phase 4 sicherzustellen.

---

## 8. Ergänzende Erkenntnisse aus der Code-Analyse

### 8.1 Persistenzformat und Dateistruktur

Aus den Codeanalysen (easycashview.cpp, todo.txt, tipps.txt, getprogramversion.h) ergibt sich folgendes Bild:

**Dateiformat:** `.eca`-Dateien (z.B. `Jahr2024.eca`) – pro Buchungsjahr eine Datei, serialisiert über MFC `CArchive`. Die Serialisierung erfolgt über `CEasyCashDoc::Serialize(CArchive& ar)` und nutzt vermutlich die MFC-Versionierung (`IMPLEMENT_SERIAL`). Jede .eca-Datei enthält genau ein Buchungsjahr.

**Einstellungen:** `easyct.ini` – enthält den Kontenrahmen, Buchungs-Presets, Mandant-Informationen und weitere Konfiguration. Wird separat von den Buchungsdaten gespeichert und muss bei Datensicherungen mitgesichert werden.

**Datensicherung:** Beim Jahreswechsel wird ein Verzeichnis `Backup{JJJJ}-Jahreswechsel` angelegt mit: der Buchungsdatei (`.eca`), Einstellungsdaten (`easyct.ini`) und CSV-Exporten (getrennt für Einnahmen/Ausgaben: `easyct_E.csv`, `easyct_A.csv`).

**Implikation für ECTEngine:** Der `LegacyArchiveRepository` muss das MFC-CArchive-Binärformat Byte-kompatibel lesen und schreiben können. Die sicherste Strategie ist, die Deserialisierung in der C++/CLI-Bridge zu belassen (CArchive bleibt nativ) und nur die extrahierten Daten als managed Objekte an ECTEngine zu übergeben.

### 8.2 Zusätzliche CEasyCashDoc-Felder

Aus den Nutzungen in `easycashview.cpp` sind folgende zusätzliche Felder des Dokumentmodells ersichtlich:

| Feld | Typ (C++) | Bedeutung |
|------|-----------|-----------|
| `nJahr` | int | Aktuelles Buchungsjahr |
| `nLaufendeBuchungsnummerFuerEinnahmen` | int | Auto-Inkrement für Einnahmen-Belegnummern |
| `nLaufendeBuchungsnummerFuerAusgaben` | int | Auto-Inkrement für Ausgaben-Belegnummern |
| `einnahmen_posten_name[100]` | CString[] | Einnahmen-Kontennamen |
| `ausgaben_posten_name[100]` | CString[] | Ausgaben-Kontennamen |

Die Kontenlisten werden mit bis zu je 100 Slots indiziert und in den Einstellungen E/Ü-Konten konfiguriert. Darüber hinaus werden auch Konten aus tatsächlichen Buchungen berücksichtigt, die nicht in den vordefinierten 100 Slots vorkommen.

### 8.3 Dauerbuchung – Detailbefunde

Aus der Code-Analyse ergeben sich folgende Präzisierungen:

- **`AktualisiertBisDatum`** (vermutlich `COleDateTime`) – das Datum, bis zu dem die Dauerbuchung zuletzt ausgeführt wurde. Kann manuell zurückgesetzt werden, um Dauerbuchungen erneut auszuführen.
- **Platzhalter** werden sowohl in Beschreibung als auch in Belegnummer unterstützt.
- **Jahreswechsel-Prüfung:** Vor dem Jahreswechsel wird geprüft, ob Dauerbuchungen bis Dezember ausgeführt wurden. Nicht komplett ausgeführte Dauerbuchungen erzeugen eine Warnung.
- **Buchungsnummern:** Bei der Ausführung von Dauerbuchungen werden `nLaufendeBuchungsnummerFuerEinnahmen` bzw. `nLaufendeBuchungsnummerFuerAusgaben` automatisch hochgezählt.

### 8.4 USt-Voranmeldung und Formularsystem

Das Formularsystem ist tief in die View-Schicht integriert und referenziert spezifische EÜR/UVA-Felder:

- EÜR-Felder: 1140 (reine USt), 1185 (reine VSt)
- UVA-Felder: 39 (Sondervorauszahlung/Dauerfristverlängerung), 43 (weitere steuerfreie Umsätze mit Vorsteuerabzug), 90 (AT: sonstige Berichtigungen)

**Empfehlung:** Das Formularsystem (Formularabschnitt, Formularfeld) zunächst NICHT migrieren. Es ist stark mit der View-Schicht verflochten und gehört zur Darstellungslogik, nicht zur Geschäftslogik. Die Berechnung der Formularwerte (Summen pro EÜR-Feld) kann allerdings in ECTEngine als Service implementiert werden, der von der View-Schicht aufgerufen wird.

### 8.5 Aktualisiertes C#-Modell für BuchungsDocument

Basierend auf den zusätzlichen Erkenntnissen:

```csharp
public class BuchungsDocument
{
    // Kernlisten
    public List<Buchung> Buchungen { get; } = new();
    public List<Dauerbuchung> Dauerbuchungen { get; } = new();

    // Buchungsjahr
    public int Jahr { get; set; }

    // Auto-Inkrement Belegnummern
    public int LaufendeBelegnummerEinnahmen { get; set; }
    public int LaufendeBelegnummerAusgaben { get; set; }

    // Kontenrahmen (je max. 100)
    public string[] EinnahmenKonten { get; } = new string[100];
    public string[] AusgabenKonten { get; } = new string[100];

    // Mandant
    public string MandantName { get; set; } = "";
    public string Datenverzeichnis { get; set; } = "";

    // Umsatzsteuer-Konfiguration
    public decimal? Sondervorauszahlung { get; set; }

    // Methoden
    public string NaechsteBelegnummer(Buchungsart art) { ... }
    public Betrag Saldo(DateTime? von = null, DateTime? bis = null) { ... }
    public Betrag SaldoNetto(DateTime? von = null, DateTime? bis = null) { ... }
    public Betrag SaldoFuerKonto(string konto, ...) { ... }

    // Dauerbuchungen
    public List<Buchung> FuehreDauerbuchungenAus(DateTime stichtag) { ... }
    public List<string> PruefeDauerbuchungenVollstaendig() { ... }

    // Jahreswechsel
    public BuchungsDocument Jahreswechsel(int neuesJahr) { ... }

    // EÜR-Feldwerte berechnen
    public Dictionary<int, Betrag> BerechneEuerFeldwerte() { ... }

    // UVA-Werte berechnen
    public Dictionary<int, Betrag> BerechneUvaWerte(
        int monatVon, int monatBis) { ... }
}
```

### 8.6 Exaktes Serialize-Protokoll (.eca-Dateiformat)

Die folgenden Protokolle sind direkt aus `EasyCashDoc.cpp` extrahiert. Sie definieren die exakte Byte-Reihenfolge der `.eca`-Dateien und sind die Grundlage für die Bridge-Implementierung.

**Magic Key und Versionsheader:**
```
Bytes 0-3:  char[4] "ECDo"    // Magic Key
Bytes 4-7:  DWORD VERSION     // Dokumentversion (aktuell 13)
```

**CEasyCashDoc::Serialize (Schreib-Reihenfolge):**
```
[magic + version header]
BOOL  hasEinnahmen           → if true: CObject* Einnahmen (rekursive Liste)
BOOL  hasAusgaben            → if true: CObject* Ausgaben  (rekursive Liste)
int   Buchungszaehler
BOOL  hasDauerbuchungen      → if true: CObject* Dauerbuchungen (rekursive Liste)
≥v2:  int nLaufendeBuchungsnummerFuerEinnahmen
      int nLaufendeBuchungsnummerFuerAusgaben
≥v4:  int nJahr
≥v6:  CString csWaehrung
      CString csUrspruenglicheWaehrung
≥v7:  CString Erweiterung
      int AbschreibungGenauigkeit
≥v8:  int nLaufendeBuchungsnummerFuerBank
      int nLaufendeBuchungsnummerFuerKasse
≥v13: int nNachfrageIntervall
      CTime ctNachfrageTermin
```

**CBuchung::Serialize (pro Knoten in der Linked List):**
```
[CObject-Header mit Schema-Version via VERSIONABLE_SCHEMA|VERSION]
int     Betrag                    // Brutto in Cent
int     MWSt                      // Promille (ab v5; davor Ganzzahl×1000)
int     AbschreibungNr
int     AbschreibungJahre
CString Beschreibung
CTime   Datum
≥v3:  CString Konto
≥v7:  CString Belegnummer
      CString Erweiterung
      int     AbschreibungRestwert
      BOOL    AbschreibungDegressiv
      int     AbschreibungSatz
≥v10: int     AbschreibungGenauigkeit
≥v11: CString Bestandskonto
      CString Betrieb
CObject* next                     // nächster Knoten oder NULL
```

**CDauerbuchung::Serialize (pro Knoten):**
```
[CObject-Header mit Schema-Version]
int     Betrag                    // Brutto in Cent
int     MWSt                      // Promille (ab v5)
CString Beschreibung
int     Buchungstyp               // 0=Einnahmen, 1=Ausgaben
int     Intervall                 // 1..5
CTime   VonDatum
CTime   BisDatum
int     Buchungstag
CTime   AktualisiertBisDatum
≥v3:  CString Konto
≥v7:  CString Belegnummer
      CString Erweiterung
≥v12: CString Bestandskonto
      CString Betrieb
CObject* next                     // nächster Knoten oder NULL
```

**CBetrag::GetNetto()-Algorithmus (exakt für C#-Reimplementierung):**
```
if (|Betrag| ≤ 1.000.000 Cent UND MWSt ist ganzzahlig):
    netto = Betrag × 1000 / (100 + MWSt/1000)
    netto = (netto ± 5) / 10          // kaufmännische Rundung
else:
    temp = Betrag × 100.0 / (100.0 + MWSt/1000.0)
    netto = round(temp)               // double-Rundung
```

### 8.7 Strategie für die Serialize-Bridge

Da das MFC-CArchive-Format `IMPLEMENT_SERIAL` mit eingebetteten Klassennamen und Versionsschemata nutzt und die Linked Lists rekursiv über `ar << next` (CObject-Pointer-Serialisierung) serialisiert werden, ist eine Reimplementierung in purem C# **nicht praktikabel**.

**Empfohlene Implementierung:**

```cpp
// In CEasyCashDocBridge::Serialize(CArchive& ar):
void CEasyCashDocBridge::Serialize(CArchive& ar)
{
    // === SCHRITT 1: Natives Serialize wie bisher ===
    // Der gesamte existierende Serialize-Code bleibt 1:1 erhalten.
    // Er liest/schreibt die nativen Linked Lists.

    if (ar.IsStoring())
    {
        // Vor dem Speichern: Managed → Native synchronisieren
        ManagedToLinkedList();
    }

    // Existierender Serialize-Code (unverändert aus CEasyCashDoc)
    CEasyCashDoc_OriginalSerialize(ar);  // oder inline

    if (!ar.IsStoring())
    {
        // Nach dem Laden: Native → Managed synchronisieren
        LinkedListToManaged();
    }
}
```

Diese Strategie bedeutet: **Null Risiko** bei der Dateiformatkompatibilität. Der bewährte C++-Serialize-Code wird weiterverwendet. Die .NET-Engine arbeitet nur mit den In-Memory-Daten, nie direkt mit dem Dateistrom.
