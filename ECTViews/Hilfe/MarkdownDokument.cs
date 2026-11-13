// MarkdownDokument.cs -- macht aus einem Markdown-Text ein WPF-FlowDocument
//
// Die Benutzer-Anleitungen (ECTViews/Hilfe/*.md) sind bewusst schlichtes
// Markdown: so lassen sie sich im Repository lesen und pflegen, auf GitHub
// ansehen und ohne Umweg in die Programm-Hilfe einbetten. Dieser Umsetzer
// kennt genau den Ausschnitt, den die Anleitungen brauchen -- kein
// vollständiges Markdown:
//
//   # / ## / ###          Überschriften
//   Absätze               (aufeinanderfolgende Zeilen gehören zusammen)
//   - Punkt / 1. Punkt    Listen (Fortsetzungszeilen eingerückt)
//   | a | b |             Tabellen (zweite Zeile = Trennzeile |---|---|)
//   ```                   Code-Block
//   > Hinweis             Hinweis-Kasten
//   ---                   Trennlinie
//   `code` **fett** *kursiv*   im Fließtext
//
// Unbekanntes wird als normaler Text ausgegeben, nie verworfen.

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Media;

namespace ECTViews.Hilfe
{
    /// <summary>Eine Überschrift des Dokuments (für das Inhaltsverzeichnis).</summary>
    public sealed class HilfeUeberschrift
    {
        public string Text { get; internal set; }
        /// <summary>1 = Titel, 2 = Abschnitt, 3 = Unterabschnitt.</summary>
        public int Ebene { get; internal set; }
        /// <summary>Der Absatz im Dokument (Sprungziel).</summary>
        public Block Block { get; internal set; }
        /// <summary>Einrückung im Inhaltsverzeichnis.</summary>
        public Thickness Einzug => new Thickness((Ebene - 2) * 14, 1, 0, 1);
        public FontWeight Gewicht => Ebene <= 2 ? FontWeights.SemiBold : FontWeights.Normal;
        public override string ToString() => Text;
    }

    public static class MarkdownDokument
    {
        private static readonly FontFamily CodeSchrift = new FontFamily("Consolas");
        private static readonly Brush CodeHintergrund = Fest(0xF1, 0xF3, 0xF5);
        private static readonly Brush CodeFarbe = Fest(0x8A, 0x1C, 0x4A);
        private static readonly Brush Akzent = Fest(0x15, 0x65, 0xC0);
        private static readonly Brush Linie = Fest(0xD0, 0xD0, 0xD0);
        private static readonly Brush KopfHintergrund = Fest(0xEE, 0xF3, 0xF9);
        private static readonly Brush HinweisHintergrund = Fest(0xFF, 0xF8, 0xE1);
        private static readonly Brush HinweisRand = Fest(0xE0, 0xC4, 0x6C);

        private static Brush Fest(byte r, byte g, byte b)
        {
            var pinsel = new SolidColorBrush(Color.FromRgb(r, g, b));
            pinsel.Freeze();
            return pinsel;
        }

