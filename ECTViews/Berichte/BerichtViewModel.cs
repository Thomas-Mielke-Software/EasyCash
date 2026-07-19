// BerichtViewModel.cs - ViewModel der Formlos-Ansicht (Freestyle-EÜR,
// formlose USt-Erklärung, Kontenplan).
//
// Hält den aktuellen Bericht (ECTEngine-Modell) und projiziert ihn auf
// anzeigefertige Zeilen-VMs (BerichtZeileVm) mit vorberechneten
// Schrift-/Layout-Werten. Filter (Monatsfilter, Betrieb) kommen vom
// nativen Ribbon über ECT_BerichtAktualisiere -- WYSIWYG: Ansicht und
// Druck arbeiten auf demselben Bericht-Objekt.
//
// Zoom funktioniert wie im Journal: Schriftgroesse skaliert alle
// abgeleiteten Werte; die Zeilen-VMs werden dabei neu aufgebaut
// (Berichte sind klein, das ist billig).

using System;
using System.Collections.ObjectModel;
using System.Linq;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.Berichte
{
    /// <summary>Die vier Formlos-Berichte (Reihenfolge = Ribbon-Menü;
    /// Werte müssen mit ECT_BerichtEinbetten in der Bridge übereinstimmen).</summary>
    public enum BerichtTyp
    {
        FreestyleEuer = 0,
        UstErklaerung = 1,
        Kontenplan = 2,
        KontenplanMitFeldern = 3
    }

    public class BerichtViewModel : ViewModelBase
    {
        private BuchungsDocument _doc;
        private BerichtTyp _typ;
        private int _monatsfilter;
        private string _betriebFilter = "";

        /// <summary>Der zuletzt erzeugte Bericht (auch Druck-Quelle).</summary>
        public Bericht Bericht { get; private set; }

        public BerichtTyp Typ => _typ;

        public ObservableCollection<BerichtZeileVm> Zeilen { get; } =
            new ObservableCollection<BerichtZeileVm>();

        // ----------------------------------------------------------
        // Kopfbereich
        // ----------------------------------------------------------
        private string _titel = "";
        public string Titel
        {
            get => _titel;
            private set => SetProperty(ref _titel, value);
        }

        private string _untertitel = "";
        public string Untertitel
        {
            get => _untertitel;
            private set => SetProperty(ref _untertitel, value);
        }
        public bool HatUntertitel => !string.IsNullOrEmpty(_untertitel);

        private string _kopfLinks = "";
        public string KopfLinks
        {
            get => _kopfLinks;
            private set => SetProperty(ref _kopfLinks, value);
        }

        private string _kopfRechts = "";
        public string KopfRechts
        {
            get => _kopfRechts;
            private set => SetProperty(ref _kopfRechts, value);
        }

        // ----------------------------------------------------------
        // Schrift + Skalierung (Basis 13, wie das Journal)
        // ----------------------------------------------------------
        private const double Basis = 13.0;
        private double _schriftgroesse = Basis;

        public double Schriftgroesse
        {
            get => _schriftgroesse;
            set
            {
                if (Math.Abs(_schriftgroesse - value) > 0.01 && value > 0)
                {
                    _schriftgroesse = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(TitelSchriftgroesse));
                    OnPropertyChanged(nameof(WertSpaltenBreite));
                    BaueZeilen();   // Zeilen-VMs tragen skalierte Werte
                }
            }
        }

        private double Skala => _schriftgroesse / Basis;

        public double TitelSchriftgroesse => 17.0 * Skala;
        public double WertSpaltenBreite => 120.0 * Skala;

        public string Schriftart
        {
            get
            {
                var s = GlobaleEinstellungen.Bildschirmschrift;
                return string.IsNullOrWhiteSpace(s) ? "Segoe UI" : s;
            }
        }

        // ----------------------------------------------------------
        // Events (Verdrahtung in der Bridge, Muster JournalViewModel)
        // ----------------------------------------------------------
        public event Action<int> ZoomAendern;
        public void MeldeZoomAenderung(int deltaProzent) =>
            ZoomAendern?.Invoke(deltaProzent);

        public event Action DruckAnfordern;
        public void MeldeDruckAnforderung() => DruckAnfordern?.Invoke();

        // ----------------------------------------------------------
        // Aufbau
        // ----------------------------------------------------------

        public void Initialisiere(BuchungsDocument doc, BerichtTyp typ)
        {
            _doc = doc;
            _typ = typ;
        }

        /// <summary>
        /// Erzeugt den Bericht neu (Filter-Änderung, Buchungs-Änderung,
        /// Moduswechsel im Formlos-Menü). schriftgroesse &lt;= 0 = beibehalten.
        /// </summary>
        public void Aktualisiere(int monatsfilter, string betriebFilter,
            double schriftgroesse = 0)
        {
            _monatsfilter = monatsfilter;
            _betriebFilter = betriebFilter ?? "";
            if (schriftgroesse > 0) _schriftgroesse = schriftgroesse;

            switch (_typ)
            {
                case BerichtTyp.UstErklaerung:
                    Bericht = UstErklaerungBericht.Erzeuge(_doc, _monatsfilter);
                    break;
                case BerichtTyp.Kontenplan:
                    Bericht = KontenplanBericht.Erzeuge(mitFeldern: false);
                    break;
                case BerichtTyp.KontenplanMitFeldern:
                    Bericht = KontenplanBericht.Erzeuge(mitFeldern: true);
                    break;
                default:
                    Bericht = EuerBericht.Erzeuge(_doc, _monatsfilter, _betriebFilter);
                    break;
            }

            Titel = Bericht.Titel;
            Untertitel = Bericht.Untertitel;
            OnPropertyChanged(nameof(HatUntertitel));
            KopfLinks = string.Join(Environment.NewLine, Bericht.KopfLinks);
            KopfRechts = string.Join(Environment.NewLine, Bericht.KopfRechts);
            OnPropertyChanged(nameof(Schriftart));
            OnPropertyChanged(nameof(Schriftgroesse));
            OnPropertyChanged(nameof(TitelSchriftgroesse));
            OnPropertyChanged(nameof(WertSpaltenBreite));
            BaueZeilen();
        }

        /// <summary>Wechselt den Berichtstyp (Formlos-Menü) und baut neu.</summary>
        public void WechsleTyp(BerichtTyp typ)
        {
            _typ = typ;
            Aktualisiere(_monatsfilter, _betriebFilter);
        }

        private void BaueZeilen()
        {
            Zeilen.Clear();
            if (Bericht == null) return;
            foreach (var z in Bericht.Zeilen)
                Zeilen.Add(new BerichtZeileVm(z, _schriftgroesse, Skala));
        }
    }

    /// <summary>
    /// Anzeigefertige Berichtszeile: Text + bis zu drei rechtsbündige
    /// Wertspalten (rechtsbündig aufgefüllt: der letzte Wert steht immer
    /// in der rechtesten Spalte), plus vorberechnete Schrift-/Layout-Werte.
    /// </summary>
    public class BerichtZeileVm
    {
        public string Text { get; }
        public string Wert1 { get; }
        public string Wert2 { get; }
        public string Wert3 { get; }

        public double FontSize { get; }
        public System.Windows.FontWeight FontWeight { get; }
        public System.Windows.FontStyle FontStyle { get; }
        public System.Windows.Media.Brush Foreground { get; }
        public System.Windows.Thickness Margin { get; }
        public System.Windows.Thickness BorderThickness { get; }

        private static readonly System.Windows.Media.Brush GrauBrush =
            new System.Windows.Media.SolidColorBrush(
                System.Windows.Media.Color.FromRgb(0x60, 0x60, 0x60));

        public BerichtZeileVm(BerichtZeile zeile, double schrift, double skala)
        {
            Text = zeile.Text ?? "";

            var werte = zeile.Werte ?? Array.Empty<string>();
            Wert3 = werte.Length >= 1 ? werte[werte.Length - 1] : "";
            Wert2 = werte.Length >= 2 ? werte[werte.Length - 2] : "";
            Wert1 = werte.Length >= 3 ? werte[werte.Length - 3] : "";

            switch (zeile.Stil)
            {
                case BerichtStil.Ueberschrift:
                    FontSize = schrift * 1.2;
                    FontWeight = System.Windows.FontWeights.Bold;
                    FontStyle = System.Windows.FontStyles.Normal;
                    Foreground = System.Windows.Media.Brushes.Black;
                    break;
                case BerichtStil.Zwischentitel:
                case BerichtStil.Summe:
                    FontSize = schrift;
                    FontWeight = System.Windows.FontWeights.Bold;
                    FontStyle = System.Windows.FontStyles.Normal;
                    Foreground = System.Windows.Media.Brushes.Black;
                    break;
                case BerichtStil.Hinweis:
                    FontSize = schrift * 0.9;
                    FontWeight = System.Windows.FontWeights.Normal;
                    FontStyle = System.Windows.FontStyles.Italic;
                    Foreground = GrauBrush;
                    break;
                default:
                    FontSize = schrift;
                    FontWeight = System.Windows.FontWeights.Normal;
                    FontStyle = System.Windows.FontStyles.Normal;
                    Foreground = System.Windows.Media.Brushes.Black;
                    break;
            }

            double einzug = zeile.Einrueckung * 20.0 * skala;
            double oben = (zeile.Stil == BerichtStil.Ueberschrift) ? 10.0 * skala : 1.0;
            Margin = new System.Windows.Thickness(einzug, oben, 0, 1);
            BorderThickness = new System.Windows.Thickness(0,
                zeile.LinieOben ? 1 : 0, 0, zeile.LinieUnten ? 1 : 0);
        }
    }
}
