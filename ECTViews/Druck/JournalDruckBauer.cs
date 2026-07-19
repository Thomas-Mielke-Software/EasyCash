// JournalDruckBauer.cs - WYSIWYG-Druck des Buchungsjournals.
//
// Erzeugt aus dem AKTUELLEN Zustand eines JournalViewModel (dieselben
// JournalRow-Listen, die die ListBox anzeigt - inklusive der aktiven
// Ribbon-Filter) ein FixedDocument über die DruckDokument-Infrastruktur.
// Es gibt bewusst KEINEN zweiten Filter-Codepfad: Was im ViewModel
// steht, wird gedruckt.
//
// Layout: Spiegel der DataTemplates in JournalView.xaml (gleiche
// Spaltenreihenfolge, gleiche Schrift, Consolas für Beträge, Farben der
// Abschnittstitel). Abweichungen vom Bildschirm:
//   - Feste Druck-Schriftgröße (unabhängig vom Bildschirm-Zoom; ein
//     300%-Zoom soll den Ausdruck nicht aufblähen)
//   - Keine Zebra-Streifen, keine Selektion
//   - Icon-Spalten nur, wenn tatsächlich Icons vorkommen

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using ECTViews.Journal;

namespace ECTViews.Druck
{
    public static class JournalDruckBauer
    {
        // Druck-Schriftgröße: unabhängig vom Bildschirm-Zoom. Alle
        // Spaltenbreiten sind auf die Basis 13 ausgelegt (wie im
        // JournalViewModel) und werden mit s = DruckSchrift/13 skaliert.
        private const double DruckSchrift = 10.0;
        private const double Basis = 13.0;
        private const double S = DruckSchrift / Basis;

        private static readonly Brush FarbeEinnahme =
            new SolidColorBrush(Color.FromRgb(0x10, 0x30, 0xD0));
        private static readonly Brush FarbeAusgabe =
            new SolidColorBrush(Color.FromRgb(0xC6, 0x28, 0x28));
        private static readonly Brush FarbeGrau =
            new SolidColorBrush(Color.FromRgb(0x90, 0x90, 0x90));
        private static readonly FontFamily SchriftBetraege = new FontFamily("Consolas");

        /// <summary>
        /// Baut das Druckdokument aus dem aktuellen ViewModel-Zustand.
        /// <paramref name="titel"/> liefert den zusammengesetzten Titel
        /// (auch als Druckjob-Name verwendbar).
        /// </summary>
        public static FixedDocument Baue(JournalViewModel vm, out string titel)
        {
            titel = "";
            if (vm == null) return null;

            var zeilen = vm.Zeilen.ToList();
            var filter = vm.AktuellerFilter;
            titel = BaueTitel(filter, vm.Doc?.Jahr ?? 0);
            string untertitel = BaueUntertitel(filter);

            var layout = ErmittleLayout(vm, zeilen);
            var schrift = new FontFamily(vm.Schriftart);

            var bloecke = new List<DruckBlock>();
            foreach (var zeile in zeilen)
            {
                DruckBlock block = null;
                switch (zeile)
                {
                    case JournalSectionTitle t:
                        block = new DruckBlock
                        {
                            Element = BaueSektionsTitel(t, schrift),
                            MitNaechstemZusammenhalten = true
                        };
                        break;

                    case JournalAnlagenHeaderRow _:
                        block = new DruckBlock
                        {
                            Element = BaueAnlagenHeader(layout, schrift),
                            KopfKopie = () => BaueAnlagenHeader(layout, schrift)
                        };
                        break;

                    case JournalHeaderRow h:
                        block = new DruckBlock
                        {
                            Element = BaueHeader(h, layout, schrift),
                            KopfKopie = () => BaueHeader(h, layout, schrift)
                        };
                        break;

                    case JournalAnlagenRow ar:
                        block = new DruckBlock
                        {
                            Element = BaueAnlagenZeile(ar, layout, schrift)
                        };
                        break;

                    case JournalBuchungRow b:
                        block = new DruckBlock
                        {
                            Element = BaueBuchungsZeile(b, layout, schrift)
                        };
                        break;

                    case JournalAnlagenFooterRow af:
                        block = new DruckBlock
                        {
                            Element = BaueAnlagenFooter(af, layout, schrift)
                        };
                        break;

                    case JournalFooterRow f:
                        block = new DruckBlock
                        {
                            Element = BaueFooter(f, layout, schrift)
                        };
                        break;

                    case JournalSpacerRow sp:
                        block = new DruckBlock
                        {
                            Element = new System.Windows.Shapes.Rectangle
                            {
                                Height = Math.Max(2.0, sp.Height * S),
                                Fill = Brushes.Transparent
                            }
                        };
                        break;
                }
                if (block != null) bloecke.Add(block);
            }

            return DruckDokument.Baue(titel, untertitel, bloecke, vm.Schriftart);
        }

