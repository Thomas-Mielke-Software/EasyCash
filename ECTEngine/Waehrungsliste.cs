// Waehrungsliste.cs -- Liste der von der ect-forex-API unterstuetzten
// Waehrungen fuer den Umrechnungsknopf im Buchen-Dialog.
//
// Der REST-Endpunkt liefert unter "currency=*&oneshot=1" ein JSON-Objekt
// { "Code": "Vollname", ... } aller verfuegbaren Waehrungen
// (siehe https://github.com/Thomas-Mielke-Software/ect-forex-api).
//
// Diese Liste wird beim ersten Oeffnen der Waehrungen-Einstellungsseite
// abgerufen und lokal zwischengespeichert; danach wird generell aus dem
// Cache gelesen. Nur der "Liste aktualisieren"-Knopf holt sie neu (und
// ueberschreibt den Cache NUR bei erfolgreichem Abruf).
//
// Bewusst als globale (mandantenunabhaengige) Cache-DATEI statt in der
// easyct.ini: (1) die Waehrungs-Universum ist fuer alle Mandanten gleich,
// (2) die Namen enthalten Zeichen ausserhalb Latin-1 (z.B. "Zloty" mit
// l-Strich, U+0142) -- ueber die MultiByte-ini-Schnittstelle wuerde das
// zu Mojibake fuehren, eine UTF-8-Datei loest das sauber.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;

namespace ECTEngine
{
    /// <summary>Eine Waehrung aus der ect-forex-API: Ticker-Code, Vollname
    /// und ob sie im Buchen-Dialog-Umrechner angeboten werden soll.</summary>
    public sealed class WaehrungsEintrag
    {
        public string Code  { get; set; } = "";
        public string Name  { get; set; } = "";
        public bool   Aktiv { get; set; }
    }

    /// <summary>
    /// Kurs-Information einer Waehrung zu einem Datum (aus der meta-Abfrage).
    /// WICHTIG -- die API ist NICHT einheitlich (empirisch verifiziert):
    ///   - Fiat  (type "fiat",  Quelle EZB):           price = FREMDWAEHRUNG pro EUR
    ///     (EZB-Referenzkurs, z.B. USD 1,1339 = 1 EUR).  -> Euro = Betrag / price
    ///   - Krypto (type "crypto", Quelle LiveCoinWatch): price = EUR pro Einheit
    ///     (z.B. BTC 92019 = 1 BTC).                      -> Euro = Betrag * price
    /// Die README behauptet faelschlich einheitlich "EUR pro Einheit"; die
    /// realen Fiat-Daten widersprechen dem. Daher entscheidet der type-Wert.
    /// </summary>
    public sealed class KursInfo
    {
        public string  Code  { get; set; } = "";
        public decimal Preis { get; set; }        // roher API-Wert (price)
        public string  Typ   { get; set; } = "";  // "fiat" | "crypto" | ...

        /// <summary>True fuer EZB-Fiat (price = Fremdwaehrung pro EUR).</summary>
        public bool IstFiat => string.Equals(Typ, "fiat", StringComparison.OrdinalIgnoreCase);

        /// <summary>Euro-Wert EINER Einheit dieser Waehrung. Fiat -> Kehrwert,
        /// Krypto -> unveraendert (siehe Klassen-Kommentar).</summary>
        public decimal EuroProEinheit => IstFiat ? 1m / Preis : Preis;
    }

    /// <summary>
    /// Zwischenspeicher und API-Anbindung der verfuegbaren Waehrungen.
    /// Rein statisch; der Aktiv-Zustand pro Waehrung ueberlebt eine
    /// Aktualisierung der Liste.
    /// </summary>
    public static class Waehrungsliste
    {
        /// <summary>Basis-URL der ect-forex-API.</summary>
        public const string ApiBasis =
            "https://easyct.de/ect-forex-api/ect-forex-api.php";

        /// <summary>API-URL fuer die vollstaendige Waehrungsliste
        /// ({ "Code": "Name", ... }).</summary>
        public const string ApiUrl = ApiBasis + "?currency=*&oneshot=1";

