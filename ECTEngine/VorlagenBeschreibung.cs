// VorlagenBeschreibung.cs -- Beschreibungstext nach der Wahl einer
// Buchungsvorlage beim BEARBEITEN einer Buchung.
//
// Bei einer neuen Buchung ersetzt die Vorlage den Text einfach (der Dialog
// ruft diese Klasse dann gar nicht). Beim Bearbeiten steckt im Text aber
// oft mehr als der Vorlagenname: ein Zusatz ("Einnahme Eigenverbrauch
// 3Q/2026") oder ein ganz eigener Inhalt ("Hotel Berlin, 3 Naechte").
// Regel:
//   1. Leerer Text                     -> Vorlagenname.
//   2. Text beginnt mit dem Namen einer Vorlage (der laengste passende
//      gewinnt, nur an Wortgrenze)    -> nur dieser Anfang wird gegen den
//      neuen Vorlagennamen getauscht, der Rest bleibt:
//        "Einnahme Eigenverbrauch 3Q/2026" -> "Finanzamtsrueckzahlung 3Q/2026"
//        "Reverse Charge ...: Hotel Berlin" -> "<neue Vorlage>: Hotel Berlin"
//      Damit stapeln sich auch keine alten Gruppen-Praefixe.
//   3. Frei getippter Text ohne Vorlagen-Anfang:
//        Gruppen-Vorlage   -> Name vorangestellt ("<Vorlage>: Hotel Berlin"),
//                             damit die Gruppe im Journal erkennbar ist;
//        einfache Vorlage  -> Text bleibt unveraendert.

using System;
using System.Collections.Generic;

namespace ECTEngine
{
    public static class VorlagenBeschreibung
    {
        /// <summary>
        /// Liefert die Beschreibung nach der Wahl der Vorlage
        /// <paramref name="vorlagenName"/>.
        /// </summary>
        /// <param name="beschreibung">Bisheriger Beschreibungstext.</param>
        /// <param name="vorlagenName">Text der gewaehlten Vorlage.</param>
        /// <param name="istGruppenVorlage">True bei mehrzeiliger Vorlage.</param>
        /// <param name="alleVorlagenNamen">Texte aller Vorlagen (beider
        /// Buchungsarten) -- Kandidaten fuer den auszutauschenden Anfang.</param>
        public static string NachVorlagenwahl(string beschreibung, string vorlagenName,
            bool istGruppenVorlage, IEnumerable<string> alleVorlagenNamen)
        {
            var name = (vorlagenName ?? "").Trim();
            var text = (beschreibung ?? "").Trim();
            if (name.Length == 0) return text;
            if (text.Length == 0) return name;

            var anfang = LaengsterVorlagenAnfang(text, alleVorlagenNamen);
            if (anfang != null)
                return name + text.Substring(anfang.Length);

            return istGruppenVorlage ? name + ": " + text : text;
        }

        /// <summary>
        /// Laengster Vorlagenname, mit dem <paramref name="text"/> beginnt
        /// (Gross-/Kleinschreibung egal), oder null. Der Name muss an einer
        /// Wortgrenze enden -- "Miete" ist kein Anfang von "Mieterhoehung".
        /// Liefert den Anfang in der Laenge des Namens (Schreibweise des
        /// Textes ist egal, es wird nur die Laenge verwendet).
        /// </summary>
        public static string LaengsterVorlagenAnfang(string text,
            IEnumerable<string> alleVorlagenNamen)
        {
            if (string.IsNullOrEmpty(text) || alleVorlagenNamen == null) return null;
            string bester = null;
            foreach (var roh in alleVorlagenNamen)
            {
                var n = (roh ?? "").Trim();
                if (n.Length == 0 || n.Length > text.Length) continue;
                if (!text.StartsWith(n, StringComparison.OrdinalIgnoreCase)) continue;
                if (n.Length < text.Length && char.IsLetterOrDigit(text[n.Length]))
                    continue;   // keine Wortgrenze
                if (bester == null || n.Length > bester.Length)
                    bester = n;
            }
            return bester;
        }
    }
}
