// BuchungsdateiXml.cs -- XML-Speicherformat für Buchungsdokumente
//
// Parallel zum MFC-CArchive-Format der .eca-Dateien. Noch NICHT im UI
// erreichbar; genutzt wird es vorerst für die Beispieldaten des
// Demo-Modus (eingebettete Sammeldatei) und von BuchungsOrdner.
//
// Ziel ist ein Format, das ohne C++/MFC auskommt, menschenlesbar ist und
// sich Buchung für Buchung ablegen, versionieren, hashen und mit einem
// RFC3161-Zeitstempel beglaubigen lässt (GoBDify). Darum gibt es ZWEI
// Hüllen um dieselben Elemente:
//
//   * die Sammeldatei: ein Dokument komplett in EINER Datei
//       <ECTBuchungsdatei Format="1">
//         <Dokument .../>
//         <Dauerbuchungen> <Dauerbuchung .../> ... </Dauerbuchungen>
//         <Buchungen> <Buchung .../> ... </Buchungen>
//       </ECTBuchungsdatei>
//
//   * der Buchungsordner (BuchungsOrdner.cs): je Buchung eine eigene,
//     nach dem Schreiben nie mehr veränderte Datei.
//
// Die Elemente:
//
//   <Buchung Id="9f3c..." Art="Ausgabe" Datum="2025-03-14" Brutto="119.00"
//            MwSt="19" Konto="..." Beleg="..." Betrieb="..." Bestandskonto="...">
//     <Beschreibung>Druckerei Stempel und Söhne</Beschreibung>
//     <AfA Jahre="3" Nr="1" Restwert="1200.00" Degressiv="0" Satz="0"
//          Genauigkeit="EntsprechendEinstellungen"/>       (nur wenn nötig)
//     <Erweiterung Ns="EasyCash" Schluessel="Buchungsgruppe">...</Erweiterung>
//   </Buchung>
//
//   <Dauerbuchung Art="Ausgabe" Brutto="..." MwSt="..." Intervall="Monatlich"
//                 Von="..." Bis="..." Tag="1" AusgefuehrtBis="..." Konto="..."
//                 Beleg="..." Betrieb="..." Bestandskonto="...">
//     <Beschreibung>...</Beschreibung> <Erweiterung .../>
//   </Dauerbuchung>
//
//   <Dokument Jahr="2025" Waehrung="EUR" ... > <Erweiterung .../> </Dokument>
//
// Regeln, damit die Ausgabe BYTEGENAU reproduzierbar ist (Voraussetzung
// für Hashes, die sich nur bei echten Änderungen ändern):
//   * Zahlen und Datumsangaben kulturunabhängig (Punkt als
//     Dezimaltrenner, ISO-Datum),
//   * Attribute in fester Reihenfolge, leere Text-Attribute entfallen,
//   * Erweiterungen sortiert nach Namensraum und Schlüssel,
//   * UTF-8 ohne BOM, Zeilenende LF.
//
// Die Uuid einer Buchung ist in diesem Format, anders als in .eca,
// DAUERHAFT: sie ist die Identität der Buchung über alle Versionen.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Linq;

namespace ECTEngine
{
    public static class BuchungsdateiXml
    {
        /// <summary>Version des Formats. Leser lehnen höhere Versionen ab.</summary>
        public const int Format = 1;

        public const string WurzelSammeldatei = "ECTBuchungsdatei";

        private static readonly CultureInfo Inv = CultureInfo.InvariantCulture;

        // -------------------------------------------------------------------
        // Sammeldatei
        // -------------------------------------------------------------------

        /// <summary>Das ganze Dokument als Sammeldatei (XML-Text).</summary>
        public static string SchreibeSammeldatei(BuchungsDocument doc)
        {
            if (doc == null) throw new ArgumentNullException(nameof(doc));
            var wurzel = new XElement(WurzelSammeldatei,
                new XAttribute("Format", Format),
                DokumentZuXml(doc),
                new XElement("Dauerbuchungen", doc.Dauerbuchungen.Select(DauerbuchungZuXml)),
                new XElement("Buchungen", SortiertFuerAusgabe(doc.Buchungen).Select(BuchungZuXml)));
            return ZuText(wurzel);
        }

