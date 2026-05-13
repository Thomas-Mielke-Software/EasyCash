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

        public Preset(string text, bool ausgabe, int mwst, int afaj, string konto)
        {
            Text    = text    ?? "";
            Ausgabe = ausgabe;
            Mwst    = mwst;
            AfaJ    = afaj;
            Konto   = konto   ?? "";
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
            _cache.Clear();
            if (daten != null)
                foreach (var kv in daten)
                    _cache[kv.Key] = kv.Value ?? "";
            BaueListenAuf();
        }

        /// <summary>Cache leeren (z.B. bei Mandantenwechsel vor neuem Laden).</summary>
        public static void Leere()
        {
            _cache.Clear();
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
            return _cache.TryGetValue(key, out var v) ? v : "";
        }

        /// <summary>
        /// Speichert den Wert im Cache und loest WertGeaendert aus
        /// (Bridge schreibt dann in die ini-Datei).
        /// </summary>
        public static void Speichere(string key, string value)
        {
            if (string.IsNullOrEmpty(key)) return;
            value = value ?? "";
            _cache[key] = value;
            WertGeaendert?.Invoke(key, value);
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

            // Presets: immer alle 100, Lücken erlaubt
            var ps = new List<Preset>(100);
            for (int i = 0; i < 100; i++)
            {
                var pfx     = i.ToString("D2");
                var text    = Hole(pfx + "Text");
                var ausgabe = Hole(pfx + "Ausg") == "1";
                var mwst    = HoleInt(pfx + "MWSt");
                var afaj    = HoleInt(pfx + "AfAJ");
                var konto   = Hole(pfx + "Rech");
                ps.Add(new Preset(text, ausgabe, mwst, afaj, konto));
            }
            _presets = ps;

            // Betriebe: Betrieb00Name, Betrieb01Name, ... stopp bei erstem leeren Namen
            var bt = new List<Betrieb>();
            for (int i = 0; i < 100; i++)
            {
                var pfx  = i.ToString("D2");
                var name = Hole("Betrieb" + pfx + "Name");
                if (string.IsNullOrEmpty(name)) break;
                var ua   = Hole("Betrieb" + pfx + "Unternehmensart");
                var icon = HoleInt("Betrieb" + pfx + "Icon");
                bt.Add(new Betrieb(name, ua, icon));
            }
            _betriebe = bt;

            // Bestandskonten: Bestandskonto00Name usw., stopp bei erstem leeren Namen;
            // Saldo-Eintraege für jedes Jahr 1990-2049 pruefen
            var bk = new List<Bestandskonto>();
            for (int i = 0; i < 100; i++)
            {
                var pfx  = i.ToString("D2");
                var name = Hole("Bestandskonto" + pfx + "Name");
                if (string.IsNullOrEmpty(name)) break;
                var icon  = HoleInt("Bestandskonto" + pfx + "Icon");
                var saldo = new Dictionary<int, decimal>();
                for (int year = 1990; year <= 2049; year++)
                {
                    var saldoStr = Hole("Bestandskonto" + pfx + "Saldo" + year);
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
