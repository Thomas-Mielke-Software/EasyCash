// FormularDefinitionTests.cs — Tests für den .ecf-Vollparser
// (FormularDefinition): alle Feldtypen, Geometrie-Attribute, Seitenbilder,
// Abschnitte, atoi-Toleranz und der CP1252-Fallback für Alt-Dateien ohne
// XML-Encoding-Deklaration.

using System;
using System.IO;
using System.Linq;
using System.Text;
using Xunit;

namespace ECTEngine.Tests
{
    public class FormularDefinitionTests : IDisposable
    {
        private readonly string _tempDir;

        public FormularDefinitionTests()
        {
            _tempDir = Path.Combine(Path.GetTempPath(),
                "ect-ecf-tests-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(_tempDir);
        }

        public void Dispose()
        {
            try { Directory.Delete(_tempDir, recursive: true); } catch { }
        }

        private string SchreibeEcf(string name, string inhalt, Encoding encoding)
        {
            string pfad = Path.Combine(_tempDir, name);
            // .ecf-Welt ist CRLF -- Zeilenenden des Literal-Strings
            // vereinheitlichen, damit Roundtrip-Vergleiche stimmen
            inhalt = inhalt.Replace("\r\n", "\n").Replace("\n", "\r\n");
            File.WriteAllText(pfad, inhalt, encoding);
            return pfad;
        }

        // Vollständiges Beispiel mit allen Feldtypen und Sektionen
        private const string BeispielXml =
@"<?xml version=""1.0"" encoding=""windows-1252"" ?>
<formular name=""E/Ü-Rechnung"" anzeigename=""E/Ü-Rechnung 2025"" seiten=""4"" schriftart=""Courier New"" schriftgroesse=""24"" filter=""betrieb"" >
	<felder>
		<feld id=""1"" typ=""Einstellungsdaten"" seite=""1"" horizontal=""318"" vertikal=""180"" ausrichtung=""linksbuendig"" anteil=""netto"" nachkommaanteil=""ohne"" >
			<name>Steuernummer</name>
			<erweiterung>fsteuernummer</erweiterung>
		</feld>
		<feld id=""1111"" typ=""Einnahmen"" seite=""1"" horizontal=""1178"" vertikal=""668"" ausrichtung=""rechtsbuendig"" anteil=""netto"" nachkommaanteil=""mit"" nullwertdarstellung=""nein"" elsterfeldname=""EUER/BEin/Kleinunternehmer/Sum/E6000101"" veraltet=""nein"" >
			<name>Kleinunternehmer-Einnahmen</name>
			<erweiterung/>
		</feld>
		<feld id=""4"" typ=""Summe"" seite=""2"" horizontal=""931"" vertikal=""668"" anteil=""netto"" nachkommaanteil=""mit"" nullwertdarstellung=""ja"" veraltet=""ja"" >
			<name>Summenfeld</name>
			<erweiterung>119+1111</erweiterung>
		</feld>
		<feld id=""50"" typ=""Ausgaben"" seite=""3"" horizontal=""600"" vertikal=""100"" anteil=""mwst"" >
			<name>Vorsteuer</name>
			<erweiterung/>
		</feld>
		<feld id=""60"" typ=""Dokumentdaten"" seite=""3"" horizontal=""10"" vertikal=""20"" >
			<name>Zahlbetrag</name>
			<erweiterung>[Elster]UST-Zahlbetrag-$J-01</erweiterung>
		</feld>
		<feld id=""70"" typ=""Freitext"" seite=""4"" horizontal=""500"" vertikal=""700"" anteil=""brutto"" >
			<name>Jahrestext</name>
			<erweiterung>Kalenderjahr $J</erweiterung>
		</feld>
		<feld id=""80"" typ=""KryptischerNeuerTyp"" seite=""4"" horizontal=""1"" vertikal=""2"" >
			<name>Zukunftsfeld</name>
			<erweiterung/>
		</feld>
	</felder>
	<seiten>
		<seite nr=""1"" >EUeR-d-2025-1.png</seite>
		<seite nr=""2"" >EUeR-d-2025-2.png</seite>
	</seiten>
	<abschnitte>
		<abschnitt name=""Betriebseinnahmen"" seite=""1"" vertikal=""495"" />
		<abschnitt name=""Absetzung für Abnutzung (AfA)"" seite=""2"" vertikal=""100"" />
	</abschnitte>
</formular>";

        // ------------------------------------------------------------------
        // Grundparsen
        // ------------------------------------------------------------------

        [Fact]
        public void ParstFormularAttribute()
        {
            var pfad = SchreibeEcf("test.ecf", BeispielXml, Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);

            Assert.Equal("E/Ü-Rechnung", def.Name);
            Assert.Equal("E/Ü-Rechnung 2025", def.Anzeigename);
            Assert.Equal(4, def.Seitenzahl);
            Assert.Equal("Courier New", def.Schriftart);
            Assert.Equal(24, def.Schriftgroesse);
            Assert.Equal("betrieb", def.Filter);
            Assert.False(def.Querformat);
            Assert.Equal(0, def.Voranmeldungszeitraum);
            Assert.Equal(pfad, def.Quellpfad);
            Assert.NotNull(def.Dokument);
        }

        [Fact]
        public void ParstAlleFeldtypen()
        {
            var pfad = SchreibeEcf("typen.ecf", BeispielXml, Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);

            Assert.Equal(7, def.Felder.Count);
            Assert.Equal(FormularFeldTyp.Einstellungsdaten, def.Felder[0].Typ);
            Assert.Equal(FormularFeldTyp.Einnahmen, def.Felder[1].Typ);
            Assert.Equal(FormularFeldTyp.Summe, def.Felder[2].Typ);
            Assert.Equal(FormularFeldTyp.Ausgaben, def.Felder[3].Typ);
            Assert.Equal(FormularFeldTyp.Dokumentdaten, def.Felder[4].Typ);
            Assert.Equal(FormularFeldTyp.Freitext, def.Felder[5].Typ);
            // Unbekannte Typen tolerieren (Vorwärtskompatibilität)
            Assert.Equal(FormularFeldTyp.Unbekannt, def.Felder[6].Typ);
            Assert.Equal("KryptischerNeuerTyp", def.Felder[6].TypRoh);
        }

        [Fact]
        public void ParstFeldAttribute()
        {
            var pfad = SchreibeEcf("attr.ecf", BeispielXml, Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);

            var steuernr = def.Felder[0];
            Assert.Equal(1, steuernr.Id);
            Assert.Equal(1, steuernr.Seite);
            Assert.Equal(318, steuernr.Horizontal);
            Assert.Equal(180, steuernr.Vertikal);
            Assert.False(steuernr.RechtsBuendig);   // explizit linksbuendig
            Assert.Equal(FormularFeldAnteil.Netto, steuernr.Anteil);
            Assert.True(steuernr.NachkommaanteilOhne);
            Assert.False(steuernr.NullwertAnzeigen);
            Assert.False(steuernr.Veraltet);
            Assert.Equal("Steuernummer", steuernr.Name);
            Assert.Equal("fsteuernummer", steuernr.Erweiterung);

            var kleinunternehmer = def.Felder[1];
            Assert.Equal(1111, kleinunternehmer.Id);
            Assert.True(kleinunternehmer.RechtsBuendig);
            Assert.Equal(1178, kleinunternehmer.Horizontal);   // > 1000 erlaubt!
            Assert.False(kleinunternehmer.NachkommaanteilOhne);
            Assert.Equal("EUER/BEin/Kleinunternehmer/Sum/E6000101",
                kleinunternehmer.Elsterfeldname);
            Assert.Equal("", kleinunternehmer.Erweiterung);   // <erweiterung/>

            var summe = def.Felder[2];
            Assert.True(summe.NullwertAnzeigen);
            Assert.True(summe.Veraltet);
            Assert.Equal("119+1111", summe.Erweiterung);
            // Ausrichtung fehlt -> rechtsbuendig (nativer Default)
            Assert.True(summe.RechtsBuendig);

            var vorsteuer = def.Felder[3];
            Assert.Equal(FormularFeldAnteil.Mwst, vorsteuer.Anteil);
            // anteil-Attribut fehlt bei Feld 60 -> Brutto (nativer Default)
            Assert.Equal(FormularFeldAnteil.Brutto, def.Felder[4].Anteil);
        }

        [Fact]
        public void ParstSeitenbilderUndAbschnitte()
        {
            var pfad = SchreibeEcf("seiten.ecf", BeispielXml, Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);

            Assert.Equal(2, def.Seitenbilder.Count);
            Assert.Equal("EUeR-d-2025-1.png", def.Seitenbilder[1]);
            Assert.Equal("EUeR-d-2025-2.png", def.Seitenbilder[2]);

            Assert.Equal(2, def.Abschnitte.Count);
            Assert.Equal("Betriebseinnahmen", def.Abschnitte[0].Name);
            Assert.Equal(1, def.Abschnitte[0].Seite);
            Assert.Equal(495, def.Abschnitte[0].Vertikal);
            Assert.Equal("Absetzung für Abnutzung (AfA)", def.Abschnitte[1].Name);
            Assert.Equal(2, def.Abschnitte[1].Seite);
        }

        // ------------------------------------------------------------------
        // Sonderfälle Formular-Ebene
        // ------------------------------------------------------------------

        [Fact]
        public void QuerformatUndVoranmeldungszeitraum()
        {
            var xml = @"<?xml version=""1.0"" encoding=""windows-1252"" ?>
<formular name=""U30"" anzeigename=""U30 2011 Q1 (AT)"" seiten=""2"" querformat=""1"" voranmeldungszeitraum=""13"" >
	<felder/>
</formular>";
            var pfad = SchreibeEcf("u30.ecf", xml, Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);

            Assert.True(def.Querformat);
            Assert.Equal(13, def.Voranmeldungszeitraum);
            Assert.Empty(def.Felder);
        }

        [Fact]
        public void AnzeigenameFallbackAufName()
        {
            var xml = @"<?xml version=""1.0"" ?><formular name=""Testform"" seiten=""1"" ><felder/></formular>";
            var pfad = SchreibeEcf("noanzeige.ecf", xml, Encoding.ASCII);
            var def = FormularDefinition.Lade(pfad);
            Assert.Equal("Testform", def.Anzeigename);
            Assert.Equal(1, def.Seitenzahl);
        }

        [Fact]
        public void KeinFormularRootWirftInvalidData()
        {
            var pfad = SchreibeEcf("kaputt.ecf",
                @"<?xml version=""1.0"" ?><anderes/>", Encoding.ASCII);
            Assert.Throws<InvalidDataException>(() => FormularDefinition.Lade(pfad));
        }

        [Fact]
        public void FehlendeDateiWirft()
        {
            Assert.ThrowsAny<IOException>(() =>
                FormularDefinition.Lade(Path.Combine(_tempDir, "gibtsnicht.ecf")));
        }

        // ------------------------------------------------------------------
        // Encoding: Alt-Datei OHNE Deklaration mit rohen CP1252-Bytes
        // (wie EUeR2005.ecf: 'Ü' = Byte 0xDC, für UTF-8 ungültig)
        // ------------------------------------------------------------------

        [Fact]
        public void Cp1252FallbackFuerAltdateienOhneDeklaration()
        {
            var xml = @"<?xml version=""1.0"" ?>
<formular name=""E/Ü-Rechnung"" seiten=""4"" >
	<felder>
		<feld id=""1"" typ=""Freitext"" seite=""1"" horizontal=""10"" vertikal=""20"" >
			<name>Überschrift</name>
			<erweiterung>Größe</erweiterung>
		</feld>
	</felder>
</formular>";
            // Bewusst als CP1252 OHNE BOM schreiben -> rohe 0xDC/0xFC-Bytes
            var pfad = SchreibeEcf("alt2005.ecf", xml, Encoding.GetEncoding(1252));

            var def = FormularDefinition.Lade(pfad);
            Assert.Equal("E/Ü-Rechnung", def.Name);
            Assert.Equal("Überschrift", def.Felder[0].Name);
            Assert.Equal("Größe", def.Felder[0].Erweiterung);
        }

        // ------------------------------------------------------------------
        // atoi-Toleranz
        // ------------------------------------------------------------------

        [Theory]
        [InlineData(null, 0)]
        [InlineData("", 0)]
        [InlineData("42", 42)]
        [InlineData("  42  ", 42)]
        [InlineData("-7", -7)]
        [InlineData("+7", 7)]
        [InlineData("13abc", 13)]     // fuehrende Zahl, Rest ignoriert
        [InlineData("abc", 0)]
        [InlineData("-", 0)]
        public void AtoiSemantik(string eingabe, int erwartet)
        {
            Assert.Equal(erwartet, FormularDefinition.Atoi(eingabe));
        }

        // ------------------------------------------------------------------
        // Echte .ecf aus dem Repo (falls vorhanden): Grundinvarianten
        // ------------------------------------------------------------------

        [Fact]
        public void EchteRepoDateienParsenFehlerfrei()
        {
            // Debug\*.ecf = die deployten Formulare; Test läuft nur, wenn das
            // Repo-Layout verfügbar ist (im CI-freien Einzelplatz-Setup immer).
            string repoDebug = FindeRepoDebugVerzeichnis();
            if (repoDebug == null) return;   // kein Repo-Kontext -> still ok

            var dateien = Directory.GetFiles(repoDebug, "*.ecf");
            Assert.NotEmpty(dateien);

            foreach (var datei in dateien)
            {
                var def = FormularDefinition.Lade(datei);
                Assert.False(string.IsNullOrEmpty(def.Name));
                Assert.True(def.Seitenzahl >= 1);
                Assert.NotEmpty(def.Felder);
                // jede Seite mit Feldern sollte im gueltigen Bereich liegen
                Assert.All(def.Felder, f => Assert.InRange(f.Seite, 1, def.Seitenzahl));
                // UVA-Dateien tragen einen Voranmeldungszeitraum 1-16
                string basis = Path.GetFileNameWithoutExtension(datei);
                if (basis.StartsWith("uva-", StringComparison.OrdinalIgnoreCase))
                    Assert.InRange(def.Voranmeldungszeitraum, 1, 16);
            }
        }

        private static string FindeRepoDebugVerzeichnis()
        {
            // vom Test-Bin-Verzeichnis aufwärts nach Debug\*.ecf suchen
            var dir = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
            for (int i = 0; i < 8 && dir != null; i++, dir = dir.Parent)
            {
                string kandidat = Path.Combine(dir.FullName, "Debug");
                if (Directory.Exists(kandidat)
                    && Directory.GetFiles(kandidat, "*.ecf").Length > 0)
                    return kandidat;
            }
            return null;
        }

        // ------------------------------------------------------------------
        // Designer: Speichern im XMLite-Format (Roundtrip + Mutationen)
        // ------------------------------------------------------------------

        /// <summary>Entfernt die stilistisch optionalen Leerzeichen vor
        /// '&gt;' bzw. '/&gt;' -- der XMLite-Writer schreibt sie (Attribut
        /// mit nachgestelltem Leerzeichen), manche neuere .ecf wurden ohne
        /// sie erzeugt. Der Designer normalisiert beim ersten Speichern auf
        /// den XMLite-Stil (genau wie der native Designer es taete).</summary>
        private static string OhneStilLeerzeichen(string text)
            => text.Replace("\" >", "\">").Replace("\" />", "\"/>")
                   .Replace("\" ?>", "\"?>");

        [Fact]
        public void RoundtripEchteDateienErhaeltInhalt()
        {
            // Laden + unveraendert Speichern: bis auf die optionalen
            // XMLite-Stil-Leerzeichen byte-genau. Und der zweite Speicher-
            // lauf muss ein Fixpunkt sein (keine schleichenden Aenderungen).
            string repoDebug = FindeRepoDebugVerzeichnis();
            if (repoDebug == null) return;

            var enc = Encoding.GetEncoding(1252);
            foreach (var datei in Directory.GetFiles(repoDebug, "*.ecf"))
            {
                var def = FormularDefinition.Lade(datei);
                string ziel = Path.Combine(_tempDir,
                    Path.GetFileName(datei) + ".roundtrip");
                def.SpeichereAls(ziel);

                string original = File.ReadAllText(datei, enc);
                string kopie = File.ReadAllText(ziel, enc);
                Assert.True(
                    OhneStilLeerzeichen(original) == OhneStilLeerzeichen(kopie),
                    "Roundtrip veraendert Inhalt: " + Path.GetFileName(datei));

                // Fixpunkt: erneut laden + speichern == erster Speicherstand
                var wieder = FormularDefinition.Lade(ziel);
                string ziel2 = ziel + "2";
                wieder.SpeichereAls(ziel2);
                Assert.True(kopie == File.ReadAllText(ziel2, enc),
                    "Zweiter Speicherlauf kein Fixpunkt: " + Path.GetFileName(datei));
            }
        }

        [Fact]
        public void FeldVerschiebenAendertNurDieBetroffeneZeile()
        {
            var pfad = SchreibeEcf("verschieben.ecf", BeispielXml,
                Encoding.GetEncoding(1252));
            string[] vorher = File.ReadAllLines(pfad);

            var def = FormularDefinition.Lade(pfad);
            var feld = def.Felder.First(f => f.Id == 1111);
            feld.Horizontal = 999;
            feld.Vertikal = 700;
            def.UebernehmeFeld(feld);
            def.Speichere();

            string[] nachher = File.ReadAllLines(pfad);
            Assert.Equal(vorher.Length, nachher.Length);
            int geaendert = 0;
            for (int i = 0; i < vorher.Length; i++)
                if (vorher[i] != nachher[i]) geaendert++;
            Assert.Equal(1, geaendert);   // nur die feld-Zeile selbst

            // Reload liefert die neuen Werte
            var neu = FormularDefinition.Lade(pfad);
            var feldNeu = neu.Felder.First(f => f.Id == 1111);
            Assert.Equal(999, feldNeu.Horizontal);
            Assert.Equal(700, feldNeu.Vertikal);
        }

        [Fact]
        public void FehlendeAttributeWerdenNurBeiAbweichungErgaenzt()
        {
            // Feld 60 hat weder ausrichtung noch anteil/nachkommaanteil --
            // Uebernehmen ohne Aenderung darf sie NICHT hinzufuegen.
            var pfad = SchreibeEcf("defaults.ecf", BeispielXml,
                Encoding.GetEncoding(1252));
            string vorher = File.ReadAllText(pfad);

            var def = FormularDefinition.Lade(pfad);
            var feld = def.Felder.First(f => f.Id == 60);
            def.UebernehmeFeld(feld);
            def.Speichere();

            Assert.Equal(vorher, File.ReadAllText(pfad));

            // ...wohl aber, wenn der Wert vom Default abweicht:
            feld.RechtsBuendig = false;   // Default waere rechtsbuendig
            def.UebernehmeFeld(feld);
            def.Speichere();
            var neu = FormularDefinition.Lade(pfad);
            Assert.False(neu.Felder.First(f => f.Id == 60).RechtsBuendig);
        }

        [Fact]
        public void NeuesFeldUndLoeschenRoundtrippen()
        {
            var pfad = SchreibeEcf("neu.ecf", BeispielXml,
                Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);
            int vorher = def.Felder.Count;

            var feld = def.NeuesFeld(4711);
            feld.Seite = 2;
            feld.Horizontal = 123;
            feld.Vertikal = 456;
            FormularDefinition.SetzeTyp(feld, "Freitext");
            feld.Name = "Testfeld";
            feld.Erweiterung = "Hallo $J";
            def.UebernehmeFeld(feld);
            def.Speichere();

            var neu = FormularDefinition.Lade(pfad);
            Assert.Equal(vorher + 1, neu.Felder.Count);
            var geladen = neu.Felder.First(f => f.Id == 4711);
            Assert.Equal(FormularFeldTyp.Freitext, geladen.Typ);
            Assert.Equal(123, geladen.Horizontal);
            Assert.Equal("Testfeld", geladen.Name);
            Assert.Equal("Hallo $J", geladen.Erweiterung);
            Assert.True(geladen.HatInhalt);

            // Loeschen
            neu.LoescheFeld(geladen);
            neu.Speichere();
            var nachLoeschen = FormularDefinition.Lade(pfad);
            Assert.Equal(vorher, nachLoeschen.Felder.Count);
            Assert.DoesNotContain(nachLoeschen.Felder, f => f.Id == 4711);
        }

        [Fact]
        public void AbschnitteAnlegenAendernLoeschen()
        {
            var pfad = SchreibeEcf("abschnitte.ecf", BeispielXml,
                Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);
            int vorher = def.Abschnitte.Count;

            var a = def.NeuerAbschnitt("Neuer Abschnitt", 3, 250);
            def.Speichere();

            var neu = FormularDefinition.Lade(pfad);
            Assert.Equal(vorher + 1, neu.Abschnitte.Count);
            var geladen = neu.Abschnitte.First(x => x.Name == "Neuer Abschnitt");
            Assert.Equal(3, geladen.Seite);
            Assert.Equal(250, geladen.Vertikal);

            geladen.Vertikal = 300;
            neu.UebernehmeAbschnitt(geladen);
            neu.LoescheAbschnitt(neu.Abschnitte.First(x => x.Name == "Betriebseinnahmen"));
            neu.Speichere();

            var nach = FormularDefinition.Lade(pfad);
            Assert.Equal(vorher, nach.Abschnitte.Count);
            Assert.Equal(300, nach.Abschnitte.First(x => x.Name == "Neuer Abschnitt").Vertikal);
            Assert.DoesNotContain(nach.Abschnitte, x => x.Name == "Betriebseinnahmen");
        }

        [Fact]
        public void AbschnittsSektionWirdBeiBedarfAngelegt()
        {
            var xml = @"<?xml version=""1.0"" encoding=""windows-1252"" ?>
<formular name=""Mini"" seiten=""1"" >
	<felder/>
</formular>";
            var pfad = SchreibeEcf("mini.ecf", xml, Encoding.GetEncoding(1252));
            var def = FormularDefinition.Lade(pfad);
            def.NeuerAbschnitt("Erster", 1, 100);
            def.Speichere();

            var neu = FormularDefinition.Lade(pfad);
            Assert.Single(neu.Abschnitte);
            Assert.Equal("Erster", neu.Abschnitte[0].Name);
        }
    }
}