        /// <summary>Liest eine Sammeldatei.</summary>
        /// <exception cref="FormatException">Kein gültiges Dokument oder
        /// eine neuere Formatversion.</exception>
        public static BuchungsDocument LeseSammeldatei(string xml)
        {
            XElement wurzel;
            try { wurzel = XElement.Parse(xml ?? "", LoadOptions.None); }
            catch (XmlException ex) { throw new FormatException("Kein gültiges XML: " + ex.Message, ex); }
            return LeseSammeldatei(wurzel);
        }

        public static BuchungsDocument LeseSammeldatei(XElement wurzel)
        {
            if (wurzel == null || wurzel.Name.LocalName != WurzelSammeldatei)
                throw new FormatException("Keine EasyCash&Tax-Buchungsdatei (Wurzelelement "
                    + WurzelSammeldatei + " erwartet).");
            PruefeFormat(wurzel);

            var doc = new BuchungsDocument();
            var kopf = wurzel.Element("Dokument")
                ?? throw new FormatException("Element <Dokument> fehlt.");
            DokumentAusXml(kopf, doc);

            foreach (var d in wurzel.Element("Dauerbuchungen")?.Elements("Dauerbuchung")
                              ?? Enumerable.Empty<XElement>())
                doc.Dauerbuchungen.Add(DauerbuchungAusXml(d));

            var ids = new HashSet<Guid>();
            foreach (var b in wurzel.Element("Buchungen")?.Elements("Buchung")
                              ?? Enumerable.Empty<XElement>())
            {
                var buchung = BuchungAusXml(b);
                if (!ids.Add(buchung.Uuid))
                    throw new FormatException("Buchungs-Id doppelt vergeben: " + buchung.Uuid);
                doc.Buchungen.Add(buchung);
            }
            doc.Sort();
            return doc;
        }

        /// <summary>Wirft, wenn das Element eine neuere Formatversion trägt.</summary>
        internal static void PruefeFormat(XElement e)
        {
            int f = Int(e, "Format", 1);
            if (f > Format)
                throw new FormatException($"Die Datei hat das Format {f}, dieses Programm kennt "
                    + $"nur bis {Format}. Bitte EasyCash&Tax aktualisieren.");
        }

        /// <summary>Stabile Reihenfolge für die Ausgabe: Datum, dann Id.</summary>
        private static IEnumerable<Buchung> SortiertFuerAusgabe(IEnumerable<Buchung> buchungen)
            => buchungen.OrderBy(b => b.Datum).ThenBy(b => b.Uuid);

        // -------------------------------------------------------------------
        // Buchung
        // -------------------------------------------------------------------

        public static XElement BuchungZuXml(Buchung b)
        {
            if (b == null) throw new ArgumentNullException(nameof(b));
            var e = new XElement("Buchung",
                new XAttribute("Id", b.Uuid.ToString("D")),
                new XAttribute("Art", b.Art == Buchungsart.Einnahme ? "Einnahme" : "Ausgabe"),
                new XAttribute("Datum", Datum(b.Datum)),
                new XAttribute("Brutto", Geld(b.BruttoBetrag.InCent)),
                new XAttribute("MwSt", Prozent(b.BruttoBetrag.MwstPromille)));
            TextAttr(e, "Konto", b.Konto);
            TextAttr(e, "Beleg", b.Belegnummer);
            TextAttr(e, "Betrieb", b.Betrieb);
            TextAttr(e, "Bestandskonto", b.Bestandskonto);

            e.Add(new XElement("Beschreibung", b.Beschreibung ?? ""));

            bool afaStandard = b.AfaJahre == 1 && b.AfaNr == 1 && b.AfaRestwertCent == 0
                && !b.AfaDegressiv && b.AfaSatz == 0
                && b.AfaGenauigkeit == AfaGenauigkeit.EntsprechendEinstellungen;
            if (!afaStandard)
                e.Add(new XElement("AfA",
                    new XAttribute("Jahre", b.AfaJahre),
                    new XAttribute("Nr", b.AfaNr),
                    new XAttribute("Restwert", Geld(b.AfaRestwertCent)),
                    new XAttribute("Degressiv", b.AfaDegressiv ? 1 : 0),
                    new XAttribute("Satz", b.AfaSatz),
                    new XAttribute("Genauigkeit", b.AfaGenauigkeit.ToString())));

            ErweiterungenZuXml(e, b.Erweiterungen);
            return e;
        }

