// BerichtTests.cs — Tests für die formlosen Berichte (WYSIWYG-Druck):
// BerichtZeitraum (Monatsfilter -> Datumsbereich), EuerBericht
// (Posten-Summierung mit/ohne USt-VSt-Ausweis, neutrale Konten, AfA,
// Betrieb-Filter, Steuernummer-Fallback), UstErklaerungBericht
// (Satz-Gruppierung in Promille, Sonderkonten, Vorauszahlungen,
// Anlage UR) und KontenplanBericht.

using System;
using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    // ======================================================================
    // BerichtZeitraum — Monatsfilter -> von/bis
    // ======================================================================

    public class BerichtZeitraumTests
    {
        [Fact]
        public void GanzesJahr()
        {
            BerichtZeitraum.AusMonatsfilter(2024, 0, out var von, out var bis);
            Assert.Equal(new DateTime(2024, 1, 1), von.Date);
            Assert.Equal(new DateTime(2024, 12, 31), bis.Date);
            Assert.True(BerichtZeitraum.IstGanzesJahr(0));
        }

        [Fact]
        public void EinzelnerMonat_Februar_Schaltjahr()
        {
            BerichtZeitraum.AusMonatsfilter(2024, 2, out var von, out var bis);
            Assert.Equal(new DateTime(2024, 2, 1), von.Date);
            Assert.Equal(new DateTime(2024, 2, 29), bis.Date);
            Assert.False(BerichtZeitraum.IstGanzesJahr(2));
        }

        [Theory]
        [InlineData(13, 1, 3)]
        [InlineData(14, 4, 6)]
        [InlineData(15, 7, 9)]
        [InlineData(16, 10, 12)]
        public void Quartale(int filter, int vonMonat, int bisMonat)
        {
            BerichtZeitraum.AusMonatsfilter(2024, filter, out var von, out var bis);
            Assert.Equal(vonMonat, von.Month);
            Assert.Equal(bisMonat, bis.Month);
            Assert.Equal(DateTime.DaysInMonth(2024, bisMonat), bis.Day);
        }

        [Fact]
        public void BisEnthaeltDenLetztenTag()
        {
            // 23:59:59 — eine Buchung am 31.03. faellt noch ins 1. Quartal
            BerichtZeitraum.AusMonatsfilter(2024, 13, out _, out var bis);
            Assert.True(new DateTime(2024, 3, 31, 12, 0, 0) <= bis);
        }
    }

    // ======================================================================
    // EuerBericht
    // ======================================================================

    [Collection("EinstellungenCache")]   // statischer Einstellungs-Cache
    public class EuerBerichtTests
    {
        private static string Geld(decimal euro) => Waehrungsformat.Betrag(euro);

        private static BuchungsDocument NeuesDoc()
        {
            var doc = new BuchungsDocument { Jahr = 2024, Waehrung = "EUR" };
            return doc;
        }

        private static void SeedKonten(params (string key, string val)[] extra)
        {
            var dict = new Dictionary<string, string>
            {
                ["e00"] = "Erlöse",
                ["a00"] = "Bürobedarf",
                ["fsteuernummer"] = "11/222/33333"
            };
            foreach (var (key, val) in extra) dict[key] = val;
            Einstellungen.LadeAusBridge(dict);
        }

        private static BerichtZeile FindeZeile(Bericht b, string text) =>
            b.Zeilen.FirstOrDefault(z => z.Text == text);

        [Fact]
        public void BruttoModus_SummenUndGewinn()
        {
            SeedKonten();   // ustvst_gesondert nicht gesetzt = 0
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(119000, 19000),
                Datum = new DateTime(2024, 3, 15),
                Konto = "Erlöse"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = Betrag.AusCent(23800, 19000),
                Datum = new DateTime(2024, 4, 2),
                Konto = "Bürobedarf"
            });

            var bericht = EuerBericht.Erzeuge(doc, 0, "");

            // Brutto-Ausweis: Posten mit Bruttobetrag
            Assert.Equal(Geld(1190m) + " EUR", FindeZeile(bericht, "Erlöse").Werte[0]);
            Assert.Equal(Geld(238m) + " EUR", FindeZeile(bericht, "Bürobedarf").Werte[0]);

            // Einnahmen-Titel traegt den MwSt-Hinweis
            Assert.Contains(bericht.Zeilen, z =>
                z.Text == "1. Betriebseinnahmen (einschl. Umsatzsteuer)");

            // Gewinn = 1190 - 238
            Assert.Equal(Geld(952m) + " EUR",
                FindeZeile(bericht, "Gewinn/Verlust").Werte[0]);

            // Kein synthetischer UST/VST-Posten
            Assert.Null(FindeZeile(bericht, "UST"));
            Assert.Null(FindeZeile(bericht, "VST"));

            // Steuernummer aus dem Finanzamt (kein Betrieb-Filter)
            Assert.Contains("11/222/33333", bericht.KopfRechts);
        }

        [Fact]
        public void GesondertModus_NettoPlusSammelposten()
        {
            SeedKonten(("[Allgemein]ustvst_gesondert", "1"));
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(119000, 19000),
                Datum = new DateTime(2024, 3, 15),
                Konto = "Erlöse"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = Betrag.AusCent(23800, 19000),
                Datum = new DateTime(2024, 4, 2),
                Konto = "Bürobedarf"
            });

            var bericht = EuerBericht.Erzeuge(doc, 0, "");

            // Netto-Ausweis + synthetische Sammelposten
            Assert.Equal(Geld(1000m) + " EUR", FindeZeile(bericht, "Erlöse").Werte[0]);
            Assert.Equal(Geld(190m) + " EUR", FindeZeile(bericht, "UST").Werte[0]);
            Assert.Equal(Geld(200m) + " EUR", FindeZeile(bericht, "Bürobedarf").Werte[0]);
            Assert.Equal(Geld(38m) + " EUR", FindeZeile(bericht, "VST").Werte[0]);

            // Titel OHNE "(einschl. Umsatzsteuer)"
            Assert.Contains(bericht.Zeilen, z => z.Text == "1. Betriebseinnahmen");

            // Gesamtsummen bleiben brutto -> Gewinn unveraendert
            Assert.Equal(Geld(952m) + " EUR",
                FindeZeile(bericht, "Gewinn/Verlust").Werte[0]);
        }

        [Fact]
        public void NeutralesKonto_NurMwstAnteil()
        {
            SeedKonten();
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(119000, 19000),
                Datum = new DateTime(2024, 5, 1),
                Konto = "/Durchlaufende Posten"
            });

            var bericht = EuerBericht.Erzeuge(doc, 0, "");

            // Neutrale Konten: nur der MwSt-Anteil, mit "(nur UST)"-Suffix
            var zeile = FindeZeile(bericht, "/Durchlaufende Posten (nur UST)");
            Assert.NotNull(zeile);
            Assert.Equal(Geld(190m) + " EUR", zeile.Werte[0]);
            Assert.Equal(Geld(190m) + " EUR", FindeZeile(bericht, "Summe").Werte[0]);
        }

        [Fact]
        public void AfaFolgejahr_KeineVorsteuer()
        {
            SeedKonten(("[Allgemein]ustvst_gesondert", "1"));
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = Betrag.AusCent(119000, 19000),
                Datum = new DateTime(2024, 1, 10),
                Konto = "Bürobedarf",
                AfaJahre = 3,
                AfaNr = 2   // Folgejahr
            });

            var bericht = EuerBericht.Erzeuge(doc, 0, "");

            // Im AfA-Folgejahr faellt keine Vorsteuer mehr an -> der
            // VST-Sammelposten bleibt 0 und wird gar nicht ausgegeben
            Assert.Null(FindeZeile(bericht, "VST"));
        }

        [Fact]
        public void MonatsfilterBegrenztDenZeitraum()
        {
            SeedKonten();
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(10000, 0),
                Datum = new DateTime(2024, 3, 15),
                Konto = "Erlöse"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(50000, 0),
                Datum = new DateTime(2024, 7, 15),
                Konto = "Erlöse"
            });

            var bericht = EuerBericht.Erzeuge(doc, 3, "");   // nur Maerz

            Assert.Equal(Geld(100m) + " EUR", FindeZeile(bericht, "Erlöse").Werte[0]);
        }

        [Fact]
        public void BetriebFilter_MitBetriebsSteuernummer()
        {
            SeedKonten(
                ("[Betriebe]Betrieb00Name", "Werkstatt"),
                ("[Betriebe]Betrieb00Unternehmensart",
                    "Kfz-Werkstatt\tEinzelunternehmen\t99/888/77777\t"));
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(10000, 0),
                Datum = new DateTime(2024, 2, 1),
                Konto = "Erlöse",
                Betrieb = "Werkstatt"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(70000, 0),
                Datum = new DateTime(2024, 2, 2),
                Konto = "Erlöse",
                Betrieb = "Laden"
            });

            var bericht = EuerBericht.Erzeuge(doc, 0, "Werkstatt");

            Assert.Equal(Geld(100m) + " EUR", FindeZeile(bericht, "Erlöse").Werte[0]);
            Assert.Contains("Werkstatt", bericht.Titel);
            Assert.Contains("99/888/77777", bericht.KopfRechts);
        }

        [Fact]
        public void BuchungOhneKonto_PostenPlusHinweis()
        {
            SeedKonten();
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(5000, 0),
                Datum = new DateTime(2024, 6, 1),
                Konto = ""
            });

            var bericht = EuerBericht.Erzeuge(doc, 0, "");

            Assert.NotNull(FindeZeile(bericht, "(ohne Konto)"));
            Assert.Contains(bericht.Zeilen, z =>
                z.Stil == BerichtStil.Hinweis
                && z.Text.Contains("ohne Konto-Zuweisung"));
        }

        [Fact]
        public void UnbekanntesKontoWirdHintenAngehaengt()
        {
            SeedKonten();
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(4200, 0),
                Datum = new DateTime(2024, 6, 1),
                Konto = "Sonstiges (nicht in den Einstellungen)"
            });

            var bericht = EuerBericht.Erzeuge(doc, 0, "");

            var zeile = FindeZeile(bericht, "Sonstiges (nicht in den Einstellungen)");
            Assert.NotNull(zeile);
            Assert.Equal(Geld(42m) + " EUR", zeile.Werte[0]);
        }
    }

    // ======================================================================
    // UstErklaerungBericht
    // ======================================================================

    [Collection("EinstellungenCache")]
    public class UstErklaerungBerichtTests
    {
        private static string Geld(decimal euro) => Waehrungsformat.Betrag(euro);

        private static BuchungsDocument NeuesDoc()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            return new BuchungsDocument { Jahr = 2024, Waehrung = "EUR" };
        }

        [Fact]
        public void SaetzeGruppiertUndAbsteigend()
        {
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(119000, 19000),
                Datum = new DateTime(2024, 1, 5),
                Konto = "Erlöse 19"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(10700, 7000),
                Datum = new DateTime(2024, 2, 5),
                Konto = "Erlöse 7"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(21400, 7000),
                Datum = new DateTime(2024, 3, 5),
                Konto = "Erlöse 7"
            });

            var bericht = UstErklaerungBericht.Erzeuge(doc, 0);

            var z19 = bericht.Zeilen.First(z => z.Text == "19%");
            Assert.Equal(Geld(1000m), z19.Werte[0]);   // Netto
            Assert.Equal(Geld(190m), z19.Werte[1]);    // USt
            Assert.Equal(Geld(1190m), z19.Werte[2]);   // Brutto

            var z7 = bericht.Zeilen.First(z => z.Text == "7%");
            Assert.Equal(Geld(300m), z7.Werte[0]);
            Assert.Equal(Geld(21m), z7.Werte[1]);

            // 19 % vor 7 % (absteigend)
            Assert.True(bericht.Zeilen.IndexOf(z19) < bericht.Zeilen.IndexOf(z7));

            var gesamt = bericht.Zeilen.First(z => z.Text == "Gesamt:");
            Assert.Equal(Geld(1300m), gesamt.Werte[0]);
            Assert.Equal(Geld(211m), gesamt.Werte[1]);
            Assert.Equal(Geld(1511m), gesamt.Werte[2]);
        }

        [Fact]
        public void VorsteuerMitSeparatUndAfaRegeln()
        {
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = Betrag.AusCent(23800, 19000),   // 38 EUR VSt
                Datum = new DateTime(2024, 4, 1),
                Konto = "Bürobedarf"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = Betrag.AusCent(1200, 0),        // brutto = VSt
                Datum = new DateTime(2024, 5, 1),
                Konto = "VST-Beträge separat"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = Betrag.AusCent(119000, 19000),
                Datum = new DateTime(2024, 6, 1),
                Konto = "Anlagen",
                AfaJahre = 3,
                AfaNr = 2   // Folgejahr -> keine VSt
            });

            var bericht = UstErklaerungBericht.Erzeuge(doc, 0);

            var vstZeile = bericht.Zeilen.First(
                z => z.Text == "aus Rechnungen von anderen Unternehmen:");
            Assert.Equal(Geld(50m), vstZeile.Werte[0]);   // 38 + 12

            // Verbleibender Betrag = 0 (USt) - 50 (VSt) - 0 (EUSt)
            var verbleibend = bericht.Zeilen.First(
                z => z.Text == "Verbleibender Betrag:");
            Assert.Equal(Geld(-50m), verbleibend.Werte[0]);
        }

        [Fact]
        public void SonderkontenUndAnlageUR()
        {
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(50000, 0),
                Datum = new DateTime(2024, 1, 1),
                Konto = "Steuerfreie Umsätze"
            });
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(30050, 0),
                Datum = new DateTime(2024, 2, 1),
                Konto = "Export UST-ID Frankreich"
            });

            var bericht = UstErklaerungBericht.Erzeuge(doc, 0);

            Assert.Contains(bericht.Zeilen, z => z.Text == "Anlage UR:");
            var ustId = bericht.Zeilen.First(z =>
                z.Text.StartsWith("Innergemeinschaftliche Lieferungen"));
            Assert.Equal("300", ustId.Werte[0]);   // volle Euro
            var frei = bericht.Zeilen.First(z =>
                z.Text == "Steuerfreie Umsätze ohne Vorsteuerabzug:");
            Assert.Equal("500", frei.Werte[0]);
        }

        [Fact]
        public void KeineAnlageUROhneSonderumsaetze()
        {
            var doc = NeuesDoc();
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(11900, 19000),
                Datum = new DateTime(2024, 1, 1),
                Konto = "Erlöse"
            });

            var bericht = UstErklaerungBericht.Erzeuge(doc, 0);

            Assert.DoesNotContain(bericht.Zeilen, z => z.Text == "Anlage UR:");
        }

        [Fact]
        public void GemerkteVorauszahlungenMonatUndQuartal()
        {
            var doc = NeuesDoc();
            doc.Erweiterungen.Setze("Elster", "UST-Zahlbetrag-2024-02", "100,00");
            doc.Erweiterungen.Setze("Elster", "UST-Zahlbetrag-2024-41", "250,00");
            doc.Erweiterungen.Setze("Elster", "UST-Zahlbetrag-2024-07", "-30,00");

            var bericht = UstErklaerungBericht.Erzeuge(doc, 0);

            Assert.Contains(bericht.Zeilen, z =>
                z.Text == "Vorauszahlung für Monat 2:" && z.Werte[0] == Geld(100m));
            Assert.Contains(bericht.Zeilen, z =>
                z.Text == "Vorauszahlung für Quartal 1:" && z.Werte[0] == Geld(250m));
            Assert.Contains(bericht.Zeilen, z =>
                z.Text == "Erstattung für Monat 7:" && z.Werte[0] == Geld(-30m));

            var summe = bericht.Zeilen.First(z =>
                z.Text == "Summe aller gemerkten Vorauszahlungen:");
            Assert.Equal(Geld(320m), summe.Werte[0]);
        }

        [Fact]
        public void QuartalsfilterBegrenztVorauszahlungen()
        {
            var doc = NeuesDoc();
            doc.Erweiterungen.Setze("Elster", "UST-Zahlbetrag-2024-02", "100,00");
            doc.Erweiterungen.Setze("Elster", "UST-Zahlbetrag-2024-07", "999,00");

            var bericht = UstErklaerungBericht.Erzeuge(doc, 13);   // Q1

            Assert.Contains(bericht.Zeilen,
                z => z.Text == "Vorauszahlung für Monat 2:");
            Assert.DoesNotContain(bericht.Zeilen,
                z => z.Text == "Vorauszahlung für Monat 7:");
            Assert.StartsWith("Vorläufige USt-Erklärung", bericht.Titel);
        }
    }

    // ======================================================================
    // KontenplanBericht
    // ======================================================================

    [Collection("EinstellungenCache")]
    public class KontenplanBerichtTests
    {
        [Fact]
        public void EinfacherKontenplanMitUnterkategorie()
        {
            var fz = new ErweiterungStore();
            fz.Setze("ECT", "E/Ü-Rechnung", "1103");

            Einstellungen.LadeAusBridge(new Dictionary<string, string>
            {
                ["e00"] = "Honorar",
                ["e01"] = "Zinsen",
                ["a00"] = "Bürobedarf",
                ["[EinnahmenUnterkategorien]00"] = "Dienstleistungen",
                ["[EinnahmenFeldzuweisungen]00"] = fz.ZuPipeFormat()
            });

            var bericht = KontenplanBericht.Erzeuge(mitFeldern: false);

            Assert.Equal("Kontenplan", bericht.Titel);
            Assert.Contains(bericht.Zeilen, z =>
                z.Text == "Einnahmen-Konten" && z.Stil == BerichtStil.Ueberschrift);
            Assert.Contains(bericht.Zeilen, z =>
                z.Text == "Dienstleistungen" && z.Stil == BerichtStil.Zwischentitel);
            Assert.Contains(bericht.Zeilen, z => z.Text == "Honorar");
            Assert.Contains(bericht.Zeilen, z => z.Text == "Zinsen");
            Assert.Contains(bericht.Zeilen, z => z.Text == "Bürobedarf");
            // Ohne mitFeldern keine Feld-Zeilen
            Assert.DoesNotContain(bericht.Zeilen, z => z.Text.Contains("Feld 1103"));
        }

        [Fact]
        public void KontenplanMitFeldverknuepfungen()
        {
            var fz = new ErweiterungStore();
            fz.Setze("ECT", "E/Ü-Rechnung", "1103");

            Einstellungen.LadeAusBridge(new Dictionary<string, string>
            {
                ["e00"] = "Honorar",
                ["[EinnahmenFeldzuweisungen]00"] = fz.ZuPipeFormat()
            });

            var bericht = KontenplanBericht.Erzeuge(mitFeldern: true);

            Assert.Contains("Feldverknüpfungen", bericht.Titel);
            // Bezeichnung fehlt (keine .ecf im Testlauf) -> nur Formular + Id
            Assert.Contains(bericht.Zeilen, z =>
                z.Text == "E/Ü-Rechnung: Feld 1103" && z.Einrueckung == 2);
        }
    }
}
