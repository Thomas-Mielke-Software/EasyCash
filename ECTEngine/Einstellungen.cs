using System;
using System.Collections.Generic;
using System.Globalization;

namespace ECTEngine
{
    /// <summary>Ein Betrieb aus der ini-Sektion [Betriebe].</summary>
    public sealed class Betrieb
    {
        public string Name            { get; }
        public string Unternehmensart { get; }  // kann Tabs enthalten, nicht auflösen
        public int    Icon            { get; }

        public Betrieb(string name, string unternehmensart, int icon)
        {
            Name            = name            ?? "";
            Unternehmensart = unternehmensart ?? "";
            Icon            = icon;
        }
    }

    /// <summary>Ein Bestandskonto aus der ini-Sektion [Bestandskonten].</summary>
    public sealed class Bestandskonto
    {
        public string                            Name  { get; }
        public int                               Icon  { get; }
        /// <summary>Anfangssalden nach Buchungsjahr (leer wenn kein Eintrag in ini).</summary>
        public IReadOnlyDictionary<int, decimal> Saldo { get; }

        public Bestandskonto(string name, int icon, Dictionary<int, decimal> saldo)
        {
            Name  = name ?? "";
            Icon  = icon;
            Saldo = saldo ?? new Dictionary<int, decimal>();
        }
    }

    /// <summary>Ein Buchungsposten-Preset aus der ini-Sektion [Buchungsposten].</summary>
    public sealed class Preset
    {
        public string Text    { get; }
        public bool   Ausgabe { get; }  // false=Einnahmen, true=Ausgaben
        public int    Mwst    { get; }  // Festkomma x1000 (19000 = 19 %)
        public int    AfaJ    { get; }
        public string Konto   { get; }
        /// <summary>Freitext-Notiz (Erinnerungen/Detailinfos zur Buchung).
        /// Wird im Buchen-Dialog beim Auswählen des Presets kurz als
        /// Balloon angezeigt.</summary>
        public string Notiz   { get; }
        /// <summary>Degressive AfA (statt linear).</summary>
        public bool   Degressiv { get; }
        /// <summary>Degressiver AfA-Satz in Prozent (nur relevant bei Degressiv).</summary>
        public int    AfaSatz { get; }

        public Preset(string text, bool ausgabe, int mwst, int afaj, string konto,
            string notiz = "", bool degressiv = false, int afaSatz = 0)
        {
            Text      = text    ?? "";
            Ausgabe   = ausgabe;
            Mwst      = mwst;
            AfaJ      = afaj;
            Konto     = konto   ?? "";
            Notiz     = notiz   ?? "";
            Degressiv = degressiv;
            AfaSatz   = afaSatz;
        }

        public bool IstLeer => string.IsNullOrEmpty(Text) && string.IsNullOrEmpty(Konto);
    }

    /// <summary>
    /// Globaler Key-Value-Cache für die easyct.ini-Einstellungen des aktiven
    /// Mandanten. Loest das alte Pro-Dokument-Modell ab, in dem jede
    /// CEasyCashView ihre eigenen einstellungen1..5-Property-Pages hielt
    /// (was bei mehreren offenen MDI-Dokumenten zu Inkonsistenzen führte).
    ///
    /// Schlüsselformat (Plugin-API-kompatibel):
    ///   - Kurzform: "fname"  -> Sektion "Finanzamt", Ini-Key "name"
    ///   - Explizit: "[Sektion]Key"
    ///
    /// Variante X: Der Cache speichert den Schlüssel genau so wie vom
    /// Aufrufer übergeben. Konvention: Kurzform (wie Plugin-Interface).
    /// </summary>
    public static class Einstellungen
    {
        private static readonly Dictionary<string, string> _cache =
            new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        // Schützt _cache, damit Hintergrund-Persistenz (z.B. Konten-Reorder)
        // und UI-Lesezugriffe sich nicht ins Gehege kommen. Nur die kurze
        // Dictionary-Operation wird gesperrt -- NICHT der ini-Schreibvorgang
        // (WertGeaendert), damit Disk-I/O die Lesezugriffe nicht blockiert.
        private static readonly object _gate = new object();

