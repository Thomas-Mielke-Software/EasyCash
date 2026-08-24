// VorlagenBibliothekFelderTests.cs -- prüft die Konto-Spezifikationen der
// mitgelieferten Vorlagen gegen die ECHTEN .ecf-Formulare des Repos.
//
// Der Syntax-Test in VorlagenBibliothekTests sagt nur, dass eine
// Spezifikation PARSEBAR ist. Er hätte nicht gemerkt, dass eine Zeile ein
// Einnahmen- mit einem Ausgaben-Feld kombiniert -- so ein Konto lässt sich
// gar nicht anlegen (ErmittleFeldInfo bricht mit "E/A gemischt" ab), und
// zwar erst beim Anwender im Buchen-Dialog. Genau diese Semantik prüft
// dieser Test:
//
//   * jede Feld-Id existiert im genannten Formular,
//   * das Feld ist überhaupt zuweisbar (typ Einnahmen/Ausgaben, nicht
//     Summe -- Summenfelder rechnet das Formular selbst aus),
//   * alle Felder EINES Land-Blocks zeigen in dieselbe Richtung,
//   * und diese Richtung passt zur Buchungsart der Zeile bzw. der Basis.
//
// Die Formulare werden direkt aus den Quellordnern des Repos gelesen
// (EUeR/, UST/, UVA/), nicht aus Debug\ -- dort liegen nur die deutschen
// Formulare, die österreichischen sind eine eigene Setup-Komponente.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    public class VorlagenBibliothekFelderTests
    {
        [Fact]
        public void JedeSpezifikation_PasstZuDenEchtenFormularfeldern()
        {
            if (FindeRepoWurzel() == null)
                return;   // außerhalb des Repos gebaut: Test überspringen

            var formulare = LadeFormularfelder();

            // Kein stiller Durchmarsch: sind die Formulare da, die der
            // Katalog überhaupt anspricht? Sonst prüfte der Test nichts.
            foreach (var pflicht in new[] { "E/Ü-Rechnung", "Umsatzsteuer-Voranmeldung",
                                            "Beilage E1a", "Umsatzsteuer" })
                Assert.True(formulare.ContainsKey(pflicht),
                    $"Formular \"{pflicht}\" wurde im Repo nicht gefunden.");

            var fehler = new List<string>();

            foreach (var e in VorlagenBibliothek.Lade().SelectMany(a => a.AlleEintraege()))
            {
                var p = PresetXml.Importiere(e.VorlagenXml);
                PruefeSpezifikation(formulare, fehler, p.Konto,
                    !p.Ausgabe, e.Name + " / Basis");

                for (int i = 0; i < p.Zeilen.Count; i++)
                {
                    // Leere Zeilen-Art = wie die Basis
                    string art = (p.Zeilen[i].Art ?? "").Trim();
                    bool einnahme = art.Length == 0
                        ? !p.Ausgabe
                        : art.Equals("E", StringComparison.OrdinalIgnoreCase);
                    PruefeSpezifikation(formulare, fehler, p.Zeilen[i].Konto,
                        einnahme, $"{e.Name} / Zeile {i + 1}");
                }
            }

            Assert.True(fehler.Count == 0,
                "Feld-Spezifikationen passen nicht zu den Formularen:"
                + Environment.NewLine + string.Join(Environment.NewLine, fehler));
        }

        private static void PruefeSpezifikation(
            IReadOnlyDictionary<string, Dictionary<string, string>> formulare,
            List<string> fehler, string konto, bool istEinnahme, string wo)
        {
            if (!KontoFeldSpezifikation.IstSpezifikation(konto)) return;

            var spez = KontoFeldSpezifikation.Parse(konto, out string parseFehler);
            if (spez == null) { fehler.Add($"{wo}: {parseFehler}"); return; }

            foreach (var land in spez.Laender)
            {
                bool? richtung = null;
                foreach (var b in spez.FuerLand(land))
                {
                    if (!formulare.TryGetValue(b.Formular, out var felder))
                    {
                        fehler.Add($"{wo} [{land}]: Formular \"{b.Formular}\" gibt es im Repo nicht.");
                        continue;
                    }
                    if (!felder.TryGetValue(b.FeldId, out string typ))
                    {
                        fehler.Add($"{wo} [{land}]: \"{b.Formular}\" hat kein Feld {b.FeldId}.");
                        continue;
                    }

                    bool feldIstEinnahme;
                    if (typ == "Einnahmen")      feldIstEinnahme = true;
                    else if (typ == "Ausgaben")  feldIstEinnahme = false;
                    else
                    {
                        fehler.Add($"{wo} [{land}]: Feld {b.FeldId} in \"{b.Formular}\" "
                                 + $"ist typ=\"{typ}\" und damit nicht bebuchbar.");
                        continue;
                    }

                    if (richtung == null) richtung = feldIstEinnahme;
                    else if (richtung != feldIstEinnahme)
                        fehler.Add($"{wo} [{land}]: mischt Einnahmen- und Ausgaben-Felder "
                                 + "(so ein Konto lässt sich nicht anlegen).");

                    if (feldIstEinnahme != istEinnahme)
                        fehler.Add($"{wo} [{land}]: Feld {b.FeldId} in \"{b.Formular}\" ist ein "
                                 + (feldIstEinnahme ? "Einnahmen" : "Ausgaben")
                                 + "-Feld, die Buchung aber "
                                 + (istEinnahme ? "eine Einnahme." : "eine Ausgabe."));
                }
            }
        }

        // ------------------------------------------------------------------

        /// <summary>Formularname -> (Feld-Id -> typ), aus allen .ecf des Repos.
        /// Aufsteigend sortiert eingelesen, damit der jüngste Jahrgang gewinnt.</summary>
        private static IReadOnlyDictionary<string, Dictionary<string, string>> LadeFormularfelder()
        {
            var ergebnis = new Dictionary<string, Dictionary<string, string>>(
                StringComparer.OrdinalIgnoreCase);

            string repo = FindeRepoWurzel();
            if (repo == null) return ergebnis;

            Encoding cp1252;
            try { cp1252 = Encoding.GetEncoding(1252); }
            catch { cp1252 = Encoding.Default; }

            var dateien = new List<string>();
            foreach (var ordner in new[] { "EUeR", "UST", "UVA" })
            {
                string pfad = Path.Combine(repo, ordner);
                if (Directory.Exists(pfad))
                    dateien.AddRange(Directory.GetFiles(pfad, "*.ecf", SearchOption.AllDirectories));
            }
            dateien.Sort(StringComparer.OrdinalIgnoreCase);

            foreach (var datei in dateien)
            {
                XDocument doc;
                try
                {
                    // .ecf sind windows-1252; die XML-Deklaration sagt es zwar,
                    // aber der Reader soll nicht daran scheitern.
                    using (var leser = new StreamReader(datei, cp1252))
                        doc = XDocument.Load(leser);
                }
                catch { continue; }

                var root = doc.Root;
                string name = (string)root?.Attribute("name");
                if (string.IsNullOrEmpty(name)) continue;

                if (!ergebnis.TryGetValue(name, out var felder))
                    ergebnis[name] = felder = new Dictionary<string, string>(StringComparer.Ordinal);

                foreach (var feld in root.Descendants("feld"))
                {
                    string id  = (string)feld.Attribute("id");
                    string typ = (string)feld.Attribute("typ");
                    if (!string.IsNullOrEmpty(id) && !string.IsNullOrEmpty(typ))
                        felder[id] = typ;
                }
            }
            return ergebnis;
        }

        /// <summary>Vom Test-Bin-Verzeichnis aufwärts, bis EUeR und UVA
        /// nebeneinander liegen (Muster aus FormularDefinitionTests).</summary>
        private static string FindeRepoWurzel()
        {
            var dir = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
            for (int i = 0; i < 8 && dir != null; i++, dir = dir.Parent)
            {
                if (Directory.Exists(Path.Combine(dir.FullName, "EUeR"))
                    && Directory.Exists(Path.Combine(dir.FullName, "UVA")))
                    return dir.FullName;
            }
            return null;
        }
    }
}
