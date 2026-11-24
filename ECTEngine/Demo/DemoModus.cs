// DemoModus.cs -- Beispieldaten für den Demo-Modus (Programmstart mit /demo)
//
// Liefert die beiden eingebetteten Ressourcen in einer Form, die der native
// Teil nur noch ablegen bzw. laden muss:
//
//   * Ressourcen/Demo/easyct.ini    Einstellungen + Kontenrahmen der Persona
//                                   (Platzhalter {JAHR}/{VORJAHR})
//   * Ressourcen/Demo/Demodaten.xml Buchungsdokument als Sammeldatei im
//                                   XML-Format (BuchungsdateiXml), geschrieben
//                                   für ein festes Jahr und beim Laden auf das
//                                   Demojahr verschoben
//
// Das Demojahr ist der älteste gemeinsame Stand der installierten
// Formulare (EÜR, USt-Voranmeldung, USt-Erklärung): Für dieses Jahr gibt es
// jedes Formular, die Formular-Ansichten zeigen also alle etwas.
//
// Sicherheit: Diese Klasse schreibt selbst NICHTS. Wohin die Daten kommen
// (ein frisches Temp-Verzeichnis, nie das Datenverzeichnis des Nutzers),
// entscheidet ECTBridge/DemoModus.cpp.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace ECTEngine
{
    public static class DemoModus
    {
        /// <summary>
        /// True, solange das Programm im Demo-Modus läuft (gesetzt von
        /// ECTBridge/DemoModus.cpp). Managed Schreibwege, die außerhalb des
        /// Demo-Verzeichnisses landen würden -- z.B. der Formular-Designer,
        /// der direkt in die .ecf im Programmverzeichnis speichert --, fragen
        /// das hier ab.
        /// </summary>
        public static bool Aktiv { get; set; }

        private const string IniRessource = "ECTEngine.Ressourcen.Demo.easyct.ini";
        private const string DatenRessource = "ECTEngine.Ressourcen.Demo.Demodaten.xml";

        /// <summary>Formulare, deren gemeinsamer Stand das Demojahr bestimmt
        /// (Formularname = Attribut "name" der .ecf).</summary>
        public static readonly IReadOnlyList<string> JahresFormulare = new[]
        {
            "E/Ü-Rechnung", "Umsatzsteuer-Voranmeldung", "Umsatzsteuererklärung"
        };

        // -------------------------------------------------------------------
        // Demojahr
        // -------------------------------------------------------------------

        /// <summary>
        /// Demojahr aus den .ecf-Dateien eines Verzeichnisses (in der Regel
        /// das Programmverzeichnis).
        /// </summary>
        public static int ErmittleJahr(string formularVerzeichnis)
        {
            string[] dateien;
            try { dateien = Directory.GetFiles(formularVerzeichnis ?? "", "*.ecf"); }
            catch { dateien = new string[0]; }
            return ErmittleJahr(dateien);
        }

        /// <summary>
        /// Demojahr aus einer Liste von .ecf-Dateien: je Formular aus
        /// <see cref="JahresFormulare"/> das neueste Jahr, davon das kleinste.
        /// Fehlt eines der Formulare ganz, zählen die übrigen; gibt es gar
        /// keines, ist es das Vorjahr.
        /// </summary>
        public static int ErmittleJahr(IEnumerable<string> ecfDateien)
        {
            var neuestes = new Dictionary<string, int>(StringComparer.Ordinal);
            foreach (var datei in ecfDateien ?? Enumerable.Empty<string>())
            {
                var (name, jahr) = LeseFormularKopf(datei);
                if (jahr == 0 || !JahresFormulare.Contains(name)) continue;
                if (!neuestes.TryGetValue(name, out int bisher) || jahr > bisher)
                    neuestes[name] = jahr;
            }
            return neuestes.Count > 0 ? neuestes.Values.Min() : DateTime.Today.Year - 1;
        }

        /// <summary>Name und Jahr eines Formulars (nur das Wurzelelement).
        /// Die Erklärungsformulare bis 2024 heißen intern ebenfalls
        /// "Umsatzsteuer-Voranmeldung" -- echte Voranmeldungen erkennt man wie
        /// in UstAbgleich.Lade am Anzeigenamen.</summary>
        private static (string Name, int Jahr) LeseFormularKopf(string datei)
        {
            if (!UstAbgleich.LiesFormularkopf(datei, out string name, out string anzeige))
                return ("", 0);
            if (name == UstAbgleich.FormularUva
                && !(anzeige ?? "").StartsWith(UstAbgleich.FormularUva + " ", StringComparison.Ordinal))
                return ("", 0);
            int jahr = ErstesJahr(anzeige);
            if (jahr == 0) jahr = ErstesJahr(Path.GetFileNameWithoutExtension(datei));
            return (name, jahr);
        }

        private static int ErstesJahr(string text)
        {
            var m = Regex.Match(text ?? "", @"(?<!\d)(19|20)\d\d(?!\d)");
            return m.Success ? int.Parse(m.Value, CultureInfo.InvariantCulture) : 0;
        }

        // -------------------------------------------------------------------
        // Einstellungen
        // -------------------------------------------------------------------

        /// <summary>Inhalt der easyct.ini für das Demojahr.</summary>
        public static string ErzeugeIni(int jahr)
            => LeseRessource(IniRessource)
                .Replace("{VORJAHR}", (jahr - 1).ToString(CultureInfo.InvariantCulture))
                .Replace("{JAHR}", jahr.ToString(CultureInfo.InvariantCulture));

        /// <summary>
        /// Schreibt die easyct.ini für das Demojahr in Windows-1252 (wie die
        /// nativen GetPrivateProfile*-Leser sie erwarten). Die Datei darf noch
        /// nicht existieren -- der Demo-Modus legt sie immer in einem frischen
        /// Verzeichnis an, eine vorhandene ini würde nie überschrieben.
        /// </summary>
        public static void SchreibeIni(string pfad, int jahr)
        {
            var bytes = Encoding.GetEncoding(1252).GetBytes(
                ErzeugeIni(jahr).Replace("\r\n", "\n").Replace("\n", "\r\n"));
            using (var fs = new FileStream(pfad, FileMode.CreateNew, FileAccess.Write))
                fs.Write(bytes, 0, bytes.Length);
        }

        /// <summary>
        /// ini-Text in die Schlüssel des Einstellungs-Caches übersetzen -- genau
        /// wie ECT_LadeEinstellungen (EinstellungenExports.cpp, KuerzelFuerCache):
        /// Finanzamt/Einnahmen-/AusgabenRechnungsposten als Kurzform ("fname",
        /// "e00", "a00"), alle anderen Sektionen als "[Sektion]Key".
        /// </summary>
        public static Dictionary<string, string> IniZuCache(string iniText)
        {
            var cache = new Dictionary<string, string>(StringComparer.Ordinal);
            string sektion = null;
            foreach (var rohzeile in (iniText ?? "").Split('\n'))
            {
                string zeile = rohzeile.TrimEnd('\r');
                string getrimmt = zeile.Trim();
                if (getrimmt.Length == 0 || getrimmt[0] == ';') continue;
                if (getrimmt[0] == '[' && getrimmt.EndsWith("]"))
                {
                    sektion = getrimmt.Substring(1, getrimmt.Length - 2);
                    continue;
                }
                int gleich = zeile.IndexOf('=');
                if (sektion == null || gleich <= 0) continue;
                string key = zeile.Substring(0, gleich).Trim();
                string wert = zeile.Substring(gleich + 1);   // Tabs am Ende gehören dazu
                string praefix = sektion == "Finanzamt" ? "f"
                               : sektion == "EinnahmenRechnungsposten" ? "e"
                               : sektion == "AusgabenRechnungsposten" ? "a" : null;
                cache[praefix != null ? praefix + key : "[" + sektion + "]" + key] = wert;
            }
            return cache;
        }

        // -------------------------------------------------------------------
        // Buchungsdokument
        // -------------------------------------------------------------------

        /// <summary>Die Beispieldaten, verschoben auf das Demojahr.</summary>
        public static BuchungsDocument ErzeugeDokument(int jahr)
        {
            var doc = BuchungsdateiXml.LeseSammeldatei(LeseRessource(DatenRessource));
            VerschiebeJahr(doc, jahr);
            return doc;
        }

        /// <summary>Jahr, für das die eingebettete Sammeldatei geschrieben ist.</summary>
        public static int JahrDerBeispieldaten()
            => BuchungsdateiXml.LeseSammeldatei(LeseRessource(DatenRessource)).Jahr;

        /// <summary>
        /// Verschiebt ein Dokument samt aller Datumsangaben um ganze Jahre:
        /// Buchungen, Dauerbuchungen und die jahresbezogenen Schlüssel der
        /// Dokument-Erweiterungen (z.B. "UST-Zahlbetrag-JJJJ-NN" der
        /// USt-Vorauszahlungen). Der 29. Februar wird in Nicht-Schaltjahren
        /// zum 28.
        /// </summary>
        public static void VerschiebeJahr(BuchungsDocument doc, int neuesJahr)
        {
            if (doc == null) throw new ArgumentNullException(nameof(doc));
            int delta = neuesJahr - doc.Jahr;
            if (delta == 0) return;

            int altesJahr = doc.Jahr;
            foreach (var b in doc.Buchungen)
            {
                b.Datum = Verschiebe(b.Datum, delta);
                // Jahreszahlen in Texten ("USt-Vorauszahlung 4. Quartal 2024",
                // Beleg "E-17/2024" einer fortgeführten AfA, "Miete 01/2025")
                b.Beschreibung = VerschiebeJahreszahlen(b.Beschreibung, altesJahr, delta);
                b.Belegnummer = VerschiebeJahreszahlen(b.Belegnummer, altesJahr, delta);
                foreach (var e in b.Erweiterungen.Alle().ToList())
                    if (e.Ns == "EasyCash" && e.Key.StartsWith("Vorlage", StringComparison.Ordinal))
                        b.Erweiterungen.Setze(e.Ns, e.Key, VerschiebeJahreszahlen(e.Wert, altesJahr, delta));
            }

            foreach (var d in doc.Dauerbuchungen)
            {
                d.VonDatum = Verschiebe(d.VonDatum, delta);
                d.BisDatum = Verschiebe(d.BisDatum, delta);
                d.AusgefuehrtBis = Verschiebe(d.AusgefuehrtBis, delta);
            }

            if (doc.BackupNachfrageTermin != default(DateTime))
                doc.BackupNachfrageTermin = Verschiebe(doc.BackupNachfrageTermin, delta);

            string alt = doc.Jahr.ToString(CultureInfo.InvariantCulture);
            string neu = neuesJahr.ToString(CultureInfo.InvariantCulture);
            var erw = new ErweiterungStore();
            foreach (var e in doc.Erweiterungen.Alle())
                erw.Setze(e.Ns, Regex.Replace(e.Key, @"(?<!\d)" + alt + @"(?!\d)", neu), e.Wert);
            doc.Erweiterungen = erw;

            doc.Jahr = neuesJahr;
        }

        /// <summary>Verschiebt freistehende Jahreszahlen aus dem Umfeld des
        /// Buchungsjahrs (zehn Jahre davor bis eins danach) -- andere
        /// vierstellige Zahlen bleiben, wie sie sind.</summary>
        private static string VerschiebeJahreszahlen(string text, int jahr, int delta)
        {
            if (string.IsNullOrEmpty(text)) return text;
            return Regex.Replace(text, @"(?<!\d)\d{4}(?!\d)", m =>
            {
                int j = int.Parse(m.Value, CultureInfo.InvariantCulture);
                return j >= jahr - 10 && j <= jahr + 1
                    ? (j + delta).ToString(CultureInfo.InvariantCulture)
                    : m.Value;
            });
        }

        private static DateTime Verschiebe(DateTime d, int jahre)
        {
            if (d == default(DateTime) || d.Year >= 9000) return d;   // "nie"/"unbegrenzt"
            int jahr = d.Year + jahre;
            if (jahr < 1 || jahr > 9999) return d;
            int tag = Math.Min(d.Day, DateTime.DaysInMonth(jahr, d.Month));
            return new DateTime(jahr, d.Month, tag) + d.TimeOfDay;
        }

        // -------------------------------------------------------------------

        private static string LeseRessource(string name)
        {
            var asm = typeof(DemoModus).Assembly;
            using (var s = asm.GetManifestResourceStream(name)
                   ?? throw new InvalidOperationException("Ressource fehlt: " + name))
            using (var r = new StreamReader(s, new UTF8Encoding(false), true))
                return r.ReadToEnd();
        }
    }
}
