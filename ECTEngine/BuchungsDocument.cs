// BuchungsDocument.cs — Zentrales Buchungsdokument
//
// Ersetzt die Geschäftslogik aus: CEasyCashDoc : CDocument (EasyCashDoc.h/cpp)
// NICHT von CDocument abgeleitet — die MFC-Vererbung bleibt in der Bridge.
//
// Die beiden separaten Linked Lists (CBuchung *Einnahmen, *Ausgaben) werden
// zu einer einzigen List<Buchung> zusammengeführt. Die Unterscheidung erfolgt
// über Buchung.Art.
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

using System;
using System.Collections.Generic;
using System.Linq;

namespace ECTEngine
{
    /// <summary>
    /// Zentrales Buchungsdokument — enthält alle Buchungen, Dauerbuchungen
    /// und die zugehörige Geschäftslogik.
    ///
    /// CEasyCashDoc-Feldmapping:
    ///
    ///   C++ Member                              → C# Property
    ///   CBuchung *Einnahmen, *Ausgaben           → Buchungen (List, Art-Property)
    ///   CDauerbuchung *Dauerbuchungen             → Dauerbuchungen (List)
    ///   int Buchungszaehler                       → Buchungszaehler
    ///   int nLaufende..FuerEinnahmen/Ausgaben     → LaufendeBelegnr...
    ///   int nLaufende..FuerBank/Kasse             → LaufendeBelegnr...
    ///   int nJahr                                 → Jahr
    ///   CString csWaehrung                        → Waehrung
    ///   CString csUrspruenglicheWaehrung          → UrspruenglicheWaehrung
    ///   int AbschreibungGenauigkeit               → GlobaleAfaGenauigkeit
    ///   CString Erweiterung                       → Erweiterungen
    ///   int Version                               → DokumentVersion
    ///   int nNachfrageIntervall                    → BackupNachfrageIntervallTage
    ///   CTime ctNachfrageTermin                   → BackupNachfrageTermin
    /// </summary>
    public class BuchungsDocument
    {
        // ──────────────────────────────────────────────
        // Buchungslisten (ersetzen die Linked Lists)
        // ──────────────────────────────────────────────

        /// <summary>
        /// Alle Buchungen (Einnahmen und Ausgaben in einer Liste).
        /// Im Original: zwei separate Linked Lists (Einnahmen, Ausgaben).
        /// </summary>
        public List<Buchung> Buchungen { get; } = new List<Buchung>();

        /// <summary>Alle Dauerbuchungen.</summary>
        public List<Dauerbuchung> Dauerbuchungen { get; } = new List<Dauerbuchung>();

        // ──────────────────────────────────────────────
        // Zähler und Belegnummern
        // ──────────────────────────────────────────────

        /// <summary>Gesamtzähler aller jemals erstellten Buchungen.</summary>
        public int Buchungszaehler { get; set; }

        public int LaufendeBelegnrEinnahmen { get; set; } = 1;
        public int LaufendeBelegnrAusgaben { get; set; } = 1;
        public int LaufendeBelegnrBank { get; set; } = 1;     // seit Doc-Version 8
        public int LaufendeBelegnrKasse { get; set; } = 1;    // seit Doc-Version 8

        // ──────────────────────────────────────────────
        // Dokumentmetadaten
        // ──────────────────────────────────────────────

        /// <summary>Buchungsjahr (int nJahr). Seit Version 4.</summary>
        public int Jahr { get; set; }

        /// <summary>Aktuelle Währung (CString csWaehrung). Seit Version 6.</summary>
        public string Waehrung { get; set; } = "EUR";

        /// <summary>Ursprüngliche Währung (CString csUrspruenglicheWaehrung).</summary>
        public string UrspruenglicheWaehrung { get; set; } = "";

        /// <summary>Globale AfA-Genauigkeit (int AbschreibungGenauigkeit). Seit Version 7.</summary>
        public AfaGenauigkeit GlobaleAfaGenauigkeit { get; set; } = AfaGenauigkeit.Monatsgenau;

