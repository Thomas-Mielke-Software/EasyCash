using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>
    /// Allgemeine, globale Einstellungen ([Allgemein] / [Druck]).
    /// Dokumentspezifische Werte (Buchungsjahr, lfd. Belegnummern) liegen
    /// NICHT hier, sondern auf der "Aktuelles Dokument"-Seite.
    /// </summary>
    public class AllgemeinPageViewModel : ViewModelBase
    {
        // Buchen-Dialog-Steuerung
        public bool MwstFeldAktiviert   { get => GlobaleEinstellungen.MwstFeldAktiviert;   set { GlobaleEinstellungen.MwstFeldAktiviert = value;   OnPropertyChanged(); } }
        public bool JahresfeldAktiviert { get => GlobaleEinstellungen.JahresfeldAktiviert; set { GlobaleEinstellungen.JahresfeldAktiviert = value; OnPropertyChanged(); } }

        // Laufende Belegnummern erzeugen
        public bool BelegnrEinnahmen { get => GlobaleEinstellungen.ErzeugeBelegnrEinnahmen; set { GlobaleEinstellungen.ErzeugeBelegnrEinnahmen = value; OnPropertyChanged(); } }
        public bool BelegnrAusgaben  { get => GlobaleEinstellungen.ErzeugeBelegnrAusgaben;  set { GlobaleEinstellungen.ErzeugeBelegnrAusgaben = value;  OnPropertyChanged(); } }
        public bool BelegnrBank      { get => GlobaleEinstellungen.ErzeugeBelegnrBank;      set { GlobaleEinstellungen.ErzeugeBelegnrBank = value;      OnPropertyChanged(); } }
        public bool BelegnrKasse     { get => GlobaleEinstellungen.ErzeugeBelegnrKasse;     set { GlobaleEinstellungen.ErzeugeBelegnrKasse = value;     OnPropertyChanged(); } }

        // Buchen-Verhalten
        public bool TaeglichBuchen        { get => GlobaleEinstellungen.TaeglichBuchen;        set { GlobaleEinstellungen.TaeglichBuchen = value;        OnPropertyChanged(); } }
        public bool BuchungsdatumBelassen { get => GlobaleEinstellungen.BuchungsdatumBelassen; set { GlobaleEinstellungen.BuchungsdatumBelassen = value; OnPropertyChanged(); } }

        /// <summary>0 = ganzjährig, 1 = halbjährig, 2 = monatsgenau
        /// (GANZJAHRES_AFA / HALBJAHRES_AFA / MONATSGENAUE_AFA).</summary>
        public int AfaGenauigkeit { get => GlobaleEinstellungen.AbschreibungGenauigkeit; set { GlobaleEinstellungen.AbschreibungGenauigkeit = value; OnPropertyChanged(); } }

        // Schriften
        public string Bildschirmschrift        { get => GlobaleEinstellungen.Bildschirmschrift;        set { GlobaleEinstellungen.Bildschirmschrift = value;        OnPropertyChanged(); } }
        public int    Bildschirmschriftgroesse { get => GlobaleEinstellungen.Bildschirmschriftgroesse; set { GlobaleEinstellungen.Bildschirmschriftgroesse = value; OnPropertyChanged(); } }
        public string Druckerschrift           { get => GlobaleEinstellungen.Druckerschrift;           set { GlobaleEinstellungen.Druckerschrift = value;           OnPropertyChanged(); } }
        public int    Druckerschriftgroesse    { get => GlobaleEinstellungen.Druckerschriftgroesse;    set { GlobaleEinstellungen.Druckerschriftgroesse = value;    OnPropertyChanged(); } }
    }
}
