// AnfangssaldoViewModel.cs -- Anfangssaldo eines Bestandskontos
// (WPF-Ersatz für CAnfangssaldoDlg). Der Wert gilt fürs Vorjahr des
// Buchungsjahrs und wird als Rohstring in der ini gehalten
// (BestandskontoNNSaldoJJJJ), Format deutsch: "1234,56".

using System.Globalization;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.Stammdaten
{
    public class AnfangssaldoViewModel : ViewModelBase
    {
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
        /// zwei Nachkommastellen im SYSTEMFORMAT normalisiert (der native
        /// Leser currency_to_int arbeitet mit den Locale-Zeichen), leer
        /// bleibt leer.</summary>
        public string NormalisierterWert
        {
            get
            {
                var w = (_wert ?? "").Trim();
                if (w.Length == 0) return "";
                return TryParseBetrag(w, out decimal betrag)
                    ? Waehrungsformat.BetragOhneGruppierung(betrag)
                    : w;
            }
        }

        public AnfangssaldoViewModel(string kontoName, int saldoJahr, string aktuellerWert)
        {
            KontoName = kontoName ?? "";
            SaldoJahr = saldoJahr;
            _wert = aktuellerWert ?? "";
        }

        /// <summary>Toleranter, locale-freier Betrag-Parser (deutsche,
        /// schweizerische und englische Schreibweisen).</summary>
        private static bool TryParseBetrag(string s, out decimal betrag)
            => Waehrungsformat.TryParse(s, out betrag);
    }
}
