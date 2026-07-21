// FormularDefinition.cs -- Vollparser fuer .ecf-Formulardateien.
//
// Pendant zum nativen XMLite-Lesen in DrawFormularToDC (easycashview.cpp:4703)
// bzw. GetFormularwertByIndex (ECTBridge/easycashdoc.cpp:2259): liest ALLE
// Formular-Attribute inkl. Layout-Geometrie, Seitenbilder und Abschnitte.
//
// Abgrenzung zu EUKonten.LadeFormularGruppen (EUKonten.cs): dort werden nur
// Feld-Id/Bezeichnung/Einnahme-Flag der zuweisbaren Felder fuer die
// Konten-Seite gelesen (prozessweiter Cache). Diese Klasse hier laedt EINE
// .ecf-Datei komplett und on demand -- fuer die WPF-Formular-Ansicht, den
// FormularRechner und (spaeter) den Designer. EUKonten bleibt unangetastet.
//
// Koordinaten: horizontal/vertikal sind Promille der Seitenbreite (0-1000)
// bzw. Seitenlaenge (0-1414; 1000x1414 = A4-Seitenverhaeltnis Wurzel 2).
// Werte > 1000 kommen vor (z.B. Feld 1111 der EUeR-d-2025) -- nicht clippen!
//
// Encoding: aktuelle .ecf deklarieren encoding="windows-1252"; sehr alte
// Dateien (z.B. EUeR2005.ecf) haben KEINE Deklaration, enthalten aber rohe
// Windows-1252-Bytes. XDocument.Load wuerde die als UTF-8 lesen und
// scheitern -- deshalb Fallback auf StreamReader mit CP1252.
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2026 Thomas Mielke

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Linq;

namespace ECTEngine
{
    /// <summary>Feldtyp eines .ecf-Formularfelds (Attribut typ).</summary>
    public enum FormularFeldTyp
    {
        Unbekannt = 0,
        Einnahmen,          // Summe der Buchungen verknuepfter Einnahmen-Konten
        Ausgaben,           // dito Ausgaben (mit AfA-Sonderbehandlung)
        Summe,              // Formel ueber Feld-Ids und Kontenkategorie-Terme
        Einstellungsdaten,  // Stammdaten aus der ini (Kurzform-Keys wie "fname")
        Dokumentdaten,      // Werte aus dem Dokument-ErweiterungStore
        Freitext            // statischer Text mit Platzhaltern
    }

    /// <summary>Betragsanteil eines Felds (Attribut anteil).</summary>
    public enum FormularFeldAnteil
    {
        Brutto = 0,   // Default wie nativ: alles ausser "netto"/"mwst" ist brutto
        Netto,
        Mwst
    }

    /// <summary>Ein Feld einer .ecf-Formulardatei (alle Attribute).</summary>
    public sealed class FormularFeldDef
    {
        public int    Id            { get; set; }
        public FormularFeldTyp Typ  { get; set; }
        /// <summary>Roher typ-Attributtext (fuer Unbekannt-Diagnose/Designer).</summary>
        public string TypRoh        { get; set; } = "";
        public int    Seite         { get; set; } = 1;
        /// <summary>Promille der Seitenbreite (kann &gt; 1000 sein).</summary>
        public int    Horizontal    { get; set; }
        /// <summary>Promille der Seitenlaenge (0-1414).</summary>
        public int    Vertikal      { get; set; }
        /// <summary>Default rechtsbuendig -- nativ ist alles ausser
        /// "linksbuendig" rechtsbuendig (easycashview.cpp:4888).</summary>
        public bool   RechtsBuendig { get; set; } = true;
        public FormularFeldAnteil Anteil { get; set; } = FormularFeldAnteil.Brutto;
        /// <summary>true = Attribut nachkommaanteil="ohne" (ganze Euro,
        /// Truncation wie nativ).</summary>
        public bool   NachkommaanteilOhne { get; set; }
        /// <summary>true = Attribut nullwertdarstellung="ja" ("0,00" wird
        /// angezeigt statt unterdrueckt).</summary>
        public bool   NullwertAnzeigen { get; set; }
        public bool   Veraltet      { get; set; }
        /// <summary>ELSTER-Mapping-Metadaten (nativ nicht ausgewertet).</summary>
        public string Elsterfeldname { get; set; } = "";
        /// <summary>Beschreibung (Kindelement name).</summary>
        public string Name          { get; set; } = "";
        /// <summary>Formel bzw. Einstellungs-Key (Kindelement erweiterung).</summary>
        public string Erweiterung   { get; set; } = "";

