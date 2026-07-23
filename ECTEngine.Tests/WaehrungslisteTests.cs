// WaehrungslisteTests.cs -- Tests für die Waehrungsliste (Parsen der
// ect-forex-API-Antwort, Verschmelzen mit der bestehenden Auswahl,
// Cache-Roundtrip). Kein Netzzugriff -- die reine ParseUndVerschmelze-
// Methode bekommt die JSON-Antwort als String.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Xunit;

namespace ECTEngine.Tests
{
    public class WaehrungslisteTests
    {
        // Ausschnitt im Format der echten API ({ "Code": "Name", ... }),
        // bewusst UNSORTIERT und mit einem Nicht-Latin-1-Zeichen (ł).
        private const string ApiJson =
            "{\"USD\":\"US-Dollar\",\"BTC\":\"Bitcoin\",\"CHF\":\"Schweizer Franken\"," +
            "\"PLN\":\"Polnischer Z\\u0142oty\",\"AUD\":\"Australischer Dollar\"}";

        [Fact]
        public void ParseSortiertNachCodeUndBehaeltNamen()
        {
            var liste = Waehrungsliste.ParseUndVerschmelze(ApiJson, null);

            Assert.Equal(new[] { "AUD", "BTC", "CHF", "PLN", "USD" },
                liste.Select(w => w.Code).ToArray());
            Assert.Equal("Polnischer Złoty", liste.Single(w => w.Code == "PLN").Name);
        }

        [Fact]
        public void ErstbefuellungHaktLandesDefaultsAn()
        {
            // bestehend == null -> Erstbefuellung mit den Landes-Defaults (DE=0):
            // USD/CHF/PLN dabei, BTC (Krypto) und AUD nicht.
            var liste = Waehrungsliste.ParseUndVerschmelze(ApiJson, null, 0);

            Assert.True(liste.Single(w => w.Code == "USD").Aktiv);
            Assert.True(liste.Single(w => w.Code == "CHF").Aktiv);
            Assert.True(liste.Single(w => w.Code == "PLN").Aktiv);
            Assert.False(liste.Single(w => w.Code == "BTC").Aktiv);
            Assert.False(liste.Single(w => w.Code == "AUD").Aktiv);
        }

        [Fact]
        public void StartauswahlHaengtVomLandAb()
        {
            string[] Aktiv(int land) => Waehrungsliste.StandardFiatListe(land)
                .Where(w => w.Aktiv).Select(w => w.Code).OrderBy(c => c).ToArray();

            // DE: Nachbarn DKK + PLN dabei, ungarischer HUF nicht.
            Assert.Contains("DKK", Aktiv(0));
            Assert.Contains("PLN", Aktiv(0));
            Assert.DoesNotContain("HUF", Aktiv(0));

            // AT: Nachbar HUF dabei, DKK/PLN (keine AT-Nachbarn) nicht.
            Assert.Contains("HUF", Aktiv(1));
            Assert.DoesNotContain("DKK", Aktiv(1));
            Assert.DoesNotContain("PLN", Aktiv(1));

            // CH: CHF (Basiswährung) NICHT vorangehakt.
            Assert.DoesNotContain("CHF", Aktiv(2));

            // USD ist überall Teil der Startauswahl.
            Assert.All(new[] { 0, 1, 2 }, l => Assert.Contains("USD", Aktiv(l)));
        }

        [Fact]
        public void VerschmelzenBehaeltAuswahlNeueKommenAbgehakt()
        {
            // Bestehende Auswahl: CHF an, USD aus (abweichend vom Standard).
            var bestehend = new List<WaehrungsEintrag>
            {
                new WaehrungsEintrag { Code = "CHF", Name = "Schweizer Franken", Aktiv = true },
                new WaehrungsEintrag { Code = "USD", Name = "US-Dollar",         Aktiv = false },
            };

            var liste = Waehrungsliste.ParseUndVerschmelze(ApiJson, bestehend);

            // vorhandene Zustände bleiben erhalten (KEINE Standard-Voreinstellung mehr)
            Assert.True(liste.Single(w => w.Code == "CHF").Aktiv);
            Assert.False(liste.Single(w => w.Code == "USD").Aktiv);
            // neue Währungen (nicht im Bestand) kommen abgehakt hinzu
            Assert.False(liste.Single(w => w.Code == "BTC").Aktiv);
            Assert.False(liste.Single(w => w.Code == "PLN").Aktiv);
        }

