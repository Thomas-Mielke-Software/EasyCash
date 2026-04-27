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
            set
            {
                if (SetProperty(ref _datumTag, Math.Max(1, Math.Min(31, value))))
                    ValidiereFeldFallsAktiv(ValidiereDatum);
            }
        }

        private int _datumMonat;
        public int DatumMonat
        {
            get => _datumMonat;
            set
            {
                if (SetProperty(ref _datumMonat, Math.Max(1, Math.Min(12, value))))
                {
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereDatum);
                }
            }
        }

        private int _datumJahr;
        public int DatumJahr
        {
            get => _datumJahr;
            set
            {
                if (SetProperty(ref _datumJahr, value))
                    ValidiereFeldFallsAktiv(ValidiereDatum);
            }
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
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereBetrag);
                }
            }
        }

        /// <summary>Parseter Betrag in Cent.</summary>
        private int BetragInCent
        {
            get => ParseBetragInCent(_betragText);
        }

        /// <summary>
        /// Parst eine Geldbetrag-Eingabe. Akzeptiert:
        ///   - Deutsches Format: "1.234,56" (Punkt = Tausender, Komma = Dezimal)
        ///   - Englisches Format: "1,234.56" oder "119.00" (Punkt = Dezimal)
        ///   - Einfache Zahlen: "119" oder "119,5"
        /// Gibt 0 zurück wenn nicht parsbar.
        /// </summary>
        private static int ParseBetragInCent(string text)
        {
            if (string.IsNullOrWhiteSpace(text)) return 0;
            string s = text.Trim();

            // Heuristik: Wenn ein Komma vorkommt, ist es deutsches Format
            // (Komma = Dezimal). Punkte werden dann als Tausendertrenner
            // entfernt. Wenn KEIN Komma vorkommt, ist Punkt der Dezimal-
            // trenner (Invariant-Culture).
            decimal d;
            if (s.Contains(","))
            {
                // Deutsches Format: Tausenderpunkte entfernen, dann mit DeDE parsen
                string s2 = s.Replace(".", "");
                if (decimal.TryParse(s2, NumberStyles.Number, DeDE, out d))
                    return (int)decimal.Round(d * 100m, 0, MidpointRounding.AwayFromZero);
            }
            else
            {
                // Kein Komma: Punkt ist Dezimaltrenner (oder gar nichts)
                if (decimal.TryParse(s, NumberStyles.Number,
                        CultureInfo.InvariantCulture, out d))
                    return (int)decimal.Round(d * 100m, 0, MidpointRounding.AwayFromZero);
            }
            return 0;
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
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereMwst);
                }
            }
        }

        private int MwstPromille
        {
            get
            {
                if (string.IsNullOrWhiteSpace(_mwstText)) return 0;
                string s = _mwstText.Trim();
                decimal d;
                if (s.Contains(","))
                {
                    if (decimal.TryParse(s, NumberStyles.Number, DeDE, out d))
                        return (int)decimal.Round(d * 1000m, 0, MidpointRounding.AwayFromZero);
                }
                else
                {
                    if (decimal.TryParse(s, NumberStyles.Number,
                            CultureInfo.InvariantCulture, out d))
                        return (int)decimal.Round(d * 1000m, 0, MidpointRounding.AwayFromZero);
                }
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
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereMwst);
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
            set
            {
                if (SetProperty(ref _beschreibung, value ?? ""))
                    ValidiereFeldFallsAktiv(ValidiereBeschreibung);
            }
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
            set
            {
                if (SetProperty(ref _afaAktiviert, value))
                {
                    if (value)
                    {
                        // Beim Einschalten der AfA-Checkbox: passendes Konto
                        // vorschlagen falls noch keins gewählt + Restwert berechnen
                        WaehleAfaKontoFallsLeer();
                        BerechneRestwertHeuristisch();
                    }
                    else
                    {
                        AfaHinweis = "";
                    }
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                }
            }
        }

        public ObservableCollection<string> AfaJahreOptionen { get; } =
            new ObservableCollection<string>(
                Enumerable.Range(1, 50).Select(i => i.ToString()));

        private string _afaJahre = "1";
        public string AfaJahre
        {
            get => _afaJahre;
            set
            {
                if (SetProperty(ref _afaJahre, value ?? "1"))
                {
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                }
            }
        }

        private string _afaNr = "1";
        public string AfaNr
        {
            get => _afaNr;
            set
            {
                if (SetProperty(ref _afaNr, value ?? "1"))
                {
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                }
            }
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

        /// <summary>
        /// Hinweistext zur AfA-Heuristik (z.B. zur Restwert-Rekonstruktion
        /// oder zum degressiv-linear-Wechsel). Wird in der View in einer
        /// anderen Farbe als die Validierungsfehler angezeigt.
        /// </summary>
        private string _afaHinweis = "";
        public string AfaHinweis
        {
            get => _afaHinweis;
            private set => SetProperty(ref _afaHinweis, value);
        }

        private bool _afaDegressiv;
        public bool AfaDegressiv
        {
            get => _afaDegressiv;
            set
            {
                if (SetProperty(ref _afaDegressiv, value))
                {
                    PruefeDegressivWechsel();
                    BerechneRestwertHeuristisch(bewahreVorhandenenHinweis: true);
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                }
            }
        }

        private string _afaSatz = "0";
        public string AfaSatz
        {
            get => _afaSatz;
            set
            {
                if (SetProperty(ref _afaSatz, value ?? "0"))
                {
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                }
            }
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
        // AfA-Heuristik (live-getriggert in den Settern)
        // Reimplementiert die OnTimer(102)-Logik aus buchendlg.cpp.
        // Statt SetTimer/KillTimer-Verzögerung wird direkt bei jeder
        // Property-Änderung neu berechnet.
        // ══════════════════════════════════════════════

        /// <summary>
        /// Verhindert rekursive Berechnungen, wenn die Heuristik selbst
        /// eine Property setzt (z.B. AfaRestwertCent), die ihrerseits
        /// einen PropertyChanged-Trigger auslösen würde.
        /// </summary>
        private bool _restwertBerechnungLaeuft;

        /// <summary>
        /// Sucht in der Konten-Liste nach einem AfA-üblichen Konto und
        /// wählt es aus, falls noch keins gewählt ist und AfaJahre > 1.
        /// 1:1-Port der Logik aus buchendlg.cpp::OnTimer(102).
        /// </summary>
        private void WaehleAfaKontoFallsLeer()
        {
            if (!string.IsNullOrEmpty(SelectedKonto)) return;

            int.TryParse(_afaJahre, out int jahre);
            if (jahre <= 1) return;

            // Prioritätsliste wie im Original (substring match, case-sensitive)
            string[] kandidaten = {
                "Abschreibungen",
                "AfA",
                "AfA auf bewegliche Wirtschaftsgüter",
                "Abschreibung auf das Anlagevermögen (Afa, GWG)"
            };

            foreach (var kandidat in kandidaten)
            {
                var treffer = Konten.FirstOrDefault(k =>
                    !string.IsNullOrEmpty(k) && k.IndexOf(kandidat,
                        StringComparison.Ordinal) >= 0);
                if (treffer != null)
                {
                    SelectedKonto = treffer;
                    return;
                }
            }
        }

        /// <summary>
        /// Berechnet den Restwert heuristisch wie buchendlg.cpp::OnTimer(102).
        ///
        /// Der Algorithmus:
        ///   - Bei AfaNr == 1: Restwert = Netto (ganzer Anschaffungspreis)
        ///   - Bei AfaNr > 1: für die schon abgelaufenen Jahre wird der
        ///     jeweilige Jahresanteil simuliert subtrahiert. Das Ergebnis
        ///     ist eine Schätzung unter der Annahme linearer AfA — bei
        ///     degressiver AfA wird ein Hinweis angezeigt.
        ///   - Restwert wird bei 0 abgeschnitten (nie negativ).
        /// </summary>
        /// <param name="bewahreVorhandenenHinweis">
        /// Wenn true, wird ein bereits gesetzter AfaHinweis (z.B. von
        /// PruefeDegressivWechsel) erhalten und ggf. mit dem Restwert-
        /// Hinweis kombiniert. Wenn false, wird der Hinweis vor der
        /// Neuberechnung geleert.
        /// </param>
        private void BerechneRestwertHeuristisch(bool bewahreVorhandenenHinweis = false)
        {
            if (_restwertBerechnungLaeuft) return;
            if (!AfaAktiviert) { AfaHinweis = ""; return; }

            try
            {
                _restwertBerechnungLaeuft = true;

                // Bei Aufrufen, die NICHT vom Degressiv-Setter kommen,
                // wird der bestehende Hinweis verworfen — sonst bliebe
                // ein veralteter Degressiv-Hinweis stehen.
                if (!bewahreVorhandenenHinweis)
                    AfaHinweis = "";

                // Eingabedaten parsen (defensive defaults)
                int jahre = int.TryParse(_afaJahre, out var j) && j > 0 ? j : 1;
                int nr    = int.TryParse(_afaNr, out var n) && n > 0 ? n : 1;
                int satz  = int.TryParse(_afaSatz, out var s) && s > 0 ? s : 0;

                int monat = (DatumMonat >= 1 && DatumMonat <= 12) ? DatumMonat : 1;

                if (jahre <= 1)
                {
                    // Keine echte AfA — Restwert = 0, kein Hinweis
                    AfaRestwertCent = 0;
                    AfaHinweis = "";
                    return;
                }

                // Simulationsbuchung anlegen, die den Original-Algorithmus
                // schrittweise durchläuft.
                var simulation = new Buchung
                {
                    BruttoBetrag = Betrag.AusCent(BetragInCent, MwstPromille),
                    Datum = new DateTime(2000, monat, 1),
                    AfaJahre = jahre,
                    AfaNr = 1,
                    AfaDegressiv = AfaDegressiv,
                    AfaSatz = satz,
                    AfaGenauigkeit = AfaGenauigkeit.EntsprechendEinstellungen,
                    AfaRestwertCent = (int)Betrag.AusCent(
                        BetragInCent, MwstPromille).NettoInCent
                };

                // Jahre vor der aktuellen AfaNr "durchlaufen" und vom
                // Restwert abziehen.
                for (int i = 1; i < nr; i++)
                {
                    simulation.AfaNr = i;
                    long jahresAfa = AfaCalculator.GetBuchungsjahrNetto(simulation);
                    simulation.AfaRestwertCent -= (int)jahresAfa;
                }

                if (simulation.AfaRestwertCent < 0)
                    simulation.AfaRestwertCent = 0;

                AfaRestwertCent = simulation.AfaRestwertCent;

                // Hinweistext bei Rekonstruktion (nur wenn nr > 1, da bei
                // nr == 1 der Wert direkt aus Netto kommt und keine
                // Annahme nötig ist).
                //
                // Falls PruefeDegressivWechsel() bereits einen Hinweis
                // gesetzt hat, wird der hier nicht überschrieben — beide
                // Hinweise werden kombiniert. So bleibt der Eingabefluss
                // ungestört (anders als im Original mit MessageBoxen).
                string restwertHinweis = "";
                if (nr > 1 && !AfaDegressiv)
                {
                    restwertHinweis = $"Hinweis: Der Restwert von " +
                        $"{(AfaRestwertCent / 100m).ToString("N2", DeDE)} " +
                        $"wurde unter der Annahme rekonstruiert, dass das " +
                        $"Anlagegut von Anfang an linear abgeschrieben wurde. " +
                        $"Bei früher degressiver AfA ist der reale Restwert " +
                        $"wahrscheinlich niedriger und muss manuell angepasst " +
                        $"werden.";
                }

                // Existierenden Hinweis (z.B. aus PruefeDegressivWechsel)
                // mit dem Restwert-Hinweis kombinieren.
                if (string.IsNullOrEmpty(AfaHinweis))
                    AfaHinweis = restwertHinweis;
                else if (!string.IsNullOrEmpty(restwertHinweis))
                    AfaHinweis = AfaHinweis + "\n\n" + restwertHinweis;
                // Sonst: AfaHinweis bleibt unverändert (nur Degressiv-Hinweis)
            }
            finally
            {
                _restwertBerechnungLaeuft = false;
            }
        }

        /// <summary>
        /// Reimplementiert die OnTimer(103)-Logik aus buchendlg.cpp:
        /// wird beim Umschalten der Degressiv-Checkbox aufgerufen, um
        /// Hinweise zu unsinnigen Konfigurationen anzuzeigen.
        ///
        /// Im Original wurden hier MessageBoxen mit OK/Abbrechen-Logik
        /// genutzt. Hier wird das durch einen passiven Hinweistext
        /// ersetzt — das stört den Eingabefluss nicht.
        /// </summary>
        private void PruefeDegressivWechsel()
        {
            int.TryParse(_afaNr, out int nr);
            int.TryParse(_afaJahre, out int jahre);
            int.TryParse(_afaSatz, out int satz);

            // Nur relevant wenn dies eine Folgejahres-Buchung einer
            // bestehenden Buchung ist (nr > 1 + IstBearbeitung)
            if (nr > 1 && IstBearbeitung)
            {
                if (!AfaDegressiv)
                {
                    AfaHinweis = "Hinweis: EC&T stellt den Abschreibungsmodus " +
                        "in der Jahreswechsel-Funktion zum optimalen Zeitpunkt " +
                        "automatisch von degressiv auf linear um — eine manuelle " +
                        "Umstellung ist meist nicht nötig.";
                    return;
                }
                else if (satz <= 0 && jahre < 99)
                {
                    AfaHinweis = "Hinweis: Bei degressiver AfA ist ein " +
                        "Abschreibungssatz von 0 ungewöhnlich. Sinnvoll nur " +
                        "bei nicht-abnutzbaren Anlagegütern (z.B. Grundstücke); " +
                        "in dem Fall bitte 99 Jahre Abschreibungsdauer eintragen.";
                    return;
                }
            }
            // Sonst: Hinweis bleibt leer (oder wird von BerechneRestwertHeuristisch gesetzt)
        }

        // ══════════════════════════════════════════════
        // Command-Implementierungen
        // ══════════════════════════════════════════════

        private bool CanOk() => true;  // Validierung passiert in OnOk, nicht vorher

        private void OnOk()
        {
            // Validierung ab jetzt live — sonst würden Fehler nie verschwinden
            _validierungAktiv = true;

            if (!ValidiereAlles())
                return;  // Fenster bleibt offen, Fehler werden angezeigt

            // Datum zusammenbauen (mit evtl. expandiertem 2-stelligem Jahr)
            int jahr = ExpandiereJahr(DatumJahr);
            if (jahr != DatumJahr)
                DatumJahr = jahr;  // expandierten Wert sichtbar machen

            var datum = new DateTime(jahr, DatumMonat, DatumTag);

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

                // AfA — AfaRestwertCent kommt aus dem ViewModel-Wert, der
                // durch BerechneRestwertHeuristisch() live aktuell gehalten
                // wird. Bei deaktivierter AfA wird er auf 0 gesetzt.
                AfaJahre = AfaAktiviert && int.TryParse(AfaJahre, out var j) ? j : 1,
                AfaNr = int.TryParse(AfaNr, out var n) ? n : 1,
                AfaRestwertCent = AfaAktiviert ? AfaRestwertCent : 0,
                AfaDegressiv = AfaDegressiv,
                AfaSatz = int.TryParse(AfaSatz, out var s) ? s : 0,
            };

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
        // Validierung
        // Reimplementiert die Prüfungen aus buchendlg.cpp Zeilen 335-482
        // ══════════════════════════════════════════════

        /// <summary>
        /// Wird nach dem ersten OK-Klick auf true gesetzt.
        /// Ab dann laufen Validierungen live bei jeder Feldänderung,
        /// damit Fehler beim Korrigieren sofort verschwinden.
        /// </summary>
        private bool _validierungAktiv;

        // ── Fehler-Properties ──

        private string _datumError = "";
        public string DatumError
        {
            get => _datumError;
            private set => SetProperty(ref _datumError, value);
        }

        private string _betragError = "";
        public string BetragError
        {
            get => _betragError;
            private set => SetProperty(ref _betragError, value);
        }

        private string _mwstError = "";
        public string MwstError
        {
            get => _mwstError;
            private set => SetProperty(ref _mwstError, value);
        }

        private string _beschreibungError = "";
        public string BeschreibungError
        {
            get => _beschreibungError;
            private set => SetProperty(ref _beschreibungError, value);
        }

        private string _afaError = "";
        public string AfaError
        {
            get => _afaError;
            private set => SetProperty(ref _afaError, value);
        }

        // ── Validierungs-Methoden ──

        /// <summary>
        /// Führt alle Validierungen durch und setzt die Error-Properties.
        /// Gibt true zurück wenn alle Felder gültig sind.
        /// </summary>
        public bool ValidiereAlles()
        {
            bool ok = true;
            ok &= ValidiereDatum();
            ok &= ValidiereBetrag();
            ok &= ValidiereMwst();
            ok &= ValidiereBeschreibung();
            ok &= ValidiereAfa();
            return ok;
        }

        /// <summary>Expandiert 2-stellige Jahreszahlen (25 → 2025, 98 → 1998).</summary>
        private static int ExpandiereJahr(int j)
        {
            if (j >= 0 && j <= 37) return j + 2000;
            if (j > 37 && j < 100) return j + 1900;
            return j;
        }

        private bool ValidiereDatum()
        {
            // Tag: 1 bis 31
            if (DatumTag < 1 || DatumTag > 31)
            {
                DatumError = "Tag muss zwischen 1 und 31 liegen.";
                return false;
            }

            // Monat: 1 bis 12
            if (DatumMonat < 1 || DatumMonat > 12)
            {
                DatumError = "Monat muss zwischen 1 und 12 liegen.";
                return false;
            }

            // Jahr: 2-stellig wird auf 4-stellig expandiert, dann 1990-3000
            int jahr = ExpandiereJahr(DatumJahr);
            if (jahr < 1990 || jahr > 3000)
            {
                DatumError = "Jahr muss zwischen 1990 und 3000 liegen.";
                return false;
            }

            // Prüfen, ob das Datum tatsächlich existiert (31.02. etc.)
            try
            {
                var _ = new DateTime(jahr, DatumMonat, DatumTag);
            }
            catch (ArgumentOutOfRangeException)
            {
                DatumError = $"Ungültiges Datum: {DatumTag}.{DatumMonat}.{jahr}";
                return false;
            }

            DatumError = "";
            return true;
        }

        private bool ValidiereBetrag()
        {
            if (string.IsNullOrWhiteSpace(BetragText))
            {
                BetragError = "Bitte einen Betrag angeben.";
                return false;
            }
            if (BetragInCent == 0)
            {
                BetragError = "Der Betrag muss ungleich null sein.";
                return false;
            }
            BetragError = "";
            return true;
        }

        private bool ValidiereMwst()
        {
            if (!MwstAktiviert)
            {
                MwstError = "";
                return true;
            }

            if (string.IsNullOrWhiteSpace(MwstText))
            {
                MwstError = "Kein gültiger MWSt-Satz angegeben.";
                return false;
            }

            // Parse-Versuch wie in CBetrag::SetMWSt — culture-aware
            string s = MwstText.Trim();
            decimal wert;
            bool parsed = s.Contains(",")
                ? decimal.TryParse(s, NumberStyles.Number, DeDE, out wert)
                : decimal.TryParse(s, NumberStyles.Number,
                        CultureInfo.InvariantCulture, out wert);

            if (!parsed)
            {
                MwstError = "MWSt-Satz ist keine gültige Zahl.";
                return false;
            }
            if (wert < 0 || wert > 100)
            {
                MwstError = "MWSt-Satz muss zwischen 0 und 100 liegen.";
                return false;
            }

            MwstError = "";
            return true;
        }

        private bool ValidiereBeschreibung()
        {
            if (string.IsNullOrWhiteSpace(Beschreibung))
            {
                BeschreibungError = "Bitte eine Beschreibung angeben.";
                return false;
            }
            BeschreibungError = "";
            return true;
        }

        private bool ValidiereAfa()
        {
            if (!AfaAktiviert)
            {
                AfaError = "";
                return true;
            }

            // Abschreibungsjahre parsen
            if (string.IsNullOrWhiteSpace(AfaJahre))
            {
                AfaError = "Kein Abschreibungszeitraum angegeben.";
                return false;
            }
            if (!int.TryParse(AfaJahre, out int jahre) || jahre < 1)
            {
                AfaError = "Gesamt-Abschreibungsdauer muss mindestens 1 sein.";
                return false;
            }

            // Abschreibungsnummer parsen
            if (string.IsNullOrWhiteSpace(AfaNr))
            {
                AfaError = "Kein Abschreibungsjahr angegeben.";
                return false;
            }
            if (!int.TryParse(AfaNr, out int nr) || nr < 1)
            {
                AfaError = "Laufende Abschreibungsnummer muss mindestens 1 sein.";
                return false;
            }

            // Nr darf Jahre um max. 1 übersteigen (Extra-Jahr bei nicht-ganzjähriger AfA)
            if (nr > jahre + 1)
            {
                AfaError = "Das aktuelle Abschreibungsjahr übersteigt den " +
                           "Abschreibungszeitraum um mehr als 1.";
                return false;
            }

            // Degressive AfA: Satz muss gesetzt sein, Jahre > 1
            if (AfaDegressiv)
            {
                if (string.IsNullOrWhiteSpace(AfaSatz))
                {
                    AfaError = "Kein Abschreibungssatz angegeben, obwohl degressiv ausgewählt.";
                    return false;
                }
                if (!int.TryParse(AfaSatz, out int satz) || satz <= 0)
                {
                    AfaError = "Abschreibungssatz muss eine positive Zahl sein.";
                    return false;
                }
                if (jahre <= 1)
                {
                    AfaError = "Bei degressiver AfA muss die Gesamt-Abschreibungsdauer größer als 1 sein.";
                    return false;
                }
            }

            AfaError = "";
            return true;
        }

        /// <summary>Wird aus dem Setter jedes überwachten Feldes aufgerufen.</summary>
        private void ValidiereFeldFallsAktiv(Func<bool> validator)
        {
            if (_validierungAktiv)
                validator();
        }

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
