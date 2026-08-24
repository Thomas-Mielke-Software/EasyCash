// VorlagenBibliothekTests.cs -- Tests für die mitgelieferte Vorlagen-
// Bibliothek: Laden der eingebetteten Ressource, Baumstruktur, Suche und
// -- am wichtigsten -- die maschinelle Prüfung JEDER katalogisierten
// Vorlage gegen die produktiven Parser (PresetXml, KontoFeldSelektor).
// Ein Tippfehler in einer Konto-Spezifikation fällt damit im Test auf und
// nicht erst im Buchen-Dialog des Anwenders.

using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    public class VorlagenBibliothekTests
    {
        [Fact]
        public void Lade_RessourceVorhandenUndFehlerfrei()
        {
            var wurzeln = VorlagenBibliothek.Lade();
            Assert.Equal("", VorlagenBibliothek.Fehler);
            Assert.NotEmpty(wurzeln);
        }

        [Fact]
        public void Baum_HatVerschachtelteAbschnitte()
        {
            var wurzel = Assert.Single(VorlagenBibliothek.Lade());
            Assert.Equal("Auslandsgeschäfte", wurzel.Titel);
            // Die Wurzel trägt selbst keine Einträge, nur Unterabschnitte
            Assert.Empty(wurzel.Eintraege);

            var rc = Assert.Single(wurzel.Abschnitte,
                a => a.Titel == "Übergang der Steuerschuld (Reverse Charge)");
            Assert.Contains(wurzel.Abschnitte,
                a => a.Titel == "Innergemeinschaftlicher Warenverkehr");

            // Dritte Ebene: Länder-Unterabschnitte dort, wo sich DE und AT
            // nicht in einer Vorlage zusammenfassen lassen -- daneben ein
            // Eintrag, der für beide Länder gilt.
            Assert.Contains(rc.Abschnitte, a => a.Titel.StartsWith("Deutschland"));
            Assert.Contains(rc.Abschnitte, a => a.Titel.StartsWith("Österreich"));
            Assert.NotEmpty(rc.Eintraege);
        }

        [Fact]
        public void JedeVorlage_IstUeberDenImportWegLesbar()
        {
            foreach (var e in AlleEintraege())
            {
                var p = PresetXml.Importiere(e.VorlagenXml);   // wirft bei Fehler
                Assert.False(string.IsNullOrWhiteSpace(p.Text));
                Assert.Equal(e.Name, p.Text);
                Assert.Equal(e.IstEinnahme, !p.Ausgabe);
                Assert.Equal(e.ZeilenAnzahl, p.Zeilen.Count);
            }
        }

        [Fact]
        public void JedeKontoSpezifikation_IstSyntaktischGueltig()
        {
            foreach (var e in AlleEintraege())
            {
                var p = PresetXml.Importiere(e.VorlagenXml);
                PruefeKonto(p.Konto, e.Name + " (Basis)");
                for (int i = 0; i < p.Zeilen.Count; i++)
                    PruefeKonto(p.Zeilen[i].Konto, $"{e.Name} (Zeile {i + 1})");
            }
        }

        [Fact]
        public void JedeVorlage_HatTitelStichworteUndNotiz()
        {
            foreach (var e in AlleEintraege())
            {
                Assert.False(string.IsNullOrWhiteSpace(e.Titel), e.Name);
                Assert.False(string.IsNullOrWhiteSpace(e.Stichworte), e.Name);
                Assert.False(string.IsNullOrWhiteSpace(e.Notiz), e.Name);
            }
        }

        [Fact]
        public void Namen_SindEindeutig()
        {
            // Der Name landet als Preset-Text in der ini -- doppelte Namen
            // wären in der Vorlagen-Auswahl nicht auseinanderzuhalten.
            var namen = AlleEintraege().Select(e => e.Name).ToList();
            Assert.Equal(namen.Count, namen.Distinct().Count());
        }

        // ------------------------------------------------------------------
        // Suche
        // ------------------------------------------------------------------

        [Fact]
        public void Suche_LeerLiefertAllesUnveraendert()
        {
            Assert.Equal(ZaehleEintraege(VorlagenBibliothek.Lade()),
                         ZaehleEintraege(VorlagenBibliothek.Suche("   ")));
        }

        [Fact]
        public void Suche_AbschnittstitelLiefertGanzenAbschnitt()
        {
            // "Reverse Charge" steht im Titel des Abschnitts -- dann bleibt
            // sein GANZER Teilbaum stehen, samt Länder-Unterabschnitten und
            // deren Einträgen, auch wenn einzelne Einträge das Wort nicht
            // im Titel führen.
            var treffer = VorlagenBibliothek.Suche("reverse charge");
            var abschnitt = Assert.Single(
                Sammle(treffer).Where(a => a.Titel.Contains("Reverse Charge")));
            var vollstaendig = VorlagenBibliothek.Lade()
                .SelectMany(Sammle1).Single(a => a.Titel.Contains("Reverse Charge"));
            Assert.Equal(vollstaendig.AlleEintraege().Count(),
                         abschnitt.AlleEintraege().Count());
            Assert.NotEmpty(abschnitt.Abschnitte);
        }

        private static IEnumerable<BibliothekAbschnitt> Sammle1(BibliothekAbschnitt a)
        {
            yield return a;
            foreach (var u in a.Abschnitte)
                foreach (var t in Sammle1(u)) yield return t;
        }

        [Fact]
        public void Suche_StichwortFindetEinzelnenEintrag()
        {
            var namen = Eintraege(VorlagenBibliothek.Suche("zoll")).Select(e => e.Name);
            Assert.Contains("Entrichtete Einfuhrumsatzsteuer", namen);
            Assert.DoesNotContain("Reverse Charge EU-Ausgangsrechnung", namen);
        }

        [Theory]
        [InlineData("gemäß")]      // Original-Schreibweise
        [InlineData("gemass")]     // ohne Umlaut/Eszett getippt
        [InlineData("GEMÄSS")]     // Großschreibung
        public void Suche_IstGegenUmlauteUndGrossschreibungRobust(string eingabe)
        {
            // "gemäß" steht in den Konto-Namen der §13b-Zusatzzeilen und
            // damit im durchsuchten Vorlagen-XML nicht -- wohl aber in der
            // Notiz bzw. dem Titel; entscheidend ist, dass alle drei
            // Schreibweisen dasselbe finden.
            Assert.Equal(
                Eintraege(VorlagenBibliothek.Suche("gemäß")).Count(),
                Eintraege(VorlagenBibliothek.Suche(eingabe)).Count());
        }

        [Fact]
        public void Suche_MehrereWoerterWerdenUndVerknuepft()
        {
            var beide = Eintraege(VorlagenBibliothek.Suche("erwerb kleinunternehmer"));
            var eintrag = Assert.Single(beide);
            Assert.Equal("i.g. Erwerb (Kleinunternehmer)", eintrag.Name);
        }

        [Fact]
        public void Suche_OhneTrefferLiefertLeer()
            => Assert.Empty(VorlagenBibliothek.Suche("gibtesnicht"));

        [Fact]
        public void Normalisiere_LoestUmlauteAuf()
        {
            Assert.Equal("gemaess", VorlagenBibliothek.Normalisiere("Gemäß"));
            Assert.Equal("ueberoe", VorlagenBibliothek.Normalisiere("Überö"));
        }

        // ------------------------------------------------------------------
        // Helfer
        // ------------------------------------------------------------------

        private static void PruefeKonto(string konto, string wo)
        {
            if (!KontoFeldSpezifikation.IstSpezifikation(konto))
                return;   // normaler Kontoname ist erlaubt
            var spez = KontoFeldSpezifikation.Parse(konto, out string fehler);
            Assert.True(spez != null, $"{wo}: {fehler}");
            // Mindestens ein Land-Block; WELCHE Länder, hängt von der Vorlage
            // ab (rein österreichische Vorlagen haben keinen de-Block). Ob die
            // Felder inhaltlich passen, prüft VorlagenBibliothekFelderTests.
            Assert.NotEmpty(spez.Laender);
        }

        private static IEnumerable<BibliothekEintrag> AlleEintraege()
            => VorlagenBibliothek.Lade().SelectMany(a => a.AlleEintraege());

        private static IEnumerable<BibliothekEintrag> Eintraege(
            IReadOnlyList<BibliothekAbschnitt> baum)
            => baum.SelectMany(a => a.AlleEintraege());

        private static int ZaehleEintraege(IReadOnlyList<BibliothekAbschnitt> baum)
            => Eintraege(baum).Count();

        /// <summary>Alle Abschnitte eines Baums flach.</summary>
        private static IEnumerable<BibliothekAbschnitt> Sammle(
            IReadOnlyList<BibliothekAbschnitt> baum)
        {
            foreach (var a in baum)
            {
                yield return a;
                foreach (var u in Sammle(a.Abschnitte)) yield return u;
            }
        }
    }
}
