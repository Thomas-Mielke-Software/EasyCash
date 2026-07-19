// UstErklaerungBericht.cs -- Formlose USt-Erklärung.
//
// Portierung der Rechen- und Aufbaulogik von DrawUmStErklaerungToDC
// (easycashview.cpp:3917ff) auf die Engine:
//   C. Lieferungen und sonstige Leistungen: Netto/USt je MwSt-Satz
//      (nur Einnahmen, neutrale '/'-Konten ausgenommen), Sätze absteigend
//   D. Abziehbare Vorsteuerbeträge: VSt aus Ausgaben (AfA: nur im
//      Anschaffungsjahr; Sonderkonto "VST-Beträge separat" brutto),
//      entrichtete Einfuhrumsatzsteuer (Konto-Suffix)
//   F. Verbleibender Betrag + gemerkte Vorauszahlungen (Dokument-
//      ErweiterungStore "Elster"/"UST-Zahlbetrag-JJJJ-NN")
//   Anlage UR: UST-ID-Umsätze, steuerfreie Umsätze (Konto-Namen)
//
// Zeitraum kommt aus dem Ribbon-Monatsfilter (0 = Jahr, 1-12 = Monat,
// 13-16 = Quartal). Beträge wie im Original ohne Währungs-Suffix,
// dafür die "Währung:"-Zeile im Kopf.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;