        [Theory]
        [InlineData("")]
        [InlineData("kein-json")]
        [InlineData("{}")]
        [InlineData("[{\"currency\":\"ZZZ\",\"meta\":{\"error\":\"not in database\"}}]")]
        public void UnlesbareOderLeereAntwortWirftFormatException(string json)
        {
            Assert.Throws<FormatException>(
                () => Waehrungsliste.ParseUndVerschmelze(json, null));
        }

        [Theory]
        [InlineData("\"1.133900000000\"", 1.1339)]
        [InlineData("\"96117.725770200000\"", 96117.7257702)]
        [InlineData("  \"0.5\"  ", 0.5)]
        public void ParseKursLiestNacktenJsonString(string antwort, double erwartet)
        {
            decimal kurs = Waehrungsliste.ParseKurs(antwort);
            Assert.Equal((decimal)erwartet, kurs);
        }

        [Theory]
        [InlineData("\"error\"")]
        [InlineData("")]
        [InlineData("\"\"")]
        [InlineData("\"0\"")]                                    // Kurs 0 = ungültig
        [InlineData("[{\"currency\":\"ZZZ\",\"meta\":{\"error\":\"not in database\"}}]")]
        public void ParseKursWirftBeiFehlerOderUngueltig(string antwort)
        {
            Assert.Throws<FormatException>(() => Waehrungsliste.ParseKurs(antwort));
        }

        // Echte meta=2-Antworten der API (gekuerzt).
        private const string MetaUsd =
            "[{\"currency\":\"USD\",\"price\":\"1.133900000000\",\"meta\":{\"name\":\"US-Dollar\","
            + "\"type\":\"fiat\",\"source\":\"EZB\"}}]";
        private const string MetaBtc =
            "[{\"currency\":\"BTC\",\"price\":\"92019.000044355000\",\"meta\":{\"name\":\"Bitcoin\","
            + "\"type\":\"crypto\",\"source\":\"LiveCoinWatch\"}}]";

        [Fact]
        public void ParseKursInfoLiestPreisUndTyp()
        {
            var usd = Waehrungsliste.ParseKursInfo(MetaUsd, "USD");
            Assert.Equal(1.1339m, usd.Preis);
            Assert.Equal("fiat", usd.Typ);
            Assert.True(usd.IstFiat);

            var btc = Waehrungsliste.ParseKursInfo(MetaBtc, "BTC");
            Assert.Equal(92019.000044355m, btc.Preis);
            Assert.Equal("crypto", btc.Typ);
            Assert.False(btc.IstFiat);
        }

        [Fact]
        public void EuroProEinheitDrehtNurFiatUm()
        {
            // Fiat (EZB): price = Fremdwaehrung pro EUR -> Kehrwert.
            var usd = Waehrungsliste.ParseKursInfo(MetaUsd, "USD");
            Assert.Equal(1m / 1.1339m, usd.EuroProEinheit);
            // 100 USD -> ~88,19 EUR (NICHT 113,39!)
            Assert.Equal(88.19m, decimal.Round(100m * usd.EuroProEinheit, 2));

            // Krypto (LiveCoinWatch): price = EUR pro Einheit -> unveraendert.
            var btc = Waehrungsliste.ParseKursInfo(MetaBtc, "BTC");
            Assert.Equal(92019.000044355m, btc.EuroProEinheit);
        }

        [Theory]
        [InlineData("")]
        [InlineData("[{\"currency\":\"ZZZ\",\"meta\":{\"error\":\"not in database\"}}]")]
        [InlineData("[{\"currency\":\"X\",\"price\":\"0\",\"meta\":{\"type\":\"fiat\"}}]")]
        public void ParseKursInfoWirftBeiFehler(string antwort)
        {
            Assert.Throws<FormatException>(() => Waehrungsliste.ParseKursInfo(antwort, "X"));
        }