        /// <summary>
        /// Baut das Dokument. <paramref name="ueberschriften"/> bekommt die
        /// Überschriften der Ebenen 2 und 3 in Dokument-Reihenfolge.
        /// </summary>
        public static FlowDocument Baue(string markdown, List<HilfeUeberschrift> ueberschriften)
        {
            var doc = new FlowDocument
            {
                FontFamily = new FontFamily("Segoe UI"),
                FontSize = 13.5,
                PagePadding = new Thickness(24, 16, 24, 24),
                TextAlignment = TextAlignment.Left,
                IsOptimalParagraphEnabled = false,
                IsHyphenationEnabled = false,
                // sonst bricht der Viewer in schmale Zeitungs-Spalten um
                ColumnWidth = double.PositiveInfinity
            };

            var zeilen = (markdown ?? "").Replace("\r\n", "\n").Replace('\r', '\n').Split('\n');
            int i = 0;
            while (i < zeilen.Length)
            {
                string zeile = zeilen[i];
                string t = zeile.Trim();

                if (t.Length == 0) { i++; continue; }

                // Code-Block
                if (t.StartsWith("```", StringComparison.Ordinal))
                {
                    var code = new StringBuilder();
                    i++;
                    while (i < zeilen.Length
                           && !zeilen[i].Trim().StartsWith("```", StringComparison.Ordinal))
                    {
                        if (code.Length > 0) code.Append('\n');
                        code.Append(zeilen[i]);
                        i++;
                    }
                    i++;   // schließende ```
                    doc.Blocks.Add(new Paragraph(new Run(code.ToString()))
                    {
                        FontFamily = CodeSchrift,
                        FontSize = 12.5,
                        Background = CodeHintergrund,
                        Padding = new Thickness(10, 7, 10, 7),
                        Margin = new Thickness(0, 4, 0, 10),
                        TextAlignment = TextAlignment.Left
                    });
                    continue;
                }

                // Überschrift
                int ebene = UeberschriftEbene(t);
                if (ebene > 0)
                {
                    string text = t.Substring(ebene).Trim();
                    var kopf = new Paragraph
                    {
                        FontWeight = FontWeights.SemiBold,
                        FontSize = ebene == 1 ? 22 : ebene == 2 ? 17 : 14.5,
                        Foreground = ebene <= 2 ? Akzent : Brushes.Black,
                        Margin = new Thickness(0, ebene == 1 ? 0 : ebene == 2 ? 20 : 14, 0, 6),
                        KeepWithNext = true
                    };
                    FuegeInlinesHinzu(kopf.Inlines, text);
                    doc.Blocks.Add(kopf);
                    if (ebene >= 2 && ueberschriften != null)
                        ueberschriften.Add(new HilfeUeberschrift
                        {
                            Text = OhneAuszeichnung(text), Ebene = ebene, Block = kopf
                        });
                    i++;
                    continue;
                }

                // Trennlinie
                if (t == "---" || t == "***")
                {
                    doc.Blocks.Add(new Paragraph
                    {
                        BorderBrush = Linie,
                        BorderThickness = new Thickness(0, 1, 0, 0),
                        Margin = new Thickness(0, 10, 0, 10),
                        FontSize = 1
                    });
                    i++;
                    continue;
                }

                // Tabelle
                if (t.StartsWith("|", StringComparison.Ordinal))
                {
                    var tabellenZeilen = new List<string>();
                    while (i < zeilen.Length
                           && zeilen[i].Trim().StartsWith("|", StringComparison.Ordinal))
                    {
                        tabellenZeilen.Add(zeilen[i].Trim());
                        i++;
                    }
                    doc.Blocks.Add(BaueTabelle(tabellenZeilen));
                    continue;
                }

                // Hinweis-Kasten
                if (t.StartsWith(">", StringComparison.Ordinal))
                {
                    var text = new StringBuilder();
                    while (i < zeilen.Length
                           && zeilen[i].Trim().StartsWith(">", StringComparison.Ordinal))
                    {
                        if (text.Length > 0) text.Append(' ');
                        text.Append(zeilen[i].Trim().Substring(1).Trim());
                        i++;
                    }
                    var kasten = new Paragraph
                    {
                        Background = HinweisHintergrund,
                        BorderBrush = HinweisRand,
                        BorderThickness = new Thickness(3, 0, 0, 0),
                        Padding = new Thickness(10, 6, 10, 6),
                        Margin = new Thickness(0, 4, 0, 10)
                    };
                    FuegeInlinesHinzu(kasten.Inlines, text.ToString());
                    doc.Blocks.Add(kasten);
                    continue;
                }

                // Liste
                if (IstListenpunkt(t, out bool nummeriert, out _))
                {
                    var liste = new List
                    {
                        MarkerStyle = nummeriert ? TextMarkerStyle.Decimal : TextMarkerStyle.Disc,
                        Margin = new Thickness(0, 2, 0, 8),
                        Padding = new Thickness(22, 0, 0, 0)
                    };
                    while (i < zeilen.Length
                           && IstListenpunkt(zeilen[i].Trim(), out bool n2, out string inhalt)
                           && n2 == nummeriert)
                    {
                        var punkt = new StringBuilder(inhalt);
                        i++;
                        // eingerückte Fortsetzungszeilen gehören zum Punkt
                        while (i < zeilen.Length && zeilen[i].Length > 0
                               && char.IsWhiteSpace(zeilen[i][0])
                               && zeilen[i].Trim().Length > 0
                               && !IstListenpunkt(zeilen[i].Trim(), out _, out _))
                        {
                            punkt.Append(' ').Append(zeilen[i].Trim());
                            i++;
                        }
                        var absatz = new Paragraph { Margin = new Thickness(0, 1, 0, 3) };
                        FuegeInlinesHinzu(absatz.Inlines, punkt.ToString());
                        liste.ListItems.Add(new ListItem(absatz));
                    }
                    doc.Blocks.Add(liste);
                    continue;
                }

                // Absatz: bis zur nächsten Leerzeile bzw. zum nächsten Block
                var fliesstext = new StringBuilder(t);
                i++;
                while (i < zeilen.Length)
                {
                    string n = zeilen[i].Trim();
                    if (n.Length == 0 || UeberschriftEbene(n) > 0
                        || n.StartsWith("|", StringComparison.Ordinal)
                        || n.StartsWith(">", StringComparison.Ordinal)
                        || n.StartsWith("```", StringComparison.Ordinal)
                        || n == "---" || IstListenpunkt(n, out _, out _))
                        break;
                    fliesstext.Append(' ').Append(n);
                    i++;
                }
                var p = new Paragraph { Margin = new Thickness(0, 0, 0, 8), LineHeight = 20 };
                FuegeInlinesHinzu(p.Inlines, fliesstext.ToString());
                doc.Blocks.Add(p);
            }
            return doc;
        }

