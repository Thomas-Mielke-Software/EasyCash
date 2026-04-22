// BuchungViewModel.cs — ViewModel für den Buchungseingabedialog
//
// Bildet alle Felder des MFC-BuchenDlg (buchendlg.cpp) in einem
// MVVM-konformen ViewModel ab, das direkt auf ECTEngine-Typen arbeitet.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Windows.Input;
using ECTEngine;

namespace ECTViews.ViewModels
{
    /// <summary>
    /// ViewModel für eine Buchungseingabe/-bearbeitung.
    ///
    /// Arbeitet direkt mit ECTEngine-Typen (Buchung, Betrag, etc.)
    /// ohne native MFC-Abhängigkeiten.
    ///
    /// Mapping zum Original-BuchenDlg:
    ///   IDC_EINNAHMEN/IDC_AUSGABEN   → IstAusgabe
    ///   IDC_DATUM_TAG/MONAT/JAHR     → DatumTag, DatumMonat, DatumJahr
    ///   IDC_BETRAG                   → BetragText
    ///   IDC_MWST                     → MwstText, MwstOptionen
    ///   IDC_BESCHREIBUNG             → Beschreibung, BeschreibungsHistorie
    ///   IDC_BELEGNUMMER              → Belegnummer
    ///   IDC_EURECHNUNGSPOSTEN        → SelectedKonto, Konten
    ///   IDC_ABSCHREIBUNGJAHRE        → AfaJahre
    ///   IDC_ABSCHREIBUNGNUMMER       → AfaNr
    ///   IDC_ABSCHREIBUNGSATZ         → AfaSatz
    ///   IDC_ABSCHREIBUNGDEGRESSIV    → AfaDegressiv
    ///   Bestandskonto ComboBox       → Bestandskonto
    ///   Betrieb ComboBox             → Betrieb
    /// </summary>
    public class BuchungViewModel : ViewModelBase
    {
        private static readonly CultureInfo DeDE = new CultureInfo("de-DE");

        // ──────────────────────────────────────────────
        // Zustand
        // ──────────────────────────────────────────────

        /// <summary>True = Bearbeitung einer bestehenden Buchung.</summary>
        private bool _istBearbeitung;
        public bool IstBearbeitung
        {
            get => _istBearbeitung;
            set
            {
                if (SetProperty(ref _istBearbeitung, value))
                    OnPropertyChanged(nameof(OkButtonText));  // abhängige Property benachrichtigen
            }
        }
        public string OkButtonText => IstBearbeitung ? "Speichern" : "Buchen";

        /// <summary>Das Ergebnis: die fertige Buchung (null wenn abgebrochen).</summary>
        public Buchung Ergebnis { get; private set; }

        /// <summary>True wenn der Dialog mit OK bestätigt wurde.</summary>
        public bool Bestaetigt { get; private set; }

        // ──────────────────────────────────────────────
        // Buchungsart
        // ──────────────────────────────────────────────

        private bool _istAusgabe;
        public bool IstAusgabe
        {
            get => _istAusgabe;
            set
            {
                if (SetProperty(ref _istAusgabe, value))
                {
                    OnPropertyChanged(nameof(IstEinnahme));
                    OnPropertyChanged(nameof(BuchungsartText));
                    LadeKonten();
                }
            }
        }

        public bool IstEinnahme
        {
            get => !_istAusgabe;
            set => IstAusgabe = !value;
        }

        public string BuchungsartText =>
            _istAusgabe ? "Ausgabe" : "Einnahme";

        // ──────────────────────────────────────────────
        // Datum (drei Felder wie im Original)
        // ──────────────────────────────────────────────

        private int _datumTag;
        public int DatumTag
        {
            get => _datumTag;
            set => SetProperty(ref _datumTag, Math.Max(1, Math.Min(31, value)));
        }

        private int _datumMonat;
        public int DatumMonat
        {
            get => _datumMonat;
            set => SetProperty(ref _datumMonat, Math.Max(1, Math.Min(12, value)));
        }

        private int _datumJahr;
        public int DatumJahr
        {
            get => _datumJahr;
            set => SetProperty(ref _datumJahr, value);
        }