        /// <summary>
        /// Startauswahl der voreingestellt angehakten Waehrungen, abhaengig vom
        /// eingestellten Land ([Persoenliche_Daten]land: 0=DE, 1=AT, 2=CH).
        /// Heuristik: gaengige Welt-/Geschaeftswaehrungen (USD, GBP) + der
        /// sichere Hafen CHF (ausser fuer die Schweiz, wo CHF die
        /// Buchhaltungs-/Basiswaehrung ist) + die NICHT-Euro-NACHBARLAENDER,
        /// wo grenznaher Handel/Reise/Einkauf typisch ist:
        ///   DE -> Daenemark (DKK), Polen (PLN), Tschechien (CZK), Schweiz (CHF)
        ///   AT -> Tschechien (CZK), Ungarn (HUF), Schweiz (CHF)
        ///   CH -> keine Nicht-Euro-Nachbarn -> nur Welt-Waehrungen
        /// Nur die STARTauswahl; der Anwender kann jederzeit um-/abhaken.
        /// </summary>
        private static string[] StandardAktivFuerLand(int land)
        {
            switch (land)
            {
                case 1:  // Oesterreich
                    return new[] { "USD", "GBP", "CHF", "CZK", "HUF" };
                case 2:  // Schweiz (CHF = Basiswaehrung -> nicht vorangehakt)
                    return new[] { "USD", "GBP" };
                default: // Deutschland (0)
                    return new[] { "USD", "GBP", "CHF", "DKK", "PLN", "CZK" };
            }
        }

        private static HashSet<string> StandardAktivMenge(int land)
            => new HashSet<string>(
                StandardAktivFuerLand(land < 0 ? GlobaleEinstellungen.Land : land),
                StringComparer.OrdinalIgnoreCase);

        private static string _cacheDateiPfad = StandardPfad();

        /// <summary>Pfad der Cache-Datei. Fuer Tests umleitbar.</summary>
        public static string CacheDateiPfad
        {
            get => _cacheDateiPfad;
            set => _cacheDateiPfad = value;
        }

        private static string StandardPfad()
        {
            var basis = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            return Path.Combine(basis, "EasyCashTax", "waehrungen.json");
        }

        // -----------------------------------------------------------------
        // DSGVO-Einwilligung fuer die Online-Kursabfrage
        // -----------------------------------------------------------------
        // Vor dem ERSTEN API-Zugriff muss der Anwender einwilligen (die Abfrage
        // uebertraegt Waehrung, Datum und technisch bedingt die IP an easyct.de).
        // Persistiert als Markierungsdatei -- global pro Benutzer (wie der
        // Waehrungs-Cache), nicht pro Mandant. Der Dialog selbst liegt in der
        // View-Schicht; hier nur die Persistenz + der Schutz-Guard.

        private static string _zustimmungPfad = StandardZustimmungspfad();

        /// <summary>Pfad der Einwilligungs-Markierung. Fuer Tests umleitbar.</summary>
        public static string ZustimmungDateiPfad
        {
            get => _zustimmungPfad;
            set => _zustimmungPfad = value;
        }

        private static string StandardZustimmungspfad()
        {
            var basis = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            return Path.Combine(basis, "EasyCashTax", "ect-forex-api-einwilligung.txt");
        }

        /// <summary>True, wenn der Anwender der Online-Kursabfrage zugestimmt hat.</summary>
        public static bool ApiEinwilligungErteilt
        {
            get { try { return File.Exists(ZustimmungDateiPfad); } catch { return false; } }
        }

        /// <summary>Setzt bzw. widerruft die Einwilligung (schreibt/loescht die
        /// Markierungsdatei mit Zeitstempel). <paramref name="protokollText"/> =
        /// der vollstaendige Wortlaut der angezeigten Einwilligungs-Dialogbox;
        /// wird zur Dokumentation mit in die Datei geschrieben.</summary>
        public static void SetzeApiEinwilligung(bool erteilt, string protokollText = null)
        {
            try
            {
                if (erteilt)
                {
                    var verzeichnis = Path.GetDirectoryName(ZustimmungDateiPfad);
                    if (!string.IsNullOrEmpty(verzeichnis)) Directory.CreateDirectory(verzeichnis);

                    var inhalt = new StringBuilder();
                    inhalt.Append("Einwilligung zur Online-Kursabfrage (ect-forex-API) erteilt am ")
                          .Append(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss",
                              CultureInfo.InvariantCulture))
                          .Append(".");
                    if (!string.IsNullOrWhiteSpace(protokollText))
                        inhalt.Append("\r\n\r\n")
                              .Append("Wortlaut der angezeigten Einwilligung:\r\n\r\n")
                              .Append(protokollText);

                    File.WriteAllText(ZustimmungDateiPfad, inhalt.ToString(), new UTF8Encoding(false));
                }
                else if (File.Exists(ZustimmungDateiPfad))
                {
                    File.Delete(ZustimmungDateiPfad);
                }
            }
            catch { /* Persistenz-Fehler ignorieren (Einwilligung dann fluechtig) */ }
        }

