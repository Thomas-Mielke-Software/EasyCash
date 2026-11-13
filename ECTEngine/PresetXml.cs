// PresetXml.cs -- XML-Export/Import einzelner Buchungs-Presets bzw.
// Buchungsgruppen-Vorlagen (zum Teilen, z.B. im Forum).
//
// Bewusst REINE DATEN: das Format enthält nur Feldwerte und Formel-
// Strings, die vom FormelParser interpretiert werden -- keine
// ausführbaren Anteile (siehe Design-Entscheidung "kein C#-Scripting").
//
// Format (Version 2; Vorlagen OHNE Dialog-Erweiterung werden weiter als
// Version 1 geschrieben, damit ältere Programmstände sie lesen können):
//   <ECTBuchungsvorlage Version="2">
//     <Name>Reverse Charge §13b</Name>
//     <Typ>A</Typ>                  (E = Einnahme, A = Ausgabe)
//     <MwSt>19000</MwSt>            (x1000)
//     <AfaJahre>1</AfaJahre>
//     <Konto>Fremdleistungen</Konto>
//     <Notiz>...</Notiz>
//     <Degressiv>false</Degressiv>
//     <AfaSatz>0</AfaSatz>
//     <BasisBetrag>$rest</BasisBetrag>
//     <Zeilen>
//       <Zeile Typ="A" Konto="..." MwSt="$vat2" Betrag="$netto*0,19"
//              Text="..." Beleg="" Betrieb="" Bestandskonto=""
//              Darstellung=""/>       (Typ leer = wie Basis)
//     </Zeilen>
//     [ab hier Version 2: Erweiterung des Buchen-Dialogs]
//     <EingabeBetrag>$tage*14</EingabeBetrag>   (Betrag bei gesperrtem Feld)
//     <BasisText>Bewirtung $anlass</BasisText>
//     <BasisBeleg></BasisBeleg>
//     <BasisBestandskonto></BasisBestandskonto>
//     <BasisBetrieb></BasisBetrieb>
//     <OriginalFelder>
//       <Feld Name="Betrag" Zustand="ausgeblendet" Bezeichnung=""/>
//     </OriginalFelder>
//     <Felder>
//       <Feld Name="anteil2" Bezeichnung="Anteil Betrieb 2" Typ="prozent"
//             Box="betrag" Vorbelegung="50" Pflicht="true" Merken="false"
//             Optionen=""/>
//     </Felder>
//   </ECTBuchungsvorlage>
//
// "Typ" ist überall einheitlich E/A (kein bool); beim Import werden die
// kurzlebigen Ur-Schreibweisen <Ausgabe>true/false</Ausgabe> und
// Zeilen-Attribut "Art" noch toleriert.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Xml.Linq;

namespace ECTEngine
{
    public static class PresetXml
    {
        public const string WurzelElement = "ECTBuchungsvorlage";
        public const int AktuelleVersion = 2;

        /// <summary>Version, die eine Vorlage mindestens braucht: 2, sobald
        /// sie den Buchen-Dialog erweitert (ein älteres Programm ließe die
        /// Zusatz-Felder stillschweigend weg und scheiterte dann an den
        /// Formeln, die sie verwenden).</summary>
        private static int NoetigeVersion(Preset p)
            => p.Erweiterung.IstLeer ? 1 : 2;

