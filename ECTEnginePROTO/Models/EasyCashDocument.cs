using System;
using System.Collections.Generic;
using System.Linq;
using ECTEngine.Calculations;
using ECTEngine.Helpers;

namespace ECTEngine.Models
{
    /// <summary>
    /// Hauptdokument-Klasse für EasyCash Buchungsdaten
    /// Verwaltet Einnahmen, Ausgaben und Dauerbuchungen für ein Geschäftsjahr
    /// </summary>
    public class EasyCashDocument
    {
        private const int DOCUMENT_VERSION = 13;
        private const string MAGIC_KEY = "ECDo";
        private readonly BackupManager _backupManager = new();
        private readonly SortingEngine _sortingEngine = new();
        private readonly CurrencyConverter _currencyConverter = new();

        public LinkedList<Buchung> Einnahmen { get; } = new();
        public LinkedList<Buchung> Ausgaben { get; } = new();
        public LinkedList<Dauerbuchung> Dauerbuchungen { get; } = new();

        public int Buchungszaehler { get; set; }
        public int Jahr { get; set; }
        public string Waehrung { get; set; } = "EUR";
        public string UrspruenglicheWaehrung { get; set; } = "EUR";
        public string Erweiterung { get; set; } = "";
        
        public AbschreibungsGenauigkeit AbschreibungGenauigkeit { get; set; } = 
            AbschreibungsGenauigkeit.MonatsgenauAfa;

        // Laufende Buchungsnummern
        public int LaufendeBuchungsnummerFuerEinnahmen { get; set; } = 1;
        public int LaufendeBuchungsnummerFuerAusgaben { get; set; } = 1;
        public int LaufendeBuchungsnummerFuerBank { get; set; } = 1;
        public int LaufendeBuchungsnummerFuerKasse { get; set; } = 1;

        // Backup-Nachfrage
        public int NachfrageIntervall { get; set; } = 7;
        public DateTime NachfrageTermin { get; set; }

        public bool IsModified { get; set; }
        public string FilePath { get; set; }

        /// <summary>
        /// Erstellt ein neues EasyCash-Dokument für das aktuelle Jahr
        /// </summary>
        public EasyCashDocument()
        {
            Jahr = DateTime.Now.Year;
            NachfrageTermin = DateTime.Now.AddDays(NachfrageIntervall);
            InitializeLocaleSettings();
        }

        /// <summary>
        /// Erstellt ein neues EasyCash-Dokument für ein spezifisches Jahr
        /// </summary>
        public EasyCashDocument(int year, string currency = "EUR")
        {
            Jahr = year;
            Waehrung = currency;
            UrspruenglicheWaehrung = currency;
            NachfrageTermin = DateTime.Now.AddDays(NachfrageIntervall);
        }

        /// <summary>
        /// Initialisiert Ländereinstellungen (Währung basierend auf Systemkultur)
        /// </summary>
        private void InitializeLocaleSettings()
        {
            var cultureInfo = System.Globalization.CultureInfo.CurrentCulture;
            var region = new System.Globalization.RegionInfo(cultureInfo.LCID);
            
            Waehrung = region.ISOCurrencySymbol ?? "EUR";
            UrspruenglicheWaehrung = Waehrung;
        }

        /// <summary>
        /// Berechnet die Summe der Einnahmen mit optionalen Filtern
        /// </summary>
        /// <param name="monatFilter">Monat (1-12) oder Quartal (13-16), null für alle</param>
        /// <param name="kontoFilter">Konto zum Filtern, null für alle</param>
        public long BerechneEinnahmenSumme(int? monatFilter = null, string kontoFilter = null)
        {
            return Einnahmen
                .Where(b => PasstFilter(b, monatFilter, kontoFilter))
                .Where(b => !b.Konto.StartsWith("/"))  // Keine neutralen Konten
                .Sum(b => b.Wert);
        }