        private static IReadOnlyList<string>       _einnahmenKonten = new List<string>();
        private static IReadOnlyList<string>       _ausgabenKonten  = new List<string>();
        private static IReadOnlyList<Preset>       _presets         = new List<Preset>();
        private static IReadOnlyList<Betrieb>      _betriebe        = new List<Betrieb>();
        private static IReadOnlyList<Bestandskonto> _bestandskonten  = new List<Bestandskonto>();

        /// <summary>
        /// Wird beim Speichern eines Wertes ausgelöst. Die Bridge hängt
        /// sich darauf, um den Wert sofort in die ini-Datei zu schreiben.
        /// </summary>
        public static event Action<string, string> WertGeaendert;

        /// <summary>
        /// Cache komplett ersetzen (von der Bridge nach ini-Lesen aufgerufen).
        /// Loest KEINE WertGeaendert-Events aus.
        /// </summary>
        public static void LadeAusBridge(IDictionary<string, string> daten)
        {
            lock (_gate)
            {
                _cache.Clear();
                if (daten != null)
                    foreach (var kv in daten)
                        _cache[kv.Key] = kv.Value ?? "";
            }
            BaueListenAuf();
        }

        /// <summary>Cache leeren (z.B. bei Mandantenwechsel vor neuem Laden).</summary>
        public static void Leere()
        {
            lock (_gate) { _cache.Clear(); }
            _einnahmenKonten = new List<string>();
            _ausgabenKonten  = new List<string>();
            _presets         = new List<Preset>();
            _betriebe        = new List<Betrieb>();
            _bestandskonten  = new List<Bestandskonto>();
        }

        /// <summary>
        /// Liefert den Wert zum Schlüssel, oder Leerstring bei Cache-Miss.
        /// </summary>
        public static string Hole(string key)
        {
            if (string.IsNullOrEmpty(key)) return "";
            lock (_gate) { return _cache.TryGetValue(key, out var v) ? v : ""; }
        }

        /// <summary>
        /// Speichert den Wert im Cache und loest WertGeaendert aus
        /// (Bridge schreibt dann in die ini-Datei).
        /// </summary>
        public static void Speichere(string key, string value)
        {
            if (string.IsNullOrEmpty(key)) return;
            value = value ?? "";
            lock (_gate) { _cache[key] = value; }
            // WertGeaendert (ini-Schreibvorgang) bewusst AUSSERHALB des Locks,
            // damit Disk-I/O keine UI-Lesezugriffe blockiert.
            WertGeaendert?.Invoke(key, value);
        }

        /// <summary>
        /// Wird ausgelöst, um eine GANZE ini-Sektion auf einmal zu schreiben
        /// (statt vieler Einzel-Keys). Die Bridge schreibt das per
        /// WritePrivateProfileSection -- drastisch weniger Datei-I/O.
        /// Args: (ini-Sektionsname, iniKey -> Wert).
        /// </summary>
        public static event Action<string, IReadOnlyDictionary<string, string>> SektionGeaendert;

        /// <summary>Aktualisiert NUR den Cache (kein Event/keine ini). Für
        /// gebündeltes Schreiben: erst alle Cache-Keys setzen, dann die
        /// Sektion(en) per <see cref="SchreibeSektion"/> in die ini spülen.</summary>
        public static void SetzeCacheNur(string key, string value)
        {
            if (string.IsNullOrEmpty(key)) return;
            lock (_gate) { _cache[key] = value ?? ""; }
        }

        /// <summary>Schreibt eine komplette ini-Sektion (löst SektionGeaendert aus).
        /// Aktualisiert NICHT den Cache -- das macht der Aufrufer per
        /// <see cref="SetzeCacheNur"/>.</summary>
        public static void SchreibeSektion(string iniSektion, IReadOnlyDictionary<string, string> eintraege)
        {
            if (string.IsNullOrEmpty(iniSektion) || eintraege == null) return;
            SektionGeaendert?.Invoke(iniSektion, eintraege);
        }

        public static int HoleInt(string key, int defaultValue = 0)
        {
            return int.TryParse(Hole(key), NumberStyles.Integer,
                CultureInfo.InvariantCulture, out var n) ? n : defaultValue;
        }

        public static bool HoleBool(string key, bool defaultValue = false)
        {
            var s = Hole(key);
            if (string.IsNullOrEmpty(s)) return defaultValue;
            return s != "0" && !s.Equals("false", StringComparison.OrdinalIgnoreCase);
        }

        public static void Speichere(string key, int value)
        {
            Speichere(key, value.ToString(CultureInfo.InvariantCulture));
        }