        public static Buchung BuchungAusXml(XElement e)
        {
            if (e == null || e.Name.LocalName != "Buchung")
                throw new FormatException("Element <Buchung> erwartet.");

            var b = new Buchung
            {
                Uuid = LeseId(e, "Id"),
                Art = Art(e),
                Datum = Datum(e, "Datum"),
                BruttoBetrag = Betrag.AusCent(GeldCent(e, "Brutto"), Promille(e, "MwSt")),
                Konto = Text(e, "Konto"),
                Belegnummer = Text(e, "Beleg"),
                Betrieb = Text(e, "Betrieb"),
                Bestandskonto = Text(e, "Bestandskonto"),
                Beschreibung = (string)e.Element("Beschreibung") ?? ""
            };

            var afa = e.Element("AfA");
            if (afa != null)
            {
                b.AfaJahre = Int(afa, "Jahre", 1);
                b.AfaNr = Int(afa, "Nr", 1);
                b.AfaRestwertCent = GeldCent(afa, "Restwert");
                b.AfaDegressiv = Int(afa, "Degressiv", 0) != 0;
                b.AfaSatz = Int(afa, "Satz", 0);
                b.AfaGenauigkeit = Enum<AfaGenauigkeit>(afa, "Genauigkeit",
                    AfaGenauigkeit.EntsprechendEinstellungen);
            }

            b.Erweiterungen = ErweiterungenAusXml(e);
            return b;
        }

        // -------------------------------------------------------------------
        // Dauerbuchung
        // -------------------------------------------------------------------

        public static XElement DauerbuchungZuXml(Dauerbuchung d)
        {
            if (d == null) throw new ArgumentNullException(nameof(d));
            var e = new XElement("Dauerbuchung",
                new XAttribute("Art", d.Art == Buchungsart.Einnahme ? "Einnahme" : "Ausgabe"),
                new XAttribute("Brutto", Geld(d.BruttoBetrag.InCent)),
                new XAttribute("MwSt", Prozent(d.BruttoBetrag.MwstPromille)),
                new XAttribute("Intervall", d.Intervall.ToString()),
                new XAttribute("Von", Datum(d.VonDatum)),
                new XAttribute("Bis", Datum(d.BisDatum)),
                new XAttribute("Tag", d.Buchungstag),
                new XAttribute("AusgefuehrtBis", Datum(d.AusgefuehrtBis)));
            TextAttr(e, "Konto", d.Konto);
            TextAttr(e, "Beleg", d.Belegnummer);
            TextAttr(e, "Betrieb", d.Betrieb);
            TextAttr(e, "Bestandskonto", d.Bestandskonto);
            e.Add(new XElement("Beschreibung", d.Beschreibung ?? ""));
            ErweiterungenZuXml(e, d.Erweiterungen);
            return e;
        }

        public static Dauerbuchung DauerbuchungAusXml(XElement e)
        {
            if (e == null || e.Name.LocalName != "Dauerbuchung")
                throw new FormatException("Element <Dauerbuchung> erwartet.");
            return new Dauerbuchung
            {
                Art = Art(e),
                BruttoBetrag = Betrag.AusCent(GeldCent(e, "Brutto"), Promille(e, "MwSt")),
                Intervall = Enum(e, "Intervall", Intervall.Monatlich),
                VonDatum = Datum(e, "Von"),
                BisDatum = Datum(e, "Bis"),
                Buchungstag = Int(e, "Tag", 1),
                AusgefuehrtBis = Datum(e, "AusgefuehrtBis"),
                Konto = Text(e, "Konto"),
                Belegnummer = Text(e, "Beleg"),
                Betrieb = Text(e, "Betrieb"),
                Bestandskonto = Text(e, "Bestandskonto"),
                Beschreibung = (string)e.Element("Beschreibung") ?? "",
                Erweiterungen = ErweiterungenAusXml(e)
            };
        }

        // -------------------------------------------------------------------
        // Dokumentkopf
        // -------------------------------------------------------------------