        private static int UeberschriftEbene(string t)
        {
            int n = 0;
            while (n < t.Length && t[n] == '#') n++;
            return n >= 1 && n <= 3 && n < t.Length && t[n] == ' ' ? n : 0;
        }

        private static bool IstListenpunkt(string t, out bool nummeriert, out string inhalt)
        {
            nummeriert = false;
            inhalt = "";
            if (t.StartsWith("- ", StringComparison.Ordinal)
                || t.StartsWith("* ", StringComparison.Ordinal))
            {
                inhalt = t.Substring(2).Trim();
                return true;
            }
            int n = 0;
            while (n < t.Length && char.IsDigit(t[n])) n++;
            if (n > 0 && n + 1 < t.Length && t[n] == '.' && t[n + 1] == ' ')
            {
                nummeriert = true;
                inhalt = t.Substring(n + 2).Trim();
                return true;
            }
            return false;
        }

        // ------------------------------------------------------------------
        // Tabelle
        // ------------------------------------------------------------------

        private static Table BaueTabelle(List<string> zeilen)
        {
            var tabelle = new Table
            {
                CellSpacing = 0,
                Margin = new Thickness(0, 4, 0, 12),
                BorderBrush = Linie,
                BorderThickness = new Thickness(1, 1, 0, 0)
            };
            var gruppe = new TableRowGroup();
            tabelle.RowGroups.Add(gruppe);

            int spalten = 0;
            bool kopf = true;
            foreach (var z in zeilen)
            {
                var zellen = ZerlegeTabellenzeile(z);
                if (zellen.Count == 0) continue;

                // Trennzeile |---|---| markiert nur das Ende des Kopfs
                if (zellen.TrueForAll(c => c.Length > 0 && c.Trim('-', ':', ' ').Length == 0))
                {
                    kopf = false;
                    continue;
                }

                spalten = Math.Max(spalten, zellen.Count);
                var reihe = new TableRow();
                if (kopf) reihe.Background = KopfHintergrund;
                foreach (var inhalt in zellen)
                {
                    var absatz = new Paragraph { Margin = new Thickness(0) };
                    if (kopf) absatz.FontWeight = FontWeights.SemiBold;
                    FuegeInlinesHinzu(absatz.Inlines, inhalt);
                    reihe.Cells.Add(new TableCell(absatz)
                    {
                        Padding = new Thickness(7, 4, 7, 4),
                        BorderBrush = Linie,
                        BorderThickness = new Thickness(0, 0, 1, 1)
                    });
                }
                gruppe.Rows.Add(reihe);
            }

            // Erste Spalte schmaler (meist ein Stichwort), Rest teilt sich
            // den Platz
            for (int s = 0; s < spalten; s++)
                tabelle.Columns.Add(new TableColumn
                {
                    Width = new GridLength(s == 0 && spalten > 2 ? 1.2 : 2, GridUnitType.Star)
                });
            return tabelle;
        }