        /// <summary>Serialisiert ein Preset (inkl. Zusatz-Zeilen) als XML.</summary>
        public static string Exportiere(Preset p)
        {
            if (p == null) throw new ArgumentNullException(nameof(p));

            var wurzel = new XElement(WurzelElement,
                new XAttribute("Version", NoetigeVersion(p)),
                new XElement("Name", p.Text),
                new XElement("Typ", p.Ausgabe ? "A" : "E"),
                new XElement("MwSt", p.Mwst.ToString(CultureInfo.InvariantCulture)),
                new XElement("AfaJahre", p.AfaJ.ToString(CultureInfo.InvariantCulture)),
                new XElement("Konto", p.Konto),
                new XElement("Notiz", p.Notiz),
                new XElement("Degressiv", p.Degressiv ? "true" : "false"),
                new XElement("AfaSatz", p.AfaSatz.ToString(CultureInfo.InvariantCulture)),
                new XElement("BasisBetrag", p.BasisBetragFormel));

            var zeilen = new XElement("Zeilen");
            foreach (var z in p.Zeilen)
            {
                zeilen.Add(new XElement("Zeile",
                    new XAttribute("Typ", z.Art),
                    new XAttribute("Konto", z.Konto),
                    new XAttribute("MwSt", z.MwstAusdruck),
                    new XAttribute("Betrag", z.BetragFormel),
                    new XAttribute("Text", z.TextTemplate),
                    new XAttribute("Beleg", z.BelegTemplate),
                    new XAttribute("Betrieb", z.BetriebTemplate),
                    new XAttribute("Bestandskonto", z.BestandskontoTemplate),
                    new XAttribute("Darstellung", z.Darstellung)));
            }
            wurzel.Add(zeilen);

            var e = p.Erweiterung;
            if (!e.IstLeer)
            {
                wurzel.Add(
                    new XElement("EingabeBetrag", e.EingabeBetragFormel),
                    new XElement("BasisText", e.BasisTextTemplate),
                    new XElement("BasisBeleg", e.BasisBelegTemplate),
                    new XElement("BasisBestandskonto", e.BasisBestandskonto),
                    new XElement("BasisBetrieb", e.BasisBetrieb));

                var regeln = new XElement("OriginalFelder");
                foreach (var r in e.OriginalFelder)
                    regeln.Add(new XElement("Feld",
                        new XAttribute("Name", r.Feld),
                        new XAttribute("Zustand", OriginalFeldRegel.ZustandName(r.Zustand)),
                        new XAttribute("Bezeichnung", r.Bezeichnung)));
                wurzel.Add(regeln);

                var felder = new XElement("Felder");
                foreach (var f in e.Felder)
                    felder.Add(new XElement("Feld",
                        new XAttribute("Name", f.Name),
                        new XAttribute("Bezeichnung", f.Bezeichnung),
                        new XAttribute("Typ", f.TypName),
                        new XAttribute("Box", f.Box),
                        new XAttribute("Vorbelegung", f.Vorbelegung),
                        new XAttribute("Pflicht", f.Pflicht ? "true" : "false"),
                        new XAttribute("Merken", f.Merken ? "true" : "false"),
                        new XAttribute("Optionen", f.Optionen)));
                wurzel.Add(felder);
            }

            return new XDocument(
                new XDeclaration("1.0", "utf-8", null), wurzel).ToString();
        }