        // ----------------------------------------------------------
        // Titel + Untertitel (Filterbeschreibung)
        // ----------------------------------------------------------

        private static string BaueTitel(JournalFilter filter, int jahr)
        {
            string basis;
            switch (filter?.AnzeigeModus ?? JournalAnzeigeModus.Datum)
            {
                case JournalAnzeigeModus.Konten:
                    basis = "Buchungsjournal nach Konten"; break;
                case JournalAnzeigeModus.Bestandskonten:
                    basis = "Bestandskonten-Journal"; break;
                case JournalAnzeigeModus.Anlagenverzeichnis:
                    basis = "Anlagenverzeichnis"; break;
                default:
                    basis = "Buchungsjournal"; break;
            }
            return (jahr > 0) ? basis + " " + jahr : basis;
        }

        private static string BaueUntertitel(JournalFilter filter)
        {
            if (filter == null) return "";
            var teile = new List<string>();

            if (filter.MonatsFilter >= 1 && filter.MonatsFilter <= 12)
                teile.Add("Monat: " + CultureInfo.CurrentCulture.DateTimeFormat
                    .GetMonthName(filter.MonatsFilter));
            else if (filter.MonatsFilter >= 13 && filter.MonatsFilter <= 16)
                teile.Add("Quartal " + (filter.MonatsFilter - 12));

            if (filter.IstKontenFilterAktiv)
                teile.Add("Konto: " + filter.KontenFilter);
            if (!string.IsNullOrEmpty(filter.BetriebFilter))
                teile.Add("Betrieb: " + filter.BetriebFilter);
            if (!string.IsNullOrEmpty(filter.BestandskontoFilter))
                teile.Add("Bestandskonto: " + filter.BestandskontoFilter);

            return string.Join("  |  ", teile);
        }

        // ----------------------------------------------------------
        // Spalten-Layout
        // ----------------------------------------------------------

        private class Layout
        {
            // Standard-Modi (Datum/Konten/Bestandskonten):
            // [IconBetrieb][IconBK][Datum][Beleg][Beschreibung*]
            // [Netto][USt%][USt-Betr][Brutto][Saldo][AfA-Nr]
            public double IconBetrieb, IconBK, Datum, Beleg;
            public double Netto, Satz, Betrag, Brutto, Saldo, Afa;
            public double IconGroesse;
            public bool ZeigeSteuer, ZeigeSaldo, ZeigeAfa, ZeigeBeleg;
        }

        private static Layout ErmittleLayout(
            JournalViewModel vm, List<JournalRow> zeilen)
        {
            var l = new Layout();

            var header = zeilen.OfType<JournalHeaderRow>().FirstOrDefault();
            l.ZeigeSteuer = header?.ZeigeSteuer ?? true;
            l.ZeigeSaldo  = header?.ZeigeSaldo  ?? false;
            l.ZeigeAfa    = header?.ZeigeAfaNr  ?? false;
            l.ZeigeBeleg  = header?.ZeigeBelegnummer ?? true;

            bool betriebIcons = zeilen.OfType<JournalBuchungRow>()
                .Any(b => b.BetriebIcon != null);
            bool bkIcons = zeilen.OfType<JournalBuchungRow>()
                .Any(b => b.BestandskontoIcon != null);

            // Beleg-Breite: die im ViewModel berechnete (95-Perzentil-)Breite
            // auf die Druck-Skala normieren (sie liegt in Bildschirm-Skala vor).
            double bildschirmSkala = (vm.Schriftgroesse > 0)
                ? vm.Schriftgroesse / Basis : 1.0;
            double belegBasis = vm.BelegSpaltenBreite / bildschirmSkala;

            l.IconBetrieb = betriebIcons ? 24.0 * S : 0;
            l.IconBK      = bkIcons ? 24.0 * S : 0;
            l.Datum       = 90.0 * S;
            l.Beleg       = l.ZeigeBeleg ? belegBasis * S : 0;
            l.Netto       = l.ZeigeSteuer ? 100.0 * S : 0;
            l.Satz        = l.ZeigeSteuer ? 60.0 * S : 0;
            l.Betrag      = l.ZeigeSteuer ? 100.0 * S : 0;
            l.Brutto      = 110.0 * S;
            l.Saldo       = l.ZeigeSaldo ? 110.0 * S : 0;
            l.Afa         = l.ZeigeAfa ? 60.0 * S : 0;
            l.IconGroesse = DruckSchrift * 4.0 / 3.0;

            return l;
        }

