// JournalFilter.cs - Filter-Optionen fuer das Buchungsjournal
//
// Replikat der Display-Filter aus easycashview.cpp:
//   m_KontenFilterDisplay, m_MonatsFilterDisplay,
//   m_BetriebFilterDisplay, m_BestandskontoFilterDisplay
//
// Anzeige-Modus (m_nAnzeige im Original):
//   Datum   - Buchungen sortiert nach Datum, gruppiert in Einnahmen plus Ausgaben
//   Konten  - Buchungen gruppiert nach EUER-Konto

namespace ECTViews.Journal
{
    public enum JournalAnzeigeModus
    {
        Datum = 0,                  // Einnahmen plus Ausgaben, sortiert nach Datum
        Konten = 1,                 // pro Konto eine Tabelle, gruppiert E/A
        Bestandskonten = 2,         // pro Bestandskonto eine Tabelle mit Saldo
        Anlagenverzeichnis = 3      // pro Konto die Anlagegueter
    }

    public class JournalFilter
    {
        /// <summary>Anzeige-Modus (Datum oder Konten).</summary>
        public JournalAnzeigeModus AnzeigeModus { get; set; } = JournalAnzeigeModus.Datum;

        /// <summary>
        /// Konten-Filter. Leer oder "[alle Konten]" = kein Filter.
        /// Spezialwerte wie "Einnahmen: Honorar" oder
        /// die "noch zu keinem Konto zugewiesene"-Labels werden
        /// 1:1 wie im Original behandelt.
        /// </summary>
        public string KontenFilter { get; set; } = "";

        /// <summary>
        /// Monats-Filter:
        ///   0    = alle Monate
        ///   1-12 = einzelner Monat
        ///   13-16 = Quartal 1-4
        /// </summary>
        public int MonatsFilter { get; set; }

        /// <summary>Betriebs-Filter. Leer = kein Filter.</summary>
        public string BetriebFilter { get; set; } = "";

        /// <summary>Bestandskonto-Filter. Leer = kein Filter.</summary>
        public string BestandskontoFilter { get; set; } = "";

        /// <summary>Schriftgroesse (Zoom).</summary>
        public double Schriftgroesse { get; set; } = 13.0;

        /// <summary>Optionale Spalten anzeigen.</summary>
        public bool ZeigeBelegnummernspalte { get; set; } = true;
        public bool ZeigeSteuerspalte { get; set; } = true;

        public bool IstKontenFilterAktiv =>
            !string.IsNullOrEmpty(KontenFilter) && KontenFilter != "<alle Konten>";

        public JournalFilter Klon()
        {
            return new JournalFilter
            {
                AnzeigeModus = AnzeigeModus,
                KontenFilter = KontenFilter,
                MonatsFilter = MonatsFilter,
                BetriebFilter = BetriebFilter,
                BestandskontoFilter = BestandskontoFilter,
                Schriftgroesse = Schriftgroesse,
                ZeigeBelegnummernspalte = ZeigeBelegnummernspalte,
                ZeigeSteuerspalte = ZeigeSteuerspalte
            };
        }
    }
}
