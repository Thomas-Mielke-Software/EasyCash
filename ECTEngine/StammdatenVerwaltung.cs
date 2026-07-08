// StammdatenVerwaltung.cs -- Lesen/Schreiben der Stammdaten-Sektionen
// [Betriebe] und [Bestandskonten] über den globalen Einstellungs-Cache.
//
// Ersetzt die ini-Zugriffe des alten CIconAuswahl-Dialogs
// (IconAuswahl.cpp: OnNeu/OnLoeschen/OnUmbenennen/OnIconAendern/OnProperty).
//
// Die Einträge werden ROH gehalten (Suffix -> Wert), damit auch Schlüssel
// überleben, die diese Klasse nicht kennt (z.B. die vielen SaldoJJJJ-
// Einträge eines Bestandskontos oder zukünftige Erweiterungen). Beim
// Schreiben wird die ganze ini-Sektion neu aufgebaut -- anders als der
// alte MFC-Code (der beim Löschen den letzten Eintrag in die Lücke
// verschob und dabei Reihenfolge und teils Properties verlor) bleibt
// die Reihenfolge hier stabil.

using System;
using System.Collections.Generic;
using System.Globalization;

namespace ECTEngine
{
    /// <summary>
    /// Ein roher Stammdaten-Eintrag: alle ini-Key-Suffixe ("Name", "Icon",
    /// "Unternehmensart", "Saldo2024", ...) mit ihren Werten.
    /// </summary>
    public sealed class StammdatenEintrag
    {
        /// <summary>Suffix (ohne "BetriebNN"-Präfix) -> Rohwert.</summary>
        public Dictionary<string, string> Werte { get; }
            = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        public string Name
        {
            get => Werte.TryGetValue("Name", out var v) ? v : "";
            set => Werte["Name"] = value ?? "";
        }

        public int Icon
        {
            get => Werte.TryGetValue("Icon", out var v)
                && int.TryParse(v, NumberStyles.Integer,
                    CultureInfo.InvariantCulture, out var n) ? n : 0;
            set => Werte["Icon"] = value.ToString(CultureInfo.InvariantCulture);
        }
    }

    /// <summary>
    /// Liest und schreibt die sequenziellen Stammdaten-Einträge einer
    /// ini-Sektion (BetriebNNxxx / BestandskontoNNxxx). Schreibvorgänge
    /// aktualisieren den Cache, ersetzen die ini-Sektion komplett
    /// (WritePrivateProfileSection über die Bridge) und bauen die
    /// abgeleiteten Listen (Einstellungen.Betriebe/Bestandskonten) neu auf.
    /// </summary>
    public static class StammdatenVerwaltung
    {
        /// <summary>Wie im ini-Format: maximal 100 Einträge (NN = 00-99).</summary>
        public const int MaxEintraege = 100;

        /// <summary>
        /// Liest die Einträge der Sektion. Stoppt wie das restliche Programm
        /// (UpdateBetriebe in easycashview.cpp, Einstellungen.BaueListenAuf)
        /// beim ersten Eintrag ohne Namen.
        /// </summary>
        /// <param name="sektion">ini-Sektionsname, z.B. "Betriebe".</param>
        /// <param name="praefix">Eintrags-Präfix, z.B. "Betrieb".</param>
        public static List<StammdatenEintrag> Lese(string sektion, string praefix)
        {
            var liste = new List<StammdatenEintrag>();
            string sektionsPraefix = "[" + sektion + "]";

            // Alle Eintrags-Keys der Sektion einsammeln und nach Index gruppieren.
            var proIndex = new Dictionary<int, StammdatenEintrag>();
            foreach (var key in Einstellungen.HoleSchluesselMitPraefix(sektionsPraefix + praefix))
            {
                if (!ZerlegeEintragsKey(key, sektionsPraefix, praefix,
                        out int index, out string suffix))
                    continue;
                if (!proIndex.TryGetValue(index, out var eintrag))
                    proIndex[index] = eintrag = new StammdatenEintrag();
                eintrag.Werte[suffix] = Einstellungen.Hole(key);
            }

            for (int i = 0; i < MaxEintraege; i++)
            {
                if (!proIndex.TryGetValue(i, out var eintrag)
                    || string.IsNullOrEmpty(eintrag.Name))
                    break;
                liste.Add(eintrag);
            }
            return liste;
        }

        /// <summary>
        /// Schreibt die Einträge zurück: Cache-Keys der alten Einträge werden
        /// geleert, die neuen gesetzt, dann die ini-Sektion in EINEM Rutsch
        /// ersetzt. Keys der Sektion, die keinem Eintrag gehören, bleiben
        /// unangetastet erhalten.
        /// </summary>
        public static void Schreibe(string sektion, string praefix,
            IReadOnlyList<StammdatenEintrag> eintraege)
        {
            if (eintraege == null) return;
            string sektionsPraefix = "[" + sektion + "]";

            // ini-Sektion komplett neu zusammenstellen: erst die Fremd-Keys
            // (Pass-through), dann die Eintrags-Keys in neuer Reihenfolge.
            var sektionsInhalt = new Dictionary<string, string>(
                StringComparer.OrdinalIgnoreCase);

            foreach (var key in Einstellungen.HoleSchluesselMitPraefix(sektionsPraefix))
            {
                if (ZerlegeEintragsKey(key, sektionsPraefix, praefix, out _, out _))
                    Einstellungen.SetzeCacheNur(key, "");   // alter Eintrags-Key: leeren
                else
                    sektionsInhalt[key.Substring(sektionsPraefix.Length)] =
                        Einstellungen.Hole(key);            // Fremd-Key: erhalten
            }

            int anzahl = Math.Min(eintraege.Count, MaxEintraege);
            for (int i = 0; i < anzahl; i++)
            {
                string eintragsPraefix = praefix + i.ToString("D2", CultureInfo.InvariantCulture);
                foreach (var kv in eintraege[i].Werte)
                {
                    if (string.IsNullOrEmpty(kv.Value)) continue;
                    Einstellungen.SetzeCacheNur(
                        sektionsPraefix + eintragsPraefix + kv.Key, kv.Value);
                    sektionsInhalt[eintragsPraefix + kv.Key] = kv.Value;
                }
            }

            Einstellungen.SchreibeSektion(sektion, sektionsInhalt);
            Einstellungen.NeuAufbauen();   // Betriebe/Bestandskonten-Listen aktualisieren
        }

        /// <summary>
        /// Zerlegt einen Cache-Key der Form "[Sektion]PraefixNNSuffix" in
        /// Index und Suffix. False, wenn der Key kein Eintrags-Key ist.
        /// </summary>
        private static bool ZerlegeEintragsKey(string key, string sektionsPraefix,
            string praefix, out int index, out string suffix)
        {
            index = -1; suffix = null;
            int fest = sektionsPraefix.Length + praefix.Length;
            if (key.Length < fest + 2) return false;
            if (!key.StartsWith(sektionsPraefix + praefix,
                    StringComparison.OrdinalIgnoreCase)) return false;

            char z1 = key[fest], z2 = key[fest + 1];
            if (z1 < '0' || z1 > '9' || z2 < '0' || z2 > '9') return false;

            index = (z1 - '0') * 10 + (z2 - '0');
            suffix = key.Substring(fest + 2);
            return suffix.Length > 0;
        }
    }
}