        // ──────────────────────────────────────────────
        // Betrag
        // ──────────────────────────────────────────────

        private string _betragText = "0,00";
        public string BetragText
        {
            get => _betragText;
            set
            {
                if (SetProperty(ref _betragText, value))
                {
                    OnPropertyChanged(nameof(NettoText));
                    OnPropertyChanged(nameof(MwstBetragText));
                }
            }
        }

        /// <summary>Parseter Betrag in Cent.</summary>
        private int BetragInCent
        {
            get
            {
                if (string.IsNullOrWhiteSpace(_betragText)) return 0;
                string s = _betragText.Replace(".", "").Trim();
                if (decimal.TryParse(s, NumberStyles.Number, DeDE, out var d))
                    return (int)(d * 100m);
                return 0;
            }
        }

        public string NettoText
        {
            get
            {
                var betrag = Betrag.AusCent(BetragInCent, MwstPromille);
                return betrag.NettoWert.ToString("N2", DeDE);
            }
        }

        public string MwstBetragText
        {
            get
            {
                var betrag = Betrag.AusCent(BetragInCent, MwstPromille);
                return betrag.MwstBetrag.ToString("N2", DeDE);
            }
        }

        // ──────────────────────────────────────────────
        // Mehrwertsteuer
        // ──────────────────────────────────────────────

        public ObservableCollection<string> MwstOptionen { get; } =
            new ObservableCollection<string>
            {
                "0", "7", "19", "5", "16", "10,7"
            };

        private string _mwstText = "19";
        public string MwstText
        {
            get => _mwstText;
            set
            {
                if (SetProperty(ref _mwstText, value))
                {
                    OnPropertyChanged(nameof(NettoText));
                    OnPropertyChanged(nameof(MwstBetragText));
                }
            }
        }

        private int MwstPromille
        {
            get
            {
                if (string.IsNullOrWhiteSpace(_mwstText)) return 0;
                string s = _mwstText.Replace(".", "").Trim();
                if (decimal.TryParse(s, NumberStyles.Number, DeDE, out var d))
                    return (int)(d * 1000m);
                return 0;
            }
        }

        private bool _mwstAktiviert = true;
        public bool MwstAktiviert
        {
            get => _mwstAktiviert;
            set
            {
                if (SetProperty(ref _mwstAktiviert, value))
                {
                    if (!value) MwstText = "0";
                    OnPropertyChanged(nameof(NettoText));
                    OnPropertyChanged(nameof(MwstBetragText));
                }
            }
        }

        // ──────────────────────────────────────────────
        // Beschreibung
        // ──────────────────────────────────────────────

        private string _beschreibung = "";
        public string Beschreibung
        {
            get => _beschreibung;
            set => SetProperty(ref _beschreibung, value ?? "");
        }

        public ObservableCollection<string> BeschreibungsHistorie { get; } =
            new ObservableCollection<string>();

        // ──────────────────────────────────────────────
        // Belegnummer
        // ──────────────────────────────────────────────

        private string _belegnummer = "";
        public string Belegnummer
        {
            get => _belegnummer;
            set => SetProperty(ref _belegnummer, value ?? "");
        }

        // ──────────────────────────────────────────────
        // Konto (E/Ü-Rechnungsposten)
        // ──────────────────────────────────────────────

        public ObservableCollection<string> Konten { get; } =
            new ObservableCollection<string>();

        private string _selectedKonto = "";
        public string SelectedKonto
        {
            get => _selectedKonto;
            set => SetProperty(ref _selectedKonto, value ?? "");
        }

        // ──────────────────────────────────────────────
        // Abschreibung (AfA)
        // ──────────────────────────────────────────────

        private bool _afaAktiviert;
        public bool AfaAktiviert
        {
            get => _afaAktiviert;
            set => SetProperty(ref _afaAktiviert, value);
        }

        public ObservableCollection<string> AfaJahreOptionen { get; } =
            new ObservableCollection<string>(
                Enumerable.Range(1, 50).Select(i => i.ToString()));

        private string _afaJahre = "1";
        public string AfaJahre
        {
            get => _afaJahre;
            set => SetProperty(ref _afaJahre, value ?? "1");
        }

