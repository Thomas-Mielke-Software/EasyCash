// FormularRechnerTests.cs — Tests für den FormularRechner (Port von
// GetFormularwertByIndex): Zeiträume inkl. Dezember/Q4-Jahreswechsel,
// netto/mwst/brutto, AfA-Sonderfälle, Betriebsfilter, Summe-Formelparser
// (Feld-Verweise mit Faktor/Divisor, Kontenkategorie-Terme, Rekursion,
// Zyklen, Fehlerwert), nachkommaanteil-Truncation, Nullwertdarstellung,
// Einstellungsdaten-Verkettung mit Betriebs-Spezialfällen, Dokumentdaten
// und Freitext-Platzhalter sowie currency_to_int-Semantik.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using Xunit;

namespace ECTEngine.Tests
{
    [Collection("EinstellungenCache")]   // statischer Einstellungs-Cache
    public class FormularRechnerTests : IDisposable
    {
        private readonly string _tempDir;

        public FormularRechnerTests()
        {
            _tempDir = Path.Combine(Path.GetTempPath(),
                "ect-rechner-tests-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(_tempDir);
        }

        public void Dispose()
        {
            try { Directory.Delete(_tempDir, recursive: true); } catch { }
        }

        // ------------------------------------------------------------------
        // Aufbau-Helfer
        // ------------------------------------------------------------------

        private static string Geld(decimal euro)
            => Waehrungsformat.BetragOhneGruppierung(euro);

        /// <summary>Formular-Definition aus Feld-Schnipseln bauen und laden.</summary>
        private FormularDefinition Formular(string formularAttribute, params string[] felder)
        {
            string xml = "<?xml version=\"1.0\" encoding=\"windows-1252\" ?>\r\n"
                + "<formular name=\"Testform\" seiten=\"1\" " + formularAttribute + " >\r\n"
                + "<felder>\r\n" + string.Join("\r\n", felder) + "\r\n</felder>\r\n"
                + "</formular>";
            string pfad = Path.Combine(_tempDir, Guid.NewGuid().ToString("N") + ".ecf");
            File.WriteAllText(pfad, xml, Encoding.GetEncoding(1252));
            return FormularDefinition.Lade(pfad);
        }

        private static string Feld(int id, string typ, string erweiterung,
            string extraAttribute = "")
        {
            return "<feld id=\"" + id + "\" typ=\"" + typ
                + "\" seite=\"1\" horizontal=\"100\" vertikal=\"100\" "
                + extraAttribute
                + " ><name>Feld" + id + "</name><erweiterung>"
                + erweiterung + "</erweiterung></feld>";
        }

        private static string Blob(string formularName, string feldId)
        {
            var store = new ErweiterungStore();
            store.Setze("ECT", formularName, feldId);
            return store.ZuPipeFormat();
        }

        /// <summary>Einstellungs-Cache mit Konten + Zuweisungen befüllen:
        /// e00 "Erlöse"-&gt;Feld 20, e01 "Provisionen"-&gt;Feld 21,
        /// a00 "Bürobedarf"-&gt;Feld 40.</summary>
        private static void SeedStandard(params (string key, string val)[] extra)
        {
            var dict = new Dictionary<string, string>
            {
                ["e00"] = "Erlöse",
                ["[EinnahmenFeldzuweisungen]00"] = Blob("Testform", "20"),
                ["e01"] = "Provisionen",
                ["[EinnahmenFeldzuweisungen]01"] = Blob("Testform", "21"),
                ["a00"] = "Bürobedarf",
                ["[AusgabenFeldzuweisungen]00"] = Blob("Testform", "40"),
            };
            foreach (var (key, val) in extra) dict[key] = val;
            Einstellungen.LadeAusBridge(dict);
        }

        private static BuchungsDocument Doc2024() =>
            new BuchungsDocument { Jahr = 2024, Waehrung = "EUR" };

        private static Buchung Einnahme(string konto, int brutto, int mwstPromille,
            DateTime datum, string betrieb = "")
            => new Buchung
            {
                Art = Buchungsart.Einnahme,
                Konto = konto,
                BruttoBetrag = Betrag.AusCent(brutto, mwstPromille),
                Datum = datum,
                Betrieb = betrieb
            };

        private static Buchung Ausgabe(string konto, int brutto, int mwstPromille,
            DateTime datum, string betrieb = "")
            => new Buchung
            {
                Art = Buchungsart.Ausgabe,
                Konto = konto,
                BruttoBetrag = Betrag.AusCent(brutto, mwstPromille),
                Datum = datum,
                Betrieb = betrieb
            };

        private static FormularFeldWert WertVon(
            IReadOnlyList<FormularFeldWert> werte, int id)
            => werte.First(w => w.Feld.Id == id);

        // ------------------------------------------------------------------
        // Einnahmen/Ausgaben: Anteile + Konten-Verknüpfung
        // ------------------------------------------------------------------

        [Fact]
        public void EinnahmenNettoMwstBrutto()
        {
            SeedStandard();
            var doc = Doc2024();
            // 1190,00 brutto bei 19 % -> netto 1000,00, MwSt 190,00
            doc.Buchungen.Add(Einnahme("Erlöse", 119000, 19000, new DateTime(2024, 3, 15)));
            // nicht verknüpftes Konto und Ausgaben-Buchung: ignoriert
            doc.Buchungen.Add(Einnahme("Sonstiges", 50000, 19000, new DateTime(2024, 3, 15)));
            doc.Buchungen.Add(Ausgabe("Erlöse", 10000, 19000, new DateTime(2024, 3, 15)));

            var def = Formular("",
                Feld(20, "Einnahmen", "", "anteil=\"netto\""),
                Feld(21, "Einnahmen", "", "anteil=\"mwst\""),
                Feld(22, "Einnahmen", "", "anteil=\"brutto\""));
            // Feld 21/22 sind nicht verknüpft (e00 zeigt auf 20) -> für den
            // Test alle drei auf dasselbe Konto zeigen lassen:
            SeedStandard(
                ("[EinnahmenFeldzuweisungen]01", Blob("Testform", "21")),
                ("e01", "Erlöse"));

            // getrennt rechnen: Feld 20 (netto) über e00
            var werte = FormularRechner.Berechne(doc, def, "");
            Assert.Equal(Geld(1000.00m), WertVon(werte, 20).Text);
            Assert.Equal(100000, WertVon(werte, 20).WertCent);
            // Feld 21 (mwst) über e01 ("Erlöse" doppelt verknüpft):
            // beide Konten-Slots heißen "Erlöse" -> Buchung zählt einmal
            Assert.Equal(Geld(190.00m), WertVon(werte, 21).Text);
            Assert.Equal(19000, WertVon(werte, 21).WertCent);
            // Feld 22 hat keine Verknüpfung -> 0 -> unterdrückt
            Assert.Equal("", WertVon(werte, 22).Text);
            Assert.Equal(0, WertVon(werte, 22).WertCent);
        }

        [Fact]
        public void StatustextNenntVerknuepfteKonten()
        {
            SeedStandard();
            var doc = Doc2024();
            var def = Formular("", Feld(20, "Einnahmen", "", "anteil=\"netto\""));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Contains("ist mit dem Konto 'Erlöse' verknüpft", WertVon(werte, 20).Statustext);
            Assert.Contains("Feldname: 'Feld20'", WertVon(werte, 20).Statustext);

            var def2 = Formular("", Feld(99, "Einnahmen", "", "anteil=\"netto\""));
            var werte2 = FormularRechner.Berechne(doc, def2, "");
            Assert.Contains("Es gibt keine Einnahmenkonten", WertVon(werte2, 99).Statustext);
        }

        // ------------------------------------------------------------------
        // Zeiträume (voranmeldungszeitraum)
        // ------------------------------------------------------------------

        [Theory]
        [InlineData(1, "2024-01-01", true)]
        [InlineData(1, "2024-01-31", true)]
        [InlineData(1, "2024-02-01", false)]
        [InlineData(12, "2024-12-31", true)]    // Dezember inkl. Jahresende
        [InlineData(12, "2024-11-30", false)]
        [InlineData(12, "2025-01-01", false)]   // < bis (exklusiv)
        [InlineData(13, "2024-03-31", true)]    // Q1
        [InlineData(13, "2024-04-01", false)]
        [InlineData(16, "2024-10-01", true)]    // Q4 inkl. Jahreswechsel-Grenze
        [InlineData(16, "2024-12-31", true)]
        [InlineData(16, "2024-09-30", false)]
        public void Voranmeldungszeitraum(int zeitraum, string datum, bool erwartet)
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Buchungen.Add(Einnahme("Erlöse", 11900, 19000,
                DateTime.ParseExact(datum, "yyyy-MM-dd", CultureInfo.InvariantCulture)));

            var def = Formular("voranmeldungszeitraum=\"" + zeitraum + "\"",
                Feld(20, "Einnahmen", "", "anteil=\"brutto\""));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal(erwartet ? 11900 : 0, WertVon(werte, 20).WertCent);
        }