        private static JavaScriptSerializer Serializer()
            => new JavaScriptSerializer { MaxJsonLength = int.MaxValue };

        // -----------------------------------------------------------------
        // Cache lesen / schreiben
        // -----------------------------------------------------------------

        /// <summary>True, wenn eine echte (per API geholte) Cache-Datei vorliegt
        /// -- im Gegensatz zur hartkodierten Offline-Startliste.</summary>
        public static bool CacheVorhanden => File.Exists(CacheDateiPfad);

        /// <summary>Liefert die zwischengespeicherte Liste. Gibt es noch keine
        /// (oder ist sie unlesbar), kommt die hartkodierte Offline-Startliste
        /// der 30 EZB-Fiat-Waehrungen zurueck -- OHNE Netzzugriff. So triggert
        /// bloßes Durchklicken der Einstellungen KEINE API-Abfrage.</summary>
        public static List<WaehrungsEintrag> HoleGecachteListe()
        {
            try
            {
                if (File.Exists(CacheDateiPfad))
                {
                    var json = File.ReadAllText(CacheDateiPfad, new UTF8Encoding(false));
                    if (!string.IsNullOrWhiteSpace(json))
                    {
                        var liste = Serializer().Deserialize<List<WaehrungsEintrag>>(json);
                        if (liste != null && liste.Count > 0) return liste;
                    }
                }
            }
            catch { /* defekter Cache -> Startliste */ }

            return StandardFiatListe();
        }

        // Die 30 EZB-Fiat-Waehrungen als Offline-Startliste (Schreibweise EXAKT
        // wie die API, damit ein spaeteres "Liste aktualisieren" die Namen nicht
        // sichtbar aendert). Deckt sich mit FiatCodes.
        private static readonly (string Code, string Name)[] StandardFiat =
        {
            ("AUD","Australischer Dollar"),  ("BGN","Bulgarischer Lev"),
            ("BRL","Brasilianischer Real"),  ("CAD","Kanadischer Dollar"),
            ("CHF","Schweizer Franken"),     ("CNY","Chinesicher Renminbi Yuan"),
            ("CZK","Tschechische Krone"),    ("DKK","Dänische Krone"),
            ("GBP","Britisches Pfund"),      ("HKD","Hong-Kong-Dollar"),
            ("HUF","Ungarischer Forint"),    ("IDR","Indonesische Rupiah"),
            ("ILS","Israelischer Schekel"),  ("INR","Indische Rupie"),
            ("ISK","Isländische Krone"),     ("JPY","Japaischer Yen"),
            ("KRW","Südkoreanischer Won"),   ("MXN","Mexikanischer Peso"),
            ("MYR","Malaysischer Ringgit"),  ("NOK","Norwegische Krone"),
            ("NZD","Neuseeländischer Dollar"),("PHP","Philippinischer Peso"),
            ("PLN","Polnischer Złoty"),      ("RON","Rumänischer Leu"),
            ("SEK","Schwedische Krone"),     ("SGD","Singapur-Dollar"),
            ("THB","Thailändischer Baht"),   ("TRY","Türkische Lira"),
            ("USD","US-Dollar"),             ("ZAR","Südafrikanischer Rand"),
        };

        /// <summary>Die hartkodierte Startliste (30 EZB-Fiat-Waehrungen mit
        /// Namen), nach Code sortiert. Voreingestellt aktiv gemaess
        /// <see cref="StandardAktivFuerLand"/>. <paramref name="land"/> &lt; 0
        /// = aktuelles Land aus den Einstellungen.</summary>
        public static List<WaehrungsEintrag> StandardFiatListe(int land = -1)
        {
            var aktiv = StandardAktivMenge(land);
            return StandardFiat
                .OrderBy(x => x.Code, StringComparer.Ordinal)
                .Select(x => new WaehrungsEintrag
                {
                    Code  = x.Code,
                    Name  = x.Name,
                    Aktiv = aktiv.Contains(x.Code)
                })
                .ToList();
        }

