// EuerBericht.cs -- Formlose Einnahmen-Überschussrechnung ("Freestyle-EÜR").
//
// Portierung der Rechen- und Aufbaulogik von DrawEURechungToDC
// (easycashview.cpp:4231ff) auf die Engine: Posten-Summen je E/Ü-Konto in
// Slot-Reihenfolge, ustvst_gesondert-Schalter (Netto-Ausweis + synthetische
// UST/VST-Posten vs. Brutto-Ausweis), neutrale Konten ('/'-Präfix) nur mit
// MwSt-Anteil, AfA über GetBuchungsjahrNetto (VSt nur im Anschaffungsjahr),
// Unterkategorie-Zwischentitel, Summen + Gewinn/Verlust.
//
// Zeitraum kommt aus dem Ribbon-Monatsfilter (WYSIWYG statt Druckdialog),
// der Betrieb-Filter aus dem Ribbon-Betriebsfilter.
//
// Bewusste Abweichungen vom Original:
//   - "(einschl. Umsatzsteuer)" im Einnahmen-Titel erscheint jetzt
//     tatsächlich, wenn ohne USt/VSt-Ausweis gedruckt wird und MwSt im
//     Spiel ist (im Original war der Zweig durch einen Summenzähler-Bug
//     unerreichbar).
//   - Das "(nur UST)"-Suffix für neutrale Einnahmen-Konten stand im
//     Original versehentlich am AUSGABEN-Namen (Copy-Paste-Bug).
//   - Buchungen ohne Konto blockieren nicht mehr den Druck (alte
//     Modal-Schleife), sondern erscheinen als eigener Posten
//     "(ohne Konto)" plus Hinweiszeile.

using System;
using System.Globalization;

namespace ECTEngine
{
    public static class EuerBericht
    {
        private const int MaxPosten = 101;   // 100 Slots + 1 synthetischer UST/VST-Posten

