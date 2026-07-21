// FormularRechner.cs -- Feldwert-Berechnung fuer .ecf-Formulare.
//
// Exakter Port von CEasyCashDoc::GetFormularwertByIndex /
// BerechneFormularfeldwerte (ECTBridge/easycashdoc.cpp:2259-3039, die
// ECTBridge-Fassung ist massgeblich; die ECTIFace-Kopie ist ein Duplikat).
// Die native Funktion bleibt unangetastet -- sie versorgt weiterhin das
// ELSTER-Plugin (EasyCTX/FormularCtl.cpp). Divergenzen zwischen beiden
// Welten deckt der Debug-Vergleichsdump der Bridge auf
// (ECT_FormularVergleichsdump).
//
// ACHTUNG: Interne Arithmetik bewusst in GANZEN CENT (long) mit
// C-Truncation-Semantik (summe/100, summe/100*100, (int)(wert*faktor/divisor))
// -- NICHT auf decimal "verschoenern", sonst weichen Rundungen vom nativen
// Formular und damit von ELSTER ab.
//
// Bewusste Doppel-Logik: UstErklaerungBericht.cs (Formlos-Ansicht) rechnet
// die USt-Kennzahlen eigenstaendig decimal-basiert. Nicht vereinheitlichen
// ohne Vergleichstests -- siehe Plan "Migration der Formular-Modi".
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2026 Thomas Mielke

using System;
using System.Collections.Generic;
using System.Globalization;

namespace ECTEngine
{
    /// <summary>Berechneter Wert eines Formularfelds.</summary>
    public sealed class FormularFeldWert
    {
        public FormularFeldDef Feld { get; internal set; }
        /// <summary>Anzeigetext (formatiert wie nativ; leer bei
        /// Nullwert-Unterdrueckung oder leerem Feld).</summary>
        public string Text { get; internal set; } = "";
        /// <summary>Cache-Wert in Cent (Pendant m_laFeldWerte[id]).</summary>
        public long WertCent { get; internal set; }
        /// <summary>Statuszeilen-/Tooltip-Text (Pendant m_csaFeldStatustext).</summary>
        public string Statustext { get; internal set; } = "";
    }

    /// <summary>
    /// Rechnet alle Feldwerte eines Formulars fuer ein Dokument aus.
    /// Zustand (Feldwert-Cache, Gueltig-Flags, Kontenlisten) lebt pro
    /// Berechne()-Lauf -- die Klasse selbst ist zustandslos nutzbar.
    /// </summary>
    public sealed class FormularRechner
    {
        private const int MaxFeldId = 10000;   // IDs 0-9999, wie nativ
        private const long Fehlerwert = -99999999;

        private readonly BuchungsDocument _doc;
        private readonly FormularDefinition _def;
        private readonly string _filter;       // Betriebsname oder ""

        // Pendant ReadKontenCache (easycashdoc.cpp:2125)
        private readonly List<string> _einnahmenKonten = new List<string>();
        private readonly List<string> _einnahmenFeldzuweisungen = new List<string>();
        private readonly List<string> _ausgabenKonten = new List<string>();
        private readonly List<string> _ausgabenFeldzuweisungen = new List<string>();

        private readonly long[] _werte = new long[MaxFeldId];
        private readonly bool[] _gueltig = new bool[MaxFeldId];
        private readonly string[] _statustext = new string[MaxFeldId];

        private DateTime _von, _bis;

        private FormularRechner(BuchungsDocument doc, FormularDefinition def, string betriebFilter)
        {
            _doc = doc;
            _def = def;
            _filter = betriebFilter ?? "";
        }

        /// <summary>
        /// Berechnet alle Felder in Dateireihenfolge (Pendant
        /// BerechneFormularfeldwerte). betriebFilter = Betriebsname aus
        /// m_csaFormularfilter (leer = kein Betrieb gewaehlt).
        /// </summary>
        public static IReadOnlyList<FormularFeldWert> Berechne(
            BuchungsDocument doc, FormularDefinition def, string betriebFilter)
        {
            if (doc == null) throw new ArgumentNullException(nameof(doc));
            if (def == null) throw new ArgumentNullException(nameof(def));

            var rechner = new FormularRechner(doc, def, betriebFilter);
            rechner.LeseKontenCache();
            rechner.BerechneZeitraum();

            var ergebnis = new List<FormularFeldWert>(def.Felder.Count);
            foreach (var feld in def.Felder)
            {
                var wert = new FormularFeldWert { Feld = feld };
                if (feld.HatInhalt)
                {
                    wert.Text = rechner.BerechneFeld(feld);
                    if (feld.Id >= 0 && feld.Id < MaxFeldId)
                    {
                        wert.WertCent = rechner._werte[feld.Id];
                        wert.Statustext = rechner._statustext[feld.Id] ?? "";
                    }
                }
                ergebnis.Add(wert);
            }
            return ergebnis;
        }

