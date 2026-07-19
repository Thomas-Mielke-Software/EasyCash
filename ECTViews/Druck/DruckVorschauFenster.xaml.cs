// DruckVorschauFenster.xaml.cs - Seitenansicht für den WPF-Druck.
//
// Der DocumentViewer bekommt das FixedDocument nicht direkt, sondern
// über einen In-Memory-XPS-Roundtrip: Die Suchfunktion des
// DocumentViewer arbeitet nur auf Glyphs-Inhalten (wie sie in
// XPS-Dateien stehen) - ein im Code gebautes FixedDocument aus
// TextBlocks ist für die Suche unsichtbar. Beim Schreiben nach XPS
// werden die TextBlocks in Glyphs übersetzt, danach findet das
// Such-Feld den Text.
//
// Der Druck-Knopf des Viewers wird umgeleitet (DruckVorschauViewer),
// damit der Druckjob einen sprechenden Namen bekommt - "Microsoft
// Print to PDF" schlägt den Jobnamen als Dateinamen vor.
//
// Escape schließt das Fenster (wie die alte MFC-Druckvorschau).

using System;
using System.IO;
using System.IO.Packaging;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Xps.Packaging;

namespace ECTViews.Druck
{
    /// <summary>
    /// DocumentViewer, dessen Druck-Kommando (Toolbar-Knopf + Strg+P)
    /// statt des namenlosen Standard-Druckpfads eine eigene Aktion
    /// ausführt.
    /// </summary>
    public class DruckVorschauViewer : DocumentViewer
    {
        internal Action DruckAktion;

        protected override void OnPrintCommand()
        {
            if (DruckAktion != null) DruckAktion();
            else base.OnPrintCommand();
        }
    }

    public partial class DruckVorschauFenster : Window
    {
        private Package _paket;
        private Uri _paketUri;
        private XpsDocument _xps;
        private string _titel;

        public DruckVorschauFenster()
        {
            InitializeComponent();
            PreviewKeyDown += (s, e) =>
            {
                if (e.Key == Key.Escape)
                {
                    Close();
                    e.Handled = true;
                }
            };
            viewer.DruckAktion = Drucken;
            Closed += (s, e) => XpsFreigeben();
        }

        public void SetzeDokument(FixedDocument dokument, string titel)
        {
            _titel = titel;

            // In-Memory-XPS-Roundtrip (siehe Datei-Kommentar). Die Uri
            // muss pro Fenster eindeutig sein, sonst kollidieren zwei
            // gleichzeitig offene Seitenansichten im PackageStore.
            _paket = Package.Open(new MemoryStream(),
                FileMode.Create, FileAccess.ReadWrite);
            _paketUri = new Uri("memorystream://seitenansicht-"
                + Guid.NewGuid().ToString("N") + ".xps");
            PackageStore.AddPackage(_paketUri, _paket);
            _xps = new XpsDocument(_paket,
                CompressionOption.NotCompressed, _paketUri.AbsoluteUri);
            XpsDocument.CreateXpsDocumentWriter(_xps).Write(dokument);
            viewer.Document = _xps.GetFixedDocumentSequence();
        }

        private void Drucken()
        {
            var dokument = viewer.Document;
            if (dokument == null) return;
            DruckDokument.Drucke(dokument.DocumentPaginator, _titel);
        }

        private void XpsFreigeben()
        {
            viewer.Document = null;
            if (_xps != null) { _xps.Close(); _xps = null; }
            if (_paketUri != null)
            {
                PackageStore.RemovePackage(_paketUri);
                _paketUri = null;
            }
            if (_paket != null) { _paket.Close(); _paket = null; }
        }
    }
}