        /// <summary>
        /// Parst eine exportierte Vorlage. Wirft <see cref="FormatException"/>
        /// mit deutschem Fehlertext bei ungültigem Inhalt (der Aufrufer zeigt
        /// ihn als MessageBox).
        /// </summary>
        public static Preset Importiere(string xml)
        {
            if (string.IsNullOrWhiteSpace(xml))
                throw new FormatException("Die Datei ist leer.");

            XElement wurzel;
            try { wurzel = XDocument.Parse(xml).Root; }
            catch (System.Xml.XmlException ex)
            {
                throw new FormatException("Kein gültiges XML: " + ex.Message);
            }

            if (wurzel == null || wurzel.Name.LocalName != WurzelElement)
                throw new FormatException(
                    $"Keine EasyCash&Tax-Buchungsvorlage (Wurzelelement '{WurzelElement}' fehlt).");

            int version = LiesInt(wurzel.Attribute("Version")?.Value, 1);
            if (version > AktuelleVersion)
                throw new FormatException(
                    $"Die Vorlage hat Format-Version {version}; dieses Programm " +
                    $"unterstützt nur Version {AktuelleVersion} oder älter. " +
                    "Bitte EasyCash&Tax aktualisieren.");

            string Wert(string name) => wurzel.Element(name)?.Value ?? "";

            var zeilen = new List<PresetZeile>();
            var zeilenElement = wurzel.Element("Zeilen");
            if (zeilenElement != null)
            {
                foreach (var z in zeilenElement.Elements("Zeile"))
                {
                    string Attr(string name) => z.Attribute(name)?.Value ?? "";
                    // "Typ" (E/A, leer = wie Basis); Fallback "Art" (Ur-Format)
                    string typ = NormalisiereTyp(
                        z.Attribute("Typ")?.Value ?? Attr("Art"));
                    var zeile = new PresetZeile(
                        typ, Attr("Konto"), Attr("MwSt"), Attr("Betrag"),
                        Attr("Text"), Attr("Beleg"), Attr("Betrieb"),
                        Attr("Bestandskonto"), Attr("Darstellung"));
                    if (zeile.Konto.Length == 0)
                        throw new FormatException(
                            "Eine Vorlagen-Zeile hat kein Konto (Pflichtfeld).");
                    zeilen.Add(zeile);
                }
            }
            if (zeilen.Count > Einstellungen.MaxPresetZeilen - 1)
                throw new FormatException(
                    $"Zu viele Zeilen ({zeilen.Count}); maximal " +
                    $"{Einstellungen.MaxPresetZeilen - 1} Zusatz-Zeilen möglich.");

            // Erweiterung des Buchen-Dialogs (ab Version 2). Unbekannte
            // Original-Felder werden ignoriert, ein ungültiger Platzhalter
            // dagegen abgelehnt -- er käme in keiner Formel an.
            var regeln = new List<OriginalFeldRegel>();
            var regelnElement = wurzel.Element("OriginalFelder");
            if (regelnElement != null)
            {
                foreach (var r in regelnElement.Elements("Feld"))
                {
                    string feld = r.Attribute("Name")?.Value ?? "";
                    if (OriginalFeld.Finde(feld) == null) continue;
                    regeln.Add(new OriginalFeldRegel(feld,
                        OriginalFeldRegel.ZustandAusName(r.Attribute("Zustand")?.Value),
                        r.Attribute("Bezeichnung")?.Value));
                }
            }

            var felder = new List<PresetFeld>();
            var felderElement = wurzel.Element("Felder");
            if (felderElement != null)
            {
                foreach (var f in felderElement.Elements("Feld"))
                {
                    string FAttr(string attr) => f.Attribute(attr)?.Value ?? "";
                    var feld = new PresetFeld(FAttr("Name"), FAttr("Bezeichnung"),
                        FAttr("Typ"), FAttr("Box"), FAttr("Vorbelegung"),
                        IstWahr(FAttr("Pflicht")), IstWahr(FAttr("Merken")),
                        FAttr("Optionen"));
                    string nameFehler = VorlagenFelder.PruefeName(feld.Name);
                    if (nameFehler.Length > 0)
                        throw new FormatException(
                            "Zusatz-Feld \"" + feld.AnzeigeBezeichnung + "\": " + nameFehler);
                    felder.Add(feld);
                }
            }
            if (felder.Count > VorlagenFelder.MaxFelder)
                throw new FormatException(
                    $"Zu viele Zusatz-Felder ({felder.Count}); maximal " +
                    $"{VorlagenFelder.MaxFelder} möglich.");

            var name = Wert("Name");
            if (string.IsNullOrWhiteSpace(name))
                throw new FormatException("Die Vorlage hat keinen Namen.");

            // "Typ" (E/A); Fallback aufs kurzlebige Ur-Format <Ausgabe>bool
            bool ausgabe;
            string typBasis = NormalisiereTyp(Wert("Typ"));
            if (typBasis == "E")
                ausgabe = false;
            else if (typBasis == "A")
                ausgabe = true;
            else
                ausgabe = !string.Equals(Wert("Ausgabe"), "false",
                    StringComparison.OrdinalIgnoreCase);

            return new Preset(
                name,
                ausgabe,
                LiesInt(Wert("MwSt"), 0),
                Math.Max(1, LiesInt(Wert("AfaJahre"), 1)),
                Wert("Konto"),
                Wert("Notiz"),
                string.Equals(Wert("Degressiv"), "true", StringComparison.OrdinalIgnoreCase),
                LiesInt(Wert("AfaSatz"), 0),
                zeilen,
                Wert("BasisBetrag"),
                new VorlagenErweiterung(felder, regeln,
                    Wert("EingabeBetrag"), Wert("BasisText"), Wert("BasisBeleg"),
                    Wert("BasisBestandskonto"), Wert("BasisBetrieb")));
        }

        private static bool IstWahr(string s)
        {
            s = (s ?? "").Trim().ToLowerInvariant();
            return s == "true" || s == "1" || s == "ja";
        }

        /// <summary>Normalisiert einen Typ-Wert auf "E", "A" oder ""
        /// (= wie Basis / nicht angegeben).</summary>
        private static string NormalisiereTyp(string s)
        {
            s = (s ?? "").Trim().ToUpperInvariant();
            return s == "E" || s == "A" ? s : "";
        }

        private static int LiesInt(string s, int fallback)
        {
            return int.TryParse((s ?? "").Trim(), NumberStyles.Integer,
                CultureInfo.InvariantCulture, out int n) ? n : fallback;
        }
    }
}