        /// <summary>true wenn das feld-Element Inhalt hat (Kindelemente oder
        /// Text). Pendant zum nativen !child-&gt;value.IsEmpty()-Check
        /// (easycashdoc.cpp:2314): selbstschliessende/leere feld-Elemente
        /// werden weder gerechnet noch gezeichnet.</summary>
        public bool   HatInhalt     { get; set; }

        /// <summary>Quell-Element im gehaltenen XDocument -- fuer das
        /// Designer-Rueckschreiben (Phase 6). Nicht fuer Rechenlogik nutzen.</summary>
        public XElement Quelle      { get; set; }
    }

    /// <summary>Navigationsanker (Kindelement abschnitt) fuer die Seitenleiste.</summary>
    public sealed class FormularAbschnittDef
    {
        public string Name     { get; set; } = "";
        public int    Seite    { get; set; } = 1;
        /// <summary>Promille der Seitenlaenge (0-1414).</summary>
        public int    Vertikal { get; set; }

        public XElement Quelle { get; set; }
    }

    /// <summary>
    /// Eine komplett geparste .ecf-Formulardatei. Laden on demand ueber
    /// <see cref="Lade"/>; das XDocument bleibt fuer das spaetere
    /// Designer-Rueckschreiben erhalten.
    /// </summary>
    public sealed class FormularDefinition
    {
        /// <summary>Interner Name (Schluessel der Feldzuweisungen, z.B. "E/Ü-Rechnung").</summary>
        public string Name        { get; private set; } = "";
        public string Anzeigename { get; private set; } = "";
        public int    Seitenzahl  { get; private set; } = 1;
        public string Schriftart  { get; private set; } = "";
        /// <summary>Roher schriftgroesse-Attributwert (nativ: lfHeight in
        /// Bildschirm-Pixeln bei Zoom 100).</summary>
        public int    Schriftgroesse { get; private set; }
        /// <summary>Filter-Attribut, z.B. "betrieb" (leer = kein Filter).</summary>
        public string Filter      { get; private set; } = "";
        public bool   Querformat  { get; private set; }
        /// <summary>0 = Jahresformular; 1-12 = Monat; 13-16 = Quartal.</summary>
        public int    Voranmeldungszeitraum { get; private set; }
        /// <summary>Pfad der geladenen .ecf-Datei.</summary>
        public string Quellpfad   { get; private set; } = "";

        public IReadOnlyList<FormularFeldDef>      Felder     => _felder;
        public IReadOnlyList<FormularAbschnittDef> Abschnitte => _abschnitte;
        /// <summary>Seitennummer (1-basiert) -> Bilddateiname (relativ zum
        /// Programmverzeichnis, wie nativ easycashview.cpp:4767).</summary>
        public IReadOnlyDictionary<int, string>    Seitenbilder => _seitenbilder;

        private readonly List<FormularFeldDef>      _felder       = new List<FormularFeldDef>();
        private readonly List<FormularAbschnittDef> _abschnitte   = new List<FormularAbschnittDef>();
        private readonly Dictionary<int, string>    _seitenbilder = new Dictionary<int, string>();

        /// <summary>Gehaltenes Roh-Dokument fuer das Designer-Rueckschreiben.</summary>
        public XDocument Dokument { get; private set; }

        private FormularDefinition() { }

        // -----------------------------------------------------------------
        // Laden
        // -----------------------------------------------------------------