        // Spaltenbreiten der Standard-Modi in Grid-Reihenfolge; Index 4 ist
        // die Star-Spalte (Beschreibung).
        private static double[] StandardSpalten(Layout l) => new[]
        {
            l.IconBetrieb, l.IconBK, l.Datum, l.Beleg, -1.0,
            l.Netto, l.Satz, l.Betrag, l.Brutto, l.Saldo, l.Afa
        };

        // Anlagenverzeichnis: [Icon][Beschreibung*][AnschDatum]
        // [AnschKosten][BuchwBeginn][AfA][Abgang][BuchwEnde][AfA-Nr]
        private static double[] AnlagenSpalten(Layout l) => new[]
        {
            l.IconBetrieb, -1.0, l.Datum,
            l.Brutto, l.Brutto, l.Brutto, l.Brutto, l.Brutto, l.Afa > 0 ? l.Afa : 60.0 * S
        };

        private static Grid BaueGrid(double[] spalten)
        {
            var grid = new Grid();
            foreach (var b in spalten)
            {
                grid.ColumnDefinitions.Add(new ColumnDefinition
                {
                    Width = (b < 0)
                        ? new GridLength(1, GridUnitType.Star)
                        : new GridLength(Math.Max(0.0, b))
                });
            }
            return grid;
        }

        private static TextBlock Zelle(
            Grid grid, int spalte, string text, FontFamily schrift,
            TextAlignment ausrichtung = TextAlignment.Left,
            FontWeight? gewicht = null, bool betragsschrift = false,
            bool umbruch = false, Brush farbe = null)
        {
            var tb = new TextBlock
            {
                Text = text ?? "",
                FontFamily = betragsschrift ? SchriftBetraege : schrift,
                FontSize = DruckSchrift,
                TextAlignment = ausrichtung,
                Margin = new Thickness(3, 0, 3, 0),
                VerticalAlignment = VerticalAlignment.Top
            };
            if (gewicht.HasValue) tb.FontWeight = gewicht.Value;
            if (umbruch) tb.TextWrapping = TextWrapping.Wrap;
            if (farbe != null) tb.Foreground = farbe;
            Grid.SetColumn(tb, spalte);
            grid.Children.Add(tb);
            return tb;
        }

        private static void IconZelle(
            Grid grid, int spalte, ImageSource quelle, double groesse)
        {
            if (quelle == null) return;
            var img = new Image
            {
                Source = quelle,
                Width = groesse,
                Height = groesse,
                Stretch = Stretch.Uniform,
                VerticalAlignment = VerticalAlignment.Top,
                HorizontalAlignment = HorizontalAlignment.Center
            };
            Grid.SetColumn(img, spalte);
            grid.Children.Add(img);
        }

        private static Border MitRahmen(
            UIElement inhalt, bool oben, bool unten)
        {
            return new Border
            {
                BorderBrush = Brushes.Black,
                BorderThickness = new Thickness(
                    0, oben ? 0.75 : 0, 0, unten ? 0.75 : 0),
                Padding = new Thickness(0, 2, 0, 2),
                Child = inhalt
            };
        }

        // ----------------------------------------------------------
        // Zeilen-Renderer (Spiegel der DataTemplates in JournalView.xaml)
        // ----------------------------------------------------------