        /// <summary>Schreibt die Liste als UTF-8-JSON in den Cache.</summary>
        public static void SpeichereListe(IEnumerable<WaehrungsEintrag> liste)
        {
            if (liste == null) return;
            var pfad = CacheDateiPfad;
            var verzeichnis = Path.GetDirectoryName(pfad);
            if (!string.IsNullOrEmpty(verzeichnis))
                Directory.CreateDirectory(verzeichnis);
            var json = Serializer().Serialize(liste.ToList());
            File.WriteAllText(pfad, json, new UTF8Encoding(false));
        }

        /// <summary>Die aktuell angehakten Waehrungen (fuer den
        /// Buchen-Dialog-Umrechner). Liest aus dem Cache.</summary>
        public static IReadOnlyList<WaehrungsEintrag> AktiveWaehrungen()
            => HoleGecachteListe().Where(w => w.Aktiv).ToList();

        /// <summary>Nur die Ticker-Codes der hartkodierten Fiat-Startliste
        /// (sortiert, OHNE EUR -- EUR ist die API-Basis und nicht Teil der
        /// Liste). Fuer das Waehrungs-Dropdown im "Buchungsjahr waehlen"-Dialog;
        /// rein hartkodiert, KEIN Netzzugriff.</summary>
        public static IReadOnlyList<string> StandardFiatCodes()
            => StandardFiat.Select(x => x.Code)
                .OrderBy(c => c, StringComparer.Ordinal).ToList();

        /// <summary>
        /// Baut die Auswahlliste des Umrechnen-Menues im Buchen-Dialog: die
        /// aktiven Waehrungen OHNE die Buchhaltungswaehrung selbst (X->X waere
        /// sinnlos), plus EUR als festen ersten Eintrag, wenn die
        /// Buchhaltungswaehrung nicht EUR ist (deutschsprachige Software -- Euro
        /// immer anbieten). Rein -- testbar.
        /// </summary>
        public static List<WaehrungsEintrag> MenueWaehrungen(
            IReadOnlyList<WaehrungsEintrag> aktive, string zielwaehrung)
        {
            zielwaehrung = (zielwaehrung ?? "EUR").Trim();
            var liste = (aktive ?? new List<WaehrungsEintrag>())
                .Where(w => w != null && !string.IsNullOrEmpty(w.Code)
                    && !string.Equals(w.Code, zielwaehrung, StringComparison.OrdinalIgnoreCase))
                .ToList();
            if (!string.Equals(zielwaehrung, "EUR", StringComparison.OrdinalIgnoreCase))
                liste.Insert(0, new WaehrungsEintrag { Code = "EUR", Name = "Euro", Aktiv = true });
            return liste;
        }

        // -----------------------------------------------------------------
        // API-Abruf
        // -----------------------------------------------------------------

        /// <summary>Holt die Liste von der API und verschmilzt sie mit der
        /// bestehenden Auswahl (Aktiv-Zustand bleibt erhalten). Wirft bei
        /// Netz-/Parse-Fehlern -- der Aufrufer laesst dann den Cache stehen.
        /// Speichert NICHT selbst.</summary>
        public static async Task<List<WaehrungsEintrag>> LadeVonApiAsync()
        {
            string json = await HoleApiTextAsync(ApiUrl, 20).ConfigureAwait(false);
            return ParseUndVerschmelze(json, HoleGecachteListe());
        }

