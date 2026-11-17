// UstAbgleich.cs -- Abgleich der Kontenzuweisungen zwischen USt-Voranmeldung
// und Umsatzsteuererklärung.
//
// Port von CUStAbgleich (UStAbgleich.h/.cpp, v3/MFC). Datenmodell und Regeln
// sind bewusst 1:1 übernommen: v3 und v4 lesen und schreiben dieselbe
// easyct.ini. Übergabe-Doku: UST-ABGLEICH.md im Projektverzeichnis.
//
// Ein E/Ü-Konto trägt je Formularname eine Feldzuweisung (ini-Sektionen
// [EinnahmenFeldzuweisungen]/[AusgabenFeldzuweisungen], Erweiterung "ECT",
// Key = Formularname, Wert = Feld-ID; in v4 EUKonto.Feldzuweisungen).
// Voranmeldung und Erklärung haben verschiedene Formularnamen und
// verschiedene Feld-IDs, ihre Zuweisungen können also auseinanderlaufen.
// Welches Feld der Erklärung welchem Feld der Voranmeldung entspricht, steht
// als Attribut uvafeld="<Feld-ID der Voranmeldung>" an den Feldern des
// Erklärungsformulars. Mehrere Felder der Erklärung dürfen auf dasselbe Feld
// der Voranmeldung zeigen (die Erklärung gliedert feiner). Formulare ohne
// dieses Attribut nehmen am Abgleich nicht teil.
//
// Der Vermerk "dieses Paar ist so gewollt" steht als Key "USt-Abgleich" im
// selben Namensraum "ECT" (Wert "<Feld Voranmeldung>/<Feld Erklärung>") und
// liegt damit in EUKonto.Feldzuweisungen wie ein Formularname. Das ist
// gewollt (so wird er mitgespeichert); er darf nur nirgends als Formular
// behandelt werden.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml.Linq;

namespace ECTEngine
{
    public enum UstAbgleichBefund
    {
        /// <summary>nichts zu tun</summary>
        Stimmig = 0,
        /// <summary>Voranmeldung zugewiesen, Erklärung nicht; genau ein passendes Feld der Erklärung</summary>
        UstFehlt,
        /// <summary>wie UstFehlt, aber mehrere Felder der Erklärung kommen in Frage</summary>
        UstMehrdeutig,
        /// <summary>Erklärung zugewiesen, Voranmeldung nicht</summary>
        UvaFehlt,
        /// <summary>beide zugewiesen, passen aber nicht zusammen</summary>
        Widerspruch
    }

    /// <summary>Für welches Formular wird gezählt (Hinweis am Formular)?</summary>
    public enum UstAbgleichFormular
    {
        /// <summary>was die Voranmeldung verfälscht: UvaFehlt, Widerspruch</summary>
        Voranmeldung,
        /// <summary>was die Erklärung verfälscht: UstFehlt, UstMehrdeutig, Widerspruch</summary>
        Erklaerung,
        /// <summary>jeder Befund außer Stimmig</summary>
        Beide
    }

    /// <summary>Ein Feld eines der beiden Formulare (nur typ Einnahmen/Ausgaben).</summary>
    public sealed class UstAbgleichFeld
    {
        public int    Id          { get; }
        public bool   IstEinnahme { get; }
        public bool   IstVeraltet { get; }
        /// <summary>Nur Erklärung: entsprechendes Feld der Voranmeldung, 0 = keines.</summary>
        public int    UvaFeld     { get; }
        public string Name        { get; }

        public UstAbgleichFeld(int id, bool istEinnahme, bool istVeraltet, int uvaFeld, string name)
        {
            Id          = id;
            IstEinnahme = istEinnahme;
            IstVeraltet = istVeraltet;
            UvaFeld     = uvaFeld;
            Name        = name ?? "";
        }
    }

    /// <summary>Ergebnis von <see cref="UstAbgleich.NachZuweisung"/>.</summary>
    public sealed class UstMitfuehrung
    {
        public enum ArtTyp
        {
            /// <summary>nichts zu tun</summary>
            Nichts,
            /// <summary>Gegenstück wurde still mitgeführt (PartnerNeu)</summary>
            PartnerGesetzt,
            /// <summary>mehrere Felder kommen in Frage (Kandidaten), der Benutzer muss wählen</summary>
            PartnerWaehlen,
            /// <summary>Gegenstück hat seine Entsprechung verloren: entfernen? (Rückfrage)</summary>
            PartnerEntfernen,
            /// <summary>Gegenstück war schon vorher abweichend belegt: angleichen? (Rückfrage)</summary>
            Widerspruch
        }

