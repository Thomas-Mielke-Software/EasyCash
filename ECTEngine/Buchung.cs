// Buchung.cs — Einzelbuchung im Journal
//
// Ersetzt: CBuchung : CBetrag : CObject (EasyCashDoc.h)
// Die Linked-List-Verkettung (CBuchung *next) wird aufgelöst:
// BuchungsDocument.Buchungen ist eine List<Buchung>.
// Die Zugehörigkeit zu Einnahmen/Ausgaben wird durch Buchung.Art explizit.
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

using System;

namespace ECTEngine
{
    /// <summary>
    /// Einzelne Buchung im Journal.
    ///
    /// Feld-für-Feld-Mapping zur C++-Klasse CBuchung:
    ///
    ///   C++ (CBetrag)              → C# (Betrag-Struct)
    ///   int Betrag (Cent)          → BruttoBetrag.InCent
    ///   int MWSt (Promille)        → BruttoBetrag.MwstPromille
    ///
    ///   C++ (CBuchung)             → C# Property
    ///   CString Beschreibung       → Beschreibung
    ///   CTime Datum                → Datum
    ///   CString Konto              → Konto
    ///   CString Belegnummer        → Belegnummer               (seit v7)
    ///   int AbschreibungNr         → AfaNr
    ///   int AbschreibungJahre      → AfaJahre
    ///   int AbschreibungRestwert   → AfaRestwertCent           (seit v7)
    ///   BOOL AbschreibungDegressiv → AfaDegressiv              (seit v7)
    ///   int AbschreibungSatz       → AfaSatz                   (seit v7)
    ///   int AbschreibungGenauigkeit→ AfaGenauigkeit            (seit v10)
    ///   CString Bestandskonto      → Bestandskonto             (seit v11)
    ///   CString Betrieb            → Betrieb                   (seit v11)
    ///   CString Erweiterung        → Erweiterungen (parsed)    (seit v7)
    ///   CBuchung *next             → [entfällt – List statt Linked List]
    ///   [implizit via Liste]       → Art (Einnahme/Ausgabe)
    /// </summary>
    public class Buchung
    {
        // ──────────────────────────────────────────────
        // Betrag (aus CBetrag-Basisklasse)
        // ──────────────────────────────────────────────

        /// <summary>Bruttobetrag inkl. MWSt-Information.</summary>
        public Betrag BruttoBetrag { get; set; }

        // ──────────────────────────────────────────────
        // Buchungsart (war implizit durch Listenzugehörigkeit)
        // ──────────────────────────────────────────────

        /// <summary>
        /// Einnahme oder Ausgabe. Im Original war dies implizit durch die
        /// Zugehörigkeit zur Einnahmen- bzw. Ausgaben-Linked-List bestimmt.
        /// </summary>
        public Buchungsart Art { get; set; }

        // ──────────────────────────────────────────────
        // Kernfelder
        // ──────────────────────────────────────────────

        /// <summary>Buchungsdatum (CTime Datum).</summary>
        public DateTime Datum { get; set; }

        /// <summary>Buchungstext (CString Beschreibung).</summary>
        public string Beschreibung { get; set; } = "";

        /// <summary>E/Ü-Kontoname (CString Konto). Seit Version 3.</summary>
        public string Konto { get; set; } = "";

        /// <summary>Belegnummer (CString Belegnummer). Seit Version 7.</summary>
        public string Belegnummer { get; set; } = "";

        // ──────────────────────────────────────────────
        // Abschreibung (AfA)
        // ──────────────────────────────────────────────

        /// <summary>Laufende AfA-Nummer (int AbschreibungNr). Default: 1.</summary>
        public int AfaNr { get; set; } = 1;

        /// <summary>AfA-Dauer in Jahren (int AbschreibungJahre). Default: 1 (keine AfA).</summary>
        public int AfaJahre { get; set; } = 1;