        /// <summary>
        /// Rechnet <paramref name="betrag"/> der Waehrung
        /// <paramref name="quellCode"/> per Tageskurs in die Zielwaehrung
        /// <paramref name="zielCode"/> um (Buchhaltungswaehrung, meist "EUR").
        /// Ist das Ziel EUR, genuegt EINE Abfrage; sonst wird der Kurs der
        /// Zielwaehrung in einer ZWEITEN, PARALLELEN Abfrage geholt und ueber
        /// EUR umgerechnet (z.B. Buchhaltung in Schweizer Franken).
        /// Laeuft asynchron (kein UI-Block), abbrechbar; wirft bei Fehlern.
        /// </summary>
        public static async Task<decimal> RechneUmAsync(decimal betrag,
            string quellCode, string zielCode, DateTime datum,
            int timeoutSekunden = 15, CancellationToken abbruch = default)
        {
            if (betrag == 0m)
                throw new ArgumentException("Betrag ist 0.", nameof(betrag));

            string quellApi = ApiWaehrung(quellCode);
            string zielApi  = ApiWaehrung(zielCode);
            bool quellIstEuro = IstEuro(quellApi);
            bool zielIstEuro  = IstEuro(zielApi);

            // EUR ist die Basis der API und hat selbst keinen Kurs (Kurs = 1).
            if (quellIstEuro && zielIstEuro) return betrag;

            // Nur fuer Nicht-EUR-Waehrungen abfragen -- PARALLEL (Task laeuft
            // ab Erzeugung), dann per Task.WhenAll gemeinsam beobachten.
            Task<KursInfo> quellTask = quellIstEuro ? null
                : HoleKursInfoAsync(quellApi, datum, timeoutSekunden, abbruch);
            Task<KursInfo> zielTask  = zielIstEuro ? null
                : HoleKursInfoAsync(zielApi, datum, timeoutSekunden, abbruch);

            var abfragen = new List<Task>(2);
            if (quellTask != null) abfragen.Add(quellTask);
            if (zielTask  != null) abfragen.Add(zielTask);
            await Task.WhenAll(abfragen).ConfigureAwait(false);

            decimal euro = quellIstEuro ? betrag : betrag * quellTask.Result.EuroProEinheit;
            return zielIstEuro ? euro : euro / zielTask.Result.EuroProEinheit;
        }

        /// <summary>Holt Kurs + Metadaten (type) einer Waehrung zum Datum
        /// (meta=2-Abfrage). Asynchron, abbrechbar.</summary>
        public static async Task<KursInfo> HoleKursInfoAsync(string code, DateTime datum,
            int timeoutSekunden = 15, CancellationToken abbruch = default)
        {
            if (string.IsNullOrWhiteSpace(code))
                throw new ArgumentException("Kein Waehrungscode angegeben.", nameof(code));

            string url = ApiBasis
                + "?currency=" + Uri.EscapeDataString(code)
                + "&date=" + datum.ToString("yyyy-MM-dd", CultureInfo.InvariantCulture)
                + "&time=12:00&meta=2";

            string antwort = await HoleApiTextAsync(url, timeoutSekunden, abbruch).ConfigureAwait(false);
            return ParseKursInfo(antwort, code);
        }

        /// <summary>Normalisiert einen Waehrungscode fuer die API (nur trimmen).
        /// Der Schweizer Franken heisst durchgaengig CHF (ISO), kein SFR.</summary>
        private static string ApiWaehrung(string code)
            => (code ?? "").Trim();

        private static bool IstEuro(string code)
            => string.Equals(code, "EUR", StringComparison.OrdinalIgnoreCase);

        // Fiat-Waehrungen der API = die EZB-Referenzkurse (Quelle "EZB",
        // type "fiat"). Fester, stabiler Satz von 30 Codes -- dient NUR dem
        // UI-Vorfilter der Einstellungs-Seite. Die eigentliche Umrechnung
        // entscheidet die Richtung IMMER am Live-meta-type (KursInfo), bleibt
        // also korrekt, falls die EZB den Satz mal aendert.
        private static readonly HashSet<string> FiatCodes =
            new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            {
                "USD","JPY","BGN","CZK","DKK","GBP","HUF","PLN","RON","SEK",
                "CHF","ISK","NOK","TRY","AUD","BRL","CAD","CNY","HKD","IDR",
                "ILS","INR","KRW","MXN","MYR","NZD","PHP","SGD","THB","ZAR"
            };

        /// <summary>True, wenn der Code eine EZB-Fiat-Waehrung ist (fuer den
        /// Typ-Vorfilter der Einstellungs-Seite). Alles andere gilt als
        /// Krypto/Token.</summary>
        public static bool IstFiatWaehrung(string code)
            => !string.IsNullOrEmpty(code) && FiatCodes.Contains(ApiWaehrung(code));

