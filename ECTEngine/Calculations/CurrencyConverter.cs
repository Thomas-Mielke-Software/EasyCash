using System;
using System.Collections.Generic;
using ECTEngine.Models;

namespace ECTEngine.Calculations
{
    /// <summary>
    /// Konvertiert Beträge und ganze Dokumente zwischen Währungen
    /// </summary>
    public class CurrencyConverter
    {
        /// <summary>
        /// Konvertiert ein komplettes Dokument zu Euro
        /// </summary>
        public bool ConvertDocumentToEuro(EasyCashDocument document)
        {
            if (document.Waehrung == "EUR")
                return true;

            try
            {
                // Konvertiere Einnahmen
                foreach (var buchung in document.Einnahmen)
                {
                    if (!buchung.ConvertToEuro(document.Waehrung))
                        return false;
                }

                // Konvertiere Ausgaben
                foreach (var buchung in document.Ausgaben)
                {
                    if (!buchung.ConvertToEuro(document.Waehrung))
                        return false;
                }

                // Konvertiere Dauerbuchungen
                foreach (var dauerBuchung in document.Dauerbuchungen)
                {
                    if (!dauerBuchung.ConvertToEuro(document.Waehrung))
                        return false;
                }

                document.Waehrung = "EUR";
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        /// <summary>
        /// Konvertiert ein komplettes Dokument von Euro zu einer anderen Währung
        /// </summary>
        public bool ConvertDocumentFromEuro(EasyCashDocument document, string targetCurrency)
        {
            if (targetCurrency == "EUR")
                return true;

            try
            {
                foreach (var buchung in document.Einnahmen)
                {
                    if (!buchung.ConvertFromEuro(targetCurrency))
                        return false;
                }

                foreach (var buchung in document.Ausgaben)
                {
                    if (!buchung.ConvertFromEuro(targetCurrency))
                        return false;
                }

                foreach (var dauerBuchung in document.Dauerbuchungen)
                {
                    if (!dauerBuchung.ConvertFromEuro(targetCurrency))
                        return false;
                }

                document.Waehrung = targetCurrency;
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
    }
}