        /// <summary>
        /// Verbleibender Buchwert in Cent (int AbschreibungRestwert).
        /// Seit Version 7. Für Versionen &lt;7 wird der Restwert aus
        /// Netto/Dauer/Nr berechnet.
        /// </summary>
        public int AfaRestwertCent { get; set; }

        /// <summary>Degressive AfA aktiv (BOOL AbschreibungDegressiv). Seit Version 7.</summary>
        public bool AfaDegressiv { get; set; }

        /// <summary>Degressions-Prozentsatz (int AbschreibungSatz). Seit Version 7.</summary>
        public int AfaSatz { get; set; }

        /// <summary>
        /// AfA-Genauigkeit für diese Buchung (int AbschreibungGenauigkeit).
        /// Seit Version 10. Default: EntsprechendEinstellungen (3).
        /// </summary>
        public AfaGenauigkeit AfaGenauigkeit { get; set; } = AfaGenauigkeit.EntsprechendEinstellungen;

        // ──────────────────────────────────────────────
        // Zuordnungen (seit v11)
        // ──────────────────────────────────────────────

        /// <summary>Bestandskonto (CString Bestandskonto). Seit Version 11.</summary>
        public string Bestandskonto { get; set; } = "";

        /// <summary>Betriebszuordnung (CString Betrieb). Seit Version 11.</summary>
        public string Betrieb { get; set; } = "";

        // ──────────────────────────────────────────────
        // Erweiterungen (seit v7)
        // ──────────────────────────────────────────────

        /// <summary>
        /// Plugin-Erweiterungsdaten. Ersetzt CString Erweiterung (Pipe-Format).
        /// Im Original: "DLLName|Key=Val||..."
        /// </summary>
        public ErweiterungStore Erweiterungen { get; set; } = new ErweiterungStore();

        // ──────────────────────────────────────────────
        // Berechnete Properties
        // ──────────────────────────────────────────────

        /// <summary>Nettobetrag (delegiert an Betrag.NettoWert).</summary>
        public decimal NettoWert => BruttoBetrag.NettoWert;

        /// <summary>MWSt-Betrag.</summary>
        public decimal MwstBetrag => BruttoBetrag.MwstBetrag;

        /// <summary>
        /// True wenn das Konto mit '/' beginnt (neutrales/durchlaufendes Konto).
        /// Solche Konten erscheinen nicht in der EÜR.
        /// </summary>
        public bool IstNeutralesKonto =>
            !string.IsNullOrEmpty(Konto) && Konto.StartsWith("/");

        /// <summary>True wenn die Buchung eine AfA-Buchung ist (Dauer > 1 Jahr).</summary>
        public bool HatAfA => AfaJahre > 1;

        /// <summary>True wenn dies eine Split-Buchung ist.</summary>
        public bool IstSplitBuchung =>
            Erweiterungen.Hat("EasyCash", "SplitGegenbuchungOhneVorsteuerabzug");

        /// <summary>Restwert als decimal (Convenience).</summary>
        public decimal AfaRestwert => AfaRestwertCent / 100m;

        // ──────────────────────────────────────────────
        // Kopie
        // ──────────────────────────────────────────────

        /// <summary>
        /// Erstellt eine tiefe Kopie der Buchung.
        /// Entspricht CBuchung::operator= (ohne next-Zeiger).
        /// </summary>
        public Buchung Clone()
        {
            return new Buchung
            {
                BruttoBetrag = BruttoBetrag,
                Art = Art,
                Datum = Datum,
                Beschreibung = Beschreibung,
                Konto = Konto,
                Belegnummer = Belegnummer,
                AfaNr = AfaNr,
                AfaJahre = AfaJahre,
                AfaRestwertCent = AfaRestwertCent,
                AfaDegressiv = AfaDegressiv,
                AfaSatz = AfaSatz,
                AfaGenauigkeit = AfaGenauigkeit,
                Bestandskonto = Bestandskonto,
                Betrieb = Betrieb,
                Erweiterungen = Erweiterungen.Clone()
            };
        }
    }
}
