using System;
using System.Collections.Generic;
using ECTEngine.Models;

namespace ECTEngine.Calculations
{
    /// <summary>
    /// Verwaltet den Jahreswechsel - Überträge mit Abschreibungsanpassungen
    /// </summary>
    public class YearTransitionEngine
    {
        private readonly SortingEngine _sortingEngine = new();

        /// <summary>
        /// Erstellt ein neues Dokument für das Folgejahr mit angepassten Abschreibungen
        /// </summary>
        public EasyCashDocument CreateNextYearDocument(EasyCashDocument currentYear)
        {
            var nextYear = new EasyCashDocument
            {
                Buchungszaehler = currentYear.Buchungszaehler,
                Jahr = currentYear.Jahr + 1,
                Waehrung = currentYear.Waehrung,
                UrspruenglicheWaehrung = currentYear.UrspruenglicheWaehrung,
                Erweiterung = currentYear.Erweiterung,
                AbschreibungGenauigkeit = AdjustAbschreibungsGenauigkeit(currentYear.Jahr + 1, currentYear.AbschreibungGenauigkeit)
            };

            // Ausgaben mit Abschreibungen ins nächste Jahr übertragen
            TransferAusgabenWithAbschreibungen(currentYear, nextYear);

            // Dauerbuchungen komplett übertragen
            TransferDauerbuchungen(currentYear, nextYear);

            // Ggf. zu Euro konvertieren
            if (nextYear.Jahr >= 2002 && nextYear.Waehrung != "EUR")
            {
                nextYear.ConvertToEuro();
            }

            return nextYear;
        }

        /// <summary>
        /// Überträgt Ausgaben mit Abschreibungsresten ins nächste Jahr
        /// </summary>
        private void TransferAusgabenWithAbschreibungen(EasyCashDocument sourceYear, EasyCashDocument targetYear)
        {
            var slashOldYear = $"/{sourceYear.Jahr}";

            foreach (var buchung in sourceYear.Ausgaben)
            {
                // Nur Abschreibungen mit Restwert übertragen
                if (buchung.AbschreibungJahre <= 1)
                    continue;

                long yearlyDepreciation = buchung.GetBuchungsjahrNetto(sourceYear);
                if (buchung.AbschreibungRestwert <= yearlyDepreciation)
                    continue;

                // Buchung kopieren und für neues Jahr anpassen
                var nextYearBuchung = new Buchung();
                nextYearBuchung.CopyFrom(buchung);

                // Belegnummer mit Jahreszahl erweitern
                if (!string.IsNullOrEmpty(nextYearBuchung.Belegnummer) 
                    && !nextYearBuchung.Belegnummer.EndsWith("/20"))
                {
                    nextYearBuchung.Belegnummer += slashOldYear;
                }

                // Restwert anpassen
                nextYearBuchung.AbschreibungRestwert -= yearlyDepreciation;
                nextYearBuchung.AbschreibungNr++;
                nextYearBuchung.Datum = new DateTime(
                    nextYearBuchung.Datum.Year + 1,
                    nextYearBuchung.Datum.Month,
                    nextYearBuchung.Datum.Day);

                // Prüfe ob degressive AfA noch sinnvoll ist
                AdjustDepreciationMethod(nextYearBuchung, sourceYear);

                targetYear.Ausgaben.AddLast(nextYearBuchung);
            }

            _sortingEngine.SortiereListe(targetYear.Ausgaben);
        }

        /// <summary>
        /// Passt die Abschreibungsmethode an, falls degressive AfA nicht mehr rentabel ist
        /// </summary>
        private void AdjustDepreciationMethod(Buchung buchung, EasyCashDocument doc)
        {
            if (!buchung.AbschreibungDegressiv)
                return;

            // Ausnahme: Elektroauto-AfA
            if (buchung.AbschreibungSatz == 75 && 
                buchung.AbschreibungGenauigkeit == AbschreibungsGenauigkeit.GanzjahresAfa)
                return;

            // Vergleiche degressive vs. lineare AfA
            long rateDegressiv = buchung.GetBuchungsjahrNetto(doc);
            
            buchung.AbschreibungDegressiv = false;
            long rateLinear = buchung.GetBuchungsjahrNetto(doc);
            
            if (rateDegressiv < rateLinear)
            {
                // Degressive AfA bleibt ausgeschaltet (linear ist besser)
                return;
            }

            // Sonst degressive AfA wiederherstellen
            buchung.AbschreibungDegressiv = true;
        }

        /// <summary>
        /// Überträgt alle Dauerbuchungen ins nächste Jahr
        /// </summary>
        private void TransferDauerbuchungen(EasyCashDocument sourceYear, EasyCashDocument targetYear)
        {
            foreach (var dauerBuchung in sourceYear.Dauerbuchungen)
            {
                var nextYearDauerBuchung = new Dauerbuchung();
                nextYearDauerBuchung.CopyFrom(dauerBuchung);
                targetYear.Dauerbuchungen.AddLast(nextYearDauerBuchung);
            }
        }

        /// <summary>
        /// Passt die Abschreibungsgenauigkeit an (z.B. ab 2004 in Deutschland: monatgenau statt halbjährlich)
        /// </summary>
        private AbschreibungsGenauigkeit AdjustAbschreibungsGenauigkeit(int jahr, AbschreibungsGenauigkeit current)
        {
            // Ab 2004 in Deutschland Umstellung auf monatsgenaue AfA
            if (jahr == 2004)
                return AbschreibungsGenauigkeit.MonatsgenauAfa;

            return current;
        }
    }
}