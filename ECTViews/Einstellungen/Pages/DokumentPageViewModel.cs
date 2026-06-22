using System;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>
    /// Dokumentbezogene Werte des aktuell geöffneten Buchungsdokuments
    /// (aus dem alten CEinstellungen1, die per-Dokument-Felder): Buchungsjahr
    /// und die vier laufenden Belegnummern-Zähler.
    ///
    /// Schreibt direkt in das übergebene <see cref="BuchungsDocument"/> (=
    /// dieselbe Engine-Instanz, die die Bridge hält). Bei jeder tatsächlichen
    /// Änderung wird <see cref="_onGeaendert"/> aufgerufen -- die native Seite
    /// setzt darüber sofort das Modified-Flag des Dokuments, sodass MFC beim
    /// Schließen "Speichern?" fragt (unabhängig vom Teardown-Pfad). Die Daten
    /// selbst werden beim Speichern über CEasyCashDocBridge::Serialize ->
    /// SyncManagedToNative geschrieben.
    /// </summary>
    public class DokumentPageViewModel : ViewModelBase
    {
        private readonly BuchungsDocument _doc;
        private readonly Action _onGeaendert;

        public DokumentPageViewModel(BuchungsDocument doc, Action onGeaendert)
        {
            _doc = doc;
            _onGeaendert = onGeaendert;
        }

        /// <summary>Buchungsjahr (int nJahr). Wie im Original auf 1980..3000
        /// begrenzt (vgl. DDV_MinMaxInt in einstellungen1.cpp).</summary>
        public int Buchungsjahr
        {
            get => _doc.Jahr;
            set
            {
                int v = value < 1980 ? 1980 : (value > 3000 ? 3000 : value);
                bool geaendert = _doc.Jahr != v;
                _doc.Jahr = v;
                OnPropertyChanged();   // immer, damit ein geklemmter Wert zurück in die UI fließt
                if (geaendert) { _onGeaendert?.Invoke(); }
            }
        }

        public int BelegnrEinnahmen
        {
            get => _doc.LaufendeBelegnrEinnahmen;
            set { if (_doc.LaufendeBelegnrEinnahmen != value) { _doc.LaufendeBelegnrEinnahmen = value; OnPropertyChanged(); _onGeaendert?.Invoke(); } }
        }

        public int BelegnrAusgaben
        {
            get => _doc.LaufendeBelegnrAusgaben;
            set { if (_doc.LaufendeBelegnrAusgaben != value) { _doc.LaufendeBelegnrAusgaben = value; OnPropertyChanged(); _onGeaendert?.Invoke(); } }
        }

        public int BelegnrBank
        {
            get => _doc.LaufendeBelegnrBank;
            set { if (_doc.LaufendeBelegnrBank != value) { _doc.LaufendeBelegnrBank = value; OnPropertyChanged(); _onGeaendert?.Invoke(); } }
        }

        public int BelegnrKasse
        {
            get => _doc.LaufendeBelegnrKasse;
            set { if (_doc.LaufendeBelegnrKasse != value) { _doc.LaufendeBelegnrKasse = value; OnPropertyChanged(); _onGeaendert?.Invoke(); } }
        }
    }
}