        private string _afaNr = "1";
        public string AfaNr
        {
            get => _afaNr;
            set => SetProperty(ref _afaNr, value ?? "1");
        }

        private int _afaRestwertCent;
        public int AfaRestwertCent
        {
            get => _afaRestwertCent;
            set
            {
                if (SetProperty(ref _afaRestwertCent, value))
                    OnPropertyChanged(nameof(AfaRestwertText));
            }
        }

        public string AfaRestwertText =>
            (AfaRestwertCent / 100m).ToString("N2", DeDE);

        private bool _afaDegressiv;
        public bool AfaDegressiv
        {
            get => _afaDegressiv;
            set => SetProperty(ref _afaDegressiv, value);
        }

        private string _afaSatz = "0";
        public string AfaSatz
        {
            get => _afaSatz;
            set => SetProperty(ref _afaSatz, value ?? "0");
        }

        // ──────────────────────────────────────────────
        // Bestandskonto und Betrieb
        // ──────────────────────────────────────────────

        public ObservableCollection<string> Bestandskonten { get; } =
            new ObservableCollection<string>();

        private string _bestandskonto = "";
        public string Bestandskonto
        {
            get => _bestandskonto;
            set => SetProperty(ref _bestandskonto, value ?? "");
        }

        public ObservableCollection<string> Betriebe { get; } =
            new ObservableCollection<string>();

        private string _betrieb = "";
        public string SelectedBetrieb
        {
            get => _betrieb;
            set => SetProperty(ref _betrieb, value ?? "");
        }

        // ──────────────────────────────────────────────
        // Commands
        // ──────────────────────────────────────────────

        public ICommand OkCommand { get; }
        public ICommand AbbrechenCommand { get; }

        // ──────────────────────────────────────────────
        // Referenz auf das Dokument (für Konten, Belegnummern, etc.)
        // ──────────────────────────────────────────────

        private readonly BuchungsDocument _doc;

        // ══════════════════════════════════════════════
        // Konstruktor
        // ══════════════════════════════════════════════

        /// <summary>
        /// Erstellt ein ViewModel für eine neue Buchung.
        /// </summary>
        public BuchungViewModel(BuchungsDocument doc, bool ausgaben = false)
        {
            _doc = doc ?? throw new ArgumentNullException(nameof(doc));
            IstBearbeitung = false;
            IstAusgabe = ausgaben;

            // Datum auf heute setzen
            var heute = DateTime.Today;
            _datumTag = heute.Day;
            _datumMonat = heute.Month;
            _datumJahr = doc.Jahr > 0 ? doc.Jahr : heute.Year;

            // Belegnummer vorbelegen
            _belegnummer = ausgaben
                ? doc.LaufendeBelegnrAusgaben.ToString()
                : doc.LaufendeBelegnrEinnahmen.ToString();

            OkCommand = new RelayCommand(OnOk, CanOk);
            AbbrechenCommand = new RelayCommand(OnAbbrechen);

            LadeKonten();
        }

        /// <summary>
        /// Erstellt ein ViewModel zur Bearbeitung einer bestehenden Buchung.
        /// </summary>
        public BuchungViewModel(BuchungsDocument doc, Buchung buchung)
            : this(doc, buchung.Art == Buchungsart.Ausgabe)
        {
            IstBearbeitung = true;

            // Felder aus der Buchung befüllen
            _datumTag = buchung.Datum.Day;
            _datumMonat = buchung.Datum.Month;
            _datumJahr = buchung.Datum.Year;
            _betragText = (buchung.BruttoBetrag.InCent / 100m).ToString("N2", DeDE);
            _mwstText = (buchung.BruttoBetrag.MwstPromille / 1000m).ToString(DeDE);
            _mwstAktiviert = buchung.BruttoBetrag.MwstPromille > 0;
            _beschreibung = buchung.Beschreibung;
            _belegnummer = buchung.Belegnummer;
            _selectedKonto = buchung.Konto;
            _bestandskonto = buchung.Bestandskonto;
            _betrieb = buchung.Betrieb;

            // AfA
            if (buchung.HatAfA)
            {
                _afaAktiviert = true;
                _afaJahre = buchung.AfaJahre.ToString();
                _afaNr = buchung.AfaNr.ToString();
                _afaRestwertCent = buchung.AfaRestwertCent;
                _afaDegressiv = buchung.AfaDegressiv;
                _afaSatz = buchung.AfaSatz.ToString();
            }
        }

