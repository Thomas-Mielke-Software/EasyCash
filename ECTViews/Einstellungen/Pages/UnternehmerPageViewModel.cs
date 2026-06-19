using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>Stammdaten des Unternehmers ([Persoenliche_Daten]).</summary>
    public class UnternehmerPageViewModel : ViewModelBase
    {
        public string Name            { get => GlobaleEinstellungen.UnternehmerName;    set { GlobaleEinstellungen.UnternehmerName = value;    OnPropertyChanged(); } }
        public string Vorname         { get => GlobaleEinstellungen.UnternehmerVorname; set { GlobaleEinstellungen.UnternehmerVorname = value; OnPropertyChanged(); } }
        public string Strasse         { get => GlobaleEinstellungen.UnternehmerStrasse; set { GlobaleEinstellungen.UnternehmerStrasse = value; OnPropertyChanged(); } }
        public string Plz             { get => GlobaleEinstellungen.UnternehmerPlz;     set { GlobaleEinstellungen.UnternehmerPlz = value;     OnPropertyChanged(); } }
        public string Ort             { get => GlobaleEinstellungen.UnternehmerOrt;     set { GlobaleEinstellungen.UnternehmerOrt = value;     OnPropertyChanged(); } }
        public string Unternehmensart1 { get => GlobaleEinstellungen.Unternehmensart1;  set { GlobaleEinstellungen.Unternehmensart1 = value;  OnPropertyChanged(); } }
        public string Unternehmensart2 { get => GlobaleEinstellungen.Unternehmensart2;  set { GlobaleEinstellungen.Unternehmensart2 = value;  OnPropertyChanged(); } }

        /// <summary>0 = Deutschland, 1 = Österreich, 2 = Schweiz.</summary>
        public int Land { get => GlobaleEinstellungen.Land; set { GlobaleEinstellungen.Land = value; OnPropertyChanged(); } }
    }
}