        private static UIElement BaueSektionsTitel(
            JournalSectionTitle t, FontFamily schrift)
        {
            Brush farbe = Brushes.Black;
            if (t.IsEinnahme == true) farbe = FarbeEinnahme;
            else if (t.IsEinnahme == false) farbe = FarbeAusgabe;

            return new TextBlock
            {
                Text = t.Text ?? "",
                FontFamily = schrift,
                FontSize = (t.IsMain ? 17.0 : 15.0) * S,
                FontWeight = FontWeights.Bold,
                Foreground = farbe,
                Margin = new Thickness(0, 8, 0, 3)
            };
        }

        private static UIElement BaueHeader(
            JournalHeaderRow h, Layout l, FontFamily schrift)
        {
            var grid = BaueGrid(StandardSpalten(l));
            Zelle(grid, 2, "Datum", schrift, gewicht: FontWeights.SemiBold);
            if (l.ZeigeBeleg && h.ZeigeBelegnummer)
                Zelle(grid, 3, "Beleg", schrift, gewicht: FontWeights.SemiBold);
            Zelle(grid, 4, "Beschreibung", schrift, gewicht: FontWeights.SemiBold);
            if (l.ZeigeSteuer && h.ZeigeSteuer)
            {
                Zelle(grid, 5, "Netto", schrift, TextAlignment.Right,
                    FontWeights.SemiBold);
                Zelle(grid, 6, h.IsAusgabe ? "VSt" : "USt", schrift,
                    TextAlignment.Right, FontWeights.SemiBold);
                Zelle(grid, 7, h.IsAusgabe ? "VSt-Betr" : "USt-Betr", schrift,
                    TextAlignment.Right, FontWeights.SemiBold);
            }
            Zelle(grid, 8, "Brutto", schrift, TextAlignment.Right,
                FontWeights.SemiBold);
            if (l.ZeigeSaldo && h.ZeigeSaldo)
                Zelle(grid, 9, "Saldo", schrift, TextAlignment.Right,
                    FontWeights.SemiBold);
            if (l.ZeigeAfa && h.ZeigeAfaNr)
                Zelle(grid, 10, "AfA-Nr", schrift, TextAlignment.Right,
                    FontWeights.SemiBold);
            return MitRahmen(grid, oben: false, unten: true);
        }

        private static UIElement BaueBuchungsZeile(
            JournalBuchungRow b, Layout l, FontFamily schrift)
        {
            var grid = BaueGrid(StandardSpalten(l));
            grid.Margin = new Thickness(0, 1, 0, 1);

            IconZelle(grid, 0, b.BetriebIcon, l.IconGroesse);
            IconZelle(grid, 1, b.BestandskontoIcon, l.IconGroesse);
            Zelle(grid, 2, b.DatumText, schrift);
            if (l.ZeigeBeleg)
                Zelle(grid, 3, b.BelegText, schrift);

            // Beschreibung: "[G]"-Präfix für Gruppen-Mitglieder wie am
            // Bildschirm, aber als Text-Präfix statt eigener Spalte.
            string beschreibung = b.IstGruppenMitglied
                ? "[G] " + (b.BeschreibungText ?? "")
                : b.BeschreibungText;
            Zelle(grid, 4, beschreibung, schrift, umbruch: true);

            if (l.ZeigeSteuer)
            {
                Zelle(grid, 5, b.NettoText, schrift, TextAlignment.Right,
                    betragsschrift: true);
                Zelle(grid, 6, b.MwstSatzText, schrift, TextAlignment.Right);
                Zelle(grid, 7, b.MwstBetragText, schrift, TextAlignment.Right,
                    betragsschrift: true);
            }
            Zelle(grid, 8, b.BruttoText, schrift, TextAlignment.Right,
                FontWeights.SemiBold, betragsschrift: true);
            if (l.ZeigeSaldo)
                Zelle(grid, 9, b.SaldoText, schrift, TextAlignment.Right,
                    FontWeights.SemiBold, betragsschrift: true);
            if (l.ZeigeAfa)
                Zelle(grid, 10, b.AfaNrText, schrift, TextAlignment.Right);
            return grid;
        }

