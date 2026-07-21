// DruckDokument.cs - Gemeinsames Fundament für den WPF-Druck (WYSIWYG).
//
// Baut aus einer Liste von Inhaltsblöcken (fertig konstruierte UIElements)
// ein paginiertes FixedDocument im A4-Format: Kopfzeile mit Titel +
// Druckdatum + Untertitel (Filterbeschreibung) auf jeder Seite,
// Fußzeile mit Seitennummer. Wird vom Journal-Druck (JournalDruckBauer)
// und später von den Formlos-Berichten (BerichtDruckBauer) benutzt.
//
// Pagination-Regeln:
//   - Ein Block, der nicht mehr auf die Seite passt, rückt komplett auf
//     die nächste Seite (kein Zerschneiden von Zeilen).
//   - Blöcke mit MitNaechstemZusammenhalten (Abschnittstitel) bleiben
//     mit dem Folgeblock zusammen und stehen nie allein am Seitenende.
//   - Blöcke mit KopfKopie (Spaltenköpfe) werden nach einem Seitenumbruch
//     innerhalb ihres Abschnitts oben auf der neuen Seite wiederholt.
//     (Kopie nötig, weil ein UIElement nur EINEN visuellen Parent haben kann.)

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Shapes;

namespace ECTViews.Druck
{
    /// <summary>Ein druckbarer Inhaltsblock (typisch: eine Journal-Zeile).</summary>
    public class DruckBlock
    {
        public UIElement Element { get; set; }

        /// <summary>Abschnittstitel: nicht allein am Seitenende stehen lassen.</summary>
        public bool MitNaechstemZusammenhalten { get; set; }

        /// <summary>
        /// Nur bei Spaltenkopf-Zeilen gesetzt: erzeugt eine frische Kopie
        /// des Kopfes, die nach einem Seitenumbruch wiederholt wird.
        /// </summary>
        public Func<UIElement> KopfKopie { get; set; }
    }

    public static class DruckDokument
    {
        // A4 hochkant in DIP (96 dpi): 210 x 297 mm
        public const double SeitenBreite = 794.0;
        public const double SeitenHoehe  = 1123.0;
        public const double RandLinks  = 57.0;   // ca. 1,5 cm
        public const double RandRechts = 57.0;
        public const double RandOben   = 50.0;
        public const double RandUnten  = 50.0;

        public static double InhaltsBreite => SeitenBreite - RandLinks - RandRechts;