        // -----------------------------------------------------------------
        // Konten-Cache (Pendant ReadKontenCache): Kontennamen +
        // Feldzuweisungs-Blobs aus dem Einstellungs-Cache. Die nativen
        // Arrays sind fix 100 Eintraege mit Stopp beim ersten Leerstring --
        // hier Listen, die Schleifen stoppen identisch.
        // -----------------------------------------------------------------
        private void LeseKontenCache()
        {
            for (int i = 0; i < 100; i++)
            {
                string nn = i.ToString("D2", CultureInfo.InvariantCulture);
                _einnahmenKonten.Add(Einstellungen.Hole("e" + nn));
                _einnahmenFeldzuweisungen.Add(Einstellungen.Hole("[EinnahmenFeldzuweisungen]" + nn));
                _ausgabenKonten.Add(Einstellungen.Hole("a" + nn));
                _ausgabenFeldzuweisungen.Add(Einstellungen.Hole("[AusgabenFeldzuweisungen]" + nn));
            }
        }

        // -----------------------------------------------------------------
        // Zeitraum aus dem voranmeldungszeitraum-Attribut
        // (Pendant easycashdoc.cpp:2266-2310):
        // 1-11 Monat, 12 Dezember (Jahreswechsel), 13-15 Quartal 1-3,
        // 16 Quartal 4 (Jahreswechsel), sonst ganzes Buchungsjahr.
        // Vergleich: Datum >= von && Datum < bis.
        // -----------------------------------------------------------------
        private void BerechneZeitraum()
        {
            int z = _def.Voranmeldungszeitraum;
            int jahr = _doc.Jahr;
            if (z >= 1 && z <= 16)
            {
                if (z < 12)
                {
                    _von = new DateTime(jahr, z, 1);
                    _bis = new DateTime(jahr, z + 1, 1);
                }
                else if (z == 12)
                {
                    _von = new DateTime(jahr, 12, 1);
                    _bis = new DateTime(jahr + 1, 1, 1);
                }
                else if (z < 16)
                {
                    _von = new DateTime(jahr, ((z - 13) * 3) + 1, 1);
                    _bis = new DateTime(jahr, ((z - 13) * 3) + 4, 1);
                }
                else
                {
                    _von = new DateTime(jahr, 10, 1);
                    _bis = new DateTime(jahr + 1, 1, 1);
                }
            }
            else
            {
                _von = new DateTime(jahr, 1, 1);
                _bis = new DateTime(jahr + 1, 1, 1);
            }
        }

        // -----------------------------------------------------------------
        // Betriebsfilter-Bedingung -- 1:1-Port inkl. C-Praezedenz
        // (easycashdoc.cpp:2351):
        //   !attr_filter || *attr_filter=='\0'
        //   || (!stricmp(attr_filter,"betrieb") && (!sFilter || *sFilter=='\0')
        //       || p->Betrieb == sFilter)
        // && bindet staerker als ||, also: Attribut leer ODER
        // (Attribut=="betrieb" UND Filter leer) ODER Betrieb==Filter.
        // Kurios, aber absichtlich beibehalten: bei gesetztem Attribut
        // != "betrieb" bleibt nur der Betrieb==Filter-Vergleich.
        // -----------------------------------------------------------------
        private bool PasstBetrieb(Buchung b)
        {
            string attr = _def.Filter;
            if (string.IsNullOrEmpty(attr)) return true;
            bool istBetriebAttr = attr.Equals("betrieb", StringComparison.OrdinalIgnoreCase);
            return (istBetriebAttr && string.IsNullOrEmpty(_filter))
                || (b.Betrieb ?? "") == _filter;
        }

        private bool ImZeitraum(Buchung b) => b.Datum >= _von && b.Datum < _bis;

