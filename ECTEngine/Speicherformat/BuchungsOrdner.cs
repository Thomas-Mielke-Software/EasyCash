// BuchungsOrdner.cs -- Buchungsdokument als Ordner mit einer Datei je Buchung
//
// Zweite Hülle des XML-Formats (siehe BuchungsdateiXml.cs), gedacht als
// künftiges GoBD- und DSGVO-freundliches Speicherformat. Noch NICHT im UI.
//
//   <ordner>/
//     kopf/kopf-000001.xml          Dokumentkopf + Dauerbuchungen, Version 1
//     kopf/kopf-000002.xml          ... jede Änderung = neue Version
//     buchungen/<id>-000001.xml     eine Buchung, Version 1
//     buchungen/<id>-000002.xml     dieselbe Buchung, geändert
//     buchungen/<id>-000003.xml     Löschvermerk (Geloescht="1")
//
// Grundsätze:
//   * Eine einmal geschriebene Datei wird NIE verändert oder überschrieben
//     (Unveränderbarkeit, GoBD Rz. 58 ff.). Änderungen und Löschungen sind
//     neue Versionen; der alte Stand bleibt nachvollziehbar.
//   * Gespeichert wird nur, was sich geändert hat. Damit ist jeder
//     Speichervorgang eine Menge NEUER Dateien -- genau das, was GoBDify
//     hasht und per RFC3161-Zeitstempel beglaubigt (sha256sum-kompatibel).
//   * Jede Version nennt den SHA-256 ihrer Vorgängerversion (Attribut
//     Vorgaenger); Lücken und vertauschte Dateien fallen in Pruefe() auf.
//   * Je Buchung eine Datei erlaubt es später, personenbezogene Daten
//     einzelner Buchungen nach Ablauf der Aufbewahrungsfrist gezielt zu
//     löschen (DSGVO Art. 17), ohne den Rest anzufassen.
//
// Die Wurzeln:
//   <ECTBuchung Format="1" Version="2" Zeitpunkt="2026-11-24T10:00:00Z"
//               Vorgaenger="ab12...">  <Buchung .../>  </ECTBuchung>
//   <ECTBuchung Format="1" Version="3" ... Id="..." Geloescht="1"/>
//   <ECTDokumentkopf Format="1" Version="7" ...>
//     <Dokument .../> <Dauerbuchungen>...</Dauerbuchungen>
//   </ECTDokumentkopf>

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text.RegularExpressions;
using System.Xml.Linq;

namespace ECTEngine
{
    /// <summary>Was ein Speichervorgang geschrieben hat.</summary>
    public sealed class OrdnerSpeicherErgebnis
    {
        public int NeueBuchungen { get; internal set; }
        public int GeaenderteBuchungen { get; internal set; }
        public int GeloeschteBuchungen { get; internal set; }
        public bool KopfGeschrieben { get; internal set; }

        /// <summary>Neu angelegte Dateien, relativ zum Ordner.</summary>
        public List<string> Dateien { get; } = new List<string>();

        public bool NichtsGeaendert => Dateien.Count == 0;
    }

    public static class BuchungsOrdner
    {
        public const string KopfUnterordner = "kopf";
        public const string BuchungenUnterordner = "buchungen";

        private const string WurzelBuchung = "ECTBuchung";
        private const string WurzelKopf = "ECTDokumentkopf";

        private static readonly Regex BuchungsDatei =
            new Regex(@"^([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})-(\d{6})\.xml$",
                RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);
        private static readonly Regex KopfDatei =
            new Regex(@"^kopf-(\d{6})\.xml$", RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);

        /// <summary>Uhr für den Zeitpunkt-Vermerk (für Tests austauschbar).</summary>
        public static Func<DateTime> Uhr { get; set; } = () => DateTime.UtcNow;

        // -------------------------------------------------------------------
        // Laden
        // -------------------------------------------------------------------

        /// <summary>True, wenn der Ordner ein Buchungsordner ist (mindestens
        /// ein Dokumentkopf).</summary>
        public static bool IstBuchungsOrdner(string ordner)
            => Directory.Exists(Path.Combine(ordner ?? "", KopfUnterordner))
               && Versionen(Path.Combine(ordner, KopfUnterordner), KopfDatei).Any();