        /// <summary>Dokumentformat-Version (aktuell 13).</summary>
        public int DokumentVersion { get; set; } = 13;

        // ──────────────────────────────────────────────
        // Konten (aus INI-Datei, nicht aus .eca)
        // Die Bridge befüllt diese aus der easyct.ini
        // ──────────────────────────────────────────────

        /// <summary>
        /// Einnahmen-Kontennamen (max. 100, aus m_csEinnahmenKonten).
        /// Werden in der Bridge aus der INI-Datei geladen.
        /// </summary>
        public string[] EinnahmenKonten { get; set; } = new string[100];

        /// <summary>Ausgaben-Kontennamen.</summary>
        public string[] AusgabenKonten { get; set; } = new string[100];

        // ──────────────────────────────────────────────
        // Erweiterungen (Dokument-Ebene)
        // ──────────────────────────────────────────────

        /// <summary>
        /// Plugin-Erweiterungsdaten auf Dokumentebene.
        /// Seit Version 7. Pipe-Format wie bei Buchungen.
        /// </summary>
        public ErweiterungStore Erweiterungen { get; set; } = new ErweiterungStore();

        // ──────────────────────────────────────────────
        // Backup-Einstellungen (seit Version 13)
        // ──────────────────────────────────────────────

        /// <summary>Backup-Nachfrage-Intervall in Tagen.</summary>
        public int BackupNachfrageIntervallTage { get; set; } = 7;

        /// <summary>Nächster Backup-Nachfrage-Termin.</summary>
        public DateTime BackupNachfrageTermin { get; set; }

        // ──────────────────────────────────────────────
        // Komfort-Zugriffe
        // ──────────────────────────────────────────────

        /// <summary>Alle Einnahmen-Buchungen.</summary>
        public IEnumerable<Buchung> Einnahmen =>
            Buchungen.Where(b => b.Art == Buchungsart.Einnahme);

        /// <summary>Alle Ausgaben-Buchungen.</summary>
        public IEnumerable<Buchung> Ausgaben =>
            Buchungen.Where(b => b.Art == Buchungsart.Ausgabe);

        // ──────────────────────────────────────────────
        // Summenberechnung
        // Reimplementiert: CEasyCashDoc::EinnahmenSumme/AusgabenSumme
        // ──────────────────────────────────────────────

        /// <summary>
        /// Brutto-Summe der Einnahmen in Cent.
        /// Reimplementiert CEasyCashDoc::EinnahmenSumme(int MonatsFilter, LPCTSTR KontoFilter).
        /// </summary>
        /// <param name="monatsFilter">0 = alle Monate, 1-12 = nur dieser Monat.</param>
        /// <param name="kontoFilter">Leer = alle Konten, sonst nur dieses Konto.</param>
        public int EinnahmenSumme(int monatsFilter = 0, string kontoFilter = "")
            => Summe(Buchungsart.Einnahme, monatsFilter, kontoFilter, brutto: true);

        /// <summary>Brutto-Summe der Ausgaben in Cent.</summary>
        public int AusgabenSumme(int monatsFilter = 0, string kontoFilter = "")
            => Summe(Buchungsart.Ausgabe, monatsFilter, kontoFilter, brutto: true);

        /// <summary>Netto-Summe der Einnahmen in Cent.</summary>
        public int EinnahmenSummeNetto(int monatsFilter = 0, string kontoFilter = "")
            => Summe(Buchungsart.Einnahme, monatsFilter, kontoFilter, brutto: false);

        /// <summary>Netto-Summe der Ausgaben in Cent.</summary>
        public int AusgabenSummeNetto(int monatsFilter = 0, string kontoFilter = "")
            => Summe(Buchungsart.Ausgabe, monatsFilter, kontoFilter, brutto: false);

        /// <summary>Saldo (Einnahmen - Ausgaben) brutto in Cent.</summary>
        public int Saldo(int monatsFilter = 0) =>
            EinnahmenSumme(monatsFilter) - AusgabenSumme(monatsFilter);