        // -----------------------------------------------------------------
        // Betragsanteile (Cent, long)
        // -----------------------------------------------------------------
        private static long AnteilEinnahme(Buchung b, FormularFeldAnteil anteil)
        {
            switch (anteil)
            {
                case FormularFeldAnteil.Netto: return b.BruttoBetrag.NettoInCent;
                case FormularFeldAnteil.Mwst: return b.BruttoBetrag.InCent - b.BruttoBetrag.NettoInCent;
                default: return b.BruttoBetrag.InCent;
            }
        }

        private long AnteilAusgabe(Buchung b, FormularFeldAnteil anteil)
        {
            // AfA-Sonderfaelle wie nativ (easycashdoc.cpp:2415-2423):
            // netto = Jahresanteil der Abschreibung; Vorsteuer nur im
            // ersten Abschreibungsjahr.
            switch (anteil)
            {
                case FormularFeldAnteil.Netto:
                    return AfaCalculator.GetBuchungsjahrNetto(b, _doc.GlobaleAfaGenauigkeit);
                case FormularFeldAnteil.Mwst:
                    return b.AfaNr == 1
                        ? b.BruttoBetrag.InCent - b.BruttoBetrag.NettoInCent
                        : 0;
                default:
                    return AfaCalculator.GetBuchungsjahrNetto(b, _doc.GlobaleAfaGenauigkeit)
                        + (b.AfaNr == 1
                            ? b.BruttoBetrag.InCent - b.BruttoBetrag.NettoInCent
                            : 0);
            }
        }

        // -----------------------------------------------------------------
        // Verknuepfte Konten einer Feld-Id (via "ECT"-Feldzuweisungs-Blob,
        // Key = Formularname). Pendant der Schleifen easycashdoc.cpp:2323/2385.
        // -----------------------------------------------------------------
        private List<string> VerknuepfteKonten(bool einnahmen, int feldId)
        {
            var konten = einnahmen ? _einnahmenKonten : _ausgabenKonten;
            var zuweisungen = einnahmen ? _einnahmenFeldzuweisungen : _ausgabenFeldzuweisungen;
            var ergebnis = new List<string>();
            for (int j = 0; j < 100 && !string.IsNullOrEmpty(konten[j]); j++)
            {
                var store = ErweiterungStore.AusPipeFormat(zuweisungen[j]);
                if (feldId == FormularDefinition.Atoi(store.Hole("ECT", _def.Name)))
                    ergebnis.Add(konten[j]);
            }
            return ergebnis;
        }

        // -----------------------------------------------------------------
        // Pendant GetFormularwertByIndex fuer EIN Feld
        // -----------------------------------------------------------------
        private string BerechneFeld(FormularFeldDef feld)
        {
            int nID = feld.Id;
            bool idImBereich = nID >= 0 && nID < MaxFeldId;

            // Deadlock-Schutz bei Formel-Rekursionen: Feld VOR der
            // Berechnung als gueltig markieren (easycashdoc.cpp:2278) --
            // zyklische Verweise sehen dann den (noch) gespeicherten Wert.
            if (idImBereich) _gueltig[nID] = true;

            string text;
            switch (feld.Typ)
            {
                case FormularFeldTyp.Einnahmen:
                    text = BerechneEinnahmenAusgaben(feld, einnahmen: true);
                    break;
                case FormularFeldTyp.Ausgaben:
                    text = BerechneEinnahmenAusgaben(feld, einnahmen: false);
                    break;
                case FormularFeldTyp.Summe:
                    text = BerechneSumme(feld);
                    break;
                case FormularFeldTyp.Einstellungsdaten:
                    text = BerechneEinstellungsdaten(feld);
                    if (idImBereich) _werte[nID] = CurrencyToCent(text);
                    break;
                case FormularFeldTyp.Dokumentdaten:
                    text = BerechneDokumentdaten(feld);
                    if (idImBereich) _werte[nID] = CurrencyToCent(text);
                    break;
                case FormularFeldTyp.Freitext:
                    text = ErsetzePlatzhalter(feld.Erweiterung);
                    if (idImBereich) _werte[nID] = CurrencyToCent(text);
                    break;
                default:
                    text = "";
                    break;
            }

            // Nullwerte unterdruecken, ausser nullwertdarstellung="ja"
            // (nativ wird hart gegen "0,00" verglichen -- easycashdoc.cpp:2909;
            // hier locale-korrekt gegen den formatierten Nullbetrag, was den
            // Alt-Bug auf Nicht-Komma-Locales behebt).
            if (!feld.NullwertAnzeigen)
                if (text == NullBetragText || text == "0")
                    text = "";

            if (idImBereich)
            {
                if (!string.IsNullOrEmpty(_statustext[nID]))
                    _statustext[nID] += " -- ";
                _statustext[nID] += "Feldname: '" + feld.Name + "'";
            }

            return text;
        }

