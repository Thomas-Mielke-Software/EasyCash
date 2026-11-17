// UstAbgleichTests.cs -- Abgleich der Kontenzuweisungen zwischen
// USt-Voranmeldung und Umsatzsteuererklärung (ECTEngine/UstAbgleich.cs).
//
// Port des v3-Testrahmens (ECTElster\Testrahmen\ust-abgleich\test.cpp,
// 82 Prüfungen) -- dieselben Fälle gegen dieselben ECHTEN Formulare, damit
// v3 und v4 nachweislich gleich entscheiden. Die Formulare kommen direkt aus
// den Quellordnern des Repos (UST/, UVA/), wie in
// VorlagenBibliothekFelderTests; außerhalb des Repos werden die Tests
// übersprungen.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    public class UstAbgleichTests
    {
        private const string UVA = UstAbgleich.FormularUva;
        private const string UST = UstAbgleich.FormularUst;
        private const UstMitfuehrung.ArtTyp NICHTS            = UstMitfuehrung.ArtTyp.Nichts;
        private const UstMitfuehrung.ArtTyp PARTNER_GESETZT   = UstMitfuehrung.ArtTyp.PartnerGesetzt;
        private const UstMitfuehrung.ArtTyp PARTNER_WAEHLEN   = UstMitfuehrung.ArtTyp.PartnerWaehlen;
        private const UstMitfuehrung.ArtTyp PARTNER_ENTFERNEN = UstMitfuehrung.ArtTyp.PartnerEntfernen;
        private const UstMitfuehrung.ArtTyp WIDERSPRUCH       = UstMitfuehrung.ArtTyp.Widerspruch;

        // -----------------------------------------------------------------
        // Formulare aus dem Repo
        // -----------------------------------------------------------------
        private static string FindeRepoWurzel()
        {
            var dir = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
            for (int i = 0; i < 8 && dir != null; i++, dir = dir.Parent)
                if (Directory.Exists(Path.Combine(dir.FullName, "UST"))
                    && Directory.Exists(Path.Combine(dir.FullName, "UVA")))
                    return dir.FullName;
            return null;
        }

        /// <summary>Alle Erklärungs- und Voranmeldungsformulare des Repos
        /// (DE und AT, alle Jahre) -- wie ein Programmverzeichnis mit allen
        /// Komponenten.</summary>
        private static List<string> AlleFormulare(string repo)
        {
            var dateien = new List<string>();
            foreach (var ordner in new[] { "UST", "UVA" })
                dateien.AddRange(Directory.GetFiles(Path.Combine(repo, ordner), "*.ecf", SearchOption.AllDirectories));
            dateien.Sort(StringComparer.OrdinalIgnoreCase);
            return dateien;
        }

        private static string Datei(string repo, string ordner, string name)
            => Path.Combine(repo, ordner, name);

        private static Dictionary<string, string> Zuweisung(int uva, int ust)
        {
            // ein fremder Key davor, wie in echten Daten
            var d = new Dictionary<string, string> { ["E/Ü-Rechnung"] = "1112" };
            if (uva != 0) UstAbgleich.SchreibeZuweisung(d, UVA, uva);
            if (ust != 0) UstAbgleich.SchreibeZuweisung(d, UST, ust);
            return d;
        }

        // führt eine Änderung aus (Zuweisung für ust ? Erklärung : Voranmeldung von alt auf neu)
        private static UstMitfuehrung Aendere(UstAbgleich a, Dictionary<string, string> d, bool einnahme,
            bool ust, int alt, int neu)
        {
            UstAbgleich.SchreibeZuweisung(d, ust ? UST : UVA, neu);
            return a.NachZuweisung(d, einnahme, ust, alt);
        }

        private static int Lies(Dictionary<string, string> d, string formular)
            => UstAbgleich.LiesZuweisung(d, formular);

        // -----------------------------------------------------------------
        // Formularkopf und Jahr
        // -----------------------------------------------------------------
        [Fact]
        public void Formularkopf_UndJahr()
        {
            Assert.Equal(2026, UstAbgleich.JahrAusAnzeigename("Umsatzsteuer-Voranmeldung 2026 Januar (D)"));
            Assert.Equal(0, UstAbgleich.JahrAusAnzeigename("ohne Jahr 123"));
            Assert.Equal(0, UstAbgleich.JahrAusAnzeigename(null));

            string repo = FindeRepoWurzel();
            if (repo == null) return;

            Assert.True(UstAbgleich.LiesFormularkopf(Datei(repo, @"UST\D-2026", "USt-d-2026.ecf"), out var name, out var anzeige));
            Assert.Equal(UST, name);
            Assert.Equal(2026, UstAbgleich.JahrAusAnzeigename(anzeige));
            Assert.True(UstAbgleich.LiesFormularkopf(Datei(repo, @"UST\D-2024", "USt-d-2024.ecf"), out name, out _));
            Assert.Equal(UVA, name);   // das alte Erklärungsformular heißt intern wie die Voranmeldung
        }

        // -----------------------------------------------------------------
        // Auswahl der Formulare
        // -----------------------------------------------------------------
        [Fact]
        public void Lade_WaehltErklaerungDesJahresSonstNeueste()
        {
            string repo = FindeRepoWurzel();
            if (repo == null) return;
            var formulare = AlleFormulare(repo);

            var a = UstAbgleich.Lade(2025, formulare);
            Assert.True(a.IstAktiv);
            Assert.EndsWith("USt-d-2025.ecf", a.UstFormulardatei, StringComparison.OrdinalIgnoreCase);
            Assert.Equal(2025, a.UstJahr);
            Assert.Contains("uva-d-2025-", Path.GetFileName(a.UvaFormulardatei), StringComparison.OrdinalIgnoreCase);
            Assert.Equal(new[] { 752, 2371, 2372, 2374 }, a.UstKandidaten(43, true));   // 2025 ohne 2373

            a = UstAbgleich.Lade(2019, formulare);   // kein Formular des Jahres: das neueste mit uvafeld
            Assert.True(a.IstAktiv);
            Assert.EndsWith("USt-d-2026.ecf", a.UstFormulardatei, StringComparison.OrdinalIgnoreCase);

            a = UstAbgleich.Lade(2026, formulare);
            Assert.True(a.IstAktiv);
            Assert.EndsWith("USt-d-2026.ecf", a.UstFormulardatei, StringComparison.OrdinalIgnoreCase);
            Assert.Contains("uva-d-2026-", Path.GetFileName(a.UvaFormulardatei), StringComparison.OrdinalIgnoreCase);
        }

        [Fact]
        public void Lade_OhnePassendesFormular_KeinAbgleich()
        {
            string repo = FindeRepoWurzel();
            if (repo == null) return;

            // nur Formulare bis 2024 und Österreich: keines hat uvafeld
            var alt = AlleFormulare(repo).Where(f =>
            {
                string n = Path.GetFileName(f).ToLowerInvariant();
                return !n.Contains("2025") && !n.Contains("2026");
            }).ToList();
            Assert.False(UstAbgleich.Lade(2024, alt).IstAktiv);

            var leer = UstAbgleich.LadeAusDateien(Datei(repo, @"UST\D-2024", "USt-d-2024.ecf"),
                                                  Datei(repo, @"UVA\D-2026", "uva-d-2026-01.ecf"));
            Assert.False(leer.IstAktiv);
            Assert.Equal(UstAbgleichBefund.Stimmig, leer.Pruefe(Zuweisung(41, 744), true));
            var m = leer.NachZuweisung(Zuweisung(41, 0), true, false, 0);
            Assert.Equal(NICHTS, m.Art);

            Assert.False(UstAbgleich.Lade(2026, new string[0]).IstAktiv);
            Assert.False(UstAbgleich.Inaktiv().IstAktiv);
        }

        [Fact]
        public void Korrespondenz_AnzahlUvafelder()
        {
            string repo = FindeRepoWurzel();
            if (repo == null) return;
            var formulare = AlleFormulare(repo);
            // 2026: 47 Felder mit uvafeld, 2025: 46 (UST-ABGLEICH.md, Abschnitt 3)
            Assert.Equal(47, UstAbgleich.Lade(2026, formulare).UstFelder.Count(f => f.UvaFeld != 0));
            Assert.Equal(46, UstAbgleich.Lade(2025, formulare).UstFelder.Count(f => f.UvaFeld != 0));
        }

        // -----------------------------------------------------------------
        // Korrespondenz (2026)
        // -----------------------------------------------------------------
        private static UstAbgleich Lade2026()
        {
            string repo = FindeRepoWurzel();
            if (repo == null) return null;
            var a = UstAbgleich.Lade(2026, AlleFormulare(repo));
            Assert.True(a.IstAktiv);
            return a;
        }

        [Fact]
        public void Korrespondenz()
        {
            var a = Lade2026();
            if (a == null) return;

            Assert.Equal(41, a.UvaFeldZu(741));
            Assert.Equal(0, a.UvaFeldZu(178));     // gibt es nur in der Erklärung
            Assert.Equal(0, a.UvaFeldZu(177));     // Summenfeld, nicht zuweisbar
            Assert.Equal(66, a.UvaFeldZu(1320));
            Assert.Equal(new[] { 741 }, a.UstKandidaten(41, true));
            Assert.Equal(new[] { 752, 2371, 2372, 2373, 2374 }, a.UstKandidaten(43, true));
            Assert.Equal(new[] { 286, 287, 236, 240 }.OrderBy(x => x), a.UstKandidaten(48, true).OrderBy(x => x));
            Assert.Equal(new[] { 3571, 3572, 3573 }, a.UstKandidaten(64, false).OrderBy(x => x));
            Assert.Equal(new[] { 781 }, a.UstKandidaten(89, false));
            Assert.Empty(a.UstKandidaten(89, true));    // falscher Kontentyp
            Assert.Empty(a.UstKandidaten(79, true));    // veraltetes Feld der Voranmeldung
            Assert.True(a.UvaFeldZuweisbar(41, true));
            Assert.False(a.UvaFeldZuweisbar(41, false));
            Assert.False(a.UvaFeldZuweisbar(79, true));  // veraltet
            Assert.False(a.UvaFeldZuweisbar(81, true));  // Summenfeld

            Assert.StartsWith("Feld 741: ", a.FeldText(true, 741));
            Assert.Equal("", a.FeldText(true, 0));
            Assert.Equal("Feld 9999 (im Formular nicht vorhanden)", a.FeldText(true, 9999));
        }

        // -----------------------------------------------------------------
        // Prüfung eines Paars
        // -----------------------------------------------------------------
        [Theory]
        [InlineData(0, 0, true, UstAbgleichBefund.Stimmig)]
        [InlineData(41, 0, true, UstAbgleichBefund.UstFehlt)]
        [InlineData(43, 0, true, UstAbgleichBefund.UstMehrdeutig)]
        [InlineData(0, 741, true, UstAbgleichBefund.UvaFehlt)]
        [InlineData(0, 178, true, UstAbgleichBefund.Stimmig)]
        [InlineData(41, 741, true, UstAbgleichBefund.Stimmig)]
        [InlineData(43, 752, true, UstAbgleichBefund.Stimmig)]
        [InlineData(43, 2371, true, UstAbgleichBefund.Stimmig)]
        [InlineData(41, 744, true, UstAbgleichBefund.Widerspruch)]
        [InlineData(41, 178, true, UstAbgleichBefund.Widerspruch)]
        [InlineData(79, 0, true, UstAbgleichBefund.Stimmig)]        // veraltetes Feld ohne Gegenstück
        [InlineData(79, 178, true, UstAbgleichBefund.Stimmig)]      // beide ohne Gegenstück
        [InlineData(999, 741, true, UstAbgleichBefund.UvaFehlt)]    // unbekanntes Feld zählt als nicht zugewiesen
        [InlineData(41, 9999, true, UstAbgleichBefund.UstFehlt)]
        [InlineData(66, 1320, false, UstAbgleichBefund.Stimmig)]
        [InlineData(66, 0, false, UstAbgleichBefund.UstFehlt)]
        [InlineData(64, 0, false, UstAbgleichBefund.UstMehrdeutig)]
        [InlineData(89, 781, false, UstAbgleichBefund.Stimmig)]
        [InlineData(1189, 1781, true, UstAbgleichBefund.Stimmig)]
        public void PruefePaar(int uva, int ust, bool einnahme, UstAbgleichBefund erwartet)
        {
            var a = Lade2026();
            if (a == null) return;
            Assert.Equal(erwartet, a.PruefePaar(uva, ust, einnahme));
        }

        // -----------------------------------------------------------------
        // Feldzuweisung lesen/schreiben, Vermerk "so gewollt"
        // -----------------------------------------------------------------
        [Fact]
        public void Feldzuweisung_UndVermerk()
        {
            var a = Lade2026();
            if (a == null) return;

            var d = new Dictionary<string, string>();
            UstAbgleich.SchreibeZuweisung(d, UVA, 41);
            Assert.Equal("41", d[UVA]);
            Assert.Equal(41, Lies(d, UVA));
            Assert.Equal(0, Lies(d, UST));
            Assert.Equal(UstAbgleichBefund.UstFehlt, a.Pruefe(d, true));
            UstAbgleich.SetzeBelassen(d, true);
            Assert.True(UstAbgleich.IstBelassen(d, 41, 0));
            Assert.Equal("41/0", d[UstAbgleich.KeyBelassen]);
            Assert.Equal(UstAbgleichBefund.Stimmig, a.Pruefe(d, true));
            UstAbgleich.SchreibeZuweisung(d, UST, 744);   // Paar geändert: der Vermerk gilt nicht mehr
            Assert.Equal(UstAbgleichBefund.Widerspruch, a.Pruefe(d, true));
            UstAbgleich.SetzeBelassen(d, false);
            Assert.False(d.ContainsKey(UstAbgleich.KeyBelassen));
            Assert.Equal(UstAbgleichBefund.Stimmig, a.Pruefe(new Dictionary<string, string>(), true));
            Assert.Equal(UstAbgleichBefund.Stimmig, a.Pruefe(null, true));

            d = Zuweisung(41, 741);
            Assert.True(Lies(d, UVA) == 41 && Lies(d, UST) == 741 && Lies(d, "E/Ü-Rechnung") == 1112);

            UstAbgleich.SchreibeZuweisung(d, UST, 0);     // 0 = Zuweisung entfernen
            Assert.False(d.ContainsKey(UST));
            d[UST] = "0";                                 // so schreibt v3 ein Entfernen
            Assert.Equal(0, Lies(d, UST));
            Assert.Equal(UstAbgleichBefund.UstFehlt, a.Pruefe(d, true));
        }

        /// <summary>v3 und v4 teilen die easyct.ini: der Vermerk muss den Weg
        /// über EUKonto.Feldzuweisungen und das ECT-Pipe-Format überstehen.</summary>
        [Fact]
        public void Feldzuweisung_Pipeformat_Roundtrip()
        {
            string v3 = "ECT|E/Ü-Rechnung=1112|" + UVA + "=41|" + UST + "=744|" + UstAbgleich.KeyBelassen + "=41/744||";
            var d = new Dictionary<string, string>();
            foreach (var e in ErweiterungStore.AusPipeFormat(v3).Alle())
                if (e.Ns == "ECT") d[e.Key] = e.Wert;
            Assert.True(UstAbgleich.IstBelassen(d, 41, 744));

            var store = new ErweiterungStore();
            foreach (var kv in d) store.Setze("ECT", kv.Key, kv.Value);
            var zurueck = ErweiterungStore.AusPipeFormat(store.ZuPipeFormat());
            Assert.Equal("41/744", zurueck.Hole("ECT", UstAbgleich.KeyBelassen));
            Assert.Equal("744", zurueck.Hole("ECT", UST));
        }

        // -----------------------------------------------------------------
        // Mitführen nach einer Änderung
        // -----------------------------------------------------------------
        [Fact]
        public void NachZuweisung_GegenstueckFrei()
        {
            var a = Lade2026();
            if (a == null) return;

            var d = Zuweisung(0, 0);   // neues Konto, Voranmeldung zugewiesen: Erklärung folgt still
            var m = Aendere(a, d, true, false, 0, 41);
            Assert.Equal(PARTNER_GESETZT, m.Art);
            Assert.Equal(741, Lies(d, UST));
            Assert.True(m.PartnerIstUst);
            Assert.Equal(UstAbgleichBefund.Stimmig, a.Pruefe(d, true));

            d = Zuweisung(0, 0);       // mehrdeutig: nichts eintragen, wählen lassen
            m = Aendere(a, d, true, false, 0, 43);
            Assert.Equal(PARTNER_WAEHLEN, m.Art);
            Assert.Equal(5, m.Kandidaten.Count);
            Assert.Equal(0, Lies(d, UST));

            d = Zuweisung(0, 0);       // Erklärung zuerst: Voranmeldung folgt still
            m = Aendere(a, d, true, true, 0, 741);
            Assert.Equal(PARTNER_GESETZT, m.Art);
            Assert.Equal(41, Lies(d, UVA));
            Assert.False(m.PartnerIstUst);

            d = Zuweisung(0, 0);       // in dieser Richtung ist auch Feld 43 eindeutig
            m = Aendere(a, d, true, true, 0, 2371);
            Assert.Equal(PARTNER_GESETZT, m.Art);
            Assert.Equal(43, Lies(d, UVA));

            d = Zuweisung(0, 0);       // Feld nur in der Erklärung
            m = Aendere(a, d, true, true, 0, 178);
            Assert.Equal(NICHTS, m.Art);
            Assert.Equal(0, Lies(d, UVA));

            d = Zuweisung(41, 0);      // Altbestand vor der Übernahme: Voranmeldung umgehängt, Erklärung noch leer
            m = Aendere(a, d, true, false, 41, 44);
            Assert.Equal(PARTNER_GESETZT, m.Art);
            Assert.Equal(744, Lies(d, UST));
        }

        [Fact]
        public void NachZuweisung_Gleichschritt()
        {
            var a = Lade2026();
            if (a == null) return;

            var d = Zuweisung(41, 741);   // im Gleichschritt: Erklärung folgt der Voranmeldung
            var m = Aendere(a, d, true, false, 41, 44);
            Assert.Equal(PARTNER_GESETZT, m.Art);
            Assert.Equal(744, Lies(d, UST));

            d = Zuweisung(41, 741);       // ... und umgekehrt
            m = Aendere(a, d, true, true, 741, 744);
            Assert.Equal(PARTNER_GESETZT, m.Art);
            Assert.Equal(44, Lies(d, UVA));

            d = Zuweisung(41, 741);       // Zuweisung entfernt: nachfragen, nichts still löschen und nichts zurückfüllen
            m = Aendere(a, d, true, false, 41, 0);
            Assert.Equal(PARTNER_ENTFERNEN, m.Art);
            Assert.Equal(741, Lies(d, UST));
            Assert.Equal(0, Lies(d, UVA));

            d = Zuweisung(41, 741);       // neues Feld mehrdeutig
            m = Aendere(a, d, true, false, 41, 43);
            Assert.Equal(PARTNER_WAEHLEN, m.Art);
            Assert.Equal(741, Lies(d, UST));

            d = Zuweisung(41, 741);       // neues Feld ohne Gegenstück
            m = Aendere(a, d, true, true, 741, 178);
            Assert.Equal(PARTNER_ENTFERNEN, m.Art);
            Assert.Equal(41, Lies(d, UVA));

            d = Zuweisung(43, 752);       // Wechsel innerhalb der Felder zu 43: bleibt stimmig
            m = Aendere(a, d, true, true, 752, 2371);
            Assert.Equal(NICHTS, m.Art);
            Assert.Equal(43, Lies(d, UVA));
        }

        [Fact]
        public void NachZuweisung_Widerspruch()
        {
            var a = Lade2026();
            if (a == null) return;

            var d = Zuweisung(41, 744);   // schon vorher abweichend: nachfragen, mit Vorschlag
            var m = Aendere(a, d, true, false, 41, 49);
            Assert.Equal(WIDERSPRUCH, m.Art);
            Assert.Equal(749, m.PartnerNeu);
            Assert.Equal(744, Lies(d, UST));

            d = Zuweisung(41, 744);       // die Änderung behebt die Abweichung
            m = Aendere(a, d, true, false, 41, 44);
            Assert.Equal(NICHTS, m.Art);
            Assert.Equal(UstAbgleichBefund.Stimmig, a.Pruefe(d, true));

            d = Zuweisung(41, 744);       // abweichendes Paar, Zuweisung entfernt: Gegenstück bleibt unberührt
            m = Aendere(a, d, true, false, 41, 0);
            Assert.Equal(NICHTS, m.Art);
            Assert.Equal(744, Lies(d, UST));

            d = Zuweisung(0, 741);        // fehlende Voranmeldung nachgetragen
            m = Aendere(a, d, true, false, 0, 41);
            Assert.Equal(NICHTS, m.Art);
            Assert.Equal(UstAbgleichBefund.Stimmig, a.Pruefe(d, true));
        }

        [Fact]
        public void NachZuweisung_VermerkUndSonderfaelle()
        {
            var a = Lade2026();
            if (a == null) return;

            var d = Zuweisung(41, 744);   // Vermerk verfällt mit der Änderung
            UstAbgleich.SetzeBelassen(d, true);
            Assert.Equal(UstAbgleichBefund.Stimmig, a.Pruefe(d, true));
            var m = Aendere(a, d, true, false, 41, 49);
            Assert.Equal(WIDERSPRUCH, m.Art);
            Assert.False(UstAbgleich.IstBelassen(d, 49, 744));
            Assert.Equal(UstAbgleichBefund.Widerspruch, a.Pruefe(d, true));

            d = Zuweisung(41, 741);       // keine Änderung: nichts tun
            m = Aendere(a, d, true, false, 41, 41);
            Assert.Equal(NICHTS, m.Art);

            d = Zuweisung(0, 0);          // Ausgabenkonto "VST-Beträge separat"
            m = Aendere(a, d, false, false, 0, 66);
            Assert.Equal(PARTNER_GESETZT, m.Art);
            Assert.Equal(1320, Lies(d, UST));
        }

        // -----------------------------------------------------------------
        // Zählung und Hinweistext
        // -----------------------------------------------------------------
        [Fact]
        public void ZaehleBefunde_NachFormular()
        {
            var a = Lade2026();
            if (a == null) return;

            var konten = new List<EUKonto>
            {
                new EUKonto("Erlöse 19%", true, 0, "", Zuweisung(41, 0)),        // 41 ohne Erklärung: UstFehlt
                new EUKonto("Ausfuhr", true, 1, "", Zuweisung(43, 0)),           // UstMehrdeutig
                new EUKonto("Nur Erklärung", true, 2, "", Zuweisung(0, 741)),    // UvaFehlt
                new EUKonto("Abweichend", true, 3, "", Zuweisung(41, 744)),      // Widerspruch
                new EUKonto("Stimmig", true, 4, "", Zuweisung(41, 741)),
                new EUKonto("Wareneinkauf", false, 0, "", Zuweisung(66, 0)),     // UstFehlt
            };
            var doc = new BuchungsDocument { Jahr = 2026 };
            doc.Buchungen.Add(new Buchung { Datum = new DateTime(2026, 3, 1), Konto = "Ausfuhr", Art = Buchungsart.Einnahme });
            doc.Buchungen.Add(new Buchung { Datum = new DateTime(2025, 3, 1), Konto = "Abweichend", Art = Buchungsart.Einnahme });   // anderes Jahr
            doc.Buchungen.Add(new Buchung { Datum = new DateTime(2026, 3, 1), Konto = "Nur Erklärung", Art = Buchungsart.Einnahme });

            Assert.Equal(4, a.ZaehleBefunde(konten, doc, UstAbgleichFormular.Erklaerung, out int mit));
            Assert.Equal(1, mit);
            Assert.Equal(2, a.ZaehleBefunde(konten, doc, UstAbgleichFormular.Voranmeldung, out mit));
            Assert.Equal(1, mit);
            Assert.Equal(5, a.ZaehleBefunde(konten, null, UstAbgleichFormular.Beide, out mit));
            Assert.Equal(0, mit);
        }

        [Fact]
        public void HinweisText_WieV3()
        {
            Assert.Equal("Bei 3 E/Ü-Konten fehlt die Zuordnung zur Umsatzsteuererklärung oder passt nicht zu der für die "
                       + "Umsatzsteuer-Voranmeldung (2 davon mit Buchungen in 2025). Beträge dieser Konten können hier "
                       + "fehlen oder im falschen Feld stehen.",
                UstAbgleich.HinweisText(UstAbgleichFormular.Erklaerung, 3, 2, 2025));
            Assert.StartsWith("Bei einem E/Ü-Konto fehlt die Zuordnung zur Umsatzsteuer-Voranmeldung oder passt nicht zu der für die Umsatzsteuererklärung (es hat Buchungen in 2025).",
                UstAbgleich.HinweisText(UstAbgleichFormular.Voranmeldung, 1, 1, 2025));
            Assert.Contains("(ohne Buchungen in 2025)", UstAbgleich.HinweisText(UstAbgleichFormular.Erklaerung, 1, 0, 2025));
            Assert.Contains("(keines davon mit Buchungen in 2025)", UstAbgleich.HinweisText(UstAbgleichFormular.Erklaerung, 2, 0, 2025));
            Assert.Equal("", UstAbgleich.HinweisText(UstAbgleichFormular.Erklaerung, 0, 0, 2025));
        }

        // -----------------------------------------------------------------
        // Automatisch angelegte Konten (KontoFeldSelektor.ErzeugeKonto)
        // -----------------------------------------------------------------
        [Fact]
        public void KontoAnlage_FuehrtEindeutigesStillMit()
        {
            var a = Lade2026();
            if (a == null) return;

            var d = new Dictionary<string, string> { ["E/Ü-Rechnung"] = "1103", [UVA] = "41" };
            KontoFeldSelektor.UstAbgleichMitfuehren(d, true, a);
            Assert.Equal("741", d[UST]);

            d = new Dictionary<string, string> { ["E/Ü-Rechnung"] = "1103", [UVA] = "43" };   // Ausfuhr: mehrdeutig
            KontoFeldSelektor.UstAbgleichMitfuehren(d, true, a);
            Assert.False(d.ContainsKey(UST));

            d = new Dictionary<string, string> { [UVA] = "41", [UST] = "744" };   // beide genannt: unangetastet
            KontoFeldSelektor.UstAbgleichMitfuehren(d, true, a);
            Assert.Equal("744", d[UST]);

            d = new Dictionary<string, string> { ["E/Ü-Rechnung"] = "1185" };   // gar kein USt-Formular
            KontoFeldSelektor.UstAbgleichMitfuehren(d, false, a);
            Assert.Single(d);

            d = new Dictionary<string, string> { [UVA] = "41" };   // ohne Abgleich (keine Formulare): nichts
            KontoFeldSelektor.UstAbgleichMitfuehren(d, true, UstAbgleich.Inaktiv());
            Assert.Single(d);
        }
    }
}
