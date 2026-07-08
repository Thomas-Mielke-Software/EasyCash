// UnternehmensartViewModel.cs -- Unternehmensdaten eines Betriebs
// (WPF-Ersatz für CUnternehmensartDlg). Der Rohwert in der ini
// (BetriebNNUnternehmensart) ist ein Tab-getrennter String:
//   Unternehmensart1 \t Rechtsform \t Steuernummer \t W-IdNr [\t ...]
// Zukünftige, unbekannte Zusatz-Tokens werden wie im MFC-Original
// (CIconAuswahlBetrieb::ChooseProperty) unverändert beibehalten.

using ECTViews.ViewModels;

namespace ECTViews.Stammdaten
{
    public class UnternehmensartViewModel : ViewModelBase
    {
        public string BetriebName { get; }

        private string _unternehmensart1 = "";
        public string Unternehmensart1
        {
            get => _unternehmensart1;
            set => SetProperty(ref _unternehmensart1, value ?? "");
        }

        private string _unternehmensart2 = "";
        public string Unternehmensart2
        {
            get => _unternehmensart2;
            set => SetProperty(ref _unternehmensart2, value ?? "");
        }

        private string _steuernummer = "";
        public string Steuernummer
        {
            get => _steuernummer;
            set => SetProperty(ref _steuernummer, value ?? "");
        }

        private string _wirtschaftsIdNr = "";
        public string WirtschaftsIdNr
        {
            get => _wirtschaftsIdNr;
            set
            {
                if (SetProperty(ref _wirtschaftsIdNr, value ?? ""))
                    OnPropertyChanged(nameof(WirtschaftsIdNrFehler));
            }
        }

        /// <summary>Advisory-Validierung (rote Zeile unterm Feld), blockiert
        /// das Speichern nicht -- wie die Hinweis-MessageBoxen im Original.</summary>
        public string WirtschaftsIdNrFehler => WIdNrPruefung.Pruefe(_wirtschaftsIdNr) ?? "";

        // Unbekannte Zusatz-Tokens hinter den vier bekannten Feldern
        // (inklusive führendem Tab), beim Speichern wieder angehängt.
        private readonly string _rest = "";

        public UnternehmensartViewModel(string betriebName, string rohwert)
        {
            BetriebName = betriebName ?? "";

            var teile = (rohwert ?? "").Split('\t');
            if (teile.Length > 0) _unternehmensart1 = teile[0];
            if (teile.Length > 1) _unternehmensart2 = teile[1];
            if (teile.Length > 2) _steuernummer     = teile[2];
            if (teile.Length > 3) _wirtschaftsIdNr  = teile[3];
            for (int i = 4; i < teile.Length; i++)
                _rest += "\t" + teile[i];
        }

        /// <summary>Setzt den Tab-getrennten Rohwert wieder zusammen.</summary>
        public string Rohwert =>
            Unternehmensart1 + "\t" + Unternehmensart2 + "\t" +
            Steuernummer + "\t" + WirtschaftsIdNr + _rest;
    }
}
