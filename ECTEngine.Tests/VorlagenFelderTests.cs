// VorlagenFelderTests.cs -- Tests für die Erweiterung des Buchen-Dialogs
// durch Buchungsvorlagen: Zusatz-Felder (Werte, Variablen), Original-Feld-
// Regeln, Persistenz (ini-Cache, XML), der Rechner mit Feldwerten und der
// Vorlagen-Bezug der Buchung (Bearbeiten-Roundtrip).

using System;
using System.Collections.Generic;
using System.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    // ======================================================================
    // Feldwerte: Eingabe prüfen und in Variablen-Form bringen
    // ======================================================================

    public class FeldWertTests
    {
        private static FeldWert W(string typ, string roh, bool pflicht = false,
            string optionen = "")
            => VorlagenFelder.Werte(
                new PresetFeld("feld", "Mein Feld", typ, pflicht: pflicht, optionen: optionen),
                roh);

        [Theory]
        [InlineData("12,345", 12.35)]      // gerundet auf Cent (kaufmännisch)
        [InlineData("1.234,56", 1234.56)]  // deutsch
        [InlineData("1'234.56", 1234.56)]  // schweizerisch
        [InlineData("7", 7)]
        public void Betrag_WirdTolerantGelesen(string roh, double erwartet)
        {
            var w = W("betrag", roh);
            Assert.Equal("", w.Fehler);
            Assert.True(w.HatZahl);
            Assert.Equal((decimal)erwartet, w.Zahl);
        }

        [Fact]
        public void Betrag_Ungueltig_LiefertFehlerUndNull()
        {
            var w = W("betrag", "abc");
            Assert.NotEqual("", w.Fehler);
            Assert.True(w.HatZahl);      // die Live-Vorschau rechnet mit 0 weiter
            Assert.Equal(0m, w.Zahl);
        }

        [Fact]
        public void LeeresFeld_NurMitPflichtEinFehler()
        {
            Assert.Equal("", W("zahl", "").Fehler);
            Assert.Equal(0m, W("zahl", "").Zahl);
            Assert.NotEqual("", W("zahl", "", pflicht: true).Fehler);
            Assert.NotEqual("", W("text", "  ", pflicht: true).Fehler);
            Assert.NotEqual("", W("bestandskonto", "", pflicht: true).Fehler);
        }

        [Fact]
        public void Zahl_BehaeltNachkommastellen()
        {
            Assert.Equal(0.3m, W("zahl", "0,30").Zahl);
            Assert.Equal(12.345m, W("zahl", "12.345").Zahl);
        }

        [Theory]
        [InlineData("prozent")]
        [InlineData("mwst")]
        public void Prozent_NurNullBisHundert(string typ)
        {
            Assert.Equal(30m, W(typ, "30").Zahl);
            Assert.Equal(7.5m, W(typ, "7,5 %").Zahl);
            Assert.NotEqual("", W(typ, "101").Fehler);
            Assert.NotEqual("", W(typ, "-1").Fehler);
        }

        [Theory]
        [InlineData("1", 1)]
        [InlineData("ja", 1)]
        [InlineData("0", 0)]
        [InlineData("", 0)]
        public void JaNein_LiefertEinsOderNull(string roh, int erwartet)
        {
            var w = W("janein", roh, pflicht: true);   // Pflicht ist hier egal
            Assert.Equal("", w.Fehler);
            Assert.Equal(erwartet, w.Zahl);
            Assert.Equal(erwartet == 1 ? "1" : "0", w.Roh);
        }

        [Fact]
        public void Auswahl_WertMussVorgesehenSein()
        {
            const string opt = "1. Quartal=1Q;2. Quartal=2Q";
            var ok = W("auswahl", "2Q", optionen: opt);
            Assert.Equal("", ok.Fehler);
            Assert.Equal("2Q", ok.Text);
            Assert.False(ok.HatZahl);

            Assert.NotEqual("", W("auswahl", "5Q", optionen: opt).Fehler);
        }

        [Fact]
        public void Auswahl_MitZahlenwerten_LiefertZahlVariable()
        {
            var w = W("auswahl", "7", optionen: "Regelsatz=19;ermäßigt=7");
            Assert.True(w.HatZahl);
            Assert.Equal(7m, w.Zahl);
        }

        [Fact]
        public void Auswahl_OhneGleichheitszeichen_AnzeigeIstWert()
        {
            var feld = new PresetFeld("art", "Art", "auswahl", optionen: "bar; Karte");
            Assert.Equal(new[] { "bar", "Karte" },
                feld.AuswahlOptionen.Select(o => o.Wert).ToArray());
        }

        [Theory]
        [InlineData("3.7.2026", "03.07.2026")]
        [InlineData("03.07.26", "03.07.2026")]
        [InlineData("2026-07-03", "03.07.2026")]
        public void Datum_WirdNormalisiert(string roh, string erwartet)
        {
            var w = W("datum", roh);
            Assert.Equal("", w.Fehler);
            Assert.Equal(erwartet, w.Text);
        }

        [Fact]
        public void Datum_Ungueltig()
        {
            Assert.NotEqual("", W("datum", "31.02.2026").Fehler);
            Assert.NotEqual("", W("datum", "morgen").Fehler);
        }

        [Fact]
        public void UnbekannterTyp_WirdZuText()
        {
            var feld = new PresetFeld("neu", "Neu", "hologramm");
            Assert.False(feld.TypBekannt);
            Assert.Equal(ZusatzFeldTyp.Text, feld.Typ);
            Assert.Equal("beschreibung", feld.Box);   // Standard-Box des Typs
        }

        [Fact]
        public void Box_UnbekanntFaelltAufStandardDesTyps()
        {
            Assert.Equal("betrag", new PresetFeld("ab", "", "prozent", "irgendwo").Box);
            Assert.Equal("zuordnung", new PresetFeld("ab", "", "konto").Box);
            Assert.Equal("afa", new PresetFeld("ab", "", "zahl", "AfA").Box);
        }

        [Theory]
        [InlineData("anteil2", true)]
        [InlineData("$anteil2", true)]     // führendes '$' wird toleriert
        [InlineData("x", false)]           // zu kurz (Datums-Platzhalter!)
        [InlineData("2anteil", false)]     // beginnt mit Ziffer
        [InlineData("an teil", false)]
        [InlineData("brutto", false)]      // eingebaut
        [InlineData("Rest", false)]        // eingebaut, Groß/Klein egal
        [InlineData("de", false)]          // Länderkürzel der Konto-Spezifikation
        [InlineData("größe", false)]       // nur a-z (ini-Key!)
        public void PruefeName(string name, bool gueltig)
        {
            Assert.Equal(gueltig, VorlagenFelder.PruefeName(name) == "");
        }

        [Fact]
        public void StandardWert_FormelAusKonstanten()
        {
            Assert.Equal(50m, VorlagenFelder.Werte(
                new PresetFeld("ab", "", "prozent", vorbelegung: "100/2"),
                VorlagenFelder.StandardWert(
                    new PresetFeld("ab", "", "prozent", vorbelegung: "100/2"))).Zahl);
            Assert.Equal("1", VorlagenFelder.StandardWert(
                new PresetFeld("ab", "", "janein", vorbelegung: "ja")));
            Assert.Equal("0", VorlagenFelder.StandardWert(
                new PresetFeld("ab", "", "janein")));
            // Auswahl: Vorbelegung darf die Anzeige nennen
            Assert.Equal("2Q", VorlagenFelder.StandardWert(new PresetFeld(
                "ab", "", "auswahl", vorbelegung: "2. Quartal",
                optionen: "1. Quartal=1Q;2. Quartal=2Q")));
        }

        [Fact]
        public void Kodierung_VertraegtPipeUndProzent()
        {
            const string text = "100 % A|B\nC";
            string kodiert = VorlagenFelder.Kodiere(text);
            Assert.DoesNotContain("|", kodiert);
            Assert.DoesNotContain("\n", kodiert);
            Assert.Equal(text, VorlagenFelder.Dekodiere(kodiert));
        }
    }

    // ======================================================================
    // Rechner mit Zusatz-Feldern
    // ======================================================================

    public class VorlagenRechnerTests
    {
        private static Buchung Basis(int bruttoCent, int mwstPromille,
            string beschreibung = "Druckerpapier") => new Buchung
        {
            Art = Buchungsart.Ausgabe,
            Datum = new DateTime(2026, 11, 12),
            Beschreibung = beschreibung,
            Belegnummer = "A42",
            Konto = "Bürobedarf",
            Betrieb = "Verlag",
            Bestandskonto = "PayPal",
            BruttoBetrag = Betrag.AusCent(bruttoCent, mwstPromille)
        };

        private static FeldWert[] Werte(Preset p, params string[] roh)
            => p.Felder.Select((f, i) => VorlagenFelder.Werte(f, roh[i])).ToArray();

        [Fact]
        public void ZweiBetriebe_AnteilUndBetriebAusFeldern()
        {
            // Beispiel 2 aus der Anforderung: zweite Betriebs-Liste +
            // "Anteil Betrieb 2" in der Betrags-Box
            var vorlage = new Preset("Aufteilung zwei Betriebe", true, 19000, 1, "",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "$konto", "", "$brutto*$anteil2/100",
                        "$beschreibung (Anteil $betrieb2)", "", "$betrieb2")
                },
                basisBetragFormel: "$rest",
                erweiterung: new VorlagenErweiterung(new[]
                {
                    new PresetFeld("betrieb2", "Betrieb 2", "betrieb", pflicht: true),
                    new PresetFeld("anteil2", "Anteil Betrieb 2", "prozent", "betrag", "50")
                }));

            var basis = Basis(20000, 19000);
            var felder = Werte(vorlage, "Agentur", "30");
            var b = BuchungsgruppenRechner.Berechne(vorlage, basis, felder);

            Assert.False(b.HatFehler);
            var z = Assert.Single(b.Zeilen);
            Assert.Equal(60m, z.Brutto);
            Assert.Equal("Agentur", z.Betrieb);
            Assert.Equal("Bürobedarf", z.Konto);                 // $konto der Basis
            Assert.Equal(19000, z.MwstPromille);                 // leer = Satz der Basis
            Assert.Equal("Druckerpapier (Anteil Agentur)", z.Beschreibung);
            Assert.Equal(140m, b.BasisGebuchtBrutto);

            var buchungen = BuchungsgruppenRechner.BaueBuchungen(
                vorlage, 12, basis, b, null, felder);
            Assert.Equal(2, buchungen.Count);
            Assert.Equal(14000, buchungen[0].BruttoBetrag.InCent);
            Assert.Equal("Verlag", buchungen[0].Betrieb);
            Assert.Equal(6000, buchungen[1].BruttoBetrag.InCent);
            Assert.Equal("Agentur", buchungen[1].Betrieb);
        }

        [Fact]
        public void Umbuchung_ZielBestandskontoAusFeld()
        {
            // Beispiel 1: Abbuchung PayPal -> Bank, Gegenbuchung landet
            // auf dem im Zusatz-Feld gewählten Bestandskonto
            var vorlage = new Preset("PayPal-Auszahlung", true, 0, 1, "/Geldtransit",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("E", "/Geldtransit", "0", "$brutto",
                        "$beschreibung", "", "", "$zielkonto")
                },
                erweiterung: new VorlagenErweiterung(new[]
                {
                    new PresetFeld("zielkonto", "Gutschrift auf", "bestandskonto", pflicht: true)
                }));

            var b = BuchungsgruppenRechner.Berechne(vorlage, Basis(50000, 0),
                Werte(vorlage, "Girokonto"));
            Assert.False(b.HatFehler);
            Assert.Equal("Girokonto", b.Zeilen[0].Bestandskonto);
            Assert.Equal(Buchungsart.Einnahme, b.Zeilen[0].Art);
            Assert.Equal(500m, b.Zeilen[0].Brutto);
        }

        [Fact]
        public void NullZeile_WirdUebersprungen_RollenBleibenStabil()
        {
            // Beispiel 3: Warenwert / Skonto / Versand -- leer gelassenes
            // Skonto bucht keine Zeile, die Versand-Zeile behält Rolle 2
            var vorlage = new Preset("Wareneinkauf", true, 19000, 1, "Wareneinkauf",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("E", "Erhaltene Skonti", "", "$skonto", "$beschreibung Skonto"),
                    new PresetZeile("", "Bezugsnebenkosten", "", "$versand", "$beschreibung Versand")
                },
                basisBetragFormel: "$brutto-$versand",
                erweiterung: new VorlagenErweiterung(new[]
                {
                    new PresetFeld("skonto", "Skonto", "betrag"),
                    new PresetFeld("versand", "Versandkosten", "betrag")
                }));

            var basis = Basis(11900, 19000);
            var felder = Werte(vorlage, "", "5,95");
            var b = BuchungsgruppenRechner.Berechne(vorlage, basis, felder);

            Assert.False(b.HatFehler);
            Assert.True(b.Zeilen[0].Uebersprungen);
            Assert.False(b.Zeilen[1].Uebersprungen);
            Assert.Single(b.GebuchteZeilen);

            var buchungen = BuchungsgruppenRechner.BaueBuchungen(
                vorlage, 4, basis, b, null, felder);
            Assert.Equal(2, buchungen.Count);
            Assert.Equal(2, buchungen[1].GruppenRolle);        // Vorlagen-Zeile 2
            Assert.Equal("Druckerpapier Versand", buchungen[1].Beschreibung);
            Assert.Equal(11305, buchungen[0].BruttoBetrag.InCent);
        }

        [Fact]
        public void AlleZeilenNull_ErgibtEinzelbuchungOhneGruppe()
        {
            var vorlage = new Preset("Mit optionalem Skonto", true, 19000, 1, "Wareneinkauf",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("E", "Erhaltene Skonti", "", "$skonto", "")
                },
                erweiterung: new VorlagenErweiterung(new[]
                {
                    new PresetFeld("skonto", "Skonto", "betrag")
                }));

            var basis = Basis(11900, 19000);
            // Die Basis war schon einmal Gruppen-Mitglied (Bearbeiten)
            basis.SetzeGruppe("alte-uuid", 0, 9);

            var felder = Werte(vorlage, "");
            var b = BuchungsgruppenRechner.Berechne(vorlage, basis, felder);
            var buchungen = BuchungsgruppenRechner.BaueBuchungen(
                vorlage, 9, basis, b, "alte-uuid", felder);

            var einzige = Assert.Single(buchungen);
            Assert.Null(einzige.GruppenUuid);
            Assert.Equal(-1, einzige.GruppenRolle);
            // ... der Vorlagen-Bezug bleibt aber, damit beim nächsten
            // Bearbeiten das Skonto-Feld wieder da ist
            Assert.Equal(9, einzige.GruppenVorlage);
            Assert.Equal("Mit optionalem Skonto", einzige.VorlagenName);
        }

        [Fact]
        public void ZeileOhneBetragsformel_WirdTolerantUebersprungen()
        {
            // Vorlage aus der Zeit der manuellen Zeilen-Eingabe
            var vorlage = new Preset("Alt", true, 19000, 1, "K",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "K2", "", "", "$B (7%)", darstellung: "maske")
                });

            var b = BuchungsgruppenRechner.Berechne(vorlage, Basis(11900, 19000));
            Assert.False(b.HatFehler);
            Assert.True(b.Zeilen[0].Uebersprungen);
            Assert.NotEqual("", b.Zeilen[0].Hinweis);
            Assert.Single(BuchungsgruppenRechner.BaueBuchungen(
                vorlage, 1, Basis(11900, 19000), b));
        }

        [Fact]
        public void UebersprungeneZeile_BlockiertNichtMitKontoFehler()
        {
            var vorlage = new Preset("X", true, 0, 1, "K",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "$konto2", "", "$extra", "")
                },
                erweiterung: new VorlagenErweiterung(new[]
                {
                    new PresetFeld("extra", "Extra", "betrag"),
                    new PresetFeld("konto2", "Konto", "konto")
                }));

            // Betrag 0: Zeile entfällt, leeres Konto stört nicht
            var b = BuchungsgruppenRechner.Berechne(vorlage, Basis(1000, 0),
                Werte(vorlage, "", ""));
            Assert.False(b.HatFehler);

            // Betrag vorhanden, Konto leer: jetzt ist es ein Fehler
            b = BuchungsgruppenRechner.Berechne(vorlage, Basis(1000, 0),
                Werte(vorlage, "5", ""));
            Assert.True(b.HatFehler);
            Assert.Contains("Konto", b.Zeilen[0].Fehler);
        }

        [Theory]
        [InlineData("130", true)]     // mehr als die Eingabe (119)
        [InlineData("119", true)]     // genau die Eingabe: Basis wäre 0
        [InlineData("30", false)]
        public void Aufteilung_DarfDieBasisNichtAufzehren(string anteil, bool fehler)
        {
            var feld = new PresetFeld("betrag7", "davon 7 %", "betrag");
            var vorlage = new Preset("Gemischt", true, 19000, 1, "K",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "K", "7", "$betrag7", "")
                },
                basisBetragFormel: "$rest",
                erweiterung: new VorlagenErweiterung(new[] { feld }));

            var b = BuchungsgruppenRechner.Berechne(vorlage, Basis(11900, 19000),
                new[] { VorlagenFelder.Werte(feld, anteil) });
            Assert.Equal(fehler, b.BasisFehler.Length > 0);
        }

        [Fact]
        public void EingabeBetrag_AusFeldernBerechnet()
        {
            // Reisekosten: der Betrag wird nicht eingegeben, sondern aus
            // Mengen berechnet (Betragsfeld gesperrt)
            var vorlage = new Preset("Reisekosten", true, 0, 1, "Reisekosten",
                erweiterung: new VorlagenErweiterung(
                    new[]
                    {
                        new PresetFeld("tage8", "Tage über 8 Std.", "zahl"),
                        new PresetFeld("tage24", "volle Tage", "zahl"),
                        new PresetFeld("km", "gefahrene km", "zahl")
                    },
                    new[] { new OriginalFeldRegel(OriginalFeld.Betrag, FeldZustand.Gesperrt) },
                    eingabeBetragFormel: "$tage8*14 + $tage24*28 + $km*0,30"));

            var e = BuchungsgruppenRechner.BerechneEingabeBetrag(
                vorlage, Werte(vorlage, "2", "1", "150"));
            Assert.True(e.Ok);
            Assert.Equal(101m, e.Wert);     // 28 + 28 + 45

            // ohne Formel: null (der Dialog nimmt dann die Eingabe)
            Assert.Null(BuchungsgruppenRechner.BerechneEingabeBetrag(
                new Preset("X", true, 0, 1, "K"), null));
        }

        [Fact]
        public void EingabeBetrag_KenntKeinBrutto()
        {
            var vorlage = new Preset("X", true, 0, 1, "K",
                erweiterung: new VorlagenErweiterung(eingabeBetragFormel: "$brutto*2"));
            Assert.False(BuchungsgruppenRechner.BerechneEingabeBetrag(vorlage, null).Ok);
        }

        [Fact]
        public void BasisTemplates_BeschreibungUndBelegAusFeldern()
        {
            var vorlage = new Preset("Bewirtung", true, 19000, 1, "Bewirtungskosten",
                erweiterung: new VorlagenErweiterung(
                    new[]
                    {
                        new PresetFeld("anlass", "Anlass", "text", pflicht: true),
                        new PresetFeld("personen", "Bewirtete Personen", "text")
                    },
                    basisTextTemplate: "Bewirtung $anlass ($personen) $m/$J",
                    basisBelegTemplate: "$beleg-B"));

            var basis = Basis(8000, 19000, "Bewirtung");
            var felder = Werte(vorlage, "Projektabschluss", "Müller, Meier");
            var b = BuchungsgruppenRechner.Berechne(vorlage, basis, felder);

            Assert.Equal("Bewirtung Projektabschluss (Müller, Meier) 11/2026",
                b.BasisBeschreibung);
            Assert.Equal("A42-B", b.BasisBelegnummer);

            var buchungen = BuchungsgruppenRechner.BaueBuchungen(
                vorlage, 30, basis, b, null, felder);
            var einzige = Assert.Single(buchungen);
            Assert.Equal(b.BasisBeschreibung, einzige.Beschreibung);
            Assert.Equal("Bewirtung", einzige.VorlagenEingabeText);   // getippter Text
            Assert.Equal("A42", einzige.VorlagenEingabeBeleg);        // eingegebener Beleg
            Assert.Equal("A42-B", einzige.Belegnummer);
            Assert.Null(einzige.GruppenUuid);
        }

        [Fact]
        public void OhneTemplates_BleibenBeschreibungUndBelegDerEingabe()
        {
            var vorlage = new Preset("Plain", true, 0, 1, "K",
                zeilen: new List<PresetZeile> { new PresetZeile("", "K2", "", "$brutto", "") });
            var b = BuchungsgruppenRechner.Berechne(vorlage, Basis(1000, 0));
            Assert.Equal("Druckerpapier", b.BasisBeschreibung);
            Assert.Equal("A42", b.BasisBelegnummer);
        }

        [Fact]
        public void FeldInMwstAusdruck()
        {
            var vorlage = new Preset("Zwei Sätze", true, 19000, 1, "K",
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "K", "$satz2", "$betrag2", "")
                },
                basisBetragFormel: "$rest",
                erweiterung: new VorlagenErweiterung(new[]
                {
                    new PresetFeld("betrag2", "davon", "betrag"),
                    new PresetFeld("satz2", "MWSt", "mwst", vorbelegung: "7")
                }));
            var b = BuchungsgruppenRechner.Berechne(vorlage, Basis(11900, 19000),
                Werte(vorlage, "10,70", "7"));
            Assert.False(b.HatFehler);
            Assert.Equal(7000, b.Zeilen[0].MwstPromille);
        }
    }

    // ======================================================================
    // Vorlagen-Bezug der Buchung (Bearbeiten-Roundtrip)
    // ======================================================================

    public class VorlagenBezugTests
    {
        [Fact]
        public void Bezug_UeberlebtDasPipeFormatDerErweiterung()
        {
            var feldText = new PresetFeld("anlass", "Anlass", "text");
            var feldZahl = new PresetFeld("anteil2", "Anteil", "prozent");
            var b = new Buchung();
            b.Erweiterungen.Setze("Fremdes Plugin", "Schluessel", "bleibt");
            b.SetzeVorlagenBezug(17, "Vorlage | mit Pipe", 119m, "Text 100 % | roh", "B|1",
                new[]
                {
                    VorlagenFelder.Werte(feldText, "A|B"),
                    VorlagenFelder.Werte(feldZahl, "30")
                });

            // so wie die Datei es speichert und wieder liest
            var gelesen = new Buchung
            {
                Erweiterungen = ErweiterungStore.AusPipeFormat(b.Erweiterungen.ZuPipeFormat())
            };

            Assert.Equal(17, gelesen.GruppenVorlage);
            Assert.Equal("Vorlage | mit Pipe", gelesen.VorlagenName);
            Assert.Equal(119m, gelesen.VorlagenEingabeBetrag);
            Assert.Equal("Text 100 % | roh", gelesen.VorlagenEingabeText);
            Assert.Equal("B|1", gelesen.VorlagenEingabeBeleg);
            var werte = gelesen.VorlagenFeldWerte();
            Assert.Equal("A|B", werte["anlass"]);
            Assert.Equal("30", werte["anteil2"]);
            Assert.Equal("bleibt", gelesen.Erweiterungen.Hole("Fremdes Plugin", "Schluessel"));
        }

        [Fact]
        public void Bezug_NichtGemerkteAngabenSindNull()
        {
            var b = new Buchung();
            b.SetzeVorlagenBezug(3, "V", null, null, null, null);
            Assert.Null(b.VorlagenEingabeBetrag);
            Assert.Null(b.VorlagenEingabeText);
            Assert.Null(b.VorlagenEingabeBeleg);
            Assert.Empty(b.VorlagenFeldWerte());
        }

        [Fact]
        public void SetzeVorlagenBezug_ErsetztAlteFeldwerte()
        {
            var alt = new PresetFeld("alt", "", "text");
            var neu = new PresetFeld("neu", "", "text");
            var b = new Buchung();
            b.SetzeVorlagenBezug(1, "A", null, null, null, new[] { VorlagenFelder.Werte(alt, "x") });
            b.SetzeVorlagenBezug(2, "B", null, null, null, new[] { VorlagenFelder.Werte(neu, "y") });

            var werte = b.VorlagenFeldWerte();
            Assert.False(werte.ContainsKey("alt"));
            Assert.Equal("y", werte["neu"]);
            Assert.Equal("B", b.VorlagenName);
        }

        [Fact]
        public void EntferneGruppe_NimmtDenVorlagenBezugMit()
        {
            // Kopieren eines Gruppen-Mitglieds: die Kopie ist eine
            // gewöhnliche Buchung
            var feld = new PresetFeld("anteil2", "", "prozent");
            var b = new Buchung();
            b.SetzeGruppe("uuid", 0, 5);
            b.SetzeVorlagenBezug(5, "V", 10m, "t", "b", new[] { VorlagenFelder.Werte(feld, "30") });

            b.EntferneGruppe();

            Assert.Null(b.GruppenUuid);
            Assert.Equal(-1, b.GruppenVorlage);
            Assert.Equal("", b.VorlagenName);
            Assert.Empty(b.VorlagenFeldWerte());
            Assert.Equal(0, b.Erweiterungen.Count);
        }
    }

    // ======================================================================
    // Persistenz: ini-Cache
    // ======================================================================

    [Collection("EinstellungenCache")]   // statischer Cache, siehe PresetZeilenTests
    public class VorlagenErweiterungPersistenzTests
    {
        private static Preset BeispielVorlage() => new Preset(
            "Aufteilung", true, 19000, 1, "Bürobedarf",
            zeilen: new List<PresetZeile>
            {
                new PresetZeile("", "$konto", "", "$brutto*$anteil2/100", "", "", "$betrieb2")
            },
            basisBetragFormel: "$rest",
            erweiterung: new VorlagenErweiterung(
                new[]
                {
                    new PresetFeld("betrieb2", "Betrieb 2", "betrieb", pflicht: true),
                    new PresetFeld("anteil2", "Anteil Betrieb 2", "prozent", "betrag",
                        "50", merken: true),
                    new PresetFeld("zeitraum", "Zeitraum", "auswahl",
                        optionen: "1. Quartal=1Q;2. Quartal=2Q")
                },
                new[]
                {
                    new OriginalFeldRegel(OriginalFeld.Afa, FeldZustand.Ausgeblendet),
                    new OriginalFeldRegel(OriginalFeld.Betrag, FeldZustand.Normal, "Rechnungsbetrag"),
                    new OriginalFeldRegel(OriginalFeld.Mwst, FeldZustand.Gesperrt)
                },
                basisTextTemplate: "$B ($zeitraum)",
                basisBestandskonto: "Girokonto"));

        [Fact]
        public void SpeicherePreset_MitErweiterung_Roundtrip()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());

            var p = Einstellungen.Presets[8];
            Assert.True(p.IstErweitert);
            Assert.Equal(3, p.Felder.Count);
            Assert.Equal("betrieb2", p.Felder[0].Name);
            Assert.Equal(ZusatzFeldTyp.Betrieb, p.Felder[0].Typ);
            Assert.True(p.Felder[0].Pflicht);
            Assert.Equal("Anteil Betrieb 2", p.Felder[1].Bezeichnung);
            Assert.Equal("betrag", p.Felder[1].Box);
            Assert.Equal("50", p.Felder[1].Vorbelegung);
            Assert.True(p.Felder[1].Merken);
            Assert.Equal(2, p.Felder[2].AuswahlOptionen.Count);

            var e = p.Erweiterung;
            Assert.Equal(FeldZustand.Ausgeblendet, e.ZustandVon(OriginalFeld.Afa));
            Assert.Equal(FeldZustand.Gesperrt, e.ZustandVon(OriginalFeld.Mwst));
            Assert.Equal(FeldZustand.Normal, e.ZustandVon(OriginalFeld.Betrag));
            Assert.Equal("Rechnungsbetrag", e.RegelFuer(OriginalFeld.Betrag).Bezeichnung);
            Assert.Equal(FeldZustand.Normal, e.ZustandVon(OriginalFeld.Konto));
            Assert.Equal("$B ($zeitraum)", e.BasisTextTemplate);
            Assert.Equal("Girokonto", e.BasisBestandskonto);

            // Nachbar-Plätze unberührt
            Assert.False(Einstellungen.Presets[7].IstErweitert);
            Assert.Empty(Einstellungen.Presets[9].Felder);
        }

        [Fact]
        public void SpeicherePreset_EntfalleneFelderUndRegelnWerdenGeleert()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());

            // auf ein Feld und keine Regel reduzieren
            Einstellungen.SpeicherePreset(8, new Preset("Aufteilung", true, 19000, 1, "K",
                erweiterung: new VorlagenErweiterung(new[]
                {
                    new PresetFeld("betrieb2", "Betrieb 2", "betrieb")
                })));

            var p = Einstellungen.Presets[8];
            Assert.Single(p.Felder);
            Assert.Empty(p.Erweiterung.OriginalFelder);
            Assert.Equal("", Einstellungen.Hole("[Buchungsposten]08F2Name"));
            Assert.Equal("", Einstellungen.Hole("[Buchungsposten]08F2Typ"));
            Assert.Equal("", Einstellungen.Hole("[Buchungsposten]08OAfA"));
            Assert.Equal("", Einstellungen.Hole("[Buchungsposten]08BasisText"));
        }

        // ------------------------------------------------------------------
        // Feste Kennung der Vorlage (Preset.Id / VorlageId in der Buchung)
        // ------------------------------------------------------------------

        private static Buchung GebuchtMit(int slot)
        {
            var p = Einstellungen.Presets[slot];
            var basis = new Buchung
            {
                Art = Buchungsart.Ausgabe,
                Datum = new DateTime(2026, 11, 12),
                Beschreibung = "Druckerpapier",
                Konto = "Bürobedarf",
                BruttoBetrag = Betrag.AusCent(10000, 19000)
            };
            var felder = p.Felder.Select(f => VorlagenFelder.Werte(f,
                f.Name == "anteil2" ? "30" : "")).ToArray();
            var b = BuchungsgruppenRechner.Berechne(p, basis, felder);
            return BuchungsgruppenRechner.BaueBuchungen(p, slot, basis, b, null, felder)[0];
        }

        [Fact]
        public void Kennung_WirdVergebenUndBleibtBeimErneutenSpeichern()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            string id = Einstellungen.Presets[8].Id;
            Assert.False(string.IsNullOrEmpty(id));

            // Der Editor speichert ein frisch gebautes Preset ohne Kennung
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            Assert.Equal(id, Einstellungen.Presets[8].Id);
        }

        [Fact]
        public void Kennung_BuchungFindetUmbenannteVorlage()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            var gebucht = GebuchtMit(8);
            Assert.Equal(Einstellungen.Presets[8].Id, gebucht.VorlagenId);

            var alt = Einstellungen.Presets[8];
            Einstellungen.SpeicherePreset(8, new Preset("Ganz anderer Name", alt.Ausgabe,
                alt.Mwst, alt.AfaJ, alt.Konto, zeilen: alt.Zeilen,
                basisBetragFormel: alt.BasisBetragFormel, erweiterung: alt.Erweiterung,
                id: alt.Id));

            Assert.Equal(8, Einstellungen.FindeVorlagenSlot(gebucht));
        }

        [Fact]
        public void Kennung_BuchungFindetUmnummerierteVorlage_UndNichtDieNeueAmAltenPlatz()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            var gebucht = GebuchtMit(8);

            // Verschieben wie im Editor: alter Platz leer, neuer mit Kennung
            var p = Einstellungen.Presets[8];
            Einstellungen.SpeicherePreset(8, new Preset("", false, 0, 1, "", ""));
            Einstellungen.SpeicherePreset(15, p);
            // eine neue Vorlage gleichen Namens auf dem alten Platz
            Einstellungen.SpeicherePreset(8, BeispielVorlage());

            Assert.NotEqual(Einstellungen.Presets[8].Id, Einstellungen.Presets[15].Id);
            Assert.Equal(15, Einstellungen.FindeVorlagenSlot(gebucht));
        }

        [Fact]
        public void Kennung_GeloeschteVorlage_WirdNichtGefunden()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            var gebucht = GebuchtMit(8);
            Einstellungen.SpeicherePreset(8, new Preset("", false, 0, 1, "", ""));
            Assert.Equal("", Einstellungen.Hole("[Buchungsposten]08Id"));
            Assert.Equal(-1, Einstellungen.FindeVorlagenSlot(gebucht));
        }

        [Fact]
        public void Kennung_AlteBuchungOhneKennung_BleibtBeimPlatz()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            var b = new Buchung();
            b.SetzeVorlagenBezug(8, "Alter Name", null, null, null, null);
            Assert.Equal(8, Einstellungen.FindeVorlagenSlot(b));
        }

        [Fact]
        public void Kennung_ErweiterteVorlageOhneKennung_BekommtSieBeimLaden()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>
            {
                ["[Buchungsposten]05Text"] = "Alt",
                ["[Buchungsposten]05Ausg"] = "1",
                ["[Buchungsposten]05Rech"] = "K",
                ["[Buchungsposten]05F1Name"] = "anteil2",
                ["[Buchungsposten]05F1Typ"] = "prozent",
                ["[Buchungsposten]06Text"] = "Klassisch",
                ["[Buchungsposten]06Rech"] = "K",
            });
            Assert.False(string.IsNullOrEmpty(Einstellungen.Presets[5].Id));
            Assert.Equal(Einstellungen.Presets[5].Id, Einstellungen.Hole("[Buchungsposten]05Id"));
            Assert.Equal("", Einstellungen.Presets[6].Id);   // klassisch: braucht keine
        }

        [Fact]
        public void LoeschenDesPlatzes_RaeumtAllesAb()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            Einstellungen.MerkeFeldwert(8, "anteil2", "30");

            Einstellungen.SpeicherePreset(8, new Preset("", false, 0, 1, "", ""));

            Assert.True(Einstellungen.Presets[8].IstLeer);
            Assert.False(Einstellungen.Presets[8].IstErweitert);
            Assert.Equal("", Einstellungen.HoleGemerktenFeldwert(8, "anteil2"));
        }

        [Fact]
        public void GemerkterWert_BleibtBeimSpeichernDerVorlage()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            Einstellungen.MerkeFeldwert(8, "anteil2", "30");

            // Vorlage im Editor geändert (anderes Feld), "anteil2" merkt weiter
            Einstellungen.SpeicherePreset(8, BeispielVorlage());
            Assert.Equal("30", Einstellungen.HoleGemerktenFeldwert(8, "anteil2"));
        }

        [Fact]
        public void UnveraenderteVorlage_SchreibtKeineErweiterungsKeys()
        {
            // Der Editor speichert bei jeder Feldänderung die ganze Vorlage --
            // was sich nicht geändert hat, soll keinen ini-Zugriff kosten.
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            Einstellungen.SpeicherePreset(8, BeispielVorlage());

            var geschrieben = new List<string>();
            Action<string, string> mitschreiber = (k, v) => geschrieben.Add(k);
            Einstellungen.WertGeaendert += mitschreiber;
            try { Einstellungen.SpeicherePreset(8, BeispielVorlage()); }
            finally { Einstellungen.WertGeaendert -= mitschreiber; }

            Assert.DoesNotContain(geschrieben, k => k.Contains("08F"));
            Assert.DoesNotContain(geschrieben, k => k.Contains("08O"));
            Assert.DoesNotContain(geschrieben, k => k.Contains("08BasisText"));
        }

        [Fact]
        public void KlassischesPreset_HatKeineErweiterung()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>
            {
                ["[Buchungsposten]05Text"] = "Büromaterial",
                ["[Buchungsposten]05Ausg"] = "1",
                ["[Buchungsposten]05Rech"] = "Bürobedarf"
            });
            var p = Einstellungen.Presets[5];
            Assert.False(p.IstErweitert);
            Assert.Same(VorlagenErweiterung.Leer, p.Erweiterung);
        }
    }

    // ======================================================================
    // Persistenz: XML-Austauschformat
    // ======================================================================

    public class VorlagenErweiterungXmlTests
    {
        [Fact]
        public void ExportImport_Roundtrip_MitErweiterung()
        {
            var original = new Preset("Reisekosten", true, 0, 1, "Reisekosten",
                erweiterung: new VorlagenErweiterung(
                    new[]
                    {
                        new PresetFeld("km", "gefahrene km", "zahl", "betrag", "0", true, true),
                        new PresetFeld("art", "Verkehrsmittel", "auswahl",
                            optionen: "Pkw=0,30;Motorrad=0,20")
                    },
                    new[]
                    {
                        new OriginalFeldRegel(OriginalFeld.Betrag, FeldZustand.Gesperrt, "Erstattung"),
                        new OriginalFeldRegel(OriginalFeld.Afa, FeldZustand.Ausgeblendet)
                    },
                    "$km*$art", "Fahrt $B", "$beleg-R", "Kasse", "Verlag"));

            var xml = PresetXml.Exportiere(original);
            Assert.Contains("Version=\"2\"", xml);
            var kopie = PresetXml.Importiere(xml);

            Assert.Equal(2, kopie.Felder.Count);
            Assert.Equal("km", kopie.Felder[0].Name);
            Assert.Equal(ZusatzFeldTyp.Zahl, kopie.Felder[0].Typ);
            Assert.True(kopie.Felder[0].Pflicht);
            Assert.True(kopie.Felder[0].Merken);
            Assert.Equal("Pkw=0,30;Motorrad=0,20", kopie.Felder[1].Optionen);

            var e = kopie.Erweiterung;
            Assert.Equal(FeldZustand.Gesperrt, e.ZustandVon(OriginalFeld.Betrag));
            Assert.Equal("Erstattung", e.RegelFuer(OriginalFeld.Betrag).Bezeichnung);
            Assert.Equal(FeldZustand.Ausgeblendet, e.ZustandVon(OriginalFeld.Afa));
            Assert.Equal("$km*$art", e.EingabeBetragFormel);
            Assert.Equal("Fahrt $B", e.BasisTextTemplate);
            Assert.Equal("$beleg-R", e.BasisBelegTemplate);
            Assert.Equal("Kasse", e.BasisBestandskonto);
            Assert.Equal("Verlag", e.BasisBetrieb);
        }

        [Fact]
        public void VorlageOhneErweiterung_BleibtVersion1()
        {
            // ältere Programmstände lesen sie weiter
            var xml = PresetXml.Exportiere(new Preset("Plain", true, 19000, 1, "K"));
            Assert.Contains("Version=\"1\"", xml);
            Assert.DoesNotContain("<Felder", xml);
        }

        [Fact]
        public void Import_LehntUngueltigenPlatzhalterAb()
        {
            Assert.Throws<FormatException>(() => PresetXml.Importiere(
                "<ECTBuchungsvorlage Version=\"2\"><Name>X</Name>" +
                "<Felder><Feld Name=\"brutto\" Typ=\"betrag\"/></Felder>" +
                "</ECTBuchungsvorlage>"));
        }

        [Fact]
        public void Import_IgnoriertUnbekannteOriginalFelder()
        {
            var p = PresetXml.Importiere(
                "<ECTBuchungsvorlage Version=\"2\"><Name>X</Name>" +
                "<OriginalFelder>" +
                "<Feld Name=\"Hologramm\" Zustand=\"gesperrt\"/>" +
                "<Feld Name=\"Konto\" Zustand=\"gesperrt\"/>" +
                "</OriginalFelder></ECTBuchungsvorlage>");
            var regel = Assert.Single(p.Erweiterung.OriginalFelder);
            Assert.Equal(OriginalFeld.Konto, regel.Feld);
        }

        [Fact]
        public void Beschreibung_LaesstSichNichtSperren()
        {
            var regel = new OriginalFeldRegel(OriginalFeld.Beschreibung,
                FeldZustand.Ausgeblendet, "Reisezweck");
            Assert.Equal(FeldZustand.Normal, regel.Zustand);
            Assert.Equal("Reisezweck", regel.Bezeichnung);
        }
    }

    // ======================================================================
    // Mitgelieferte Vorlagen mit Zusatz-Feldern (Vorlagen-Bibliothek)
    // ======================================================================

    [Collection("EinstellungenCache")]   // $vat1/$vat2 kommen aus dem Cache
    public class BibliothekZusatzFeldTests
    {
        private static void LadeSteuersaetze()
            => Einstellungen.LadeAusBridge(new Dictionary<string, string>
            {
                ["[Persoenliche_Daten]vat1"] = "19",
                ["[Persoenliche_Daten]vat2"] = "7"
            });

        private static Preset Vorlage(string name)
        {
            var eintrag = VorlagenBibliothek.Lade()
                .SelectMany(a => a.AlleEintraege())
                .Single(e => e.Name == name);
            return PresetXml.Importiere(eintrag.VorlagenXml);
        }

        private static Buchung Basis(decimal brutto, int mwstPromille, string text) => new Buchung
        {
            Art = Buchungsart.Ausgabe,
            Datum = new DateTime(2026, 11, 12),
            Beschreibung = text,
            Belegnummer = "17",
            Konto = "Wareneinkauf",
            Betrieb = "Verlag",
            Bestandskonto = "PayPal",
            BruttoBetrag = Betrag.AusCent((int)(brutto * 100m), mwstPromille)
        };

        /// <summary>Feldwerte in der Reihenfolge der Felder der Vorlage.</summary>
        private static FeldWert[] Werte(Preset p, params string[] roh)
        {
            Assert.Equal(p.Felder.Count, roh.Length);
            return p.Felder.Select((f, i) => VorlagenFelder.Werte(f, roh[i])).ToArray();
        }

        [Fact]
        public void JedeVorlageDerBibliothek_IstInSichStimmig()
        {
            // Dieselbe Prüfung, die der Vorlagen-Editor anzeigt: keine
            // unbekannten Platzhalter in Formeln, kein ausgeblendetes Feld
            // ohne Wert aus der Vorlage.
            LadeSteuersaetze();
            foreach (var e in VorlagenBibliothek.Lade().SelectMany(a => a.AlleEintraege()))
            {
                var probleme = VorlagenFelder.Pruefe(PresetXml.Importiere(e.VorlagenXml));
                Assert.True(probleme.Count == 0,
                    e.Name + ": " + string.Join(" | ", probleme));
            }
        }

        [Fact]
        public void ZweiSteuersaetze_ZweiterSatzAusDenEinstellungenVorbelegt()
        {
            LadeSteuersaetze();
            var p = Vorlage("Rechnung mit zwei Steuersätzen");
            var satz = p.Felder.Single(f => f.Name == "satz2");
            Assert.Equal("7", VorlagenFelder.StandardWert(satz));

            var b = BuchungsgruppenRechner.Berechne(p, Basis(119m, 19000, "Bürobedarf"),
                Werte(p, "10,70", "7"));
            Assert.False(b.HatFehler);
            Assert.Equal(10.70m, b.Zeilen[0].Brutto);
            Assert.Equal(7000, b.Zeilen[0].MwstPromille);
            Assert.Equal("Bürobedarf (7 %)", b.Zeilen[0].Beschreibung);
            Assert.Equal(108.30m, b.BasisGebuchtBrutto);
        }

        [Fact]
        public void ZweiSteuersaetze_OhneZweitenSatzInDenEinstellungenKeineVorbelegung()
        {
            Einstellungen.LadeAusBridge(new Dictionary<string, string>());
            var satz = Vorlage("Rechnung mit zwei Steuersätzen").Felder.Single(f => f.Name == "satz2");
            Assert.Equal("", VorlagenFelder.StandardWert(satz));
        }

        [Fact]
        public void Wareneinkauf_SummeIstDerGezahlteBetrag()
        {
            LadeSteuersaetze();
            var p = Vorlage("Wareneinkauf mit Versand und Skonto");
            var basis = Basis(238m, 19000, "Müller GmbH R-4711");
            var felder = Werte(p, "11,90", "4,76");
            var b = BuchungsgruppenRechner.Berechne(p, basis, felder);

            Assert.Equal(11.90m, b.Zeilen[0].Brutto);
            Assert.Equal(-4.76m, b.Zeilen[1].Brutto);          // Skonto mindert
            Assert.Equal(226.10m, b.BasisGebuchtBrutto);        // Warenwert
            Assert.Equal("", b.BasisFehler);
            Assert.Equal("Müller GmbH R-4711 (Warenwert)", b.BasisBeschreibung);
            Assert.Equal("Müller GmbH R-4711 (Skonto)", b.Zeilen[1].Beschreibung);
            // Warenwert + Versand - Skonto = Rechnung - Skonto = gezahlt
            Assert.Equal(238m - 4.76m,
                b.BasisGebuchtBrutto + b.GebuchteZeilen.Sum(z => z.Brutto));

            // ohne Versand und Skonto: eine einzige Buchung
            b = BuchungsgruppenRechner.Berechne(p, basis, Werte(p, "", ""));
            Assert.Empty(b.GebuchteZeilen);
            Assert.Equal(238m, b.BasisGebuchtBrutto);
        }

        [Fact]
        public void Bewirtung_TeiltSiebzigDreissig_SummeBleibtDerGezahlteBetrag()
        {
            LadeSteuersaetze();
            var p = Vorlage("Bewirtungskosten");
            var b = BuchungsgruppenRechner.Berechne(p, Basis(119m, 19000, "Bewirtungskosten"),
                Werte(p, "Projektabschluss", "Müller, Meier", "10"));

            Assert.Equal(32.70m, b.Zeilen[0].Brutto);   // 30 % von 109 (Rechnung)
            Assert.Equal(19000, b.Zeilen[0].MwstPromille);
            Assert.Equal(7.00m, b.Zeilen[1].Brutto);    // 70 % des Trinkgelds
            Assert.Equal(0, b.Zeilen[1].MwstPromille);
            Assert.Equal(3.00m, b.Zeilen[2].Brutto);    // 30 % des Trinkgelds
            Assert.Equal(76.30m, b.BasisGebuchtBrutto); // 70 % von 109
            Assert.Equal(119m, b.BasisGebuchtBrutto + b.Zeilen.Sum(z => z.Brutto));
            Assert.Equal("Bewirtung Projektabschluss (Müller, Meier)", b.BasisBeschreibung);

            // ohne Trinkgeld entfallen dessen beide Zeilen
            b = BuchungsgruppenRechner.Berechne(p, Basis(100.05m, 19000, "Bewirtungskosten"),
                Werte(p, "Akquise", "Schulz", ""));
            Assert.Single(b.GebuchteZeilen);
            // Rundungsrest landet in der Basis: Summe bleibt exakt
            Assert.Equal(100.05m, b.BasisGebuchtBrutto + b.GebuchteZeilen.Sum(z => z.Brutto));
        }

        [Fact]
        public void Darlehensrate_ZinsenAlsBasis_TilgungAlsZeile()
        {
            LadeSteuersaetze();
            var p = Vorlage("Darlehensrate");
            var b = BuchungsgruppenRechner.Berechne(p, Basis(500m, 0, "Darlehensrate"),
                Werte(p, "83,12"));
            Assert.Equal(83.12m, b.BasisGebuchtBrutto);
            Assert.Equal(416.88m, b.Zeilen[0].Brutto);
            Assert.Equal("/Darlehenstilgung", b.Zeilen[0].Konto);
            Assert.Equal("Darlehensrate (Zinsen)", b.BasisBeschreibung);
            Assert.Equal("Darlehensrate (Tilgung)", b.Zeilen[0].Beschreibung);
            Assert.False(b.HatFehler);
        }

        [Fact]
        public void Verpflegung_UndFahrtkosten_BetragAusMengen()
        {
            LadeSteuersaetze();
            var p = Vorlage("Verpflegungsmehraufwand");
            Assert.Equal(FeldZustand.Gesperrt, p.Erweiterung.ZustandVon(OriginalFeld.Betrag));
            Assert.Equal(56m, BuchungsgruppenRechner.BerechneEingabeBetrag(
                p, Werte(p, "2", "1")).Wert);

            p = Vorlage("Fahrtkosten privater Pkw");
            var felder = Werte(p, "150");
            Assert.Equal(45m, BuchungsgruppenRechner.BerechneEingabeBetrag(p, felder).Wert);
            var b = BuchungsgruppenRechner.Berechne(p, Basis(45m, 0, "Messe Köln"), felder);
            Assert.Equal("Fahrtkosten Messe Köln (150 km)", b.BasisBeschreibung);
        }

        [Fact]
        public void Umbuchung_GegenbuchungAufDemZielkonto()
        {
            LadeSteuersaetze();
            var p = Vorlage("Umbuchung zwischen Bestandskonten");
            var basis = Basis(250m, 0, "Auszahlung PayPal");
            basis.Konto = p.Konto;
            var felder = Werte(p, "Girokonto");
            var b = BuchungsgruppenRechner.Berechne(p, basis, felder);
            Assert.False(b.HatFehler);

            var buchungen = BuchungsgruppenRechner.BaueBuchungen(p, 40, basis, b, null, felder);
            Assert.Equal(2, buchungen.Count);
            Assert.Equal(Buchungsart.Ausgabe, buchungen[0].Art);
            Assert.Equal("PayPal", buchungen[0].Bestandskonto);
            Assert.Equal(Buchungsart.Einnahme, buchungen[1].Art);
            Assert.Equal("Girokonto", buchungen[1].Bestandskonto);
            Assert.Equal(buchungen[0].BruttoBetrag.InCent, buchungen[1].BruttoBetrag.InCent);
            Assert.True(buchungen[0].IstNeutralesKonto && buchungen[1].IstNeutralesKonto);
            Assert.Equal("Girokonto", buchungen[0].VorlagenFeldWerte()["zielkonto"]);
        }
    }

    // ======================================================================
    // Prüfung einer Vorlagen-Definition
    // ======================================================================

    public class VorlagenPruefungTests
    {
        private static IReadOnlyList<string> Pruefe(VorlagenErweiterung e,
            string konto = "K", List<PresetZeile> zeilen = null, string basisBetrag = null)
            => VorlagenFelder.Pruefe(new Preset("V", true, 0, 1, konto,
                zeilen: zeilen, basisBetragFormel: basisBetrag, erweiterung: e));

        [Fact]
        public void SaubereVorlage_OhneProbleme()
        {
            Assert.Empty(Pruefe(new VorlagenErweiterung(
                new[] { new PresetFeld("anteil2", "Anteil", "prozent", vorbelegung: "50") }),
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "K2", "", "$brutto*$anteil2/100", "")
                },
                basisBetrag: "$rest"));
        }

        [Fact]
        public void AusgeblendeterBetrag_BrauchtFormel()
        {
            var probleme = Pruefe(new VorlagenErweiterung(originalFelder: new[]
            {
                new OriginalFeldRegel(OriginalFeld.Betrag, FeldZustand.Ausgeblendet)
            }));
            Assert.Contains(probleme, p => p.Contains("Betrag"));
        }

        [Fact]
        public void GesperrtesKonto_BrauchtKontoInDerVorlage()
        {
            var e = new VorlagenErweiterung(originalFelder: new[]
            {
                new OriginalFeldRegel(OriginalFeld.Konto, FeldZustand.Gesperrt)
            });
            Assert.NotEmpty(Pruefe(e, konto: ""));
            Assert.Empty(Pruefe(e, konto: "Bürobedarf"));
        }

        [Fact]
        public void DoppelterPlatzhalter_UndAuswahlOhneOptionen()
        {
            var probleme = Pruefe(new VorlagenErweiterung(new[]
            {
                new PresetFeld("feld", "A", "text"),
                new PresetFeld("FELD", "B", "text"),
                new PresetFeld("wahl", "C", "auswahl")
            }));
            Assert.Contains(probleme, p => p.Contains("mehrfach"));
            Assert.Contains(probleme, p => p.Contains("Auswahlmöglichkeiten"));
        }

        [Fact]
        public void Formel_MitUnbekanntemFeld_WirdGemeldet()
        {
            var probleme = Pruefe(new VorlagenErweiterung(
                new[] { new PresetFeld("anteil2", "Anteil", "prozent") }),
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "K2", "", "$brutto*$anteil3/100", "")
                });
            Assert.Contains(probleme, p => p.Contains("anteil3"));
        }

        [Fact]
        public void TextFeld_IstInFormelnKeineZahl()
        {
            var probleme = Pruefe(new VorlagenErweiterung(
                new[] { new PresetFeld("anlass", "Anlass", "text") }),
                zeilen: new List<PresetZeile>
                {
                    new PresetZeile("", "K2", "", "$brutto*$anlass", "")
                });
            Assert.Contains(probleme, p => p.Contains("anlass"));
        }

        [Fact]
        public void ZeileOhneFormel_WirdAlsHinweisGemeldet()
        {
            var probleme = Pruefe(VorlagenErweiterung.Leer,
                zeilen: new List<PresetZeile> { new PresetZeile("", "K2", "", "", "") });
            Assert.Contains(probleme, p => p.Contains("übersprungen"));
        }
    }
}
