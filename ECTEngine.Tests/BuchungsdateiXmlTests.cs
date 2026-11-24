// BuchungsdateiXmlTests.cs -- XML-Speicherformat (ECTEngine/Speicherformat)
//
// Sammeldatei (BuchungsdateiXml) und Buchungsordner (BuchungsOrdner): beide
// müssen ein Dokument verlustfrei und bytegenau reproduzierbar ablegen; der
// Ordner zusätzlich nur Änderungen als neue Versionen schreiben und nie eine
// vorhandene Datei überschreiben.

using System;
using System.IO;
using System.Linq;
using System.Text;
using Xunit;

namespace ECTEngine.Tests
{
    public class BuchungsdateiXmlTests
    {
        internal static BuchungsDocument Beispiel()
        {
            var doc = new BuchungsDocument
            {
                Jahr = 2025, Waehrung = "EUR", UrspruenglicheWaehrung = "DEM",
                GlobaleAfaGenauigkeit = AfaGenauigkeit.Halbjahr, Buchungszaehler = 42,
                LaufendeBelegnrEinnahmen = 7, LaufendeBelegnrAusgaben = 9,
                LaufendeBelegnrBank = 3, LaufendeBelegnrKasse = 4,
                BackupNachfrageIntervallTage = 14, BackupNachfrageTermin = new DateTime(2025, 3, 1)
            };
            doc.Erweiterungen.Setze("Elster", "UST-Zahlbetrag-2025-41", "91,33");

            var b1 = new Buchung
            {
                Art = Buchungsart.Einnahme, Datum = new DateTime(2025, 1, 14),
                BruttoBetrag = new Betrag(2140.00m, 7m), Beschreibung = "Honorar <Kinderbuch> & \"Rate\"\nzweite Zeile",
                Konto = "Honorare", Belegnummer = "R-01", Betrieb = "Illustration", Bestandskonto = "Girokonto"
            };
            b1.Erweiterungen.Setze("EasyCash", "Buchungsgruppe", "abc");
            b1.Erweiterungen.Setze("Plugin", "Wert", "a|b=c");
            var b2 = new Buchung
            {
                Art = Buchungsart.Ausgabe, Datum = new DateTime(2025, 6, 14, 10, 30, 0),
                BruttoBetrag = new Betrag(-3570.00m, 10.7m), Beschreibung = "",
                Konto = "AfA", AfaJahre = 7, AfaNr = 2, AfaRestwertCent = 275000,
                AfaDegressiv = true, AfaSatz = 25, AfaGenauigkeit = AfaGenauigkeit.Ganzjahr
            };
            doc.Buchungen.Add(b1);
            doc.Buchungen.Add(b2);

            var d = new Dauerbuchung
            {
                Art = Buchungsart.Ausgabe, Beschreibung = "Miete $M/$J", BruttoBetrag = new Betrag(450m, 0m),
                Intervall = Intervall.Quartalsweise, VonDatum = new DateTime(2023, 1, 1),
                BisDatum = new DateTime(2037, 12, 31), Buchungstag = 3, AusgefuehrtBis = new DateTime(2025, 3, 3),
                Konto = "Raum", Belegnummer = "M-$M", Betrieb = "B", Bestandskonto = "Giro"
            };
            d.Erweiterungen.Setze("X", "Y", "Z");
            doc.Dauerbuchungen.Add(d);
            return doc;
        }

