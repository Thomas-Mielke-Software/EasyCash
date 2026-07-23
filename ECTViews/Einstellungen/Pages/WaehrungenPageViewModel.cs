// WaehrungenPageViewModel.cs -- Einstellungs-Seite "Währungen" (M3).
//
// Listet alle von der ect-forex-API unterstützten Währungen mit je einer
// Checkbox. Angehakte Währungen sollen im Buchen-Dialog beim
// Umrechnungsknopf als Option wählbar sein (verallgemeinert das früher fest
// verdrahtete USD/Bitcoin; die Auswahl liefert ECTEngine.Waehrungsliste).
//
// Die Liste wird beim ersten Öffnen von der API geholt und lokal
// zwischengespeichert; danach wird aus dem Cache gelesen. Der Knopf
// "Liste aktualisieren" holt sie erneut (und überschreibt den Cache nur bei
// erfolgreichem Abruf).

using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Input;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    public class WaehrungenPageViewModel : ViewModelBase
    {
        public ObservableCollection<WaehrungItemViewModel> Waehrungen { get; }
            = new ObservableCollection<WaehrungItemViewModel>();

        /// <summary>Holt die Liste neu von der API (asynchron).</summary>
        public RelayCommand AktualisiereCommand { get; }

        /// <summary>Gefilterte Sicht auf <see cref="Waehrungen"/> (die Liste ist
        /// mit ~2800 Einträgen zu groß zum Durchscrollen -- die ListBox bindet
        /// an diese Sicht, das Filterfeld schränkt sie ein).</summary>
        public ICollectionView Sicht { get; }

        public WaehrungenPageViewModel()
        {
            AktualisiereCommand = new RelayCommand(
                async () => await AktualisiereAsync(), () => !Laedt);

            Sicht = CollectionViewSource.GetDefaultView(Waehrungen);
            Sicht.Filter = Passt;

            // KEIN Auto-Abruf: die Seite zeigt zunächst die hartkodierte
            // EZB-Startliste (siehe Waehrungsliste.HoleGecachteListe). Die API
            // wird erst durch "Liste aktualisieren" kontaktiert (mit Einwilligung).
            LadeAusCache();
        }

        /// <summary>Callback für die DSGVO-Einwilligung vor dem ersten
        /// API-Zugriff (von der View gesetzt). Liefert true = einverstanden.</summary>
        public Func<bool> ApiEinwilligungAbfrage { get; set; }

        private string _filterText = "";
        /// <summary>Freitext-Filter (Code oder Name, teilweise, ohne Groß-/Kleinschreibung).</summary>
        public string FilterText
        {
            get => _filterText;
            set
            {
                if (SetProperty(ref _filterText, value))
                {
                    Sicht.Refresh();
                    OnPropertyChanged(nameof(TrefferAnzeige));
                }
            }
        }

        // Typ-Vorfilter (Radio-Buttons). Default = Fiat: blendet die ~2800
        // Krypto-/Token-Werte aus und zeigt nur die EZB-Fiat-Währungen.
        private WaehrungsTypFilter _typFilter = WaehrungsTypFilter.Fiat;

        public bool FilterFiat
        {
            get => _typFilter == WaehrungsTypFilter.Fiat;
            set { if (value) SetzeTypFilter(WaehrungsTypFilter.Fiat); }
        }
        public bool FilterCrypto
        {
            get => _typFilter == WaehrungsTypFilter.Crypto;
            set { if (value) SetzeTypFilter(WaehrungsTypFilter.Crypto); }
        }
        public bool FilterBeide
        {
            get => _typFilter == WaehrungsTypFilter.Beide;
            set { if (value) SetzeTypFilter(WaehrungsTypFilter.Beide); }
        }

        private void SetzeTypFilter(WaehrungsTypFilter neu)
        {
            if (_typFilter == neu) return;
            _typFilter = neu;
            OnPropertyChanged(nameof(FilterFiat));
            OnPropertyChanged(nameof(FilterCrypto));
            OnPropertyChanged(nameof(FilterBeide));
            Sicht.Refresh();
            OnPropertyChanged(nameof(TrefferAnzeige));

            // Krypto/Beide gewählt, aber im Cache liegt nur die hartkodierte
            // Fiat-Startliste (noch nie online nachgeladen)? Dann jetzt die
            // vollständige Liste (inkl. Krypto) von der API holen -- mit
            // DSGVO-Einwilligung; bei Ablehnung bleibt es bei der Fiat-Liste.
            if ((neu == WaehrungsTypFilter.Crypto || neu == WaehrungsTypFilter.Beide)
                && !Laedt && !Waehrungsliste.CacheVorhanden)
            {
                _ = AktualisiereAsync();
            }
        }

        private bool Passt(object o)
        {
            if (!(o is WaehrungItemViewModel v)) return false;

            // Typ-Vorfilter
            if (_typFilter == WaehrungsTypFilter.Fiat && !v.IstFiat) return false;
            if (_typFilter == WaehrungsTypFilter.Crypto && v.IstFiat) return false;

            // Freitext-Filter (Code oder Name)
            if (!string.IsNullOrWhiteSpace(_filterText))
            {
                var f = _filterText.Trim();
                if (v.Code.IndexOf(f, StringComparison.OrdinalIgnoreCase) < 0
                    && v.Name.IndexOf(f, StringComparison.OrdinalIgnoreCase) < 0)
                    return false;
            }
            return true;
        }

        /// <summary>"N von M angezeigt" bzw. "M Währungen", wenn nichts gefiltert.</summary>
        public string TrefferAnzeige
        {
            get
            {
                if (Waehrungen.Count == 0) return "";
                int n = Sicht.Cast<object>().Count();
                return n == Waehrungen.Count
                    ? Waehrungen.Count + " Währungen"
                    : n + " von " + Waehrungen.Count + " angezeigt";
            }
        }

        private bool _laedt;
        public bool Laedt
        {
            get => _laedt;
            private set
            {
                if (SetProperty(ref _laedt, value))
                {
                    OnPropertyChanged(nameof(NichtLaedt));
                    OnPropertyChanged(nameof(KeineEintraege));
                    CommandManager.InvalidateRequerySuggested();
                }
            }
        }

        /// <summary>Für Bindings, die während des Ladens ausgeblendet werden.</summary>
        public bool NichtLaedt => !_laedt;

        private string _statusText = "";
        public string StatusText
        {
            get => _statusText;
            private set => SetProperty(ref _statusText, value);
        }

        /// <summary>True, wenn (nach Laden) keine Währung im Cache liegt --
        /// dann zeigt die Seite einen Hinweis statt einer leeren Liste.</summary>
        public bool KeineEintraege => Waehrungen.Count == 0 && !_laedt;

        private void LadeAusCache()
        {
            Waehrungen.Clear();
            foreach (var e in Waehrungsliste.HoleGecachteListe())
                Waehrungen.Add(new WaehrungItemViewModel(this, e.Code, e.Name, e.Aktiv));
            if (Waehrungen.Count > 0)
                StatusText = Waehrungsliste.CacheVorhanden
                    ? "Aus dem lokalen Cache geladen."
                    : "Fiat = gängige Landeswährungen (EZB); Krypto = Kryptowährungen wie Bitcoin oder Etherum; 'Liste aktualisieren' lädt alle "
                      + "verfügbaren Währungen (Fiat und Krypto) online nach.";
            OnPropertyChanged(nameof(KeineEintraege));
            OnPropertyChanged(nameof(TrefferAnzeige));
        }

        private async Task AktualisiereAsync()
        {
            if (Laedt) return;

            // Vor dem Netzzugriff: DSGVO-Einwilligung sicherstellen.
            if (ApiEinwilligungAbfrage != null && !ApiEinwilligungAbfrage())
            {
                StatusText = "Aktualisierung abgebrochen – keine Einwilligung zur Online-Abfrage.";
                return;
            }

            Laedt = true;
            StatusText = "Lade Währungsliste von easyct.de ...";
            try
            {
                var liste = await Waehrungsliste.LadeVonApiAsync();
                Waehrungsliste.SpeichereListe(liste);

                Waehrungen.Clear();
                foreach (var e in liste)
                    Waehrungen.Add(new WaehrungItemViewModel(this, e.Code, e.Name, e.Aktiv));

                StatusText = "Aktualisiert am "
                    + DateTime.Now.ToString("dd.MM.yyyy HH:mm") + ".";
                OnPropertyChanged(nameof(TrefferAnzeige));
            }
            catch (Exception ex)
            {
                StatusText = "Aktualisierung fehlgeschlagen: " + KurzeMeldung(ex)
                    + (Waehrungen.Count > 0
                        ? " Die zwischengespeicherte Liste bleibt erhalten."
                        : " Bitte Internetverbindung prüfen und erneut versuchen.");
            }
            finally
            {
                Laedt = false;
                OnPropertyChanged(nameof(KeineEintraege));
            }
        }

        private static string KurzeMeldung(Exception ex)
        {
            // Bei aggregierten/verschachtelten Netz-Fehlern die innerste
            // Meldung nehmen -- die ist meist die aussagekräftigste.
            while (ex.InnerException != null) ex = ex.InnerException;
            return ex.Message;
        }

        /// <summary>Wird von den Item-VMs beim Umschalten einer Checkbox
        /// gerufen -- schreibt die gesamte (kleine) Liste in den Cache.</summary>
        internal void PersistiereAuswahl()
        {
            Waehrungsliste.SpeichereListe(
                Waehrungen.Select(v => new WaehrungsEintrag
                {
                    Code  = v.Code,
                    Name  = v.Name,
                    Aktiv = v.Aktiv
                }));
        }
    }

    /// <summary>Typ-Vorfilter der Währungen-Seite.</summary>
    public enum WaehrungsTypFilter { Fiat, Crypto, Beide }

    /// <summary>Eine Zeile der Währungsliste: Code + Name + Aktiv-Checkbox.</summary>
    public class WaehrungItemViewModel : ViewModelBase
    {
        private readonly WaehrungenPageViewModel _seite;

        public string Code { get; }
        public string Name { get; }

        /// <summary>EZB-Fiat-Währung? (für den Typ-Vorfilter).</summary>
        public bool IstFiat { get; }

        public WaehrungItemViewModel(WaehrungenPageViewModel seite,
            string code, string name, bool aktiv)
        {
            _seite  = seite;
            Code    = code ?? "";
            Name    = name ?? "";
            IstFiat = Waehrungsliste.IstFiatWaehrung(Code);
            _aktiv  = aktiv;
        }

        private bool _aktiv;
        public bool Aktiv
        {
            get => _aktiv;
            set { if (SetProperty(ref _aktiv, value)) _seite.PersistiereAuswahl(); }
        }
    }
}
