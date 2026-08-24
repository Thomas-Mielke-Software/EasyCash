// VorlagenBibliothek.cs -- mitgelieferte Buchungsvorlagen (Bibliothek)
//
// Ergaenzung zum Datei-Import: eine kuratierte, hierarchisch geordnete
// Sammlung von Buchungsvorlagen, die MIT dem Programm ausgeliefert wird.
// Quelle ist die eingebettete Ressource Ressourcen/Vorlagenbibliothek.xml.
//
// Warum eingebettet und nicht als Dateien im Programmverzeichnis: die
// Bibliothek gehoert zur Programmversion, nicht zum Datenbestand. Anders
// als die .ecf-Formulare ist sie nicht nach Steuerjahr versioniert, nicht
// laenderweise abwaehlbar und muss nicht einzeln in easycash.iss gepflegt
// werden. Eigene Vorlagen kommen weiterhin ueber den Datei-Import.
//
// Die Blaetter des Baums enthalten GENAU EIN <ECTBuchungsvorlage>-Element
// im Austauschformat von PresetXml -- die Bibliothek hat also keinen
// eigenen Vorlagen-Parser, sondern reicht das Element als String an
// PresetXml.Importiere weiter (derselbe Weg wie beim Datei-Import).

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Xml.Linq;

namespace ECTEngine
{
    /// <summary>Ein Blatt der Bibliothek: eine mitgelieferte Vorlage.</summary>
    public sealed class BibliothekEintrag
    {
        /// <summary>Anzeigetitel im Baum (ausfuehrlicher als der
        /// Vorlagen-Name, der spaeter im Preset-Slot landet).</summary>
        public string Titel { get; internal set; } = "";

        /// <summary>Kommagetrennte Suchbegriffe aus dem Katalog.</summary>
        public string Stichworte { get; internal set; } = "";

        /// <summary>Das komplette &lt;ECTBuchungsvorlage&gt;-Element als
        /// String -- Futter fuer PresetXml.Importiere.</summary>
        public string VorlagenXml { get; internal set; } = "";

        /// <summary>Name der Vorlage (landet als Preset-Text in der ini).</summary>
        public string Name { get; internal set; } = "";

        /// <summary>Erlaeuterung aus der Vorlage (wird mitdurchsucht).</summary>
        public string Notiz { get; internal set; } = "";

        /// <summary>True = Einnahme-Vorlage, false = Ausgabe.</summary>
        public bool IstEinnahme { get; internal set; }

        /// <summary>Anzahl der Zusatzzeilen (0 = einfache Vorlage,
        /// sonst Buchungsgruppe).</summary>
        public int ZeilenAnzahl { get; internal set; }

        /// <summary>Passt der Eintrag zu ALLEN Suchwoertern?</summary>
        public bool Passt(IReadOnlyList<string> woerter)
        {
            if (woerter == null || woerter.Count == 0) return true;
            string heu = VorlagenBibliothek.Normalisiere(
                Titel + " " + Stichworte + " " + Name + " " + Notiz);
            return woerter.All(w => heu.IndexOf(w, StringComparison.Ordinal) >= 0);
        }
    }

    /// <summary>Ein Knoten der Bibliothek: Unterabschnitte + Eintraege.</summary>
    public sealed class BibliothekAbschnitt
    {
        public string Titel { get; internal set; } = "";

        public List<BibliothekAbschnitt> Abschnitte { get; } = new List<BibliothekAbschnitt>();
        public List<BibliothekEintrag>   Eintraege   { get; } = new List<BibliothekEintrag>();

        /// <summary>Alle Eintraege dieses Teilbaums (Tiefensuche).</summary>
        public IEnumerable<BibliothekEintrag> AlleEintraege()
        {
            foreach (var e in Eintraege) yield return e;
            foreach (var a in Abschnitte)
                foreach (var e in a.AlleEintraege()) yield return e;
        }

        /// <summary>
        /// Gefilterte Kopie des Teilbaums; null wenn nichts passt. Trifft
        /// der Abschnittstitel selbst zu, bleibt sein Inhalt vollstaendig
        /// erhalten -- eine Suche nach "Reverse Charge" soll den ganzen
        /// Abschnitt zeigen, nicht nur die Eintraege mit dem Wort im Namen.
        /// </summary>
        public BibliothekAbschnitt Filtere(IReadOnlyList<string> woerter)
        {
            if (woerter == null || woerter.Count == 0) return this;

            string titelNorm = VorlagenBibliothek.Normalisiere(Titel);
            if (woerter.All(w => titelNorm.IndexOf(w, StringComparison.Ordinal) >= 0))
                return this;

            var kopie = new BibliothekAbschnitt { Titel = Titel };
            kopie.Eintraege.AddRange(Eintraege.Where(e => e.Passt(woerter)));
            foreach (var unter in Abschnitte)
            {
                var gefiltert = unter.Filtere(woerter);
                if (gefiltert != null) kopie.Abschnitte.Add(gefiltert);
            }
            return kopie.Eintraege.Count > 0 || kopie.Abschnitte.Count > 0 ? kopie : null;
        }
    }

    /// <summary>
    /// Zugriff auf die mitgelieferte Vorlagen-Bibliothek. Rein lesend;
    /// das Uebernehmen einer Vorlage in einen Preset-Slot macht die UI
    /// ueber den vorhandenen Import-Weg.
    /// </summary>
    public static class VorlagenBibliothek
    {
        private const string RessourcenEndung = "Vorlagenbibliothek.xml";

        private static List<BibliothekAbschnitt> _wurzeln;
        private static string _fehler = "";