        public ArtTyp Art { get; set; }
        /// <summary>true: das Gegenstück ist die Zuweisung für die Erklärung.</summary>
        public bool PartnerIstUst { get; set; }
        /// <summary>neue Feld-ID im geänderten Formular</summary>
        public int GeaendertNeu { get; set; }
        /// <summary>bisherige Feld-ID des Gegenstücks</summary>
        public int PartnerAlt { get; set; }
        /// <summary>PartnerGesetzt: gesetzte Feld-ID; Widerspruch: eindeutiger Vorschlag oder 0</summary>
        public int PartnerNeu { get; set; }
        /// <summary>PartnerWaehlen und Widerspruch ohne eindeutigen Vorschlag</summary>
        public List<int> Kandidaten { get; } = new List<int>();

        /// <summary>Formularname des Gegenstücks.</summary>
        public string PartnerFormular => PartnerIstUst ? UstAbgleich.FormularUst : UstAbgleich.FormularUva;
        /// <summary>Formularname des geänderten Formulars.</summary>
        public string GeaendertFormular => PartnerIstUst ? UstAbgleich.FormularUva : UstAbgleich.FormularUst;
    }

    public sealed class UstAbgleich
    {
        public const string FormularUva  = "Umsatzsteuer-Voranmeldung";
        public const string FormularUst  = "Umsatzsteuererklärung";
        public const string KeyBelassen  = "USt-Abgleich";

        private readonly List<UstAbgleichFeld> _uva = new List<UstAbgleichFeld>();
        private readonly List<UstAbgleichFeld> _ust = new List<UstAbgleichFeld>();

        /// <summary>false = kein Abgleich möglich (kein passendes Formular); alle
        /// Prüfungen liefern dann Stimmig, NachZuweisung tut nichts.</summary>
        public bool   IstAktiv         { get; private set; }
        public string UstFormulardatei { get; private set; } = "";
        public string UvaFormulardatei { get; private set; } = "";
        /// <summary>Jahr der geladenen Erklärung (aus dem Anzeigenamen), 0 = unbekannt.</summary>
        public int    UstJahr          { get; private set; }

        public IReadOnlyList<UstAbgleichFeld> UvaFelder => _uva;
        public IReadOnlyList<UstAbgleichFeld> UstFelder => _ust;

        private UstAbgleich() { }

        /// <summary>Ein Abgleich ohne Formulare (IstAktiv = false).</summary>
        public static UstAbgleich Inaktiv() => new UstAbgleich();

        // -----------------------------------------------------------------
        // Gemerkte Instanzen (Pendant zu GetUStAbgleich()): die .ecf ändern
        // sich zur Laufzeit nicht, das Laden kostet aber ein paar hundert
        // Dateizugriffe. Gemerkt wird je verlangtem Jahr (Konten-Seite und
        // Formularansicht dürfen verschiedene Jahre zeigen, ohne sich
        // gegenseitig das Laden aufzuzwingen). Die Instanzen sind nach dem
        // Laden unveränderlich und damit gefahrlos teilbar.
        // -----------------------------------------------------------------
        private static readonly object _sperre = new object();
        private static readonly Dictionary<int, UstAbgleich> _gemerkt = new Dictionary<int, UstAbgleich>();

        /// <summary>Abgleich für das Jahr aus den installierten Formularen
        /// (Programmverzeichnis); je Jahr gemerkt. Jahr 0 (o.ä. ohne eigene
        /// Erklärung) = neueste Erklärung mit uvafeld-Attributen.</summary>
        public static UstAbgleich FuerJahr(int jahr)
        {
            lock (_sperre)
            {
                if (!_gemerkt.TryGetValue(jahr, out var a))
                {
                    a = Lade(jahr);
                    _gemerkt[jahr] = a;
                }
                return a;
            }
        }