        /// <summary>
        /// Laedt und parst eine .ecf-Datei. Wirft bei nicht lesbarer Datei
        /// oder fehlendem formular-Root (IOException/XmlException/
        /// InvalidDataException) -- Aufrufer zeigen die Meldung an, wie der
        /// native Pfad ("Formular ... konnte nicht gefunden werden").
        /// Unbekannte Attribute/Feldtypen werden tolerant behandelt
        /// (25 Jahre Dateibestand).
        /// </summary>
        public static FormularDefinition Lade(string pfad)
        {
            if (string.IsNullOrEmpty(pfad)) throw new ArgumentNullException(nameof(pfad));

            XDocument doc;
            try
            {
                // Normalfall: XML-Deklaration mit encoding="windows-1252"
                // (oder korrektes UTF-8) -- der XmlReader folgt der Deklaration.
                doc = XDocument.Load(pfad, LoadOptions.PreserveWhitespace);
            }
            catch (XmlException)
            {
                // Alt-Dateien ohne Deklaration mit rohen CP1252-Bytes.
                using (var reader = new StreamReader(pfad, Encoding.GetEncoding(1252)))
                    doc = XDocument.Load(reader, LoadOptions.PreserveWhitespace);
            }

            var root = doc.Root;
            if (root == null || root.Name.LocalName != "formular")
                throw new InvalidDataException(
                    "'" + pfad + "' ist keine .ecf-Formulardatei (formular-Root fehlt).");

            var def = new FormularDefinition
            {
                Dokument    = doc,
                Quellpfad   = pfad,
                Name        = (string)root.Attribute("name") ?? "",
                Anzeigename = (string)root.Attribute("anzeigename") ?? "",
                Schriftart  = (string)root.Attribute("schriftart") ?? "",
                Filter      = (string)root.Attribute("filter") ?? "",
                Seitenzahl  = Math.Max(1, Atoi((string)root.Attribute("seiten"))),
                Schriftgroesse = Atoi((string)root.Attribute("schriftgroesse")),
                Querformat  = Atoi((string)root.Attribute("querformat")) != 0,
                Voranmeldungszeitraum = Atoi((string)root.Attribute("voranmeldungszeitraum"))
            };
            if (string.IsNullOrEmpty(def.Anzeigename)) def.Anzeigename = def.Name;

            // Felder -- Reihenfolge der Datei beibehalten (der Rechner liefert
            // Werte in derselben Reihenfolge, wie nativ das Index-Array)
            var felderNode = root.Element("felder");
            if (felderNode != null)
                foreach (var feld in felderNode.Elements("feld"))
                    def._felder.Add(ParseFeld(feld));

            // Seitenbilder (amtliche Formularscans als PNG)
            var seitenNode = root.Element("seiten");
            if (seitenNode != null)
            {
                foreach (var seite in seitenNode.Elements("seite"))
                {
                    int nr = Atoi((string)seite.Attribute("nr"));
                    string bild = (seite.Value ?? "").Trim();
                    if (nr >= 1 && bild.Length > 0)
                        def._seitenbilder[nr] = bild;
                }
            }

            // Abschnitte (Navigationsanker der Seitenleiste)
            var abschnitteNode = root.Element("abschnitte");
            if (abschnitteNode != null)
            {
                foreach (var a in abschnitteNode.Elements("abschnitt"))
                {
                    def._abschnitte.Add(new FormularAbschnittDef
                    {
                        Name     = (string)a.Attribute("name") ?? "",
                        Seite    = Math.Max(1, Atoi((string)a.Attribute("seite"))),
                        Vertikal = Atoi((string)a.Attribute("vertikal")),
                        Quelle   = a
                    });
                }
            }

            return def;
        }