        public static void Speichere(string key, bool value)
        {
            Speichere(key, value ? "1" : "0");
        }

        // ---------------------------------------------------------------------
        // Listen-Properties (schreibgeschützt, nach LadeAusBridge aktuell)
        // ---------------------------------------------------------------------

        /// <summary>Einnahmen-Konten aus [EinnahmenRechnungsposten] (Cache-Praefix "e").</summary>
        public static IReadOnlyList<string> EinnahmenKonten => _einnahmenKonten;

        /// <summary>Ausgaben-Konten aus [AusgabenRechnungsposten] (Cache-Praefix "a").</summary>
        public static IReadOnlyList<string> AusgabenKonten => _ausgabenKonten;

        /// <summary>
        /// Buchungsposten-Presets aus [Buchungsposten].
        /// Immer alle 100; Lücken ergeben Preset-Objekte mit IstLeer==true.
        /// </summary>
        public static IReadOnlyList<Preset> Presets => _presets;

        /// <summary>Betriebe aus [Betriebe] (sequenziell bis erstes leeres Name).</summary>
        public static IReadOnlyList<Betrieb> Betriebe => _betriebe;

        /// <summary>
        /// Bestandskonten aus [Bestandskonten] (sequenziell bis erstes leeres Name).
        /// Jedes Bestandskonto hat ein Saldo-Dictionary (Jahr => Anfangssaldo in Euro).
        /// </summary>
        public static IReadOnlyList<Bestandskonto> Bestandskonten => _bestandskonten;

        /// <summary>Diagnose: alle aktuell gecachten Schlüssel.</summary>
        public static IEnumerable<string> AlleSchluessel => _cache.Keys;

        // ---------------------------------------------------------------------

        /// <summary>
        /// Schreibt ein Buchungsposten-Preset (Index 0-99) in den Cache/ini
        /// und baut die Listen neu auf. Verwendet die Bracket-Cache-Form
        /// "[Buchungsposten]NN..." -- exakt die Form, in der
        /// <see cref="BaueListenAuf"/> liest, damit das Ergebnis sofort
        /// (ohne ECT_LadeEinstellungen) sichtbar ist.
        /// </summary>
        public static void SpeicherePreset(int index, Preset p)
        {
            if (index < 0 || index > 99 || p == null) return;
            var pfx = "[Buchungsposten]" + index.ToString("D2");
            Speichere(pfx + "Text", p.Text);
            Speichere(pfx + "Ausg", p.Ausgabe ? "1" : "0");
            Speichere(pfx + "MWSt", p.Mwst);
            Speichere(pfx + "AfAJ", p.AfaJ);
            Speichere(pfx + "Rech", p.Konto);
            Speichere(pfx + "Notiz", p.Notiz);
            Speichere(pfx + "Degr", p.Degressiv ? "1" : "0");
            Speichere(pfx + "AfASatz", p.AfaSatz);
            BaueListenAuf();
        }

        /// <summary>Baut die abgeleiteten Listen (Presets, Konten, …) aus dem
        /// aktuellen Cache neu auf. Nach direkten Speichere-Aufrufen nötig,
        /// wenn die Listen-Properties sofort aktuell sein sollen.</summary>
        public static void NeuAufbauen() => BaueListenAuf();

        /// <summary>Baut NUR die Einnahmen-/Ausgaben-Kontenlisten neu auf
        /// (günstig). Für das Konten-Umsortieren -- der teure Komplett-Rebuild
        /// (Presets/Betriebe/Bestandskonten mit 60-Jahres-Schleife) ist dafür
        /// unnötig.</summary>
        public static void NeuAufbauenKonten()
        {
            var ek = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var v = Hole("e" + i.ToString("D2", CultureInfo.InvariantCulture));
                if (string.IsNullOrEmpty(v)) break;
                ek.Add(v);
            }
            _einnahmenKonten = ek;