        /// <summary>Vergisst die gemerkten Instanzen (z.B. nach Formular-Installation).</summary>
        public static void Verwerfen()
        {
            lock (_sperre)
                _gemerkt.Clear();
        }

        /// <summary>Installierte Formulare = *.ecf im Programmverzeichnis
        /// (wie LadeECFormulare und EUKonten).</summary>
        public static IEnumerable<string> InstallierteFormulare()
        {
            try { return Directory.GetFiles(AppDomain.CurrentDomain.BaseDirectory, "*.ecf"); }
            catch { return new string[0]; }
        }

        // -----------------------------------------------------------------
        // Laden
        // -----------------------------------------------------------------

        /// <summary>
        /// Sucht unter den Formulardateien die Erklärung des Jahres (ersatzweise
        /// die neueste mit uvafeld-Attributen) und eine Voranmeldung dazu.
        /// <paramref name="formulardateien"/> = null: installierte Formulare.
        /// </summary>
        public static UstAbgleich Lade(int jahr, IEnumerable<string> formulardateien = null)
        {
            var dateien = (formulardateien ?? InstallierteFormulare()).ToList();

            var ust = new List<(string Datei, int Jahr)>();
            var uva = new List<(string Datei, int Jahr)>();
            foreach (var datei in dateien)
            {
                if (!LiesFormularkopf(datei, out string name, out string anzeigename)) continue;
                if (name == FormularUst)
                    ust.Add((datei, JahrAusAnzeigename(anzeigename)));
                else if (name == FormularUva && anzeigename.StartsWith(FormularUva + " ", StringComparison.Ordinal))
                    // Die Erklärungsformulare bis 2024 heißen intern ebenfalls
                    // "Umsatzsteuer-Voranmeldung"; sie scheiden hier aus.
                    uva.Add((datei, JahrAusAnzeigename(anzeigename)));
            }

            // Erklärung: die des verlangten Jahres, sonst die neueste, die uvafeld-Attribute hat
            string ustDatei = null;
            int ustJahr = 0;
            while (ust.Count > 0)
            {
                int beste = WaehleBeste(ust, jahr);
                var felder = LadeFelder(ust[beste].Datei, istUst: true);
                if (felder != null && felder.Any(f => f.UvaFeld != 0))
                {
                    ustDatei = ust[beste].Datei;
                    ustJahr = ust[beste].Jahr;
                    break;
                }
                ust.RemoveAt(beste);
            }
            if (ustDatei == null || uva.Count == 0) return Inaktiv();

            // Voranmeldung: eine aus dem Jahr der Erklärung, sonst die neueste
            // (der Zeitraum spielt keine Rolle, alle haben dieselben Felder)
            var ergebnis = LadeAusDateien(ustDatei, uva[WaehleBeste(uva, ustJahr)].Datei);
            ergebnis.UstJahr = ustJahr;
            return ergebnis;
        }

        // Index des Eintrags mit dem gesuchten Jahr, sonst des neuesten. Bei
        // Gleichstand gewinnt der erste (wie in v3).
        private static int WaehleBeste(List<(string Datei, int Jahr)> liste, int jahr)
        {
            int beste = 0;
            for (int i = 1; i < liste.Count; i++)
                if (liste[beste].Jahr != jahr && (liste[i].Jahr == jahr || liste[i].Jahr > liste[beste].Jahr))
                    beste = i;
            return beste;
        }

        /// <summary>Lädt genau diese beiden Formulare. Inaktiv, wenn die
        /// Erklärung keine uvafeld-Attribute hat oder eine Datei fehlt.</summary>
        public static UstAbgleich LadeAusDateien(string ustFormulardatei, string uvaFormulardatei)
        {
            var a = new UstAbgleich();
            var ust = LadeFelder(ustFormulardatei, istUst: true);
            if (ust == null || !ust.Any(f => f.UvaFeld != 0)) return a;   // ohne uvafeld: kein Abgleich
            var uva = LadeFelder(uvaFormulardatei, istUst: false);
            if (uva == null) return a;

            a._ust.AddRange(ust);
            a._uva.AddRange(uva);
            a.UstFormulardatei = ustFormulardatei;
            a.UvaFormulardatei = uvaFormulardatei;
            if (LiesFormularkopf(ustFormulardatei, out _, out string anzeigename))
                a.UstJahr = JahrAusAnzeigename(anzeigename);
            a.IstAktiv = true;
            return a;
        }

