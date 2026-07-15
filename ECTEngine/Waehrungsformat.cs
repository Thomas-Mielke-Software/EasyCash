// Waehrungsformat.cs -- zentrale Formatierung/Parsen von Geldbeträgen
// und Prozentwerten für die C#/WPF-Seite.
//
// Pendant zur DEZIMALKOMMA/TAUSENDERTRENNER-Mechanik des Alt-Programms
// (NativeInit.cpp holt LOCALE_SDECIMAL etc. per GetLocaleInfo;
// currency_to_int/int_to_currency in ectifacemisc.cpp benutzen sie):
//
//  - ANZEIGE und PERSISTENZ folgen den Windows-Regionaleinstellungen
//    (CultureInfo.CurrentCulture) -- auf Schweizer Systemen also
//    "1'234.56", auf deutschen "1.234,56". Damit verstehen die nativen
//    Leser (currency_to_int mit Locale-Zeichen) die von WPF
//    geschriebenen Werte auf demselben System.
//
//  - PARSEN ist bewusst locale-FREI und deterministisch tolerant, damit
//    Eingaben und Bestandsdaten aus deutschen ("1.234,56"),
//    schweizerischen ("1'234.56") und englischen ("1,234.56")
//    Schreibweisen gleichermassen verstanden werden -- wichtig beim
//    Umzug eines Mandanten zwischen Systemen mit unterschiedlichen
//    Regionaleinstellungen.
//
// NICHT betroffen: die Formelsprache der Buchungsgruppen-Vorlagen
// (FormelParser) -- Formeln sind portable Vorlagen-Daten mit fester,
// locale-unabhängiger Syntax.

using System.Globalization;

namespace ECTEngine
{
    public static class Waehrungsformat
    {
        /// <summary>Geldbetrag fürs Anzeigen, mit Tausendergruppierung
        /// nach Systemeinstellung (wie int_to_currency_tausenderpunkt).</summary>
        public static string Betrag(decimal wert)
            => wert.ToString("N2", CultureInfo.CurrentCulture);

        /// <summary>Geldbetrag ohne Gruppierung -- für Eingabefeld-
        /// Vorbelegungen und Persistenz (wie int_to_currency).</summary>
        public static string BetragOhneGruppierung(decimal wert)
            => wert.ToString("0.00", CultureInfo.CurrentCulture);

        /// <summary>Zahl (z.B. Prozentsatz) im Systemformat.</summary>
        public static string Zahl(decimal wert, string format = "0.##")
            => wert.ToString(format, CultureInfo.CurrentCulture);

        /// <summary>
        /// Toleranter Betrags-/Zahlen-Parser (siehe Kopf-Kommentar).
        /// Regeln:
        ///  - Apostroph (' und typografisch) und Leerraum gelten immer als
        ///    Gruppierung und werden entfernt;
        ///  - kommen Komma UND Punkt vor, ist das weiter RECHTS stehende
        ///    Zeichen der Dezimaltrenner;
        ///  - kommt nur eines mehrfach vor, ist es Gruppierung
        ///    ("1.234.567" bzw. "1,234,567");
        ///  - kommt nur eines einfach vor, ist es der Dezimaltrenner
        ///    ("1234,56" und "1234.56").
        /// </summary>
        public static bool TryParse(string s, out decimal wert)
        {
            wert = 0m;
            if (string.IsNullOrWhiteSpace(s)) return false;

            // Eindeutige Gruppierungszeichen entfernen: Apostroph (Schweiz,
            // auch typografisch U+2019), Leerzeichen sowie geschuetztes und
            // schmales geschuetztes Leerzeichen (Gruppierung u.a. in fr-CH)
            s = s.Trim()
                 .Replace("'", "").Replace("’", "")
                 .Replace(" ", "").Replace(" ", "").Replace(" ", "");

            int letztesKomma = s.LastIndexOf(',');
            int letzterPunkt = s.LastIndexOf('.');
            string invariant;
            if (letztesKomma >= 0 && letzterPunkt >= 0)
            {
                char dezimal = letztesKomma > letzterPunkt ? ',' : '.';
                char gruppe  = letztesKomma > letzterPunkt ? '.' : ',';
                invariant = s.Replace(gruppe.ToString(), "")
                             .Replace(dezimal, '.');
            }
            else if (letztesKomma >= 0)
            {
                invariant = s.IndexOf(',') != letztesKomma
                    ? s.Replace(",", "")     // mehrere Kommas: Gruppierung
                    : s.Replace(',', '.');   // ein Komma: Dezimaltrenner
            }
            else if (letzterPunkt >= 0 && s.IndexOf('.') != letzterPunkt)
            {
                invariant = s.Replace(".", "");   // mehrere Punkte: Gruppierung
            }
            else
            {
                invariant = s;   // höchstens ein Punkt: Dezimaltrenner
            }

            return decimal.TryParse(invariant,
                NumberStyles.AllowLeadingSign | NumberStyles.AllowDecimalPoint,
                CultureInfo.InvariantCulture, out wert);
        }

        /// <summary>Wie <see cref="TryParse"/>, zusätzlich wird ein
        /// angehängtes Prozentzeichen toleriert ("19 %").</summary>
        public static bool TryParseProzent(string s, out decimal wert)
        {
            wert = 0m;
            if (string.IsNullOrWhiteSpace(s)) return false;
            return TryParse(s.Trim().TrimEnd('%'), out wert);
        }
    }
}