        private static FormularFeldDef ParseFeld(XElement feld)
        {
            string typRoh = (string)feld.Attribute("typ") ?? "";
            string ausrichtung = (string)feld.Attribute("ausrichtung") ?? "";

            return new FormularFeldDef
            {
                Id            = Atoi((string)feld.Attribute("id")),
                Typ           = ParseTyp(typRoh),
                TypRoh        = typRoh,
                Seite         = Math.Max(1, Atoi((string)feld.Attribute("seite"))),
                Horizontal    = Atoi((string)feld.Attribute("horizontal")),
                Vertikal      = Atoi((string)feld.Attribute("vertikal")),
                // nativ: nur exakt "linksbuendig" ist links, alles andere rechts
                RechtsBuendig = !ausrichtung.Equals("linksbuendig", StringComparison.OrdinalIgnoreCase),
                Anteil        = ParseAnteil((string)feld.Attribute("anteil") ?? ""),
                NachkommaanteilOhne = IstJa((string)feld.Attribute("nachkommaanteil"), "ohne"),
                NullwertAnzeigen    = IstJa((string)feld.Attribute("nullwertdarstellung"), "ja"),
                Veraltet            = IstJa((string)feld.Attribute("veraltet"), "ja"),
                Elsterfeldname = (string)feld.Attribute("elsterfeldname") ?? "",
                Name          = (string)feld.Element("name") ?? "",
                Erweiterung   = (string)feld.Element("erweiterung") ?? "",
                HatInhalt     = feld.Nodes().GetEnumerator().MoveNext(),
                Quelle        = feld
            };
        }

        private static FormularFeldTyp ParseTyp(string typ)
        {
            // nativ per stricmp (case-insensitiv), easycashdoc.cpp:2317ff
            if (typ.Equals("Einnahmen", StringComparison.OrdinalIgnoreCase)) return FormularFeldTyp.Einnahmen;
            if (typ.Equals("Ausgaben", StringComparison.OrdinalIgnoreCase)) return FormularFeldTyp.Ausgaben;
            if (typ.Equals("Summe", StringComparison.OrdinalIgnoreCase)) return FormularFeldTyp.Summe;
            if (typ.Equals("Einstellungsdaten", StringComparison.OrdinalIgnoreCase)) return FormularFeldTyp.Einstellungsdaten;
            if (typ.Equals("Dokumentdaten", StringComparison.OrdinalIgnoreCase)) return FormularFeldTyp.Dokumentdaten;
            if (typ.Equals("Freitext", StringComparison.OrdinalIgnoreCase)) return FormularFeldTyp.Freitext;
            return FormularFeldTyp.Unbekannt;
        }

        private static FormularFeldAnteil ParseAnteil(string anteil)
        {
            // nativ: "netto"/"mwst", ALLES andere (auch leer) ist brutto
            if (anteil.Equals("netto", StringComparison.OrdinalIgnoreCase)) return FormularFeldAnteil.Netto;
            if (anteil.Equals("mwst", StringComparison.OrdinalIgnoreCase)) return FormularFeldAnteil.Mwst;
            return FormularFeldAnteil.Brutto;
        }

        private static bool IstJa(string attr, string sollwert)
            => attr != null && attr.Equals(sollwert, StringComparison.OrdinalIgnoreCase);

        // =================================================================
        // Designer-Unterstuetzung: Mutations-API + Speichern (Phase 6).
        //
        // Gespeichert wird NICHT ueber XDocument.Save, sondern ueber einen
        // eigenen Serialisierer, der das Ausgabeformat des nativen
        // XMLite-Writers (ECTBridge/XMLite.cpp, GetXML) byte-genau
        // reproduziert: Attribute mit NACHGESTELLTEM Leerzeichen
        // (name="wert" ), dadurch Leerzeichen vor '>' bzw. '/>',
        // Deklaration mit ' ?>', keine eigene Einrueckung (die Whitespace-
        // Textknoten der Datei bleiben durch PreserveWhitespace erhalten).
        // So bleiben unveraenderte Dateien beim Roundtrip identisch und
        // der native Leser (XMLite, auch im ELSTER-Umfeld) liest die
        // Dateien unveraendert.
        // =================================================================

        private const string EinzugFeld = "\r\n\t\t";        // Ebene <feld>/<abschnitt>
        private const string EinzugFeldKind = "\r\n\t\t\t";  // Ebene <name>/<erweiterung>
        private const string EinzugSektion = "\r\n\t";       // Ebene <felder>/<abschnitte>

        /// <summary>Setzt den Feldtyp konsistent (Roh-Text + Enum).</summary>
        public static void SetzeTyp(FormularFeldDef feld, string typRoh)
        {
            feld.TypRoh = typRoh ?? "";
            feld.Typ = ParseTyp(feld.TypRoh);
        }