        public static XElement DokumentZuXml(BuchungsDocument doc)
        {
            var e = new XElement("Dokument",
                new XAttribute("Jahr", doc.Jahr),
                new XAttribute("Waehrung", doc.Waehrung ?? ""),
                new XAttribute("AfaGenauigkeit", doc.GlobaleAfaGenauigkeit.ToString()),
                new XAttribute("Buchungszaehler", doc.Buchungszaehler),
                new XAttribute("BelegnrEinnahmen", doc.LaufendeBelegnrEinnahmen),
                new XAttribute("BelegnrAusgaben", doc.LaufendeBelegnrAusgaben),
                new XAttribute("BelegnrBank", doc.LaufendeBelegnrBank),
                new XAttribute("BelegnrKasse", doc.LaufendeBelegnrKasse),
                new XAttribute("EcaVersion", doc.DokumentVersion),
                new XAttribute("BackupIntervallTage", doc.BackupNachfrageIntervallTage),
                new XAttribute("BackupTermin", Datum(doc.BackupNachfrageTermin)));
            TextAttr(e, "UrspruenglicheWaehrung", doc.UrspruenglicheWaehrung);
            ErweiterungenZuXml(e, doc.Erweiterungen);
            return e;
        }

        public static void DokumentAusXml(XElement e, BuchungsDocument doc)
        {
            doc.Jahr = Int(e, "Jahr", 0);
            if (doc.Jahr < 1900 || doc.Jahr > 2999)
                throw new FormatException("Ungültiges Buchungsjahr: " + (string)e.Attribute("Jahr"));
            doc.Waehrung = Text(e, "Waehrung");
            if (doc.Waehrung.Length == 0) doc.Waehrung = "EUR";
            doc.UrspruenglicheWaehrung = Text(e, "UrspruenglicheWaehrung");
            doc.GlobaleAfaGenauigkeit = Enum(e, "AfaGenauigkeit", AfaGenauigkeit.Monatsgenau);
            doc.Buchungszaehler = Int(e, "Buchungszaehler", 0);
            doc.LaufendeBelegnrEinnahmen = Int(e, "BelegnrEinnahmen", 1);
            doc.LaufendeBelegnrAusgaben = Int(e, "BelegnrAusgaben", 1);
            doc.LaufendeBelegnrBank = Int(e, "BelegnrBank", 1);
            doc.LaufendeBelegnrKasse = Int(e, "BelegnrKasse", 1);
            doc.DokumentVersion = Int(e, "EcaVersion", doc.DokumentVersion);
            doc.BackupNachfrageIntervallTage = Int(e, "BackupIntervallTage", 7);
            doc.BackupNachfrageTermin = e.Attribute("BackupTermin") != null
                ? Datum(e, "BackupTermin") : default(DateTime);
            doc.Erweiterungen = ErweiterungenAusXml(e);
        }

        // -------------------------------------------------------------------
        // Erweiterungen
        // -------------------------------------------------------------------

        private static void ErweiterungenZuXml(XElement e, ErweiterungStore store)
        {
            if (store == null) return;
            foreach (var x in store.Alle()
                         .OrderBy(a => a.Ns, StringComparer.Ordinal)
                         .ThenBy(a => a.Key, StringComparer.Ordinal))
                e.Add(new XElement("Erweiterung",
                    new XAttribute("Ns", x.Ns),
                    new XAttribute("Schluessel", x.Key),
                    x.Wert ?? ""));
        }

        private static ErweiterungStore ErweiterungenAusXml(XElement e)
        {
            var store = new ErweiterungStore();
            foreach (var x in e.Elements("Erweiterung"))
            {
                string ns = (string)x.Attribute("Ns") ?? "";
                string key = (string)x.Attribute("Schluessel") ?? "";
                if (ns.Length == 0 || key.Length == 0)
                    throw new FormatException("<Erweiterung> ohne Ns oder Schluessel.");
                store.Setze(ns, key, x.Value);
            }
            return store;
        }

        // -------------------------------------------------------------------
        // Text-Ausgabe
        // -------------------------------------------------------------------

        /// <summary>XML-Text in der kanonischen Form (UTF-8, LF, Einrückung
        /// mit zwei Leerzeichen, mit XML-Deklaration).</summary>
        public static string ZuText(XElement wurzel)
        {
            var einst = new XmlWriterSettings
            {
                Encoding = new UTF8Encoding(false),
                Indent = true,
                IndentChars = "  ",
                NewLineChars = "\n",
                NewLineHandling = NewLineHandling.Entitize,
                OmitXmlDeclaration = false
            };
            using (var ms = new MemoryStream())
            {
                using (var w = XmlWriter.Create(ms, einst))
                    new XDocument(wurzel).Save(w);
                return new UTF8Encoding(false).GetString(ms.ToArray()) + "\n";
            }
        }

