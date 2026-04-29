// JournalRow.cs — Datenmodell für die einzelnen Zeilentypen im Journal
//
// Das Journal besteht aus einer flachen Liste von Zeilen unterschiedlichen
// Typs (Sektions-Titel, Header, Buchungszeile, Footer). Per DataTemplate-
// Selector wählt das ItemsControl pro Zeile das richtige visuelle Template.
//
// Diese Aufteilung in flache, gemischte Liste statt geschachtelter
// Sections macht UI-Virtualisierung möglich, was bei vielen Buchungen
// massiv Performance bringt.

using System.Windows.Media.Imaging;
using ECTEngine;

namespace ECTViews.Journal
{
    /// <summary>Basisklasse für alle Zeilentypen.</summary>
    public abstract class JournalRow
    {
    }

    /// <summary>
    /// Sektions-Überschrift wie "EINNAHMEN für 2024" oder
    /// "--- Software-Entwicklung ---". Wird in farbiger Schrift,
    /// größer und fett gerendert.
    /// </summary>
    public class JournalSectionTitle : JournalRow
    {
        public string Text { get; set; }
        /// <summary>Hauptüberschrift (groß) vs. Unterüberschrift (mittel).</summary>
        public bool IsMain { get; set; }
        /// <summary>true für Einnahmen-Färbung, false für Ausgaben-Färbung, null für neutral.</summary>
        public bool? IsEinnahme { get; set; }
    }

    /// <summary>
    /// Header-Zeile mit den Spaltenüberschriften (Datum, Beleg,
    /// Beschreibung, Netto, USt%, USt-Betrag, Brutto, AfA-Nr).
    /// </summary>
    public class JournalHeaderRow : JournalRow
    {
        public bool IsAusgabe { get; set; }
        public bool ZeigeBelegnummer { get; set; }
        public bool ZeigeSteuer { get; set; }
        public bool ZeigeAfaNr { get; set; }
    }

    /// <summary>
    /// Buchungszeile — der häufigste Zeilentyp. Enthält die formatierten
    /// Spaltenwerte, eine Referenz auf die zugrundeliegende Buchung
    /// (für Selektion + Bearbeiten), und Hintergrund-Eigenschaften
    /// (gerade/ungerade für Zebra-Streifen, selektiert für gelb).
    /// </summary>
    public class JournalBuchungRow : JournalRow
    {
        /// <summary>Die zugrundeliegende Buchung (für Doppelklick/Kontextmenü).</summary>
        public Buchung Buchung { get; set; }

        /// <summary>True = Ausgabe, False = Einnahme.</summary>
        public bool IstAusgabe { get; set; }

        /// <summary>Index für Zebra-Streifen-Hintergrund.</summary>
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
        public string NettoSummeText { get; set; }
        public string SteuerSummeText { get; set; }   // USt oder VST
        public string BruttoSummeText { get; set; }
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