        private static string NullBetragText => Waehrungsformat.BetragOhneGruppierung(0m);

        // -----------------------------------------------------------------
        // Feldtypen Einnahmen/Ausgaben (easycashdoc.cpp:2317/2379)
        // -----------------------------------------------------------------
        private string BerechneEinnahmenAusgaben(FormularFeldDef feld, bool einnahmen)
        {
            int nID = feld.Id;
            var verknuepft = VerknuepfteKonten(einnahmen, nID);

            // Statustext (Formatstrings wie nativ)
            if (nID >= 0 && nID < MaxFeldId)
            {
                string kontenart = einnahmen ? "Einnahmenkonten" : "Ausgabenkonten";
                if (verknuepft.Count == 0)
                    _statustext[nID] = string.Format(CultureInfo.InvariantCulture,
                        "Es gibt keine {0}, die mit Feld {1} ({2}) verknüpft sind. "
                        + "Das kann unter Einstellungen->E/Ü-Konten geändert werden.",
                        kontenart, nID, _def.Name);
                else if (verknuepft.Count == 1)
                    _statustext[nID] = string.Format(CultureInfo.InvariantCulture,
                        "{0} Feld {1} ist mit dem Konto '{2}' verknüpft.",
                        feld.Erweiterung, nID, verknuepft[0]);
                else
                {
                    var sb = new System.Text.StringBuilder();
                    sb.AppendFormat(CultureInfo.InvariantCulture,
                        "{0} Feld {1} ist verknüpft mit den Konten ", feld.Erweiterung, nID);
                    for (int i = 0; i < verknuepft.Count; i++)
                        sb.Append('\'').Append(verknuepft[i]).Append('\'')
                          .Append(i == verknuepft.Count - 1 ? "" : ", ");
                    _statustext[nID] = sb.ToString();
                }
            }

            var art = einnahmen ? Buchungsart.Einnahme : Buchungsart.Ausgabe;
            long summe = 0;
            foreach (var b in _doc.Buchungen)
            {
                if (b.Art != art) continue;
                if (!verknuepft.Contains(b.Konto)) continue;
                if (!ImZeitraum(b) || !PasstBetrieb(b)) continue;
                summe += einnahmen
                    ? AnteilEinnahme(b, feld.Anteil)
                    : AnteilAusgabe(b, feld.Anteil);
            }

            return FormatiereSumme(feld, summe);
        }

        // -----------------------------------------------------------------
        // Formatierung + Wert-Cache (gemeinsamer Schwanz der drei
        // Summier-Feldtypen, easycashdoc.cpp:2363-2377 etc.)
        // -----------------------------------------------------------------
        private string FormatiereSumme(FormularFeldDef feld, long summe)
        {
            string text;
            if (feld.NachkommaanteilOhne)
            {
                // ganze Euro mit C-Truncation (summe/100), Cache auf volle
                // Euro abgeschnitten (summe/100*100)
                text = (summe / 100).ToString(CultureInfo.InvariantCulture);
                if (feld.Id >= 0 && feld.Id < MaxFeldId)
                    _werte[feld.Id] = summe / 100 * 100;
            }
            else
            {
                // int_to_currency: Betrag ohne Tausendergruppierung,
                // Dezimaltrenner nach Systemeinstellung
                text = Waehrungsformat.BetragOhneGruppierung(summe / 100m);
                if (feld.Id >= 0 && feld.Id < MaxFeldId)
                    _werte[feld.Id] = summe;
            }
            return text;
        }

