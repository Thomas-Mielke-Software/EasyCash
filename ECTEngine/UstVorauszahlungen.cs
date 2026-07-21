// UstVorauszahlungen.cs -- Lese-/Schreib-Helfer fuer die manuell erfassten
// USt-Zahlbetraege und die Sondervorauszahlung (Dauerfristverlaengerung).
//
// Port von CEasyCashView::GetUmsatzsteuervorauszahlung /
// SetUmsatzsteuervorauszahlung (easycashview.cpp:6761/6779) und dem
// Sondervorauszahlungs-Zugriff (OnEditUmsatzsteuervorauszahlungen).
// Speicherort ist der Dokument-ErweiterungStore:
//   - Zahlbetraege: Sektion "Elster", Key "UST-Zahlbetrag-JJJJ-NN"
//     (NN: 01-12 = Monat, 41-44 = Quartal 1-4)
//   - Sondervorauszahlung: Sektion "Dauerfristverlängerung",
//     Key "SondervorauszahlungJJJJ"
//
// Zahlbetraege werden beim Schreiben wie nativ normalisiert
// (currency_to_int + int_to_currency): Betrag 0 oder unlesbar -> leer,
// sonst Betragsformat ohne Gruppierung. Die Sondervorauszahlung wird wie
// nativ ROH gespeichert (die Einstellungs-Seite "Aktuelles Dokument"
// validiert sie zusaetzlich).

using System.Globalization;

namespace ECTEngine
{
    public static class UstVorauszahlungen
    {
        private const string ElsterNs = "Elster";
        private const string SonderNs = "Dauerfristverlängerung";

        /// <summary>Zeitraum-Codes der Quartale (wie nativ: 41-44).</summary>
        public static int QuartalZeitraum(int quartal) => 40 + quartal;

        private static string Schluessel(int jahr, int zeitraum)
            => string.Format(CultureInfo.InvariantCulture,
                "UST-Zahlbetrag-{0:D4}-{1:D2}", jahr, zeitraum);

        // -----------------------------------------------------------------
        // Zahlbetraege (1-12 Monat, 41-44 Quartal)
        // -----------------------------------------------------------------

        /// <summary>Gespeicherter Zahlbetrag als Text ("" = nicht gemerkt).</summary>
        public static string Hole(BuchungsDocument doc, int zeitraum)
            => doc.Erweiterungen.Hole(ElsterNs,
                Schluessel(doc.Jahr, zeitraum), "");

        /// <summary>Zahlbetrag in Cent, oder null wenn nicht gemerkt/lesbar
        /// (tolerant geparst -- wie der USt-Erklaerungs-Bericht).</summary>
        public static long? HoleCent(BuchungsDocument doc, int zeitraum)
        {
            string wert = Hole(doc, zeitraum);
            if (string.IsNullOrEmpty(wert)) return null;
            if (!Waehrungsformat.TryParse(wert, out decimal betrag)) return null;
            return (long)decimal.Round(betrag * 100m, 0,
                System.MidpointRounding.AwayFromZero);
        }

        /// <summary>
        /// Schreibt einen Zahlbetrag (Normalisierung wie nativ: 0/unlesbar
        /// -> leer, sonst Betragsformat ohne Gruppierung).
        /// Liefert true, wenn sich der gespeicherte Wert geaendert hat.
        /// </summary>
        public static bool Setze(BuchungsDocument doc, int zeitraum, string wert)
        {
            long cent = FormularRechner.CurrencyToCent(wert);
            string normalisiert = cent != 0
                ? Waehrungsformat.BetragOhneGruppierung(cent / 100m)
                : "";

            string vorher = Hole(doc, zeitraum);
            doc.Erweiterungen.Setze(ElsterNs,
                Schluessel(doc.Jahr, zeitraum), normalisiert);
            return normalisiert != vorher;
        }

        // -----------------------------------------------------------------
        // Sondervorauszahlung (Dauerfristverlaengerung), pro Buchungsjahr
        // -----------------------------------------------------------------

        public static string HoleSondervorauszahlung(BuchungsDocument doc)
            => doc.Erweiterungen.Hole(SonderNs,
                "Sondervorauszahlung" + doc.Jahr.ToString("D4", CultureInfo.InvariantCulture), "");

        /// <summary>Schreibt die Sondervorauszahlung (roher Text, wie
        /// nativ). Liefert true bei Aenderung.</summary>
        public static bool SetzeSondervorauszahlung(BuchungsDocument doc, string wert)
        {
            wert = wert ?? "";
            string vorher = HoleSondervorauszahlung(doc);
            if (wert == vorher) return false;
            doc.Erweiterungen.Setze(SonderNs,
                "Sondervorauszahlung" + doc.Jahr.ToString("D4", CultureInfo.InvariantCulture), wert);
            return true;
        }
    }
}
