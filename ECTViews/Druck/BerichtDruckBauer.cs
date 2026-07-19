// BerichtDruckBauer.cs - WYSIWYG-Druck der Formlos-Berichte
// (Freestyle-EÜR, formlose USt-Erklärung, Kontenplan).
//
// Erzeugt aus einem ECTEngine-Bericht ein FixedDocument über die
// DruckDokument-Infrastruktur. Layout spiegelt die BerichtView:
// eingerückter Text (Star) + bis zu drei rechtsbündige Wertspalten,
// Stil-Attribute (Überschrift/Zwischentitel/Summe/Hinweis) und
// Trennlinien identisch zur Bildschirmdarstellung.

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using ECTEngine;

namespace ECTViews.Druck
{
    public static class BerichtDruckBauer
    {
        // Feste Druck-Schriftgröße, unabhängig vom Bildschirm-Zoom
        // (Berichte haben wenige Spalten, deshalb etwas größer als
        // der Journal-Druck).
        private const double DruckSchrift = 11.0;
        private const double WertSpalte = 110.0;

        private static readonly FontFamily SchriftBetraege = new FontFamily("Consolas");
        private static readonly Brush FarbeGrau =
            new SolidColorBrush(Color.FromRgb(0x60, 0x60, 0x60));

        public static FixedDocument Baue(Bericht bericht, string schriftart)
        {
            if (bericht == null) return null;
            var schrift = new FontFamily(
                string.IsNullOrWhiteSpace(schriftart) ? "Segoe UI" : schriftart);

            var bloecke = new List<DruckBlock>();

            // Adresskopf (Titel + Untertitel übernimmt die Seitenkopfzeile
            // von DruckDokument selbst)
            if (bericht.KopfLinks.Count > 0 || bericht.KopfRechts.Count > 0)
                bloecke.Add(new DruckBlock
                {
                    Element = BaueAdresskopf(bericht, schrift)
                });

            foreach (var zeile in bericht.Zeilen)
            {
                bloecke.Add(new DruckBlock
                {
                    Element = BaueZeile(zeile, schrift),
                    MitNaechstemZusammenhalten =
                        zeile.Stil == BerichtStil.Ueberschrift
                        || zeile.Stil == BerichtStil.Zwischentitel
                });
            }

            return DruckDokument.Baue(bericht.Titel, bericht.Untertitel,
                bloecke, schriftart);
        }

        private static UIElement BaueAdresskopf(Bericht bericht, FontFamily schrift)
        {
            var grid = new Grid { Margin = new Thickness(0, 0, 0, 14) };
            grid.ColumnDefinitions.Add(new ColumnDefinition
            {
                Width = new GridLength(1, GridUnitType.Star)
            });
            grid.ColumnDefinitions.Add(new ColumnDefinition
            {
                Width = GridLength.Auto
            });

            grid.Children.Add(new TextBlock
            {
                Text = string.Join(Environment.NewLine, bericht.KopfLinks),
                FontFamily = schrift,
                FontSize = DruckSchrift
            });

            var rechts = new TextBlock
            {
                Text = string.Join(Environment.NewLine, bericht.KopfRechts),
                FontFamily = schrift,
                FontSize = DruckSchrift,
                TextAlignment = TextAlignment.Right,
                VerticalAlignment = VerticalAlignment.Top
            };
            Grid.SetColumn(rechts, 1);
            grid.Children.Add(rechts);

            return grid;
        }

        private static UIElement BaueZeile(BerichtZeile zeile, FontFamily schrift)
        {
            double groesse = DruckSchrift;
            var gewicht = FontWeights.Normal;
            var stilArt = FontStyles.Normal;
            Brush farbe = Brushes.Black;

            switch (zeile.Stil)
            {
                case BerichtStil.Ueberschrift:
                    groesse = DruckSchrift * 1.2;
                    gewicht = FontWeights.Bold;
                    break;
                case BerichtStil.Zwischentitel:
                case BerichtStil.Summe:
                    gewicht = FontWeights.Bold;
                    break;
                case BerichtStil.Hinweis:
                    groesse = DruckSchrift * 0.9;
                    stilArt = FontStyles.Italic;
                    farbe = FarbeGrau;
                    break;
            }

            var grid = new Grid();
            grid.ColumnDefinitions.Add(new ColumnDefinition
            {
                Width = new GridLength(1, GridUnitType.Star)
            });
            for (int i = 0; i < 3; i++)
                grid.ColumnDefinitions.Add(new ColumnDefinition
                {
                    Width = new GridLength(WertSpalte)
                });

            double oben = (zeile.Stil == BerichtStil.Ueberschrift) ? 8 : 1;
            grid.Margin = new Thickness(zeile.Einrueckung * 18.0, oben, 0, 1);

            var text = new TextBlock
            {
                Text = zeile.Text ?? "",
                FontFamily = schrift,
                FontSize = groesse,
                FontWeight = gewicht,
                FontStyle = stilArt,
                Foreground = farbe,
                TextWrapping = TextWrapping.Wrap
            };
            grid.Children.Add(text);

            // Wertspalten rechtsbündig auffüllen (letzter Wert ganz rechts)
            var werte = zeile.Werte ?? Array.Empty<string>();
            for (int i = 0; i < werte.Length && i < 3; i++)
            {
                var tb = new TextBlock
                {
                    Text = werte[werte.Length - 1 - i] ?? "",
                    FontFamily = SchriftBetraege,
                    FontSize = groesse,
                    FontWeight = gewicht,
                    Foreground = farbe,
                    TextAlignment = TextAlignment.Right
                };
                Grid.SetColumn(tb, 3 - i);
                grid.Children.Add(tb);
            }

            if (!zeile.LinieOben && !zeile.LinieUnten)
                return grid;

            return new Border
            {
                BorderBrush = Brushes.Black,
                BorderThickness = new Thickness(0,
                    zeile.LinieOben ? 0.75 : 0, 0, zeile.LinieUnten ? 0.75 : 0),
                Padding = new Thickness(0, 1, 0, 1),
                Child = grid
            };
        }
    }
}