        [Theory]
        [InlineData("USD", true)]
        [InlineData("PLN", true)]
        [InlineData("CHF", true)]
        [InlineData("chf", true)]   // Groß-/Kleinschreibung egal
        [InlineData("BTC", false)]
        [InlineData("USDT", false)]
        [InlineData("", false)]
        public void IstFiatWaehrungKlassifiziert(string code, bool erwartet)
            => Assert.Equal(erwartet, Waehrungsliste.IstFiatWaehrung(code));

        [Fact]
        public void UmrechnungsanhangWirdWiedererkanntUndEntfernt()
        {
            string doku = Waehrungsliste.BaueUmrechnungsText(
                "108,22", "PLN", "25,46", "EUR", new DateTime(2026, 1, 1), 0.235298m);

            // Als kompletter Text (Umrechnung ohne vorherige Beschreibung).
            Assert.Equal("", Waehrungsliste.OhneUmrechnungsanhang(doku));
            // Mit vorangehender Beschreibung (Trenner wird mitentfernt).
            Assert.Equal("Büromaterial",
                Waehrungsliste.OhneUmrechnungsanhang("Büromaterial -- " + doku));
        }

        [Fact]
        public void ErneutesUmrechnenErsetztStattAnzuhaengen()
        {
            string d1 = Waehrungsliste.BaueUmrechnungsText(
                "100", "USD", "88,19", "EUR", new DateTime(2026, 1, 1), 0.8819m);

            // Basis wird korrekt zurückgewonnen -> Aufrufer hängt neuen Text an.
            Assert.Equal("Hotelrechnung",
                Waehrungsliste.OhneUmrechnungsanhang("Hotelrechnung -- " + d1));

            // Auch ein doppelter Alt-Anhang wird vollständig entfernt.
            Assert.Equal("Hotelrechnung",
                Waehrungsliste.OhneUmrechnungsanhang("Hotelrechnung -- " + d1 + " -- " + d1));

            // Normaler Text ohne Anhang bleibt unangetastet.
            Assert.Equal("Konferenz Zürich",
                Waehrungsliste.OhneUmrechnungsanhang("Konferenz Zürich"));
        }

        [Fact]
        public void StandardFiatListeHatDie30EzbWaehrungen()
        {
            var liste = Waehrungsliste.StandardFiatListe(0);   // Deutschland
            Assert.Equal(30, liste.Count);
            Assert.Contains(liste, w => w.Code == "CHF" && w.Name == "Schweizer Franken");
            Assert.True(liste.Single(w => w.Code == "USD").Aktiv);   // Default aktiv (DE)
            Assert.Equal("AUD", liste.First().Code);                 // nach Code sortiert
            Assert.All(liste, w => Assert.True(Waehrungsliste.IstFiatWaehrung(w.Code)));
        }

        [Fact]
        public void HoleGecachteListeFaelltAufStartlisteZurueck()
        {
            var vorher = Waehrungsliste.CacheDateiPfad;
            try
            {
                // Nicht existierender Cache -> Startliste, KEIN Netzzugriff.
                Waehrungsliste.CacheDateiPfad = Path.Combine(Path.GetTempPath(),
                    "ectengine-kein-cache-" + Guid.NewGuid().ToString("N") + ".json");
                Assert.False(Waehrungsliste.CacheVorhanden);
                Assert.Equal(30, Waehrungsliste.HoleGecachteListe().Count);
            }
            finally { Waehrungsliste.CacheDateiPfad = vorher; }
        }

        [Fact]
        public void EinwilligungWirdGespeichertUndWiderrufen()
        {
            var vorher = Waehrungsliste.ZustimmungDateiPfad;
            var tmp = Path.Combine(Path.GetTempPath(),
                "ectengine-einw-" + Guid.NewGuid().ToString("N") + ".txt");
            try
            {
                Waehrungsliste.ZustimmungDateiPfad = tmp;
                Assert.False(Waehrungsliste.ApiEinwilligungErteilt);
                Waehrungsliste.SetzeApiEinwilligung(true);
                Assert.True(Waehrungsliste.ApiEinwilligungErteilt);
                Waehrungsliste.SetzeApiEinwilligung(false);
                Assert.False(Waehrungsliste.ApiEinwilligungErteilt);
            }
            finally
            {
                Waehrungsliste.ZustimmungDateiPfad = vorher;
                try { File.Delete(tmp); } catch { /* egal */ }
            }
        }