        /// <summary>
        /// Parst die meta=2-Antwort ([{"currency":..,"price":"..","meta":{"type":".."}}]).
        /// Extrahiert price und type gezielt per String-Suche (statt vollem
        /// JSON-Parser), damit ungueltige Escapes im Namensfeld (siehe
        /// <see cref="ParseUndVerschmelze"/>) nicht stoeren. Rein -- testbar.
        /// </summary>
        public static KursInfo ParseKursInfo(string antwort, string code)
        {
            if (string.IsNullOrWhiteSpace(antwort))
                throw new FormatException("Leere Kursantwort.");

            string preisStr = ExtrahiereJsonWert(antwort, "price");
            if (string.IsNullOrEmpty(preisStr))
            {
                string fehler = ExtrahiereJsonWert(antwort, "error");
                throw new FormatException(fehler != null
                    ? "Kein Kurs fuer " + code + " (" + fehler + ")."
                    : "Kein Kurs fuer " + code + " erhalten.");
            }
            if (!decimal.TryParse(preisStr, NumberStyles.Float,
                    CultureInfo.InvariantCulture, out decimal preis) || preis <= 0m)
                throw new FormatException("Kurs unlesbar fuer " + code + ": '" + preisStr + "'.");

            return new KursInfo
            {
                Code  = code,
                Preis = preis,
                Typ   = ExtrahiereJsonWert(antwort, "type") ?? ""
            };
        }

        /// <summary>Liefert den String-Wert zu "schluessel":"wert" (erstes
        /// Vorkommen). Unbekannte Escape-Sequenzen werden literal genommen.
        /// null, wenn der Schluessel fehlt.</summary>
        private static string ExtrahiereJsonWert(string json, string schluessel)
        {
            string marker = "\"" + schluessel + "\"";
            int i = json.IndexOf(marker, StringComparison.Ordinal);
            if (i < 0) return null;
            i += marker.Length;
            while (i < json.Length && (json[i] == ':' || char.IsWhiteSpace(json[i]))) i++;
            if (i >= json.Length || json[i] != '"') return null;
            i++;
            var sb = new StringBuilder();
            while (i < json.Length)
            {
                char c = json[i++];
                if (c == '\\' && i < json.Length) { sb.Append(json[i++]); continue; }
                if (c == '"') break;
                sb.Append(c);
            }
            return sb.ToString();
        }

        // -----------------------------------------------------------------
        // Umrechnungs-Dokumentation im Buchungstext (bauen + wiedererkennen)
        // -----------------------------------------------------------------

        /// <summary>Baut den Dokumentations-Text der Umrechnung fuer das
        /// Beschreibungsfeld. Format bewusst mit <see cref="OhneUmrechnungsanhang"/>
        /// abgestimmt (Wiedererkennung am "(Kurs vom ...").</summary>
        public static string BaueUmrechnungsText(string quellBetrag, string quellCode,
            string zielBetrag, string zielCode, DateTime datum, decimal kursProEinheit)
        {
            return quellBetrag + " " + quellCode + " = " + zielBetrag + " " + zielCode
                + " (Kurs vom " + datum.ToString("dd.MM.yyyy", CultureInfo.CurrentCulture)
                + ": 1 " + quellCode + " = "
                + kursProEinheit.ToString("0.######", CultureInfo.CurrentCulture)
                + " " + zielCode + ")";
        }

        // Erkennt einen von BaueUmrechnungsText erzeugten Anhang am ENDE eines
        // Textes. Verankert am distinktiven "(Kurs vom TT.MM.JJJJ:" -- das
        // kommt in normalem Buchungstext praktisch nicht vor. Optionaler
        // " -- "-Trenner wird mitentfernt.
        private static readonly Regex _umrechnungAmEnde = new Regex(
            @"\s*(?:--\s*)?\d[\d.,]*\s+\S+\s*=\s*\d[\d.,]*\s+\S+\s*\(Kurs vom \d{2}\.\d{2}\.\d{4}:[^)]*\)\s*$",
            RegexOptions.Compiled);

        /// <summary>Entfernt am Ende angehaengte Umrechnungstexte (auch mehrere,
        /// falls durch fruehere Versionen entstanden), damit erneutes Umrechnen
        /// den vorhandenen Text ERSETZT statt anzuhaengen.</summary>
        public static string OhneUmrechnungsanhang(string beschreibung)
        {
            if (string.IsNullOrEmpty(beschreibung)) return beschreibung ?? "";
            string vorher;
            do
            {
                vorher = beschreibung;
                beschreibung = _umrechnungAmEnde.Replace(beschreibung, "");
            }
            while (beschreibung != vorher);
            return beschreibung.TrimEnd();
        }

