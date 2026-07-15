using System;
using System.Collections.Generic;
using System.Globalization;

namespace ECTEngine
{
    /// <summary>Ein Betrieb aus der ini-Sektion [Betriebe].</summary>
    public sealed class Betrieb
    {
        public string Name            { get; }
        public string Unternehmensart { get; }  // kann Tabs enthalten, nicht auflösen
        public int    Icon            { get; }

        public Betrieb(string name, string unternehmensart, int icon)
        {
            Name            = name            ?? "";
            Unternehmensart = unternehmensart ?? "";
            Icon            = icon;
        }
    }

    /// <summary>Ein Bestandskonto aus der ini-Sektion [Bestandskonten].</summary>
    public sealed class Bestandskonto
    {
        public string                            Name  { get; }
        public int                               Icon  { get; }
        /// <summary>Anfangssalden nach Buchungsjahr (leer wenn kein Eintrag in ini).</summary>
        public IReadOnlyDictionary<int, decimal> Saldo { get; }

        public Bestandskonto(string name, int icon, Dictionary<int, decimal> saldo)
        {
            Name  = name ?? "";
            Icon  = icon;
            Saldo = saldo ?? new Dictionary<int, decimal>();
        }
    }

    /// <summary>
    /// Eine Zusatz-Zeile einer Buchungsgruppen-Vorlage (mehrzeiliges Preset).
    /// Alle Felder sind ROHE Strings aus der ini -- Formeln/Templates werden
    /// erst beim Buchen ausgewertet (FormelParser). ini-Keys pro Slot NN und
    /// Zeile k: NNZ&lt;k&gt;Art, NNZ&lt;k&gt;Konto, NNZ&lt;k&gt;MWSt,
    /// NNZ&lt;k&gt;Betrag, NNZ&lt;k&gt;Text, NNZ&lt;k&gt;Beleg,
    /// NNZ&lt;k&gt;Betrieb, NNZ&lt;k&gt;Bestandskonto, NNZ&lt;k&gt;Darstellung.
    /// </summary>
    public sealed class PresetZeile
    {
        /// <summary>"E"/"A"; leer = wie Basiszeile.</summary>
        public string Art { get; }
        /// <summary>Konto-Name oder Template (Pflichtfeld der Zeile).</summary>
        public string Konto { get; }
        /// <summary>MWSt: Festwert x1000 ("19000") oder Variable ("$vat2");
        /// leer = wie Basiszeile.</summary>
        public string MwstAusdruck { get; }
        /// <summary>Arithmetische Formel ("$brutto*0,3", "$rest");
        /// leer = manuelle Eingabe beim Buchen.</summary>
        public string BetragFormel { get; }
        /// <summary>Beschreibungs-Template ("$B (7% Anteil)");
        /// leer = von Basis kopiert.</summary>
        public string TextTemplate { get; }
        /// <summary>Belegnummern-Template ("$beleg-$vorlagenzeile");
        /// leer = von Basis kopiert.</summary>
        public string BelegTemplate { get; }
        /// <summary>Betrieb (fest oder Template); leer = von Basis kopiert.</summary>
        public string BetriebTemplate { get; }
        /// <summary>Bestandskonto (fest oder Template); leer = von Basis kopiert.</summary>
        public string BestandskontoTemplate { get; }
        /// <summary>Darstellungs-Override im Buchen-Dialog:
        /// "" = Heuristik, "kompakt", "maske".</summary>
        public string Darstellung { get; }

        public PresetZeile(string art, string konto, string mwstAusdruck,
            string betragFormel, string textTemplate, string belegTemplate = "",
            string betriebTemplate = "", string bestandskontoTemplate = "",
            string darstellung = "")
        {
            Art                   = art                   ?? "";
            Konto                 = konto                 ?? "";
            MwstAusdruck          = mwstAusdruck          ?? "";
            BetragFormel          = betragFormel          ?? "";
            TextTemplate          = textTemplate          ?? "";
            BelegTemplate         = belegTemplate         ?? "";
            BetriebTemplate       = betriebTemplate       ?? "";
            BestandskontoTemplate = bestandskontoTemplate ?? "";
            Darstellung           = darstellung           ?? "";
        }

        /// <summary>True wenn der Betrag beim Buchen manuell einzugeben ist.</summary>
        public bool BetragManuell => string.IsNullOrEmpty(BetragFormel);

