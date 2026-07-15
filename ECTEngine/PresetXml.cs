// PresetXml.cs -- XML-Export/Import einzelner Buchungs-Presets bzw.
// Buchungsgruppen-Vorlagen (zum Teilen, z.B. im Forum).
//
// Bewusst REINE DATEN: das Format enthält nur Feldwerte und Formel-
// Strings, die vom FormelParser interpretiert werden -- keine
// ausführbaren Anteile (siehe Design-Entscheidung "kein C#-Scripting").
//
// Format (Version 1):
//   <ECTBuchungsvorlage Version="1">
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
        public const int AktuelleVersion = 1;

        /// <summary>Serialisiert ein Preset (inkl. Zusatz-Zeilen) als XML.</summary>
        public static string Exportiere(Preset p)
        {
            if (p == null) throw new ArgumentNullException(nameof(p));

            var wurzel = new XElement(WurzelElement,
                new XAttribute("Version", AktuelleVersion),
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
                Wert("BasisBetrag"));
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