        /// <summary>Saldo netto in Cent.</summary>
        public int SaldoNetto(int monatsFilter = 0) =>
            EinnahmenSummeNetto(monatsFilter) - AusgabenSummeNetto(monatsFilter);

        private int Summe(Buchungsart art, int monatsFilter, string kontoFilter, bool brutto)
        {
            var query = Buchungen.Where(b => b.Art == art);

            if (monatsFilter > 0 && monatsFilter <= 12)
                query = query.Where(b => b.Datum.Month == monatsFilter);

            if (!string.IsNullOrEmpty(kontoFilter))
                query = query.Where(b => b.Konto == kontoFilter);

            if (brutto)
                return query.Sum(b => b.BruttoBetrag.InCent);
            else
                return (int)query.Sum(b => b.BruttoBetrag.NettoInCent);
        }

        // ──────────────────────────────────────────────
        // Sortierung
        // Reimplementiert: CEasyCashDoc::Sort()
        // ──────────────────────────────────────────────

        /// <summary>Sortiert alle Buchungen nach Datum.</summary>
        public void Sort()
        {
            Buchungen.Sort((a, b) => a.Datum.CompareTo(b.Datum));
        }

        // ──────────────────────────────────────────────
        // Buchungszähler
        // ──────────────────────────────────────────────

        /// <summary>
        /// Inkrementiert den Buchungszähler und gibt den neuen Wert zurück.
        /// Reimplementiert CEasyCashDoc::InkrementBuchungszaehler().
        /// </summary>
        public int InkrementBuchungszaehler() => ++Buchungszaehler;

        /// <summary>Gibt die nächste Belegnummer für die Art zurück und inkrementiert.</summary>
        public string NaechsteBelegnummer(Buchungsart art)
        {
            if (art == Buchungsart.Einnahme)
                return (LaufendeBelegnrEinnahmen++).ToString();
            else
                return (LaufendeBelegnrAusgaben++).ToString();
        }

        // ──────────────────────────────────────────────
        // Dauerbuchungen ausführen
        // ──────────────────────────────────────────────

        /// <summary>
        /// Führt alle fälligen Dauerbuchungen bis zum Stichtag aus.
        /// Gibt die neu erzeugten Buchungen zurück.
        /// </summary>
        public List<Buchung> FuehreDauerbuchungenAus(DateTime stichtag)
        {
            var neueBuchungen = new List<Buchung>();

            foreach (var db in Dauerbuchungen)
            {
                foreach (var termin in db.FaelligeTermine(stichtag))
                {
                    var buchung = db.ErzeugeBuchung(termin);
                    Buchungen.Add(buchung);
                    neueBuchungen.Add(buchung);

                    // Belegnummer-Auto-Inkrement
                    if (buchung.Art == Buchungsart.Einnahme)
                        LaufendeBelegnrEinnahmen++;
                    else
                        LaufendeBelegnrAusgaben++;
                }

                // AusgefuehrtBis aktualisieren
                if (neueBuchungen.Count > 0)
                    db.AusgefuehrtBis = stichtag;
            }

            Sort();
            return neueBuchungen;
        }

        /// <summary>
        /// Prüft, ob alle Dauerbuchungen bis Jahresende ausgeführt wurden.
        /// Gibt Warnungen für nicht vollständig ausgeführte Dauerbuchungen zurück.
        /// </summary>
        public List<string> PruefeDauerbuchungenVollstaendig()
        {
            var warnungen = new List<string>();
            var jahresende = new DateTime(Jahr, 12, 31);

            foreach (var db in Dauerbuchungen)
            {
                if (db.AusgefuehrtBis < jahresende && db.BisDatum >= jahresende)
                {
                    warnungen.Add($"Dauerbuchung '{db.Beschreibung}' wurde noch nicht " +
                                  $"bis zum Jahresende ausgeführt (zuletzt: {db.AusgefuehrtBis:d}).");
                }
            }

            return warnungen;
        }

        // ──────────────────────────────────────────────
        // Jahreswechsel
        // Reimplementiert: CEasyCashDoc::Jahreswechsel()
        // ──────────────────────────────────────────────

