using System;
using System.Collections.Generic;
using System.Globalization;

namespace ECTEngine
{
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
            if (daten == null) return;
            foreach (var kv in daten)
                _cache[kv.Key] = kv.Value ?? "";
        }

        /// <summary>Cache leeren (z.B. bei Mandantenwechsel vor neuem Laden).</summary>
        public static void Leere() => _cache.Clear();

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

        /// <summary>
        /// Diagnose: alle aktuell gecachten Schluessel.
        /// </summary>
        public static IEnumerable<string> AlleSchluessel => _cache.Keys;
    }
}