        /// <summary>
        /// Berechnet die Summe der Ausgaben mit optionalen Filtern
        /// </summary>
        public long BerechneAusgabenSumme(int? monatFilter = null, string kontoFilter = null)
        {
            long summe = 0;
            foreach (var buchung in Ausgaben)
            {
                if (!PasstFilter(buchung, monatFilter, kontoFilter))
                    continue;
                if (buchung.Konto.StartsWith("/"))
                    continue;

                if (buchung.AbschreibungJahre > 1)
                {
                    long netto = buchung.GetBuchungsjahrNetto(this);
                    long mwst = buchung.AbschreibungNr == 1 
                        ? buchung.Wert - buchung.GetNetto() 
                        : 0;
                    summe += netto + mwst;
                }
                else
                {
                    summe += buchung.Wert;
                }
            }
            return summe;
        }

        /// <summary>
        /// Berechnet die Netto-Summe der Einnahmen
        /// </summary>
        public long BerechneEinnahmenNettoSumme(int? monatFilter = null, string kontoFilter = null)
        {
            return Einnahmen
                .Where(b => PasstFilter(b, monatFilter, kontoFilter))
                .Where(b => !b.Konto.StartsWith("/"))
                .Sum(b => b.GetNetto());
        }

        /// <summary>
        /// Berechnet die Netto-Summe der Ausgaben
        /// </summary>
        public long BerechneAusgabenNettoSumme(int? monatFilter = null, string kontoFilter = null)
        {
            return Ausgaben
                .Where(b => PasstFilter(b, monatFilter, kontoFilter))
                .Where(b => !b.Konto.StartsWith("/"))
                .Sum(b => b.GetBuchungsjahrNetto(this));
        }

        /// <summary>
        /// Konvertiert alle Buchungen zu Euro
        /// </summary>
        public bool ConvertToEuro()
        {
            return _currencyConverter.ConvertDocumentToEuro(this);
        }

        /// <summary>
        /// Konvertiert alle Buchungen von Euro zu einer anderen Währung
        /// </summary>
        public bool ConvertFromEuro(string targetCurrency)
        {
            return _currencyConverter.ConvertDocumentFromEuro(this, targetCurrency);
        }

        /// <summary>
        /// Sortiert Buchungen chronologisch nach Datum, Belegnummer und Beschreibung
        /// </summary>
        public void SortiereBuchungen()
        {
            _sortingEngine.SortiereBuchungen(this);
            IsModified = true;
        }

        /// <summary>
        /// Fügt eine neue Buchung an der richtigen Stelle ein (sortiert)
        /// </summary>
        public void AddEinnahme(Buchung buchung)
        {
            if (buchung == null)
                throw new ArgumentNullException(nameof(buchung));
            
            _sortingEngine.InsertSorted(Einnahmen, buchung);
            IsModified = true;
        }

        /// <summary>
        /// Fügt eine neue Ausgabe an der richtigen Stelle ein (sortiert)
        /// </summary>
        public void AddAusgabe(Buchung buchung)
        {
            if (buchung == null)
                throw new ArgumentNullException(nameof(buchung));
            
            _sortingEngine.InsertSorted(Ausgaben, buchung);
            IsModified = true;
        }

        /// <summary>
        /// Fügt eine Dauerbuchung hinzu
        /// </summary>
        public void AddDauerbuchung(Dauerbuchung dauerBuchung)
        {
            if (dauerBuchung == null)
                throw new ArgumentNullException(nameof(dauerBuchung));
            
            Dauerbuchungen.AddLast(dauerBuchung);
            IsModified = true;
        }

        /// <summary>
        /// Erstellt ein Dokument für das Folgejahr mit Abschreibungsüberträgen
        /// </summary>
        public EasyCashDocument CreateNextYearDocument()
        {
            var transitionEngine = new YearTransitionEngine();
            var nextYear = transitionEngine.CreateNextYearDocument(this);
            return nextYear;
        }

        /// <summary>
        /// Prüft ob eine Backup-Anfrage angezeigt werden soll
        /// </summary>
        public bool ShouldPromptForBackup()
        {
            return _backupManager.ShouldPromptForBackup();
        }

