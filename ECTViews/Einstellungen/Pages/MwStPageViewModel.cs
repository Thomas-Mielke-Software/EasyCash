using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>
    /// Konfigurierbare Mehrwertsteuersätze ([Persoenliche_Daten] vat1..vat4).
    /// Diese Sätze erscheinen zur Auswahl im Buchen-Dialog. Format als String,
    /// weil deutsches Dezimalkomma erlaubt ist (z.B. "7,5").
    /// </summary>
    public class MwStPageViewModel : ViewModelBase
    {
        public string Satz1 { get => GlobaleEinstellungen.Vat1; set { GlobaleEinstellungen.Vat1 = value; OnPropertyChanged(); } }
        public string Satz2 { get => GlobaleEinstellungen.Vat2; set { GlobaleEinstellungen.Vat2 = value; OnPropertyChanged(); } }
        public string Satz3 { get => GlobaleEinstellungen.Vat3; set { GlobaleEinstellungen.Vat3 = value; OnPropertyChanged(); } }
        public string Satz4 { get => GlobaleEinstellungen.Vat4; set { GlobaleEinstellungen.Vat4 = value; OnPropertyChanged(); } }
    }
}