        /// <summary>
        /// Erstellt ein neues Dokument für das Folgejahr.
        /// Übernimmt AfA-Buchungen mit fortgesetzter Abschreibung
        /// und kopiert alle Dauerbuchungen.
        /// Reimplementiert CEasyCashDoc::Jahreswechsel().
        /// </summary>
        public BuchungsDocument Jahreswechsel()
        {
            var neuesDoc = new BuchungsDocument
            {
                Jahr = Jahr + 1,
                Buchungszaehler = Buchungszaehler,
                Waehrung = Waehrung,
                UrspruenglicheWaehrung = UrspruenglicheWaehrung,
                GlobaleAfaGenauigkeit = GlobaleAfaGenauigkeit,
                Erweiterungen = Erweiterungen.Clone(),
                LaufendeBelegnrEinnahmen = 1,
                LaufendeBelegnrAusgaben = 1,
                LaufendeBelegnrBank = 1,
                LaufendeBelegnrKasse = 1
            };

            string slashAltesJahr = $"/{Jahr:D4}";

            // AfA-Ausgaben ins neue Jahr übernehmen
            foreach (var b in Ausgaben)
            {
                if (b.AfaJahre <= 1) continue;

                long afaBetragDiesesJahr = AfaCalculator.GetBuchungsjahrNetto(b, GlobaleAfaGenauigkeit);
                if (b.AfaRestwertCent <= afaBetragDiesesJahr) continue;

                var kopie = b.Clone();
                kopie.Art = Buchungsart.Ausgabe;

                // Belegnummer mit Jahreskennung erweitern
                if (!string.IsNullOrEmpty(kopie.Belegnummer) &&
                    !kopie.Belegnummer.Contains("/20"))
                {
                    kopie.Belegnummer += slashAltesJahr;
                }

                // Restwert und Nummer fortschreiben
                kopie.AfaRestwertCent -= (int)afaBetragDiesesJahr;
                kopie.AfaNr++;

                // Datum ins neue Jahr setzen
                kopie.Datum = new DateTime(
                    kopie.Datum.Year + 1,
                    kopie.Datum.Month,
                    kopie.Datum.Day);

                // Degressive AfA: prüfen ob Wechsel auf linear sinnvoll
                if (kopie.AfaDegressiv && !(kopie.AfaSatz == 75 &&
                    kopie.AfaGenauigkeit == AfaGenauigkeit.Ganzjahr))
                {
                    long rateDegressiv = AfaCalculator.GetBuchungsjahrNetto(
                        kopie, GlobaleAfaGenauigkeit);

                    kopie.AfaDegressiv = false; // temporär für Test
                    long rateLinear = AfaCalculator.GetBuchungsjahrNetto(
                        kopie, GlobaleAfaGenauigkeit);

                    // Nur zurück auf degressiv, wenn degressiv noch höher
                    if (rateDegressiv >= rateLinear)
                        kopie.AfaDegressiv = true;
                    // sonst bleibt linear (automatischer Wechsel)
                }

                neuesDoc.Buchungen.Add(kopie);
            }

            // Alle Dauerbuchungen komplett übernehmen
            foreach (var db in Dauerbuchungen)
            {
                neuesDoc.Dauerbuchungen.Add(db.Clone());
            }

            return neuesDoc;
        }

        // ──────────────────────────────────────────────
        // Hilfsmethoden
        // ──────────────────────────────────────────────

        /// <summary>
        /// Prüft ob eine Buchung zu den Einnahmen gehört.
        /// Reimplementiert CEasyCashDoc::BuchungIstEinnahme().
        /// Im Original: lineare Suche durch die Einnahmen-Linked-List.
        /// </summary>
        public bool BuchungIstEinnahme(Buchung b) =>
            b.Art == Buchungsart.Einnahme;

        /// <summary>
        /// Prüft ob eine Buchung zu den Ausgaben gehört.
        /// Reimplementiert CEasyCashDoc::BuchungIstAusgabe().
        /// </summary>
        public bool BuchungIstAusgabe(Buchung b) =>
            b.Art == Buchungsart.Ausgabe;
    }
}