        [Fact]
        public void Sammeldatei_RoundtripIstVerlustfreiUndBytegenau()
        {
            var doc = Beispiel();
            string xml = BuchungsdateiXml.SchreibeSammeldatei(doc);
            var gelesen = BuchungsdateiXml.LeseSammeldatei(xml);

            Assert.Equal(xml, BuchungsdateiXml.SchreibeSammeldatei(gelesen));
            Assert.Equal(doc.Jahr, gelesen.Jahr);
            Assert.Equal("DEM", gelesen.UrspruenglicheWaehrung);
            Assert.Equal(AfaGenauigkeit.Halbjahr, gelesen.GlobaleAfaGenauigkeit);
            Assert.Equal(42, gelesen.Buchungszaehler);
            Assert.Equal(4, gelesen.LaufendeBelegnrKasse);
            Assert.Equal("91,33", gelesen.Erweiterungen.Hole("Elster", "UST-Zahlbetrag-2025-41"));

            var b1 = gelesen.Buchungen.Single(b => b.Uuid == doc.Buchungen[0].Uuid);
            Assert.Equal("Honorar <Kinderbuch> & \"Rate\"\nzweite Zeile", b1.Beschreibung);
            Assert.Equal(214000, b1.BruttoBetrag.InCent);
            Assert.Equal(7000, b1.BruttoBetrag.MwstPromille);
            Assert.Equal("a|b=c", b1.Erweiterungen.Hole("Plugin", "Wert"));

            var b2 = gelesen.Buchungen.Single(b => b.Uuid == doc.Buchungen[1].Uuid);
            Assert.Equal(new DateTime(2025, 6, 14, 10, 30, 0), b2.Datum);
            Assert.Equal(-357000, b2.BruttoBetrag.InCent);
            Assert.Equal(10700, b2.BruttoBetrag.MwstPromille);
            Assert.Equal((7, 2, 275000, true, 25, AfaGenauigkeit.Ganzjahr),
                (b2.AfaJahre, b2.AfaNr, b2.AfaRestwertCent, b2.AfaDegressiv, b2.AfaSatz, b2.AfaGenauigkeit));

            var d = gelesen.Dauerbuchungen.Single();
            Assert.Equal(Intervall.Quartalsweise, d.Intervall);
            Assert.Equal("M-$M", d.Belegnummer);
            Assert.Equal(new DateTime(2025, 3, 3), d.AusgefuehrtBis);
            Assert.Equal("Z", d.Erweiterungen.Hole("X", "Y"));
        }

        [Fact]
        public void Sammeldatei_IstUtf8OhneBomMitLf()
        {
            string xml = BuchungsdateiXml.SchreibeSammeldatei(Beispiel());
            Assert.DoesNotContain("\r", xml);
            Assert.StartsWith("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n", xml);
        }

        [Fact]
        public void Sammeldatei_LehntNeuereFormateUndUnsinnAb()
        {
            Assert.Throws<FormatException>(() => BuchungsdateiXml.LeseSammeldatei(
                "<ECTBuchungsdatei Format=\"99\"><Dokument Jahr=\"2025\"/></ECTBuchungsdatei>"));
            Assert.Throws<FormatException>(() => BuchungsdateiXml.LeseSammeldatei("<Quatsch/>"));
            Assert.Throws<FormatException>(() => BuchungsdateiXml.LeseSammeldatei("kein xml"));
            Assert.Throws<FormatException>(() => BuchungsdateiXml.LeseSammeldatei(
                "<ECTBuchungsdatei Format=\"1\"><Dokument Jahr=\"2025\"/><Buchungen>"
                + "<Buchung Id=\"x\" Art=\"Einnahme\" Datum=\"2025-01-01\" Brutto=\"1\" MwSt=\"0\"/>"
                + "</Buchungen></ECTBuchungsdatei>"));
        }

        [Fact]
        public void Sammeldatei_DemodatenSindKanonisch()
        {
            // die eingebettete Datei ist genau so formatiert, wie der Schreiber
            // sie schreiben würde -- sonst stimmten Hashes nach einem Roundtrip nicht
            var doc = DemoModus.ErzeugeDokument(DemoModus.JahrDerBeispieldaten());
            string neu = BuchungsdateiXml.SchreibeSammeldatei(doc);
            Assert.Equal(neu, BuchungsdateiXml.SchreibeSammeldatei(BuchungsdateiXml.LeseSammeldatei(neu)));
        }
    }

    public class BuchungsOrdnerTests : IDisposable
    {
        private readonly string _ordner =
            Path.Combine(Path.GetTempPath(), "ect-buchungsordner-" + Guid.NewGuid().ToString("N"));

        public void Dispose()
        {
            BuchungsOrdner.Uhr = () => DateTime.UtcNow;
            if (Directory.Exists(_ordner)) Directory.Delete(_ordner, true);
        }

        [Fact]
        public void Speichern_UndLaden_ErgibtDasselbeDokument()
        {
            var doc = BuchungsdateiXmlTests.Beispiel();
            var e = BuchungsOrdner.Speichere(doc, _ordner);
            Assert.Equal(2, e.NeueBuchungen);
            Assert.True(e.KopfGeschrieben);
            Assert.True(BuchungsOrdner.IstBuchungsOrdner(_ordner));

            var geladen = BuchungsOrdner.Lade(_ordner);
            Assert.Equal(BuchungsdateiXml.SchreibeSammeldatei(doc), BuchungsdateiXml.SchreibeSammeldatei(geladen));
            Assert.Empty(BuchungsOrdner.Pruefe(_ordner));
        }