        // -----------------------------------------------------------------
        // Feldtyp Summe -- Formelparser (easycashdoc.cpp:2444-2660).
        // Syntax: [+|-] Term { (+|-) Term }
        //   Term = FeldID ['*'Faktor] ['/'Divisor]
        //        | ('netto'|'mwst'|'brutto') [SatzPromille] '(' (e|a) [':'FeldID] ')'
        // Fehlerhafte Formeln liefern den Fehlerwert -99999999.
        // -----------------------------------------------------------------
        private string BerechneSumme(FormularFeldDef feld)
        {
            string formel = feld.Erweiterung ?? "";
            int pos = 0;
            char operation = '+';

            if (pos < formel.Length && formel[pos] == '+') pos++;
            else if (pos < formel.Length && formel[pos] == '-') { operation = '-'; pos++; }

            long summe = 0;
            while (pos < formel.Length)
            {
                if (char.IsDigit(formel[pos]))
                {
                    // Feld-ID mit optionalem *Faktor / Divisor
                    int feldId = LiesZahl(formel, ref pos);
                    int faktor = 1, divisor = 1;
                    if (pos < formel.Length && formel[pos] == '*')
                    {
                        pos++;
                        faktor = LiesZahl(formel, ref pos);
                    }
                    if (pos < formel.Length && formel[pos] == '/')
                    {
                        pos++;
                        divisor = LiesZahl(formel, ref pos);
                    }

                    if (feldId >= 0 && feldId < MaxFeldId)
                    {
                        if (!_gueltig[feldId]) BerechneFeldById(feldId);
                        // Divisor 0 waere nativ ein Absturz -- defensiv wie 1
                        if (divisor == 0) divisor = 1;
                        // Cast-Kette wie nativ: (int)((LONGLONG)wert * faktor / divisor)
                        long anteil = unchecked((int)(_werte[feldId] * (long)faktor / divisor));
                        if (operation == '+') summe += anteil;
                        else summe -= anteil;
                    }
                }
                else
                {
                    // Kontenkategorie-Term netto|mwst|brutto[satz](e|a[:id])
                    char anteil = ' ';
                    if (Folgt(formel, pos, "netto")) { anteil = 'n'; pos += 5; }
                    else if (Folgt(formel, pos, "mwst")) { anteil = 'm'; pos += 4; }
                    else if (Folgt(formel, pos, "brutto")) { anteil = 'b'; pos += 6; }

                    int mwstsatz = -1;   // in Promille (19000 = 19 %)
                    if (pos < formel.Length && char.IsDigit(formel[pos]))
                        mwstsatz = LiesZahl(formel, ref pos);

                    if (pos >= formel.Length || formel[pos] != '(') { summe = Fehlerwert; break; }
                    pos++;
                    int klammerZu = formel.IndexOf(')', pos);
                    if (klammerZu < 0) { summe = Fehlerwert; break; }

                    if (pos >= formel.Length) { summe = Fehlerwert; break; }
                    char kontenart = char.ToLowerInvariant(formel[pos]);
                    pos++;
                    if (kontenart != 'e' && kontenart != 'a') { summe = Fehlerwert; break; }

                    int kontenFeldId;
                    if (pos < formel.Length && formel[pos] == ')')
                    {
                        kontenFeldId = -1;   // (e)/(a) = alle Buchungen der Kategorie
                    }
                    else
                    {
                        if (pos >= formel.Length || formel[pos] != ':') { summe = Fehlerwert; break; }
                        pos++;
                        kontenFeldId = LiesZahlAb(formel, pos);
                    }
                    pos = klammerZu + 1;

                    long untersumme = SummiereKategorie(kontenart == 'e', kontenFeldId, anteil, mwstsatz);

                    // Operations-Kette wie nativ (auch die faktisch toten
                    // * und / Zweige, easycashdoc.cpp:2625-2628)
                    if (operation == '+') summe += untersumme;
                    else if (operation == '-') summe -= untersumme;
                    else if (operation == '*') summe *= untersumme;
                    else summe = untersumme != 0 ? summe / untersumme : summe;
                }

                // Nur '+'/'-' verbinden Terme; jedes andere Zeichen laeuft in
                // der naechsten Runde in den Kontenkategorie-Zweig und
                // scheitert dort ggf. am '('-Check (Fehlerwert) -- exakt wie
                // der native Parser. Insbesondere ist "(e)" OHNE Anteil-Wort
                // gueltig (anteil ' ' faellt auf brutto zurueck).
                if (pos < formel.Length && formel[pos] == '+') { operation = '+'; pos++; }
                else if (pos < formel.Length && formel[pos] == '-') { operation = '-'; pos++; }
            }

            if (feld.Id >= 0 && feld.Id < MaxFeldId)
                _statustext[feld.Id] = string.Format(CultureInfo.InvariantCulture,
                    "Feld {0} summiert sich aus {1}", feld.Id, formel);

            return FormatiereSumme(feld, summe);
        }

