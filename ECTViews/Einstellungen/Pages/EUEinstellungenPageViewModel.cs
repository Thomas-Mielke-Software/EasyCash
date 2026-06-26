using System.Collections.Generic;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>
    /// E/Ü-spezifische Einstellungen aus dem alten CEinstellungen4 (ohne die
    /// Konten-/Feldzuweisungs-Liste -- die liegt auf der Seite "E/Ü-Konten"):
    /// USt/VSt gesondert ausweisen, Seitenumbruch nach Konten beim Druck und
    /// die Konten-Zuordnung für Finanzamts-Erstattungen/-Zahlungen. Alle Werte
    /// global in [Allgemein].
    /// </summary>
    public class EUEinstellungenPageViewModel : ViewModelBase
    {
        public bool UstVstGesondert
        {
            get => GlobaleEinstellungen.UstVstGesondert;
            set { GlobaleEinstellungen.UstVstGesondert = value; OnPropertyChanged(); }
        }

        public bool SeitenumbruchNachKonten
        {
            get => GlobaleEinstellungen.SeitenumbruchNachKonten;
            set { GlobaleEinstellungen.SeitenumbruchNachKonten = value; OnPropertyChanged(); }
        }

        /// <summary>Auswahl-Liste der Einnahmenkonten (mit führendem Leereintrag
        /// für "keines").</summary>
        public IReadOnlyList<string> EinnahmenKonten { get; }

        /// <summary>Auswahl-Liste der Ausgabenkonten (mit führendem Leereintrag).</summary>
        public IReadOnlyList<string> AusgabenKonten { get; }

        public string FinanzamtsErstattungenKonto
        {
            get => GlobaleEinstellungen.FinanzamtsErstattungenKonto;
            set { GlobaleEinstellungen.FinanzamtsErstattungenKonto = value ?? ""; OnPropertyChanged(); }
        }

        public string FinanzamtsZahlungenKonto
        {
            get => GlobaleEinstellungen.FinanzamtsZahlungenKonto;
            set { GlobaleEinstellungen.FinanzamtsZahlungenKonto = value ?? ""; OnPropertyChanged(); }
        }

        public EUEinstellungenPageViewModel()
        {
            EinstellungenLiveSync.Registriere(this);

            var ein = new List<string> { "" };
            ein.AddRange(Einstellungen.EinnahmenKonten);
            EinnahmenKonten = ein;

            var aus = new List<string> { "" };
            aus.AddRange(Einstellungen.AusgabenKonten);
            AusgabenKonten = aus;
        }
    }
}
