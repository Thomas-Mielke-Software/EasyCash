// FormularDruckBauer.cs - Druckt ein .ecf-Formular als FixedDocument.
//
// Anders als Journal/Bericht (Fliesstext ueber DruckDokument.Baue) ist ein
// Formular seitengetreu: pro Formularseite eine FixedPage in voller
// Seitengroesse mit dem PNG-Formularscan als Hintergrund und den absolut
// positionierten Feldwerten darueber. Die Geometrie kommt aus EXAKT
// derselben FormularLayout-Klasse wie die Bildschirm-Ansicht -> WYSIWYG.
// Kein Kopf/Fuss -- gedruckt wird das amtliche Formular selbst
// (wie der native Pfad DrawFormularToDC mit DrawInfo::pm).
//
// Gedruckt werden die von der Ansicht bereits berechneten Feldwerte
// (FormularFeldWert-Liste des ViewModels) -- keine Zweitberechnung.

using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using ECTEngine;
using ECTViews.Formulare;

namespace ECTViews.Druck
{
    public static class FormularDruckBauer
    {
        /// <summary>
        /// Baut das FixedDocument fuer ein Formular. Liefert null, wenn
        /// keine Definition vorliegt.
        /// </summary>
        public static FixedDocument Baue(FormularDefinition def,
            IReadOnlyList<FormularFeldWert> werte)
        {
            if (def == null || werte == null) return null;

            double breite = FormularLayout.SeitenBreite(def.Querformat);
            double hoehe = FormularLayout.SeitenHoehe(def.Querformat);
            double fontDip = FormularLayout.FontDip(def.Schriftgroesse, hoehe);
            var schrift = new FontFamily(
                string.IsNullOrWhiteSpace(def.Schriftart) ? "Courier New" : def.Schriftart);

            var dokument = new FixedDocument();
            dokument.DocumentPaginator.PageSize = new Size(breite, hoehe);

            for (int nr = 1; nr <= def.Seitenzahl; nr++)
            {
                var page = new FixedPage
                {
                    Width = breite,
                    Height = hoehe,
                    Background = Brushes.White
                };

                // Formularscan als Hintergrund (volle Seite; das
                // formular_nicht_mitdrucken-Verhalten des Alt-Programms --
                // Scan beim Druck auf Vordrucke weglassen -- steuert der
                // Einstellungs-Key wie nativ)
                if (!FormularscanNichtMitdrucken()
                    && def.Seitenbilder.TryGetValue(nr, out string bildDatei))
                {
                    var bild = FormularViewModel.LadeSeitenbild(bildDatei);
                    if (bild != null)
                    {
                        var image = new Image
                        {
                            Source = bild,
                            Width = breite,
                            Height = hoehe,
                            Stretch = Stretch.Fill
                        };
                        RenderOptions.SetBitmapScalingMode(image,
                            BitmapScalingMode.HighQuality);
                        FixedPage.SetLeft(image, 0);
                        FixedPage.SetTop(image, 0);
                        page.Children.Add(image);
                    }
                }

                // Feldwerte dieser Seite
                foreach (var wert in werte)
                {
                    var feld = wert.Feld;
                    if (!feld.HatInhalt || feld.Seite != nr) continue;
                    if (string.IsNullOrEmpty(wert.Text)) continue;

                    var text = new TextBlock
                    {
                        Text = wert.Text,
                        FontFamily = schrift,
                        FontSize = fontDip
                    };

                    double x = FormularLayout.X(feld.Horizontal, breite);
                    if (feld.RechtsBuendig)
                    {
                        // FixedPage kennt SetRight -- der Textblock haengt
                        // dann mit seiner rechten Kante am Ankerpunkt
                        // (Pendant TA_RIGHT)
                        FixedPage.SetRight(text, breite - x);
                    }
                    else
                    {
                        FixedPage.SetLeft(text, x);
                    }
                    FixedPage.SetTop(text,
                        FormularLayout.YTextOben(feld.Vertikal, hoehe));
                    page.Children.Add(text);
                }

                var pageContent = new PageContent();
                ((System.Windows.Markup.IAddChild)pageContent).AddChild(page);
                dokument.Pages.Add(pageContent);
            }

            return dokument;
        }

        /// <summary>[Druck]formular_nicht_mitdrucken -- Anwender druckt auf
        /// amtliche Vordrucke, der Scan bleibt dann weg (nur die Werte).</summary>
        private static bool FormularscanNichtMitdrucken()
            => Einstellungen.HoleBool("[Druck]formular_nicht_mitdrucken", false);
    }
}
