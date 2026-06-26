// EUKonten.cs -- Datenmodell + Laden für die E/Ü-Konten-Seite (M2).
//
// Ein E/Ü-Konto (Einnahmen-/Ausgaben-Rechnungsposten) wird über die
// "Feldzuweisung" mit je einem Feld pro Formular (EÜR, USt-Voranmeldung,
// E/Ü-Rechnung, ...) verknüpft. Die Formulare sind .ecf-Dateien (XML) im
// Programmverzeichnis; die Zuweisungen liegen als "ECT"-Erweiterungs-Blob
// in den ini-Sektionen [EinnahmenFeldzuweisungen]/[AusgabenFeldzuweisungen]
// (über den globalen Einstellungs-Cache erreichbar).
//
// Pro Formular-Name kann es mehrere .ecf geben (verschiedene Jahre). Sie
// werden zu einer FormularGruppe zusammengefasst; die Varianten sind nach
// Jahr absteigend sortiert (neueste zuerst = Default).

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace ECTEngine
{
    /// <summary>Ein zuweisbares Formularfeld (nur typ=Einnahmen/Ausgaben).</summary>
    public sealed class Formularfeld
    {
        public string Id          { get; }
        public string Bezeichnung { get; }
        public bool   IstEinnahme { get; }   // true=Einnahmen-Feld, false=Ausgaben-Feld

        public Formularfeld(string id, string bezeichnung, bool istEinnahme)
        {
            Id          = id ?? "";
            Bezeichnung = bezeichnung ?? "";
            IstEinnahme = istEinnahme;
        }
    }

    /// <summary>Eine konkrete Formular-Variante (eine .ecf-Datei).</summary>
    public sealed class Formular
    {
        /// <summary>Interner Name (Schlüssel der Feldzuweisung, z.B. "E/Ü-Rechnung").</summary>
        public string Name        { get; }
        public string Anzeigename { get; }
        /// <summary>Anzeigename gekürzt bis einschließlich der Jahreszahl
        /// (z.B. "Umsatzsteuer-Voranmeldung 2026 Juli (D)" -> "...Voranmeldung 2026").</summary>
        public string AnzeigenameKurz { get; }
        /// <summary>Jahr (aus Anzeigename/Dateiname), 0 wenn unbekannt -- für die Sortierung.</summary>
        public int    Jahr        { get; }
        public IReadOnlyList<Formularfeld> Felder { get; }

        public Formular(string name, string anzeigename, int jahr, IReadOnlyList<Formularfeld> felder)
        {
            Name            = name ?? "";
            Anzeigename     = string.IsNullOrEmpty(anzeigename) ? Name : anzeigename;
            AnzeigenameKurz = KuerzeNachJahr(Anzeigename);
            Jahr            = jahr;
            Felder          = felder ?? new List<Formularfeld>();
        }

        /// <summary>Schneidet alles nach der ersten 4-stelligen Zahl ab
        /// (Monat/Länderkürzel entfernen).</summary>
        private static string KuerzeNachJahr(string text)
        {
            if (string.IsNullOrEmpty(text)) return text;
            for (int i = 0; i + 4 <= text.Length; i++)
            {
                if (char.IsDigit(text[i]) && char.IsDigit(text[i + 1])
                    && char.IsDigit(text[i + 2]) && char.IsDigit(text[i + 3]))
                {
                    // nicht mitten in einer längeren Zahl abschneiden
                    bool davorZiffer  = i > 0 && char.IsDigit(text[i - 1]);
                    bool danachZiffer = i + 4 < text.Length && char.IsDigit(text[i + 4]);
                    if (!davorZiffer && !danachZiffer)
                        return text.Substring(0, i + 4).TrimEnd();
                }
            }
            return text;
        }
    }

    /// <summary>Alle Varianten (Jahre) eines Formulars, neueste zuerst.</summary>
    public sealed class FormularGruppe
    {
        public string Name { get; }
        public IReadOnlyList<Formular> Varianten { get; }
        /// <summary>Neueste Variante (= Default-Auswahl).</summary>
        public Formular Neueste => Varianten.Count > 0 ? Varianten[0] : null;

        public FormularGruppe(string name, IReadOnlyList<Formular> varianten)
        {
            Name      = name ?? "";
            Varianten = varianten ?? new List<Formular>();
        }
    }

    /// <summary>
    /// Ein E/Ü-Konto (Einnahmen- oder Ausgaben-Rechnungsposten) mit seinen
    /// Formularfeld-Zuweisungen (pro Formularname genau eine Feld-Id).
    /// Veränderbar (Drag&amp;Drop-Sortierung + -Zuweisung in der UI).
    /// </summary>
    public sealed class EUKonto
    {
        public string Name        { get; set; }
        public bool   IstEinnahme { get; }
        /// <summary>Aktueller Slot-Index (0-99); wird beim Umsortieren neu gesetzt.</summary>
        public int    Index       { get; set; }
        /// <summary>Unterkategorie (parallele ini-Sektion). Steuert die
        /// Einrückung/Strukturierung in der EÜR; in der KontenPage editierbar
        /// und wird beim Umsortieren mitgeführt.</summary>
        public string Unterkategorie { get; set; }
        /// <summary>Formularname -> Feld-Id.</summary>
        public Dictionary<string, string> Feldzuweisungen { get; }

        public EUKonto(string name, bool istEinnahme, int index, string unterkategorie,
            Dictionary<string, string> feldzuweisungen)
        {
            Name            = name ?? "";
            IstEinnahme     = istEinnahme;
            Index           = index;
            Unterkategorie  = unterkategorie ?? "";
            Feldzuweisungen = feldzuweisungen ?? new Dictionary<string, string>();
        }
    }

    /// <summary>Lädt und hält die E/Ü-Konten + Formular-Gruppen.</summary>
    public static class EUKonten
    {
        private static List<FormularGruppe> _gruppen        = new List<FormularGruppe>();
        private static List<EUKonto>        _einnahmenKonten = new List<EUKonto>();
        private static List<EUKonto>        _ausgabenKonten  = new List<EUKonto>();

        public static IReadOnlyList<FormularGruppe> FormularGruppen => _gruppen;
        public static IReadOnlyList<EUKonto>        EinnahmenKonten  => _einnahmenKonten;
        public static IReadOnlyList<EUKonto>        AusgabenKonten   => _ausgabenKonten;

        private static bool _formulareGeladen;

        public static void Lade()
        {
            // Die .ecf-Formulare im Programmverzeichnis ändern sich zur Laufzeit
            // nicht -- nur EINMAL pro Prozess von der Platte lesen und parsen
            // (das sind dutzende XML-Dateien; jedes erneute Öffnen/Aktualisieren
            // der Konten-Seite würde sonst alles neu parsen). Die Konten selbst
            // kommen aus dem (mandantenabhängigen) Cache und werden stets frisch
            // aufgebaut.
            if (!_formulareGeladen)
            {
                _gruppen        = LadeFormularGruppen();
                _formulareGeladen = true;
            }
            _einnahmenKonten = LadeKonten(einnahmen: true);
            _ausgabenKonten  = LadeKonten(einnahmen: false);
        }

        /// <summary>
        /// Schreibt die Feldzuweisungen eines Kontos als "ECT"-Erweiterungs-Blob
        /// zurück in den Cache/die ini (Sektion [EinnahmenFeldzuweisungen]/
        /// [AusgabenFeldzuweisungen], Index = Slot des Kontos). Bracket-Form ->
        /// Cache und ini bleiben konsistent.
        /// </summary>
        public static void SpeichereFeldzuweisungen(EUKonto konto)
        {
            if (konto == null) return;
            var store = new ErweiterungStore();
            foreach (var kv in konto.Feldzuweisungen)
                if (!string.IsNullOrEmpty(kv.Value))
                    store.Setze("ECT", kv.Key, kv.Value);

            string sektion = konto.IstEinnahme
                ? "[EinnahmenFeldzuweisungen]"
                : "[AusgabenFeldzuweisungen]";
            Einstellungen.Speichere(
                sektion + konto.Index.ToString("D2", CultureInfo.InvariantCulture),
                store.ZuPipeFormat());
        }

        /// <summary>
        /// Schreibt die Unterkategorie EINES Kontos zurück in Cache/ini
        /// (Sektion [EinnahmenUnterkategorien]/[AusgabenUnterkategorien], Index =
        /// Slot des Kontos). Bracket-Form -> Cache und ini bleiben konsistent.
        /// Die Unterkategorie steuert die Einrückung/Strukturierung der Konten
        /// in der Einnahmen-/Überschussrechnung.
        /// </summary>
        public static void SpeichereUnterkategorie(EUKonto konto)
        {
            if (konto == null) return;
            string sektion = konto.IstEinnahme
                ? "[EinnahmenUnterkategorien]"
                : "[AusgabenUnterkategorien]";
            Einstellungen.Speichere(
                sektion + konto.Index.ToString("D2", CultureInfo.InvariantCulture),
                konto.Unterkategorie ?? "");
        }

        /// <summary>
        /// Schreibt die Konten-Reihenfolge beider Gruppen zurück: Name
        /// (e/a-Präfix -> [EinnahmenRechnungsposten]/[AusgabenRechnungsposten]),
        /// Feldzuweisungen und Unterkategorien werden PARALLEL am neuen Index
        /// abgelegt. Aktualisiert dabei <see cref="EUKonto.Index"/>.
        /// </summary>
        public static void SpeichereReihenfolge(IList<EUKonto> einnahmen, IList<EUKonto> ausgaben)
        {
            var sw = System.Diagnostics.Stopwatch.StartNew();
            int n = SchreibeGruppe(einnahmen, "e",
                        "EinnahmenRechnungsposten", "EinnahmenFeldzuweisungen", "EinnahmenUnterkategorien")
                  + SchreibeGruppe(ausgaben, "a",
                        "AusgabenRechnungsposten", "AusgabenFeldzuweisungen", "AusgabenUnterkategorien");
            Einstellungen.NeuAufbauenKonten();   // nur die Kontenlisten (günstig)
            sw.Stop();
            PerfLog($"SpeichereReihenfolge: {n} Konten, 6 Sektions-Schreibvorgaenge in {sw.ElapsedMilliseconds} ms "
                  + $"(Thread {System.Threading.Thread.CurrentThread.ManagedThreadId})");
        }

        /// <summary>Hängt eine Zeile an ect_perf.log im Programmverzeichnis an
        /// (Zeitmessung für die Performance-Analyse).</summary>
        public static void PerfLog(string msg)
        {
            try
            {
                string p = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "ect_perf.log");
                File.AppendAllText(p, DateTime.Now.ToString("HH:mm:ss.fff") + "  " + msg + Environment.NewLine);
            }
            catch { }
        }

        private static int SchreibeGruppe(IList<EUKonto> liste, string namePrefix,
            string rechSektion, string fzSektion, string ukSektion)
        {
            if (liste == null) return 0;

            // Pro Sektion EINEN Block aufbauen (iniKey -> Wert) und den Cache
            // (kanonische Keys) im selben Zug aktualisieren.
            var rech = new Dictionary<string, string>();
            var fz   = new Dictionary<string, string>();
            var uk   = new Dictionary<string, string>();

            for (int i = 0; i < liste.Count; i++)
            {
                var k  = liste[i];
                string nn = i.ToString("D2", CultureInfo.InvariantCulture);

                var store = new ErweiterungStore();
                foreach (var kv in k.Feldzuweisungen)
                    if (!string.IsNullOrEmpty(kv.Value))
                        store.Setze("ECT", kv.Key, kv.Value);
                string blob = store.ZuPipeFormat();
                string ukWert = k.Unterkategorie ?? "";

                rech[nn] = k.Name;
                fz[nn]   = blob;
                uk[nn]   = ukWert;

                // Cache: Namen via Praefix ("e00"), Rest via Bracket-Form.
                Einstellungen.SetzeCacheNur(namePrefix + nn, k.Name);
                Einstellungen.SetzeCacheNur("[" + fzSektion + "]" + nn, blob);
                Einstellungen.SetzeCacheNur("[" + ukSektion + "]" + nn, ukWert);

                k.Index = i;   // Slot folgt der neuen Reihenfolge
            }

            // Drei gebündelte Sektions-Schreibvorgaenge (ini).
            Einstellungen.SchreibeSektion(rechSektion, rech);
            Einstellungen.SchreibeSektion(fzSektion, fz);
            Einstellungen.SchreibeSektion(ukSektion, uk);

            return liste.Count;
        }

        // -----------------------------------------------------------------
        // Formulare aus den .ecf-Dateien im Programmverzeichnis
        // -----------------------------------------------------------------
        private static List<FormularGruppe> LadeFormularGruppen()
        {
            var alle = new List<Formular>();
            string dir = AppDomain.CurrentDomain.BaseDirectory;

            string[] dateien;
            try { dateien = Directory.GetFiles(dir, "*.ecf"); }
            catch { return new List<FormularGruppe>(); }

            foreach (var datei in dateien)
            {
                try
                {
                    var doc  = XDocument.Load(datei);
                    var root = doc.Root;
                    if (root == null || root.Name.LocalName != "formular") continue;

                    string name = (string)root.Attribute("name") ?? "";
                    if (string.IsNullOrEmpty(name)) continue;
                    string anzeige = (string)root.Attribute("anzeigename") ?? name;

                    var felder     = new List<Formularfeld>();
                    var felderNode = root.Element("felder");
                    if (felderNode != null)
                    {
                        foreach (var feld in felderNode.Elements("feld"))
                        {
                            string typ = (string)feld.Attribute("typ") ?? "";
                            bool einnahme = typ.Equals("Einnahmen", StringComparison.OrdinalIgnoreCase);
                            bool ausgabe  = typ.Equals("Ausgaben",  StringComparison.OrdinalIgnoreCase);
                            if (!einnahme && !ausgabe) continue;

                            string veraltet = (string)feld.Attribute("veraltet") ?? "";
                            if (veraltet.Equals("ja", StringComparison.OrdinalIgnoreCase)) continue;

                            string id = (string)feld.Attribute("id") ?? "";
                            if (string.IsNullOrEmpty(id)) continue;

                            string bez = (string)feld.Element("name") ?? "";
                            felder.Add(new Formularfeld(id, bez, einnahme));
                        }
                    }
                    if (felder.Count == 0) continue;

                    int jahr = JahrAus(anzeige);
                    if (jahr == 0) jahr = JahrAus(Path.GetFileNameWithoutExtension(datei));
                    alle.Add(new Formular(name, anzeige, jahr, felder));
                }
                catch { /* defekte/fremde .ecf überspringen */ }
            }

            // Nach Name gruppieren; je Jahr nur EINE Variante (Monats-/Zeitraum-
            // Varianten der USt-Voranmeldung haben dieselben Felder), nach Jahr
            // absteigend.
            return alle
                .GroupBy(f => f.Name)
                .Select(g => new FormularGruppe(
                    g.Key,
                    g.GroupBy(f => f.Jahr)
                     .Select(jg => jg.First())   // exemplarisch ein Formular je Jahr
                     .OrderByDescending(f => f.Jahr)
                     .ToList()))
                .OrderBy(gr => gr.Name, StringComparer.OrdinalIgnoreCase)
                .ToList();
        }

        /// <summary>Sucht die erste 4-stellige Jahreszahl (1990-2099) im Text.</summary>
        private static int JahrAus(string text)
        {
            if (string.IsNullOrEmpty(text)) return 0;
            int best = 0;
            for (int i = 0; i + 4 <= text.Length; i++)
            {
                if (char.IsDigit(text[i]) && char.IsDigit(text[i + 1])
                    && char.IsDigit(text[i + 2]) && char.IsDigit(text[i + 3]))
                {
                    int j = (text[i] - '0') * 1000 + (text[i + 1] - '0') * 100
                          + (text[i + 2] - '0') * 10 + (text[i + 3] - '0');
                    if (j >= 1990 && j <= 2099 && j > best) best = j;
                }
            }
            return best;
        }

        // -----------------------------------------------------------------
        // Konten + Feldzuweisungen aus dem Einstellungs-Cache
        // -----------------------------------------------------------------
        private static List<EUKonto> LadeKonten(bool einnahmen)
        {
            var namen = einnahmen
                ? Einstellungen.EinnahmenKonten
                : Einstellungen.AusgabenKonten;
            string fzSektion = einnahmen ? "[EinnahmenFeldzuweisungen]" : "[AusgabenFeldzuweisungen]";
            string ukSektion = einnahmen ? "[EinnahmenUnterkategorien]"  : "[AusgabenUnterkategorien]";

            var liste = new List<EUKonto>();
            for (int i = 0; i < namen.Count; i++)
            {
                string nn    = i.ToString("D2", CultureInfo.InvariantCulture);
                string blob  = Einstellungen.Hole(fzSektion + nn);
                var    store = ErweiterungStore.AusPipeFormat(blob);

                var zuw = new Dictionary<string, string>();
                foreach (var eintrag in store.Alle())
                    if (eintrag.Ns == "ECT" && !string.IsNullOrEmpty(eintrag.Wert))
                        zuw[eintrag.Key] = eintrag.Wert;

                string uk = Einstellungen.Hole(ukSektion + nn);
                liste.Add(new EUKonto(namen[i], einnahmen, i, uk, zuw));
            }
            return liste;
        }
    }
}