        /// <summary>
        /// Erstellt eine automatische Wiederherstellungsdatei
        /// </summary>
        public void CreateRecoveryFile()
        {
            if (!string.IsNullOrEmpty(FilePath))
                _backupManager.CreateRecoveryFile(FilePath);
        }

        /// <summary>
        /// Löscht die Wiederherstellungsdatei
        /// </summary>
        public void DeleteRecoveryFile()
        {
            if (!string.IsNullOrEmpty(FilePath))
                _backupManager.DeleteRecoveryFile(FilePath);
        }

        /// <summary>
        /// Prüft ob eine neuere Wiederherstellungsdatei existiert
        /// </summary>
        public bool HasNewerRecoveryFile()
        {
            if (string.IsNullOrEmpty(FilePath))
                return false;
            return _backupManager.HasNewerRecoveryFile(FilePath);
        }

        /// <summary>
        /// Prüft ob eine Buchung in der Einnahmen-Liste vorhanden ist
        /// </summary>
        public bool IsEinnahme(Buchung buchung)
        {
            return Einnahmen.Contains(buchung);
        }

        /// <summary>
        /// Prüft ob eine Buchung in der Ausgaben-Liste vorhanden ist
        /// </summary>
        public bool IsAusgabe(Buchung buchung)
        {
            return Ausgaben.Contains(buchung);
        }

        /// <summary>
        /// Löscht eine Buchung aus dem Dokument
        /// </summary>
        public bool RemoveBuchung(Buchung buchung)
        {
            bool removed = Einnahmen.Remove(buchung) || Ausgaben.Remove(buchung);
            if (removed)
                IsModified = true;
            return removed;
        }

        /// <summary>
        /// Löscht alle Buchungen
        /// </summary>
        public void ClearAllBuchungen()
        {
            Einnahmen.Clear();
            Ausgaben.Clear();
            Dauerbuchungen.Clear();
            IsModified = true;
        }

        /// <summary>
        /// Gibt die Anzahl aller Buchungen zurück
        /// </summary>
        public int GetTotalBuchungenCount()
        {
            return Einnahmen.Count + Ausgaben.Count;
        }

        /// <summary>
        /// Setzt das Modifiziert-Flag und erstellt eine Wiederherstellungsdatei
        /// </summary>
        public void SetModified(string action = "")
        {
            IsModified = true;
            CreateRecoveryFile();
            
            if (!string.IsNullOrEmpty(action))
                System.Diagnostics.Debug.WriteLine($"[{DateTime.Now:yyyy-MM-dd HH:mm:ss}] {action}");
        }

        /// <summary>
        /// Filtert Buchungen nach Monat und Konto
        /// </summary>
        private bool PasstFilter(Buchung buchung, int? monatFilter, string kontoFilter)
        {
            if (monatFilter.HasValue && monatFilter.Value > 0)
            {
                if (monatFilter <= 12)
                {
                    // Monatsfilter (1-12)
                    if (buchung.Datum.Month != monatFilter)
                        return false;
                }
                else if (monatFilter <= 16)
                {
                    // Quartalsfilter (13-16)
                    int quartal = ((buchung.Datum.Month - 1) / 3) + 1;
                    if (quartal != monatFilter - 12)
                        return false;
                }
            }

            if (!string.IsNullOrEmpty(kontoFilter) && buchung.Konto == kontoFilter)
                return false;

            return true;
        }

        /// <summary>
        /// Gibt Zusammenfassung des Dokuments zurück
        /// </summary>
        public override string ToString()
        {
            return $"EasyCashDocument Jahr={Jahr}, Waehrung={Waehrung}, " +
                   $"Einnahmen={Einnahmen.Count}, Ausgaben={Ausgaben.Count}, " +
                   $"Dauerbuchungen={Dauerbuchungen.Count}, Modified={IsModified}";
        }
    }
}