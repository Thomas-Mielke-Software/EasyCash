// BuchungViewModel.cs -- ViewModel für den Buchungseingabedialog
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
    ///   IDC_EINNAHMEN/IDC_AUSGABEN   --> IstAusgabe
    ///   IDC_DATUM_TAG/MONAT/JAHR     --> DatumTag, DatumMonat, DatumJahr
    ///   IDC_BETRAG                   --> BetragText
    ///   IDC_MWST                     --> MwstText, MwstOptionen
    ///   IDC_BESCHREIBUNG             --> Beschreibung, BeschreibungsHistorie
    ///   IDC_BELEGNUMMER              --> Belegnummer
    ///   IDC_EURECHNUNGSPOSTEN        --> SelectedKonto, Konten
    ///   IDC_ABSCHREIBUNGJAHRE        --> AfaJahre
    ///   IDC_ABSCHREIBUNGNUMMER       --> AfaNr
    ///   IDC_ABSCHREIBUNGSATZ         --> AfaSatz
    ///   IDC_ABSCHREIBUNGDEGRESSIV    --> AfaDegressiv
    ///   Bestandskonto ComboBox       --> Bestandskonto
    ///   Betrieb ComboBox             --> Betrieb
    /// </summary>
    public partial class BuchungViewModel : ViewModelBase
    {
        private static readonly CultureInfo DeDE = new CultureInfo("de-DE");

        // ----------------------------------------------
        // Zustand
        // ----------------------------------------------

        /// <summary>True = Bearbeitung einer bestehenden Buchung.</summary>
        private bool _istBearbeitung;
        public bool IstBearbeitung
        {
            get => _istBearbeitung;
            set
            {
                if (SetProperty(ref _istBearbeitung, value))
                {
                    OnPropertyChanged(nameof(OkButtonText));          // abhängige Property benachrichtigen
                    OnPropertyChanged(nameof(WeiterbuchenSichtbar));
                }
            }
        }

        /// <summary>
        /// Sichtbarkeit des "Weiterbuchen"-Buttons: nur beim Anlegen einer
        /// neuen Buchung sinnvoll. Beim Bearbeiten/Kopieren schliesst der
        /// Dialog wie gehabt nach dem Speichern (kein Weiterbuchen).
        /// </summary>
        public bool WeiterbuchenSichtbar => !IstBearbeitung;

        /// <summary>
        /// Beschriftung des OK-Buttons. Wechselt zwischen "Buchen" (neue
        /// Buchung) und "Speichern" (Bearbeitung). Über an IstBearbeitung
        /// gebunden -- dort feuert der Setter OnPropertyChanged für diese
        /// Property mit.
        /// </summary>
        public string OkButtonText => IstBearbeitung ? "Speichern" : "Buchen";

        /// <summary>Das Ergebnis: die fertige Buchung (null wenn abgebrochen).
        /// Bei einer Buchungsgruppe ist dies die BASIS-Buchung; alle
        /// Buchungen (inkl. Zusatz-Zeilen) stehen in
        /// <see cref="ErgebnisBuchungen"/>.</summary>
        public Buchung Ergebnis { get; private set; }

        /// <summary>Alle beim OK erzeugten Buchungen (Basis zuerst).
        /// Ohne Gruppen-Vorlage genau ein Element. Null wenn abgebrochen.</summary>
        public IReadOnlyList<Buchung> ErgebnisBuchungen { get; private set; }

        /// <summary>True wenn der Dialog mit OK bestätigt wurde.</summary>
        public bool Bestaetigt { get; private set; }

        // ----------------------------------------------
        // Buchungsart
        // ----------------------------------------------

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
                    LadePresets();   // Vorschlagsliste auf neue Buchungsart filtern
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

        // ----------------------------------------------
        // Datum (drei Felder wie im Original)
        // ----------------------------------------------

        private int _datumTag;
        public int DatumTag
        {
            // 0 = leeres Feld (noch nicht eingegeben); sonst auf 1..31 begrenzt.
            get => _datumTag;
            set
            {
                if (SetProperty(ref _datumTag, Math.Max(0, Math.Min(31, value))))
                    ValidiereFeldFallsAktiv(ValidiereDatum);
            }
        }

        private int _datumMonat;
        public int DatumMonat
        {
            // 0 = leeres Feld (noch nicht eingegeben); sonst auf 1..12 begrenzt.
            get => _datumMonat;
            set
            {
                if (SetProperty(ref _datumMonat, Math.Max(0, Math.Min(12, value))))
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

        // ----------------------------------------------
        // Betrag
        // ----------------------------------------------

        private string _betragText = Waehrungsformat.BetragOhneGruppierung(0m);
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
                    AktualisiereZusatzzeilen();
                }
            }
        }

        /// <summary>Parseter Betrag in Cent.</summary>
        private int BetragInCent
        {
            get => ParseBetragInCent(_betragText);
        }

        /// <summary>
        /// Parst eine Geldbetrag-Eingabe über den toleranten, locale-freien
        /// Parser (Waehrungsformat.TryParse): versteht deutsche ("1.234,56"),
        /// schweizerische ("1'234.56") und englische ("1,234.56")
        /// Schreibweisen. Gibt 0 zurück wenn nicht parsbar.
        /// </summary>
        private static int ParseBetragInCent(string text)
        {
            return Waehrungsformat.TryParse(text, out decimal d)
                ? (int)decimal.Round(d * 100m, 0, MidpointRounding.AwayFromZero)
                : 0;
        }

        public string NettoText
        {
            get
            {
                var betrag = Betrag.AusCent(BetragInCent, MwstPromille);
                return Waehrungsformat.Betrag(betrag.NettoWert);
            }
        }

        public string MwstBetragText
        {
            get
            {
                var betrag = Betrag.AusCent(BetragInCent, MwstPromille);
                return Waehrungsformat.Betrag(betrag.MwstBetrag);
            }
        }

        // ----------------------------------------------
        // Mehrwertsteuer
        // ----------------------------------------------

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
                // Ist das MWSt-Feld global ausgeblendet, enthaelt es immer 0
                // (kein Steueranteil) -- auch wenn z.B. ein Preset einen Satz
                // setzen wollte.
                if (!MwstFeldAktiviert) value = "0";
                if (SetProperty(ref _mwstText, value))
                {
                    OnPropertyChanged(nameof(NettoText));
                    OnPropertyChanged(nameof(MwstBetragText));
                    BerechneRestwertHeuristisch();
                    ValidiereFeldFallsAktiv(ValidiereMwst);
                    AktualisiereZusatzzeilen();
                }
            }
        }

        private int MwstPromille
        {
            get
            {
                return Waehrungsformat.TryParseProzent(_mwstText, out decimal d)
                    ? (int)decimal.Round(d * 1000m, 0, MidpointRounding.AwayFromZero)
                    : 0;
            }
        }

        // ----------------------------------------------
        // Globale Feld-Sichtbarkeit (aus den Einstellungen)
        //
        // "JahresfeldAktiviert" (Default 0) und "MwstFeldAktiviert" (Default 1)
        // steuern, ob Buchungsjahr- bzw. MWSt-Feld im Buchen-Dialog ueberhaupt
        // bearbeitbar sind. Sind sie aus, wird das jeweilige Feld disabled und
        // per Tooltip auf die Einstellungen verwiesen. Das MWSt-Feld enthaelt
        // dann in jedem Fall 0 (kein Steueranteil). Eine eigene MWSt-Checkbox
        // im Buchen-Dialog gibt es nicht mehr -- das regelt die Einstellung.
        // ----------------------------------------------

        /// <summary>True wenn das Buchungsjahr-Feld bearbeitet werden darf
        /// (Einstellung "Buchungsjahr-Feld anzeigen").</summary>
        public bool BuchungsjahrFeldAktiviert { get; }

        /// <summary>True wenn das MWSt-Feld benutzt werden darf
        /// (Einstellung "MWSt.-Feld anzeigen").</summary>
        public bool MwstFeldAktiviert { get; }

        /// <summary>
        /// Tooltip fuer das deaktivierte Buchungsjahr-Feld (null wenn aktiv,
        /// dann wird kein Tooltip angezeigt).
        /// </summary>
        public string BuchungsjahrFeldHinweis =>
            BuchungsjahrFeldAktiviert ? null
            : "Das Buchungsjahr-Feld ist über die Einstellungen -> Allgemein ausgeblendet. "
              + "Dort kann es bei Bedarf wieder eingeschaltet werden.";

        /// <summary>
        /// Tooltip fuer das deaktivierte MWSt-Feld (null wenn aktiv).
        /// </summary>
        public string MwstFeldHinweis =>
            MwstFeldAktiviert ? null
            : "Das MWSt.-Feld ist in den Einstellungen -> Allgemein ausgeblendet. "
              + "Dort kann es bei Bedarf wieder eingeschaltet werden.";

        /// <summary>
        /// Erzwingt MWSt=0, wenn das MWSt-Feld global ausgeblendet ist.
        /// Wird am Ende beider Konstruktoren aufgerufen, weil die Felder dort
        /// direkt (unter Umgehung der Setter) gesetzt werden.
        /// </summary>
        private void WendeMwstFeldEinstellungAn()
        {
            if (!MwstFeldAktiviert)
                _mwstText = "0";
        }

        // ----------------------------------------------
        // Beschreibung
        // ----------------------------------------------

        private string _beschreibung = "";
        public string Beschreibung
        {
            get => _beschreibung;
            set
            {
                if (SetProperty(ref _beschreibung, value ?? ""))
                {
                    ValidiereFeldFallsAktiv(ValidiereBeschreibung);
                    if (!_presetLaden)
                    {
                        if (VerarbeitePresetEingabe(_beschreibung))
                            VorschlaegeOffen = false;          // Preset geladen -> Liste zu
                        else
                            AktualisiereVorschlaege(_beschreibung);
                    }
                    AktualisiereZusatzzeilen();   // $B in Zeilen-Templates
                }
            }
        }

        /// <summary>Gefilterte Vorschlagsliste für das Beschreibungsfeld
        /// (Anzeige "NN  Text"). Beim Tippen wird nach Nummer ODER
        /// Beschreibungstext gefiltert.</summary>
        public ObservableCollection<PresetVorschlag> PresetVorschlaege { get; } =
            new ObservableCollection<PresetVorschlag>();

        // Alle Presets der aktuellen Buchungsart (ungefiltert) -- Basis für
        // die Filterung in AktualisiereVorschlaege.
        private readonly List<PresetVorschlag> _allePresetVorschlaege =
            new List<PresetVorschlag>();

        private bool _vorschlaegeOffen;
        /// <summary>Steuert das Aufklappen der Vorschlagsliste (Auto-Complete).</summary>
        public bool VorschlaegeOffen
        {
            get => _vorschlaegeOffen;
            set => SetProperty(ref _vorschlaegeOffen, value);
        }

        private PresetVorschlag _gewaehlterVorschlag;
        /// <summary>Vom Anwender in der Vorschlags-Liste angeklickter Eintrag.
        /// Beim Setzen wird das zugehörige Preset geladen und die Liste
        /// geschlossen. Danach auf null zurückgesetzt, damit derselbe Eintrag
        /// erneut wählbar ist.</summary>
        public PresetVorschlag GewaehlterVorschlag
        {
            get => _gewaehlterVorschlag;
            set
            {
                if (!SetProperty(ref _gewaehlterVorschlag, value) || value == null)
                    return;
                var presets = Einstellungen.Presets;
                if (value.Nummer >= 0 && value.Nummer < presets.Count)
                {
                    var p = presets[value.Nummer];
                    if (!p.IstLeer) LadePresetInFelder(p, value.Nummer);
                }
                VorschlaegeOffen = false;
                _gewaehlterVorschlag = null;
                OnPropertyChanged();
            }
        }

        /// <summary>
        /// Wird ausgelöst, wenn ein Preset mit Notiz geladen wurde. Die View
        /// zeigt den Notiztext dann ein paar Sekunden als Balloon.
        /// </summary>
        public event System.Action<string> PresetNotizAnzeigen;

        /// <summary>Slot (0-99) einer Buchungsvorlage, die beim Oeffnen des
        /// Dialogs automatisch geladen werden soll -- gesetzt beim Aufruf ueber
        /// das Ribbon-Dropdown der Einnahme-/Ausgabe-Knoepfe. -1 = keine Vorwahl.
        /// Die View wendet sie ueber <see cref="LadeVorgewaehlteVorlage"/> an,
        /// sobald das Fenster steht.</summary>
        public int VorgewaehltesPreset { get; set; } = -1;

        /// <summary>Laedt die per <see cref="VorgewaehltesPreset"/> gewaehlte
        /// Vorlage in die Felder (wie eine manuelle Auswahl aus der
        /// Vorschlagsliste). Wird von der View beim Laden aufgerufen, damit ein
        /// eventuell noetiger "Konto anlegen"-Dialog (Feld-Spezifikation) einen
        /// bereits sichtbaren Owner hat.</summary>
        public void LadeVorgewaehlteVorlage()
        {
            int slot = VorgewaehltesPreset;
            if (slot < 0) return;
            var presets = Einstellungen.Presets;
            if (slot >= presets.Count) return;
            var p = presets[slot];
            if (p.IstLeer || p.Ausgabe != _istAusgabe) return;
            LadePresetInFelder(p, slot);
        }

        // Verhindert Re-Entrancy, wenn das Laden eines Presets die
        // Beschreibung selbst auf den reinen Preset-Text setzt.
        private bool _presetLaden;

        /// <summary>
        /// Reagiert auf Eingaben im Beschreibungsfeld (wie im alten Buchen-
        /// Dialog):
        ///  - Zwei führende Ziffern (00-99): das Preset mit dieser Nummer wird
        ///    geladen, sofern es zur aktuellen Buchungsart passt.
        ///  - Exakter Treffer eines Preset-Texts (z.B. Auswahl aus der Liste):
        ///    das Preset wird ebenfalls geladen.
        /// "Laden" füllt Konto, MwSt und AfA-Jahre, setzt den reinen
        /// Preset-Text als Beschreibung und zeigt ggf. die Notiz als Balloon.
        /// </summary>
        /// <returns>true, wenn ein Preset geladen wurde.</returns>
        private bool VerarbeitePresetEingabe(string eingabe)
        {
            if (string.IsNullOrEmpty(eingabe)) return false;
            var presets = Einstellungen.Presets;

            // Fall 1: zwei führende Ziffern -> Preset-Nummer 00-99
            // (greift auch bei Auswahl aus der Liste, deren Anzeige "NN  Text" ist)
            if (eingabe.Length >= 2 && char.IsDigit(eingabe[0]) && char.IsDigit(eingabe[1]))
            {
                int idx = (eingabe[0] - '0') * 10 + (eingabe[1] - '0');
                if (idx >= 0 && idx < presets.Count)
                {
                    var p = presets[idx];
                    if (!p.IstLeer && p.Ausgabe == _istAusgabe)
                    {
                        LadePresetInFelder(p, idx);
                        return true;
                    }
                }
            }

            // Fall 2: exakter Treffer eines Preset-Texts der aktuellen Buchungsart
            for (int i = 0; i < presets.Count; i++)
            {
                var p = presets[i];
                if (p.IstLeer || p.Ausgabe != _istAusgabe) continue;
                if (string.Equals(p.Text, eingabe, System.StringComparison.OrdinalIgnoreCase))
                {
                    LadePresetInFelder(p, i);
                    return true;
                }
            }
            return false;
        }

        private void LadePresetInFelder(Preset p, int presetSlot)
        {
            // Vorschlags-Popup zuerst schliessen: das Auflösen einer
            // Feld-Spezifikation kann den modalen "Konto anlegen"-Dialog
            // öffnen, der sonst hinter dem noch offenen (immer obenauf
            // liegenden) Popup verschwindet.
            VorschlaegeOffen = false;

            _presetLaden = true;
            try
            {
                // Feld-Spezifikation ("$de:Formular=Id|...||") -> verknüpftes
                // Konto suchen bzw. anlegen lassen; sonst 1:1 übernehmen.
                SelectedKonto = LoeseKontoFeld(p.Konto);

                // MwstText-Setter erzwingt 0, falls das Feld global aus ist.
                MwstText = p.Mwst > 0
                    ? Waehrungsformat.Zahl(p.Mwst / 1000m)
                    : "0";

                if (p.AfaJ > 1)
                {
                    AfaJahre = p.AfaJ.ToString();
                    AfaDegressiv = p.Degressiv;
                    AfaSatz = p.AfaSatz.ToString(CultureInfo.InvariantCulture);
                    AfaAktiviert = true;
                }
                else
                {
                    AfaAktiviert = false;
                }
            }
            finally
            {
                _presetLaden = false;
            }

            // Beschreibung auf den reinen Preset-Text setzen -- VERZÖGERT:
            // Ein synchrones Setzen während der laufenden ComboBox-Texteingabe
            // (z.B. direkt nach Eingabe der 2. Ziffer) wird von WPF nicht in das
            // Editierfeld zurückgeschrieben (TwoWay-Binding-Re-Entrancy). Daher
            // per Dispatcher nach dem aktuellen Eingabe-Zyklus nachziehen.
            var beschreibungNeu = p.Text;
            System.Windows.Threading.Dispatcher.CurrentDispatcher.BeginInvoke(
                new Action(() =>
                {
                    _presetLaden = true;
                    try { Beschreibung = beschreibungNeu; }
                    finally { _presetLaden = false; }
                }),
                System.Windows.Threading.DispatcherPriority.Background);

            if (!string.IsNullOrWhiteSpace(p.Notiz))
                PresetNotizAnzeigen?.Invoke(p.Notiz);

            // Buchungsgruppen-Vorlage: Zusatz-Zeilen laden bzw. wegräumen.
            // Im Bearbeiten-Modus bewusst NICHT (der Bearbeiten-Pfad liefert
            // eine Einzelbuchung; Gruppen entstehen nur beim Neu-Buchen).
            if (!IstBearbeitung && p.IstMehrzeilig)
                UebernimmGruppenVorlage(p, presetSlot);
            else
                EntferneGruppenVorlage();
        }

        /// <summary>
        /// Baut die sichtbare Vorschlagsliste aus <see cref="_allePresetVorschlaege"/>
        /// neu auf, gefiltert nach dem aktuellen Eingabetext: beginnt dieser mit
        /// einer Ziffer, wird nach der Preset-Nummer gefiltert, sonst nach dem
        /// Beschreibungstext (enthält, Groß-/Kleinschreibung egal).
        /// </summary>
        private void AktualisiereVorschlaege(string filter, bool oeffnen = true)
        {
            var f = (filter ?? "").Trim();
            PresetVorschlaege.Clear();
            foreach (var v in _allePresetVorschlaege)
                if (PasstAufFilter(v, f))
                    PresetVorschlaege.Add(v);
            VorschlaegeOffen = oeffnen && f.Length > 0 && PresetVorschlaege.Count > 0;
        }

        private static bool PasstAufFilter(PresetVorschlag v, string f)
        {
            if (f.Length == 0) return true;
            if (char.IsDigit(f[0]))
                return v.Nummer.ToString("00").StartsWith(f, System.StringComparison.Ordinal);
            return v.Text.IndexOf(f, System.StringComparison.OrdinalIgnoreCase) >= 0;
        }

        /// <summary>Ein Eintrag der Beschreibungs-Vorschlagsliste: Preset-Nummer
        /// (Index 0-99) + Beschreibungstext. Anzeige als "NN  Text" -- die
        /// Nummer dient als Tipp-Kurzwahl.</summary>
        public class PresetVorschlag
        {
            public int    Nummer { get; set; }
            public string Text   { get; set; }
            public string Anzeige => Nummer.ToString("00") + "  " + Text;
        }

        // ----------------------------------------------
        // Belegnummer
        // ----------------------------------------------

        private string _belegnummer = "";
        public string Belegnummer
        {
            get => _belegnummer;
            set
            {
                if (SetProperty(ref _belegnummer, value ?? ""))
                    AktualisiereZusatzzeilen();   // $beleg in Zeilen-Templates
            }
        }

        // ----------------------------------------------
        // Konto (E/Ü-Rechnungsposten)
        // ----------------------------------------------

        public ObservableCollection<string> Konten { get; } =
            new ObservableCollection<string>();

        private string _selectedKonto = "";
        public string SelectedKonto
        {
            get => _selectedKonto;
            set => SetProperty(ref _selectedKonto, value ?? "");
        }

        // ----------------------------------------------
        // Abschreibung (AfA)
        // ----------------------------------------------

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
                        PruefeDegressivWechsel();
                        BerechneRestwertHeuristisch(bewahreVorhandenenHinweis: true);
                    }
                    else
                    {
                        AfaHinweis = "";
                    }
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                    OnPropertyChanged(nameof(AbgangButtonSichtbar));
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
                    PruefeDegressivWechsel();
                    BerechneRestwertHeuristisch(bewahreVorhandenenHinweis: true);
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                    OnPropertyChanged(nameof(AbgangButtonSichtbar));
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
                    PruefeDegressivWechsel();
                    BerechneRestwertHeuristisch(bewahreVorhandenenHinweis: true);
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                    OnPropertyChanged(nameof(AbgangButtonSichtbar));
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
                {
                    OnPropertyChanged(nameof(AfaRestwertText));
                    OnPropertyChanged(nameof(AbgangButtonSichtbar));
                }
            }
        }

        /// <summary>
        /// Editierbare Text-Repräsentation des Restwerts.
        /// Get: formatiert AfaRestwertCent als deutsche Währung.
        /// Set: parst die Eingabe (culture-aware) und schreibt in AfaRestwertCent.
        ///
        /// Das Restwert-Feld muss benutzer-editierbar sein, weil das Anlagegut
        /// anfangs degressiv abgeschrieben worden sein könnte. In dem Fall
        /// kann die Software die Abschreibungshistorie nicht rekonstruieren --
        /// der korrekte Restwert ist nur dem Benutzer bekannt.
        /// </summary>
        public string AfaRestwertText
        {
            get => Waehrungsformat.BetragOhneGruppierung(AfaRestwertCent / 100m);
            set
            {
                int neuerCent = ParseBetragInCent(value);
                if (neuerCent != _afaRestwertCent)
                {
                    // Direkt das Backing-Field schreiben, um keine
                    // BerechneRestwertHeuristisch()-Schleife auszulösen.
                    // _restwertBerechnungLaeuft schützt zusätzlich, falls
                    // dieser Setter durch eine Heuristik getriggert wird.
                    _afaRestwertCent = neuerCent;
                    OnPropertyChanged(nameof(AfaRestwertCent));
                    OnPropertyChanged(nameof(AfaRestwertText));
                    OnPropertyChanged(nameof(AbgangButtonSichtbar));
                }
            }
        }

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
                    PruefeDegressivWechsel();
                    BerechneRestwertHeuristisch(bewahreVorhandenenHinweis: true);
                    ValidiereFeldFallsAktiv(ValidiereAfa);
                }
            }
        }

        // ----------------------------------------------
        // AfA-Abgang ("Abgang buchen")
        //
        // Pendant zum gleichnamigen Button im MFC-BuchenDlg und zum
        // Kontextmenue-Eintrag "AfA-Abgang buchen" im WPF-Journal. Die
        // eigentliche Mutation passiert nativ in der Bridge (ECTBridge_-
        // FuehreAfaAbgang); das ViewModel signalisiert nur den Wunsch und
        // schliesst den Dialog. AbgangErlaubt wird vom ViewHost nur fuer
        // die echten Bearbeiten-Pfade gesetzt (nicht beim Kopieren/Neu).
        // ----------------------------------------------

        private bool _abgangErlaubt;
        public bool AbgangErlaubt
        {
            get => _abgangErlaubt;
            set
            {
                if (SetProperty(ref _abgangErlaubt, value))
                    OnPropertyChanged(nameof(AbgangButtonSichtbar));
            }
        }

        /// <summary>True wenn der Benutzer "Abgang buchen" geklickt hat.</summary>
        public bool AbgangGewuenscht { get; private set; }

        /// <summary>
        /// Sichtbarkeit des "Abgang buchen"-Buttons. Analog zur korrigierten
        /// Logik in buchendlg.cpp::InitRestwert (m_ppb && n > 1 && Restwert):
        /// nur bei Bearbeitung einer bestehenden, mehrjaehrigen AfA-Buchung in
        /// einem Folgejahr (Nr > 1) mit vorhandenem Restwert.
        /// </summary>
        public bool AbgangButtonSichtbar
        {
            get
            {
                if (!AbgangErlaubt || !AfaAktiviert) return false;
                int jahre = int.TryParse(_afaJahre, out var j) ? j : 0;
                int nr    = int.TryParse(_afaNr,   out var n) ? n : 0;
                return jahre > 1 && nr > 1 && AfaRestwertCent != 0;
            }
        }

        // ----------------------------------------------
        // Bestandskonto und Betrieb
        //
        // Listen mit Icon-Items (analog zu CListCtrl im MFC-Original).
        // Wenn die Listen leer bleiben, werden die zugehörigen UI-Elemente
        // ausgeblendet und der jeweilige String-Wert (Bestandskonto bzw.
        // SelectedBetrieb) bleibt unverändert beim Speichern.
        // ----------------------------------------------

        public ObservableCollection<IconListItem> Bestandskonten { get; } =
            new ObservableCollection<IconListItem>();

        public bool BestandskontenAnzeigen => Bestandskonten.Count > 0;

        /// <summary>True wenn mindestens eine der beiden Listen Inhalte hat.</summary>
        public bool ListenAnzeigen =>
            BestandskontenAnzeigen || BetriebeAnzeigen;

        private IconListItem _selectedBestandskonto;
        public IconListItem SelectedBestandskonto
        {
            get => _selectedBestandskonto;
            set
            {
                if (SetProperty(ref _selectedBestandskonto, value))
                    ValidiereFeldFallsAktiv(ValidiereBestandskonto);
            }
        }

        /// <summary>
        /// Initialer String-Wert (vor dem Befüllen der Liste) -- nur relevant
        /// wenn die Liste leer bleibt. Beim Speichern wird in diesem Fall
        /// der ursprüngliche Wert beibehalten (oder Leerstring bei neuer Buchung).
        /// </summary>
        private string _bestandskontoFallback = "";

        public ObservableCollection<IconListItem> Betriebe { get; } =
            new ObservableCollection<IconListItem>();

        public bool BetriebeAnzeigen => Betriebe.Count > 0;

        private IconListItem _selectedBetrieb;
        public IconListItem SelectedBetrieb
        {
            get => _selectedBetrieb;
            set
            {
                if (SetProperty(ref _selectedBetrieb, value))
                    ValidiereFeldFallsAktiv(ValidiereBetrieb);
            }
        }

        private string _betriebFallback = "";

        // Validierungsfehler für die zwei Listen
        private string _bestandskontoError = "";
        public string BestandskontoError
        {
            get => _bestandskontoError;
            private set => SetProperty(ref _bestandskontoError, value);
        }

        private string _betriebError = "";
        public string BetriebError
        {
            get => _betriebError;
            private set => SetProperty(ref _betriebError, value);
        }

        // ----------------------------------------------
        // Commands
        // ----------------------------------------------

        public ICommand OkCommand { get; }
        public ICommand AbbrechenCommand { get; }
        public ICommand AbgangBuchenCommand { get; }
        public ICommand WeiterbuchenCommand { get; }

        /// <summary>
        /// Wird beim Klick auf "Weiterbuchen" ausgeloest, sobald die
        /// Eingabe gueltig ist. Der native Aufrufer (ECTBridge) persistiert die
        /// uebergebene Buchung (Einfuegen, Sortieren, Sync, Modified-Flag),
        /// ohne dass der Dialog schliesst.
        /// </summary>
        public event Action<IReadOnlyList<Buchung>> GebuchtUndWeiter;

        /// <summary>
        /// Bittet die View, nach dem Weiterbuchen den Fokus zu setzen:
        /// true = Betrag-Feld (Einstellung "Tagesdatum einfügen und Cursor ins
        /// Betragsfeld"), false = Tag-Feld.
        /// </summary>
        public event Action<bool> RequestFokus;

        // ----------------------------------------------
        // Referenz auf das Dokument (für Konten, Belegnummern, etc.)
        // ----------------------------------------------

        private readonly BuchungsDocument _doc;

        // ----------------------------------------------
        // Konstruktor
        // ----------------------------------------------

        /// <summary>
        /// Erstellt ein ViewModel für eine neue Buchung.
        /// </summary>
        public BuchungViewModel(BuchungsDocument doc, bool ausgaben = false)
        {
            _doc = doc ?? throw new ArgumentNullException(nameof(doc));
            IstBearbeitung = false;

            // Globale Feld-Einstellungen lesen (Sektion [Allgemein] der easyct.ini).
            BuchungsjahrFeldAktiviert =
                Einstellungen.HoleBool("[Allgemein]JahresfeldAktiviert", false);
            MwstFeldAktiviert =
                Einstellungen.HoleBool("[Allgemein]MwstFeldAktiviert", true);

            IstAusgabe = ausgaben;

            // Startdatum beim Oeffnen: Tagesdatum nur, wenn "Tagesdatum
            // einfügen" an ist, sonst leere Tag/Monat-Felder mit vorbelegtem
            // Buchungsjahr. "Datum belassen" greift erst beim Weiterbuchen.
            SetzeFrischesDatum();

            // Belegnummer vorbelegen
            _belegnummer = ausgaben
                ? doc.LaufendeBelegnrAusgaben.ToString()
                : doc.LaufendeBelegnrEinnahmen.ToString();

            OkCommand = new RelayCommand(OnOk, CanOk);
            AbbrechenCommand = new RelayCommand(OnAbbrechen);
            AbgangBuchenCommand = new RelayCommand(OnAbgangBuchen, () => AbgangButtonSichtbar);
            WeiterbuchenCommand = new RelayCommand(OnWeiterbuchen);

            LadeKonten();
            LadePresets();

            // MWSt erzwingen (0/aus), falls das Feld global ausgeblendet ist.
            WendeMwstFeldEinstellungAn();
        }

        /// <summary>
        /// Erstellt ein ViewModel zur Bearbeitung einer bestehenden Buchung.
        /// </summary>
        public BuchungViewModel(BuchungsDocument doc, Buchung buchung)
            : this(doc, buchung.Art == Buchungsart.Ausgabe)
        {
            IstBearbeitung = true;
            _originalBuchung = buchung;   // fuer Erhalt von Erweiterungen/AfaGenauigkeit

            // Felder aus der Buchung befüllen
            _datumTag = buchung.Datum.Day;
            _datumMonat = buchung.Datum.Month;
            _datumJahr = buchung.Datum.Year;
            _betragText = Waehrungsformat.BetragOhneGruppierung(
                buchung.BruttoBetrag.InCent / 100m);
            _mwstText = Waehrungsformat.Zahl(
                buchung.BruttoBetrag.MwstPromille / 1000m);
            _beschreibung = buchung.Beschreibung;
            _belegnummer = buchung.Belegnummer;
            _selectedKonto = buchung.Konto;
            _bestandskontoFallback = buchung.Bestandskonto ?? "";
            _betriebFallback = buchung.Betrieb ?? "";

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

            // MWSt-Werte aus der Buchung ggf. wieder auf 0/aus zwingen, wenn das
            // Feld global ausgeblendet ist (der neue-Buchung-Konstruktor hat das
            // schon getan, hier wurden die Felder aber neu aus der Buchung gesetzt).
            WendeMwstFeldEinstellungAn();
        }

        // ----------------------------------------------
        // AfA-Heuristik (live-getriggert in den Settern)
        // Reimplementiert die OnTimer(102)-Logik aus buchendlg.cpp.
        // Statt SetTimer/KillTimer-Verzögerung wird direkt bei jeder
        // Property-Änderung neu berechnet.
        // ----------------------------------------------

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
        ///     ist eine Schätzung unter der Annahme linearer AfA -- bei
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
                // wird der bestehende Hinweis verworfen -- sonst bliebe
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
                    // Keine echte AfA -- Restwert = 0, kein Hinweis
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
                // gesetzt hat, wird der hier nicht überschrieben -- beide
                // Hinweise werden kombiniert. So bleibt der Eingabefluss
                // ungestört (anders als im Original mit MessageBoxen).
                string restwertHinweis = "";
                if (nr > 1 && !AfaDegressiv)
                {
                    restwertHinweis = $"Hinweis: Der Restwert von " +
                        $"{Waehrungsformat.Betrag(AfaRestwertCent / 100m)} " +
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
        /// wird beim Umschalten der Degressiv-Checkbox UND bei Änderungen
        /// am Satz/Jahre-Feld aufgerufen, um Hinweise zu unsinnigen
        /// Konfigurationen anzuzeigen.
        ///
        /// Im Original wurden hier MessageBoxen mit OK/Abbrechen-Logik
        /// genutzt. Hier wird das durch passive Hinweistexte ersetzt --
        /// das stört den Eingabefluss nicht.
        /// </summary>
        private void PruefeDegressivWechsel()
        {
            int.TryParse(_afaNr, out int nr);
            int.TryParse(_afaJahre, out int jahre);
            int.TryParse(_afaSatz, out int satz);

            // Hinweis 1: Bei degressiver AfA ist Satz=0 fast immer ein Fehler.
            // Sinnvoll nur bei nicht-abnutzbaren Anlagegütern (Grundstücke),
            // dort werden konventionell 99 Jahre Abschreibungsdauer eingetragen.
            // Diese Prüfung gilt unabhängig vom Abschreibungsjahr (nr) und
            // unabhängig davon, ob die Buchung neu oder bearbeitet wird --
            // denn die Inkonsistenz ist immer dieselbe.
            if (AfaDegressiv && satz <= 0 && jahre < 99)
            {
                AfaHinweis = "Hinweis: Bei degressiver AfA ist ein " +
                    "Abschreibungssatz von 0 ungewöhnlich. Sinnvoll nur " +
                    "bei nicht-abnutzbaren Anlagegütern (z.B. Grundstücke); " +
                    "in dem Fall bitte 99 Jahre Abschreibungsdauer eintragen.";
                return;
            }

            // Hinweis 2: Wenn bei einer Folgejahres-Buchung (nr > 1, also
            // bestehende AfA-Reihe) der Modus auf linear gesetzt wird,
            // ist das meist überflüssig -- EC&T regelt das automatisch.
            if (nr > 1 && IstBearbeitung && !AfaDegressiv)
            {
                AfaHinweis = "Hinweis: EC&T stellt den Abschreibungsmodus " +
                    "in der Jahreswechsel-Funktion zum optimalen Zeitpunkt " +
                    "automatisch von degressiv auf linear um -- eine manuelle " +
                    "Umstellung ist meist nicht nötig.";
                return;
            }
            // Sonst: Hinweis bleibt leer (oder wird von BerechneRestwertHeuristisch gesetzt)
        }

        // ----------------------------------------------
        // Command-Implementierungen
        // ----------------------------------------------

        private bool CanOk() => true;  // Validierung passiert in OnOk, nicht vorher

        private void OnOk()
        {
            // Validierung ab jetzt live -- sonst würden Fehler nie verschwinden
            _validierungAktiv = true;

            if (!ValidiereAlles())
                return;  // Fenster bleibt offen, Fehler werden angezeigt
            if (!ValidiereGruppe())
                return;  // Gruppen-Fehler (Formeln, fehlende Beträge)

            ErgebnisBuchungen = BaueAlleBuchungen();
            Ergebnis = ErgebnisBuchungen[0];

            Bestaetigt = true;
            RequestClose?.Invoke();
        }

        /// <summary>
        /// "Weiterbuchen": validiert wie OnOk, persistiert die Buchung ueber
        /// das <see cref="GebuchtUndWeiter"/>-Event (nativer Aufrufer), laesst
        /// den Dialog aber offen und initialisiert die Maske fuer die naechste
        /// Buchung neu -- unter Beachtung der "Weiterbuchen-Verhalten"-
        /// Einstellungen (Buchungsdatum belassen / Cursor ins Betragsfeld).
        /// </summary>
        private void OnWeiterbuchen()
        {
            _validierungAktiv = true;

            if (!ValidiereAlles())
                return;  // Fenster bleibt offen, Fehler werden angezeigt
            if (!ValidiereGruppe())
                return;  // Gruppen-Fehler (Formeln, fehlende Beträge)

            var buchungen = BaueAlleBuchungen();

            // Persistieren ueberlaesst der ViewModel dem nativen Aufrufer
            // (Einfuegen in die Engine, Sync, Modified-Flag). Geschieht
            // synchron, also sind die Buchungen danach im Dokument.
            GebuchtUndWeiter?.Invoke(buchungen);

            // Maske fuer die naechste Buchung vorbereiten und Fokus setzen.
            InitFuerNaechsteBuchung();
            RequestFokus?.Invoke(CursorInsBetragsfeld);
        }

        /// <summary>
        /// Baut aus den aktuellen Feldwerten eine <see cref="Buchung"/>.
        /// Erwartet, dass zuvor erfolgreich validiert wurde. Expandiert dabei
        /// ein evtl. 2-stelliges Jahr und macht den expandierten Wert sichtbar.
        /// </summary>
        // Beim Bearbeiten: die Original-Buchung, damit Felder, die der Dialog
        // nicht kennt (Plugin-Erweiterungen inkl. Buchungsgruppen-Keys,
        // AfaGenauigkeit), NICHT verloren gehen. Ohne diesen Erhalt bekam die
        // Ergebnis-Buchung einen leeren ErweiterungStore -- jedes Bearbeiten
        // zerlegte damit Split-/Gruppen-Verknuepfungen und Plugin-Daten.
        private readonly Buchung _originalBuchung;

        private Buchung BaueBuchungAusFeldern()
        {
            // Datum zusammenbauen (mit evtl. expandiertem 2-stelligem Jahr)
            int jahr = ExpandiereJahr(DatumJahr);
            if (jahr != DatumJahr)
                DatumJahr = jahr;  // expandierten Wert sichtbar machen

            var datum = new DateTime(jahr, DatumMonat, DatumTag);

            return new Buchung
            {
                Erweiterungen = _originalBuchung?.Erweiterungen.Clone()
                    ?? new ErweiterungStore(),
                AfaGenauigkeit = _originalBuchung?.AfaGenauigkeit
                    ?? AfaGenauigkeit.EntsprechendEinstellungen,
                Art = IstAusgabe ? Buchungsart.Ausgabe : Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(BetragInCent, MwstPromille),
                Datum = datum,
                Beschreibung = Beschreibung,
                Belegnummer = Belegnummer,
                Konto = SelectedKonto,
                // Bestandskonto/Betrieb:
                //   - Wenn Liste sichtbar: Name aus der Selektion
                //     (oder Leerstring wenn nichts gewählt)
                //   - Wenn Liste leer: Fallback-Wert beibehalten
                //     (also den ursprünglichen Wert aus der Buchung)
                Bestandskonto = BestandskontenAnzeigen
                    ? (SelectedBestandskonto?.Name ?? "")
                    : _bestandskontoFallback,
                Betrieb = BetriebeAnzeigen
                    ? (SelectedBetrieb?.Name ?? "")
                    : _betriebFallback,

                // AfA -- AfaRestwertCent kommt aus dem ViewModel-Wert, der
                // durch BerechneRestwertHeuristisch() live aktuell gehalten
                // wird. Bei deaktivierter AfA wird er auf 0 gesetzt.
                AfaJahre = AfaAktiviert && int.TryParse(AfaJahre, out var j) ? j : 1,
                AfaNr = int.TryParse(AfaNr, out var n) ? n : 1,
                AfaRestwertCent = AfaAktiviert ? AfaRestwertCent : 0,
                AfaDegressiv = AfaDegressiv,
                AfaSatz = int.TryParse(AfaSatz, out var s) ? s : 0,
            };
        }

        /// <summary>
        /// Setzt die Maske nach einem "Weiterbuchen" fuer die naechste Buchung
        /// zurueck. Port der "neu"-Logik aus buchendlg.cpp::InitDlg:
        ///   - Buchungsart, Betrieb- und Bestandskonto-Auswahl bleiben erhalten.
        ///   - Datum nur zuruecksetzen, wenn die Einstellung "Buchungsdatum der
        ///     letzten Buchung belassen" NICHT gesetzt ist.
        ///   - Betrag/MWSt/Beschreibung/AfA/Konto werden geleert bzw. auf die
        ///     Voreinstellungen gesetzt.
        ///   - Belegnummer wird wie bei einem frisch geoeffneten Dialog aus dem
        ///     Dokument uebernommen.
        /// </summary>
        private void InitFuerNaechsteBuchung()
        {
            // Live-Validierung zuruecksetzen -- die frische Maske soll keine
            // roten Fehler aus der eben gebuchten Eingabe zeigen.
            _validierungAktiv = false;

            // Gruppen-Vorlage nicht in die naechste Buchung uebernehmen.
            EntferneGruppenVorlage();

            // AfA zuruecksetzen (AfaAktiviert zuerst -> raeumt Hinweis ab).
            AfaAktiviert = false;
            AfaDegressiv = false;
            AfaJahre = "1";
            AfaNr = "1";
            AfaSatz = "0";
            AfaRestwertCent = 0;

            // Betrag, MWSt, Beschreibung leeren/voreinstellen.
            BetragText = Waehrungsformat.BetragOhneGruppierung(0m);
            MwstText = "19";          // Setter erzwingt 0, falls MWSt-Feld aus
            Beschreibung = "";
            VorschlaegeOffen = false;

            // Konto-Auswahl aufheben.
            SelectedKonto = "";

            // Betrieb/Bestandskonto: nur dann erhalten, wenn die jeweilige
            // Einstellung das Belassen erlaubt -- sonst Auswahl aufheben.
            if (!GlobaleEinstellungen.BetriebBelassen)
                SelectedBetrieb = null;
            if (!GlobaleEinstellungen.BestandskontoBelassen)
                SelectedBestandskonto = null;

            // Datum: Ist "Buchungsdatum der letzten Buchung belassen" gesetzt,
            // wird das Datum der gerade gebuchten Buchung recycelt -- die
            // Felder bleiben einfach unangetastet. Das hat Vorrang vor dem
            // "Tagesdatum einfügen". Andernfalls die Datumsfelder wie bei einer
            // frisch geoeffneten Buchung vorbelegen (Tagesdatum oder leer).
            if (!GlobaleEinstellungen.BuchungsdatumBelassen)
                SetzeFrischesDatum();

            // Belegnummer wie bei einem frisch geoeffneten Dialog vorbelegen.
            Belegnummer = IstAusgabe
                ? _doc.LaufendeBelegnrAusgaben.ToString()
                : _doc.LaufendeBelegnrEinnahmen.ToString();

            // Etwaige stehengebliebene Fehlertexte loeschen.
            DatumError = ""; BetragError = ""; MwstError = "";
            BeschreibungError = ""; AfaError = "";
            BestandskontoError = ""; BetriebError = "";
        }

        /// <summary>
        /// Belegt die Datumsfelder einer frischen Buchung vor -- beim
        /// erstmaligen Oeffnen und nach "Weiterbuchen" (sofern dort das Datum
        /// nicht recycelt wird).
        ///
        ///   - Ist "Tagesdatum einfügen und Cursor ins Betragsfeld" gesetzt,
        ///     wird das heutige Datum vorgeneriert (bzw. der 31.12. des
        ///     Dokumentjahres, falls das Dokument ein anderes als das laufende
        ///     Jahr fuehrt -- wie buchendlg.cpp::InitDlg).
        ///   - Sonst bleiben Tag und Monat leer (0); nur das Buchungsjahr wird
        ///     aus dem Dokument vorbelegt.
        ///
        /// Die Einstellung "Buchungsdatum der letzten Buchung belassen" greift
        /// hier NICHT: sie recycelt das Datum beim Druck auf "Weiterbuchen"
        /// (siehe <see cref="InitFuerNaechsteBuchung"/>).
        /// </summary>
        private void SetzeFrischesDatum()
        {
            var heute = DateTime.Today;
            int docJahr = _doc.Jahr > 0 ? _doc.Jahr : heute.Year;

            if (GlobaleEinstellungen.TaeglichBuchen)
            {
                DateTime datum = (heute.Year == docJahr)
                    ? heute
                    : new DateTime(docJahr, 12, 31);
                DatumTag = datum.Day;
                DatumMonat = datum.Month;
                DatumJahr = datum.Year;
            }
            else
            {
                DatumTag = 0;       // leeres Feld
                DatumMonat = 0;     // leeres Feld
                DatumJahr = docJahr;
            }
        }

        /// <summary>
        /// True, wenn nach dem Oeffnen des Dialogs und nach "Weiterbuchen" der
        /// Cursor ins Betragsfeld springen soll -- Einstellung "Tagesdatum
        /// einfügen und Cursor ins Betragsfeld". Sonst landet der Fokus im
        /// Tag-Feld. Die View liest dies beim Laden und nach dem Weiterbuchen.
        /// </summary>
        public bool CursorInsBetragsfeld => GlobaleEinstellungen.TaeglichBuchen;

        private void OnAbbrechen()
        {
            Bestaetigt = false;
            Ergebnis = null;
            RequestClose?.Invoke();
        }

        /// <summary>
        /// "Abgang buchen": signalisiert dem nativen Aufrufer (ViewExports),
        /// dass fuer die bearbeitete Buchung ein AfA-Abgang durchgefuehrt
        /// werden soll, und schliesst den Dialog. Bestaetigung und die
        /// eigentliche Buchungs-Mutation passieren danach nativ (gleiche
        /// Logik wie der Journal-Kontextmenue-Eintrag).
        /// </summary>
        private void OnAbgangBuchen()
        {
            if (!AbgangButtonSichtbar) return;
            AbgangGewuenscht = true;
            Bestaetigt = false;   // kein normales Speichern
            Ergebnis = null;
            RequestClose?.Invoke();
        }

        /// <summary>Event zum Schließen des Fensters (vom View abonniert).</summary>
        public event Action RequestClose;

        // ----------------------------------------------
        // Validierung
        // Reimplementiert die Prüfungen aus buchendlg.cpp Zeilen 335-482
        // ----------------------------------------------

        /// <summary>
        /// Wird nach dem ersten OK-Klick auf true gesetzt.
        /// Ab dann laufen Validierungen live bei jeder Feldänderung,
        /// damit Fehler beim Korrigieren sofort verschwinden.
        /// </summary>
        private bool _validierungAktiv;

        // -- Fehler-Properties --

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

        // -- Validierungs-Methoden --

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
            ok &= ValidiereBestandskonto();
            ok &= ValidiereBetrieb();
            return ok;
        }

        /// <summary>
        /// Prüft, dass ein Bestandskonto gewählt ist -- aber nur wenn die
        /// Liste überhaupt befüllt wurde. Bei leerer Liste keine Validierung.
        /// </summary>
        private bool ValidiereBestandskonto()
        {
            if (!BestandskontenAnzeigen)
            {
                BestandskontoError = "";
                return true;
            }
            if (SelectedBestandskonto == null)
            {
                BestandskontoError = "Bitte ein Bestandskonto auswählen.";
                return false;
            }
            BestandskontoError = "";
            return true;
        }

        private bool ValidiereBetrieb()
        {
            if (!BetriebeAnzeigen)
            {
                BetriebError = "";
                return true;
            }
            if (SelectedBetrieb == null)
            {
                BetriebError = "Bitte einen Betrieb auswählen.";
                return false;
            }
            BetriebError = "";
            return true;
        }

        /// <summary>Expandiert 2-stellige Jahreszahlen (25 --> 2025, 98 --> 1998).</summary>
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
            // Ist das MWSt-Feld global ausgeblendet, enthaelt es zwangsweise 0
            // -- keine Validierung noetig.
            if (!MwstFeldAktiviert)
            {
                MwstError = "";
                return true;
            }

            if (string.IsNullOrWhiteSpace(MwstText))
            {
                MwstError = "Kein gültiger MWSt-Satz angegeben.";
                return false;
            }

            // toleranter, locale-freier Parse (Waehrungsformat)
            if (!Waehrungsformat.TryParseProzent(MwstText, out decimal wert))
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

        // ----------------------------------------------
        // Hilfsmethoden
        // ----------------------------------------------

        private void LadeKonten()
        {
            // Altes Konto merken, um nach dem Neuladen den ähnlichsten
            // Eintrag in der neuen Liste vorauswählen zu können.
            string altesKonto = SelectedKonto;

            Konten.Clear();
            var kontenListe = IstAusgabe
                ? Einstellungen.AusgabenKonten
                : Einstellungen.EinnahmenKonten;

            foreach (var k in kontenListe)
                if (!string.IsNullOrEmpty(k))
                    Konten.Add(k);

            // Auch Konten aus bestehenden Buchungen hinzufügen
            var buchungskonten = _doc.Buchungen
                .Where(b => b.Art == (IstAusgabe ? Buchungsart.Ausgabe : Buchungsart.Einnahme))
                .Select(b => b.Konto)
                .Where(k => !string.IsNullOrEmpty(k))
                .Distinct()
                .Where(k => !Konten.Contains(k));

            foreach (var k in buchungskonten)
                Konten.Add(k);

            // Vorauswahl nach Buchungsart-Wechsel: exakter Treffer bevorzugt,
            // sonst den Eintrag mit dem längsten gemeinsamen Präfix (von links).
            // Bei gleichem Präfix alphabetisch, damit das Ergebnis deterministisch
            // ist. Kein gemeinsamer Präfix (Länge 0) → Auswahl leer lassen.
            if (string.IsNullOrEmpty(altesKonto) || Konten.Count == 0)
            {
                SelectedKonto = "";
            }
            else if (Konten.Contains(altesKonto))
            {
                SelectedKonto = altesKonto;
            }
            else
            {
                var bester = Konten
                    .Select(k => new { Konto = k, Praefixlaenge = GemeinsamerPraefixLaenge(k, altesKonto) })
                    .OrderByDescending(x => x.Praefixlaenge)
                    .ThenBy(x => x.Konto)
                    .First();

                SelectedKonto = bester.Praefixlaenge >= 4 ? bester.Konto : "";
            }
        }

        /// <summary>
        /// Liefert die Länge des längsten gemeinsamen Präfixes zweier
        /// Strings (Groß-/Kleinschreibung ignoriert).
        /// </summary>
        private static int GemeinsamerPraefixLaenge(string a, string b)
        {
            int len = Math.Min(a.Length, b.Length);
            for (int i = 0; i < len; i++)
                if (char.ToLowerInvariant(a[i]) != char.ToLowerInvariant(b[i]))
                    return i;
            return len;
        }

        /// <summary>
        /// Baut die Beschreibungs-Vorschlagsliste (Auto-Complete) aus dem
        /// globalen Einstellungs-Cache auf -- gefiltert nach der aktuellen
        /// Buchungsart (Einnahmen/Ausgaben), wie im alten Buchen-Dialog.
        /// Wird im Konstruktor und bei jedem Buchungsart-Wechsel gerufen.
        /// </summary>
        public void LadePresets()
        {
            _allePresetVorschlaege.Clear();
            var presets = Einstellungen.Presets;
            for (int i = 0; i < presets.Count; i++)
            {
                var p = presets[i];
                if (p.IstLeer || p.Ausgabe != _istAusgabe
                    || string.IsNullOrEmpty(p.Text)) continue;
                _allePresetVorschlaege.Add(
                    new PresetVorschlag { Nummer = i, Text = p.Text });
            }
            // Sichtbare Liste (ungefiltert) aufbauen, ohne aufzuklappen.
            AktualisiereVorschlaege(_beschreibung, oeffnen: false);
        }

        /// <summary>
        /// Lädt die Liste der Betriebe mit zugehörigen Icons.
        /// </summary>
        /// <param name="namen">Namen der Betriebe (entspricht m_Betriebe[i].name).</param>
        /// <param name="iconIndizes">Icon-Indizes (int, entspricht m_Betriebe[i].icon).</param>
        /// <param name="sprite">Die Sprite-Bitmap mit den 32x32-Icons.</param>
        public void LadeBetriebe(
            System.Collections.Generic.IList<string> namen,
            System.Collections.Generic.IList<int> iconIndizes,
            System.Windows.Media.Imaging.BitmapSource sprite)
        {
            Betriebe.Clear();
            if (namen == null || namen.Count == 0)
            {
                OnPropertyChanged(nameof(BetriebeAnzeigen)); OnPropertyChanged(nameof(ListenAnzeigen));
                return;
            }

            for (int i = 0; i < namen.Count; i++)
            {
                int idx = (iconIndizes != null && i < iconIndizes.Count) ? iconIndizes[i] : 0;

                Betriebe.Add(new IconListItem
                {
                    Name = namen[i],
                    IconIndex = idx,
                    Icon = IconSpriteSplitter.Crop(sprite, idx)
                });
            }
            OnPropertyChanged(nameof(BetriebeAnzeigen)); OnPropertyChanged(nameof(ListenAnzeigen));

            // Vorauswahl: den Eintrag selektieren, dessen Name dem Fallback-Wert
            // (aus dem Konstruktor übernommen) entspricht.
            if (!string.IsNullOrEmpty(_betriebFallback))
            {
                SelectedBetrieb = Betriebe.FirstOrDefault(
                    b => b.Name == _betriebFallback);
            }
        }

        /// <summary>
        /// Lädt die Liste der Bestandskonten mit Icons.
        /// </summary>
        public void LadeBestandskonten(
            System.Collections.Generic.IList<string> namen,
            System.Collections.Generic.IList<int> iconIndizes,
            System.Windows.Media.Imaging.BitmapSource sprite)
        {
            Bestandskonten.Clear();
            if (namen == null || namen.Count == 0)
            {
                OnPropertyChanged(nameof(BestandskontenAnzeigen)); OnPropertyChanged(nameof(ListenAnzeigen));
                return;
            }

            for (int i = 0; i < namen.Count; i++)
            {
                int idx = (iconIndizes != null && i < iconIndizes.Count) ? iconIndizes[i] : 0;

                Bestandskonten.Add(new IconListItem
                {
                    Name = namen[i],
                    IconIndex = idx,
                    Icon = IconSpriteSplitter.Crop(sprite, idx)
                });
            }
            OnPropertyChanged(nameof(BestandskontenAnzeigen)); OnPropertyChanged(nameof(ListenAnzeigen));

            if (!string.IsNullOrEmpty(_bestandskontoFallback))
            {
                SelectedBestandskonto = Bestandskonten.FirstOrDefault(
                    k => k.Name == _bestandskontoFallback);
            }
        }
    }
}
