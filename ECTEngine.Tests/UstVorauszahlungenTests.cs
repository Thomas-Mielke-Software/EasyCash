// UstVorauszahlungenTests.cs — Tests für den UstVorauszahlungen-Helfer
// (USt-Zahlbeträge + Sondervorauszahlung im Dokument-ErweiterungStore).

using Xunit;

namespace ECTEngine.Tests
{
    public class UstVorauszahlungenTests
    {
        private static BuchungsDocument Doc() =>
            new BuchungsDocument { Jahr = 2024 };

        [Fact]
        public void SetzeUndHoleZahlbetrag()
        {
            var doc = Doc();
            Assert.Equal("", UstVorauszahlungen.Hole(doc, 1));
            Assert.Null(UstVorauszahlungen.HoleCent(doc, 1));

            Assert.True(UstVorauszahlungen.Setze(doc, 1, "150"));
            Assert.Equal(Waehrungsformat.BetragOhneGruppierung(150m),
                UstVorauszahlungen.Hole(doc, 1));
            Assert.Equal(15000, UstVorauszahlungen.HoleCent(doc, 1));

            // unveraendert schreiben -> keine Aenderung gemeldet
            Assert.False(UstVorauszahlungen.Setze(doc, 1,
                Waehrungsformat.BetragOhneGruppierung(150m)));

            // Schluesselformat wie nativ (Monat 01)
            Assert.NotEqual("",
                doc.Erweiterungen.Hole("Elster", "UST-Zahlbetrag-2024-01", ""));
        }

        [Fact]
        public void QuartaleNutzenZeitraum41Bis44()
        {
            var doc = Doc();
            Assert.Equal(41, UstVorauszahlungen.QuartalZeitraum(1));
            Assert.Equal(44, UstVorauszahlungen.QuartalZeitraum(4));

            UstVorauszahlungen.Setze(doc, 42,
                Waehrungsformat.BetragOhneGruppierung(99.50m));
            Assert.Equal(9950, UstVorauszahlungen.HoleCent(doc, 42));
            Assert.NotEqual("",
                doc.Erweiterungen.Hole("Elster", "UST-Zahlbetrag-2024-42", ""));
        }

        [Fact]
        public void NullOderUnlesbarLoeschtDenWert()
        {
            var doc = Doc();
            UstVorauszahlungen.Setze(doc, 3, "77");
            Assert.True(UstVorauszahlungen.Setze(doc, 3, "0"));
            Assert.Equal("", UstVorauszahlungen.Hole(doc, 3));

            UstVorauszahlungen.Setze(doc, 3, "77");
            Assert.True(UstVorauszahlungen.Setze(doc, 3, "abc"));
            Assert.Equal("", UstVorauszahlungen.Hole(doc, 3));
        }

        [Fact]
        public void SondervorauszahlungRohUndProJahr()
        {
            var doc = Doc();
            Assert.Equal("", UstVorauszahlungen.HoleSondervorauszahlung(doc));

            Assert.True(UstVorauszahlungen.SetzeSondervorauszahlung(doc, "1234,50"));
            Assert.Equal("1234,50", UstVorauszahlungen.HoleSondervorauszahlung(doc));
            Assert.False(UstVorauszahlungen.SetzeSondervorauszahlung(doc, "1234,50"));

            // Schluessel wie nativ: Sektion Dauerfristverlängerung + Jahr
            Assert.Equal("1234,50", doc.Erweiterungen.Hole(
                "Dauerfristverlängerung", "Sondervorauszahlung2024", ""));

            // anderes Buchungsjahr = anderer Schluessel
            doc.Jahr = 2025;
            Assert.Equal("", UstVorauszahlungen.HoleSondervorauszahlung(doc));
        }
    }
}