        /// <summary>Kontenkategorie-Untersumme (easycashdoc.cpp:2533-2618).
        /// kontenFeldId = -1: alle Buchungen der Kategorie; sonst nur Konten,
        /// deren Feldzuweisung auf diese Id zeigt.</summary>
        private long SummiereKategorie(bool einnahmen, int kontenFeldId, char anteil, int mwstsatzPromille)
        {
            List<string> verknuepft = kontenFeldId != -1
                ? VerknuepfteKonten(einnahmen, kontenFeldId)
                : null;

            var art = einnahmen ? Buchungsart.Einnahme : Buchungsart.Ausgabe;
            var anteilTyp = anteil == 'n' ? FormularFeldAnteil.Netto
                          : anteil == 'm' ? FormularFeldAnteil.Mwst
                          : FormularFeldAnteil.Brutto;

            long untersumme = 0;
            foreach (var b in _doc.Buchungen)
            {
                if (b.Art != art) continue;
                if (mwstsatzPromille != -1 && mwstsatzPromille != b.BruttoBetrag.MwstPromille) continue;
                if (!ImZeitraum(b)) continue;
                if (!PasstBetrieb(b)) continue;
                if (verknuepft != null && !verknuepft.Contains(b.Konto)) continue;

                untersumme += einnahmen
                    ? AnteilEinnahme(b, anteilTyp)
                    : AnteilAusgabe(b, anteilTyp);
            }
            return untersumme;
        }

        /// <summary>Pendant GetFormularwertByID (easycashdoc.cpp:2235):
        /// Feld ueber seine Id suchen und (rekursiv) berechnen -- fuer
        /// Formel-Verweise auf noch nicht berechnete Felder.</summary>
        private void BerechneFeldById(int id)
        {
            foreach (var feld in _def.Felder)
            {
                if (!feld.HatInhalt) continue;
                if (feld.Id == id)
                {
                    BerechneFeld(feld);
                    return;
                }
            }
            // nicht gefunden: Wert bleibt 0 (wie nativ)
        }

        // -----------------------------------------------------------------
        // Feldtyp Einstellungsdaten (easycashdoc.cpp:2662-2843):
        // erweiterung = ein oder mehrere space-getrennte Einstellungs-Kurzform-
        // Keys; Werte werden mit Leerzeichen verkettet. Spezialfaelle fuer
        // Betriebs-Properties (Tab-getrennt: Unternehmensart1 \t Rechtsform
        // \t Steuernummer \t W-IdNr) wenn ein Betriebsfilter gesetzt ist.
        // -----------------------------------------------------------------
        private string BerechneEinstellungsdaten(FormularFeldDef feld)
        {
            string ids = feld.Erweiterung ?? "";
            string gesamt = "";

            do
            {
                string id;
                int trenner = ids.IndexOf(' ');
                if (trenner < 0) { id = ids; ids = ""; }
                else { id = ids.Substring(0, trenner); ids = ids.Substring(trenner + 1); }

                string inhalt;
                if (id == "unternehmensart1" && _filter.Length > 0)
                    inhalt = BetriebsProperty(0,
                        fallbackIni: false,
                        nichtGefunden: "<Unternehmensart für Betrieb nicht gefunden>",
                        fallbackKey: null);
                else if (id == "unternehmensart2" && _filter.Length > 0)
                    inhalt = BetriebsProperty(1, fallbackIni: true, nichtGefunden: "", fallbackKey: id);
                else if (id == "fsteuernummer" && _filter.Length > 0)
                    inhalt = BetriebsProperty(2, fallbackIni: true, nichtGefunden: "", fallbackKey: id);
                else if (id == "wirtschaftsidnr")
                {
                    if (_filter.Length > 0)
                        inhalt = BetriebsProperty(3, fallbackIni: true, nichtGefunden: "", fallbackKey: id);
                    else
                    {
                        // ohne Betriebsfilter: [Finanzamt]wirtschaftsidnr,
                        // nur die ersten 11 Zeichen (Stammnummer ohne
                        // Unterscheidungsmerkmal)
                        inhalt = Einstellungen.Hole("fwirtschaftsidnr");
                        if (inhalt.Length > 11) inhalt = inhalt.Substring(0, 11);
                    }
                }
                else
                    inhalt = HoleEinstellungKurzform(id);

                // Verkettungs-Logik wie nativ (fuehrende Leerwerte erzeugen
                // keinen Trenner, mittlere schon)
                if (gesamt == "") gesamt = inhalt;
                else gesamt += " " + inhalt;
            }
            while (ids != "");

            return gesamt;
        }

