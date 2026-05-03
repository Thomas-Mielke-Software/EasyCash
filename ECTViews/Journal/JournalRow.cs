// JournalRow.cs - Datenmodell fuer die einzelnen Zeilentypen im Journal.
//
// Das Journal besteht aus einer flachen Liste von Zeilen unterschiedlichen
// Typs (Sektions-Titel, Header, Buchungszeile, Footer, Spacer). Per
// DataTemplate-Selector waehlt die ListBox pro Zeile das richtige
// visuelle Template. Die flache Struktur (statt geschachtelter Sections)
// ermoeglicht UI-Virtualisierung, was bei vielen Buchungen massiv
// Performance bringt.

using System.Windows.Media.Imaging;
using ECTEngine;

namespace ECTViews.Journal
{
    /// <summary>Basisklasse fuer alle Zeilentypen.</summary>
    public abstract class JournalRow
    {
    }

    /// <summary>
    /// Sektions-Ueberschrift wie "EINNAHMEN fuer 2024" oder
    /// "[Software-Entwicklung]". Wird in farbiger Schrift,
    /// groesser und fett gerendert.
    /// </summary>
    public class JournalSectionTitle : JournalRow
    {
        public string Text { get; set; }
        /// <summary>Hauptueberschrift (gross) vs. Unterueberschrift (mittel).</summary>
        public bool IsMain { get; set; }
        /// <summary>true fuer Einnahmen-Faerbung, false fuer Ausgaben-Faerbung, null fuer neutral.</summary>
        public bool? IsEinnahme { get; set; }
    }

    /// <summary>
    /// Header-Zeile mit den Spaltenueberschriften (Datum, Beleg,
    /// Beschreibung, Netto, USt-Prozent, USt-Betrag, Brutto, AfA-Nr).
    /// </summary>
    public class JournalHeaderRow : JournalRow
    {
        public bool IsAusgabe { get; set; }
        public bool ZeigeBelegnummer { get; set; }
        public bool ZeigeSteuer { get; set; }
        public bool ZeigeAfaNr { get; set; }
        /// <summary>True im Bestandskonten-Modus -> Spaltenueberschrift "Saldo".</summary>
        public bool ZeigeSaldo { get; set; }
    }

    /// <summary>
    /// Buchungszeile - der haeufigste Zeilentyp. Enthaelt die formatierten
    /// Spaltenwerte, eine Referenz auf die zugrundeliegende Buchung
    /// (fuer Selektion plus Bearbeiten), und den Zebra-Index.
    /// </summary>
    public class JournalBuchungRow : JournalRow
    {
        /// <summary>Die zugrundeliegende Buchung.</summary>
        public Buchung Buchung { get; set; }

        /// <summary>True = Ausgabe, False = Einnahme.</summary>
        public bool IstAusgabe { get; set; }

        /// <summary>Index fuer Zebra-Streifen-Hintergrund.</summary>
        public int ZebraIndex { get; set; }

        // Vorberechnete Anzeigewerte (formatiert):
        public string DatumText { get; set; }
        public string BelegText { get; set; }
        public string BeschreibungText { get; set; }
        public string NettoText { get; set; }
        public string MwstSatzText { get; set; }
        public string MwstBetragText { get; set; }
        public string BruttoText { get; set; }
        public string AfaNrText { get; set; }
        /// <summary>
        /// Laufender Saldo - nur im Bestandskonten-Modus gefuellt,
        /// in den anderen Modi leer.
        /// </summary>
        public string SaldoText { get; set; }

        // Icons
        public BitmapSource BetriebIcon { get; set; }
        public BitmapSource BestandskontoIcon { get; set; }
    }

    /// <summary>
    /// Footer-Zeile mit Summen (Netto, USt, Brutto). Wird mit Top-Border
    /// abgegrenzt und in Fett gerendert.
    /// </summary>
    public class JournalFooterRow : JournalRow
    {
        public bool IsAusgabe { get; set; }
        public bool ZeigeSteuer { get; set; }

        /// <summary>
        /// Beschriftung des linken Footer-Felds. Default "Summe", im
        /// Bestandskonten-Modus auf "Endsaldo" umgestellt.
        /// </summary>
        public string LinkesLabel { get; set; } = "Summe";

        public string NettoSummeText { get; set; }
        public string SteuerSummeText { get; set; }
        public string BruttoSummeText { get; set; }
        /// <summary>Endsaldo - nur im Bestandskonten-Modus gefuellt.</summary>
        public string SaldoSummeText { get; set; }
        public string Waehrung { get; set; }
    }

    /// <summary>
    /// Trennzeile (leer) zwischen Sektionen.
    /// </summary>
    public class JournalSpacerRow : JournalRow
    {
        public double Height { get; set; } = 8;
    }
}