        /// <summary>
        /// Baut das paginierte FixedDocument. Titel/Untertitel erscheinen in
        /// der Kopfzeile jeder Seite, die Seitennummer in der Fußzeile.
        /// </summary>
        public static FixedDocument Baue(
            string titel, string untertitel,
            IList<DruckBlock> bloecke, string schriftart)
        {
            var schrift = new FontFamily(
                string.IsNullOrWhiteSpace(schriftart) ? "Segoe UI" : schriftart);

            // Seitenkopf einmal vermessen (er ist auf jeder Seite gleich hoch)
            double kopfHoehe = MessBlock(BaueSeitenkopf(titel, untertitel, schrift));
            double fussHoehe = 24.0;
            double inhaltsHoehe = SeitenHoehe - RandOben - RandUnten
                                  - kopfHoehe - fussHoehe;

            // ----------------------------------------------------------
            // Blöcke auf Seiten verteilen. Pro Seite eine Liste von
            // (Element, Y-Offset)-Paaren; Elemente sind schon vermessen.
            // ----------------------------------------------------------
            var seiten = new List<List<Platzierung>>();
            var aktuelleSeite = new List<Platzierung>();
            double y = 0;
            Func<UIElement> aktuellerKopf = null;   // Spaltenkopf des laufenden Abschnitts

            for (int i = 0; i < bloecke.Count; i++)
            {
                var block = bloecke[i];
                if (block?.Element == null) continue;

                double h = MessBlock(block.Element);

                // Zusammenhalt: Titel + Folgeblock gemeinsam betrachten
                double hZusammen = h;
                if (block.MitNaechstemZusammenhalten && i + 1 < bloecke.Count
                    && bloecke[i + 1]?.Element != null)
                {
                    hZusammen += MessBlock(bloecke[i + 1].Element);
                }

                if (y + hZusammen > inhaltsHoehe && y > 0)
                {
                    // Seitenumbruch
                    seiten.Add(aktuelleSeite);
                    aktuelleSeite = new List<Platzierung>();
                    y = 0;

                    // Spaltenkopf des laufenden Abschnitts wiederholen --
                    // aber nicht, wenn der aktuelle Block selbst ein
                    // (neuer) Spaltenkopf oder ein Abschnittstitel ist.
                    if (aktuellerKopf != null && block.KopfKopie == null
                        && !block.MitNaechstemZusammenhalten)
                    {
                        var kopie = aktuellerKopf();
                        double hk = MessBlock(kopie);
                        aktuelleSeite.Add(new Platzierung(kopie, y));
                        y += hk;
                    }
                }

                if (block.KopfKopie != null)
                    aktuellerKopf = block.KopfKopie;
                else if (block.MitNaechstemZusammenhalten)
                    aktuellerKopf = null;   // neuer Abschnitt beginnt -> alter Kopf gilt nicht mehr

                aktuelleSeite.Add(new Platzierung(block.Element, y));
                y += h;
            }
            if (aktuelleSeite.Count > 0 || seiten.Count == 0)
                seiten.Add(aktuelleSeite);

            // ----------------------------------------------------------
            // FixedPages bauen (jetzt ist die Gesamtseitenzahl bekannt)
            // ----------------------------------------------------------
            var dokument = new FixedDocument();
            dokument.DocumentPaginator.PageSize = new Size(SeitenBreite, SeitenHoehe);

            for (int s = 0; s < seiten.Count; s++)
            {
                var page = new FixedPage
                {
                    Width = SeitenBreite,
                    Height = SeitenHoehe,
                    Background = Brushes.White
                };

                // Kopfzeile
                var kopf = BaueSeitenkopf(titel, untertitel, schrift);
                FixedPage.SetLeft(kopf, RandLinks);
                FixedPage.SetTop(kopf, RandOben);
                page.Children.Add(kopf);

                // Inhalt
                foreach (var p in seiten[s])
                {
                    FixedPage.SetLeft(p.Element, RandLinks);
                    FixedPage.SetTop(p.Element, RandOben + kopfHoehe + p.Y);
                    page.Children.Add(p.Element);
                }

                // Fußzeile
                var fuss = BaueSeitenfuss(s + 1, seiten.Count, schrift);
                FixedPage.SetLeft(fuss, RandLinks);
                FixedPage.SetTop(fuss, SeitenHoehe - RandUnten - fussHoehe + 8);
                page.Children.Add(fuss);

                var pageContent = new PageContent();
                ((System.Windows.Markup.IAddChild)pageContent).AddChild(page);
                dokument.Pages.Add(pageContent);
            }

            return dokument;
        }

        /// <summary>
        /// Zeigt den System-Druckdialog und druckt das Dokument.
        /// Liefert false, wenn der Anwender abbricht.
        /// querformat=true erzwingt die Querformat-Ausrichtung im Druckticket
        /// (Pendant zum nativen OnPreparePrinting-Zweig für .ecf-Formulare
        /// mit querformat-Attribut, easycashview.cpp:6099).
        /// </summary>
        public static bool Drucke(FixedDocument dokument, string beschreibung,
            bool querformat = false)
            => Drucke(dokument.DocumentPaginator, beschreibung, querformat);

        /// <summary>
        /// Wie oben, aber für einen fertigen Paginator (Seitenansicht
        /// druckt die XPS-Fassung ihres Viewers). Der Jobname wird bei
        /// Datei-Druckern (Microsoft Print to PDF) zusätzlich als
        /// Dateiname in den Speichern-Prompt vorbelegt.
        /// </summary>
        public static bool Drucke(DocumentPaginator paginator, string beschreibung,
            bool querformat = false)
        {
            var dlg = new PrintDialog();
            if (dlg.ShowDialog() != true) return false;
            if (querformat && dlg.PrintTicket != null)
                dlg.PrintTicket.PageOrientation =
                    System.Printing.PageOrientation.Landscape;
            string jobName = DruckJobName(beschreibung);
            using (DateiPromptVorbelegung.Starte(jobName))
            {
                dlg.PrintDocument(paginator, jobName);
            }
            return true;
        }