        /// <summary>Lädt den jüngsten Stand aller Buchungen und des Kopfs.</summary>
        /// <exception cref="FormatException">Kein Buchungsordner oder eine
        /// unlesbare jüngste Version.</exception>
        public static BuchungsDocument Lade(string ordner)
        {
            var kopfDatei = JuengsteKopfdatei(ordner)
                ?? throw new FormatException("Kein Buchungsordner (kein Dokumentkopf): " + ordner);
            var kopf = LeseWurzel(kopfDatei, WurzelKopf);

            var doc = new BuchungsDocument();
            BuchungsdateiXml.DokumentAusXml(
                kopf.Element("Dokument") ?? throw new FormatException(kopfDatei + ": <Dokument> fehlt."),
                doc);
            foreach (var d in kopf.Element("Dauerbuchungen")?.Elements("Dauerbuchung")
                              ?? Enumerable.Empty<XElement>())
                doc.Dauerbuchungen.Add(BuchungsdateiXml.DauerbuchungAusXml(d));

            foreach (var stand in JuengsteBuchungsstaende(ordner))
            {
                if (stand.Geloescht) continue;
                var b = BuchungsdateiXml.BuchungAusXml(stand.Inhalt);
                if (b.Uuid != stand.Id)
                    throw new FormatException(stand.Datei + ": Id im Inhalt passt nicht zum Dateinamen.");
                doc.Buchungen.Add(b);
            }
            doc.Sort();
            return doc;
        }

        // -------------------------------------------------------------------
        // Speichern
        // -------------------------------------------------------------------

        /// <summary>
        /// Schreibt die Änderungen gegenüber dem jüngsten Stand im Ordner als
        /// neue Versionen. Bestehende Dateien werden nie angefasst. Legt den
        /// Ordner an, falls nötig.
        /// </summary>
        public static OrdnerSpeicherErgebnis Speichere(BuchungsDocument doc, string ordner)
        {
            if (doc == null) throw new ArgumentNullException(nameof(doc));
            if (string.IsNullOrEmpty(ordner)) throw new ArgumentNullException(nameof(ordner));

            // Vorab prüfen -- ein abgebrochener Speichervorgang soll keine
            // halbe Versionsrunde hinterlassen
            var doppelt = doc.Buchungen.GroupBy(b => b.Uuid).FirstOrDefault(g => g.Count() > 1);
            if (doppelt != null)
                throw new InvalidOperationException("Buchungs-Id doppelt im Dokument: " + doppelt.Key);

            var ergebnis = new OrdnerSpeicherErgebnis();
            string kopfOrdner = Path.Combine(ordner, KopfUnterordner);
            string buchOrdner = Path.Combine(ordner, BuchungenUnterordner);
            Directory.CreateDirectory(kopfOrdner);
            Directory.CreateDirectory(buchOrdner);

            string zeitpunkt = Uhr().ToUniversalTime().ToString("yyyy-MM-dd'T'HH:mm:ss'Z'",
                CultureInfo.InvariantCulture);

            // ---- Buchungen ----
            var bisher = JuengsteBuchungsstaende(ordner).ToDictionary(s => s.Id);
            var aktuell = new HashSet<Guid>();
            foreach (var b in doc.Buchungen)
            {
                aktuell.Add(b.Uuid);
                var inhalt = BuchungsdateiXml.BuchungZuXml(b);
                bisher.TryGetValue(b.Uuid, out var alt);
                if (alt != null && !alt.Geloescht
                    && BuchungsdateiXml.ZuText(alt.Inhalt) == BuchungsdateiXml.ZuText(inhalt))
                    continue;   // unverändert

                int version = alt == null ? 1 : alt.Version + 1;
                var wurzel = Huelle(WurzelBuchung, version, zeitpunkt, alt?.Datei);
                wurzel.Add(inhalt);
                ergebnis.Dateien.Add(SchreibeNeu(ordner, BuchungenUnterordner,
                    BuchungsDateiname(b.Uuid, version), wurzel));
                if (alt == null || alt.Geloescht) ergebnis.NeueBuchungen++;
                else ergebnis.GeaenderteBuchungen++;
            }

            // Verschwundene Buchungen: Löschvermerk
            foreach (var alt in bisher.Values.Where(s => !s.Geloescht && !aktuell.Contains(s.Id)))
            {
                var wurzel = Huelle(WurzelBuchung, alt.Version + 1, zeitpunkt, alt.Datei);
                wurzel.Add(new XAttribute("Id", alt.Id.ToString("D")),
                           new XAttribute("Geloescht", 1));
                ergebnis.Dateien.Add(SchreibeNeu(ordner, BuchungenUnterordner,
                    BuchungsDateiname(alt.Id, alt.Version + 1), wurzel));
                ergebnis.GeloeschteBuchungen++;
            }

            // ---- Kopf (nur wenn sich etwas geändert hat) ----
            var kopfInhalt = new[]
            {
                BuchungsdateiXml.DokumentZuXml(doc),
                new XElement("Dauerbuchungen", doc.Dauerbuchungen.Select(BuchungsdateiXml.DauerbuchungZuXml))
            };
            string altKopf = JuengsteKopfdatei(ordner);
            int kopfVersion = 1;
            bool kopfGleich = false;
            if (altKopf != null)
            {
                var k = LeseWurzel(altKopf, WurzelKopf);
                kopfVersion = BuchungsdateiXml.Int(k, "Version", 0) + 1;
                kopfGleich = InhaltText(k.Elements()) == InhaltText(kopfInhalt);
            }
            if (!kopfGleich)
            {
                var wurzel = Huelle(WurzelKopf, kopfVersion, zeitpunkt, altKopf);
                wurzel.Add(kopfInhalt);
                ergebnis.Dateien.Add(SchreibeNeu(ordner, KopfUnterordner,
                    $"kopf-{kopfVersion:D6}.xml", wurzel));
                ergebnis.KopfGeschrieben = true;
            }
            return ergebnis;
        }

