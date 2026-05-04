using System;
using System.Collections.Generic;
using System.Globalization;

namespace ECTEngine
{
    /// <summary>
    /// Ein Buchungsposten-Preset aus der ini-Sektion [Buchungsposten].
    /// </summary>
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
    /// Globaler Key-Value-Cache fuer die easyct.ini-Einstellungen des aktiven
    /// Mandanten. Loest das alte Pro-Dokument-Modell ab, in dem jede
    /// CEasyCashView ihre eigenen einstellungen1..5-Property-Pages hielt
    /// (was bei mehreren offenen MDI-Dokumenten zu Inkonsistenzen fuehrte).
    ///
    /// Schluesselformat (Plugin-API-kompatibel):
    ///   - Kurzform: "fname"  -> Sektion "Finanzamt", Ini-Key "name"
    ///                "monatliche_voranmeldung" -> Sektion "Allgemein"
    ///                (Aufloesung passiert in der Bridge via IniSektion()).
    ///   - Explizit: "[Sektion]Key"
    ///
    /// Variante X: Der Cache speichert den Schluessel genau so wie vom
    /// Aufrufer uebergeben. Wer mal "fname" und mal "[Finanzamt]name"
    /// nutzt, erzeugt zwei getrennte Cache-Eintraege -- die Konvention
    /// im Code ist die Kurzform (so wie auch das Plugin-Interface es nutzt).
    /// </summary>
    public static class Einstellungen
    {
        private static readonly Dictionary<string, string> _cache =
            new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        private static IReadOnlyList<string> _einnahmenKonten = new List<string>();
        private static IReadOnlyList<string> _ausgabenKonten  = new List<string>();
        private static IReadOnlyList<Preset> _presets         = new List<Preset>();

        /// <summary>
        /// Wird beim Speichern eines Wertes ausgeloest. Die Bridge haengt
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
        }

        /// <summary>
        /// Liefert den Wert zum Schluessel, oder Leerstring bei Cache-Miss.
        /// Kein impliziter ini-Fallback: der Cache muss vor dem ersten
        /// Hole() per LadeAusBridge() befuellt sein.
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

        // ─────────────────────────────────────────────────────────────────────
        // Listen-Properties (schreibgeschuetzt, nach LadeAusBridge aktuell)
        // ─────────────────────────────────────────────────────────────────────

        /// <summary>
        /// Einnahmen-Konten aus [EinnahmenRechnungsposten] (Cache-Praefix "e").
        /// Nur bis zum ersten leeren Eintrag befullt.
        /// </summary>
        public static IReadOnlyList<string> EinnahmenKonten => _einnahmenKonten;

        /// <summary>
        /// Ausgaben-Konten aus [AusgabenRechnungsposten] (Cache-Praefix "a").
        /// Nur bis zum ersten leeren Eintrag befullt.
        /// </summary>
        public static IReadOnlyList<string> AusgabenKonten => _ausgabenKonten;

        /// <summary>
        /// Buchungsposten-Presets aus [Buchungsposten] (Keys "00Text", "00Ausg" usw.).
        /// Immer alle 100 Eintraege; Luecken ergeben leere Preset-Objekte (IstLeer==true).
        /// </summary>
        public static IReadOnlyList<Preset> Presets => _presets;

        /// <summary>
        /// Diagnose: alle aktuell gecachten Schluessel.
        /// </summary>
        public static IEnumerable<string> AlleSchluessel => _cache.Keys;

        // ─────────────────────────────────────────────────────────────────────

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

            // Presets: immer alle 100, Luecken erlaubt
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
        }
    }
}
