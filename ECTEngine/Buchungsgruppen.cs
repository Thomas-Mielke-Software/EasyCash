// Buchungsgruppen.cs -- Instanziierung von Buchungsgruppen-Vorlagen
//
// Rechnet die Zusatz-Zeilen einer mehrzeiligen Preset-Vorlage gegen die
// Basis-Buchung aus (Formeln + Templates via FormelParser) und baut die
// fertigen, per Gruppen-UUID verknüpften Buchungen. Liegt in der Engine,
// damit die Logik ohne UI testbar ist -- das BuchungViewModel bindet nur.
//
// Ablauf beim Buchen einer Gruppe:
//   1. Dialog baut die Basis-Buchung aus den Feldern (Zeile 0).
//   2. Berechne() löst jede Zusatz-Zeile auf ($rest läuft mit).
//   3. BaueBuchungen() erzeugt Basis + Zusatz-Buchungen mit gemeinsamer
//      UUID (Erweiterungs-Keys, siehe Buchung.SetzeGruppe).

using System;
using System.Collections.Generic;

namespace ECTEngine
{
    /// <summary>Aufgelöste Werte einer Vorlagen-Zusatzzeile.</summary>
    public sealed class GruppenZeile
    {
        /// <summary>1-basierter Zeilenindex (= Gruppen-Rolle).</summary>
        public int Index { get; internal set; }
        public PresetZeile Vorlage { get; internal set; }

        public Buchungsart Art { get; internal set; }
        public string Konto { get; internal set; } = "";
        public string Beschreibung { get; internal set; } = "";
        public string Belegnummer { get; internal set; } = "";
        public string Betrieb { get; internal set; } = "";
        public string Bestandskonto { get; internal set; } = "";

        /// <summary>Brutto-Betrag der Zeile (berechnet oder manuell).</summary>
        public decimal Brutto { get; internal set; }
        /// <summary>MWSt im x1000-Format (19 % = 19000).</summary>
        public int MwstPromille { get; internal set; }

        /// <summary>True: Betrag ist im Dialog manuell einzugeben.</summary>
        public bool BetragManuell { get; internal set; }
        /// <summary>True: MWSt ist im Dialog wählbar (Vorlage lässt sie offen).</summary>
        public bool MwstManuell { get; internal set; }

        /// <summary>Formel-/Auswertungsfehler dieser Zeile ("" = ok).</summary>
        public string Fehler { get; internal set; } = "";

        /// <summary>Anzahl der Eingabefelder -- Grundlage der
        /// Darstellungs-Heuristik (kompakt vs. Maske).</summary>
        public int AnzahlEingabefelder =>
            (BetragManuell ? 1 : 0) + (MwstManuell ? 1 : 0);
    }

    /// <summary>Ergebnis von BuchungsgruppenRechner.Berechne.</summary>
    public sealed class GruppenBerechnung
    {
        public IReadOnlyList<GruppenZeile> Zeilen { get; internal set; }
            = Array.Empty<GruppenZeile>();

        /// <summary>Tatsächlich zu buchender Brutto-Betrag der Basis-Zeile
        /// (via BasisBetragFormel ggf. abweichend von der Eingabe).</summary>
        public decimal BasisGebuchtBrutto { get; internal set; }

        /// <summary>Eingabe-Brutto minus Summe aller Zusatz-Zeilen
        /// (Endstand von $rest).</summary>
        public decimal Rest { get; internal set; }

        /// <summary>Fehler der BasisBetragFormel ("" = ok).</summary>
        public string BasisFehler { get; internal set; } = "";

        public bool HatFehler
        {
            get
            {
                if (BasisFehler.Length > 0) return true;
                foreach (var z in Zeilen)
                    if (z.Fehler.Length > 0) return true;
                return false;
            }
        }
    }