        // -------------------------------------------------------------------
        // Prüfen
        // -------------------------------------------------------------------

        /// <summary>
        /// Prüft die Versionsketten: lückenlose Nummerierung ab 1, Vorgänger-
        /// Hash jeder Version stimmt, Inhalt lesbar und zur Datei passend.
        /// Liefert die gefundenen Probleme (leer = in Ordnung).
        /// </summary>
        public static List<string> Pruefe(string ordner)
        {
            var probleme = new List<string>();
            PruefeKette(probleme, Path.Combine(ordner, KopfUnterordner),
                Versionen(Path.Combine(ordner, KopfUnterordner), KopfDatei)
                    .Select(v => ("kopf", v.Version, v.Datei)), WurzelKopf);

            var buchungen = Versionen(Path.Combine(ordner, BuchungenUnterordner), BuchungsDatei)
                .GroupBy(v => v.Schluessel, StringComparer.OrdinalIgnoreCase);
            foreach (var g in buchungen)
                PruefeKette(probleme, Path.Combine(ordner, BuchungenUnterordner),
                    g.Select(v => (g.Key, v.Version, v.Datei)), WurzelBuchung);
            return probleme;
        }

        private static void PruefeKette(List<string> probleme, string verzeichnis,
            IEnumerable<(string Schluessel, int Version, string Datei)> kette, string wurzelName)
        {
            string vorgaenger = null;
            int erwartet = 1;
            foreach (var v in kette.OrderBy(x => x.Version))
            {
                string name = Path.GetFileName(v.Datei);
                if (v.Version != erwartet)
                    probleme.Add($"{name}: Version {erwartet} fehlt.");
                erwartet = v.Version + 1;
                try
                {
                    var w = LeseWurzel(v.Datei, wurzelName);
                    if (BuchungsdateiXml.Int(w, "Version", -1) != v.Version)
                        probleme.Add($"{name}: Versionsnummer im Inhalt passt nicht zum Dateinamen.");
                    string soll = vorgaenger == null ? null : Sha256(vorgaenger);
                    string ist = (string)w.Attribute("Vorgaenger");
                    if (!string.Equals(soll, ist, StringComparison.OrdinalIgnoreCase))
                        probleme.Add($"{name}: Vorgänger-Hash stimmt nicht.");
                }
                catch (Exception ex) when (ex is FormatException || ex is IOException)
                {
                    probleme.Add($"{name}: {ex.Message}");
                }
                vorgaenger = v.Datei;
            }
        }

        // -------------------------------------------------------------------
        // Hilfen
        // -------------------------------------------------------------------

        private sealed class Buchungsstand
        {
            public Guid Id;
            public int Version;
            public string Datei;
            public bool Geloescht;
            public XElement Inhalt;   // <Buchung>, null bei Löschvermerk
        }

