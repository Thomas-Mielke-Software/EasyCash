// JournalRow.cs - Datenmodell für die einzelnen Zeilentypen im Journal.
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
    /// <summary>Basisklasse für alle Zeilentypen.</summary>
    public abstract class JournalRow
    {
    }

    /// <summary>
    /// Sektions-Ueberschrift wie "EINNAHMEN für 2024" oder
    /// "[Software-Entwicklung]". Wird in farbiger Schrift,
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
    /// Beschreibung, Netto, USt-Prozent, USt-Betrag, Brutto, AfA-Nr).
    /// </summary>
    public class JournalHeaderRow : JournalRow
    {
        public bool IsAusgabe { get; set; }
        public bool ZeigeBelegnummer { get; set; }
        public bool ZeigeSteuer { get; set; }
        public bool ZeigeAfaNr { get; set; }
        /// <summary>True im Bestandskonten-Modus -> Spaltenüberschrift "Saldo".</summary>
        public bool ZeigeSaldo { get; set; }
    }

    /// <summary>
    /// Buchungszeile - der häufigste Zeilentyp. Enthaelt die formatierten
    /// Spaltenwerte, eine Referenz auf die zugrundeliegende Buchung
    /// (für Selektion plus Bearbeiten), und den Zebra-Index.
    /// </summary>
    public class JournalBuchungRow : JournalRow
    {
        /// <summary>Die zugrundeliegende Buchung.</summary>
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
        /// <summary>
        /// Laufender Saldo - nur im Bestandskonten-Modus gefüllt,
        /// in den anderen Modi leer.
        /// </summary>
        public string SaldoText { get; set; }

        // Icons
        public BitmapSource BetriebIcon { get; set; }
        public BitmapSource BestandskontoIcon { get; set; }

        // ----------------------------------------------
        // Buchungsgruppen-Kennzeichnung (Phase D)
        //
        // Gruppen-Mitglieder werden in der Beschreibungsspalte mit einem
        // dezenten "[G]"-Praefix markiert (konsistent zur Vorlagen-Liste
        // auf der PresetsPage) plus Tooltip mit der Rolle.
        // ----------------------------------------------

        /// <summary>UUID der Buchungsgruppe oder null (kein Mitglied).</summary>
        public string GruppenUuid { get; set; }

        /// <summary>Tooltip-Text des Gruppen-Symbols ("Teil einer
        /// Buchungsgruppe, ...") -- null wenn kein Gruppen-Mitglied.</summary>
        public string GruppenTooltip { get; set; }

        /// <summary>True wenn die Buchung Mitglied einer Buchungsgruppe ist
        /// (steuert die Sichtbarkeit des "[G]"-Symbols).</summary>
        public bool IstGruppenMitglied => !string.IsNullOrEmpty(GruppenUuid);
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
        /// <summary>Endsaldo - nur im Bestandskonten-Modus gefüllt.</summary>
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

    // ---------------------------------------------------------------------
    // Anlagenverzeichnis-spezifische Zeilentypen
    //
    // Eigene Header/Row/Footer-Typen, weil der Spaltensatz dort komplett
    // anders aussieht als in den Buchungs-Modi (Datum/Konten/Bestandskonten).
    // Die WPF-DataTemplate-Auswahl per DataType verwendet diese Typen als
    // Diskriminator.
    // ---------------------------------------------------------------------

    /// <summary>Spaltenkopf für den Anlagenverzeichnis-Modus.</summary>
    public class JournalAnlagenHeaderRow : JournalRow
    {
    }

    /// <summary>
    /// Eine Zeile im Anlagenverzeichnis. Erbt von JournalBuchungRow,
    /// damit die existierende Selektion/Edit/Loesch/Kopier-Mechanik
    /// (RowStyleSelector erkennt JournalBuchungRow) ohne Anpassung
    /// weiterfunktioniert. Der Anlagenverzeichnis-Modus rendert die
    /// Zeile aber mit anderen Spalten -- siehe DataTemplate in
    /// JournalView.xaml.
    /// </summary>
    public class JournalAnlagenRow : JournalBuchungRow
    {
        public string AnschDatumText { get; set; }
        public string AnschKostenText { get; set; }
        public string BuchwBeginnText { get; set; }
        public string AfaJahresbetragText { get; set; }
        public string AbgangText { get; set; }
        public string BuchwEndeText { get; set; }
    }

    /// <summary>Summenzeile für den Anlagenverzeichnis-Modus.</summary>
    public class JournalAnlagenFooterRow : JournalRow
    {
        public string AnschKostenSummeText { get; set; }
        public string AfaSummeText { get; set; }
        public string AbgangSummeText { get; set; }
        public string BuchwEndeSummeText { get; set; }
        public string Waehrung { get; set; }
    }
}
