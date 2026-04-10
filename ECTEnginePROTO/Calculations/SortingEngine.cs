using System;
using System.Collections.Generic;
using System.Linq;
using ECTEngine.Models;

namespace ECTEngine.Calculations
{
    /// <summary>
    /// Sortiert Buchungen chronologisch nach Datum, Belegnummer und Beschreibung
    /// </summary>
    public class SortingEngine
    {
        /// <summary>
        /// Sortiert beide Listen (Einnahmen und Ausgaben)
        /// </summary>
        public void SortiereBuchungen(EasyCashDocument document)
        {
            SortiereListe(document.Einnahmen);
            SortiereListe(document.Ausgaben);
        }

        /// <summary>
        /// Sortiert eine einzelne Buchungsliste
        /// </summary>
        private void SortiereListe(LinkedList<Buchung> buchungen)
        {
            if (buchungen.Count <= 1)
                return;

            var sortedList = buchungen
                .OrderBy(b => b.Datum)
                .ThenBy(b => b.Belegnummer)
                .ThenBy(b => b.Beschreibung)
                .ToList();

            buchungen.Clear();
            foreach (var buchung in sortedList)
                buchungen.AddLast(buchung);
        }

        /// <summary>
        /// Fügt eine Buchung an der richtigen Stelle in die sortierte Liste ein
        /// </summary>
        public void InsertSorted(LinkedList<Buchung> buchungen, Buchung neueBuchung)
        {
            if (buchungen.Count == 0)
            {
                buchungen.AddLast(neueBuchung);
                return;
            }

            var node = buchungen.First;
            while (node != null)
            {
                if (CompareBuchungen(neueBuchung, node.Value) < 0)
                {
                    buchungen.AddBefore(node, neueBuchung);
                    return;
                }
                node = node.Next;
            }

            // Am Ende hinzufügen
            buchungen.AddLast(neueBuchung);
        }

        /// <summary>
        /// Vergleicht zwei Buchungen nach Sortierkriterien
        /// Rückgabewert: < 0 wenn a vor b, > 0 wenn b vor a, 0 wenn gleich
        /// </summary>
        private int CompareBuchungen(Buchung a, Buchung b)
        {
            int dateComparison = a.Datum.CompareTo(b.Datum);
            if (dateComparison != 0)
                return dateComparison;

            int belegComparison = string.Compare(a.Belegnummer, b.Belegnummer, StringComparison.Ordinal);
            if (belegComparison != 0)
                return belegComparison;

            return string.Compare(a.Beschreibung, b.Beschreibung, StringComparison.Ordinal);
        }
    }
}