        public bool IstLeer =>
            Konto == "" && BetragFormel == "" && TextTemplate == "" &&
            MwstAusdruck == "" && BelegTemplate == "" && BetriebTemplate == "" &&
            BestandskontoTemplate == "";
    }

    /// <summary>Ein Buchungsposten-Preset aus der ini-Sektion [Buchungsposten].
    /// Mit Zusatz-Zeilen (Zeilen.Count &gt; 0) ist es eine
    /// Buchungsgruppen-Vorlage; die klassischen 8 Felder bilden Zeile 0.</summary>
    public sealed class Preset
    {
        public string Text    { get; }
        public bool   Ausgabe { get; }  // false=Einnahmen, true=Ausgaben
        public int    Mwst    { get; }  // Festkomma x1000 (19000 = 19 %)
        public int    AfaJ    { get; }
        public string Konto   { get; }
        /// <summary>Freitext-Notiz (Erinnerungen/Detailinfos zur Buchung).
        /// Wird im Buchen-Dialog beim Auswählen des Presets kurz als
        /// Balloon angezeigt.</summary>
        public string Notiz   { get; }
        /// <summary>Degressive AfA (statt linear).</summary>
        public bool   Degressiv { get; }
        /// <summary>Degressiver AfA-Satz in Prozent (nur relevant bei Degressiv).</summary>
        public int    AfaSatz { get; }

        /// <summary>Zusatz-Zeilen der Buchungsgruppen-Vorlage (leer bei
        /// klassischen Einzel-Presets).</summary>
        public IReadOnlyList<PresetZeile> Zeilen { get; }

        /// <summary>
        /// Optionale Formel für den tatsächlich gebuchten Betrag der
        /// BASIS-Zeile (ini-Key NNBasisBetrag). Leer = der eingegebene
        /// Betrag wird unverändert gebucht. Beispiel "belegweise
        /// Aufteilung": Nutzer tippt den Beleg-GESAMTbetrag ein, die
        /// Zusatz-Zeilen nehmen ihre Anteile, die Basis bucht "$rest".
        /// $brutto/$netto beziehen sich weiterhin auf die Eingabe.
        /// </summary>
        public string BasisBetragFormel { get; }

        public Preset(string text, bool ausgabe, int mwst, int afaj, string konto,
            string notiz = "", bool degressiv = false, int afaSatz = 0,
            IReadOnlyList<PresetZeile> zeilen = null, string basisBetragFormel = null)
        {
            Text      = text    ?? "";
            Ausgabe   = ausgabe;
            Mwst      = mwst;
            AfaJ      = afaj;
            Konto     = konto   ?? "";
            Notiz     = notiz   ?? "";
            Degressiv = degressiv;
            AfaSatz   = afaSatz;
            Zeilen    = zeilen  ?? System.Array.Empty<PresetZeile>();
            BasisBetragFormel = basisBetragFormel ?? "";
        }

        public bool IstLeer => string.IsNullOrEmpty(Text) && string.IsNullOrEmpty(Konto);

        /// <summary>True wenn das Preset eine Buchungsgruppen-Vorlage ist.</summary>
        public bool IstMehrzeilig => Zeilen.Count > 0;
    }

    /// <summary>
    /// Globaler Key-Value-Cache für die easyct.ini-Einstellungen des aktiven
    /// Mandanten. Loest das alte Pro-Dokument-Modell ab, in dem jede
    /// CEasyCashView ihre eigenen einstellungen1..5-Property-Pages hielt
    /// (was bei mehreren offenen MDI-Dokumenten zu Inkonsistenzen führte).
    ///
    /// Schlüsselformat (Plugin-API-kompatibel):
    ///   - Kurzform: "fname"  -> Sektion "Finanzamt", Ini-Key "name"
    ///   - Explizit: "[Sektion]Key"
    ///
    /// Variante X: Der Cache speichert den Schlüssel genau so wie vom
    /// Aufrufer übergeben. Konvention: Kurzform (wie Plugin-Interface).
    /// </summary>
    public static class Einstellungen
    {
        private static readonly Dictionary<string, string> _cache =
            new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        // Schützt _cache, damit Hintergrund-Persistenz (z.B. Konten-Reorder)
        // und UI-Lesezugriffe sich nicht ins Gehege kommen. Nur die kurze
        // Dictionary-Operation wird gesperrt -- NICHT der ini-Schreibvorgang
        // (WertGeaendert), damit Disk-I/O die Lesezugriffe nicht blockiert.
        private static readonly object _gate = new object();