        /// <summary>
        /// Macht aus dem Berichts-Titel einen Druckjob-Namen. Der Jobname
        /// wird von "Microsoft Print to PDF" als Dateiname vorgeschlagen,
        /// darum werden für Dateinamen ungültige Zeichen ersetzt.
        /// </summary>
        internal static string DruckJobName(string titel)
        {
            string name = string.IsNullOrWhiteSpace(titel)
                ? "EasyCash&Tax" : titel.Trim();
            foreach (char c in System.IO.Path.GetInvalidFileNameChars())
                name = name.Replace(c, '-');
            return name;
        }

        /// <summary>
        /// Zeigt die Seitenansicht (DocumentViewer-Fenster, mit eingebautem
        /// Druck-Knopf). ownerHwnd = natives Owner-Fenster oder IntPtr.Zero.
        /// </summary>
        public static void ZeigeVorschau(
            FixedDocument dokument, string titel, IntPtr ownerHwnd)
        {
            var fenster = new DruckVorschauFenster
            {
                Title = "Seitenansicht - " + titel
            };
            fenster.SetzeDokument(dokument, titel);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(fenster) { Owner = ownerHwnd };
            fenster.ShowDialog();
        }

        // ----------------------------------------------------------
        // interne Helfer
        // ----------------------------------------------------------

        private struct Platzierung
        {
            public readonly UIElement Element;
            public readonly double Y;
            public Platzierung(UIElement element, double y)
            {
                Element = element;
                Y = y;
            }
        }

        /// <summary>Vermisst ein Element bei Inhaltsbreite, liefert die Höhe.</summary>
        private static double MessBlock(UIElement element)
        {
            if (element is FrameworkElement fe)
                fe.Width = InhaltsBreite;
            element.Measure(new Size(InhaltsBreite, double.PositiveInfinity));
            return element.DesiredSize.Height;
        }

        private static UIElement BaueSeitenkopf(
            string titel, string untertitel, FontFamily schrift)
        {
            var panel = new StackPanel { Width = InhaltsBreite };

            var kopfzeile = new Grid();
            kopfzeile.ColumnDefinitions.Add(
                new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
            kopfzeile.ColumnDefinitions.Add(
                new ColumnDefinition { Width = GridLength.Auto });

            var titelText = new TextBlock
            {
                Text = titel ?? "",
                FontFamily = schrift,
                FontSize = 16,
                FontWeight = FontWeights.Bold
            };
            kopfzeile.Children.Add(titelText);

            var datumText = new TextBlock
            {
                Text = "gedruckt am " + DateTime.Now.ToString(
                    "d", CultureInfo.CurrentCulture),
                FontFamily = schrift,
                FontSize = 9,
                Foreground = Brushes.Gray,
                VerticalAlignment = VerticalAlignment.Bottom
            };
            Grid.SetColumn(datumText, 1);
            kopfzeile.Children.Add(datumText);
            panel.Children.Add(kopfzeile);

            if (!string.IsNullOrEmpty(untertitel))
            {
                panel.Children.Add(new TextBlock
                {
                    Text = untertitel,
                    FontFamily = schrift,
                    FontSize = 10,
                    Foreground = Brushes.Gray,
                    Margin = new Thickness(0, 2, 0, 0)
                });
            }

            panel.Children.Add(new Rectangle
            {
                Height = 1,
                Fill = Brushes.Black,
                Margin = new Thickness(0, 4, 0, 8)
            });

            return panel;
        }

        private static UIElement BaueSeitenfuss(
            int seite, int gesamt, FontFamily schrift)
        {
            var grid = new Grid { Width = InhaltsBreite };
            grid.ColumnDefinitions.Add(
                new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
            grid.ColumnDefinitions.Add(
                new ColumnDefinition { Width = GridLength.Auto });

            grid.Children.Add(new TextBlock
            {
                Text = "EasyCash&Tax",
                FontFamily = schrift,
                FontSize = 8,
                Foreground = Brushes.Gray
            });

            var seitenText = new TextBlock
            {
                Text = string.Format(CultureInfo.CurrentCulture,
                    "Seite {0} von {1}", seite, gesamt),
                FontFamily = schrift,
                FontSize = 9,
                Foreground = Brushes.Gray
            };
            Grid.SetColumn(seitenText, 1);
            grid.Children.Add(seitenText);

            return grid;
        }
    }
}
