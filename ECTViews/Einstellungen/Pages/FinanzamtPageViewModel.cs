using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>Stammdaten des zuständigen Finanzamts ([Finanzamt]).</summary>
    public class FinanzamtPageViewModel : ViewModelBase
    {
        public string Name          { get => GlobaleEinstellungen.FinanzamtName;         set { GlobaleEinstellungen.FinanzamtName = value;          OnPropertyChanged(); } }
        public string Name2         { get => GlobaleEinstellungen.FinanzamtName2;        set { GlobaleEinstellungen.FinanzamtName2 = value;         OnPropertyChanged(); } }
        public string Strasse       { get => GlobaleEinstellungen.FinanzamtStrasse;      set { GlobaleEinstellungen.FinanzamtStrasse = value;       OnPropertyChanged(); } }
        public string Plz           { get => GlobaleEinstellungen.FinanzamtPlz;          set { GlobaleEinstellungen.FinanzamtPlz = value;           OnPropertyChanged(); } }
        public string Ort           { get => GlobaleEinstellungen.FinanzamtOrt;          set { GlobaleEinstellungen.FinanzamtOrt = value;           OnPropertyChanged(); } }
        public string Steuernummer  { get => GlobaleEinstellungen.FinanzamtSteuernummer; set { GlobaleEinstellungen.FinanzamtSteuernummer = value;  OnPropertyChanged(); } }
        public string WirtschaftsId { get => GlobaleEinstellungen.FinanzamtWirtschaftsId; set { GlobaleEinstellungen.FinanzamtWirtschaftsId = value; OnPropertyChanged(); } }
    }
}