        private static IReadOnlyList<string>       _einnahmenKonten = new List<string>();
        private static IReadOnlyList<string>       _ausgabenKonten  = new List<string>();
        private static IReadOnlyList<Preset>       _presets         = new List<Preset>();
        private static IReadOnlyList<Betrieb>      _betriebe        = new List<Betrieb>();
        private static IReadOnlyList<Bestandskonto> _bestandskonten  = new List<Bestandskonto>();

        /// <summary>
        /// Wird beim Speichern eines Wertes ausgelöst. Die Bridge hängt
        /// sich darauf, um den Wert sofort in die ini-Datei zu schreiben.
        /// </summary>
        public static event Action<string, string> WertGeaendert;

        /// <summary>
        /// Cache komplett ersetzen (von der Bridge nach ini-Lesen aufgerufen).
        /// Loest KEINE WertGeaendert-Events aus.
        /// </summary>
        public static void LadeAusBridge(IDictionary<string, string> daten)
        {
            lock (_gate)
            {
                _cache.Clear();
                if (daten != null)
                    foreach (var kv in daten)
                        _cache[kv.Key] = kv.Value ?? "";
            }
            BaueListenAuf();
            ImportierePrivatSplitVorlagen();
        }

        /// <summary>Cache leeren (z.B. bei Mandantenwechsel vor neuem Laden).</summary>
        public static void Leere()
        {
            lock (_gate) { _cache.Clear(); }
            _einnahmenKonten = new List<string>();
            _ausgabenKonten  = new List<string>();
            _presets         = new List<Preset>();
            _betriebe        = new List<Betrieb>();
            _bestandskonten  = new List<Bestandskonto>();
        }

        /// <summary>
        /// Liefert den Wert zum Schlüssel, oder Leerstring bei Cache-Miss.
        /// </summary>
        public static string Hole(string key)
        {
            if (string.IsNullOrEmpty(key)) return "";
            lock (_gate) { return _cache.TryGetValue(key, out var v) ? v : ""; }
        }

        /// <summary>
        /// Speichert den Wert im Cache und loest WertGeaendert aus
        /// (Bridge schreibt dann in die ini-Datei).
        /// </summary>
        public static void Speichere(string key, string value)
        {
            if (string.IsNullOrEmpty(key)) return;
            value = value ?? "";
            lock (_gate) { _cache[key] = value; }
            // WertGeaendert (ini-Schreibvorgang) bewusst AUSSERHALB des Locks,
            // damit Disk-I/O keine UI-Lesezugriffe blockiert.
            WertGeaendert?.Invoke(key, value);
        }

        /// <summary>
        /// Wird ausgelöst, um eine GANZE ini-Sektion auf einmal zu schreiben
        /// (statt vieler Einzel-Keys). Die Bridge schreibt das per
        /// WritePrivateProfileSection -- drastisch weniger Datei-I/O.
        /// Args: (ini-Sektionsname, iniKey -> Wert).
        /// </summary>
        public static event Action<string, IReadOnlyDictionary<string, string>> SektionGeaendert;

        /// <summary>Aktualisiert NUR den Cache (kein Event/keine ini). Für
        /// gebündeltes Schreiben: erst alle Cache-Keys setzen, dann die
        /// Sektion(en) per <see cref="SchreibeSektion"/> in die ini spülen.</summary>
        public static void SetzeCacheNur(string key, string value)
        {
            if (string.IsNullOrEmpty(key)) return;
            lock (_gate) { _cache[key] = value ?? ""; }
        }

        /// <summary>Schreibt eine komplette ini-Sektion (löst SektionGeaendert aus).
        /// Aktualisiert NICHT den Cache -- das macht der Aufrufer per
        /// <see cref="SetzeCacheNur"/>.</summary>
        public static void SchreibeSektion(string iniSektion, IReadOnlyDictionary<string, string> eintraege)
        {
            if (string.IsNullOrEmpty(iniSektion) || eintraege == null) return;
            SektionGeaendert?.Invoke(iniSektion, eintraege);
        }

        public static int HoleInt(string key, int defaultValue = 0)
        {
            return int.TryParse(Hole(key), NumberStyles.Integer,
                CultureInfo.InvariantCulture, out var n) ? n : defaultValue;
        }