        /// <summary>Tab-getrennte Betriebs-Property des gefilterten Betriebs
        /// (teilIndex: 0=Unternehmensart, 1=Rechtsform, 2=Steuernummer,
        /// 3=W-IdNr). fallbackIni: bei fehlendem/leerem Teil den normalen
        /// Einstellungs-Key nachschlagen (wie die nativen Zweige).</summary>
        private string BetriebsProperty(int teilIndex, bool fallbackIni,
            string nichtGefunden, string fallbackKey)
        {
            for (int i = 0; i < 100; i++)
            {
                string prefix = "[Betriebe]Betrieb" + i.ToString("D2", CultureInfo.InvariantCulture);
                string name = Einstellungen.Hole(prefix + "Name");
                if (string.IsNullOrEmpty(name))
                    return nichtGefunden;   // Betrieb nicht gefunden
                if (name != _filter) continue;

                string[] teile = Einstellungen.Hole(prefix + "Unternehmensart").Split('\t');
                string teil = teilIndex < teile.Length ? teile[teilIndex] : "";
                if (teil.Length > 0)
                {
                    // W-IdNr ist der letzte Teil und behaelt nativ auch
                    // eventuelle weitere Tab-Reste -- hier: kompletter Rest
                    if (teilIndex == 3 && teile.Length > 4)
                        return string.Join("\t", teile, 3, teile.Length - 3);
                    return teil;
                }
                return fallbackIni && fallbackKey != null
                    ? HoleEinstellungKurzform(fallbackKey)
                    : "";
            }
            return nichtGefunden;
        }

        // -----------------------------------------------------------------
        // Kurzform-Key -> kanonischer Cache-Key (Pendant des nativen
        // GetPrivateProfileString(IniSektion(id), ...)-Musters):
        //   Praefix-Sektionen (Finanzamt 'f', EinnahmenRechnungsposten 'e',
        //   AusgabenRechnungsposten 'a'): Cache-Key == id.
        //   Sonst: "[Sektion]" + id (KuerzelFuerCache-Form).
        // -----------------------------------------------------------------
        public static string HoleEinstellungKurzform(string id)
        {
            string sektion = IniSektion(id);
            if (sektion == "Finanzamt"
                || sektion == "EinnahmenRechnungsposten"
                || sektion == "AusgabenRechnungsposten")
                return Einstellungen.Hole(id);
            return Einstellungen.Hole("[" + sektion + "]" + id);
        }

        /// <summary>Port von IniSektion (ECTBridge/ectifacemisc.cpp:444) --
        /// ordnet einer Einstellungs-Kurzform ihre ini-Sektion zu.</summary>
        public static string IniSektion(string id)
        {
            if (id == null) id = "";
            switch (id)
            {
                case "monatliche_voranmeldung":
                case "taeglich_buchen":
                case "BuchungsdatumBelassen":
                case "ErzeugeLaufendeBuchungsnummernFuerEinnahmen":
                case "ErzeugeLaufendeBuchungsnummernFuerAusgaben":
                case "JahresfeldAktiviert":
                case "tausender_mwst_saetze":
                case "AbschreibungGenauigkeit":
                case "JahreswechselAbfrage":
                case "telefon":
                case "email":
                    return "Allgemein";
                case "umstvoranmeldung_h":
                case "umstvoranmeldung_h2":
                case "umstvoranmeldung_v":
                case "formular_nicht_mitdrucken":
                case "Bildschirmschrift":
                case "Bildschirmschriftgroesse":
                case "Druckerschrift":
                case "Druckerschriftgroesse":
                    return "Druck";
                case "name":
                case "vorname":
                case "unternehmensart1":
                case "unternehmensart2":
                case "strasse":
                case "plz":
                case "ort":
                case "land":
                case "vat1":
                case "vat2":
                case "vat3":
                    return "Persoenliche_Daten";
                case "fname":
                case "fname2":
                case "fstrasse":
                case "fplz":
                case "fort":
                case "fsteuernummer":
                    return "Finanzamt";
            }

            if (id.Length == 3 && id[0] == 'e' && char.IsDigit(id[1]) && char.IsDigit(id[2]))
                return "EinnahmenRechnungsposten";
            if (id.Length == 3 && id[0] == 'a' && char.IsDigit(id[1]) && char.IsDigit(id[2]))
                return "AusgabenRechnungsposten";

            if (id.Length > 2)
            {
                string suffix = id.Substring(2);
                if (suffix == "Text" || suffix == "Ausg" || suffix == "MWSt"
                    || suffix == "AfAJ" || suffix == "Rech")
                    return "Buchungsposten";
            }

            return "Allgemein";
        }