        [Fact]
        public void ErneutesSpeichern_OhneAenderung_SchreibtNichts()
        {
            var doc = BuchungsdateiXmlTests.Beispiel();
            BuchungsOrdner.Speichere(doc, _ordner);
            var e = BuchungsOrdner.Speichere(BuchungsOrdner.Lade(_ordner), _ordner);
            Assert.True(e.NichtsGeaendert);
        }

        [Fact]
        public void Aenderung_UndLoeschung_WerdenNeueVersionen()
        {
            var doc = BuchungsdateiXmlTests.Beispiel();
            BuchungsOrdner.Speichere(doc, _ordner);
            var b1 = doc.Buchungen[0];
            string v1 = Path.Combine(_ordner, "buchungen", b1.Uuid.ToString("D") + "-000001.xml");
            byte[] v1Inhalt = File.ReadAllBytes(v1);

            b1.Beschreibung = "geändert";
            var e = BuchungsOrdner.Speichere(doc, _ordner);
            Assert.Equal(1, e.GeaenderteBuchungen);
            Assert.False(e.KopfGeschrieben);
            Assert.Equal(new[] { Path.Combine("buchungen", b1.Uuid.ToString("D") + "-000002.xml") }, e.Dateien);
            Assert.Equal(v1Inhalt, File.ReadAllBytes(v1));   // alte Version unangetastet
            Assert.Equal("geändert", BuchungsOrdner.Lade(_ordner).Buchungen.Single(b => b.Uuid == b1.Uuid).Beschreibung);

            doc.Buchungen.Remove(b1);
            e = BuchungsOrdner.Speichere(doc, _ordner);
            Assert.Equal(1, e.GeloeschteBuchungen);
            Assert.DoesNotContain(BuchungsOrdner.Lade(_ordner).Buchungen, b => b.Uuid == b1.Uuid);
            Assert.Empty(BuchungsOrdner.Pruefe(_ordner));

            // wiederhergestellt: Version 4
            doc.Buchungen.Add(b1);
            e = BuchungsOrdner.Speichere(doc, _ordner);
            Assert.Equal(1, e.NeueBuchungen);
            Assert.True(File.Exists(Path.Combine(_ordner, "buchungen", b1.Uuid.ToString("D") + "-000004.xml")));
        }

        [Fact]
        public void Pruefe_BemerktManipulationUndLuecken()
        {
            var doc = BuchungsdateiXmlTests.Beispiel();
            BuchungsOrdner.Speichere(doc, _ordner);
            doc.Buchungen[0].Beschreibung = "v2";
            BuchungsOrdner.Speichere(doc, _ordner);
            doc.Buchungen[0].Beschreibung = "v3";
            BuchungsOrdner.Speichere(doc, _ordner);

            string basis = Path.Combine(_ordner, "buchungen", doc.Buchungen[0].Uuid.ToString("D"));
            // Version 1 nachträglich verändert --> Vorgänger-Hash von Version 2 stimmt nicht
            File.AppendAllText(basis + "-000001.xml", "<!-- nachträglich -->");
            Assert.Contains(BuchungsOrdner.Pruefe(_ordner), p => p.Contains("-000002.xml") && p.Contains("Vorgänger"));

            // Version 2 entfernt --> Lücke
            File.Delete(basis + "-000002.xml");
            Assert.Contains(BuchungsOrdner.Pruefe(_ordner), p => p.Contains("Version 2 fehlt"));
        }

        [Fact]
        public void Zeitpunkt_KommtAusDerUhr()
        {
            BuchungsOrdner.Uhr = () => new DateTime(2026, 11, 24, 9, 15, 0, DateTimeKind.Utc);
            var doc = BuchungsdateiXmlTests.Beispiel();
            BuchungsOrdner.Speichere(doc, _ordner);
            string kopf = File.ReadAllText(Path.Combine(_ordner, "kopf", "kopf-000001.xml"), Encoding.UTF8);
            Assert.Contains("Zeitpunkt=\"2026-11-24T09:15:00Z\"", kopf);
        }

        [Fact]
        public void DoppelteIds_WerdenAbgelehnt()
        {
            var doc = BuchungsdateiXmlTests.Beispiel();
            doc.Buchungen.Add(doc.Buchungen[0].Clone());
            Assert.Throws<InvalidOperationException>(() => BuchungsOrdner.Speichere(doc, _ordner));
        }
    }
}
