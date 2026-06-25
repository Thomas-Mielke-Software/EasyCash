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
                OnPropertyChanged(nameof(Sondervorauszahlung));  // Sondervorauszahlung ist jahresabhängig
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

        // -----------------------------------------------------------------
        // Sondervorauszahlung (Dauerfristverlängerung). Pro Buchungsjahr, im
        // Erweiterungs-Store des Dokuments abgelegt (Sektion
        // "Dauerfristverlängerung", Key "SondervorauszahlungJJJJ") -- exakt wie
        // im alten CEasyCashView (SetErweiterungKey, easycashview.cpp:8266).
        // -----------------------------------------------------------------
        private const string DauerfristNs = "Dauerfristverlängerung";
        private string SondervorauszahlungKey => "Sondervorauszahlung" + _doc.Jahr.ToString("D4");

        /// <summary>Geleistete Sondervorauszahlung (ein Elftel der Summe aller
        /// USt-Vorauszahlungen des Vorjahres) bei Dauerfristverlängerung für das
        /// aktuelle Buchungsjahr. Geldbetrag im
        /// deutschen Währungsformat (z.B. "1234,56"); leer = keine. Gültige
        /// Eingaben werden auf zwei Nachkommastellen normalisiert (wie das alte
        /// int_to_currency), ungültige bleiben für die Korrektur stehen.</summary>
        public string Sondervorauszahlung
        {
            get => _doc.Erweiterungen.Hole(DauerfristNs, SondervorauszahlungKey);
            set
            {
                string neu = (value ?? "").Trim();
                // Gültigen Betrag aufs kanonische Format bringen; ungültige
                // Eingabe unverändert lassen, damit der Fehlertext greift.
                if (TryParseBetrag(neu, out decimal betrag) && betrag >= 0)
                    neu = betrag.ToString("0.00", _deDe);

                if (_doc.Erweiterungen.Hole(DauerfristNs, SondervorauszahlungKey) != neu)
                {
                    if (string.IsNullOrEmpty(neu))
                        _doc.Erweiterungen.Entferne(DauerfristNs, SondervorauszahlungKey);
                    else
                        _doc.Erweiterungen.Setze(DauerfristNs, SondervorauszahlungKey, neu);
                    _onGeaendert?.Invoke();
                }
                OnPropertyChanged();
                OnPropertyChanged(nameof(SondervorauszahlungFehler));
            }
        }

        /// <summary>Validierung (advisory, blockiert das Speichern nicht -- wie
        /// die übrigen Einstellungs-Felder). Leer = kein Fehler.</summary>
        public string SondervorauszahlungFehler => PruefeSondervorauszahlung(Sondervorauszahlung) ?? "";

        private static string PruefeSondervorauszahlung(string wert)
        {
            if (string.IsNullOrEmpty(wert)) return null;   // leer = keine
            if (!TryParseBetrag(wert, out decimal betrag))
                return "Bitte einen gültigen Geldbetrag eingeben (z.B. 1234,56).";
            if (betrag < 0)
                return "Die Sondervorauszahlung kann nicht negativ sein.";
            return null;
        }

        private static readonly System.Globalization.CultureInfo _deDe =
            new System.Globalization.CultureInfo("de-DE");

        /// <summary>Parst einen Geldbetrag im deutschen Format (Komma als
        /// Dezimal-, Punkt als Tausendertrennzeichen). Fallback: Punkt als
        /// Dezimaltrennzeichen, damit "1234.56" auch durchgeht.</summary>
        private static bool TryParseBetrag(string s, out decimal betrag)
        {
            betrag = 0m;
            if (string.IsNullOrWhiteSpace(s)) return false;
            if (decimal.TryParse(s, System.Globalization.NumberStyles.Number, _deDe, out betrag))
                return true;
            if (decimal.TryParse(s.Replace(".", "").Replace(',', '.'),
                System.Globalization.NumberStyles.Number,
                System.Globalization.CultureInfo.InvariantCulture, out betrag))
                return true;
            return false;
        }
    }
}