        [Fact]
        public void OhneZeitraumZaehltGanzesBuchungsjahr()
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Buchungen.Add(Einnahme("Erlöse", 10000, 0, new DateTime(2024, 1, 1)));
            doc.Buchungen.Add(Einnahme("Erlöse", 10000, 0, new DateTime(2024, 12, 31)));
            doc.Buchungen.Add(Einnahme("Erlöse", 10000, 0, new DateTime(2023, 12, 31)));
            doc.Buchungen.Add(Einnahme("Erlöse", 10000, 0, new DateTime(2025, 1, 1)));

            var def = Formular("", Feld(20, "Einnahmen", "", "anteil=\"brutto\""));
            Assert.Equal(20000, WertVon(FormularRechner.Berechne(doc, def, ""), 20).WertCent);
        }

        // ------------------------------------------------------------------
        // Betriebsfilter
        // ------------------------------------------------------------------

        [Fact]
        public void BetriebsfilterWirktNurMitFilterAttribut()
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Buchungen.Add(Einnahme("Erlöse", 10000, 0, new DateTime(2024, 5, 1), "Laden"));
            doc.Buchungen.Add(Einnahme("Erlöse", 20000, 0, new DateTime(2024, 5, 1), "Online"));

            // mit filter="betrieb" + gewähltem Betrieb: nur dieser Betrieb
            var defMitFilter = Formular("filter=\"betrieb\"",
                Feld(20, "Einnahmen", "", "anteil=\"brutto\""));
            Assert.Equal(10000,
                WertVon(FormularRechner.Berechne(doc, defMitFilter, "Laden"), 20).WertCent);

