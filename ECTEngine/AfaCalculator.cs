// AfaCalculator.cs — Abschreibungsberechnung
//
// Reimplementiert: CBuchung::GetBuchungsjahrNetto() und
//                  CBuchung::BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen()
// Quelle: EasyCashDoc.cpp Zeile 397-545
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

using System;

namespace ECTEngine
{
    /// <summary>
    /// Berechnet Abschreibungswerte (AfA = Absetzung für Abnutzung).
    ///
    /// Unterstützt:
    /// - Lineare AfA
    /// - Degressive AfA (mit automatischem Wechsel auf linear)
    /// - Sonder-AfA für Elektroautos (75% im 1. Jahr)
    /// - Drei Genauigkeitsstufen: Ganzjahr, Halbjahr, Monatsgenau
    /// </summary>
    public static class AfaCalculator
    {
        /// <summary>
        /// Berechnet den Netto-Abschreibungsbetrag für das aktuelle Buchungsjahr.
        /// 1:1-Reimplementierung von CBuchung::GetBuchungsjahrNetto().
        /// </summary>
        /// <param name="buchung">Die AfA-Buchung.</param>
        /// <param name="globaleGenauigkeit">
        /// Globale AfA-Genauigkeit aus CEasyCashDoc::AbschreibungGenauigkeit.
        /// Wird verwendet wenn buchung.AfaGenauigkeit == EntsprechendEinstellungen.
        /// </param>
        /// <returns>Jährlicher AfA-Betrag in Cent.</returns>
        public static long GetBuchungsjahrNetto(
            Buchung buchung,
            AfaGenauigkeit globaleGenauigkeit = AfaGenauigkeit.Monatsgenau)
        {
            // Effektive Genauigkeit bestimmen
            AfaGenauigkeit genauigkeit;
            if (buchung.AfaGenauigkeit == AfaGenauigkeit.EntsprechendEinstellungen)
                genauigkeit = globaleGenauigkeit;
            else
                genauigkeit = buchung.AfaGenauigkeit;

            // Nettobetrag
            long netto = buchung.BruttoBetrag.NettoInCent;

            // Keine AfA? Dann einfach netto zurückgeben
            if (buchung.AfaJahre <= 1)
                return netto;

            if (buchung.AfaDegressiv)
                return BerechneDegressiv(buchung, genauigkeit);
            else
                return BerechneLinear(buchung, genauigkeit);
        }

        // ──────────────────────────────────────────────
        // Degressive AfA
        // ──────────────────────────────────────────────

        private static long BerechneDegressiv(Buchung b, AfaGenauigkeit genauigkeit)
        {
            long netto = b.BruttoBetrag.NettoInCent;

            // Spezialfall: 75%-AfA im Anschaffungsjahr bei Elektroautos
            // (nur bei gleichzeitiger Ganzjahres-AfA)
            if (b.AfaSatz == 75 && b.AfaGenauigkeit == AfaGenauigkeit.Ganzjahr)
            {
                int[] eautoAfa = { 75, 10, 5, 5, 3, 2 };
                if (b.AfaNr >= 1 && b.AfaNr <= 6)
                    return netto * eautoAfa[b.AfaNr - 1] / 100;
                else
                    return 0L;
            }

            // Letztes Jahr: Restwert zurückgeben
            if (b.AfaNr == b.AfaJahre)
            {
                if (genauigkeit == AfaGenauigkeit.Ganzjahr ||
                    (genauigkeit == AfaGenauigkeit.Halbjahr && b.Datum.Month <= 6) ||
                    (genauigkeit == AfaGenauigkeit.Monatsgenau && b.Datum.Month <= 1))
                {
                    return b.AfaRestwertCent;
                }
            }

            // Erstes Jahr: ggf. verminderte Rate
            if (b.AfaNr == 1)
            {
                return RundenDiv10(
                    GenauigkeitAnwenden(
                        10L * b.AfaRestwertCent * b.AfaSatz / 100,
                        b, genauigkeit));
            }

            // Extra-Jahr bei nicht-ganzjähriger Genauigkeit
            if (b.AfaNr > b.AfaJahre)
                return b.AfaRestwertCent;

            // Jahre dazwischen: volle Jahresrate
            return RundenDiv10(10L * b.AfaRestwertCent * b.AfaSatz / 100);
        }