        /// <summary>Bytes einer kanonischen Datei (UTF-8 ohne BOM).</summary>
        public static byte[] ZuBytes(XElement wurzel)
            => new UTF8Encoding(false).GetBytes(ZuText(wurzel));

        // -------------------------------------------------------------------
        // Werte schreiben
        // -------------------------------------------------------------------

        private static void TextAttr(XElement e, string name, string wert)
        {
            if (!string.IsNullOrEmpty(wert)) e.Add(new XAttribute(name, wert));
        }

        private static string Geld(long cent)
            => (cent / 100m).ToString("0.00", Inv);

        /// <summary>Promille-Rohwert (19000) als Prozentzahl ("19", "10.7").</summary>
        private static string Prozent(int promille)
            => (promille / 1000m).ToString("0.###", Inv);

        internal static string Datum(DateTime d)
            => d.TimeOfDay == TimeSpan.Zero
                ? d.ToString("yyyy-MM-dd", Inv)
                : d.ToString("yyyy-MM-dd'T'HH:mm:ss", Inv);

        // -------------------------------------------------------------------
        // Werte lesen
        // -------------------------------------------------------------------

        private static string Text(XElement e, string name) => (string)e.Attribute(name) ?? "";

        internal static int Int(XElement e, string name, int vorgabe)
        {
            var s = (string)e.Attribute(name);
            if (s == null) return vorgabe;
            if (!int.TryParse(s, NumberStyles.Integer, Inv, out int i))
                throw new FormatException($"{e.Name.LocalName}/@{name}: keine ganze Zahl: \"{s}\"");
            return i;
        }

        private static int GeldCent(XElement e, string name)
        {
            var s = (string)e.Attribute(name);
            if (s == null) return 0;
            if (!decimal.TryParse(s, NumberStyles.AllowLeadingSign | NumberStyles.AllowDecimalPoint,
                    Inv, out decimal d))
                throw new FormatException($"{e.Name.LocalName}/@{name}: kein Betrag: \"{s}\"");
            return (int)decimal.Round(d * 100m, 0, MidpointRounding.AwayFromZero);
        }

        private static int Promille(XElement e, string name)
        {
            var s = (string)e.Attribute(name);
            if (s == null) return 0;
            if (!decimal.TryParse(s, NumberStyles.AllowDecimalPoint, Inv, out decimal d))
                throw new FormatException($"{e.Name.LocalName}/@{name}: kein Steuersatz: \"{s}\"");
            return (int)decimal.Round(d * 1000m, 0, MidpointRounding.AwayFromZero);
        }

        private static DateTime Datum(XElement e, string name)
        {
            var s = (string)e.Attribute(name);
            if (s == null) throw new FormatException($"{e.Name.LocalName}/@{name} fehlt.");
            if (DateTime.TryParseExact(s, new[] { "yyyy-MM-dd", "yyyy-MM-dd'T'HH:mm:ss" },
                    Inv, DateTimeStyles.None, out DateTime d))
                return d;
            throw new FormatException($"{e.Name.LocalName}/@{name}: kein Datum: \"{s}\"");
        }

        private static Guid LeseId(XElement e, string name)
        {
            var s = (string)e.Attribute(name);
            if (s == null || !System.Guid.TryParse(s, out Guid g) || g == System.Guid.Empty)
                throw new FormatException($"{e.Name.LocalName}/@{name}: keine gültige Id: \"{s}\"");
            return g;
        }

        private static Buchungsart Art(XElement e)
        {
            var s = Text(e, "Art");
            if (s == "Einnahme") return Buchungsart.Einnahme;
            if (s == "Ausgabe") return Buchungsart.Ausgabe;
            throw new FormatException($"{e.Name.LocalName}/@Art: \"{s}\" (Einnahme/Ausgabe erwartet)");
        }

        private static T Enum<T>(XElement e, string name, T vorgabe) where T : struct
        {
            var s = (string)e.Attribute(name);
            if (s == null) return vorgabe;
            if (System.Enum.TryParse(s, false, out T wert) && System.Enum.IsDefined(typeof(T), wert))
                return wert;
            throw new FormatException($"{e.Name.LocalName}/@{name}: unbekannter Wert \"{s}\"");
        }
    }
}
