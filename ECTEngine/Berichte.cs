// Berichte.cs -- Gemeinsames Datenmodell für die formlosen Berichte
// (Freestyle-EÜR, formlose USt-Erklärung, Kontenplan).
//
// Ein Bericht ist eine flache Liste von Zeilen mit Stil-Attributen --
// bewusst OHNE WPF-Abhängigkeit, damit die Berechnungen in ECTEngine
// testbar bleiben. Die Darstellung (BerichtView) und der Druck
// (BerichtDruckBauer in ECTViews) rendern dieses Modell.
//
// Ersetzt die Zeichen-Logik von DrawEURechungToDC / DrawUmStErklaerungToDC
// (easycashview.cpp) im USE_ECTENGINE-Pfad.

using System;
using System.Collections.Generic;
using System.Globalization;

namespace ECTEngine
{
    public enum BerichtStil
    {
        /// <summary>Normale Posten-Zeile.</summary>
        Normal,
        /// <summary>Abschnitts-Überschrift (fett, etwas größer).</summary>
        Ueberschrift,
        /// <summary>Zwischentitel (fett, z.B. Unterkategorie "1.2 ...").</summary>
        Zwischentitel,
        /// <summary>Summenzeile (fett).</summary>
        Summe,
        /// <summary>Hinweis-/Fehlertext (kursiv/grau).</summary>
        Hinweis
    }

    /// <summary>
    /// Eine Berichtszeile: linksbündiger Text plus 0..n rechtsbündige
    /// Wertspalten. Bei mehrspaltigen Tabellen (USt-Erklärung Abschnitt C)
    /// haben alle Zeilen eines Blocks dieselbe Werte-Anzahl.
    /// </summary>
    public class BerichtZeile
    {
        public string Text { get; set; } = "";

        /// <summary>Rechtsbündige Wertspalten (null oder leer = keine).</summary>
        public string[] Werte { get; set; }

        /// <summary>Einrückungsstufe des Textes (0..2).</summary>
        public int Einrueckung { get; set; }

        public BerichtStil Stil { get; set; } = BerichtStil.Normal;

        /// <summary>Trennlinie oberhalb der Zeile (vor Summen).</summary>
        public bool LinieOben { get; set; }

        /// <summary>Trennlinie unterhalb der Zeile.</summary>
        public bool LinieUnten { get; set; }
    }

    /// <summary>
    /// Ein kompletter formloser Bericht: Titel, Adresskopf (links Anschrift,
    /// rechts z.B. Steuernummer) und die Zeilenliste.
    /// </summary>
    public class Bericht
    {
        public string Titel { get; set; } = "";

        /// <summary>Filter-/Zeitraumbeschreibung unter dem Titel.</summary>
        public string Untertitel { get; set; } = "";

        /// <summary>Adressblock links (Name, Unternehmensart, Straße, PLZ Ort).</summary>
        public List<string> KopfLinks { get; } = new List<string>();

        /// <summary>Block rechts oben (z.B. "Steuernummer" + Wert).</summary>
        public List<string> KopfRechts { get; } = new List<string>();

        public List<BerichtZeile> Zeilen { get; } = new List<BerichtZeile>();

        public string Waehrung { get; set; } = "";

        // Komfort-Helfer für die Erzeuger ----------------------------------

        public BerichtZeile Add(string text, BerichtStil stil = BerichtStil.Normal,
            int einrueckung = 0)
        {
            var z = new BerichtZeile
            {
                Text = text ?? "",
                Stil = stil,
                Einrueckung = einrueckung
            };
            Zeilen.Add(z);
            return z;
        }

        public BerichtZeile Add(string text, string wert,
            BerichtStil stil = BerichtStil.Normal, int einrueckung = 0)
        {
            var z = Add(text, stil, einrueckung);
            z.Werte = new[] { wert };
            return z;
        }

        public BerichtZeile Add(string text, string[] werte,
            BerichtStil stil = BerichtStil.Normal, int einrueckung = 0)
        {
            var z = Add(text, stil, einrueckung);
            z.Werte = werte;
            return z;
        }

        public void Leerzeile() => Zeilen.Add(new BerichtZeile());
    }

    /// <summary>
    /// Leitet aus dem Ribbon-Monatsfilter (0 = alle, 1-12 = Monat,
    /// 13-16 = Quartal) den Datumsbereich und eine Textbeschreibung ab.
    /// Ersetzt den tagesgenauen von/bis-Bereich des alten Druckdialogs.
    /// </summary>
    public static class BerichtZeitraum
    {
        public static void AusMonatsfilter(int jahr, int monatsfilter,
            out DateTime von, out DateTime bis)
        {
            if (jahr < 1 || jahr > 9998) jahr = DateTime.Now.Year;

            int vonMonat = 1, bisMonat = 12;
            if (monatsfilter >= 1 && monatsfilter <= 12)
            {
                vonMonat = bisMonat = monatsfilter;
            }
            else if (monatsfilter >= 13 && monatsfilter <= 16)
            {
                vonMonat = (monatsfilter - 13) * 3 + 1;
                bisMonat = vonMonat + 2;
            }

            von = new DateTime(jahr, vonMonat, 1, 0, 0, 0);
            bis = new DateTime(jahr, bisMonat,
                DateTime.DaysInMonth(jahr, bisMonat), 23, 59, 59);
        }

        /// <summary>True wenn der Filter das komplette Jahr abdeckt.</summary>
        public static bool IstGanzesJahr(int monatsfilter) =>
            monatsfilter < 1 || monatsfilter > 16;

        /// <summary>"für 2026" / "für März 2026" / "für das 2. Quartal 2026".</summary>
        public static string Beschreibung(int jahr, int monatsfilter)
        {
            if (monatsfilter >= 1 && monatsfilter <= 12)
                return string.Format(CultureInfo.CurrentCulture, "für {0} {1}",
                    CultureInfo.CurrentCulture.DateTimeFormat.GetMonthName(monatsfilter),
                    jahr);
            if (monatsfilter >= 13 && monatsfilter <= 16)
                return string.Format(CultureInfo.CurrentCulture,
                    "für das {0}. Quartal {1}", monatsfilter - 12, jahr);
            return "für " + jahr.ToString(CultureInfo.InvariantCulture);
        }
    }
}