        /// <summary>
        /// Parst die oneshot-Kursantwort. Fuer eine EINZELNE Waehrung liefert
        /// die API einen nackten JSON-String, z.B. "1.133900000000"; bei einem
        /// Fehler den String "error" bzw. (bei unbekannter Waehrung) ein
        /// Array mit meta.error. Rein (kein I/O) -- testbar.
        /// </summary>
        public static decimal ParseKurs(string antwort)
        {
            if (string.IsNullOrWhiteSpace(antwort))
                throw new FormatException("Leere Kursantwort.");

            string t = antwort.Trim();
            // Fehlerform bei unbekannter Waehrung: [{"currency":..,"meta":{"error":..}}]
            if (t.StartsWith("[", StringComparison.Ordinal))
                throw new FormatException("Fuer diese Waehrung/dieses Datum ist kein Kurs verfuegbar.");

            // oneshot: nackter JSON-String -> Inhalt zwischen erstem und letztem "
            int a = t.IndexOf('"');
            int b = t.LastIndexOf('"');
            string inhalt = (a >= 0 && b > a) ? t.Substring(a + 1, b - a - 1) : t;
            inhalt = inhalt.Trim();

            if (inhalt.Length == 0 || inhalt.Equals("error", StringComparison.OrdinalIgnoreCase))
                throw new FormatException("Der Waehrungsticker-Server meldete keinen Kurs.");

            if (!decimal.TryParse(inhalt, NumberStyles.Float,
                    CultureInfo.InvariantCulture, out decimal kurs))
                throw new FormatException("Kurs unlesbar: '" + inhalt + "'.");
            if (kurs <= 0m)
                throw new FormatException("Ungueltiger Kurs: " + inhalt);

            return kurs;
        }

        private static async Task<string> HoleApiTextAsync(string url, int timeoutSekunden,
            CancellationToken abbruch = default)
        {
            // Schutz-Guard: ohne Einwilligung KEIN Netzzugriff (Defense in Depth
            // -- die UI holt die Einwilligung vorher ein und setzt sie).
            if (!ApiEinwilligungErteilt)
                throw new InvalidOperationException(
                    "Fuer die Online-Kursabfrage liegt keine Einwilligung vor.");

            try
            {
                // .NET 4.8 verwendet i.d.R. schon TLS 1.2 -- defensiv setzen.
                ServicePointManager.SecurityProtocol |= SecurityProtocolType.Tls12;
            }
            catch { /* aeltere Enum-Werte nicht verfuegbar: ignorieren */ }

            using (var client = new HttpClient { Timeout = TimeSpan.FromSeconds(timeoutSekunden) })
            using (var antwort = await client.GetAsync(url, abbruch).ConfigureAwait(false))
            {
                antwort.EnsureSuccessStatusCode();
                var bytes = await antwort.Content.ReadAsByteArrayAsync().ConfigureAwait(false);
                return Encoding.UTF8.GetString(bytes);
            }
        }

        /// <summary>
        /// Parst die API-Antwort ({ "Code": "Name", ... }) und verschmilzt
        /// sie mit der bestehenden Auswahl: der Aktiv-Zustand jeder bereits
        /// bekannten Waehrung bleibt erhalten, neue Waehrungen kommen
        /// abgehakt hinzu. Ist der bestehende Cache leer (Erstbefuellung),
        /// werden die StandardAktiv-Waehrungen vorangehakt. Sortiert nach
        /// Ticker-Code. Rein (kein I/O) -- deshalb unit-testbar.
        /// </summary>
        public static List<WaehrungsEintrag> ParseUndVerschmelze(
            string json, IReadOnlyList<WaehrungsEintrag> bestehend, int land = -1)
        {
            // Bewusst NICHT JavaScriptSerializer: die ect-forex-API liefert in
            // Einzelfaellen technisch ungueltiges JSON (ein Waehrungsname
            // "W / I / B W \ O \ B" enthaelt ungeschuetzte Backslashes). Ein
            // strikter Parser bricht daran ab -- der tolerante Parser unten
            // uebernimmt unbekannte Escape-Sequenzen literal.
            var paare = ParseFlachesObjekt(json);
            if (paare.Count == 0)
                throw new FormatException("Leere Waehrungsliste erhalten.");

            var vorherAktiv = new Dictionary<string, bool>(StringComparer.OrdinalIgnoreCase);
            if (bestehend != null)
                foreach (var e in bestehend)
                    if (!string.IsNullOrEmpty(e.Code))
                        vorherAktiv[e.Code] = e.Aktiv;
            bool erstbefuellung = vorherAktiv.Count == 0;
            var standardAktiv = erstbefuellung ? StandardAktivMenge(land) : null;

            var gesehen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var ergebnis = new List<WaehrungsEintrag>(paare.Count);
            foreach (var kv in paare)
            {
                if (string.IsNullOrEmpty(kv.Key) || !gesehen.Add(kv.Key)) continue;
                bool aktiv;
                if (vorherAktiv.TryGetValue(kv.Key, out var a))
                    aktiv = a;
                else
                    aktiv = erstbefuellung && standardAktiv.Contains(kv.Key);
                ergebnis.Add(new WaehrungsEintrag
                {
                    Code  = kv.Key,
                    Name  = kv.Value ?? "",
                    Aktiv = aktiv
                });
            }
            ergebnis.Sort((x, y) => string.CompareOrdinal(x.Code, y.Code));
            return ergebnis;
        }

