// Buchungsgruppen.cs -- Instanziierung von Buchungsvorlagen
//
// Rechnet eine Vorlage gegen die Eingaben des Buchen-Dialogs aus (Formeln
// + Templates via FormelParser) und baut die fertigen Buchungen: die
// Basis-Buchung und -- bei einer Buchungsgruppen-Vorlage -- die per
// Gruppen-UUID verknüpften Zusatz-Buchungen. Liegt in der Engine, damit
// die Logik ohne UI testbar ist -- das BuchungViewModel bindet nur.
//
// Eingaben sind die Basis-Buchung (Zeile 0, die Original-Felder des
// Dialogs) und die Werte der ZUSATZ-FELDER der Vorlage (VorlagenFelder.cs).
// Beides steht den Formeln und Templates als Variablen zur Verfügung.
//
// Ablauf beim Buchen:
//   1. Dialog baut die Basis-Buchung aus den Feldern (Zeile 0). Ist das
//      Betragsfeld gesperrt/ausgeblendet, kommt ihr Betrag vorab aus
//      BerechneEingabeBetrag().
//   2. Berechne() löst jede Zusatz-Zeile auf ($rest läuft mit), danach
//      Betrag, Beschreibung und Belegnummer der Basis.
//   3. BaueBuchungen() erzeugt Basis + Zusatz-Buchungen mit gemeinsamer
//      UUID (Erweiterungs-Keys, siehe Buchung.SetzeGruppe) und merkt die
//      Eingaben in der Basis (Buchung.SetzeVorlagenBezug), damit sich die
//      Buchung später wieder mit der Vorlage bearbeiten lässt.
//
// Zeilen ohne Betrag werden nicht gebucht: eine Zeile, deren Formel 0
// ergibt (leer gelassenes Skonto, nicht gesetztes Häkchen), fällt weg --
// ebenso eine Zeile ganz ohne Betrags-Formel (Vorlagen aus der Zeit der
// manuellen Zeilen-Eingabe; die übernehmen heute die Zusatz-Felder).