        private static string AnteilText(FormularFeldAnteil anteil)
        {
            switch (anteil)
            {
                case FormularFeldAnteil.Netto: return "netto";
                case FormularFeldAnteil.Mwst: return "mwst";
                default: return "brutto";
            }
        }

        /// <summary>
        /// Uebertraegt die Objektwerte eines Felds zurueck in sein XElement.
        /// Vorhandene Attribute werden nur bei geaendertem Wert angefasst;
        /// fehlende Attribute werden nur ergaenzt, wenn der Wert vom
        /// Parse-Default abweicht (minimale Datei-Diffs).
        /// </summary>
        public void UebernehmeFeld(FormularFeldDef feld)
        {
            var e = feld?.Quelle;
            if (e == null) return;

            string inv(int n) => n.ToString(System.Globalization.CultureInfo.InvariantCulture);
            SetzeAttribut(e, "id", inv(feld.Id), null);
            SetzeAttribut(e, "typ", feld.TypRoh, null);
            SetzeAttribut(e, "seite", inv(feld.Seite), null);
            SetzeAttribut(e, "horizontal", inv(feld.Horizontal), null);
            SetzeAttribut(e, "vertikal", inv(feld.Vertikal), null);
            // Parse-Defaults: fehlend = rechtsbuendig / brutto / mit / nein / nein
            SetzeAttribut(e, "ausrichtung",
                feld.RechtsBuendig ? "rechtsbuendig" : "linksbuendig",
                fehlenOkWert: "rechtsbuendig");
            SetzeAttribut(e, "anteil", AnteilText(feld.Anteil), fehlenOkWert: "brutto");
            SetzeAttribut(e, "nachkommaanteil",
                feld.NachkommaanteilOhne ? "ohne" : "mit", fehlenOkWert: "mit");
            SetzeAttribut(e, "nullwertdarstellung",
                feld.NullwertAnzeigen ? "ja" : "nein", fehlenOkWert: "nein");
            SetzeAttribut(e, "veraltet",
                feld.Veraltet ? "ja" : "nein", fehlenOkWert: "nein");
            SetzeAttribut(e, "elsterfeldname", feld.Elsterfeldname ?? "", fehlenOkWert: "");

            SetzeKindElement(e, "name", feld.Name ?? "");
            SetzeKindElement(e, "erweiterung", feld.Erweiterung ?? "");

            feld.HatInhalt = HatKnoten(e);
        }

        /// <summary>Legt ein neues Feld an (XElement im XMLite-Stil +
        /// Objektmodell) und liefert es zurueck.</summary>
        public FormularFeldDef NeuesFeld(int id)
        {
            var felderNode = Dokument.Root.Element("felder");
            if (felderNode == null)
            {
                felderNode = new XElement("felder");
                FuegeSektionEin(felderNode);
            }

            var e = new XElement("feld",
                new XText(EinzugFeldKind), new XElement("name", "Neues Feld"),
                new XText(EinzugFeldKind), new XElement("erweiterung"),
                new XText(EinzugFeld));
            FuegeElementEin(felderNode, e, EinzugFeld);

            var feld = ParseFeld(e);
            feld.Id = id;
            feld.Seite = 1;
            feld.Horizontal = 500;
            feld.Vertikal = 700;
            SetzeTyp(feld, "Summe");
            UebernehmeFeld(feld);
            _felder.Add(feld);
            return feld;
        }

        /// <summary>Entfernt ein Feld aus XML und Objektmodell.</summary>
        public void LoescheFeld(FormularFeldDef feld)
        {
            if (feld?.Quelle == null) return;
            EntferneMitEinzug(feld.Quelle);
            _felder.Remove(feld);
        }

        /// <summary>Uebertraegt die Objektwerte eines Abschnitts ins XElement.</summary>
        public void UebernehmeAbschnitt(FormularAbschnittDef a)
        {
            var e = a?.Quelle;
            if (e == null) return;
            SetzeAttribut(e, "name", a.Name ?? "", null);
            SetzeAttribut(e, "seite",
                a.Seite.ToString(System.Globalization.CultureInfo.InvariantCulture), null);
            SetzeAttribut(e, "vertikal",
                a.Vertikal.ToString(System.Globalization.CultureInfo.InvariantCulture), null);
        }