        /// <summary>
        /// Erzeugt die formlose EÜR für das Buchungsjahr des Dokuments.
        /// monatsfilter: 0 = ganzes Jahr, 1-12 = Monat, 13-16 = Quartal.
        /// betriebFilter: leer = alle Betriebe.
        /// </summary>
        public static Bericht Erzeuge(BuchungsDocument doc,
            int monatsfilter, string betriebFilter)
        {
            if (doc == null) throw new ArgumentNullException(nameof(doc));
            betriebFilter = betriebFilter ?? "";
            if (betriebFilter == "<alle Betriebe>") betriebFilter = "";

            BerichtZeitraum.AusMonatsfilter(doc.Jahr, monatsfilter,
                out DateTime von, out DateTime bis);
            bool gesondert = GlobaleEinstellungen.UstVstGesondert;
            string waehrung = doc.Waehrung ?? "";

            var bericht = new Bericht { Waehrung = waehrung };

            // ----------------------------------------------------------
            // Titel + Adresskopf
            // ----------------------------------------------------------
            string name = (GlobaleEinstellungen.Land == 1)
                ? "Einnahmen-Ausgabenrechnung"
                : "Einnahmen-Überschussrechnung";
            if (betriebFilter != "")
                name += " '" + betriebFilter + "'";
            bericht.Titel = name + " "
                + BerichtZeitraum.Beschreibung(doc.Jahr, monatsfilter);

            BerichtAdresse.FuelleKopf(bericht);
            bericht.KopfRechts.Add("Steuernummer");
            bericht.KopfRechts.Add(ErmittleSteuernummer(betriebFilter));

            // ----------------------------------------------------------
            // Summierung (alles in Cent, wie das Original)
            // ----------------------------------------------------------
            var einNamen  = new string[MaxPosten];
            var einSummen = new long[MaxPosten];
            var ausNamen  = new string[MaxPosten];
            var ausSummen = new long[MaxPosten];
            long einGesamt = 0, einMwst = 0;
            long ausGesamt = 0, ausMwst = 0;
            int ohneKonto = 0;

            for (int i = 0; i < 100; i++)
            {
                einNamen[i] = Einstellungen.Hole(
                    "e" + i.ToString("D2", CultureInfo.InvariantCulture));
                ausNamen[i] = Einstellungen.Hole(
                    "a" + i.ToString("D2", CultureInfo.InvariantCulture));
            }
            einNamen[100] = ""; ausNamen[100] = "";

            foreach (var b in doc.Buchungen)
            {
                if (b.Datum < von || b.Datum > bis) continue;
                if (betriebFilter != "" && b.Betrieb != betriebFilter) continue;

                string konto = b.Konto;
                if (string.IsNullOrEmpty(konto))
                {
                    konto = "(ohne Konto)";
                    ohneKonto++;
                }

                long brutto = b.BruttoBetrag.InCent;
                bool neutral = konto.StartsWith("/");

                if (b.Art == Buchungsart.Einnahme)
                {
                    long netto = b.BruttoBetrag.NettoInCent;
                    long mwst  = brutto - netto;
                    einMwst += mwst;

                    int j = FindeSlot(einNamen, konto);
                    if (j < 0) { FehlerZuVieleKonten(bericht); return bericht; }

                    if (gesondert)
                    {
                        if (!neutral)
                        {
                            einSummen[j] += netto;
                            einGesamt    += brutto;
                        }
                        // MwSt landet gesammelt im synthetischen UST-Posten
                    }
                    else
                    {
                        if (!neutral)
                        {
                            einSummen[j] += brutto;
                            einGesamt    += brutto;
                        }
                        else
                        {
                            einSummen[j] += mwst;
                            einGesamt    += mwst;
                        }
                    }
                }
                else // Ausgabe
                {
                    // "VST-Beträge separat": bei gesondertem Ausweis einfach dem
                    // VST-Sammelposten zuschlagen (wie das Original)
                    if (konto == "VST-Beträge separat" && gesondert)
                    {
                        ausMwst   += brutto;
                        ausGesamt += brutto;
                        continue;
                    }

                    // AfA: nur der Jahres-Anteil zählt; Vorsteuer nur im
                    // Anschaffungsjahr (AfaNr == 1)
                    long netto = AfaCalculator.GetBuchungsjahrNetto(
                        b, doc.GlobaleAfaGenauigkeit);
                    long mwst = (b.AfaNr == 1)
                        ? brutto - b.BruttoBetrag.NettoInCent
                        : 0;

                    int j = FindeSlot(ausNamen, konto);
                    if (j < 0) { FehlerZuVieleKonten(bericht); return bericht; }

                    if (gesondert)
                    {
                        if (!neutral)
                        {
                            ausSummen[j] += netto;
                            ausGesamt    += netto + mwst;
                        }
                        ausMwst += mwst;
                    }
                    else
                    {
                        if (!neutral)
                        {
                            ausSummen[j] += netto + mwst;
                            ausGesamt    += netto + mwst;
                        }
                        else
                        {
                            ausSummen[j] += mwst;
                            ausGesamt    += mwst;
                        }
                    }
                }
            }

            // Synthetische Sammelposten bei gesondertem USt/VSt-Ausweis
            if (gesondert)
            {
                int j = FindeSlot(einNamen, "UST");
                if (j < 0) { FehlerZuVieleKonten(bericht); return bericht; }
                einSummen[j] = einMwst;

                j = FindeSlot(ausNamen, "VST");
                if (j < 0) { FehlerZuVieleKonten(bericht); return bericht; }
                ausSummen[j] = ausMwst;
            }

            // ----------------------------------------------------------
            // Zeilen aufbauen
            // ----------------------------------------------------------
            string einTitel = "1. Betriebseinnahmen";
            if (!gesondert && einMwst != 0)
                einTitel += " (einschl. Umsatzsteuer)";
            bericht.Add(einTitel, BerichtStil.Ueberschrift);
            SchreibePosten(bericht, einNamen, einSummen,
                "[EinnahmenUnterkategorien]", "1.", " (nur UST)", waehrung);
            bericht.Add("Summe", Geld(einGesamt, waehrung), BerichtStil.Summe)
                .LinieOben = true;
            bericht.Leerzeile();

            bericht.Add("2. Betriebsausgaben", BerichtStil.Ueberschrift);
            SchreibePosten(bericht, ausNamen, ausSummen,
                "[AusgabenUnterkategorien]", "2.", " (nur VST)", waehrung);
            bericht.Add("Summe", Geld(ausGesamt, waehrung), BerichtStil.Summe)
                .LinieOben = true;
            bericht.Leerzeile();

            var gewinn = bericht.Add("Gewinn/Verlust",
                Geld(einGesamt - ausGesamt, waehrung), BerichtStil.Summe);
            gewinn.LinieOben = true;
            gewinn.LinieUnten = true;

            if (ohneKonto > 0)
            {
                bericht.Leerzeile();
                bericht.Add(string.Format(CultureInfo.CurrentCulture,
                    "Hinweis: {0} Buchung(en) ohne Konto-Zuweisung sind unter "
                    + "'(ohne Konto)' zusammengefasst. Bitte weisen Sie diesen "
                    + "Buchungen ein Konto zu.", ohneKonto), BerichtStil.Hinweis);
            }

            return bericht;
        }

        // ----------------------------------------------------------
        // Helfer
        // ----------------------------------------------------------

