// KontenplanBericht.cs -- Kontenplan als formloser Bericht (neu, hatte
// kein Pendant im alten Druckdialog).
//
// Zwei Varianten:
//   - einfach: Einnahmen-/Ausgaben-Konten in Slot-Reihenfolge mit
//     Unterkategorie-Zwischentiteln
//   - mit Formular-Feldverknüpfungen: zusätzlich pro Konto die
//     Feldzuweisungen ("Formularname: Feld <Id> (<Bezeichnung>)"),
//     Bezeichnung aus der neuesten Formular-Variante (.ecf)
//
// Datenquelle ist ECTEngine.EUKonten (gleiche Sicht wie die
// E/Ü-Konten-Seite der Einstellungen).

using System;
using System.Collections.Generic;
using System.Linq;

namespace ECTEngine
{
    public static class KontenplanBericht
    {
        public static Bericht Erzeuge(bool mitFeldern)
        {
            EUKonten.Lade();

            var bericht = new Bericht
            {
                Titel = mitFeldern
                    ? "Kontenplan mit Formular-Feldverknüpfungen"
                    : "Kontenplan"
            };
            BerichtAdresse.FuelleKopf(bericht);

            SchreibeGruppe(bericht, "Einnahmen-Konten",
                EUKonten.EinnahmenKonten, mitFeldern);
            bericht.Leerzeile();
            SchreibeGruppe(bericht, "Ausgaben-Konten",
                EUKonten.AusgabenKonten, mitFeldern);

            return bericht;
        }

        private static void SchreibeGruppe(Bericht bericht, string titel,
            IReadOnlyList<EUKonto> konten, bool mitFeldern)
        {
            bericht.Add(titel, BerichtStil.Ueberschrift);

            if (konten.Count == 0)
            {
                bericht.Add("(keine Konten angelegt)", BerichtStil.Hinweis, 1);
                return;
            }

            foreach (var konto in konten)
            {
                // Unterkategorie-Marker sind slot-verankert und leiten eine
                // Gruppe ein (wie in der EÜR)
                if (!string.IsNullOrEmpty(konto.Unterkategorie))
                    bericht.Add(konto.Unterkategorie, BerichtStil.Zwischentitel, 1);

                bericht.Add(konto.Name, BerichtStil.Normal, 1);

                if (!mitFeldern) continue;

                foreach (var zuweisung in konto.Feldzuweisungen
                    .Where(kv => !string.IsNullOrEmpty(kv.Value))
                    .OrderBy(kv => kv.Key, StringComparer.CurrentCultureIgnoreCase))
                {
                    string bezeichnung = FeldBezeichnung(
                        zuweisung.Key, zuweisung.Value, konto.IstEinnahme);
                    string text = zuweisung.Key + ": Feld " + zuweisung.Value;
                    if (!string.IsNullOrEmpty(bezeichnung))
                        text += " (" + bezeichnung + ")";
                    bericht.Add(text, BerichtStil.Hinweis, 2);
                }
            }
        }

        /// <summary>
        /// Feld-Bezeichnung aus der neuesten Variante des Formulars;
        /// bevorzugt ein Feld mit passendem E/A-Typ, sonst nur über die Id.
        /// </summary>
        private static string FeldBezeichnung(
            string formularName, string feldId, bool istEinnahme)
        {
            var gruppe = EUKonten.FormularGruppen
                .FirstOrDefault(g => g.Name == formularName);
            var felder = gruppe?.Neueste?.Felder;
            if (felder == null) return "";

            var feld = felder.FirstOrDefault(
                    f => f.Id == feldId && f.IstEinnahme == istEinnahme)
                ?? felder.FirstOrDefault(f => f.Id == feldId);
            return feld?.Bezeichnung ?? "";
        }
    }
}
