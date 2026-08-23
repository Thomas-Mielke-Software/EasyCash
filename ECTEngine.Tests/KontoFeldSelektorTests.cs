// KontoFeldSelektorTests.cs — Tests für den Ad-hoc-Kontoselektor:
// Parsen der $land:-Spezifikation, Konto-Lookup über die
// Feldzuweisungs-Blobs des Einstellungs-Caches, Hinweistext/Vorgabe-Name
// des Anlage-Dialogs und ErzeugeKonto (Slot-Vergabe, Grenzen).

using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    // ======================================================================
    // Parser + Erkennungs-Heuristik
    // ======================================================================

    public class KontoFeldSpezifikationTests
    {
        private const string Beispiel =
            "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=48||at:Beilage E1a=9040|Umsatzsteuer=1020||";

        [Theory]
        [InlineData(Beispiel, true)]
        [InlineData("$de:EÜR=14", true)]
        [InlineData("  $AT:Umsatzsteuer=1020", true)]     // Leerraum + Großschreibung
        [InlineData("Erlöse 19%", false)]                  // normales Konto
        [InlineData("$beschreibung", false)]               // Text-Template-Variable
        [InlineData("$beleg-$vorlagenzeile", false)]
        [InlineData("$J", false)]                          // Datums-Platzhalter
        [InlineData("", false)]
        public void IstSpezifikation_Heuristik(string kontoFeld, bool erwartet)
            => Assert.Equal(erwartet, KontoFeldSpezifikation.IstSpezifikation(kontoFeld));

        [Fact]
        public void Parse_Beispiel_ZweiLaender()
        {
            var spez = KontoFeldSpezifikation.Parse(Beispiel, out string fehler);
            Assert.NotNull(spez);
            Assert.Equal("", fehler);

            var de = spez.FuerLand("de");
            Assert.Equal(2, de.Count);
            Assert.Equal("E/Ü-Rechnung", de[0].Formular);
            Assert.Equal("1103", de[0].FeldId);
            Assert.Equal("Umsatzsteuer-Voranmeldung", de[1].Formular);
            Assert.Equal("48", de[1].FeldId);

            var at = spez.FuerLand("AT");   // Lookup case-insensitiv
            Assert.Equal(2, at.Count);
            Assert.Equal("Beilage E1a", at[0].Formular);
            Assert.Equal("9040", at[0].FeldId);

            Assert.Null(spez.FuerLand("ch"));   // kein Block -> null
        }

        [Fact]
        public void Parse_OhneAbschliessendePipes_Tolerant()
        {
            var spez = KontoFeldSpezifikation.Parse(
                "$de:EÜR=14", out string fehler);
            Assert.NotNull(spez);
            var de = spez.FuerLand("de");
            var b = Assert.Single(de);
            Assert.Equal("EÜR", b.Formular);
            Assert.Equal("14", b.FeldId);
        }

        [Fact]
        public void Parse_LeerraumUmTokens_WirdGetrimmt()
        {
            var spez = KontoFeldSpezifikation.Parse(
                "$ de : E/Ü-Rechnung = 1103 | UStVA = 48 ||", out string fehler);
            Assert.NotNull(spez);
            var de = spez.FuerLand("de");
            Assert.Equal("E/Ü-Rechnung", de[0].Formular);
            Assert.Equal("1103", de[0].FeldId);
            Assert.Equal("48", de[1].FeldId);
        }

        // ------------------------------------------------------------------
        // Meta-Paar "$name=" (Kontoname-Vorschlag)
        // ------------------------------------------------------------------

        [Fact]
        public void Parse_NameMetaPaar_IstKeinFeldbedarf()
        {
            var spez = KontoFeldSpezifikation.Parse(
                "$de:Umsatzsteuer-Voranmeldung=47|$name=/Verrechnung von §13b-USt||",
                out string fehler);
            Assert.NotNull(spez);
            Assert.Equal("", fehler);

            var b = Assert.Single(spez.FuerLand("de"));
            Assert.Equal("Umsatzsteuer-Voranmeldung", b.Formular);
            Assert.Equal("47", b.FeldId);
            // führendes '/' (neutrales Konto) muss überleben
            Assert.Equal("/Verrechnung von §13b-USt", spez.NameFuerLand("de"));
        }

        [Fact]
        public void Parse_NameVorFeldpaar_ReihenfolgeEgal()
        {
            var spez = KontoFeldSpezifikation.Parse(
                "$de:$name=Mein Konto|EÜR=14||", out _);
            Assert.Equal("Mein Konto", spez.NameFuerLand("de"));
            Assert.Single(spez.FuerLand("de"));
        }

        [Fact]
        public void Parse_NameProLandGetrennt()
        {
            var spez = KontoFeldSpezifikation.Parse(
                "$de:EÜR=14|$name=Deutsch||at:E1a=9040|$name=Österreichisch||", out _);
            Assert.Equal("Deutsch", spez.NameFuerLand("de"));
            Assert.Equal("Österreichisch", spez.NameFuerLand("at"));
            Assert.Equal("", spez.NameFuerLand("ch"));   // Land ohne Block
        }

        [Fact]
        public void Parse_OhneName_LeererVorschlag()
        {
            var spez = KontoFeldSpezifikation.Parse(Beispiel, out _);
            Assert.Equal("", spez.NameFuerLand("de"));
        }

        [Fact]
        public void Parse_UnbekanntesMetaPaar_WirdUebersprungen()
        {
            // Vorwärtskompatibilität: ein künftiger $-Schlüssel darf eine
            // ältere Binary nicht zum Verknüpfen gegen ein Phantom-Formular
            // verleiten -- und die Spezifikation bleibt gültig.
            var spez = KontoFeldSpezifikation.Parse(
                "$de:EÜR=14|$kuenftig=egal||", out string fehler);
            Assert.NotNull(spez);
            Assert.Equal("", fehler);
            var b = Assert.Single(spez.FuerLand("de"));
            Assert.Equal("EÜR", b.Formular);
        }

        [Fact]
        public void Parse_NurMetaPaar_OhneFeldzuordnung_IstFehler()
        {
            var spez = KontoFeldSpezifikation.Parse(
                "$de:$name=Mein Konto||", out string fehler);
            Assert.Null(spez);
            Assert.Contains("keine Feld-Zuordnung", fehler);
        }

        [Fact]
        public void Parse_ZweiNamenImBlock_IstFehler()
        {
            var spez = KontoFeldSpezifikation.Parse(
                "$de:EÜR=14|$name=A|$name=B||", out string fehler);
            Assert.Null(spez);
            Assert.Contains("$name", fehler);
        }

        [Theory]
        [InlineData("$")]                          // leer
        [InlineData("$de:")]                       // Block ohne Paar
        [InlineData("$de:EÜR")]                    // Paar ohne '='
        [InlineData("$de:=14")]                    // Paar ohne Formularname
        [InlineData("$deutschland:EÜR=14")]        // Kürzel zu lang
        [InlineData("$EÜR=14")]                    // Land-Kürzel fehlt
        public void Parse_Syntaxfehler(string spez)
        {
            Assert.Null(KontoFeldSpezifikation.Parse(spez, out string fehler));
            Assert.NotEqual("", fehler);
        }
    }

    // ======================================================================
    // Lookup + ErzeugeKonto (statischer Einstellungs-Cache)
    // ======================================================================

    [Collection("EinstellungenCache")]   // serialisiert mit den Preset-Tests (statischer Cache)
    public class KontoFeldSelektorCacheTests
    {
        /// <summary>Seed: zwei Einnahmen- und ein Ausgabenkonto; e01 trägt
        /// die gesuchte Feld-Kombination (deutsche Formulare).</summary>
        private static void SeedeKonten(int land = 0)
        {
            var blob = new ErweiterungStore();
            blob.Setze("ECT", "E/Ü-Rechnung", "1103");
            blob.Setze("ECT", "Umsatzsteuer-Voranmeldung", "48");

            var nurEuer = new ErweiterungStore();
            nurEuer.Setze("ECT", "E/Ü-Rechnung", "1103");

            Einstellungen.LadeAusBridge(new Dictionary<string, string>
            {
                ["e00"] = "Erlöse 19%",
                ["e01"] = "Steuerfreie Umsätze",
                ["a00"] = "Bürobedarf",
                ["[EinnahmenFeldzuweisungen]00"] = nurEuer.ZuPipeFormat(),
                ["[EinnahmenFeldzuweisungen]01"] = blob.ZuPipeFormat(),
                ["[Persoenliche_Daten]land"] = land.ToString()
            });
        }

        private const string Spez =
            "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=48||at:Beilage E1a=9040||";

        [Fact]
        public void LoeseAuf_FindetKontoMitBeidenFeldern()
        {
            SeedeKonten();
            var a = KontoFeldSelektor.LoeseAuf(Spez);
            Assert.True(a.IstSpezifikation);
            Assert.Equal("", a.Fehler);
            // e00 hat nur die EÜR-Verknüpfung -> e01 ist der erste volle Treffer
            Assert.Equal("Steuerfreie Umsätze", a.Konto);
        }

        [Fact]
        public void LoeseAuf_TeilmengeReichtNicht()
        {
            SeedeKonten();
            var a = KontoFeldSelektor.LoeseAuf(
                "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=81||");
            Assert.True(a.IstSpezifikation);
            Assert.Null(a.Konto);              // UStVA-Feld 81 hat niemand
            Assert.Equal(2, a.Bedarf.Count);   // Bedarf fürs Anlegen steht bereit
        }

        [Fact]
        public void LoeseAuf_EinzelnesFeld_ErstbestesKonto()
        {
            SeedeKonten();
            var a = KontoFeldSelektor.LoeseAuf("$de:E/Ü-Rechnung=1103||");
            Assert.Equal("Erlöse 19%", a.Konto);   // e00 kommt vor e01
        }

        [Fact]
        public void LoeseAuf_ReichtNameVorschlagDurch()
        {
            SeedeKonten();
            // Feld 81 hat kein Konto -> Anlage-Fall, und genau dann braucht
            // der Dialog den Namensvorschlag der Spezifikation.
            var a = KontoFeldSelektor.LoeseAuf(
                "$de:Umsatzsteuer-Voranmeldung=81|$name=/Verrechnung von §13b-USt||");
            Assert.Null(a.Konto);
            Assert.Equal("", a.Fehler);
            Assert.Equal("/Verrechnung von §13b-USt", a.NameVorschlag);
            Assert.Single(a.Bedarf);
        }

        [Fact]
        public void LoeseAuf_LandOhneBlock_Fehler()
        {
            SeedeKonten(land: 2);   // Schweiz -- Spez kennt nur de/at
            var a = KontoFeldSelektor.LoeseAuf(Spez);
            Assert.True(a.IstSpezifikation);
            Assert.Null(a.Konto);
            Assert.Contains("ch", a.Fehler);
        }

        [Fact]
        public void LoeseAuf_LandOesterreich_NimmtAtBlock()
        {
            SeedeKonten(land: 1);
            var at = new ErweiterungStore();
            at.Setze("ECT", "Beilage E1a", "9040");
            Einstellungen.Speichere("[EinnahmenFeldzuweisungen]00", at.ZuPipeFormat());

            var a = KontoFeldSelektor.LoeseAuf(Spez);
            Assert.Equal("Erlöse 19%", a.Konto);
        }

        [Fact]
        public void LoeseAuf_KeineSpezifikation()
        {
            var a = KontoFeldSelektor.LoeseAuf("Erlöse 19%");
            Assert.False(a.IstSpezifikation);
            Assert.Null(a.Konto);
        }

        [Fact]
        public void ErzeugeKonto_LegtAmEndeDerGruppeAn()
        {
            SeedeKonten();
            var bedarf = new List<KontoFeldBedarf>
            {
                new KontoFeldBedarf("E/Ü-Rechnung", "1150"),
                new KontoFeldBedarf("Umsatzsteuer-Voranmeldung", "35")
            };

            Assert.True(KontoFeldSelektor.ErzeugeKonto(
                "Sonstige Umsätze", true, bedarf, out string fehler), fehler);

            // Neues Konto am Ende der Einnahmen (Slot 02), danach auffindbar
            Assert.Equal("Sonstige Umsätze", Einstellungen.EinnahmenKonten.Last());
            var a = KontoFeldSelektor.LoeseAuf(
                "$de:E/Ü-Rechnung=1150|Umsatzsteuer-Voranmeldung=35||");
            Assert.Equal("Sonstige Umsätze", a.Konto);

            // Bestehende Konten und deren Zuweisungen unangetastet
            Assert.Equal("Erlöse 19%", Einstellungen.EinnahmenKonten[0]);
            Assert.Equal("Steuerfreie Umsätze",
                KontoFeldSelektor.LoeseAuf(Spez).Konto);
        }

        [Fact]
        public void ErzeugeKonto_NameVergeben_Fehler()
        {
            SeedeKonten();
            var bedarf = new List<KontoFeldBedarf> { new KontoFeldBedarf("EÜR", "1") };
            Assert.False(KontoFeldSelektor.ErzeugeKonto(
                "Erlöse 19%", true, bedarf, out string fehler));
            Assert.Contains("existiert", fehler);
        }

        [Fact]
        public void ErzeugeKonto_AlleSlotsBelegt_Fehler()
        {
            var seed = new Dictionary<string, string>();
            for (int i = 0; i < 100; i++)
                seed["e" + i.ToString("D2")] = "Konto " + i;
            Einstellungen.LadeAusBridge(seed);

            var bedarf = new List<KontoFeldBedarf> { new KontoFeldBedarf("EÜR", "1") };
            Assert.False(KontoFeldSelektor.ErzeugeKonto(
                "Passt nicht mehr", true, bedarf, out string fehler));
            Assert.Contains("100", fehler);
        }

        [Fact]
        public void Berechne_ZeileMitSpezifikation_LoestKontoAuf()
        {
            SeedeKonten();
            Einstellungen.SpeicherePreset(9, new Preset(
                "Feld-Konto-Test", true, 19000, 1, "",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("E", Spez, "0", "$brutto*0,5", "Anteil $beschreibung")
                }));

            var vorlage = Einstellungen.Presets[9];
            var basis = new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(10000, 19000),
                Beschreibung = "Test"
            };
            var b = BuchungsgruppenRechner.Berechne(vorlage, basis);
            Assert.False(b.HatFehler, b.Zeilen.Count > 0 ? b.Zeilen[0].Fehler : "");
            Assert.Equal("Steuerfreie Umsätze", b.Zeilen[0].Konto);
        }

        [Fact]
        public void Berechne_ZeileOhneTreffer_MeldetFehler()
        {
            SeedeKonten();
            Einstellungen.SpeicherePreset(9, new Preset(
                "Feld-Konto-Test", true, 19000, 1, "",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("E", "$de:E/Ü-Rechnung=9999||", "0",
                        "$brutto*0,5", "")
                }));

            var basis = new Buchung
            {
                Art = Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(10000, 19000)
            };
            var b = BuchungsgruppenRechner.Berechne(Einstellungen.Presets[9], basis);
            Assert.True(b.HatFehler);
            Assert.Contains("Konto", b.Zeilen[0].Fehler);
        }
    }

    // ======================================================================
    // Dialog-Bausteine (Hinweistext + Vorgabe-Name)
    // ======================================================================

    public class KontoFeldDialogTextTests
    {
        [Fact]
        public void HinweisText_EinFeld_Singular()
        {
            var text = KontoFeldSelektor.HinweisText(
                new[] { "Umsatzsteuerpflichtige Betriebseinnahmen" });
            Assert.StartsWith("Diese Vorlage benötigt ein Konto, das mit dem Feld "
                + "\"Umsatzsteuerpflichtige Betriebseinnahmen\" verknüpft ist.", text);
            Assert.Contains("geeignet kürzen", text);
        }

        [Fact]
        public void HinweisText_DreiFelder_MitUndVerbunden()
        {
            var text = KontoFeldSelektor.HinweisText(new[] { "A", "B", "C" });
            Assert.Contains("den Feldern \"A\", \"B\" und \"C\" verknüpft", text);
        }

        [Fact]
        public void VorgabeName_MitSchraegstrichGetrennt()
        {
            Assert.Equal("A / B", KontoFeldSelektor.VorgabeName(new[] { "A", "B" }));
            Assert.Equal("A", KontoFeldSelektor.VorgabeName(new[] { "A" }));
        }

        [Fact]
        public void VorgabeName_SpezifikationsNameSchlaegtFeldnamen()
        {
            Assert.Equal("/Verrechnung von §13b-USt",
                KontoFeldSelektor.VorgabeName(new[] { "A", "B" },
                    "/Verrechnung von §13b-USt"));
            // leer/Leerraum -> zurück zur Feldnamen-Vorgabe
            Assert.Equal("A / B", KontoFeldSelektor.VorgabeName(new[] { "A", "B" }, "   "));
            Assert.Equal("A / B", KontoFeldSelektor.VorgabeName(new[] { "A", "B" }, null));
        }
    }
}
