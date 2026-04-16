// BetragTests.cs — Tests für die Betrag-Struct
//
// Testet insbesondere die Kompatibilität mit CBetrag::GetNetto()

using Xunit;

namespace ECTEngine.Tests
{
    public class BetragTests
    {
        // ──────────────────────────────────────────────
        // Grundlegende Konstruktion
        // ──────────────────────────────────────────────

        [Fact]
        public void Null_HatWertNull()
        {
            var b = Betrag.Null;
            Assert.Equal(0m, b.BruttoWert);
            Assert.Equal(0, b.InCent);
        }

        [Fact]
        public void AusCent_KonvertiertKorrekt()
        {
            var b = Betrag.AusCent(1999, 19000);
            Assert.Equal(19.99m, b.BruttoWert);
            Assert.Equal(19.0m, b.MwstProzent);
        }

        [Fact]
        public void InCent_RundungKorrekt()
        {
            var b = new Betrag(19.99m);
            Assert.Equal(1999, b.InCent);
        }

        [Fact]
        public void MwstPromille_Konvertierung()
        {
            var b = new Betrag(100m, 19m);
            Assert.Equal(19000, b.MwstPromille);

            var b2 = new Betrag(100m, 7m);
            Assert.Equal(7000, b2.MwstPromille);
        }

        // ──────────────────────────────────────────────
        // GetNetto-Kompatibilität
        // Diese Tests verifizieren, dass die decimal-basierte
        // Berechnung identische Ergebnisse liefert wie
        // CBetrag::GetNetto() im C++-Original.
        // ──────────────────────────────────────────────

        [Theory]
        [InlineData(11900, 19000, 10000)]   // 119,00€ bei 19% → 100,00€
        [InlineData(10700, 7000, 10000)]    // 107,00€ bei 7%  → 100,00€
        [InlineData(100, 19000, 84)]        // 1,00€ bei 19%   → 0,84€
        [InlineData(1, 19000, 1)]           // 0,01€ bei 19%   → 0,01€ (Rundung!)
        [InlineData(0, 19000, 0)]           // 0€ → 0€
        [InlineData(-11900, 19000, -10000)] // negativ
        [InlineData(23800, 19000, 20000)]   // 238,00€ bei 19% → 200,00€
        public void NettoInCent_KompatibilMitGetNetto_KleineBetraege(
            int bruttoInCent, int mwstPromille, long erwartetNettoCent)
        {
            var b = Betrag.AusCent(bruttoInCent, mwstPromille);
            Assert.Equal(erwartetNettoCent, b.NettoInCent);
        }

        [Fact]
        public void NettoInCent_OhneMWSt_IstBrutto()
        {
            var b = Betrag.AusCent(5000, 0);
            Assert.Equal(5000, b.NettoInCent);
        }

        [Fact]
        public void MwstBetrag_Korrekt()
        {
            var b = new Betrag(119.00m, 19m);
            Assert.Equal(19.00m, b.MwstBetrag);
        }

        // ──────────────────────────────────────────────
        // Arithmetik
        // ──────────────────────────────────────────────

        [Fact]
        public void Addition()
        {
            var a = new Betrag(100m, 19m);
            var b = new Betrag(50m, 19m);
            var c = a + b;
            Assert.Equal(150m, c.BruttoWert);
        }

        [Fact]
        public void Subtraktion()
        {
            var a = new Betrag(100m, 19m);
            var b = new Betrag(50m, 19m);
            var c = a - b;
            Assert.Equal(50m, c.BruttoWert);
        }

        [Fact]
        public void Negation()
        {
            var a = new Betrag(100m);
            var b = -a;
            Assert.Equal(-100m, b.BruttoWert);
        }

        [Fact]
        public void Multiplikation()
        {
            var a = new Betrag(100m);
            var b = a * 0.5m;
            Assert.Equal(50m, b.BruttoWert);
        }

        // ──────────────────────────────────────────────
        // Formatierung
        // ──────────────────────────────────────────────

        [Fact]
        public void AlsWaehrung_DeutschesFormat()
        {
            var b = new Betrag(1234.56m);
            Assert.Equal("1.234,56", b.AlsWaehrung());
        }

        // ──────────────────────────────────────────────
        // Gleichheit
        // ──────────────────────────────────────────────

        [Fact]
        public void Equals_GleicheWerte()
        {
            var a = new Betrag(100m, 19m);
            var b = new Betrag(100m, 19m);
            Assert.Equal(a, b);
            Assert.True(a == b);
        }

        [Fact]
        public void Equals_VerschiedeneWerte()
        {
            var a = new Betrag(100m, 19m);
            var b = new Betrag(100m, 7m);
            Assert.NotEqual(a, b);
            Assert.True(a != b);
        }
    }
}