        // ──────────────────────────────────────────────
        // Lineare AfA
        // ──────────────────────────────────────────────

        private static long BerechneLinear(Buchung b, AfaGenauigkeit genauigkeit)
        {
            int gesamtMonate = 12 * b.AfaJahre;
            int verbleibendeMonate;

            switch (genauigkeit)
            {
                default:
                case AfaGenauigkeit.Ganzjahr:
                    verbleibendeMonate = (b.AfaJahre - b.AfaNr + 1) * 12;
                    break;

                case AfaGenauigkeit.Halbjahr:
                    if (b.AfaNr == 1)
                        verbleibendeMonate = b.AfaJahre * 12;
                    else if (b.Datum.Month < 7)
                        verbleibendeMonate = (b.AfaJahre - b.AfaNr + 1) * 12;
                    else
                        verbleibendeMonate = (b.AfaJahre - b.AfaNr + 2) * 12 - 6;
                    break;

                case AfaGenauigkeit.Monatsgenau:
                    if (b.AfaNr == 1)
                        verbleibendeMonate = b.AfaJahre * 12;
                    else
                        verbleibendeMonate = gesamtMonate - (b.AfaNr - 2) * 12
                                             - (13 - b.Datum.Month);
                    break;
            }

            // Extra-Jahr bei nicht-ganzjähriger Genauigkeit
            if (b.AfaNr > b.AfaJahre)
                return b.AfaRestwertCent;

            // Jahresrate aus Restwert und verbleibenden Monaten berechnen
            long jaehrlicheRate = RundenDiv10(
                10L * b.AfaRestwertCent * 12 / verbleibendeMonate);

            // Sicherheit: nie mehr als den Restwert abschreiben
            if (jaehrlicheRate > b.AfaRestwertCent)
                jaehrlicheRate = b.AfaRestwertCent;

            return GenauigkeitAnwenden(jaehrlicheRate, b, genauigkeit);
        }

        // ──────────────────────────────────────────────
        // Genauigkeitsanpassung
        // ──────────────────────────────────────────────

        /// <summary>
        /// Reimplementiert CBuchung::BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen.
        /// Passt die Jahresrate an die Genauigkeit an (Ganzjahr/Halbjahr/Monatsgenau).
        /// </summary>
        private static long GenauigkeitAnwenden(
            long jaehrlicheRate, Buchung b, AfaGenauigkeit genauigkeit)
        {
            if (b.AfaNr == 1)
            {
                // Erstes Jahr: anteilig je nach Kaufmonat
                switch (genauigkeit)
                {
                    case AfaGenauigkeit.Ganzjahr:
                        return jaehrlicheRate;
                    case AfaGenauigkeit.Halbjahr:
                        return b.Datum.Month < 7
                            ? jaehrlicheRate
                            : jaehrlicheRate / 2;
                    case AfaGenauigkeit.Monatsgenau:
                        return jaehrlicheRate * (13 - b.Datum.Month) / 12;
                }
            }
            else if (b.AfaNr <= b.AfaJahre)
            {
                // Jahre dazwischen: volle Rate
                return jaehrlicheRate;
            }
            else
            {
                // Letztes (Extra-)Jahr: verbleibende Monate
                switch (genauigkeit)
                {
                    case AfaGenauigkeit.Ganzjahr:
                        return 0L;
                    case AfaGenauigkeit.Halbjahr:
                        return b.Datum.Month < 7 ? 0L : jaehrlicheRate / 2;
                    case AfaGenauigkeit.Monatsgenau:
                        long verbleibend = 13 - b.Datum.Month;
                        return jaehrlicheRate * verbleibend / 12;
                }
            }

            return jaehrlicheRate;
        }

        // ──────────────────────────────────────────────
        // Rundung
        // ──────────────────────────────────────────────

        /// <summary>
        /// Reimplementiert CBuchung::RundenUndDurch10Dividieren.
        /// Kaufmännische Rundung eines mit 10 multiplizierten Cent-Betrags.
        /// </summary>
        private static long RundenDiv10(long betragMal10)
        {
            if (betragMal10 >= 0)
                betragMal10 += 5;
            else
                betragMal10 -= 5;
            return betragMal10 / 10;
        }
    }
}
