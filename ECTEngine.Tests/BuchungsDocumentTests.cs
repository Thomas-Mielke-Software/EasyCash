// BuchungsDocumentTests.cs — Tests für das zentrale Buchungsdokument

using System;
using System.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    public class BuchungsDocumentTests
    {
        private BuchungsDocument ErstelleTestdokument()
        {
            var doc = new BuchungsDocument { Jahr = 2024 };

            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = new Betrag(1190.00m, 19m),
                Datum = new DateTime(2024, 3, 15),
                Beschreibung = "Beratungshonorar März",
                Konto = "Erlöse"
            });

            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = new Betrag(595.00m, 19m),
                Datum = new DateTime(2024, 6, 1),
                Beschreibung = "Beratungshonorar Juni",
                Konto = "Erlöse"
            });

            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = new Betrag(119.00m, 19m),
                Datum = new DateTime(2024, 3, 20),
                Beschreibung = "Büromaterial",
                Konto = "Bürobedarf"
            });

            return doc;
        }

        // ──────────────────────────────────────────────
        // Summenberechnung
        // ──────────────────────────────────────────────

        [Fact]
        public void EinnahmenSumme_AlleMonat()
        {
            var doc = ErstelleTestdokument();
            // 1190.00 + 595.00 = 1785.00 EUR = 178500 Cent
            Assert.Equal(178500, doc.EinnahmenSumme());
        }

        [Fact]
        public void EinnahmenSumme_GefiltrtNachMonat()
        {
            var doc = ErstelleTestdokument();
            // Nur März: 1190.00 EUR = 119000 Cent
            Assert.Equal(119000, doc.EinnahmenSumme(monatsFilter: 3));
        }

        [Fact]
        public void AusgabenSumme()
        {
            var doc = ErstelleTestdokument();
            Assert.Equal(11900, doc.AusgabenSumme());
        }

        [Fact]
        public void Saldo()
        {
            var doc = ErstelleTestdokument();
            // 178500 - 11900 = 166600 Cent = 1666.00 EUR
            Assert.Equal(166600, doc.Saldo());
        }

        [Fact]
        public void EinnahmenSummeNetto()
        {
            var doc = ErstelleTestdokument();
            // 1000.00 + 500.00 = 1500.00 EUR netto = 150000 Cent
            Assert.Equal(150000, doc.EinnahmenSummeNetto());
        }

        [Fact]
        public void EinnahmenSumme_MitKontoFilter()
        {
            var doc = ErstelleTestdokument();
            Assert.Equal(178500, doc.EinnahmenSumme(kontoFilter: "Erlöse"));
            Assert.Equal(0, doc.EinnahmenSumme(kontoFilter: "NichtExistent"));
        }

        // ──────────────────────────────────────────────
        // Sortierung
        // ──────────────────────────────────────────────

        [Fact]
        public void Sort_SortiertNachDatum()
        {
            var doc = ErstelleTestdokument();

            // Buchung vor den anderen hinzufügen
            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = new Betrag(100m),
                Datum = new DateTime(2024, 1, 1),
                Beschreibung = "Erste Buchung"
            });

            doc.Sort();

            Assert.Equal(new DateTime(2024, 1, 1), doc.Buchungen.First().Datum);
            Assert.Equal(new DateTime(2024, 6, 1), doc.Buchungen.Last().Datum);
        }

        // ──────────────────────────────────────────────
        // Buchungsart-Erkennung
        // ──────────────────────────────────────────────

        [Fact]
        public void BuchungIstEinnahme_Korrekt()
        {
            var doc = ErstelleTestdokument();
            var einnahme = doc.Buchungen.First(b => b.Art == Buchungsart.Einnahme);
            var ausgabe = doc.Buchungen.First(b => b.Art == Buchungsart.Ausgabe);

            Assert.True(doc.BuchungIstEinnahme(einnahme));
            Assert.False(doc.BuchungIstEinnahme(ausgabe));
        }
    }

    public class DauerbuchungTests
    {
        // ──────────────────────────────────────────────
        // Platzhalter
        // ──────────────────────────────────────────────

        [Theory]
        [InlineData("Miete $M/$J", "2024-03-15", "Miete 03/2024")]
        [InlineData("Q$q $J", "2024-07-01", "Q3 2024")]
        [InlineData("HJ$h", "2024-03-01", "HJ1")]
        [InlineData("HJ$h", "2024-09-01", "HJ2")]
        [InlineData("$j-$M", "2024-01-15", "24-01")]
        [InlineData("Zeitraum $2", "2024-05-01", "Zeitraum 3")]
        public void ResolvePlatzhalter_KorrekteErsetzung(
            string template, string datumStr, string erwartet)
        {
            var datum = DateTime.Parse(datumStr);
            Assert.Equal(erwartet, Dauerbuchung.ResolvePlatzhalter(template, datum));
        }

        [Fact]
        public void ResolvePlatzhalter_LeerString()
        {
            Assert.Equal("", Dauerbuchung.ResolvePlatzhalter("", DateTime.Now));
        }

        // ──────────────────────────────────────────────
        // Buchungserzeugung
        // ──────────────────────────────────────────────

        [Fact]
        public void ErzeugeBuchung_UeberniemmtFelderKorrekt()
        {
            var db = new Dauerbuchung
            {
                BruttoBetrag = new Betrag(500m, 19m),
                Art = Buchungsart.Ausgabe,
                Beschreibung = "Miete $M/$J",
                Konto = "Miete",
                Belegnummer = "M-$M",
                Betrieb = "Hauptbetrieb"
            };

            var buchung = db.ErzeugeBuchung(new DateTime(2024, 3, 15));

            Assert.Equal(500m, buchung.BruttoBetrag.BruttoWert);
            Assert.Equal(19m, buchung.BruttoBetrag.MwstProzent);
            Assert.Equal(Buchungsart.Ausgabe, buchung.Art);
            Assert.Equal("Miete 03/2024", buchung.Beschreibung);
            Assert.Equal("M-03", buchung.Belegnummer);
            Assert.Equal("Miete", buchung.Konto);
            Assert.Equal("Hauptbetrieb", buchung.Betrieb);
        }

        // ──────────────────────────────────────────────
        // Fällige Termine
        // ──────────────────────────────────────────────

        [Fact]
        public void FaelligeTermine_Monatlich()
        {
            var db = new Dauerbuchung
            {
                Intervall = Intervall.Monatlich,
                VonDatum = new DateTime(2024, 1, 1),
                BisDatum = new DateTime(2024, 12, 31),
                Buchungstag = 1,
                AusgefuehrtBis = new DateTime(2024, 3, 1)
            };

            var termine = db.FaelligeTermine(new DateTime(2024, 6, 30)).ToList();

            Assert.Equal(3, termine.Count); // April, Mai, Juni
            Assert.Equal(new DateTime(2024, 4, 1), termine[0]);
            Assert.Equal(new DateTime(2024, 5, 1), termine[1]);
            Assert.Equal(new DateTime(2024, 6, 1), termine[2]);
        }

        [Fact]
        public void FaelligeTermine_Quartalsweise()
        {
            var db = new Dauerbuchung
            {
                Intervall = Intervall.Quartalsweise,
                VonDatum = new DateTime(2024, 1, 1),
                BisDatum = new DateTime(2024, 12, 31),
                Buchungstag = 15,
                AusgefuehrtBis = new DateTime(2024, 1, 15)
            };

            var termine = db.FaelligeTermine(new DateTime(2024, 12, 31)).ToList();

            Assert.Equal(3, termine.Count); // Apr, Jul, Okt
            Assert.Equal(new DateTime(2024, 4, 15), termine[0]);
            Assert.Equal(new DateTime(2024, 7, 15), termine[1]);
            Assert.Equal(new DateTime(2024, 10, 15), termine[2]);
        }
    }

    public class JahreswechselTests
    {
        [Fact]
        public void Jahreswechsel_UebernimmtAfABuchungen()
        {
            var doc = new BuchungsDocument { Jahr = 2024 };

            doc.Buchungen.Add(new Buchung
            {
                Art = Buchungsart.Ausgabe,
                BruttoBetrag = Betrag.AusCent(119000, 19000), // 1190€ brutto
                Datum = new DateTime(2024, 7, 1),
                Beschreibung = "Laptop",
                AfaJahre = 3,
                AfaNr = 1,
                AfaRestwertCent = 100000, // 1000€ netto
                AfaGenauigkeit = AfaGenauigkeit.Ganzjahr,
                Belegnummer = "A-001"
            });

            var neuesDoc = doc.Jahreswechsel();

            Assert.Equal(2025, neuesDoc.Jahr);
            Assert.Single(neuesDoc.Buchungen);

            var kopie = neuesDoc.Buchungen[0];
            Assert.Equal(2, kopie.AfaNr);
            Assert.Equal("A-001/2024", kopie.Belegnummer);
            Assert.Equal(2025, kopie.Datum.Year);
            Assert.True(kopie.AfaRestwertCent < 100000);
        }

        [Fact]
        public void Jahreswechsel_UebernimmtDauerbuchungen()
        {
            var doc = new BuchungsDocument { Jahr = 2024 };

            doc.Dauerbuchungen.Add(new Dauerbuchung
            {
                BruttoBetrag = new Betrag(500m, 19m),
                Art = Buchungsart.Ausgabe,
                Beschreibung = "Miete",
                Intervall = Intervall.Monatlich,
                VonDatum = new DateTime(2024, 1, 1),
                BisDatum = new DateTime(2025, 12, 31),
                Buchungstag = 1
            });

            var neuesDoc = doc.Jahreswechsel();

            Assert.Single(neuesDoc.Dauerbuchungen);
            Assert.Equal("Miete", neuesDoc.Dauerbuchungen[0].Beschreibung);
        }

        [Fact]
        public void Jahreswechsel_KopieIstUnabhaengig()
        {
            var doc = new BuchungsDocument { Jahr = 2024, Waehrung = "EUR" };
            var neuesDoc = doc.Jahreswechsel();

            neuesDoc.Waehrung = "USD";
            Assert.Equal("EUR", doc.Waehrung);
        }
    }
}