            var ak = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var v = Hole("a" + i.ToString("D2", CultureInfo.InvariantCulture));
                if (string.IsNullOrEmpty(v)) break;
                ak.Add(v);
            }
            _ausgabenKonten = ak;
        }

        private static void BaueListenAuf()
        {
            // EinnahmenKonten: e00, e01, ... stopp bei erstem Leerstring
            var ek = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var val = Hole("e" + i.ToString("D2"));
                if (string.IsNullOrEmpty(val)) break;
                ek.Add(val);
            }
            _einnahmenKonten = ek;

            // AusgabenKonten: a00, a01, ... stopp bei erstem Leerstring
            var ak = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var val = Hole("a" + i.ToString("D2"));
                if (string.IsNullOrEmpty(val)) break;
                ak.Add(val);
            }
            _ausgabenKonten = ak;

            // WICHTIG -- Cache-Schluesselformat:
            // ECT_LadeEinstellungen legt Werte aus nicht-praefixierten ini-
            // Sektionen (alles ausser Finanzamt/Einnahmen-/AusgabenRechnungs-
            // posten) unter der Bracket-Form "[Sektion]IniKey" ab (siehe
            // KuerzelFuerCache in EinstellungenExports.cpp). Presets, Betriebe
            // und Bestandskonten liegen in solchen Sektionen ([Buchungsposten],
            // [Betriebe], [Bestandskonten]) und MUESSEN deshalb mit dem
            // Bracket-Praefix nachgeschlagen werden. Frueher speicherte der
            // Cache fuer unbekannte Sektionen nur den nackten iniKey -- seit
            // der Umstellung auf Bracket-Form (Fix fuer den CSV-Plugin-Lookup)
            // lieferte der nackte Lookup hier leere Listen (u.a. fehlte der
            // Bestandskonto-Anfangssaldo im Journal).

            // Presets: immer alle 100, Lücken erlaubt
            var ps = new List<Preset>(100);
            for (int i = 0; i < 100; i++)
            {
                var pfx     = "[Buchungsposten]" + i.ToString("D2");
                var text    = Hole(pfx + "Text");
                var ausgabe = Hole(pfx + "Ausg") == "1";
                var mwst    = HoleInt(pfx + "MWSt");
                var afaj    = HoleInt(pfx + "AfAJ");
                var konto   = Hole(pfx + "Rech");
                var notiz   = Hole(pfx + "Notiz");
                var degr    = Hole(pfx + "Degr") == "1";
                var afaSatz = HoleInt(pfx + "AfASatz");
                ps.Add(new Preset(text, ausgabe, mwst, afaj, konto, notiz, degr, afaSatz));
            }
            _presets = ps;

            // Betriebe: Betrieb00Name, Betrieb01Name, ... stopp bei erstem leeren Namen
            var bt = new List<Betrieb>();
            for (int i = 0; i < 100; i++)
            {
                var pfx  = "[Betriebe]Betrieb" + i.ToString("D2");
                var name = Hole(pfx + "Name");
                if (string.IsNullOrEmpty(name)) break;
                var ua   = Hole(pfx + "Unternehmensart");
                var icon = HoleInt(pfx + "Icon");
                bt.Add(new Betrieb(name, ua, icon));
            }
            _betriebe = bt;

            // Bestandskonten: Bestandskonto00Name usw., stopp bei erstem leeren Namen;
            // Saldo-Eintraege für jedes Jahr 1990-2049 pruefen
            var bk = new List<Bestandskonto>();
            for (int i = 0; i < 100; i++)
            {
                var pfx  = "[Bestandskonten]Bestandskonto" + i.ToString("D2");
                var name = Hole(pfx + "Name");
                if (string.IsNullOrEmpty(name)) break;
                var icon  = HoleInt(pfx + "Icon");
                var saldo = new Dictionary<int, decimal>();
                for (int year = 1990; year <= 2049; year++)
                {
                    var saldoStr = Hole(pfx + "Saldo" + year);
                    if (!string.IsNullOrEmpty(saldoStr))
                        saldo[year] = ParseSaldo(saldoStr);
                }
                bk.Add(new Bestandskonto(name, icon, saldo));
            }
            _bestandskonten = bk;
        }

        private static readonly CultureInfo _deDe = new CultureInfo("de-DE");

        private static decimal ParseSaldo(string s)
        {
            if (string.IsNullOrWhiteSpace(s)) return 0m;
            if (decimal.TryParse(s, NumberStyles.Number, _deDe, out decimal d))
                return d;
            // Fallback: Punkt als Dezimaltrennzeichen
            if (decimal.TryParse(s.Replace(',', '.'), NumberStyles.Number,
                CultureInfo.InvariantCulture, out d))
                return d;
            return 0m;
        }
    }
}
