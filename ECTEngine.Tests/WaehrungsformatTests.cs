// WaehrungsformatTests.cs — Tests für den toleranten, locale-freien
// Betrags-Parser und die Systemformat-Ausgabe (DEZIMALKOMMA-Parität
// zur nativen currency_to_int/int_to_currency-Mechanik).

using System.Globalization;
using Xunit;

namespace ECTEngine.Tests
{
    public class WaehrungsformatTests
    {
        [Theory]
        // deutsch
        [InlineData("1234,56", 1234.56)]
        [InlineData("1.234,56", 1234.56)]
        [InlineData("1.234.567,89", 1234567.89)]
        // schweizerisch
        [InlineData("1234.56", 1234.56)]
        [InlineData("1'234.56", 1234.56)]
        [InlineData("1'234'567.89", 1234567.89)]
        [InlineData("1’234.56", 1234.56)]      // typografischer Apostroph
        // englisch
        [InlineData("1,234.56", 1234.56)]
        [InlineData("1,234,567", 1234567)]
        // Gruppierung ohne Dezimalteil
        [InlineData("1.234.567", 1234567)]
        // einfache Werte
        [InlineData("119", 119)]
        [InlineData("119,5", 119.5)]
        [InlineData("119.5", 119.5)]
        [InlineData("-42,10", -42.10)]
        [InlineData(" 7 ", 7)]
        public void TryParse_ToleranteSchreibweisen(string eingabe, decimal erwartet)
        {
            Assert.True(Waehrungsformat.TryParse(eingabe, out var wert));
            Assert.Equal(erwartet, wert);
        }

        [Theory]
        [InlineData("")]
        [InlineData("   ")]
        [InlineData("abc")]
        [InlineData("1,2,3.4,5")]
        public void TryParse_Fehlerfaelle(string eingabe)
        {
            Assert.False(Waehrungsformat.TryParse(eingabe, out _));
        }

        [Fact]
        public void TryParse_EinPunkt_IstDezimaltrenner()
        {
            // Die bewusste Mehrdeutigkeits-Regel: "1.234" ohne Komma ist
            // 1,234 (Dezimaltrenner), KEINE deutsche Tausendergruppe.
            Assert.True(Waehrungsformat.TryParse("1.234", out var wert));
            Assert.Equal(1.234m, wert);
        }

        [Theory]
        [InlineData("19", 19)]
        [InlineData("19 %", 19)]
        [InlineData("10,7%", 10.7)]
        [InlineData("10.7", 10.7)]
        public void TryParseProzent(string eingabe, decimal erwartet)
        {
            Assert.True(Waehrungsformat.TryParseProzent(eingabe, out var wert));
            Assert.Equal(erwartet, wert);
        }

        [Fact]
        public void Formatierung_FolgtSystemeinstellung()
        {
            // locale-neutral formuliert: gegen CurrentCulture vergleichen
            var kultur = CultureInfo.CurrentCulture;
            Assert.Equal(1234.56m.ToString("N2", kultur),
                Waehrungsformat.Betrag(1234.56m));
            Assert.Equal(1234.56m.ToString("0.00", kultur),
                Waehrungsformat.BetragOhneGruppierung(1234.56m));
            Assert.Equal(10.7m.ToString("0.##", kultur),
                Waehrungsformat.Zahl(10.7m));
        }

        [Fact]
        public void Roundtrip_FormatUndParse()
        {
            // Was wir schreiben (Systemformat), müssen wir auch wieder
            // lesen können -- und zwar unabhängig davon, unter welcher
            // Locale geschrieben wurde (Mandanten-Umzug DE <-> CH).
            foreach (var kultur in new[] { "de-DE", "de-CH", "fr-CH", "en-US" })
            {
                var c = new CultureInfo(kultur);
                string mitGruppe = 1234567.89m.ToString("N2", c);
                string ohneGruppe = 1234.56m.ToString("0.00", c);

                Assert.True(Waehrungsformat.TryParse(mitGruppe, out var w1),
                    $"Parse fehlgeschlagen fuer '{mitGruppe}' ({kultur})");
                Assert.Equal(1234567.89m, w1);
                Assert.True(Waehrungsformat.TryParse(ohneGruppe, out var w2));
                Assert.Equal(1234.56m, w2);
            }
        }
    }
}
