// AnfangssaldoViewModel.cs -- Anfangssaldo eines Bestandskontos
// (WPF-Ersatz für CAnfangssaldoDlg). Der Wert gilt fürs Vorjahr des
// Buchungsjahrs und wird als Rohstring in der ini gehalten
// (BestandskontoNNSaldoJJJJ), Format deutsch: "1234,56".

using System.Globalization;
using ECTViews.ViewModels;

namespace ECTViews.Stammdaten
{
    public class AnfangssaldoViewModel : ViewModelBase
    {
        private static readonly CultureInfo _deDe = new CultureInfo("de-DE");

        public string KontoName { get; }
        public int SaldoJahr { get; }

        // Wortlaut wie im alten IDD_ANFANGSSALDO, ergänzt um Konto + Stichtag
        public string Beschriftung =>
            $"Anfangssaldo bzw. Übertrag aus dem Vorjahr für \"{KontoName}\" (Stand 31.12.{SaldoJahr}):";

        private string _wert;
        public string Wert
        {
            get => _wert;
            set
            {
                if (SetProperty(ref _wert, value))
                    OnPropertyChanged(nameof(Fehler));
            }
        }

        /// <summary>Advisory-Validierung wie bei der Sondervorauszahlung --
        /// leer ist erlaubt (= kein Anfangssaldo), negativ ebenfalls
        /// (Konto im Minus).</summary>
        public string Fehler
        {
            get
            {
                var w = (_wert ?? "").Trim();
                if (w.Length == 0) return "";
                return TryParseBetrag(w, out _)
                    ? ""
                    : "Bitte einen gültigen Geldbetrag eingeben (z.B. 1234,56).";
            }
        }

        public bool IstGueltig => Fehler.Length == 0;

        /// <summary>Kanonisierter Wert fürs Speichern: gültige Beträge auf
        /// zwei Nachkommastellen normalisiert, leer bleibt leer.</summary>
        public string NormalisierterWert
        {
            get
            {
                var w = (_wert ?? "").Trim();
                if (w.Length == 0) return "";
                return TryParseBetrag(w, out decimal betrag)
                    ? betrag.ToString("0.00", _deDe)
                    : w;
            }
        }

        public AnfangssaldoViewModel(string kontoName, int saldoJahr, string aktuellerWert)
        {
            KontoName = kontoName ?? "";
            SaldoJahr = saldoJahr;
            _wert = aktuellerWert ?? "";
        }

        /// <summary>Parst einen Geldbetrag im deutschen Format (Komma als
        /// Dezimal-, Punkt als Tausendertrennzeichen). Fallback: Punkt als
        /// Dezimaltrennzeichen, damit "1234.56" auch durchgeht.</summary>
        private static bool TryParseBetrag(string s, out decimal betrag)
        {
            if (decimal.TryParse(s, NumberStyles.Number, _deDe, out betrag))
                return true;
            return decimal.TryParse(s.Replace(',', '.'), NumberStyles.Number,
                CultureInfo.InvariantCulture, out betrag);
        }
    }
}