        // ══════════════════════════════════════════════
        // Command-Implementierungen
        // ══════════════════════════════════════════════

        private bool CanOk() => BetragInCent != 0;

        private void OnOk()
        {
            // Datum zusammenbauen
            int tag = Math.Max(1, Math.Min(28, DatumTag)); // safe default
            try { tag = Math.Min(DateTime.DaysInMonth(DatumJahr, DatumMonat), DatumTag); }
            catch { }

            var datum = new DateTime(DatumJahr, DatumMonat, tag);

            Ergebnis = new Buchung
            {
                Art = IstAusgabe ? Buchungsart.Ausgabe : Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(BetragInCent, MwstPromille),
                Datum = datum,
                Beschreibung = Beschreibung,
                Belegnummer = Belegnummer,
                Konto = SelectedKonto,
                Bestandskonto = Bestandskonto,
                Betrieb = SelectedBetrieb,

                // AfA
                AfaJahre = AfaAktiviert && int.TryParse(AfaJahre, out var j) ? j : 1,
                AfaNr = int.TryParse(AfaNr, out var n) ? n : 1,
                AfaRestwertCent = AfaAktiviert ? AfaRestwertCent : 0,
                AfaDegressiv = AfaDegressiv,
                AfaSatz = int.TryParse(AfaSatz, out var s) ? s : 0,
            };

            // Restwert aus Nettobetrag berechnen, wenn neue Buchung mit AfA
            if (!IstBearbeitung && AfaAktiviert && Ergebnis.AfaJahre > 1)
            {
                Ergebnis.AfaRestwertCent =
                    (int)Ergebnis.BruttoBetrag.NettoInCent;
            }

            Bestaetigt = true;
            RequestClose?.Invoke();
        }

        private void OnAbbrechen()
        {
            Bestaetigt = false;
            Ergebnis = null;
            RequestClose?.Invoke();
        }

        /// <summary>Event zum Schließen des Fensters (vom View abonniert).</summary>
        public event Action RequestClose;

        // ══════════════════════════════════════════════
        // Hilfsmethoden
        // ══════════════════════════════════════════════

        private void LadeKonten()
        {
            Konten.Clear();
            var kontenArray = IstAusgabe
                ? _doc.AusgabenKonten
                : _doc.EinnahmenKonten;

            if (kontenArray != null)
            {
                foreach (var k in kontenArray)
                {
                    if (!string.IsNullOrEmpty(k))
                        Konten.Add(k);
                }
            }

            // Auch Konten aus bestehenden Buchungen hinzufügen
            var buchungskonten = _doc.Buchungen
                .Where(b => b.Art == (IstAusgabe ? Buchungsart.Ausgabe : Buchungsart.Einnahme))
                .Select(b => b.Konto)
                .Where(k => !string.IsNullOrEmpty(k))
                .Distinct()
                .Where(k => !Konten.Contains(k));

            foreach (var k in buchungskonten)
                Konten.Add(k);
        }

        /// <summary>
        /// Lädt Beschreibungs-Presets aus den Einstellungen.
        /// In der MFC-Version wurden diese über WritePrivateProfileString
        /// in der INI-Datei gespeichert. Hier werden sie extern übergeben.
        /// </summary>
        public void LadePresets(IEnumerable<string> presets)
        {
            BeschreibungsHistorie.Clear();
            foreach (var p in presets)
                BeschreibungsHistorie.Add(p);
        }

        /// <summary>
        /// Lädt die verfügbaren Betriebe und Bestandskonten.
        /// </summary>
        public void LadeBetriebe(IEnumerable<string> betriebe)
        {
            Betriebe.Clear();
            Betriebe.Add(""); // leerer Eintrag = kein Betrieb
            foreach (var b in betriebe)
                Betriebe.Add(b);
        }

        public void LadeBestandskonten(IEnumerable<string> konten)
        {
            Bestandskonten.Clear();
            Bestandskonten.Add("");
            foreach (var k in konten)
                Bestandskonten.Add(k);
        }
    }
}