namespace ECTEngine
{
    public static class UstErklaerungBericht
    {
        public static Bericht Erzeuge(BuchungsDocument doc, int monatsfilter)
        {
            if (doc == null) throw new ArgumentNullException(nameof(doc));

            BerichtZeitraum.AusMonatsfilter(doc.Jahr, monatsfilter,
                out DateTime von, out DateTime bis);

            var bericht = new Bericht { Waehrung = doc.Waehrung ?? "" };
            bericht.Titel = BerichtZeitraum.IstGanzesJahr(monatsfilter)
                ? "USt-Erklärung für " + doc.Jahr
                : "Vorläufige USt-Erklärung "
                    + BerichtZeitraum.Beschreibung(doc.Jahr, monatsfilter);
            BerichtAdresse.FuelleKopf(bericht);

            // ----------------------------------------------------------
            // Einnahmen: Netto/USt je Satz + Sonderfelder (alles in Cent)
            // ----------------------------------------------------------
            var nettoJeSatz = new Dictionary<int, long>();   // Promille -> Cent
            var ustJeSatz   = new Dictionary<int, long>();
            long nettoGesamt = 0, ustGesamt = 0, bruttoGesamt = 0;
            long steuerfrei = 0, weitereSteuerfrei = 0, ustId = 0;

            foreach (var b in doc.Buchungen)
            {
                if (b.Art != Buchungsart.Einnahme) continue;
                if (b.Datum < von || b.Datum > bis) continue;
                if (b.IstNeutralesKonto) continue;

                long brutto = b.BruttoBetrag.InCent;
                long netto  = b.BruttoBetrag.NettoInCent;
                long mwst   = brutto - netto;
                int satz    = b.BruttoBetrag.MwstPromille;

                if (satz >= 0)
                {
                    if (!nettoJeSatz.ContainsKey(satz))
                    {
                        nettoJeSatz[satz] = 0;
                        ustJeSatz[satz] = 0;
                    }
                    nettoJeSatz[satz] += netto;
                    ustJeSatz[satz]   += mwst;
                    nettoGesamt  += netto;
                    ustGesamt    += mwst;
                    bruttoGesamt += brutto;
                }

                string konto = b.Konto ?? "";
                if (konto.EndsWith("Steuerfreie Umsätze", StringComparison.Ordinal))
                    steuerfrei += brutto;
                if (konto.EndsWith("Weitere Steuerfreie Umsätze mit Vorsteuerabzug",
                        StringComparison.Ordinal))
                    weitereSteuerfrei += brutto;
                if (konto.IndexOf("UST-ID", StringComparison.Ordinal) >= 0)
                    ustId += brutto;
            }

            // ----------------------------------------------------------
            // Ausgaben: abziehbare Vorsteuer + Einfuhrumsatzsteuer
            // ----------------------------------------------------------
            long vst = 0, einfuhrUst = 0;
            foreach (var b in doc.Buchungen)
            {
                if (b.Art != Buchungsart.Ausgabe) continue;
                if (b.Datum < von || b.Datum > bis) continue;
                if (b.IstNeutralesKonto) continue;

                if (b.Konto == "VST-Beträge separat")
                {
                    vst += b.BruttoBetrag.InCent;
                    continue;
                }

                // Vorsteuer nur im Anschaffungsjahr (AfA-Folgejahre: 0)
                if (b.AfaNr == 1)
                    vst += b.BruttoBetrag.InCent - b.BruttoBetrag.NettoInCent;

                if ((b.Konto ?? "").EndsWith("Einfuhrumsatzsteuer",
                        StringComparison.Ordinal))
                    einfuhrUst += b.BruttoBetrag.InCent;
            }

            // ----------------------------------------------------------
            // Zeilen aufbauen
            // ----------------------------------------------------------
            bericht.Add("Währung: " + bericht.Waehrung);
            bericht.Leerzeile();

            bericht.Add("C. Lieferungen und sonstige Leistungen:",
                BerichtStil.Ueberschrift);
            var tabellenkopf = bericht.Add("USt-Satz",
                new[] { "Netto-Betrag", "USt-Betrag", "Brutto-Betrag" },
                BerichtStil.Zwischentitel, 1);
            tabellenkopf.LinieUnten = true;

            foreach (var satz in nettoJeSatz.Keys.OrderByDescending(s => s))
            {
                if (nettoJeSatz[satz] == 0) continue;
                bericht.Add(SatzText(satz), new[]
                {
                    Betrag(nettoJeSatz[satz]),
                    Betrag(ustJeSatz[satz]),
                    Betrag(nettoJeSatz[satz] + ustJeSatz[satz])
                }, BerichtStil.Normal, 1);
            }
            var gesamt = bericht.Add("Gesamt:", new[]
            {
                Betrag(nettoGesamt), Betrag(ustGesamt), Betrag(bruttoGesamt)
            }, BerichtStil.Summe, 1);
            gesamt.LinieOben = true;
            bericht.Leerzeile();

            bericht.Add("D. Abziehbare Vorsteuerbeträge:", BerichtStil.Ueberschrift);
            bericht.Add("aus Rechnungen von anderen Unternehmen:",
                Betrag(vst), BerichtStil.Normal, 1);
            bericht.Add("entrichtete Einfuhrumsatzsteuer:",
                Betrag(einfuhrUst), BerichtStil.Normal, 1);
            bericht.Leerzeile();

            bericht.Add("F. Berechnung der zu entrichtenden Umsatzsteuer:",
                BerichtStil.Ueberschrift);
            bericht.Add("Verbleibender Betrag:",
                Betrag(ustGesamt - vst - einfuhrUst), BerichtStil.Normal, 1);
            bericht.Add("Vorauszahlungssoll für das Jahr:", "- _______",
                BerichtStil.Normal, 1);
            bericht.Add("(alle Vorauszahlungen für dieses Jahr)",
                BerichtStil.Hinweis, 2);
            bericht.Leerzeile();

            SchreibeVorauszahlungen(bericht, doc, von, bis);

            bericht.Add("Noch zu entrichten/Erstattungsanspruch:", "= _______",
                BerichtStil.Normal, 1);

            // ----------------------------------------------------------
            // Anlage UR (Werte in vollen Euro, wie das Original)
            // ----------------------------------------------------------
            if (ustId / 100 != 0 || steuerfrei / 100 != 0
                || weitereSteuerfrei / 100 != 0)
            {
                bericht.Leerzeile();
                bericht.Add("Anlage UR:", BerichtStil.Ueberschrift);
                bericht.Add("Innergemeinschaftliche Lieferungen an Abnehmer mit USt-ID:",
                    VolleEuro(ustId), BerichtStil.Normal, 1);
                bericht.Add("Weitere Steuerfreie Umsätze mit Vorsteuerabzug:",
                    VolleEuro(weitereSteuerfrei), BerichtStil.Normal, 1);
                bericht.Add("Steuerfreie Umsätze ohne Vorsteuerabzug:",
                    VolleEuro(steuerfrei), BerichtStil.Normal, 1);
            }

            return bericht;
        }