        /// <summary>Legt einen neuen Abschnitt an.</summary>
        public FormularAbschnittDef NeuerAbschnitt(string name, int seite, int vertikal)
        {
            var node = Dokument.Root.Element("abschnitte");
            if (node == null)
            {
                node = new XElement("abschnitte");
                FuegeSektionEin(node);
            }

            var e = new XElement("abschnitt");
            FuegeElementEin(node, e, EinzugFeld);

            var a = new FormularAbschnittDef
            {
                Name = name ?? "",
                Seite = Math.Max(1, seite),
                Vertikal = vertikal,
                Quelle = e
            };
            UebernehmeAbschnitt(a);
            _abschnitte.Add(a);
            return a;
        }

        /// <summary>Entfernt einen Abschnitt aus XML und Objektmodell.</summary>
        public void LoescheAbschnitt(FormularAbschnittDef a)
        {
            if (a?.Quelle == null) return;
            EntferneMitEinzug(a.Quelle);
            _abschnitte.Remove(a);
        }

        /// <summary>Speichert zurueck in die Quelldatei (XMLite-Format).</summary>
        public void Speichere() => SpeichereAls(Quellpfad);

        public void SpeichereAls(string pfad)
        {
            var sb = new StringBuilder(64 * 1024);

            var decl = Dokument.Declaration;
            if (decl != null)
            {
                // XMLite-PI-Stil: jedes Attribut mit nachgestelltem
                // Leerzeichen, dann "?>" -> '<?xml version="1.0" ... ?>'
                sb.Append("<?xml version=\"").Append(
                    string.IsNullOrEmpty(decl.Version) ? "1.0" : decl.Version)
                  .Append("\" ");
                if (!string.IsNullOrEmpty(decl.Encoding))
                    sb.Append("encoding=\"").Append(decl.Encoding).Append("\" ");
                sb.Append("?>");
            }

            foreach (var node in Dokument.Nodes())
                SchreibeKnoten(sb, node);

            Encoding enc;
            if (decl != null && !string.IsNullOrEmpty(decl.Encoding)
                && decl.Encoding.Equals("utf-8", StringComparison.OrdinalIgnoreCase))
                enc = new UTF8Encoding(false);
            else
                enc = Encoding.GetEncoding(1252);   // Standard der .ecf-Welt

            File.WriteAllText(pfad, sb.ToString(), enc);
        }

        // -----------------------------------------------------------------
        // interne Schreib-Helfer
        // -----------------------------------------------------------------

        private static void SchreibeKnoten(StringBuilder sb, XNode node)
        {
            switch (node)
            {
                case XElement e:
                    sb.Append('<').Append(e.Name.LocalName);
                    bool hatAttr = false;
                    foreach (var a in e.Attributes())
                    {
                        if (!hatAttr) { sb.Append(' '); hatAttr = true; }
                        sb.Append(a.Name.LocalName).Append("=\"")
                          .Append(EscapeAttribut(a.Value)).Append("\" ");
                    }
                    if (e.IsEmpty)
                    {
                        sb.Append("/>");
                    }
                    else
                    {
                        sb.Append('>');
                        foreach (var kind in e.Nodes())
                            SchreibeKnoten(sb, kind);
                        sb.Append("</").Append(e.Name.LocalName).Append('>');
                    }
                    break;

                case XText t:   // inkl. der erhaltenen Whitespace-Knoten
                    sb.Append(EscapeText(t.Value));
                    break;

                case XComment c:
                    sb.Append("<!--").Append(c.Value).Append("-->");
                    break;
            }
        }

        private static string EscapeText(string s)
        {
            // Der XML-Parser normalisiert Zeilenenden in Textknoten zu LF
            // (XML-Spez.) -- die .ecf-Welt ist aber CRLF, also
            // zurueckwandeln (erst vereinheitlichen, dann CRLF).
            return (s ?? "")
                .Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;")
                .Replace("\r\n", "\n").Replace("\n", "\r\n");
        }

        private static string EscapeAttribut(string s)
            => EscapeText(s).Replace("\"", "&quot;");