        private static UIElement BaueFooter(
            JournalFooterRow f, Layout l, FontFamily schrift)
        {
            var grid = BaueGrid(StandardSpalten(l));
            Zelle(grid, 4, f.LinkesLabel, schrift, gewicht: FontWeights.Bold);
            if (l.ZeigeSteuer && f.ZeigeSteuer)
            {
                Zelle(grid, 5, f.NettoSummeText, schrift, TextAlignment.Right,
                    FontWeights.Bold, betragsschrift: true);
                Zelle(grid, 7, f.SteuerSummeText, schrift, TextAlignment.Right,
                    FontWeights.Bold, betragsschrift: true);
            }
            Zelle(grid, 8, f.BruttoSummeText, schrift, TextAlignment.Right,
                FontWeights.Bold, betragsschrift: true);
            if (l.ZeigeSaldo)
                Zelle(grid, 9, f.SaldoSummeText, schrift, TextAlignment.Right,
                    FontWeights.Bold, betragsschrift: true);
            Zelle(grid, 10, f.Waehrung, schrift, TextAlignment.Right,
                FontWeights.Bold);
            return MitRahmen(grid, oben: true, unten: true);
        }

        private static UIElement BaueAnlagenHeader(Layout l, FontFamily schrift)
        {
            var grid = BaueGrid(AnlagenSpalten(l));
            Zelle(grid, 1, "Beschreibung", schrift, gewicht: FontWeights.SemiBold);
            Zelle(grid, 2, "Ansch.Datum", schrift, gewicht: FontWeights.SemiBold);
            Zelle(grid, 3, "Ansch.kosten", schrift, TextAlignment.Right,
                FontWeights.SemiBold);
            Zelle(grid, 4, "Buchw.Beginn", schrift, TextAlignment.Right,
                FontWeights.SemiBold);
            Zelle(grid, 5, "AfA", schrift, TextAlignment.Right,
                FontWeights.SemiBold);
            Zelle(grid, 6, "Abgänge", schrift, TextAlignment.Right,
                FontWeights.SemiBold);
            Zelle(grid, 7, "Buchw.Ende", schrift, TextAlignment.Right,
                FontWeights.SemiBold);
            Zelle(grid, 8, "AfA-Nr", schrift, TextAlignment.Right,
                FontWeights.SemiBold);
            return MitRahmen(grid, oben: false, unten: true);
        }

        private static UIElement BaueAnlagenZeile(
            JournalAnlagenRow a, Layout l, FontFamily schrift)
        {
            var grid = BaueGrid(AnlagenSpalten(l));
            grid.Margin = new Thickness(0, 1, 0, 1);

            IconZelle(grid, 0, a.BetriebIcon, l.IconGroesse);
            Zelle(grid, 1, a.BeschreibungText, schrift, umbruch: true);
            Zelle(grid, 2, a.AnschDatumText, schrift);
            Zelle(grid, 3, a.AnschKostenText, schrift, TextAlignment.Right,
                betragsschrift: true);
            Zelle(grid, 4, a.BuchwBeginnText, schrift, TextAlignment.Right,
                betragsschrift: true);
            Zelle(grid, 5, a.AfaJahresbetragText, schrift, TextAlignment.Right,
                betragsschrift: true);
            Zelle(grid, 6, a.AbgangText, schrift, TextAlignment.Right,
                betragsschrift: true);
            Zelle(grid, 7, a.BuchwEndeText, schrift, TextAlignment.Right,
                FontWeights.SemiBold, betragsschrift: true);
            Zelle(grid, 8, a.AfaNrText, schrift, TextAlignment.Right);
            return grid;
        }

        private static UIElement BaueAnlagenFooter(
            JournalAnlagenFooterRow f, Layout l, FontFamily schrift)
        {
            var grid = BaueGrid(AnlagenSpalten(l));
            Zelle(grid, 3, f.AnschKostenSummeText, schrift, TextAlignment.Right,
                FontWeights.Bold, betragsschrift: true);
            Zelle(grid, 5, f.AfaSummeText, schrift, TextAlignment.Right,
                FontWeights.Bold, betragsschrift: true);
            Zelle(grid, 6, f.AbgangSummeText, schrift, TextAlignment.Right,
                FontWeights.Bold, betragsschrift: true);
            Zelle(grid, 7, f.BuchwEndeSummeText, schrift, TextAlignment.Right,
                FontWeights.Bold, betragsschrift: true);
            Zelle(grid, 8, f.Waehrung, schrift, TextAlignment.Right,
                FontWeights.Bold);
            return MitRahmen(grid, oben: true, unten: true);
        }
    }
}
