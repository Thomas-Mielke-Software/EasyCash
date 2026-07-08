// StammdatenVerwaltenViewModel.cs -- Verwaltung + Auswahl von Betrieben
// bzw. Bestandskonten (WPF-Ersatz für CIconAuswahlBetrieb /
// CIconAuswahlBestandskonto im Modus 1, easycashview.cpp
// OnViewJournalBetrieb / OnViewJournalBestandskonto).
//
// Der Dialog dient wie im Original doppelt:
//  - Verwaltung: Anlegen, Löschen, Umbenennen (inline), Icon ändern,
//    Property bearbeiten (Unternehmensart bzw. Anfangssaldo).
//  - Auswahl: "Sel. anzeigen" liefert den Index des gewählten Eintrags
//    als Journal-Filter, "Alle anzeigen" hebt den Filter auf.
//
// Persistenz läuft über ECTEngine.StammdatenVerwaltung (globaler
// Einstellungs-Cache + ini-Sektions-Schreiben über die Bridge).

using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.Stammdaten
{
    /// <summary>Ein Eintrag (Betrieb/Bestandskonto) in der Verwaltungs-Liste.</summary>
    public class StammdatenEintragVM : ViewModelBase
    {
        internal StammdatenEintrag Modell { get; }
        private readonly StammdatenVerwaltenViewModel _besitzer;

        internal StammdatenEintragVM(StammdatenEintrag modell,
            StammdatenVerwaltenViewModel besitzer)
        {
            Modell = modell;
            _besitzer = besitzer;
        }

        /// <summary>Name; inline editierbar (ersetzt den alten
        /// "Umbenennen"-Knopf samt CMandantName-Dialog). Leer wird ignoriert,
        /// damit kein Eintrag durch Leertippen "verschwindet" (die Leseschleifen
        /// stoppen beim ersten leeren Namen).</summary>
        public string Name
        {
            get => Modell.Name;
            set
            {
                var neu = (value ?? "").Trim();
                if (string.IsNullOrEmpty(neu) || Modell.Name == neu) return;
                Modell.Name = neu;
                OnPropertyChanged();
                OnPropertyChanged(nameof(ToolTipText));
                _besitzer.Persistiere();
                Statusleiste.Melde($"In \"{neu}\" umbenannt.");
            }
        }

        public int IconIndex
        {
            get => Modell.Icon;
            set
            {
                if (Modell.Icon == value) return;
                Modell.Icon = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(Icon));
            }
        }

        /// <summary>Zugeschnittenes Icon aus dem Sprite (null-tolerant).</summary>
        public ImageSource Icon => IconSpriteSplitter.Crop(_besitzer.Sprite, Modell.Icon);

        /// <summary>Tooltip in der Liste -- typspezifisch (Mandanten zeigen
        /// z.B. das Datenverzeichnis mit an).</summary>
        public string ToolTipText => _besitzer.EintragToolTip(this);

        internal void MeldeToolTipGeaendert() => OnPropertyChanged(nameof(ToolTipText));
    }

    public abstract class StammdatenVerwaltenViewModel : ViewModelBase
    {
        public ObservableCollection<StammdatenEintragVM> Eintraege { get; }
            = new ObservableCollection<StammdatenEintragVM>();

        private StammdatenEintragVM _auswahl;
        public StammdatenEintragVM Auswahl
        {
            get => _auswahl;
            set
            {
                if (SetProperty(ref _auswahl, value))
                    OnPropertyChanged(nameof(EintragGewaehlt));
            }
        }

        public bool EintragGewaehlt => _auswahl != null;

        // ------------------------------------------------------------------
        // Typ-spezifisches (Betriebe vs. Bestandskonten)
        // ------------------------------------------------------------------

        public abstract string Titel { get; }
        /// <summary>Beschriftung des Neu-Knopfs (wie IDC_Neu im Original).</summary>
        public abstract string NeuKnopfText { get; }
        /// <summary>Beschriftung des Property-Knopfs (wie IDC_PROPERTY im Original).</summary>
        public abstract string PropertyKnopfText { get; }

        // OK-/Abbrechen-Beschriftung: Betriebe/Bestandskonten dienen der
        // Journal-Filter-Auswahl ("Sel. anzeigen"/"Alle anzeigen"), Mandanten
        // dem Mandantenwechsel -- dort passen die Filter-Texte nicht.
        public virtual string OkKnopfText => "Sel. anzeigen";
        public virtual string AbbrechenKnopfText => "Alle anzeigen";
        public virtual string OkKnopfToolTip =>
            "Journal nur für den gewählten Eintrag anzeigen (Filter setzen)";
        public virtual string AbbrechenKnopfToolTip =>
            "Filter aufheben und wieder alle anzeigen";

        /// <summary>Tooltip eines Listen-Eintrags (Default: nur der Name).</summary>
        internal virtual string EintragToolTip(StammdatenEintragVM eintrag)
            => eintrag.Name;
        internal abstract string Sektion { get; }
        internal abstract string Praefix { get; }
        internal abstract BitmapSource Sprite { get; }
        internal abstract string[] IconNamen { get; }
        /// <summary>ini-Key-Suffix des Property-Werts ("Unternehmensart" bzw. "SaldoJJJJ").</summary>
        internal abstract string PropertySuffix { get; }
        /// <summary>Default-Name eines neuen Eintrags (wie GetIconText im Original).</summary>
        internal abstract string DefaultName(int iconIndex, int listenIndex);

        /// <summary>Öffnet den typspezifischen Property-Dialog (Unternehmensart
        /// bzw. Anfangssaldo) modal. True + neuer Wert bei OK, False bei
        /// Abbruch (entspricht ChooseProperty im Original).</summary>
        internal abstract bool FrageProperty(System.Windows.Window owner,
            string name, string aktuellerWert, out string neuerWert);

        // ------------------------------------------------------------------

        protected StammdatenVerwaltenViewModel()
        {
            // Achtung: virtuelle Member (Sektion/Praefix) erst nach der
            // Subklassen-Konstruktion nutzbar -> Laden explizit über Lade().
        }

        /// <summary>Einträge aus dem Einstellungs-Cache (neu) laden.
        /// Mandanten überschreiben das (App-Profil statt Cache).</summary>
        public virtual void Lade()
        {
            Eintraege.Clear();
            foreach (var e in StammdatenVerwaltung.Lese(Sektion, Praefix))
                Eintraege.Add(new StammdatenEintragVM(e, this));
        }

        /// <summary>Schreibt die komplette Liste in Cache + ini-Sektion.
        /// Mandanten überschreiben das (der native Aufrufer persistiert).</summary>
        internal virtual void Persistiere()
        {
            StammdatenVerwaltung.Schreibe(Sektion, Praefix,
                Eintraege.Select(vm => vm.Modell).ToList());
        }

        public bool ListeVoll => Eintraege.Count >= StammdatenVerwaltung.MaxEintraege;

        /// <summary>Legt einen neuen Eintrag mit Icon + Property an (Ablauf wie
        /// CIconAuswahl::OnNeu: erst Icon, dann Property, dann anlegen) und
        /// liefert ihn selektiert zurück.</summary>
        public StammdatenEintragVM NeuAnlegen(int iconIndex, string property)
        {
            if (ListeVoll) return null;

            var modell = new StammdatenEintrag
            {
                Name = DefaultName(iconIndex, Eintraege.Count),
                Icon = iconIndex
            };
            if (!string.IsNullOrEmpty(property))
                modell.Werte[PropertySuffix] = property;

            var vm = new StammdatenEintragVM(modell, this);
            Eintraege.Add(vm);
            Persistiere();
            Auswahl = vm;
            Statusleiste.Melde($"\"{modell.Name}\" angelegt -- der Name kann direkt im Namensfeld geändert werden.");
            return vm;
        }

        /// <summary>Löscht den Eintrag. Anders als der alte MFC-Code (der den
        /// letzten Eintrag in die Lücke verschob) bleibt die Reihenfolge der
        /// übrigen Einträge erhalten. Buchungen behalten wie bisher den Namen
        /// als String -- sie werden nicht angefasst.</summary>
        public void Loeschen(StammdatenEintragVM eintrag)
        {
            if (eintrag == null) return;
            int idx = Eintraege.IndexOf(eintrag);
            if (idx < 0) return;

            string name = eintrag.Name;
            Eintraege.Remove(eintrag);
            Persistiere();
            Auswahl = Eintraege.Count == 0
                ? null
                : Eintraege[System.Math.Min(idx, Eintraege.Count - 1)];
            Statusleiste.Melde($"\"{name}\" gelöscht.");
        }

        /// <summary>Setzt das Icon des Eintrags und persistiert.</summary>
        public void SetzeIcon(StammdatenEintragVM eintrag, int iconIndex)
        {
            if (eintrag == null || iconIndex < 0) return;
            eintrag.IconIndex = iconIndex;
            Persistiere();
        }

        public string HoleProperty(StammdatenEintragVM eintrag)
            => eintrag != null && eintrag.Modell.Werte.TryGetValue(PropertySuffix, out var v)
                ? v : "";

        public void SetzeProperty(StammdatenEintragVM eintrag, string wert)
        {
            if (eintrag == null) return;
            eintrag.Modell.Werte[PropertySuffix] = wert ?? "";
            eintrag.MeldeToolTipGeaendert();
            Persistiere();
        }
    }

    // ----------------------------------------------------------------------

    public sealed class BetriebeVerwaltenViewModel : StammdatenVerwaltenViewModel
    {
        public override string Titel => "Betrieb auswählen";
        public override string NeuKnopfText => "Neuer Betrieb...";
        public override string PropertyKnopfText => "Betriebsspezifische Daten...";
        internal override string Sektion => "Betriebe";
        internal override string Praefix => "Betrieb";
        internal override BitmapSource Sprite => ViewHost.SpriteBetriebe;
        internal override string[] IconNamen => IconKatalog.Betriebe;
        internal override string PropertySuffix => "Unternehmensart";

        // Wie CIconAuswahlBetrieb::GetIconText: "Betrieb <lfd. Nummer>"
        internal override string DefaultName(int iconIndex, int listenIndex)
            => "Betrieb " + (listenIndex + 1).ToString(CultureInfo.InvariantCulture);

        internal override bool FrageProperty(System.Windows.Window owner,
            string name, string aktuellerWert, out string neuerWert)
        {
            var vm = new UnternehmensartViewModel(name, aktuellerWert);
            var dlg = new UnternehmensartView(vm) { Owner = owner };
            if (dlg.ShowDialog() == true)
            {
                neuerWert = vm.Rohwert;
                return true;
            }
            neuerWert = aktuellerWert;
            return false;
        }
    }

    public sealed class BestandskontenVerwaltenViewModel : StammdatenVerwaltenViewModel
    {
        private readonly int _buchungsjahr;

        /// <param name="buchungsjahr">Buchungsjahr des aktiven Dokuments --
        /// der Anfangssaldo gilt fürs VORJAHR (SaldoJJJJ, wie
        /// CIconAuswahlBestandskonto::GetProperty).</param>
        public BestandskontenVerwaltenViewModel(int buchungsjahr)
        {
            _buchungsjahr = buchungsjahr;
        }

        public override string Titel => "Bestandskonto auswählen";
        public override string NeuKnopfText => "Neues Bestandskonto...";
        public override string PropertyKnopfText => "Anfangssaldo setzen...";
        internal override string Sektion => "Bestandskonten";
        internal override string Praefix => "Bestandskonto";
        internal override BitmapSource Sprite => ViewHost.SpriteBestandskonten;
        internal override string[] IconNamen => IconKatalog.Bestandskonten;
        internal override string PropertySuffix
            => "Saldo" + (_buchungsjahr - 1).ToString("D4", CultureInfo.InvariantCulture);

        // Wie CIconAuswahl::GetIconText (Default): Name des gewählten Icons
        internal override string DefaultName(int iconIndex, int listenIndex)
            => iconIndex >= 0 && iconIndex < IconNamen.Length
                ? IconNamen[iconIndex]
                : "Bestandskonto " + (listenIndex + 1).ToString(CultureInfo.InvariantCulture);

        internal override bool FrageProperty(System.Windows.Window owner,
            string name, string aktuellerWert, out string neuerWert)
        {
            var vm = new AnfangssaldoViewModel(name, _buchungsjahr - 1, aktuellerWert);
            var dlg = new AnfangssaldoView(vm) { Owner = owner };
            if (dlg.ShowDialog() == true)
            {
                neuerWert = vm.NormalisierterWert;
                return true;
            }
            neuerWert = aktuellerWert;
            return false;
        }
    }
}