        public static bool HoleBool(string key, bool defaultValue = false)
        {
            var s = Hole(key);
            if (string.IsNullOrEmpty(s)) return defaultValue;
            return s != "0" && !s.Equals("false", StringComparison.OrdinalIgnoreCase);
        }

        public static void Speichere(string key, int value)
        {
            Speichere(key, value.ToString(CultureInfo.InvariantCulture));
        }

        public static void Speichere(string key, bool value)
        {
            Speichere(key, value ? "1" : "0");
        }

        // ---------------------------------------------------------------------
        // Listen-Properties (schreibgeschützt, nach LadeAusBridge aktuell)
        // ---------------------------------------------------------------------

        /// <summary>Einnahmen-Konten aus [EinnahmenRechnungsposten] (Cache-Praefix "e").</summary>
        public static IReadOnlyList<string> EinnahmenKonten => _einnahmenKonten;

        /// <summary>Ausgaben-Konten aus [AusgabenRechnungsposten] (Cache-Praefix "a").</summary>
        public static IReadOnlyList<string> AusgabenKonten => _ausgabenKonten;

        /// <summary>
        /// Buchungsposten-Presets aus [Buchungsposten].
        /// Immer alle 100; Lücken ergeben Preset-Objekte mit IstLeer==true.
        /// </summary>
        public static IReadOnlyList<Preset> Presets => _presets;

        /// <summary>Betriebe aus [Betriebe] (sequenziell bis erstes leeres Name).</summary>
        public static IReadOnlyList<Betrieb> Betriebe => _betriebe;

        /// <summary>
        /// Bestandskonten aus [Bestandskonten] (sequenziell bis erstes leeres Name).
        /// Jedes Bestandskonto hat ein Saldo-Dictionary (Jahr => Anfangssaldo in Euro).
        /// </summary>
        public static IReadOnlyList<Bestandskonto> Bestandskonten => _bestandskonten;

        /// <summary>Diagnose: alle aktuell gecachten Schlüssel.</summary>
        public static IEnumerable<string> AlleSchluessel => _cache.Keys;

        /// <summary>
        /// Liefert einen Schnappschuss aller Cache-Schlüssel, die mit dem
        /// Präfix beginnen (case-insensitiv, thread-sicher). Wird von der
        /// Stammdaten-Verwaltung genutzt, um alle Eintrags-Keys einer
        /// Sektion (inkl. unbekannter Suffixe wie SaldoJJJJ) zu finden.
        /// </summary>
        public static List<string> HoleSchluesselMitPraefix(string praefix)
        {
            var ergebnis = new List<string>();
            if (string.IsNullOrEmpty(praefix)) return ergebnis;
            lock (_gate)
            {
                foreach (var key in _cache.Keys)
                    if (key.StartsWith(praefix, StringComparison.OrdinalIgnoreCase)
                        && !string.IsNullOrEmpty(_cache[key]))
                        ergebnis.Add(key);
            }
            return ergebnis;
        }

        // ---------------------------------------------------------------------

        /// <summary>
        /// Schreibt ein Buchungsposten-Preset (Index 0-99) in den Cache/ini
        /// und baut die Listen neu auf. Verwendet die Bracket-Cache-Form
        /// "[Buchungsposten]NN..." -- exakt die Form, in der
        /// <see cref="BaueListenAuf"/> liest, damit das Ergebnis sofort
        /// (ohne ECT_LadeEinstellungen) sichtbar ist.
        /// </summary>
        public static void SpeicherePreset(int index, Preset p)
        {
            if (index < 0 || index > 99 || p == null) return;
            var pfx = "[Buchungsposten]" + index.ToString("D2");
            Speichere(pfx + "Text", p.Text);
            Speichere(pfx + "Ausg", p.Ausgabe ? "1" : "0");
            Speichere(pfx + "MWSt", p.Mwst);
            Speichere(pfx + "AfAJ", p.AfaJ);
            Speichere(pfx + "Rech", p.Konto);
            Speichere(pfx + "Notiz", p.Notiz);
            Speichere(pfx + "Degr", p.Degressiv ? "1" : "0");
            Speichere(pfx + "AfASatz", p.AfaSatz);
            Speichere(pfx + "BasisBetrag", p.BasisBetragFormel);
            SchreibeZusatzZeilen(pfx, p.Zeilen);
            BaueListenAuf();
        }