        [Fact]
        public async Task OhneEinwilligungWirftDerApiAufrufVorDemNetzzugriff()
        {
            var vorher = Waehrungsliste.ZustimmungDateiPfad;
            try
            {
                Waehrungsliste.ZustimmungDateiPfad = Path.Combine(Path.GetTempPath(),
                    "ectengine-keine-einw-" + Guid.NewGuid().ToString("N") + ".txt");
                Assert.False(Waehrungsliste.ApiEinwilligungErteilt);
                await Assert.ThrowsAsync<InvalidOperationException>(
                    () => Waehrungsliste.HoleKursInfoAsync("USD", new DateTime(2025, 6, 2)));
            }
            finally { Waehrungsliste.ZustimmungDateiPfad = vorher; }
        }

        [Fact]
        public void MenueWaehrungenSchliesstZielAusUndFuegtEuroVoranAn()
        {
            var aktive = new List<WaehrungsEintrag>
            {
                new WaehrungsEintrag { Code = "USD", Name = "US-Dollar" },
                new WaehrungsEintrag { Code = "CHF", Name = "Schweizer Franken" },
                new WaehrungsEintrag { Code = "GBP", Name = "Britisches Pfund" },
            };

            // Ziel EUR (DE/AT): KEIN EUR-Eintrag, alle aktiven bleiben.
            var eur = Waehrungsliste.MenueWaehrungen(aktive, "EUR");
            Assert.DoesNotContain(eur, w => w.Code == "EUR");
            Assert.Equal(3, eur.Count);

            // Ziel CHF (CH): CHF (=Ziel) fällt raus, EUR kommt VORNE dazu.
            var chf = Waehrungsliste.MenueWaehrungen(aktive, "CHF");
            Assert.Equal("EUR", chf[0].Code);
            Assert.DoesNotContain(chf, w => w.Code == "CHF");
            Assert.Contains(chf, w => w.Code == "USD");
        }

        [Fact]
        public void StandardFiatCodesOhneEurSortiert()
        {
            var codes = Waehrungsliste.StandardFiatCodes();
            Assert.Equal(30, codes.Count);
            Assert.DoesNotContain("EUR", codes);
            Assert.Equal("AUD", codes.First());
            Assert.Contains("CHF", codes);
        }

        [Fact]
        public void CacheRoundtripUeberDatei()
        {
            var tempPfad = Path.Combine(Path.GetTempPath(),
                "ectengine-waehrungstest-" + Guid.NewGuid().ToString("N") + ".json");
            var vorher = Waehrungsliste.CacheDateiPfad;
            try
            {
                Waehrungsliste.CacheDateiPfad = tempPfad;

                // Noch keine Datei -> hartkodierte Startliste (kein Netzzugriff).
                Assert.Equal(30, Waehrungsliste.HoleGecachteListe().Count);
                Assert.False(Waehrungsliste.CacheVorhanden);

                var liste = Waehrungsliste.ParseUndVerschmelze(ApiJson, null, 0);   // DE
                Waehrungsliste.SpeichereListe(liste);

                var geladen = Waehrungsliste.HoleGecachteListe();
                Assert.Equal(liste.Count, geladen.Count);
                Assert.Equal("Polnischer Złoty",
                    geladen.Single(w => w.Code == "PLN").Name);

                // AktiveWaehrungen liest die angehakten aus dem Cache: DE-Defaults
                // geschnitten mit der ApiJson-Liste = CHF, PLN, USD.
                var aktiv = Waehrungsliste.AktiveWaehrungen().Select(w => w.Code).OrderBy(c => c);
                Assert.Equal(new[] { "CHF", "PLN", "USD" }, aktiv.ToArray());
            }
            finally
            {
                Waehrungsliste.CacheDateiPfad = vorher;
                try { File.Delete(tempPfad); } catch { /* egal */ }
            }
        }
    }
}