        /// <summary>"" = ok; sonst der Grund, warum die Bibliothek leer ist
        /// (fehlende/kaputte Ressource). Erst nach Lade() aussagekraeftig.</summary>
        public static string Fehler => _fehler;

        /// <summary>Oberste Abschnitte der Bibliothek (gecacht).</summary>
        public static IReadOnlyList<BibliothekAbschnitt> Lade()
        {
            if (_wurzeln != null) return _wurzeln;
            _wurzeln = new List<BibliothekAbschnitt>();
            _fehler = "";

            string xml = LiesRessource();
            if (xml == null)
            {
                _fehler = "Die Vorlagen-Bibliothek konnte nicht geladen werden "
                        + "(Ressource \"" + RessourcenEndung + "\" fehlt).";
                return _wurzeln;
            }

            try
            {
                var doc = XDocument.Parse(xml);
                var root = doc.Root;
                if (root == null || root.Name.LocalName != "ECTVorlagenBibliothek")
                {
                    _fehler = "Die Vorlagen-Bibliothek hat ein unerwartetes Format.";
                    return _wurzeln;
                }
                foreach (var a in root.Elements("Abschnitt"))
                    _wurzeln.Add(LiesAbschnitt(a));
            }
            catch (Exception ex)
            {
                _wurzeln.Clear();
                _fehler = "Die Vorlagen-Bibliothek ist fehlerhaft: " + ex.Message;
            }
            return _wurzeln;
        }

        /// <summary>
        /// Gefilterte Sicht auf die Bibliothek. Leerer Suchtext = alles.
        /// Mehrere Woerter werden UND-verknuepft.
        /// </summary>
        public static IReadOnlyList<BibliothekAbschnitt> Suche(string suchtext)
        {
            var alle = Lade();
            var woerter = ZerlegeSuche(suchtext);
            if (woerter.Count == 0) return alle;

            var treffer = new List<BibliothekAbschnitt>();
            foreach (var a in alle)
            {
                var gefiltert = a.Filtere(woerter);
                if (gefiltert != null) treffer.Add(gefiltert);
            }
            return treffer;
        }

        /// <summary>Zerlegt den Suchtext in normalisierte Woerter.</summary>
        public static IReadOnlyList<string> ZerlegeSuche(string suchtext)
            => Normalisiere(suchtext)
                .Split(new[] { ' ', ',', ';' }, StringSplitOptions.RemoveEmptyEntries)
                .ToList();

        /// <summary>
        /// Vergleichsform fuer die Suche: Kleinschreibung, Umlaute und 'ß'
        /// aufgeloest. Damit findet "gemass" auch "gemäß", und die Eingabe
        /// muss nicht die Schreibweise des Katalogs treffen.
        /// </summary>
        public static string Normalisiere(string s)
        {
            if (string.IsNullOrEmpty(s)) return "";
            var sb = new StringBuilder(s.Length);
            foreach (char c in s.ToLowerInvariant())
            {
                switch (c)
                {
                    case 'ä': sb.Append("ae"); break;
                    case 'ö': sb.Append("oe"); break;
                    case 'ü': sb.Append("ue"); break;
                    case 'ß': sb.Append("ss"); break;
                    default:  sb.Append(c);    break;
                }
            }
            return sb.ToString();
        }

        // ------------------------------------------------------------------
        // Intern
        // ------------------------------------------------------------------

        private static BibliothekAbschnitt LiesAbschnitt(XElement el)
        {
            var abschnitt = new BibliothekAbschnitt
            {
                Titel = ((string)el.Attribute("Titel") ?? "").Trim()
            };

            foreach (var kind in el.Elements())
            {
                if (kind.Name.LocalName == "Abschnitt")
                    abschnitt.Abschnitte.Add(LiesAbschnitt(kind));
                else if (kind.Name.LocalName == "Eintrag")
                {
                    var eintrag = LiesEintrag(kind);
                    if (eintrag != null) abschnitt.Eintraege.Add(eintrag);
                }
            }
            return abschnitt;
        }

        private static BibliothekEintrag LiesEintrag(XElement el)
        {
            var vorlage = el.Element("ECTBuchungsvorlage");
            if (vorlage == null) return null;   // Eintrag ohne Vorlage: ignorieren

            var zeilen = vorlage.Element("Zeilen");
            return new BibliothekEintrag
            {
                Titel        = ((string)el.Attribute("Titel") ?? "").Trim(),
                Stichworte   = ((string)el.Attribute("Stichworte") ?? "").Trim(),
                VorlagenXml  = vorlage.ToString(),
                Name         = ((string)vorlage.Element("Name") ?? "").Trim(),
                Notiz        = ((string)vorlage.Element("Notiz") ?? "").Trim(),
                IstEinnahme  = string.Equals(
                    ((string)vorlage.Element("Typ") ?? "").Trim(), "E",
                    StringComparison.OrdinalIgnoreCase),
                ZeilenAnzahl = zeilen == null ? 0 : zeilen.Elements("Zeile").Count()
            };
        }

        /// <summary>Ressourcen-Name defensiv per Endung suchen -- so bleibt
        /// die Klasse gegen Umbenennungen des Standard-Namensraums robust.</summary>
        private static string LiesRessource()
        {
            var asm = typeof(VorlagenBibliothek).Assembly;
            string name = asm.GetManifestResourceNames()
                .FirstOrDefault(n => n.EndsWith(RessourcenEndung, StringComparison.OrdinalIgnoreCase));
            if (name == null) return null;

            using (var strom = asm.GetManifestResourceStream(name))
            {
                if (strom == null) return null;
                using (var leser = new StreamReader(strom, Encoding.UTF8))
                    return leser.ReadToEnd();
            }
        }
    }
}