            // mit filter="betrieb" ohne gewählten Betrieb: alle
            Assert.Equal(30000,
                WertVon(FormularRechner.Berechne(doc, defMitFilter, ""), 20).WertCent);

            // ohne filter-Attribut: Filter wird ignoriert
            var defOhne = Formular("", Feld(20, "Einnahmen", "", "anteil=\"brutto\""));
            Assert.Equal(30000,
                WertVon(FormularRechner.Berechne(doc, defOhne, "Laden"), 20).WertCent);
        }

        // ------------------------------------------------------------------
        // AfA-Sonderfälle bei Ausgaben
        // ------------------------------------------------------------------

        [Fact]
        public void AusgabenAfaNettoIstJahresanteilVorsteuerNurImErstenJahr()
        {
            SeedStandard();
            var doc = Doc2024();
            var afaBuchung = Ausgabe("Bürobedarf", 238000, 19000, new DateTime(2024, 1, 10));
            afaBuchung.AfaJahre = 2;
            afaBuchung.AfaNr = 1;
            doc.Buchungen.Add(afaBuchung);

            long erwarteterJahresanteil = AfaCalculator.GetBuchungsjahrNetto(
                afaBuchung, doc.GlobaleAfaGenauigkeit);
            long mwst = afaBuchung.BruttoBetrag.InCent - afaBuchung.BruttoBetrag.NettoInCent;

            var def = Formular("",
                Feld(40, "Ausgaben", "", "anteil=\"netto\""),
                Feld(41, "Ausgaben", "", "anteil=\"mwst\""),
                Feld(42, "Ausgaben", "", "anteil=\"brutto\""));
            // Feld 41/42 auch aufs Konto zeigen lassen
            SeedStandard(
                ("a01", "Bürobedarf"), ("[AusgabenFeldzuweisungen]01", Blob("Testform", "41")),
                ("a02", "Bürobedarf"), ("[AusgabenFeldzuweisungen]02", Blob("Testform", "42")));

            var werte = FormularRechner.Berechne(doc, def, "");
            Assert.Equal(erwarteterJahresanteil, WertVon(werte, 40).WertCent);
            Assert.Equal(mwst, WertVon(werte, 41).WertCent);   // AfaNr == 1: VSt voll
            Assert.Equal(erwarteterJahresanteil + mwst, WertVon(werte, 42).WertCent);

            // Folgejahr (AfaNr = 2): keine Vorsteuer mehr
            afaBuchung.AfaNr = 2;
            long jahresanteil2 = AfaCalculator.GetBuchungsjahrNetto(
                afaBuchung, doc.GlobaleAfaGenauigkeit);
            var werte2 = FormularRechner.Berechne(doc, def, "");
            Assert.Equal(0, WertVon(werte2, 41).WertCent);
            Assert.Equal(jahresanteil2, WertVon(werte2, 42).WertCent);
        }

        // ------------------------------------------------------------------
        // Summe-Formeln
        // ------------------------------------------------------------------

        [Fact]
        public void SummeMitFeldverweisFaktorDivisorUndVorwaertsRekursion()
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Buchungen.Add(Einnahme("Erlöse", 100000, 0, new DateTime(2024, 6, 1)));

            // Summenfeld steht VOR dem referenzierten Feld 20 -> Rekursion
            var def = Formular("",
                Feld(10, "Summe", "20*19/100"),
                Feld(20, "Einnahmen", "", "anteil=\"brutto\""));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal(100000, WertVon(werte, 20).WertCent);
            Assert.Equal(19000, WertVon(werte, 10).WertCent);   // 1000,00 * 19/100
            Assert.Equal(Geld(190.00m), WertVon(werte, 10).Text);
        }

        [Fact]
        public void SummeAusMehrerenFeldern()
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Buchungen.Add(Einnahme("Erlöse", 10000, 0, new DateTime(2024, 6, 1)));
            doc.Buchungen.Add(Einnahme("Provisionen", 5000, 0, new DateTime(2024, 6, 1)));

            var def = Formular("",
                Feld(20, "Einnahmen", "", "anteil=\"brutto\""),
                Feld(21, "Einnahmen", "", "anteil=\"brutto\""),
                Feld(10, "Summe", "20+21"),
                Feld(11, "Summe", "20-21"));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal(15000, WertVon(werte, 10).WertCent);
            Assert.Equal(5000, WertVon(werte, 11).WertCent);
        }

        [Fact]
        public void SummeMitKontenkategorieTermen()
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Buchungen.Add(Einnahme("Erlöse", 119000, 19000, new DateTime(2024, 6, 1)));
            doc.Buchungen.Add(Einnahme("Provisionen", 21400, 7000, new DateTime(2024, 6, 1)));
            doc.Buchungen.Add(Ausgabe("Bürobedarf", 11900, 19000, new DateTime(2024, 6, 1)));

            var def = Formular("",
                // MwSt der Erlöse + Brutto der Provisionen
                Feld(10, "Summe", "mwst(e:20)+brutto(e:21)"),
                // alle Einnahmen brutto minus alle Ausgaben brutto
                Feld(11, "Summe", "brutto(e)-brutto(a)"),
                // nur Einnahmen mit 19 % (Promille-Satzfilter), netto
                Feld(12, "Summe", "netto19000(e)"),
                // "(e)" ohne Anteil-Wort: nativer Parser fällt auf brutto zurück
                Feld(13, "Summe", "(e)"));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal(19000 + 21400, WertVon(werte, 10).WertCent);
            Assert.Equal(119000 + 21400 - 11900, WertVon(werte, 11).WertCent);
            Assert.Equal(100000, WertVon(werte, 12).WertCent);
            Assert.Equal(119000 + 21400, WertVon(werte, 13).WertCent);
        }

        [Fact]
        public void SummeZyklusTerminiertMitNullwerten()
        {
            SeedStandard();
            var doc = Doc2024();
            var def = Formular("",
                Feld(30, "Summe", "31", "nullwertdarstellung=\"ja\""),
                Feld(31, "Summe", "30", "nullwertdarstellung=\"ja\""));

            var werte = FormularRechner.Berechne(doc, def, "");   // darf nicht hängen
            Assert.Equal(0, WertVon(werte, 30).WertCent);
            Assert.Equal(0, WertVon(werte, 31).WertCent);
            Assert.Equal(Geld(0m), WertVon(werte, 30).Text);
        }

        [Fact]
        public void SummeMitKaputterFormelLiefertFehlerwert()
        {
            SeedStandard();
            var doc = Doc2024();
            var def = Formular("", Feld(10, "Summe", "xyz"));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal(-99999999, WertVon(werte, 10).WertCent);
            Assert.Equal(Geld(-999999.99m), WertVon(werte, 10).Text);
        }

        // ------------------------------------------------------------------
        // nachkommaanteil="ohne" + Nullwertdarstellung
        // ------------------------------------------------------------------

        [Fact]
        public void NachkommaanteilOhneTrunkiert()
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Buchungen.Add(Einnahme("Erlöse", 123456, 0, new DateTime(2024, 6, 1)));

            var def = Formular("",
                Feld(20, "Einnahmen", "", "anteil=\"brutto\" nachkommaanteil=\"ohne\""));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal("1234", WertVon(werte, 20).Text);       // 1234,56 -> 1234
            Assert.Equal(123400, WertVon(werte, 20).WertCent);   // Cache volle Euro
        }

        [Fact]
        public void NullwertdarstellungSteuertUnterdrueckung()
        {
            SeedStandard();
            var doc = Doc2024();   // keine Buchungen -> alles 0

            var def = Formular("",
                Feld(20, "Einnahmen", "", "anteil=\"brutto\""),
                Feld(21, "Einnahmen", "", "anteil=\"brutto\" nullwertdarstellung=\"ja\""),
                Feld(22, "Einnahmen", "", "anteil=\"brutto\" nachkommaanteil=\"ohne\""));
            // Feld 21/22 verknüpfen, damit die Summierung läuft
            SeedStandard(
                ("e01", "Erlöse"), ("[EinnahmenFeldzuweisungen]01", Blob("Testform", "21")),
                ("e02", "Erlöse"), ("[EinnahmenFeldzuweisungen]02", Blob("Testform", "22")));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal("", WertVon(werte, 20).Text);          // "0,00" unterdrückt
            Assert.Equal(Geld(0m), WertVon(werte, 21).Text);    // explizit angezeigt
            Assert.Equal("", WertVon(werte, 22).Text);          // "0" unterdrückt
        }

        // ------------------------------------------------------------------
        // Einstellungsdaten
        // ------------------------------------------------------------------

        [Fact]
        public void EinstellungsdatenVerkettetMehrereKeys()
        {
            SeedStandard(
                ("[Persoenliche_Daten]vorname", "Max"),
                ("[Persoenliche_Daten]name", "Muster"),
                ("fsteuernummer", "11/222/33333"));
            var doc = Doc2024();

            var def = Formular("",
                Feld(1, "Einstellungsdaten", "vorname name"),
                Feld(2, "Einstellungsdaten", "fsteuernummer"));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal("Max Muster", WertVon(werte, 1).Text);
            Assert.Equal("11/222/33333", WertVon(werte, 2).Text);
        }

        [Fact]
        public void EinstellungsdatenBetriebsSpezialfaelle()
        {
            SeedStandard(
                ("[Betriebe]Betrieb00Name", "Laden"),
                ("[Betriebe]Betrieb00Unternehmensart",
                    "Einzelhandel\tGmbH\t99/888/77777\tDE999888777"),
                ("fsteuernummer", "11/222/33333"),
                ("fwirtschaftsidnr", "DE123456789012345"));
            var doc = Doc2024();

            var def = Formular("filter=\"betrieb\"",
                Feld(1, "Einstellungsdaten", "unternehmensart1"),
                Feld(2, "Einstellungsdaten", "unternehmensart2"),
                Feld(3, "Einstellungsdaten", "fsteuernummer"),
                Feld(4, "Einstellungsdaten", "wirtschaftsidnr"));

            // mit gewähltem Betrieb: Tab-Teile der Betriebs-Property
            var mitBetrieb = FormularRechner.Berechne(doc, def, "Laden");
            Assert.Equal("Einzelhandel", WertVon(mitBetrieb, 1).Text);
            Assert.Equal("GmbH", WertVon(mitBetrieb, 2).Text);
            Assert.Equal("99/888/77777", WertVon(mitBetrieb, 3).Text);
            Assert.Equal("DE999888777", WertVon(mitBetrieb, 4).Text);

            // unbekannter Betrieb: Meldung bzw. leer
            var unbekannt = FormularRechner.Berechne(doc, def, "Gibtsnicht");
            Assert.Equal("<Unternehmensart für Betrieb nicht gefunden>",
                WertVon(unbekannt, 1).Text);
            Assert.Equal("", WertVon(unbekannt, 3).Text);

            // ohne Betriebsfilter: W-IdNr aus [Finanzamt], erste 11 Zeichen
            var ohne = FormularRechner.Berechne(doc, def, "");
            Assert.Equal("DE123456789", WertVon(ohne, 4).Text);
            // Steuernummer fällt auf den normalen Finanzamt-Key zurück...
            // (Spezialzweig greift nur MIT Filter)
            Assert.Equal("11/222/33333", WertVon(ohne, 3).Text);
        }

        [Fact]
        public void EinstellungsdatenBetriebFallbackAufIniWennTeilFehlt()
        {
            SeedStandard(
                ("[Betriebe]Betrieb00Name", "Laden"),
                ("[Betriebe]Betrieb00Unternehmensart", "Einzelhandel"),   // nur Teil 1
                ("fsteuernummer", "11/222/33333"),
                ("[Persoenliche_Daten]unternehmensart2", "e.K."));
            var doc = Doc2024();

            var def = Formular("filter=\"betrieb\"",
                Feld(2, "Einstellungsdaten", "unternehmensart2"),
                Feld(3, "Einstellungsdaten", "fsteuernummer"));
            var werte = FormularRechner.Berechne(doc, def, "Laden");

            Assert.Equal("e.K.", WertVon(werte, 2).Text);          // Fallback ini
            Assert.Equal("11/222/33333", WertVon(werte, 3).Text);  // Fallback ini
        }

        // ------------------------------------------------------------------
        // Dokumentdaten + Freitext
        // ------------------------------------------------------------------

        [Fact]
        public void DokumentdatenLiestErweiterungMitPlatzhaltern()
        {
            SeedStandard();
            var doc = Doc2024();
            doc.Erweiterungen.Setze("Elster", "UST-Zahlbetrag-2024-01", "150,00");
            doc.Erweiterungen.Setze("Allgemein", "Notiz2024", "Hallo");

            var def = Formular("",
                Feld(1, "Dokumentdaten", "[Elster]UST-Zahlbetrag-$J-01"),
                Feld(2, "Dokumentdaten", "Notiz$J"),
                Feld(3, "Dokumentdaten", "[Elster]Gibtsnicht"));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal("150,00", WertVon(werte, 1).Text);
            Assert.Equal(15000, WertVon(werte, 1).WertCent);
            Assert.Equal("Hallo", WertVon(werte, 2).Text);   // Default-Sektion Allgemein
            Assert.Equal("", WertVon(werte, 3).Text);
        }

        [Fact]
        public void FreitextErsetztPlatzhalter()
        {
            SeedStandard();
            var doc = Doc2024();
            var def = Formular("voranmeldungszeitraum=\"13\"",
                Feld(1, "Freitext", "Kalenderjahr $J, Zeitraum $v"));
            var werte = FormularRechner.Berechne(doc, def, "");

            Assert.Equal("Kalenderjahr 2024, Zeitraum 13", WertVon(werte, 1).Text);
        }

        [Fact]
        public void UnbekannterFeldtypLiefertLeer()
        {
            SeedStandard();
            var doc = Doc2024();
            var def = Formular("", Feld(1, "NeuerTyp", "egal"));
            var werte = FormularRechner.Berechne(doc, def, "");
            Assert.Equal("", WertVon(werte, 1).Text);
        }

        // ------------------------------------------------------------------
        // currency_to_int-Port
        // ------------------------------------------------------------------

        [Fact]
        public void CurrencyToCentSemantik()
        {
            var nf = CultureInfo.CurrentCulture.NumberFormat;
            string dez = nf.NumberDecimalSeparator.Substring(0, 1);
            string grp = nf.NumberGroupSeparator.Substring(0, 1);

            Assert.Equal(0, FormularRechner.CurrencyToCent(""));
            Assert.Equal(0, FormularRechner.CurrencyToCent(null));
            Assert.Equal(0, FormularRechner.CurrencyToCent("abc"));
            Assert.Equal(15000, FormularRechner.CurrencyToCent("150" + dez + "00"));
            Assert.Equal(15050, FormularRechner.CurrencyToCent("150" + dez + "5"));
            Assert.Equal(123456, FormularRechner.CurrencyToCent("1" + grp + "234" + dez + "56"));
            Assert.Equal(15000, FormularRechner.CurrencyToCent("150"));       // Euro -> Cent
            Assert.Equal(-30000, FormularRechner.CurrencyToCent("-300"));
            // Parsen bricht am ersten Fremdzeichen ab
            Assert.Equal(15000, FormularRechner.CurrencyToCent("150 EUR"));
        }
    }
}