        /// <summary>
        /// Findet den Slot des Kontos in der Namensliste; belegt den ersten
        /// leeren Slot, wenn das Konto noch nicht vorkommt (Konten, die in
        /// den Einstellungen fehlen, werden hinten angehängt -- wie das
        /// Original). -1 = mehr als 100 Konten.
        /// </summary>
        private static int FindeSlot(string[] namen, string konto)
        {
            for (int j = 0; j < namen.Length; j++)
            {
                if (string.IsNullOrEmpty(namen[j]))
                {
                    namen[j] = konto;
                    return j;
                }
                if (namen[j] == konto)
                    return j;
            }
            return -1;
        }

        private static void FehlerZuVieleKonten(Bericht bericht)
        {
            bericht.Add("--> FEHLER: Zu viele Konten in der E/Ü-Rechnung! "
                + "Bitte reduzieren Sie die Anzahl Ihrer Konten auf unter 100. <--",
                BerichtStil.Hinweis);
        }

        /// <summary>
        /// Schreibt die Posten-Zeilen einer Gruppe inkl. Unterkategorie-
        /// Zwischentiteln ("1.1 ...", "1.2 ..."). Die Unterkategorien sind
        /// slot-verankert (parallele ini-Sektion); sobald die erste erscheint,
        /// rücken die Posten eine Stufe weiter ein (wie das Original).
        /// </summary>
        private static void SchreibePosten(Bericht bericht,
            string[] namen, long[] summen,
            string unterkategorieSektion, string nummerPrefix,
            string neutralSuffix, string waehrung)
        {
            int einrueckung = 1;
            int unterNr = 1;
            for (int i = 0; i < namen.Length; i++)
            {
                if (i < 100)
                {
                    string unter = Einstellungen.Hole(unterkategorieSektion
                        + i.ToString("D2", CultureInfo.InvariantCulture));
                    if (!string.IsNullOrEmpty(unter))
                    {
                        einrueckung = 2;
                        bericht.Add(nummerPrefix + unterNr++ + " " + unter,
                            BerichtStil.Zwischentitel, 1);
                    }
                }

                if (!string.IsNullOrEmpty(namen[i]) && summen[i] != 0)
                {
                    string text = namen[i];
                    if (text.StartsWith("/")) text += neutralSuffix;
                    bericht.Add(text, Geld(summen[i], waehrung),
                        BerichtStil.Normal, einrueckung);
                }
            }
        }

        private static string ErmittleSteuernummer(string betriebFilter)
        {
            if (!string.IsNullOrEmpty(betriebFilter))
            {
                // Betriebs-eigene Steuernummer: [Betriebe]BetriebNNUnternehmensart
                // = "Art1<TAB>Rechtsform<TAB>Steuernummer<TAB>W-IdNr"
                for (int i = 0; i < 100; i++)
                {
                    string prefix = "[Betriebe]Betrieb"
                        + i.ToString("D2", CultureInfo.InvariantCulture);
                    string name = Einstellungen.Hole(prefix + "Name");
                    if (string.IsNullOrEmpty(name)) break;
                    if (name != betriebFilter) continue;

                    var teile = Einstellungen.Hole(prefix + "Unternehmensart")
                        .Split('\t');
                    if (teile.Length >= 3 && !string.IsNullOrEmpty(teile[2]))
                        return teile[2];
                    break;
                }
            }
            return GlobaleEinstellungen.FinanzamtSteuernummer;
        }

        internal static string Geld(long cents, string waehrung)
        {
            string s = Waehrungsformat.Betrag(cents / 100m);
            return string.IsNullOrEmpty(waehrung) ? s : s + " " + waehrung;
        }
    }

    /// <summary>
    /// Gemeinsamer Adresskopf der formlosen Berichte (Name, Unternehmensart,
    /// Straße, PLZ Ort aus [Persoenliche_Daten]).
    /// </summary>
    internal static class BerichtAdresse
    {
        public static void FuelleKopf(Bericht bericht)
        {
            AddWennGesetzt(bericht, (GlobaleEinstellungen.UnternehmerVorname
                + " " + GlobaleEinstellungen.UnternehmerName).Trim());
            AddWennGesetzt(bericht, GlobaleEinstellungen.Unternehmensart1);
            AddWennGesetzt(bericht, GlobaleEinstellungen.Unternehmensart2);
            AddWennGesetzt(bericht, GlobaleEinstellungen.UnternehmerStrasse);
            AddWennGesetzt(bericht, (GlobaleEinstellungen.UnternehmerPlz
                + " " + GlobaleEinstellungen.UnternehmerOrt).Trim());
        }

        private static void AddWennGesetzt(Bericht bericht, string zeile)
        {
            if (!string.IsNullOrWhiteSpace(zeile))
                bericht.KopfLinks.Add(zeile);
        }
    }
}