using System;
using System.Collections.Generic;
using System.Linq;

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

        /// <summary>Brutto-Betrag der Zeile.</summary>
        public decimal Brutto { get; internal set; }
        /// <summary>MWSt im x1000-Format (19 % = 19000).</summary>
        public int MwstPromille { get; internal set; }

        /// <summary>True: die Zeile wird NICHT gebucht (Betrag 0 oder keine
        /// Betrags-Formel). Der Grund steht in <see cref="Hinweis"/>.</summary>
        public bool Uebersprungen { get; internal set; }
        public string Hinweis { get; internal set; } = "";

        /// <summary>Formel-/Auswertungsfehler dieser Zeile ("" = ok).</summary>
        public string Fehler { get; internal set; } = "";
    }

    /// <summary>Ergebnis von BuchungsgruppenRechner.Berechne.</summary>
    public sealed class GruppenBerechnung
    {
        /// <summary>Alle Zeilen der Vorlage, auch die übersprungenen.</summary>
        public IReadOnlyList<GruppenZeile> Zeilen { get; internal set; }
            = Array.Empty<GruppenZeile>();

        /// <summary>Die Zeilen, die tatsächlich gebucht werden.</summary>
        public IEnumerable<GruppenZeile> GebuchteZeilen =>
            Zeilen.Where(z => !z.Uebersprungen);

        /// <summary>Tatsächlich zu buchender Brutto-Betrag der Basis-Zeile
        /// (via BasisBetragFormel ggf. abweichend von der Eingabe).</summary>
        public decimal BasisGebuchtBrutto { get; internal set; }

        /// <summary>Beschreibung der Basis-Buchung (BasisTextTemplate
        /// aufgelöst; ohne Template der Inhalt des Beschreibungsfelds).</summary>
        public string BasisBeschreibung { get; internal set; } = "";

        /// <summary>Belegnummer der Basis-Buchung (BasisBelegTemplate
        /// aufgelöst; ohne Template die eingegebene).</summary>
        public string BasisBelegnummer { get; internal set; } = "";

        /// <summary>Eingabe-Brutto minus Summe aller Zusatz-Zeilen
        /// (Endstand von $rest).</summary>
        public decimal Rest { get; internal set; }

        /// <summary>Fehler der Basis-Buchung ("" = ok): Formelfehler oder
        /// ein Basis-Betrag, den die Zusatz-Zeilen aufgezehrt haben.</summary>
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
        /// Eingabe-Betrag einer Vorlage, deren Betragsfeld gesperrt oder
        /// ausgeblendet ist (VorlagenErweiterung.EingabeBetragFormel).
        /// Rechnet nur mit den Zusatz-Feldern, Konstanten und $vat1..$vat4.
        /// </summary>
        /// <returns>null, wenn die Vorlage keine solche Formel hat.</returns>
        public static FormelErgebnis BerechneEingabeBetrag(Preset vorlage,
            IEnumerable<FeldWert> felder)
        {
            string formel = vorlage?.Erweiterung.EingabeBetragFormel;
            if (string.IsNullOrEmpty(formel)) return null;
            var e = FormelParser.Berechne(formel, FormelParser.FeldVariablen(felder));
            return e.Ok
                ? FormelErgebnis.Erfolg(decimal.Round(e.Wert, 2, MidpointRounding.AwayFromZero))
                : e;
        }

        /// <summary>
        /// Löst die Vorlage gegen die Basis-Buchung und die Feldwerte auf.
        /// </summary>
        /// <param name="vorlage">Die Buchungsvorlage (mit oder ohne
        /// Zusatz-Zeilen).</param>
        /// <param name="basis">Basis-Buchung mit dem EINGABE-Betrag
        /// (Zeile 0, wie im Dialog erfasst).</param>
        /// <param name="felder">Werte der Zusatz-Felder (null = keine).</param>
        public static GruppenBerechnung Berechne(Preset vorlage, Buchung basis,
            IReadOnlyList<FeldWert> felder = null)
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

                var zahlVars = FormelParser.ZahlVariablen(basis, rest, zeile.Index, felder);
                var textVars = FormelParser.TextVariablen(basis, rest, zeile.Index, felder);

                // Betrag: Formel. Ohne Formel gibt es nichts zu buchen.
                if (string.IsNullOrWhiteSpace(vz.BetragFormel))
                {
                    zeile.Uebersprungen = true;
                    zeile.Hinweis = "ohne Betrags-Formel -- wird nicht gebucht";
                }
                else
                {
                    var e = FormelParser.Berechne(vz.BetragFormel, zahlVars);
                    if (e.Ok)
                    {
                        zeile.Brutto = decimal.Round(e.Wert, 2, MidpointRounding.AwayFromZero);
                        if (zeile.Brutto == 0m)
                        {
                            zeile.Uebersprungen = true;
                            zeile.Hinweis = "Betrag 0 -- wird nicht gebucht";
                        }
                    }
                    else zeile.Fehler = "Betrag: " + e.Fehler;
                }

                // MWSt: Ausdruck (Prozent-Formel bzw. x1000-Rohwert);
                // leer = Satz der Basis
                if (string.IsNullOrWhiteSpace(vz.MwstAusdruck))
                    zeile.MwstPromille = basis.BruttoBetrag.MwstPromille;
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

                if (zeile.Uebersprungen)
                {
                    // Was nicht gebucht wird, kann das Buchen auch nicht
                    // blockieren (z.B. ein noch nicht angelegtes Konto einer
                    // Zeile, deren Betrag 0 ist).
                    zeile.Fehler = "";
                }
                else if (zeile.Fehler.Length == 0 && string.IsNullOrWhiteSpace(zeile.Konto))
                {
                    // Ein Konto aus einem (noch) leeren Zusatz-Feld
                    zeile.Fehler = "Konto: kein Konto angegeben.";
                }

                rest -= zeile.Brutto;
                zeilen.Add(zeile);
            }

            ergebnis.Zeilen = zeilen;
            ergebnis.Rest = rest;

            // Basis-Betrag: Formel (z.B. "$rest" bei belegweiser Aufteilung)
            // oder unverändert die Eingabe
            if (string.IsNullOrWhiteSpace(vorlage.BasisBetragFormel))
                ergebnis.BasisGebuchtBrutto = eingabeBrutto;
            else
            {
                var basisVars = FormelParser.ZahlVariablen(basis, rest, 0, felder);
                var e = FormelParser.Berechne(vorlage.BasisBetragFormel, basisVars);
                if (e.Ok)
                {
                    ergebnis.BasisGebuchtBrutto = decimal.Round(e.Wert, 2, MidpointRounding.AwayFromZero);
                    ergebnis.BasisFehler = PruefeBasisBetrag(eingabeBrutto, ergebnis.BasisGebuchtBrutto);
                }
                else
                {
                    ergebnis.BasisGebuchtBrutto = eingabeBrutto;
                    ergebnis.BasisFehler = "Basis-Betrag: " + e.Fehler;
                }
            }

            // Beschreibung und Belegnummer der Basis (Templates der Vorlage)
            var basisText = FormelParser.TextVariablen(basis, rest, 0, felder);
            ergebnis.BasisBeschreibung = LoeseText(vorlage.Erweiterung.BasisTextTemplate,
                basis.Beschreibung, basisText, basis.Datum);
            ergebnis.BasisBelegnummer = LoeseText(vorlage.Erweiterung.BasisBelegTemplate,
                basis.Belegnummer, basisText, basis.Datum);

            return ergebnis;
        }

        /// <summary>
        /// Wacht darüber, dass die Formel der Basis-Buchung etwas Buchbares
        /// übrig lässt. Typischer Fall ist die Aufteilung ("Basis bucht
        /// $rest"): übersteigen die Zusatz-Beträge die Eingabe, kippte die
        /// Basis ins andere Vorzeichen (aus der Ausgabe würde faktisch eine
        /// Einnahme) -- bleibt genau 0, entstünde eine leere Buchung.
        /// Eine Eingabe von 0 wird nicht hier, sondern vom Dialog bemängelt.
        /// </summary>
        private static string PruefeBasisBetrag(decimal eingabe, decimal gebucht)
        {
            if (eingabe == 0m) return "";
            if (gebucht == 0m)
                return "Für die Basis-Buchung bleibt kein Betrag übrig (0,00) -- "
                     + "bitte die Beträge prüfen.";
            if ((eingabe > 0m) != (gebucht > 0m))
                return "Die Zusatz-Beträge übersteigen den eingegebenen Betrag: für "
                     + "die Basis-Buchung blieben "
                     + Waehrungsformat.Betrag(gebucht) + ".";
            return "";
        }

        /// <summary>
        /// Baut aus Basis + Berechnung die fertigen Buchungen (Basis zuerst,
        /// Rolle 0). Die Basis-Buchung wird dabei verändert: Betrag,
        /// Beschreibung und Belegnummer laut Berechnung, Gruppen-Keys und
        /// Vorlagen-Bezug in der Erweiterung. Bleibt keine Zusatz-Zeile
        /// übrig (alle übersprungen bzw. Vorlage ohne Zeilen), entsteht eine
        /// gewöhnliche Einzelbuchung OHNE Gruppen-Keys.
        /// </summary>
        /// <param name="vorhandeneUuid">Beim BEARBEITEN einer bestehenden
        /// Gruppe deren UUID (bleibt erhalten); null = neue Gruppe.</param>
        /// <param name="felder">Werte der Zusatz-Felder -- werden in der
        /// Basis gemerkt.</param>
        public static List<Buchung> BaueBuchungen(Preset vorlage, int vorlagenSlot,
            Buchung basis, GruppenBerechnung berechnung, string vorhandeneUuid = null,
            IReadOnlyList<FeldWert> felder = null)
        {
            var liste = new List<Buchung> { basis };
            if (vorlage == null || berechnung == null) return liste;

            // Eingaben festhalten, bevor die Basis verändert wird
            decimal eingabeBrutto = basis.BruttoBetrag.BruttoWert;
            string eingabeText = basis.Beschreibung ?? "";
            string eingabeBeleg = basis.Belegnummer ?? "";

            // Basis: ggf. reduzierter Betrag (MWSt-Satz der Eingabe bleibt),
            // Beschreibung/Beleg aus den Templates
            basis.BruttoBetrag = Betrag.AusCent(
                (int)decimal.Round(berechnung.BasisGebuchtBrutto * 100m, 0, MidpointRounding.AwayFromZero),
                basis.BruttoBetrag.MwstPromille);
            basis.Beschreibung = berechnung.BasisBeschreibung;
            basis.Belegnummer = berechnung.BasisBelegnummer;

            var gebucht = berechnung.GebuchteZeilen.ToList();

            // Reihenfolge: EntferneGruppe räumt auch einen alten
            // Vorlagen-Bezug ab, darum zuerst.
            string uuid = null;
            if (gebucht.Count > 0)
            {
                uuid = string.IsNullOrEmpty(vorhandeneUuid)
                    ? Guid.NewGuid().ToString() : vorhandeneUuid;
                basis.SetzeGruppe(uuid, 0, vorlagenSlot);
            }
            else
                basis.EntferneGruppe();

            basis.SetzeVorlagenBezug(vorlagenSlot, vorlage.Text,
                string.IsNullOrWhiteSpace(vorlage.BasisBetragFormel)
                    ? (decimal?)null : eingabeBrutto,
                string.IsNullOrEmpty(vorlage.Erweiterung.BasisTextTemplate)
                    ? null : eingabeText,
                string.IsNullOrEmpty(vorlage.Erweiterung.BasisBelegTemplate)
                    ? null : eingabeBeleg,
                felder, vorlage.Id);

            foreach (var z in gebucht)
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