        private static List<string> ZerlegeTabellenzeile(string zeile)
        {
            var zellen = new List<string>();
            string t = zeile.Trim();
            if (t.StartsWith("|", StringComparison.Ordinal)) t = t.Substring(1);
            if (t.EndsWith("|", StringComparison.Ordinal)) t = t.Substring(0, t.Length - 1);

            // '|' innerhalb von `code` trennt keine Zellen (Konto-
            // Spezifikationen enthalten Pipes); "\|" ist ein wörtliches '|'
            var aktuell = new StringBuilder();
            bool imCode = false;
            for (int i = 0; i < t.Length; i++)
            {
                char c = t[i];
                if (c == '`') imCode = !imCode;
                if (c == '\\' && i + 1 < t.Length && t[i + 1] == '|')
                {
                    aktuell.Append('|');
                    i++;
                    continue;
                }
                if (c == '|' && !imCode)
                {
                    zellen.Add(aktuell.ToString().Trim());
                    aktuell.Clear();
                    continue;
                }
                aktuell.Append(c);
            }
            zellen.Add(aktuell.ToString().Trim());
            return zellen;
        }

        // ------------------------------------------------------------------
        // Auszeichnungen im Fließtext: `code`, **fett**, *kursiv*
        // ------------------------------------------------------------------

        private static void FuegeInlinesHinzu(InlineCollection ziel, string text)
        {
            var puffer = new StringBuilder();
            bool fett = false, kursiv = false;

            void Leere()
            {
                if (puffer.Length == 0) return;
                Inline run = new Run(puffer.ToString());
                if (kursiv) run = new Italic(run);
                if (fett) run = new Bold(run);
                ziel.Add(run);
                puffer.Clear();
            }

            for (int i = 0; i < text.Length; i++)
            {
                char c = text[i];

                if (c == '`')
                {
                    int ende = text.IndexOf('`', i + 1);
                    if (ende > i)
                    {
                        Leere();
                        ziel.Add(new Run(text.Substring(i + 1, ende - i - 1))
                        {
                            FontFamily = CodeSchrift,
                            FontSize = 12.5,
                            Background = CodeHintergrund,
                            Foreground = CodeFarbe
                        });
                        i = ende;
                        continue;
                    }
                }
                else if (c == '*' && i + 1 < text.Length && text[i + 1] == '*')
                {
                    Leere();
                    fett = !fett;
                    i++;
                    continue;
                }
                else if (c == '*' && IstKursivMarke(text, i, kursiv))
                {
                    Leere();
                    kursiv = !kursiv;
                    continue;
                }
                puffer.Append(c);
            }
            Leere();
        }

        /// <summary>Ein einzelnes '*' ist nur dann eine Kursiv-Marke, wenn
        /// es an einem Wort klebt -- "$brutto * 0,3" bleibt ein Malzeichen.</summary>
        private static bool IstKursivMarke(string text, int i, bool offen)
        {
            bool davorLeer = i == 0 || char.IsWhiteSpace(text[i - 1]);
            bool danachLeer = i + 1 >= text.Length || char.IsWhiteSpace(text[i + 1]);
            return offen ? !davorLeer : (davorLeer && !danachLeer
                                         && text.IndexOf('*', i + 1) > i);
        }

        private static string OhneAuszeichnung(string text)
            => text.Replace("`", "").Replace("**", "").Replace("*", "");
    }
}