    public static class BuchungsgruppenRechner
    {
        /// <summary>
        /// Löst alle Zusatz-Zeilen der Vorlage gegen die Basis-Buchung auf.
        /// </summary>
        /// <param name="vorlage">Mehrzeiliges Preset.</param>
        /// <param name="basis">Basis-Buchung mit dem EINGABE-Betrag
        /// (Zeile 0, wie im Dialog erfasst).</param>
        /// <param name="manuelleBruttos">Manuell eingegebene Brutto-Beträge
        /// pro Zusatz-Zeile (null = nicht eingegeben); nur für Zeilen mit
        /// leerer Betrag-Formel relevant.</param>
        /// <param name="manuelleMwstPromille">Manuell gewählte MWSt (x1000)
        /// pro Zusatz-Zeile; nur für Zeilen mit leerem MWSt-Ausdruck.</param>
        public static GruppenBerechnung Berechne(Preset vorlage, Buchung basis,
            IReadOnlyList<decimal?> manuelleBruttos = null,
            IReadOnlyList<int?> manuelleMwstPromille = null)
        {
            var ergebnis = new GruppenBerechnung();
            if (vorlage == null || basis == null) return ergebnis;

            decimal eingabeBrutto = basis.BruttoBetrag.BruttoWert;
            decimal rest = eingabeBrutto;
            var zeilen = new List<GruppenZeile>(vorlage.Zeilen.Count);

            for (int i = 0; i < vorlage.Zeilen.Count; i++)
            {
                var vz = vorlage.Zeilen[i];
                var zeile = new GruppenZeile { Index = i + 1, Vorlage = vz };

                // Buchungsart
                zeile.Art = vz.Art == "E" ? Buchungsart.Einnahme
                          : vz.Art == "A" ? Buchungsart.Ausgabe
                          : basis.Art;

                var zahlVars = FormelParser.ZahlVariablen(basis, rest, zeile.Index);
                var textVars = FormelParser.TextVariablen(basis, rest, zeile.Index);

                // Betrag: Formel oder manuell
                zeile.BetragManuell = vz.BetragManuell;
                if (vz.BetragManuell)
                {
                    zeile.Brutto = (manuelleBruttos != null && i < manuelleBruttos.Count
                        ? manuelleBruttos[i] : null) ?? 0m;
                }
                else
                {
                    var e = FormelParser.Berechne(vz.BetragFormel, zahlVars);
                    if (e.Ok) zeile.Brutto = decimal.Round(e.Wert, 2, MidpointRounding.AwayFromZero);
                    else zeile.Fehler = "Betrag: " + e.Fehler;
                }

                // MWSt: Ausdruck (Prozent-Formel bzw. x1000-Rohwert) oder manuell
                zeile.MwstManuell = string.IsNullOrEmpty(vz.MwstAusdruck);
                if (zeile.MwstManuell)
                {
                    zeile.MwstPromille = (manuelleMwstPromille != null && i < manuelleMwstPromille.Count
                        ? manuelleMwstPromille[i] : null) ?? basis.BruttoBetrag.MwstPromille;
                }
                else
                {
                    int? promille = WerteMwstAusdruckAus(vz.MwstAusdruck, zahlVars, out string mwstFehler);
                    if (promille.HasValue) zeile.MwstPromille = promille.Value;
                    else if (zeile.Fehler.Length == 0) zeile.Fehler = "MWSt: " + mwstFehler;
                }

                // Text-Felder: Template interpolieren + Datums-Platzhalter,
                // leeres Template = von Basis kopiert
                zeile.Beschreibung  = LoeseText(vz.TextTemplate, basis.Beschreibung, textVars, basis.Datum);
                zeile.Belegnummer   = LoeseText(vz.BelegTemplate, basis.Belegnummer, textVars, basis.Datum);
                zeile.Konto         = LoeseKonto(vz.Konto, textVars, basis.Datum, zeile);
                zeile.Betrieb       = LoeseText(vz.BetriebTemplate, basis.Betrieb, textVars, basis.Datum);
                zeile.Bestandskonto = LoeseText(vz.BestandskontoTemplate, basis.Bestandskonto, textVars, basis.Datum);

                rest -= zeile.Brutto;
                zeilen.Add(zeile);
            }

            ergebnis.Zeilen = zeilen;
            ergebnis.Rest = rest;

            // Basis-Betrag: Formel (z.B. "$rest" bei belegweiser Aufteilung)
            // oder unverändert die Eingabe
            if (string.IsNullOrEmpty(vorlage.BasisBetragFormel))
                ergebnis.BasisGebuchtBrutto = eingabeBrutto;
            else
            {
                var basisVars = FormelParser.ZahlVariablen(basis, rest, 0);
                var e = FormelParser.Berechne(vorlage.BasisBetragFormel, basisVars);
                if (e.Ok)
                    ergebnis.BasisGebuchtBrutto = decimal.Round(e.Wert, 2, MidpointRounding.AwayFromZero);
                else
                {
                    ergebnis.BasisGebuchtBrutto = eingabeBrutto;
                    ergebnis.BasisFehler = "Basis-Betrag: " + e.Fehler;
                }
            }

            return ergebnis;
        }

