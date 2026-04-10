using System;
using System.Collections.Generic;

namespace ECTEngine.Helpers
{
    /// <summary>
    /// Hilfsklasse für Währungskonvertierungen und Verwaltung
    /// </summary>
    public static class WaehrungsHelper
    {
        private static readonly Dictionary<string, double> Kurse = new()
        {
            { "EUR", 1.0 },
            { "DEM", 1.95583 },      // Deutsche Mark
            { "BEF", 40.3399 },      // Belgischer Franc
            { "PTE", 166.386 },      // Portugiesischer Escudo
            { "FRF", 6.55957 },      // Französischer Franc
            { "IEP", 0.787564 },     // Irisches Pfund
            { "ITL", 1936.27 },      // Italienische Lira
            { "HFL", 2.20371 },      // Holländischer Gulden
            { "ATS", 13.7603 },      // Österreichischer Schilling
            { "FIM", 5.94573 }       // Finnische Mark
        };

        private static readonly string[] Waehrungen = { "EUR", "DEM", "BEF", "PTE", "FRF", "IEP", "ITL", "HFL", "ATS", "FIM" };

        /// <summary>
        /// Holt den Konversionskurs einer Währung zu Euro
        /// </summary>
        public static double GetKonversionskurs(string waehrungsKuerzel)
        {
            if (string.IsNullOrEmpty(waehrungsKuerzel))
                return 0;

            return Kurse.ContainsKey(waehrungsKuerzel) 
                ? Kurse[waehrungsKuerzel] 
                : 0;
        }

        /// <summary>
        /// Holt das Währungskürzel nach Index
        /// </summary>
        public static string GetWaehrungsKuerzel(int index)
        {
            if (index >= 0 && index < Waehrungen.Length)
                return Waehrungen[index];
            return "";
        }

        /// <summary>
        /// Prüft, ob eine Währung unterstützt wird
        /// </summary>
        public static bool IstUnterstuetztWaehrung(string waehrungsKuerzel)
        {
            return Kurse.ContainsKey(waehrungsKuerzel);
        }
    }
}