using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>
    /// Stammdaten des Unternehmers ([Persoenliche_Daten]) samt der vier
    /// MwSt-Sätze (vat1..vat4). Land und Sätze liegen bewusst auf einer Seite,
    /// weil die Landauswahl die Sätze vorbelegt (wie im alten CEinstellungen2:
    /// OnLand1/2/3) -- die Auswirkung soll direkt sichtbar sein.
    /// </summary>
    public class UnternehmerPageViewModel : ViewModelBase
    {
        public UnternehmerPageViewModel() => EinstellungenLiveSync.Registriere(this);

        public string Vorname          { get => GlobaleEinstellungen.UnternehmerVorname; set { GlobaleEinstellungen.UnternehmerVorname = value; OnPropertyChanged(); } }
        public string Name             { get => GlobaleEinstellungen.UnternehmerName;    set { GlobaleEinstellungen.UnternehmerName = value;    OnPropertyChanged(); } }
        public string Unternehmensart1 { get => GlobaleEinstellungen.Unternehmensart1;   set { GlobaleEinstellungen.Unternehmensart1 = value;  OnPropertyChanged(); } }
        public string Unternehmensart2 { get => GlobaleEinstellungen.Unternehmensart2;   set { GlobaleEinstellungen.Unternehmensart2 = value;  OnPropertyChanged(); } }
        public string Strasse          { get => GlobaleEinstellungen.UnternehmerStrasse; set { GlobaleEinstellungen.UnternehmerStrasse = value; OnPropertyChanged(); } }
        public string Plz              { get => GlobaleEinstellungen.UnternehmerPlz;     set { GlobaleEinstellungen.UnternehmerPlz = value;     OnPropertyChanged(); } }
        public string Ort              { get => GlobaleEinstellungen.UnternehmerOrt;     set { GlobaleEinstellungen.UnternehmerOrt = value;     OnPropertyChanged(); } }

        /// <summary>0 = Deutschland, 1 = Österreich, 2 = Schweiz. Eine
        /// Änderung belegt die vier MwSt-Sätze mit den Landesvorgaben vor.</summary>
        public int Land
        {
            get => GlobaleEinstellungen.Land;
            set
            {
                if (GlobaleEinstellungen.Land == value) { return; }
                GlobaleEinstellungen.Land = value;
                OnPropertyChanged();
                SetzeMwStVorgaben(value);
            }
        }

        // -----------------------------------------------------------------
        // MwSt-Sätze (vat1..vat4) -- als String, weil deutsches Dezimalkomma
        // erlaubt ist (z.B. "7,5"). 0 = nicht verwendet.
        // -----------------------------------------------------------------
        public string Satz1 { get => GlobaleEinstellungen.Vat1; set { GlobaleEinstellungen.Vat1 = value; OnPropertyChanged(); } }
        public string Satz2 { get => GlobaleEinstellungen.Vat2; set { GlobaleEinstellungen.Vat2 = value; OnPropertyChanged(); } }
        public string Satz3 { get => GlobaleEinstellungen.Vat3; set { GlobaleEinstellungen.Vat3 = value; OnPropertyChanged(); } }
        public string Satz4 { get => GlobaleEinstellungen.Vat4; set { GlobaleEinstellungen.Vat4 = value; OnPropertyChanged(); } }

        // -----------------------------------------------------------------
        // Voranmeldungszeitraum (Radio Monat=0 / Quartal=1 im alten
        // CEinstellungen1). Bestimmt, ob die USt-Voranmeldung monatlich oder
        // vierteljährlich erstellt wird. Default = Quartal.
        // -----------------------------------------------------------------
        public int Voranmeldungszeitraum
        {
            get => GlobaleEinstellungen.MonatlicheVoranmeldung;
            set { GlobaleEinstellungen.MonatlicheVoranmeldung = value; OnPropertyChanged(); }
        }

        /// <summary>
        /// Belegt die vier Sätze mit den gesetzlichen Vorgaben des Landes vor
        /// (Stand 2024). Satz 1 = Regelsatz, Satz 2/3 = ermäßigte Sätze.
        /// Wird nur bei einer tatsächlichen Land-Änderung durch den Nutzer
        /// gerufen, nicht beim Laden -- eigene Sätze bleiben also erhalten,
        /// solange das Land nicht umgestellt wird.
        /// </summary>
        private void SetzeMwStVorgaben(int land)
        {
            switch (land)
            {
                case 0: // Deutschland
                    Satz1 = "19"; Satz2 = "7"; Satz3 = "0"; Satz4 = "0";
                    break;
                case 1: // Österreich
                    Satz1 = "20"; Satz2 = "13"; Satz3 = "10"; Satz4 = "0";
                    break;
                case 2: // Schweiz (Sätze ab 01.01.2024)
                    Satz1 = "8,1"; Satz2 = "2,6"; Satz3 = "3,8"; Satz4 = "0";
                    break;
            }
        }
    }
}