        private static IEnumerable<Buchungsstand> JuengsteBuchungsstaende(string ordner)
        {
            string verz = Path.Combine(ordner, BuchungenUnterordner);
            foreach (var g in Versionen(verz, BuchungsDatei)
                         .GroupBy(v => v.Schluessel, StringComparer.OrdinalIgnoreCase))
            {
                var juengste = g.OrderByDescending(v => v.Version).First();
                var w = LeseWurzel(juengste.Datei, WurzelBuchung);
                bool geloescht = BuchungsdateiXml.Int(w, "Geloescht", 0) != 0;
                var inhalt = w.Element("Buchung");
                if (!geloescht && inhalt == null)
                    throw new FormatException(juengste.Datei + ": <Buchung> fehlt.");
                yield return new Buchungsstand
                {
                    Id = Guid.Parse(g.Key),
                    Version = juengste.Version,
                    Datei = juengste.Datei,
                    Geloescht = geloescht,
                    Inhalt = geloescht ? null : inhalt
                };
            }
        }

        private static string JuengsteKopfdatei(string ordner)
            => Versionen(Path.Combine(ordner ?? "", KopfUnterordner), KopfDatei)
                .OrderByDescending(v => v.Version).Select(v => v.Datei).FirstOrDefault();

        private static IEnumerable<(string Schluessel, int Version, string Datei)> Versionen(
            string verzeichnis, Regex muster)
        {
            if (!Directory.Exists(verzeichnis)) yield break;
            foreach (var datei in Directory.GetFiles(verzeichnis, "*.xml"))
            {
                var m = muster.Match(Path.GetFileName(datei));
                if (!m.Success) continue;
                string schluessel = m.Groups.Count > 2 ? m.Groups[1].Value.ToLowerInvariant() : "kopf";
                string nummer = m.Groups[m.Groups.Count - 1].Value;
                yield return (schluessel, int.Parse(nummer, CultureInfo.InvariantCulture), datei);
            }
        }

        private static string BuchungsDateiname(Guid id, int version)
            => $"{id:D}-{version:D6}.xml";

        private static XElement Huelle(string name, int version, string zeitpunkt, string vorgaengerDatei)
        {
            var e = new XElement(name,
                new XAttribute("Format", BuchungsdateiXml.Format),
                new XAttribute("Version", version),
                new XAttribute("Zeitpunkt", zeitpunkt));
            if (vorgaengerDatei != null)
                e.Add(new XAttribute("Vorgaenger", Sha256(vorgaengerDatei)));
            return e;
        }

        /// <summary>Schreibt über eine Temp-Datei und benennt dann um --
        /// File.Move scheitert, wenn das Ziel schon existiert. Eine bereits
        /// vorhandene Version kann so nicht überschrieben werden.</summary>
        private static string SchreibeNeu(string ordner, string unterordner, string name, XElement wurzel)
        {
            string ziel = Path.Combine(ordner, unterordner, name);
            if (File.Exists(ziel))
                throw new IOException("Version existiert bereits und wird nicht überschrieben: " + ziel);
            string temp = ziel + ".tmp";
            File.WriteAllBytes(temp, BuchungsdateiXml.ZuBytes(wurzel));
            try { File.Move(temp, ziel); }
            catch { try { File.Delete(temp); } catch { } throw; }
            return Path.Combine(unterordner, name);
        }

        private static XElement LeseWurzel(string datei, string erwartet)
        {
            XElement w;
            try { w = XElement.Load(datei); }
            catch (System.Xml.XmlException ex)
            {
                throw new FormatException(Path.GetFileName(datei) + ": kein gültiges XML: " + ex.Message, ex);
            }
            if (w.Name.LocalName != erwartet)
                throw new FormatException(Path.GetFileName(datei) + ": Wurzelelement " + erwartet + " erwartet.");
            BuchungsdateiXml.PruefeFormat(w);
            return w;
        }

        private static string InhaltText(IEnumerable<XElement> elemente)
            => string.Concat(elemente.Select(e => BuchungsdateiXml.ZuText(new XElement(e))));

        private static string Sha256(string datei)
        {
            using (var sha = SHA256.Create())
            using (var fs = File.OpenRead(datei))
                return BitConverter.ToString(sha.ComputeHash(fs)).Replace("-", "").ToLowerInvariant();
        }
    }
}
