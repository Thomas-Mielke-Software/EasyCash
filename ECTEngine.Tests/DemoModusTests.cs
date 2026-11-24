// DemoModusTests.cs -- Beispieldaten des Demo-Modus (ECTEngine/Demo/DemoModus.cs)
//
// Die Beispieldaten sollen ein Einsteiger-taugliches, in sich stimmiges
// Buchungsjahr zeigen und für Screenshots taugen. Darum wird hier nicht nur
// geprüft, dass sie sich laden lassen, sondern auch, dass sie fachlich
// zusammenpassen:
//   * jede Kontenverknüpfung trifft ein bebuchbares Feld passender Richtung
//     im Formular des DEMOJAHRS (gegen die echten .ecf des Repos),
//   * der USt-Abgleich Voranmeldung/Erklärung findet nichts,
//   * jede Buchung nutzt Konten, Betriebe und Bestandskonten der ini,
//   * jede Buchungsgruppe lässt sich mit ihrer Vorlage reproduzieren,
//   * kein Bestandskonto rutscht unterjährig ins Minus.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    [Collection("EinstellungenCache")]   // nutzt den statischen Einstellungs-Cache
    public class DemoModusTests
    {
        private const int Jahr = 2025;

        private static void LadeIni(int jahr)
            => Einstellungen.LadeAusBridge(DemoModus.IniZuCache(DemoModus.ErzeugeIni(jahr)));

        // -----------------------------------------------------------------
        // ini
        // -----------------------------------------------------------------

        [Fact]
        public void Ini_EnthaeltPersonaKontenBetriebeUndBestandskonten()
        {
            LadeIni(Jahr);

            Assert.Equal("Federkiel", Einstellungen.Hole("[Persoenliche_Daten]name"));
            Assert.Equal("99/999/99999", Einstellungen.Hole("fsteuernummer"));
            Assert.Equal("0", Einstellungen.Hole("[Persoenliche_Daten]land"));
            Assert.Contains("Honorare Illustration", Einstellungen.EinnahmenKonten);
            Assert.Contains("Raumkosten Atelier", Einstellungen.AusgabenKonten);
            Assert.Equal(new[] { "Illustration", "Druckladen" }, Einstellungen.Betriebe.Select(b => b.Name));
            Assert.Equal(new[] { "Girokonto", "Kasse", "PayPal" }, Einstellungen.Bestandskonten.Select(b => b.Name));
            // Anfangssaldo des Demojahrs = Saldo des Vorjahres
            Assert.True(Einstellungen.Bestandskonten[0].Saldo.ContainsKey(Jahr - 1));
            // Tabs im Unternehmensart-Wert überleben das Einlesen
            Assert.Contains("\t", Einstellungen.Betriebe[1].Unternehmensart);
        }

        [Fact]
        public void Ini_ErsetztJahrUndIstInWindows1252Schreibbar()
        {
            string ini = DemoModus.ErzeugeIni(2031);
            Assert.DoesNotContain("{JAHR}", ini);
            Assert.DoesNotContain("{VORJAHR}", ini);
            Assert.Contains("Saldo2030=", ini);

            // Alles muss sich verlustfrei in cp1252 darstellen lassen
            var cp = Encoding.GetEncoding(1252, EncoderFallback.ExceptionFallback, DecoderFallback.ExceptionFallback);
            cp.GetBytes(ini);

            string pfad = Path.Combine(Path.GetTempPath(), "ect-demo-test-" + Guid.NewGuid().ToString("N") + ".ini");
            try
            {
                DemoModus.SchreibeIni(pfad, 2031);
                Assert.Equal(DemoModus.ErzeugeIni(2031).Replace("\r\n", "\n"),
                             File.ReadAllText(pfad, cp).Replace("\r\n", "\n"));
                // eine vorhandene Datei wird nie überschrieben
                Assert.Throws<IOException>(() => DemoModus.SchreibeIni(pfad, 2031));
            }
            finally { File.Delete(pfad); }
        }

        [Fact]
        public void Ini_VorlagenSindVollstaendigMitFesterKennung()
        {
            LadeIni(Jahr);
            var gruppen = Einstellungen.Presets.Skip(10).Take(8).ToList();
            Assert.All(gruppen, p => Assert.False(p.IstLeer));
            Assert.All(gruppen, p => Assert.Equal(32, p.Id.Length));
            Assert.All(gruppen, p => Assert.Empty(VorlagenFelder.Pruefe(p)));
        }

        [Fact]
        public void Ini_KontenverknuepfungenPassenZuDenFormularenDesDemojahrs()
        {
            string repo = FindeRepoWurzel();
            if (repo == null) return;

            var dateien = DeutscheFormulare(repo);
            int demojahr = DemoModus.ErmittleJahr(dateien);
            var felder = FelderDesJahres(dateien, demojahr);
            foreach (var name in DemoModus.JahresFormulare)
                Assert.True(felder.ContainsKey(name), $"Formular {name} {demojahr} fehlt im Repo.");

            LadeIni(demojahr);
            EUKonten.Lade();
            var fehler = new List<string>();
            foreach (var konto in EUKonten.AlleKontenAusCache())
                foreach (var z in konto.Feldzuweisungen)
                {
                    if (!felder.TryGetValue(z.Key, out var formular))
                    {
                        fehler.Add($"{konto.Name}: Formular \"{z.Key}\" unbekannt");
                        continue;
                    }
                    if (!formular.TryGetValue(z.Value, out string typ))
                        fehler.Add($"{konto.Name}: {z.Key} {demojahr} hat kein bebuchbares Feld {z.Value}");
                    else if (typ != (konto.IstEinnahme ? "Einnahmen" : "Ausgaben"))
                        fehler.Add($"{konto.Name}: {z.Key} Feld {z.Value} ist {typ}");
                }
            Assert.True(fehler.Count == 0, string.Join(Environment.NewLine, fehler));
        }

        [Fact]
        public void Ini_UstAbgleichFindetNichts()
        {
            string repo = FindeRepoWurzel();
            if (repo == null) return;
            var dateien = DeutscheFormulare(repo);
            int demojahr = DemoModus.ErmittleJahr(dateien);

            LadeIni(demojahr);
            var abgleich = UstAbgleich.Lade(demojahr, dateien);
            var befunde = EUKonten.AlleKontenAusCache()
                .Select(k => new { k.Name, Befund = abgleich.Pruefe(k.Feldzuweisungen, k.IstEinnahme) })
                .Where(x => x.Befund != UstAbgleichBefund.Stimmig)
                .Select(x => x.Name + ": " + x.Befund).ToList();
            Assert.True(befunde.Count == 0, string.Join(Environment.NewLine, befunde));
        }

        // -----------------------------------------------------------------
        // Demojahr
        // -----------------------------------------------------------------

        [Fact]
        public void Demojahr_IstDerAeltesteGemeinsameStand()
        {
            string dir = Path.Combine(Path.GetTempPath(), "ect-demo-jahr-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(dir);
            try
            {
                void Formular(string datei, string name, string anzeige)
                    => File.WriteAllText(Path.Combine(dir, datei),
                        $"<?xml version=\"1.0\" encoding=\"windows-1252\" ?>\r\n<formular name=\"{name}\" anzeigename=\"{anzeige}\" seiten=\"1\"></formular>",
                        Encoding.GetEncoding(1252));
                Formular("EUeR-d-2024.ecf", "E/Ü-Rechnung", "E/Ü-Rechnung 2024");
                Formular("EUeR-d-2025.ecf", "E/Ü-Rechnung", "E/Ü-Rechnung 2025");
                Formular("uva-d-2026-q1.ecf", "Umsatzsteuer-Voranmeldung", "Umsatzsteuer-Voranmeldung 2026 1. Quartal (D)");
                Formular("ust-d-2026.ecf", "Umsatzsteuererklärung", "Umsatzsteuererklärung 2026");
                // alte Erklärung, die intern "Umsatzsteuer-Voranmeldung" heißt: zählt nicht als Voranmeldung
                Formular("ust-d-2027.ecf", "Umsatzsteuer-Voranmeldung", "Umsatzsteuererklärung 2027");
                Assert.Equal(2025, DemoModus.ErmittleJahr(dir));
            }
            finally { Directory.Delete(dir, true); }

            Assert.Equal(DateTime.Today.Year - 1, DemoModus.ErmittleJahr(new string[0]));
        }

        [Fact]
        public void Demojahr_AusDenRepoFormularen()
        {
            string repo = FindeRepoWurzel();
            if (repo == null) return;
            int jahr = DemoModus.ErmittleJahr(DeutscheFormulare(repo));
            Assert.InRange(jahr, 2025, DateTime.Today.Year + 1);
        }

        // -----------------------------------------------------------------
        // Buchungsdokument
        // -----------------------------------------------------------------

        [Fact]
        public void Dokument_LaedtUndIstVielfaeltig()
        {
            var doc = DemoModus.ErzeugeDokument(Jahr);
            Assert.Equal(Jahr, doc.Jahr);
            Assert.InRange(doc.Buchungen.Count, 100, 400);
            Assert.True(doc.Dauerbuchungen.Count >= 3);
            Assert.Contains(doc.Buchungen, b => b.HatAfA && b.AfaNr == 1);
            Assert.Contains(doc.Buchungen, b => b.HatAfA && b.AfaNr > 1);
            Assert.Contains(doc.Buchungen, b => b.IstSplitBuchung);
            Assert.Contains(doc.Buchungen, b => b.BruttoBetrag.MwstPromille == 7000);
            Assert.Contains(doc.Buchungen, b => b.Betrieb == "Druckladen");
            Assert.Contains(doc.Buchungen, b => b.Art == Buchungsart.Einnahme && b.Konto == "Leistungen an EU-Unternehmen");
            // jeder Monat hat Buchungen (Screenshots der Monatsansichten)
            for (int m = 1; m <= 12; m++)
                Assert.Contains(doc.Buchungen, b => b.Datum.Month == m);
            Assert.All(doc.Buchungen, b => Assert.Equal(Jahr, b.Datum.Year));
        }

        [Fact]
        public void Dokument_NutztNurKontenBetriebeUndBestandskontenDerIni()
        {
            LadeIni(Jahr);
            var doc = DemoModus.ErzeugeDokument(Jahr);
            var betriebe = Einstellungen.Betriebe.Select(b => b.Name).ToList();
            var bks = Einstellungen.Bestandskonten.Select(b => b.Name).ToList();
            var fehler = new List<string>();
            foreach (var b in doc.Buchungen)
            {
                var konten = b.Art == Buchungsart.Einnahme ? Einstellungen.EinnahmenKonten : Einstellungen.AusgabenKonten;
                if (!konten.Contains(b.Konto)) fehler.Add($"{b.Beschreibung}: Konto \"{b.Konto}\"");
                if (b.Betrieb.Length > 0 && !betriebe.Contains(b.Betrieb)) fehler.Add($"{b.Beschreibung}: Betrieb \"{b.Betrieb}\"");
                if (b.Bestandskonto.Length > 0 && !bks.Contains(b.Bestandskonto)) fehler.Add($"{b.Beschreibung}: Bestandskonto \"{b.Bestandskonto}\"");
            }
            foreach (var d in doc.Dauerbuchungen)
                if (!Einstellungen.AusgabenKonten.Contains(d.Konto) && !Einstellungen.EinnahmenKonten.Contains(d.Konto))
                    fehler.Add($"Dauerbuchung {d.Beschreibung}: Konto \"{d.Konto}\"");
            Assert.True(fehler.Count == 0, string.Join(Environment.NewLine, fehler));
        }

        [Fact]
        public void Dokument_BuchungsgruppenSindMitIhrerVorlageReproduzierbar()
        {
            LadeIni(Jahr);
            var doc = DemoModus.ErzeugeDokument(Jahr);
            var gruppen = doc.Buchungen.Where(b => b.GruppenUuid != null).GroupBy(b => b.GruppenUuid).ToList();
            Assert.NotEmpty(gruppen);

            foreach (var g in gruppen)
            {
                var basis = g.Single(b => b.GruppenRolle == 0);
                int slot = Einstellungen.FindeVorlagenSlot(basis);
                Assert.True(slot >= 0, "Vorlage nicht gefunden: " + basis.Beschreibung);
                var p = Einstellungen.Presets[slot];

                // Eingaben wie beim Bearbeiten aus der Basis zurückholen
                var gemerkt = basis.VorlagenFeldWerte();
                var felder = p.Felder.Select(f => VorlagenFelder.Werte(f,
                    gemerkt.TryGetValue(f.Name, out string w) ? w : VorlagenFelder.StandardWert(f))).ToList();
                var eingabe = basis.Clone();
                eingabe.BruttoBetrag = new Betrag(basis.VorlagenEingabeBetrag ?? basis.BruttoBetrag.BruttoWert,
                    basis.BruttoBetrag.MwstProzent);
                if (basis.VorlagenEingabeText != null) eingabe.Beschreibung = basis.VorlagenEingabeText;

                var berechnung = BuchungsgruppenRechner.Berechne(p, eingabe, felder);
                Assert.False(berechnung.HatFehler, basis.Beschreibung);
                Assert.Equal(basis.BruttoBetrag.BruttoWert, berechnung.BasisGebuchtBrutto);
                Assert.Equal(basis.Beschreibung, berechnung.BasisBeschreibung);

                var soll = berechnung.GebuchteZeilen.OrderBy(z => z.Index)
                    .Select(z => $"{z.Index}|{z.Konto}|{z.Brutto:0.00}|{z.MwstPromille}").ToList();
                var ist = g.Where(b => b.GruppenRolle > 0).OrderBy(b => b.GruppenRolle)
                    .Select(b => $"{b.GruppenRolle}|{b.Konto}|{b.BruttoBetrag.BruttoWert:0.00}|{b.BruttoBetrag.MwstPromille}").ToList();
                Assert.Equal(soll, ist);
            }
        }

        [Fact]
        public void Dokument_BestandskontenBleibenImPlus()
        {
            LadeIni(Jahr);
            var doc = DemoModus.ErzeugeDokument(Jahr);
            foreach (var bk in Einstellungen.Bestandskonten)
            {
                decimal saldo = bk.Saldo.TryGetValue(Jahr - 1, out decimal s) ? s : 0m;
                foreach (var b in doc.Buchungen.Where(x => x.Bestandskonto == bk.Name).OrderBy(x => x.Datum))
                {
                    saldo += b.Art == Buchungsart.Einnahme ? b.BruttoBetrag.BruttoWert : -b.BruttoBetrag.BruttoWert;
                    Assert.True(saldo >= 0m, $"{bk.Name} am {b.Datum:d}: {saldo} ({b.Beschreibung})");
                }
            }
        }

        [Fact]
        public void Dokument_LaesstSichAufEinAnderesJahrVerschieben()
        {
            var doc = DemoModus.ErzeugeDokument(2031);
            Assert.Equal(2031, doc.Jahr);
            Assert.All(doc.Buchungen, b => Assert.Equal(2031, b.Datum.Year));
            Assert.Contains(doc.Buchungen, b => b.Beschreibung == "USt-Vorauszahlung 4. Quartal 2030");
            Assert.Contains(doc.Buchungen, b => b.Belegnummer == "E-17/2030");
            Assert.Contains(doc.Buchungen, b => b.Beschreibung == "Atelier-Miete 03/2031");
            Assert.NotEqual("", UstVorauszahlungen.Hole(doc, UstVorauszahlungen.QuartalZeitraum(1)));
            Assert.All(doc.Dauerbuchungen, d => Assert.Equal(new DateTime(2031, 12, 31), d.AusgefuehrtBis));
            Assert.Contains(doc.Dauerbuchungen, d => d.Beschreibung.Contains("$J"));   // Platzhalter bleiben
        }

        [Fact]
        public void VerschiebeJahr_MachtAusDem29FebruarDen28()
        {
            var doc = new BuchungsDocument { Jahr = 2024 };
            doc.Buchungen.Add(new Buchung { Datum = new DateTime(2024, 2, 29), Beschreibung = "Schalttag 2024, Kundennummer 4711" });
            DemoModus.VerschiebeJahr(doc, 2025);
            Assert.Equal(new DateTime(2025, 2, 28), doc.Buchungen[0].Datum);
            Assert.Equal("Schalttag 2025, Kundennummer 4711", doc.Buchungen[0].Beschreibung);
        }

        // -----------------------------------------------------------------
        // Formulare aus dem Repo
        // -----------------------------------------------------------------

        private static string FindeRepoWurzel()
        {
            var dir = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
            for (int i = 0; i < 8 && dir != null; i++, dir = dir.Parent)
                if (Directory.Exists(Path.Combine(dir.FullName, "EUeR"))
                    && Directory.Exists(Path.Combine(dir.FullName, "UVA"))
                    && Directory.Exists(Path.Combine(dir.FullName, "UST")))
                    return dir.FullName;
            return null;
        }

        /// <summary>Die deutschen Formulare -- wie ein Programmverzeichnis mit
        /// der Setup-Komponente "deutsche Formulare".</summary>
        private static List<string> DeutscheFormulare(string repo)
        {
            var dateien = new List<string>();
            foreach (var ordner in new[] { "EUeR", "UST", "UVA" })
                foreach (var datei in Directory.GetFiles(Path.Combine(repo, ordner), "*.ecf", SearchOption.AllDirectories))
                {
                    string unter = Path.GetFileName(Path.GetDirectoryName(datei));
                    if (!unter.StartsWith("AT", StringComparison.OrdinalIgnoreCase)) dateien.Add(datei);
                }
            return dateien;
        }

        /// <summary>Formularname -> (Feld-Id -> typ) der bebuchbaren, nicht
        /// veralteten Felder, je Formular die Datei des gegebenen Jahres.</summary>
        private static Dictionary<string, Dictionary<string, string>> FelderDesJahres(List<string> dateien, int jahr)
        {
            var cp1252 = Encoding.GetEncoding(1252);
            var ergebnis = new Dictionary<string, Dictionary<string, string>>();
            foreach (var datei in dateien)
            {
                if (!UstAbgleich.LiesFormularkopf(datei, out string name, out string anzeige)) continue;
                if (!DemoModus.JahresFormulare.Contains(name) || ergebnis.ContainsKey(name)) continue;
                if (name == UstAbgleich.FormularUva && !anzeige.StartsWith(name + " ")) continue;
                if (UstAbgleich.JahrAusAnzeigename(anzeige) != jahr) continue;

                var x = XDocument.Parse(File.ReadAllText(datei, cp1252));
                ergebnis[name] = x.Root.Element("felder").Elements("feld")
                    .Where(f => ((string)f.Attribute("typ")) is "Einnahmen" or "Ausgaben")
                    .Where(f => (string)f.Attribute("veraltet") != "ja")
                    .GroupBy(f => (string)f.Attribute("id"))
                    .ToDictionary(g => g.Key, g => (string)g.First().Attribute("typ"));
            }
            return ergebnis;
        }
    }
}