        // -----------------------------------------------------------------
        // Toleranter Parser fuer ein FLACHES JSON-Objekt { "s":"s", ... }.
        // Nur fuer die API-Antwort gedacht (Werte sind immer Zeichenketten).
        // Unbekannte Escape-Sequenzen werden literal uebernommen.
        // -----------------------------------------------------------------

        private static List<KeyValuePair<string, string>> ParseFlachesObjekt(string json)
        {
            var ergebnis = new List<KeyValuePair<string, string>>();
            if (string.IsNullOrEmpty(json))
                throw new FormatException("Kein JSON-Objekt.");

            int i = 0, n = json.Length;
            UeberspringeLeerraum(json, ref i);
            if (i >= n || json[i] != '{')
                throw new FormatException("Kein JSON-Objekt.");
            i++;   // '{'

            UeberspringeLeerraum(json, ref i);
            if (i < n && json[i] == '}') return ergebnis;   // {} -> leer

            while (i < n)
            {
                UeberspringeLeerraum(json, ref i);
                if (i >= n || json[i] != '"')
                    throw new FormatException("Schluessel erwartet.");
                string key = LiesZeichenkette(json, ref i);

                UeberspringeLeerraum(json, ref i);
                if (i >= n || json[i] != ':')
                    throw new FormatException("':' erwartet.");
                i++;   // ':'

                UeberspringeLeerraum(json, ref i);
                if (i >= n || json[i] != '"')
                    throw new FormatException("Zeichenketten-Wert erwartet.");
                string wert = LiesZeichenkette(json, ref i);

                ergebnis.Add(new KeyValuePair<string, string>(key, wert));

                UeberspringeLeerraum(json, ref i);
                if (i < n && json[i] == ',') { i++; continue; }
                break;   // '}' oder unerwartetes Zeichen -> Ende (tolerant)
            }
            return ergebnis;
        }

        private static void UeberspringeLeerraum(string s, ref int i)
        {
            while (i < s.Length && char.IsWhiteSpace(s[i])) i++;
        }

        private static string LiesZeichenkette(string s, ref int i)
        {
            i++;   // oeffnendes '"'
            var sb = new StringBuilder();
            while (i < s.Length)
            {
                char c = s[i++];
                if (c == '"') return sb.ToString();
                if (c != '\\') { sb.Append(c); continue; }

                if (i >= s.Length) { sb.Append('\\'); break; }
                char e = s[i++];
                switch (e)
                {
                    case '"':  sb.Append('"');  break;
                    case '\\': sb.Append('\\'); break;
                    case '/':  sb.Append('/');  break;
                    case 'b':  sb.Append('\b'); break;
                    case 'f':  sb.Append('\f'); break;
                    case 'n':  sb.Append('\n'); break;
                    case 'r':  sb.Append('\r'); break;
                    case 't':  sb.Append('\t'); break;
                    case 'u':
                        if (i + 4 <= s.Length && int.TryParse(
                                s.Substring(i, 4), NumberStyles.HexNumber,
                                CultureInfo.InvariantCulture, out int code))
                        {
                            sb.Append((char)code);
                            i += 4;
                        }
                        else sb.Append('\\').Append('u');
                        break;
                    default:
                        // Tolerant: ungueltige Escape-Sequenz literal uebernehmen.
                        sb.Append('\\').Append(e);
                        break;
                }
            }
            return sb.ToString();   // unterminiert -> nehmen, was da ist
        }
    }
}
