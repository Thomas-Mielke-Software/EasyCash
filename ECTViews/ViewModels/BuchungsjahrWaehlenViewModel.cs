using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using ECTEngine;

namespace ECTViews.ViewModels
{
    /// <summary>
    /// Welche Aktion der Benutzer im "Buchungsjahr wählen"-Dialog gewählt hat.
    /// Korrespondiert mit den int-Rückgabewerten des nativen Exports
    /// ECT_ShowBuchungsjahrWaehlenDialog (0/1/2).
    /// </summary>
    public enum BuchungsjahrAktion
    {
        Abbruch = 0,
        Neu = 1,
        Jahreswechsel = 2
    }

    /// <summary>
    /// Ein Eintrag in der Liste bestehender Jahres-Buchungsdateien.
    /// </summary>
    public sealed class BuchungsdateiItem
    {
        public string Name { get; set; }
        public string VollerPfad { get; set; }
        public override string ToString() => Name;
    }

    /// <summary>
    /// ViewModel des "Buchungsjahr wählen"-Dialogs. Wird beim Anlegen eines
    /// neuen Dokuments gezeigt. Bietet zwei Aktionen:
    ///   - Jahreswechsel aus einer bestehenden Buchungsdatei (empfohlen)
    ///   - Anlegen einer neuen, leeren Buchungsdatei (Jahr + Währung)
    ///
    /// Migration von ECTIFace/buchungsjahrwaehlen.cpp; die reine "Öffnen"-
    /// Aktion entfällt, bestehende Dateien dienen nur noch als
    /// Jahreswechsel-Quelle.
    /// </summary>
    public class BuchungsjahrWaehlenViewModel : ViewModelBase
    {
        /// <summary>Wird ausgelöst, wenn der Dialog geschlossen werden soll.</summary>
        public event Action RequestClose;

        // Vorjahr (= aktuelles Jahr - 1). Dessen JahrXXXX.eca ist die typische
        // Jahreswechsel-Quelle und wird in der Liste vorausgewählt.
        private readonly int _vorjahr;

        public BuchungsjahrWaehlenViewModel(int defaultJahr, string defaultWaehrung)
        {
            _jahrText = defaultJahr.ToString();
            _waehrung = defaultWaehrung ?? "EUR";
            _vorjahr = defaultJahr - 1;

            JahreswechselCommand = new RelayCommand(
                FuehreJahreswechselAus, () => AusgewaehlteDatei != null);
            NeuCommand = new RelayCommand(LegeNeuAn, () => NeuMoeglich);
            AbbrechenCommand = new RelayCommand(Abbrechen);
        }

        // ----------------------------------------------
        // Liste bestehender Buchungsdateien
        // ----------------------------------------------

        public ObservableCollection<BuchungsdateiItem> Dateien { get; }
            = new ObservableCollection<BuchungsdateiItem>();

        private BuchungsdateiItem _ausgewaehlteDatei;
        public BuchungsdateiItem AusgewaehlteDatei
        {
            get => _ausgewaehlteDatei;
            set => SetProperty(ref _ausgewaehlteDatei, value);
        }

        public bool BestandsdateienVorhanden => Dateien.Count > 0;
        public bool KeineDateienVorhanden => Dateien.Count == 0;

        /// <summary>Befüllt die Liste und wählt die jüngste Datei vor.</summary>
        public void SetzeDateien(System.Collections.Generic.IEnumerable<BuchungsdateiItem> items)
        {
            Dateien.Clear();
            foreach (var item in items)
                Dateien.Add(item);

            // Bevorzugt die Vorjahres-Datei JahrXXXX.eca vorauswählen (typische
            // Jahreswechsel-Quelle); sonst als Fallback die jüngste Datei
            // (durch aufsteigende Sortierung zuletzt).
            BuchungsdateiItem vorauswahl = null;
            string vorjahrName = "Jahr" + _vorjahr.ToString("D4") + ".eca";
            foreach (var item in Dateien)
            {
                if (string.Equals(item.Name, vorjahrName, StringComparison.OrdinalIgnoreCase))
                {
                    vorauswahl = item;
                    break;
                }
            }
            if (vorauswahl == null && Dateien.Count > 0)
                vorauswahl = Dateien[Dateien.Count - 1];

            AusgewaehlteDatei = vorauswahl;
            OnPropertyChanged(nameof(BestandsdateienVorhanden));
            OnPropertyChanged(nameof(KeineDateienVorhanden));
        }

        // ----------------------------------------------
        // Neue Buchungsdatei (Jahr + Währung)
        // ----------------------------------------------

        private string _jahrText;
        public string JahrText
        {
            get => _jahrText;
            set
            {
                if (SetProperty(ref _jahrText, value))
                    OnPropertyChanged(nameof(JahrFehler));
            }
        }

        private string _waehrung;
        public string Waehrung
        {
            get => _waehrung;
            set
            {
                if (SetProperty(ref _waehrung, value))
                    OnPropertyChanged(nameof(WaehrungFehler));
            }
        }

        /// <summary>Vorschläge für das Währungskürzel-Dropdown: EUR + die
        /// hartkodierten EZB-Fiat-Codes. Rein lokal -- KEIN API-Abruf, keine
        /// DSGVO-Abfrage. Die ComboBox bleibt editierbar (Freitext möglich).</summary>
        public IReadOnlyList<string> WaehrungsVorschlaege { get; } =
            new[] { "EUR" }.Concat(Waehrungsliste.StandardFiatCodes()).ToList();

        /// <summary>Geparstes Jahr; -1 wenn ungültig.</summary>
        public int Jahr => int.TryParse(JahrText, out var j) ? j : -1;

        public string JahrFehler =>
            (Jahr < 1900 || Jahr > 2035)
                ? "Bitte ein vierstelliges Jahr zwischen 1900 und 2035 eingeben."
                : "";

        public string WaehrungFehler =>
            (Waehrung != null && Waehrung.Length > 3)
                ? "Maximal 3 Zeichen (z.B. EUR, CHF)."
                : "";

        public bool NeuMoeglich => JahrFehler == "" && WaehrungFehler == "";

        // ----------------------------------------------
        // Ergebnis
        // ----------------------------------------------

        public BuchungsjahrAktion Aktion { get; private set; } = BuchungsjahrAktion.Abbruch;
        public bool Bestaetigt => Aktion != BuchungsjahrAktion.Abbruch;
        public string QuelldateiPfad { get; private set; }

        // ----------------------------------------------
        // Commands
        // ----------------------------------------------

        public RelayCommand JahreswechselCommand { get; }
        public RelayCommand NeuCommand { get; }
        public RelayCommand AbbrechenCommand { get; }

        public void FuehreJahreswechselAus()
        {
            if (AusgewaehlteDatei == null) return;
            Aktion = BuchungsjahrAktion.Jahreswechsel;
            QuelldateiPfad = AusgewaehlteDatei.VollerPfad;
            RequestClose?.Invoke();
        }

        private void LegeNeuAn()
        {
            if (!NeuMoeglich) return;
            Aktion = BuchungsjahrAktion.Neu;
            RequestClose?.Invoke();
        }

        private void Abbrechen()
        {
            Aktion = BuchungsjahrAktion.Abbruch;
            RequestClose?.Invoke();
        }
    }
}