        // -----------------------------------------------------------------
        // Feldtyp Dokumentdaten (easycashdoc.cpp:2845-2886): Wert aus dem
        // Dokument-ErweiterungStore; Default-Sektion "Allgemein", explizit
        // per "[Sektion]Key"; Platzhalter $J/$j/$v im Key.
        // -----------------------------------------------------------------
        private string BerechneDokumentdaten(FormularFeldDef feld)
        {
            string id = ErsetzePlatzhalter(feld.Erweiterung ?? "");
            string sektion = "Allgemein";
            string key = id;

            if (id.Length > 0 && id[0] == '[')
            {
                int klammerZu = id.IndexOf(']');
                if (klammerZu > 1)
                {
                    sektion = id.Substring(1, klammerZu - 1);
                    key = id.Substring(klammerZu + 1);
                }
            }

            return _doc.Erweiterungen.Hole(sektion, key, "");
        }

        /// <summary>Platzhalter $J (Jahr JJJJ), $j (Jahr, nativ ebenfalls
        /// 4-stellig -- %-2.2d schneidet nicht ab), $v (Voranmeldungszeitraum).</summary>
        private string ErsetzePlatzhalter(string text)
        {
            if (string.IsNullOrEmpty(text)) return "";
            return text
                .Replace("$J", _doc.Jahr.ToString("D4", CultureInfo.InvariantCulture))
                .Replace("$j", _doc.Jahr.ToString("D2", CultureInfo.InvariantCulture))
                .Replace("$v", _def.Voranmeldungszeitraum.ToString(CultureInfo.InvariantCulture));
        }

        // -----------------------------------------------------------------
        // Port von currency_to_int (ECTBridge/ectifacemisc.cpp:29):
        // liest Ziffern, Dezimaltrenner (max. 2 Nachkommastellen),
        // fuehrendes Minus und Gruppierungszeichen; jedes andere Zeichen
        // beendet das Parsen. Ergebnis in Cent.
        // -----------------------------------------------------------------
        public static long CurrencyToCent(string s)
        {
            if (string.IsNullOrEmpty(s)) return 0;
            var nf = CultureInfo.CurrentCulture.NumberFormat;
            char dezimal = nf.NumberDecimalSeparator.Length > 0 ? nf.NumberDecimalSeparator[0] : ',';
            char gruppe = nf.NumberGroupSeparator.Length > 0 ? nf.NumberGroupSeparator[0] : '.';
            char minus = nf.NegativeSign.Length > 0 ? nf.NegativeSign[0] : '-';

            long n = 0;
            int budget = 200;   // wie nativ: i zaehlt Restziffern nach Komma
            bool neg = false;
            foreach (char c in s)
            {
                if (c < '0' || c > '9')
                {
                    if (c == dezimal) budget = 2;
                    else if (c == minus && n == 0) neg = true;
                    else if (c != gruppe) break;
                }
                else
                {
                    n = n * 10 + (c - '0');
                    budget--;
                    if (budget <= 0) break;
                }
            }
            if (budget > 2) budget = 2;
            for (int j = 0; j < budget; j++) n *= 10;
            if (neg && n != 0) n = -n;
            return n;
        }

        // -----------------------------------------------------------------
        // Parser-Helfer
        // -----------------------------------------------------------------
        private static bool Folgt(string s, int pos, string wort)
            => pos + wort.Length <= s.Length
               && string.CompareOrdinal(s, pos, wort, 0, wort.Length) == 0;

        /// <summary>Liest eine Ziffernfolge ab pos und schiebt pos weiter.</summary>
        private static int LiesZahl(string s, ref int pos)
        {
            long wert = 0;
            bool ziffern = false;
            while (pos < s.Length && s[pos] >= '0' && s[pos] <= '9')
            {
                wert = wert * 10 + (s[pos] - '0');
                if (wert > int.MaxValue) wert = int.MaxValue;
                pos++;
                ziffern = true;
            }
            return ziffern ? (int)wert : 0;
        }

        /// <summary>atoi ab Position (ohne pos-Fortschritt) -- fuer die
        /// Feld-Id in "(e:1234)", die der native Code per atoi liest und
        /// danach direkt hinter die Klammer springt.</summary>
        private static int LiesZahlAb(string s, int pos)
        {
            int p = pos;
            return LiesZahl(s, ref p);
        }
    }
}