        private static Encoding Cp1252()
        {
            try { return Encoding.GetEncoding(1252); }
            catch { return Encoding.Default; }
        }

        // Felder vom Typ Einnahmen/Ausgaben, MIT den veralteten; null = nicht lesbar oder leer
        private static List<UstAbgleichFeld> LadeFelder(string datei, bool istUst)
        {
            XDocument doc;
            try
            {
                using (var leser = new StreamReader(datei, Cp1252()))
                    doc = XDocument.Load(leser);
            }
            catch { return null; }

            var felderNode = doc.Root?.Element("felder");
            if (felderNode == null) return null;

            var felder = new List<UstAbgleichFeld>();
            foreach (var feld in felderNode.Elements())
            {
                string typ = (string)feld.Attribute("typ");
                if (typ == null) continue;
                bool einnahme;
                if (typ.Equals("Einnahmen", StringComparison.OrdinalIgnoreCase)) einnahme = true;
                else if (typ.Equals("Ausgaben", StringComparison.OrdinalIgnoreCase)) einnahme = false;
                else continue;   // nur Felder, denen Konten zugewiesen werden können

                int id = Atoi((string)feld.Attribute("id"));
                if (id == 0) continue;
                bool veraltet = string.Equals((string)feld.Attribute("veraltet"), "ja", StringComparison.OrdinalIgnoreCase);
                int uvaFeld = istUst ? Atoi((string)feld.Attribute("uvafeld")) : 0;
                string name = (string)feld.Element("name") ?? "";
                felder.Add(new UstAbgleichFeld(id, einnahme, veraltet, uvaFeld, name));
            }
            return felder.Count > 0 ? felder : null;
        }