        // ----------------------------------------------------------
        // Gemerkte Vorauszahlungen aus dem Dokument-ErweiterungStore
        // ("Elster" / "UST-Zahlbetrag-JJJJ-NN"; NN = 1-12 Monat,
        // 41-44 Quartal). Werte sind Währungs-Strings.
        // ----------------------------------------------------------
        private static void SchreibeVorauszahlungen(
            Bericht bericht, BuchungsDocument doc, DateTime von, DateTime bis)
        {
            bool ueberschrift = false;
            long summe = 0;

            for (int monat = von.Month; monat <= bis.Month; monat++)
            {
                long? mBetrag = HoleVorauszahlung(doc, monat);
                long? qBetrag = ((monat - 1) % 3 == 0)
                    ? HoleVorauszahlung(doc, (monat - 1) / 3 + 41)
                    : null;

                if (mBetrag == null && qBetrag == null) continue;

                if (!ueberschrift)
                {
                    ueberschrift = true;
                    bericht.Add("Gemerkte Vorauszahlungen "
                        + "(bzw. Erstattungen bei Minusbeträgen):",
                        BerichtStil.Zwischentitel, 1);
                }

                if (mBetrag != null)
                {
                    summe += mBetrag.Value;
                    bericht.Add(string.Format(CultureInfo.CurrentCulture,
                            "{0} für Monat {1}:",
                            mBetrag.Value >= 0 ? "Vorauszahlung" : "Erstattung",
                            monat),
                        Betrag(mBetrag.Value), BerichtStil.Normal, 2);
                }
                if (qBetrag != null)
                {
                    summe += qBetrag.Value;
                    bericht.Add(string.Format(CultureInfo.CurrentCulture,
                            "{0} für Quartal {1}:",
                            qBetrag.Value >= 0 ? "Vorauszahlung" : "Erstattung",
                            (monat - 1) / 3 + 1),
                        Betrag(qBetrag.Value), BerichtStil.Normal, 2);
                }
            }

            if (ueberschrift)
            {
                var summenZeile = bericht.Add(
                    "Summe aller gemerkten Vorauszahlungen:",
                    Betrag(summe), BerichtStil.Summe, 2);
                summenZeile.LinieOben = true;
                bericht.Leerzeile();
            }
        }

        /// <summary>Vorauszahlung für den Zeitraum (1-12 Monat, 41-44 Quartal)
        /// in Cent, oder null wenn nicht gemerkt.</summary>
        private static long? HoleVorauszahlung(BuchungsDocument doc, int zeitraum)
        {
            string key = string.Format(CultureInfo.InvariantCulture,
                "UST-Zahlbetrag-{0:D4}-{1:D2}", doc.Jahr, zeitraum);
            string wert = doc.Erweiterungen.Hole("Elster", key, "");
            if (string.IsNullOrEmpty(wert)) return null;
            if (!Waehrungsformat.TryParse(wert, out decimal betrag)) return null;
            return (long)decimal.Round(betrag * 100m, 0,
                MidpointRounding.AwayFromZero);
        }

        // ----------------------------------------------------------
        // Formatierung
        // ----------------------------------------------------------

        /// <summary>"19%" bzw. "7,5%" (Promille-Skala: 19000 = 19 %).</summary>
        private static string SatzText(int promille)
        {
            if (promille % 1000 != 0)
                return (promille / 1000m).ToString("0.###",
                    CultureInfo.CurrentCulture) + "%";
            return (promille / 1000).ToString(CultureInfo.InvariantCulture) + "%";
        }

        private static string Betrag(long cents) =>
            Waehrungsformat.Betrag(cents / 100m);

        private static string VolleEuro(long cents) =>
            (cents / 100).ToString(CultureInfo.InvariantCulture);
    }
}