        /// <summary>Maximale Zusatz-Zeilen pro Buchungsgruppen-Vorlage
        /// (Z1..Z19; Zeile 0 sind die klassischen Preset-Felder).</summary>
        public const int MaxPresetZeilen = 20;

        private static readonly string[] _zeilenSuffixe =
        {
            "Art", "Konto", "MWSt", "Betrag", "Text",
            "Beleg", "Betrieb", "Bestandskonto", "Darstellung"
        };

        /// <summary>Liest die Zusatz-Zeilen eines Preset-Slots
        /// ([Buchungsposten]NNZ&lt;k&gt;*). Stoppt bei der ersten Zeile
        /// ohne Konto (Pflichtfeld).</summary>
        private static IReadOnlyList<PresetZeile> LeseZusatzZeilen(string pfx)
        {
            List<PresetZeile> zeilen = null;
            for (int z = 1; z < MaxPresetZeilen; z++)
            {
                var zpfx = pfx + "Z" + z;
                var konto = Hole(zpfx + "Konto");
                if (string.IsNullOrEmpty(konto)) break;
                (zeilen ?? (zeilen = new List<PresetZeile>())).Add(new PresetZeile(
                    Hole(zpfx + "Art"),
                    konto,
                    Hole(zpfx + "MWSt"),
                    Hole(zpfx + "Betrag"),
                    Hole(zpfx + "Text"),
                    Hole(zpfx + "Beleg"),
                    Hole(zpfx + "Betrieb"),
                    Hole(zpfx + "Bestandskonto"),
                    Hole(zpfx + "Darstellung")));
            }
            return (IReadOnlyList<PresetZeile>)zeilen ?? System.Array.Empty<PresetZeile>();
        }

        /// <summary>Schreibt die Zusatz-Zeilen eines Preset-Slots und leert
        /// die Keys entfallener Zeilen (damit die Leseschleife stoppt und
        /// keine Leichen in der ini bleiben).</summary>
        private static void SchreibeZusatzZeilen(string pfx, IReadOnlyList<PresetZeile> zeilen)
        {
            int anzahl = zeilen?.Count ?? 0;
            if (anzahl > MaxPresetZeilen - 1) anzahl = MaxPresetZeilen - 1;

            for (int z = 1; z <= anzahl; z++)
            {
                var zeile = zeilen[z - 1];
                var zpfx = pfx + "Z" + z;
                Speichere(zpfx + "Art", zeile.Art);
                Speichere(zpfx + "Konto", zeile.Konto);
                Speichere(zpfx + "MWSt", zeile.MwstAusdruck);
                Speichere(zpfx + "Betrag", zeile.BetragFormel);
                Speichere(zpfx + "Text", zeile.TextTemplate);
                Speichere(zpfx + "Beleg", zeile.BelegTemplate);
                Speichere(zpfx + "Betrieb", zeile.BetriebTemplate);
                Speichere(zpfx + "Bestandskonto", zeile.BestandskontoTemplate);
                Speichere(zpfx + "Darstellung", zeile.Darstellung);
            }

            // entfallene Zeilen leeren (alle Suffixe, nicht nur Konto)
            for (int z = anzahl + 1; z < MaxPresetZeilen; z++)
            {
                var zpfx = pfx + "Z" + z;
                bool vorhanden = false;
                foreach (var suffix in _zeilenSuffixe)
                    if (!string.IsNullOrEmpty(Hole(zpfx + suffix))) { vorhanden = true; break; }
                if (!vorhanden) break;   // keine weiteren Leichen
                foreach (var suffix in _zeilenSuffixe)
                    Speichere(zpfx + suffix, "");
            }
        }

        /// <summary>Baut die abgeleiteten Listen (Presets, Konten, …) aus dem
        /// aktuellen Cache neu auf. Nach direkten Speichere-Aufrufen nötig,
        /// wenn die Listen-Properties sofort aktuell sein sollen.</summary>
        public static void NeuAufbauen() => BaueListenAuf();

