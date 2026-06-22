// EinstellungenViewModel.cs - Host-Logik der WPF-Einstellungen.
//
// Baut die flache, nach Gruppen gruppierbare Navigations-Liste auf und
// hält die aktuell angezeigte Seite. Ersetzt das alte CPropertySheet aus
// easycashview.cpp (propdlg) durch ein gehostetes Child-Window mit
// Navigation statt Registerkarten -- analog zur Journal-Architektur.
//
// Die "Aktuelles Dokument"-Gruppe wird nur aufgebaut, wenn ein Dokument
// offen ist (hatDokument == true).

using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Controls;
using ECTEngine;
using ECTViews.EinstellungenUi.Pages;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi
{
    public class EinstellungenViewModel : ViewModelBase
    {
        private const string GRUPPE_GLOBAL   = "Globale Einstellungen";
        private const string GRUPPE_DOKUMENT = "Aktuelles Dokument";

        /// <summary>Flache Liste aller Navigations-Einträge; die Gruppierung
        /// erledigt die View über eine CollectionViewSource (PropertyGroup
        /// auf <see cref="EinstellungenNavItem.Gruppe"/>).</summary>
        public ObservableCollection<EinstellungenNavItem> Items { get; }
            = new ObservableCollection<EinstellungenNavItem>();

        private EinstellungenNavItem _ausgewaehltesItem;
        public EinstellungenNavItem AusgewaehltesItem
        {
            get => _ausgewaehltesItem;
            set
            {
                if (SetProperty(ref _ausgewaehltesItem, value))
                    AktuelleSeite = value?.Seite;
            }
        }

        private UserControl _aktuelleSeite;
        public UserControl AktuelleSeite
        {
            get => _aktuelleSeite;
            private set => SetProperty(ref _aktuelleSeite, value);
        }

        /// <param name="dokument">Das aktuell geöffnete Buchungsdokument oder
        /// null. Nur wenn ein Dokument offen ist, erscheint die "Aktuelles
        /// Dokument"-Gruppe (Buchungsjahr, laufende Belegnummern).</param>
        /// <param name="onDokumentGeaendert">Callback, der bei jeder Änderung
        /// eines Dokumentwerts gerufen wird (setzt nativ das Modified-Flag).</param>
        public EinstellungenViewModel(BuchungsDocument dokument, System.Action onDokumentGeaendert = null)
        {
            Items.Add(Seite(GRUPPE_GLOBAL, "Allgemein",     new AllgemeinPage()));
            Items.Add(Seite(GRUPPE_GLOBAL, "Unternehmer*in", new UnternehmerPage()));
            Items.Add(Seite(GRUPPE_GLOBAL, "Finanzamt",     new FinanzamtPage()));
            Items.Add(Seite(GRUPPE_GLOBAL, "Buchungs-Presets", new PresetsPage()));
            Items.Add(Seite(GRUPPE_GLOBAL, "E/Ü-Konten", new KontenPage()));
            // Währungen folgt in M3.

            if (dokument != null)
            {
                Items.Add(Seite(GRUPPE_DOKUMENT, "Buchungsjahr & Belegnummern",
                    new DokumentPage(dokument, onDokumentGeaendert)));
            }

            // Erste Seite vorselektieren.
            AusgewaehltesItem = Items.FirstOrDefault();
        }

        private static EinstellungenNavItem Seite(string gruppe, string titel, UserControl seite)
            => new EinstellungenNavItem { Gruppe = gruppe, Titel = titel, Seite = seite };
    }
}