        /// <summary>
        /// Attribute name und anzeigename einer Formulardatei, gelesen aus dem
        /// Dateianfang (ohne XML-Parser). Es gibt einige hundert installierte
        /// Formulare; sie alle als XML zu laden, nur um die zwei passenden zu
        /// finden, wäre zu langsam.
        /// </summary>
        public static bool LiesFormularkopf(string datei, out string name, out string anzeigename)
        {
            name = "";
            anzeigename = "";
            string kopf;
            try
            {
                var puffer = new byte[800];
                int n;
                using (var f = new FileStream(datei, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                    n = f.Read(puffer, 0, puffer.Length);
                kopf = Cp1252().GetString(puffer, 0, n);
            }
            catch { return false; }

            int start = kopf.IndexOf("<formular", StringComparison.Ordinal);
            if (start < 0) return false;
            kopf = kopf.Substring(start);
            int ende = kopf.IndexOf('>');
            if (ende >= 0) kopf = kopf.Substring(0, ende);
            name = KopfAttribut(kopf, "name");
            anzeigename = KopfAttribut(kopf, "anzeigename");
            return name.Length > 0;
        }

        private static string KopfAttribut(string kopf, string attribut)
        {
            string such = " " + attribut + "=\"";   // das Leerzeichen trennt name= von anzeigename=
            int start = kopf.IndexOf(such, StringComparison.Ordinal);
            if (start < 0) return "";
            start += such.Length;
            int ende = kopf.IndexOf('"', start);
            return ende < 0 ? "" : kopf.Substring(start, ende - start);
        }

        /// <summary>Erste vierstellige Jahreszahl (19xx/20xx) im Anzeigenamen, 0 = keine.</summary>
        public static int JahrAusAnzeigename(string anzeigename)
        {
            if (string.IsNullOrEmpty(anzeigename)) return 0;
            string s = anzeigename;
            for (int i = 0; i + 4 <= s.Length; i++)
            {
                bool jahrhundert = (s[i] == '1' && s[i + 1] == '9') || (s[i] == '2' && s[i + 1] == '0');
                if (jahrhundert && IstZiffer(s[i + 2]) && IstZiffer(s[i + 3])
                    && (i + 4 == s.Length || !IstZiffer(s[i + 4])))
                    return (s[i] - '0') * 1000 + (s[i + 1] - '0') * 100 + (s[i + 2] - '0') * 10 + (s[i + 3] - '0');
            }
            return 0;
        }

        private static bool IstZiffer(char c) => c >= '0' && c <= '9';

        // atoi: führender Leerraum, optionales Vorzeichen, Ziffern bis zum ersten Nicht-Ziffernzeichen
        private static int Atoi(string s)
        {
            if (string.IsNullOrEmpty(s)) return 0;
            int i = 0;
            while (i < s.Length && char.IsWhiteSpace(s[i])) i++;
            bool negativ = false;
            if (i < s.Length && (s[i] == '-' || s[i] == '+')) { negativ = s[i] == '-'; i++; }
            long wert = 0;
            for (; i < s.Length && IstZiffer(s[i]); i++)
            {
                wert = wert * 10 + (s[i] - '0');
                if (wert > int.MaxValue) return 0;
            }
            return (int)(negativ ? -wert : wert);
        }

        // -----------------------------------------------------------------
        // Zugriff auf die Feldzuweisung eines Kontos (= EUKonto.Feldzuweisungen)
        // -----------------------------------------------------------------

        public static int LiesZuweisung(IDictionary<string, string> feldzuweisung, string formularname)
        {
            if (feldzuweisung == null) return 0;
            return feldzuweisung.TryGetValue(formularname, out var wert) ? Atoi(wert) : 0;
        }

        /// <summary>Setzt die Feld-ID; 0 entfernt die Zuweisung (v3 schreibt
        /// dann "0", liest einen fehlenden Key aber ebenfalls als 0).</summary>
        public static void SchreibeZuweisung(IDictionary<string, string> feldzuweisung, string formularname, int feldId)
        {
            if (feldzuweisung == null) return;
            if (feldId == 0)
                feldzuweisung.Remove(formularname);
            else
                feldzuweisung[formularname] = feldId.ToString(System.Globalization.CultureInfo.InvariantCulture);
        }

        public static bool IstBelassen(IDictionary<string, string> feldzuweisung, int uvaFeld, int ustFeld)
        {
            if (feldzuweisung == null || !feldzuweisung.TryGetValue(KeyBelassen, out var wert)) return false;
            return wert == uvaFeld + "/" + ustFeld;
        }

        /// <summary>Vermerkt bzw. löscht das aktuelle Paar als "so gewollt".</summary>
        public static void SetzeBelassen(IDictionary<string, string> feldzuweisung, bool belassen)
        {
            if (feldzuweisung == null) return;
            if (belassen)
                feldzuweisung[KeyBelassen] = LiesZuweisung(feldzuweisung, FormularUva) + "/"
                                           + LiesZuweisung(feldzuweisung, FormularUst);
            else
                feldzuweisung.Remove(KeyBelassen);
        }

        // -----------------------------------------------------------------
        // Korrespondenz
        // -----------------------------------------------------------------

        private static UstAbgleichFeld Finde(List<UstAbgleichFeld> felder, int id)
            => id == 0 ? null : felder.FirstOrDefault(f => f.Id == id);

        public UstAbgleichFeld UvaFeld(int id) => Finde(_uva, id);
        public UstAbgleichFeld UstFeld(int id) => Finde(_ust, id);

        /// <summary>Feld der Voranmeldung zum Feld der Erklärung; 0 = keine Entsprechung.</summary>
        public int UvaFeldZu(int ustFeld) => UstFeld(ustFeld)?.UvaFeld ?? 0;

        /// <summary>Alle nicht veralteten Felder der Erklärung des passenden Typs
        /// mit diesem uvafeld (Formular-Reihenfolge).</summary>
        public List<int> UstKandidaten(int uvaFeld, bool istEinnahme)
        {
            var kandidaten = new List<int>();
            if (uvaFeld == 0) return kandidaten;
            foreach (var f in _ust)
                if (f.UvaFeld == uvaFeld && f.IstEinnahme == istEinnahme && !f.IstVeraltet)
                    kandidaten.Add(f.Id);
            return kandidaten;
        }

        /// <summary>Feld der Voranmeldung existiert, Typ passt, nicht veraltet.</summary>
        public bool UvaFeldZuweisbar(int uvaFeld, bool istEinnahme)
        {
            var f = UvaFeld(uvaFeld);
            return f != null && f.IstEinnahme == istEinnahme && !f.IstVeraltet;
        }

        /// <summary>"Feld 741: &lt;Name&gt;", "" bei 0.</summary>
        public string FeldText(bool istUst, int feldId)
        {
            if (feldId == 0) return "";
            var f = istUst ? UstFeld(feldId) : UvaFeld(feldId);
            return f != null
                ? $"Feld {feldId}: {f.Name}"
                : $"Feld {feldId} (im Formular nicht vorhanden)";
        }

        // -----------------------------------------------------------------
        // Prüfung
        // -----------------------------------------------------------------

        /// <summary>Befund für ein Paar, ohne Rücksicht auf den Vermerk.</summary>
        public UstAbgleichBefund PruefePaar(int uvaFeld, int ustFeld, bool istEinnahme)
        {
            if (!IstAktiv) return UstAbgleichBefund.Stimmig;

            // Zuweisungen auf Felder, die es im Formular nicht (mehr) gibt, zählen als nicht zugewiesen
            if (UvaFeld(uvaFeld) == null) uvaFeld = 0;
            if (UstFeld(ustFeld) == null) ustFeld = 0;
            if (uvaFeld == 0 && ustFeld == 0) return UstAbgleichBefund.Stimmig;

            int uvaZurErklaerung = UvaFeldZu(ustFeld);   // 0: das Feld gibt es nur in der Erklärung
            if (uvaFeld == 0)
                return uvaZurErklaerung != 0 && UvaFeldZuweisbar(uvaZurErklaerung, istEinnahme)
                    ? UstAbgleichBefund.UvaFehlt : UstAbgleichBefund.Stimmig;

            var kandidaten = UstKandidaten(uvaFeld, istEinnahme);   // leer: das Feld gibt es nur in der Voranmeldung
            if (ustFeld == 0)
                return kandidaten.Count == 0 ? UstAbgleichBefund.Stimmig
                     : kandidaten.Count == 1 ? UstAbgleichBefund.UstFehlt
                     : UstAbgleichBefund.UstMehrdeutig;

            if (uvaZurErklaerung == uvaFeld) return UstAbgleichBefund.Stimmig;
            if (uvaZurErklaerung == 0 && kandidaten.Count == 0) return UstAbgleichBefund.Stimmig;   // beide ohne Gegenstück
            return UstAbgleichBefund.Widerspruch;
        }

        /// <summary>Befund für die Feldzuweisung eines Kontos, mit Vermerk "so gewollt".</summary>
        public UstAbgleichBefund Pruefe(IDictionary<string, string> feldzuweisung, bool istEinnahme)
        {
            if (!IstAktiv || feldzuweisung == null || feldzuweisung.Count == 0) return UstAbgleichBefund.Stimmig;
            int uva = LiesZuweisung(feldzuweisung, FormularUva);
            int ust = LiesZuweisung(feldzuweisung, FormularUst);
            var befund = PruefePaar(uva, ust, istEinnahme);
            if (befund != UstAbgleichBefund.Stimmig && IstBelassen(feldzuweisung, uva, ust)) return UstAbgleichBefund.Stimmig;
            return befund;
        }

        /// <summary>Zählt der Befund für den Hinweis an diesem Formular?</summary>
        public static bool Verfaelscht(UstAbgleichBefund befund, UstAbgleichFormular formular)
        {
            switch (formular)
            {
                case UstAbgleichFormular.Voranmeldung:
                    return befund == UstAbgleichBefund.UvaFehlt || befund == UstAbgleichBefund.Widerspruch;
                case UstAbgleichFormular.Erklaerung:
                    return befund == UstAbgleichBefund.UstFehlt || befund == UstAbgleichBefund.UstMehrdeutig
                        || befund == UstAbgleichBefund.Widerspruch;
                default:
                    return befund != UstAbgleichBefund.Stimmig;
            }
        }

        /// <summary>
        /// Nach einer geänderten Zuweisung für eines der beiden Formulare das
        /// Gegenstück mitführen. <paramref name="ustGeaendert"/>: geändert wurde
        /// die Zuweisung für die Erklärung (sonst die für die Voranmeldung);
        /// <paramref name="alt"/>: Feld-ID vor der Änderung (0 = keine).
        /// Eindeutiges wird sofort in <paramref name="feldzuweisung"/>
        /// eingetragen, alles andere meldet Art für eine Rückfrage.
        /// </summary>
        public UstMitfuehrung NachZuweisung(IDictionary<string, string> feldzuweisung, bool istEinnahme,
            bool ustGeaendert, int alt)
        {
            int uvaFeld = LiesZuweisung(feldzuweisung, FormularUva);
            int ustFeld = LiesZuweisung(feldzuweisung, FormularUst);
            int neu      = ustGeaendert ? ustFeld : uvaFeld;
            int partner  = ustGeaendert ? uvaFeld : ustFeld;
            string partnerFormular = ustGeaendert ? FormularUva : FormularUst;

            var m = new UstMitfuehrung
            {
                Art           = UstMitfuehrung.ArtTyp.Nichts,
                PartnerIstUst = !ustGeaendert,
                GeaendertNeu  = neu,
                PartnerAlt    = partner,
                PartnerNeu    = partner
            };
            if (!IstAktiv || feldzuweisung == null || neu == alt) return m;

            SetzeBelassen(feldzuweisung, false);   // der Vermerk galt dem bisherigen Paar

            // Was gehört im anderen Formular zur neuen Zuweisung?
            int ziel = 0;
            if (neu != 0)
            {
                if (ustGeaendert)
                {
                    int u = UvaFeldZu(neu);
                    if (u != 0 && UvaFeldZuweisbar(u, istEinnahme)) ziel = u;
                }
                else
                {
                    m.Kandidaten.AddRange(UstKandidaten(neu, istEinnahme));
                    if (m.Kandidaten.Count == 1)
                    {
                        ziel = m.Kandidaten[0];
                        m.Kandidaten.Clear();
                    }
                }
            }
            bool mehrdeutig = m.Kandidaten.Count > 1;

            if (partner == 0)
            {   // Gegenstück noch frei: Eindeutiges eintragen, sonst wählen lassen. Wurde die Zuweisung entfernt, bleibt es dabei.
                if (ziel != 0)
                {
                    SchreibeZuweisung(feldzuweisung, partnerFormular, ziel);
                    m.PartnerNeu = ziel;
                    m.Art = UstMitfuehrung.ArtTyp.PartnerGesetzt;
                }
                else if (mehrdeutig)
                    m.Art = UstMitfuehrung.ArtTyp.PartnerWaehlen;
                return m;
            }

            if (PruefePaar(uvaFeld, ustFeld, istEinnahme) == UstAbgleichBefund.Stimmig)
            {
                m.Kandidaten.Clear();
                return m;
            }

            // Lief das Gegenstück bisher im Gleichschritt mit der alten Zuweisung? Dann folgt es ihr auch jetzt.
            bool gleichschritt = alt != 0 && (ustGeaendert ? UvaFeldZu(alt) == partner : UvaFeldZu(partner) == alt);
            if (gleichschritt)
            {
                if (ziel != 0)
                {
                    SchreibeZuweisung(feldzuweisung, partnerFormular, ziel);
                    m.PartnerNeu = ziel;
                    m.Art = UstMitfuehrung.ArtTyp.PartnerGesetzt;
                }
                else if (mehrdeutig)
                    m.Art = UstMitfuehrung.ArtTyp.PartnerWaehlen;
                else
                    m.Art = UstMitfuehrung.ArtTyp.PartnerEntfernen;   // Zuweisung entfernt oder neues Feld ohne Gegenstück
            }
            else if (neu != 0)
            {   // das Gegenstück war schon vorher eigenständig belegt und passt auch zur neuen Zuweisung nicht
                m.Art = UstMitfuehrung.ArtTyp.Widerspruch;
                m.PartnerNeu = ziel;
            }
            // sonst: Zuweisung entfernt, das eigenständige Gegenstück bleibt unberührt
            return m;
        }

        // -----------------------------------------------------------------
        // Texte (Port von UStAbgleichBefundText / UStAbgleichMeldung)
        // -----------------------------------------------------------------

        /// <summary>Klartext zu einem Befund (Tooltip, Dialog).</summary>
        public string BefundText(UstAbgleichBefund befund, int uvaFeld, int ustFeld, bool istEinnahme)
        {
            switch (befund)
            {
                case UstAbgleichBefund.UstFehlt:
                {
                    var k = UstKandidaten(uvaFeld, istEinnahme);
                    return $"Für die {FormularUst} fehlt die Zuordnung. Zum Feld {uvaFeld} der Voranmeldung gehört dort "
                         + (k.Count > 0 ? FeldText(true, k[0]) : "") + ".";
                }
                case UstAbgleichBefund.UstMehrdeutig:
                    return $"Für die {FormularUst} fehlt die Zuordnung. Das Feld {uvaFeld} der Voranmeldung verteilt sich dort "
                         + "auf mehrere Felder; eines davon ist zu wählen.";
                case UstAbgleichBefund.UvaFehlt:
                    return $"Für die {FormularUva} fehlt die Zuordnung. Zum Feld {ustFeld} der Erklärung gehört dort "
                         + FeldText(false, UvaFeldZu(ustFeld)) + ".";
                case UstAbgleichBefund.Widerspruch:
                    return $"Die Zuordnungen passen nicht zusammen. Voranmeldung: {FeldText(false, uvaFeld)}. "
                         + $"Erklärung: {FeldText(true, ustFeld)}.";
                default:
                    return "";
            }
        }

        /// <summary>
        /// Zählt die Konten, deren Befund das Formular verfälscht, dazu die mit
        /// Buchungen im Buchungsjahr des Dokuments (Pendant zu
        /// UStAbgleichZaehleBefunde). <paramref name="doc"/> darf null sein.
        /// </summary>
        public int ZaehleBefunde(IEnumerable<EUKonto> konten, BuchungsDocument doc,
            UstAbgleichFormular formular, out int mitBuchungen)
        {
            mitBuchungen = 0;
            if (!IstAktiv || konten == null) return 0;
            int anzahl = 0;
            foreach (var k in konten)
            {
                if (!Verfaelscht(Pruefe(k.Feldzuweisungen, k.IstEinnahme), formular)) continue;
                anzahl++;
                if (ZaehleBuchungen(doc, k.IstEinnahme, k.Name) > 0) mitBuchungen++;
            }
            return anzahl;
        }

        /// <summary>Buchungen eines Kontos im Buchungsjahr des Dokuments.</summary>
        public static int ZaehleBuchungen(BuchungsDocument doc, bool istEinnahme, string konto)
        {
            if (doc == null) return 0;
            var buchungen = istEinnahme ? doc.Einnahmen : doc.Ausgaben;
            return buchungen.Count(b => b.Datum.Year == doc.Jahr && b.Konto == konto);
        }

        /// <summary>
        /// Hinweistext am Formular (Wortlaut wie v3, UStAbgleichMeldung):
        /// "Bei 3 E/Ü-Konten fehlt die Zuordnung zur Umsatzsteuererklärung oder
        /// passt nicht zu der für die Umsatzsteuer-Voranmeldung (2 davon mit
        /// Buchungen in 2025). Beträge dieser Konten können hier fehlen oder im
        /// falschen Feld stehen." "" bei anzahl 0.
        /// </summary>
        public static string HinweisText(UstAbgleichFormular formular, int anzahl, int mitBuchungen, int jahr)
        {
            if (anzahl <= 0) return "";
            string hier  = formular == UstAbgleichFormular.Voranmeldung ? FormularUva : FormularUst;
            string dort  = formular == UstAbgleichFormular.Voranmeldung ? FormularUst : FormularUva;
            string j = jahr.ToString("D4", System.Globalization.CultureInfo.InvariantCulture);
            string buchungen;
            if (anzahl == 1)
                buchungen = mitBuchungen > 0 ? $"es hat Buchungen in {j}" : $"ohne Buchungen in {j}";
            else if (mitBuchungen > 0)
                buchungen = $"{mitBuchungen} davon mit Buchungen in {j}";
            else
                buchungen = $"keines davon mit Buchungen in {j}";
            string konten = anzahl == 1 ? "Bei einem E/Ü-Konto" : $"Bei {anzahl} E/Ü-Konten";
            // Der Schlusssatz steht in v3 auch bei einem Konto im Plural -- bewusst gleich gelassen.
            return $"{konten} fehlt die Zuordnung zur {hier} oder passt nicht zu der für die {dort} ({buchungen}). "
                 + "Beträge dieser Konten können hier fehlen oder im falschen Feld stehen.";
        }
    }
}