        /// <summary>Baut NUR die Einnahmen-/Ausgaben-Kontenlisten neu auf
        /// (günstig). Für das Konten-Umsortieren -- der teure Komplett-Rebuild
        /// (Presets/Betriebe/Bestandskonten mit 60-Jahres-Schleife) ist dafür
        /// unnötig.</summary>
        public static void NeuAufbauenKonten()
        {
            var ek = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var v = Hole("e" + i.ToString("D2", CultureInfo.InvariantCulture));
                if (string.IsNullOrEmpty(v)) break;
                ek.Add(v);
            }
            _einnahmenKonten = ek;

            var ak = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var v = Hole("a" + i.ToString("D2", CultureInfo.InvariantCulture));
                if (string.IsNullOrEmpty(v)) break;
                ak.Add(v);
            }
            _ausgabenKonten = ak;
        }

        private static void BaueListenAuf()
        {
            // EinnahmenKonten: e00, e01, ... stopp bei erstem Leerstring
            var ek = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var val = Hole("e" + i.ToString("D2"));
                if (string.IsNullOrEmpty(val)) break;
                ek.Add(val);
            }
            _einnahmenKonten = ek;

            // AusgabenKonten: a00, a01, ... stopp bei erstem Leerstring
            var ak = new List<string>();
            for (int i = 0; i < 100; i++)
            {
                var val = Hole("a" + i.ToString("D2"));
                if (string.IsNullOrEmpty(val)) break;
                ak.Add(val);
            }
            _ausgabenKonten = ak;

            // WICHTIG -- Cache-Schluesselformat:
            // ECT_LadeEinstellungen legt Werte aus nicht-praefixierten ini-
            // Sektionen (alles ausser Finanzamt/Einnahmen-/AusgabenRechnungs-
            // posten) unter der Bracket-Form "[Sektion]IniKey" ab (siehe
            // KuerzelFuerCache in EinstellungenExports.cpp). Presets, Betriebe
            // und Bestandskonten liegen in solchen Sektionen ([Buchungsposten],
            // [Betriebe], [Bestandskonten]) und MUESSEN deshalb mit dem
            // Bracket-Praefix nachgeschlagen werden. Frueher speicherte der
            // Cache fuer unbekannte Sektionen nur den nackten iniKey -- seit
            // der Umstellung auf Bracket-Form (Fix fuer den CSV-Plugin-Lookup)
            // lieferte der nackte Lookup hier leere Listen (u.a. fehlte der
            // Bestandskonto-Anfangssaldo im Journal).

            // Presets: immer alle 100, Lücken erlaubt
            var ps = new List<Preset>(100);
            for (int i = 0; i < 100; i++)
            {
                var pfx     = "[Buchungsposten]" + i.ToString("D2");
                var text    = Hole(pfx + "Text");
                var ausgabe = Hole(pfx + "Ausg") == "1";
                var mwst    = HoleInt(pfx + "MWSt");
                var afaj    = HoleInt(pfx + "AfAJ");
                var konto   = Hole(pfx + "Rech");
                var notiz   = Hole(pfx + "Notiz");
                var degr    = Hole(pfx + "Degr") == "1";
                var afaSatz = HoleInt(pfx + "AfASatz");
                ps.Add(new Preset(text, ausgabe, mwst, afaj, konto, notiz, degr, afaSatz,
                    LeseZusatzZeilen(pfx), Hole(pfx + "BasisBetrag")));
            }
            _presets = ps;

            // Betriebe: Betrieb00Name, Betrieb01Name, ... stopp bei erstem leeren Namen
            var bt = new List<Betrieb>();
            for (int i = 0; i < 100; i++)
            {
                var pfx  = "[Betriebe]Betrieb" + i.ToString("D2");
                var name = Hole(pfx + "Name");
                if (string.IsNullOrEmpty(name)) break;
                var ua   = Hole(pfx + "Unternehmensart");
                var icon = HoleInt(pfx + "Icon");
                bt.Add(new Betrieb(name, ua, icon));
            }
            _betriebe = bt;

            // Bestandskonten: Bestandskonto00Name usw., stopp bei erstem leeren Namen;
            // Saldo-Eintraege für jedes Jahr 1990-2049 pruefen
            var bk = new List<Bestandskonto>();
            for (int i = 0; i < 100; i++)
            {
                var pfx  = "[Bestandskonten]Bestandskonto" + i.ToString("D2");
                var name = Hole(pfx + "Name");
                if (string.IsNullOrEmpty(name)) break;
                var icon  = HoleInt(pfx + "Icon");
                var saldo = new Dictionary<int, decimal>();
                for (int year = 1990; year <= 2049; year++)
                {
                    var saldoStr = Hole(pfx + "Saldo" + year);
                    if (!string.IsNullOrEmpty(saldoStr))
                        saldo[year] = ParseSaldo(saldoStr);
                }
                bk.Add(new Bestandskonto(name, icon, saldo));
            }
            _bestandskonten = bk;
        }

        // ---------------------------------------------------------------------
        // Einmaliger Import der alten [Privat-Split]-Konfiguration als
        // Buchungsgruppen-Vorlagen (mehrzeilige Presets). Ersetzt die alte
        // Einstellungsseite einstellungen5 (bleibt USE_ECTIFACE-only).
        // ---------------------------------------------------------------------

        private const string PrivatSplitMarker = "[Buchungsposten]PrivatSplitImportiert";

        /// <summary>
        /// Wandelt die Zeilen der ini-Sektion [Privat-Split] (ArtNN/SatzNN/
        /// UstSatzNN) in mehrzeilige Presets um -- einmalig pro Mandant
        /// (Marker-Key). Rechnung wie der alte Split in buchendlg.cpp:
        ///   mit VSt-Abzug  = Brutto * Satz * UstSatz      (MwSt wie Basis)
        ///   ohne VSt-Abzug = Netto  * Satz * (1-UstSatz)  (MwSt 0)
        /// </summary>
        private static void ImportierePrivatSplitVorlagen()
        {
            if (Hole(PrivatSplitMarker) == "1") return;

            bool importiert = false;
            for (int i = 0; i < 10; i++)
            {
                var nn = i.ToString("D2");
                var konto = Hole("[Privat-Split]Art" + nn).Trim();
                if (konto.Length == 0) continue;
                decimal satz = ParseProzent(Hole("[Privat-Split]Satz" + nn), 0m);
                if (satz <= 0m) continue;
                decimal ust = ParseProzent(Hole("[Privat-Split]UstSatz" + nn), 100m);

                decimal faktorMit = satz / 100m * (ust / 100m);
                decimal faktorOhne = satz / 100m * ((100m - ust) / 100m);
                string satzText = satz.ToString("0.##", _deDe);
                bool beide = faktorMit > 0m && faktorOhne > 0m;

                var zeilen = new List<PresetZeile>();
                if (faktorMit > 0m)
                    zeilen.Add(new PresetZeile("E", konto, "$mwstsatz",
                        "$brutto*" + faktorMit.ToString("0.######", _deDe),
                        "Privatanteil $beschreibung (" + satzText + "%)"
                            + (beide ? " - mit VSt-Abzug" : "")));
                if (faktorOhne > 0m)
                    zeilen.Add(new PresetZeile("E", konto, "0",
                        "$netto*" + faktorOhne.ToString("0.######", _deDe),
                        "Privatanteil $beschreibung (" + satzText + "%)"
                            + (beide ? " - ohne VSt-Abzug" : "")));
                if (zeilen.Count == 0) continue;

                // freien Preset-Slot suchen
                int frei = -1;
                for (int s = 0; s < _presets.Count; s++)
                    if (_presets[s].IstLeer) { frei = s; break; }
                if (frei < 0) break;   // alle 100 Slots belegt

                SpeicherePreset(frei, new Preset(
                    "Privat-Split: " + konto, true, 19000, 1, "",
                    "Importiert aus der alten Privat-Split-Konfiguration (" +
                    satzText + " % Privatanteil, " +
                    ust.ToString("0.##", _deDe) + " % davon mit VSt-Abzug).",
                    zeilen: zeilen));
                importiert = true;
            }

            if (importiert)
                Speichere(PrivatSplitMarker, "1");
        }

        /// <summary>Prozentwert tolerant parsen ("50", "50,0000", "50.5").</summary>
        private static decimal ParseProzent(string s, decimal fallback)
            => Waehrungsformat.TryParseProzent(s, out var d) ? d : fallback;

        private static readonly CultureInfo _deDe = new CultureInfo("de-DE");

        // Toleranter Parser (Waehrungsformat): versteht deutsche,
        // schweizerische und englische Schreibweisen. Der frühere
        // de-DE-Parse las "1234.56" (CH-Bestandsdaten) als 123456!
        private static decimal ParseSaldo(string s)
            => Waehrungsformat.TryParse(s, out decimal d) ? d : 0m;
    }
}