        /// <summary>Attribut nur bei Aenderung anfassen; fehlende Attribute
        /// nur ergaenzen, wenn der Wert vom "Fehlen-Default" abweicht
        /// (fehlenOkWert = null: immer ergaenzen).</summary>
        private static void SetzeAttribut(XElement e, string name, string wert,
            string fehlenOkWert)
        {
            var attr = e.Attribute(name);
            if (attr != null)
            {
                if (attr.Value != wert) attr.Value = wert;
                return;
            }
            if (fehlenOkWert != null
                && string.Equals(wert, fehlenOkWert, StringComparison.OrdinalIgnoreCase))
                return;
            e.Add(new XAttribute(name, wert));
        }

        /// <summary>Kindelement (name/erweiterung) setzen; leere
        /// selbstschliessende Elemente bleiben unangetastet, solange der
        /// Wert leer bleibt.</summary>
        private static void SetzeKindElement(XElement e, string name, string wert)
        {
            var kind = e.Element(name);
            if (kind == null)
            {
                // fehlendes Kind nur anlegen, wenn ein Wert zu speichern ist
                if (string.IsNullOrEmpty(wert)) return;
                e.Add(new XText(EinzugFeldKind),
                    new XElement(name, wert), new XText(EinzugFeld));
                return;
            }
            if (kind.Value == wert) return;
            if (wert.Length == 0)
            {
                // leeren: selbstschliessend halten (<erweiterung/>)
                kind.RemoveNodes();
                return;
            }
            kind.Value = wert;
        }

        private static bool HatKnoten(XElement e)
        {
            foreach (var _ in e.Nodes()) return true;
            return false;
        }

        /// <summary>Haengt eine neue Sektion (felder/abschnitte) im
        /// XMLite-Stil vor das schliessende Root-Tag.</summary>
        private void FuegeSektionEin(XElement sektion)
        {
            var root = Dokument.Root;
            // vor dem letzten Whitespace-Knoten (vor </formular>) einfuegen
            if (root.LastNode is XText schluss
                && schluss.Value.Trim().Length == 0)
                schluss.AddBeforeSelf(new XText(EinzugSektion), sektion);
            else
                root.Add(new XText(EinzugSektion), sektion, new XText("\r\n"));
        }

        /// <summary>Fuegt ein Element mit Einzug am Ende einer Sektion ein
        /// (vor dem schliessenden Whitespace der Sektion).</summary>
        private static void FuegeElementEin(XElement sektion, XElement element,
            string einzug)
        {
            if (sektion.LastNode is XText schluss
                && schluss.Value.Trim().Length == 0)
                schluss.AddBeforeSelf(new XText(einzug), element);
            else
                sektion.Add(new XText(einzug), element, new XText(EinzugSektion));
        }

        /// <summary>Entfernt ein Element samt vorangehendem Whitespace-
        /// Einzug (sonst blieben Leerzeilen zurueck).</summary>
        private static void EntferneMitEinzug(XElement e)
        {
            if (e.PreviousNode is XText davor && davor.Value.Trim().Length == 0)
                davor.Remove();
            e.Remove();
        }

        /// <summary>atoi-Semantik: fuehrende Zahl parsen, Rest ignorieren,
        /// leer/kaputt = 0 (wie die nativen atoi-Aufrufe auf Attributen).
        /// Public, weil auch der FormularRechner und die Tests sie nutzen.</summary>
        public static int Atoi(string s)
        {
            if (string.IsNullOrEmpty(s)) return 0;
            int i = 0, n = s.Length;
            while (i < n && char.IsWhiteSpace(s[i])) i++;
            int start = i;
            if (i < n && (s[i] == '-' || s[i] == '+')) i++;
            int endeVorzeichen = i;
            while (i < n && s[i] >= '0' && s[i] <= '9') i++;
            if (i == endeVorzeichen) return 0;   // keine Ziffern
            long wert = 0;
            for (int k = endeVorzeichen; k < i; k++)
            {
                wert = wert * 10 + (s[k] - '0');
                if (wert > int.MaxValue) { wert = int.MaxValue; break; }
            }
            return s[start] == '-' ? (int)-wert : (int)wert;
        }
    }
}