        /// <summary>
        /// Baut aus Basis + Berechnung die fertigen, per Gruppen-UUID
        /// verknüpften Buchungen (Basis zuerst, Rolle 0). Die Basis-Buchung
        /// wird dabei verändert (Betrag ggf. via BasisBetragFormel,
        /// Gruppen-Keys in der Erweiterung).
        /// </summary>
        /// <param name="vorhandeneUuid">Beim BEARBEITEN einer bestehenden
        /// Gruppe deren UUID (bleibt erhalten); null = neue Gruppe.</param>
        public static List<Buchung> BaueBuchungen(Preset vorlage, int vorlagenSlot,
            Buchung basis, GruppenBerechnung berechnung, string vorhandeneUuid = null)
        {
            var liste = new List<Buchung> { basis };
            if (vorlage == null || berechnung == null || berechnung.Zeilen.Count == 0)
                return liste;

            string uuid = string.IsNullOrEmpty(vorhandeneUuid)
                ? Guid.NewGuid().ToString() : vorhandeneUuid;

            // Basis: ggf. reduzierter Betrag (MWSt-Satz der Eingabe bleibt)
            basis.BruttoBetrag = Betrag.AusCent(
                (int)decimal.Round(berechnung.BasisGebuchtBrutto * 100m, 0, MidpointRounding.AwayFromZero),
                basis.BruttoBetrag.MwstPromille);
            basis.SetzeGruppe(uuid, 0, vorlagenSlot);

            foreach (var z in berechnung.Zeilen)
            {
                var b = new Buchung
                {
                    Art = z.Art,
                    Datum = basis.Datum,
                    BruttoBetrag = Betrag.AusCent(
                        (int)decimal.Round(z.Brutto * 100m, 0, MidpointRounding.AwayFromZero),
                        z.MwstPromille),
                    Beschreibung = z.Beschreibung,
                    Belegnummer = z.Belegnummer,
                    Konto = z.Konto,
                    Betrieb = z.Betrieb,
                    Bestandskonto = z.Bestandskonto,
                    // Zusatz-Zeilen sind nie AfA-Buchungen (wie die alten
                    // Split-Gegenbuchungen, buchendlg.cpp neutralisierte AfA)
                    AfaNr = 1,
                    AfaJahre = 1
                };
                b.SetzeGruppe(uuid, z.Index, vorlagenSlot);
                liste.Add(b);
            }
            return liste;
        }

        /// <summary>
        /// MWSt-Ausdruck einer Vorlagen-Zeile auswerten. Regel:
        /// reiner Integer &gt;= 1000 = x1000-Rohwert (Kompatibilität zum
        /// Preset-MWSt-Format, "19000" = 19 %); alles andere wird als
        /// PROZENT-Formel ausgewertet ("7", "10,7", "$vat2").
        /// </summary>
        public static int? WerteMwstAusdruckAus(string ausdruck,
            IReadOnlyDictionary<string, decimal> variablen, out string fehler)
        {
            fehler = "";
            var s = (ausdruck ?? "").Trim();
            if (s.Length == 0) return null;

            if (int.TryParse(s, System.Globalization.NumberStyles.None,
                    System.Globalization.CultureInfo.InvariantCulture, out int roh)
                && roh >= 1000)
                return roh;   // x1000-Rohwert

            var e = FormelParser.Berechne(s, variablen);
            if (!e.Ok) { fehler = e.Fehler; return null; }
            if (e.Wert < 0 || e.Wert > 100)
            {
                fehler = "MWSt-Satz muss zwischen 0 und 100 liegen.";
                return null;
            }
            return (int)decimal.Round(e.Wert * 1000m, 0, MidpointRounding.AwayFromZero);
        }

        /// <summary>Template interpolieren + Datums-Platzhalter der
        /// Dauerbuchungen auflösen; leeres Template = Basiswert kopieren.</summary>
        private static string LoeseText(string template, string basisWert,
            IReadOnlyDictionary<string, string> textVars, DateTime datum)
        {
            if (string.IsNullOrEmpty(template)) return basisWert ?? "";
            var interpoliert = FormelParser.Interpoliere(template, textVars);
            return Dauerbuchung.ResolvePlatzhalter(interpoliert, datum);
        }

        /// <summary>
        /// Konto-Feld einer Zusatz-Zeile auflösen: Feld-Spezifikationen
        /// ("$de:Formular=Id|...||") werden über den KontoFeldSelektor zum
        /// erstbesten verknüpften Konto aufgelöst (rein lesend -- die Anlage
        /// eines fehlenden Kontos macht die UI beim Laden der Vorlage);
        /// alles andere ist ein normales Text-Template.
        /// </summary>
        private static string LoeseKonto(string kontoFeld,
            IReadOnlyDictionary<string, string> textVars, DateTime datum,
            GruppenZeile zeile)
        {
            var aufloesung = KontoFeldSelektor.LoeseAuf(kontoFeld);
            if (!aufloesung.IstSpezifikation)
                return LoeseText(kontoFeld, "", textVars, datum);

            if (aufloesung.Fehler.Length > 0)
            {
                if (zeile.Fehler.Length == 0)
                    zeile.Fehler = "Konto: " + aufloesung.Fehler;
                return "";
            }
            if (aufloesung.Konto == null)
            {
                if (zeile.Fehler.Length == 0)
                    zeile.Fehler = "Konto: Es gibt noch kein Konto mit den "
                        + "benötigten Feld-Verknüpfungen.";
                return "";
            }
            return aufloesung.Konto;
        }
    }
}
