// JournalFilter.cs — Filter-Optionen für das Buchungsjournal
//
// Replikat der Display-Filter aus easycashview.cpp:
//   m_KontenFilterDisplay, m_MonatsFilterDisplay,
//   m_BetriebFilterDisplay, m_BestandskontoFilterDisplay
//
// Anzeige-Modus (m_nAnzeige im Original):
//   Datum   → Buchungen sortiert nach Datum, gruppiert in Einnahmen + Ausgaben
//   Konten  → Buchungen gruppiert nach EÜR-Konto

namespace ECTViews.Journal
{
    public enum JournalAnzeigeModus
    {
        Datum,
        Konten
    }

    public class JournalFilter
    {
        /// <summary>Anzeige-Modus (Datum oder Konten).</summary>
        public JournalAnzeigeModus AnzeigeModus { get; set; } = JournalAnzeigeModus.Datum;

        /// <summary>
        /// Konten-Filter. "" oder "&lt;alle Konten&gt;" = kein Filter.
        /// Spezialwerte wie "Einnahmen: Honorar" oder
        /// "--- [noch zu keinem Konto zugewiesene Einnahmen] ---" werden
        /// 1:1 wie im Original behandelt.
        /// </summary>
        public string KontenFilter { get; set; } = "";

        /// <summary>
        /// Monats-Filter:
        ///   0   = alle Monate
        ///   1-12 = einzelner Monat
        ///   13-16 = Quartal 1-4
        /// </summary>
        public int MonatsFilter { get; set; }

        /// <summary>Betriebs-Filter. "" = kein Filter.</summary>
        public string BetriebFilter { get; set; } = "";

        /// <summary>Bestandskonto-Filter. "" = kein Filter.</summary>
        public string BestandskontoFilter { get; set; } = "";

        /// <summary>Schriftgröße (Zoom).</summary>
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
