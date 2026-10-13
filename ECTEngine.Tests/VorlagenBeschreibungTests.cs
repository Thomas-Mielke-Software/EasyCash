// VorlagenBeschreibungTests.cs -- Beschreibungstext nach der Wahl einer
// Buchungsvorlage beim Bearbeiten (Vorlagenname am Anfang austauschen,
// Rest erhalten; frei getippter Text bleibt bzw. bekommt bei Gruppen den
// Vorlagennamen vorangestellt).

using Xunit;

namespace ECTEngine.Tests
{
    public class VorlagenBeschreibungTests
    {
        private static readonly string[] Vorlagen =
        {
            "Einnahme",
            "Einnahme Eigenverbrauch",
            "Finanzamtsrückzahlung",
            "Miete",
            "Reverse Charge EU-Eingangsrechnung",
            "Drittland-Eingangsrechnung"
        };

        private static string Nach(string text, string vorlage, bool gruppe = false)
            => VorlagenBeschreibung.NachVorlagenwahl(text, vorlage, gruppe, Vorlagen);

        [Fact]
        public void Zusatz_hinter_Vorlagennamen_bleibt_erhalten()
        {
            Assert.Equal("Finanzamtsrückzahlung 3Q/2026",
                Nach("Einnahme Eigenverbrauch 3Q/2026", "Finanzamtsrückzahlung"));
        }

        [Fact]
        public void Laengster_passender_Vorlagenname_gewinnt()
        {
            // "Einnahme" passt auch, darf aber nicht gewinnen -- sonst
            // bliebe "Eigenverbrauch" stehen.
            Assert.Equal("Miete 3Q",
                Nach("Einnahme Eigenverbrauch 3Q", "Miete"));
        }

        [Fact]
        public void Nur_an_Wortgrenze()
        {
            // "Miete" ist kein Anfang von "Mieterhöhung" -> frei getippt
            Assert.Equal("Mieterhöhung Juli",
                Nach("Mieterhöhung Juli", "Finanzamtsrückzahlung"));
        }

        [Fact]
        public void Gross_Kleinschreibung_egal()
        {
            Assert.Equal("Finanzamtsrückzahlung 1Q",
                Nach("einnahme eigenverbrauch 1Q", "Finanzamtsrückzahlung"));
        }

        [Fact]
        public void Leerer_oder_wortgleicher_Text_wird_ersetzt()
        {
            Assert.Equal("Miete", Nach("", "Miete"));
            Assert.Equal("Miete", Nach("  ", "Miete"));
            Assert.Equal("Miete", Nach("Finanzamtsrückzahlung", "Miete"));
        }

        [Fact]
        public void Freier_Text_bleibt_bei_einfacher_Vorlage()
        {
            Assert.Equal("Hotel Berlin, 3 Nächte",
                Nach("Hotel Berlin, 3 Nächte", "Miete"));
        }

        [Fact]
        public void Freier_Text_bekommt_Gruppen_Praefix()
        {
            Assert.Equal("Reverse Charge EU-Eingangsrechnung: Hotel Berlin",
                Nach("Hotel Berlin", "Reverse Charge EU-Eingangsrechnung", gruppe: true));
        }

        [Fact]
        public void Gruppen_Praefix_wird_getauscht_statt_gestapelt()
        {
            Assert.Equal("Drittland-Eingangsrechnung: Hotel Berlin",
                Nach("Reverse Charge EU-Eingangsrechnung: Hotel Berlin",
                     "Drittland-Eingangsrechnung", gruppe: true));
            // gleiche Vorlage nochmal: idempotent
            Assert.Equal("Reverse Charge EU-Eingangsrechnung: Hotel Berlin",
                Nach("Reverse Charge EU-Eingangsrechnung: Hotel Berlin",
                     "Reverse Charge EU-Eingangsrechnung", gruppe: true));
        }

        [Fact]
        public void Gruppen_Praefix_faellt_beim_Wechsel_auf_einfache_Vorlage()
        {
            Assert.Equal("Miete: Hotel Berlin",
                Nach("Reverse Charge EU-Eingangsrechnung: Hotel Berlin", "Miete"));
        }
    }
}
