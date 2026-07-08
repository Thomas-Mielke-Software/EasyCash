using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>Stammdaten des zuständigen Finanzamts ([Finanzamt]).</summary>
    public class FinanzamtPageViewModel : ViewModelBase
    {
        public FinanzamtPageViewModel() => EinstellungenLiveSync.Registriere(this);

        public string Name          { get => GlobaleEinstellungen.FinanzamtName;          set { GlobaleEinstellungen.FinanzamtName = value;          OnPropertyChanged(); } }
        public string Name2         { get => GlobaleEinstellungen.FinanzamtName2;         set { GlobaleEinstellungen.FinanzamtName2 = value;         OnPropertyChanged(); } }
        public string Strasse       { get => GlobaleEinstellungen.FinanzamtStrasse;       set { GlobaleEinstellungen.FinanzamtStrasse = value;       OnPropertyChanged(); } }
        public string Plz           { get => GlobaleEinstellungen.FinanzamtPlz;           set { GlobaleEinstellungen.FinanzamtPlz = value;           OnPropertyChanged(); } }
        public string Ort           { get => GlobaleEinstellungen.FinanzamtOrt;           set { GlobaleEinstellungen.FinanzamtOrt = value;           OnPropertyChanged(); } }
        public string Steuernummer  { get => GlobaleEinstellungen.FinanzamtSteuernummer;  set { GlobaleEinstellungen.FinanzamtSteuernummer = value;  OnPropertyChanged(); } }
        public string WirtschaftsId
        {
            get => GlobaleEinstellungen.FinanzamtWirtschaftsId;
            set
            {
                GlobaleEinstellungen.FinanzamtWirtschaftsId = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(WirtschaftsIdFehler));  // Fehlertext mit aktualisieren
            }
        }

        // -----------------------------------------------------------------
        // Validierung (advisory, blockiert das Speichern nicht -- wie der
        // Hinweis-Dialog im alten MFC-Code). Nur die W-IdNr hat eine
        // Format-Pruefung; der Fehlertext wird wie im Buchen-Dialog als rote
        // Zeile unter dem Feld angezeigt (leer = kein Fehler -> Zeile
        // verschwindet ueber den TextBlock-Trigger). Die Pruef-Logik liegt
        // in WIdNrPruefung (geteilt mit dem Unternehmensart-Dialog).
        // -----------------------------------------------------------------
        public string WirtschaftsIdFehler => WIdNrPruefung.Pruefe(WirtschaftsId) ?? "";
    }
}
