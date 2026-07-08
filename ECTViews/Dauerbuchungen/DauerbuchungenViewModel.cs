// DauerbuchungenViewModel.cs -- Verwaltung der Dauerbuchungen eines
// Dokuments (WPF-Ersatz für DauerbuchungenDlg, dauerbuchungendlg.cpp).
//
// Arbeitet direkt auf ECTEngine.BuchungsDocument.Dauerbuchungen. Der
// Bridge-Aufrufer synchronisiert vorher Native->Managed und schreibt
// bei Geaendert==true nach Dialogende Managed->Native zurück.
//
// Zustandsmodell wie das Original: Liste + Neu/Ändern/Löschen sind
// aktiv, solange NICHT editiert wird; während der Bearbeitung sind nur
// die Eingabefelder + Übernehmen/Verwerfen aktiv. "Bereits ausgeführt
// bis" ist nur beim Ändern editierbar (Zurücksetzen erlaubt erneutes
// Ausführen).

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.Dauerbuchungen
{
    /// <summary>Eine Zeile der Dauerbuchungs-Liste (wie die alte
    /// Tab-Spalten-ListBox: Beschreibung, Betrag, Intervall, bis-Datum).</summary>
    public sealed class DauerbuchungZeileVM
    {
        private static readonly CultureInfo DeDE = new CultureInfo("de-DE");

        public Dauerbuchung Modell { get; }

        public DauerbuchungZeileVM(Dauerbuchung modell) { Modell = modell; }

        public string Beschreibung => Modell.Beschreibung;
        public string BetragText =>
            (Modell.BruttoBetrag.InCent / 100m).ToString("N2", DeDE);
        public string IntervallText
        {
            get
            {
                switch (Modell.Intervall)
                {
                    case Intervall.Monatlich:     return "monatlich";
                    case Intervall.ZweiMonatlich: return "2-monatlich";
                    case Intervall.Quartalsweise: return "pro Quartal";
                    case Intervall.Halbjaehrlich: return "halbjährlich";
                    default:                      return "jährlich";
                }
            }
        }
        public string BisText => "bis " + Modell.BisDatum.ToString("dd.MM.yyyy");
    }

    /// <summary>Ein Eintrag der Beschreibungs-Vorschlagsliste (Buchungs-Preset).</summary>
    public sealed class DauerbuchungPresetOption
    {
        public string Anzeige { get; set; }
        public Preset Preset { get; set; }
        public override string ToString() => Anzeige;
    }

    public class DauerbuchungenViewModel : ViewModelBase
    {
        private static readonly CultureInfo DeDE = new CultureInfo("de-DE");

        private readonly BuchungsDocument _doc;
        private Dauerbuchung _bearbeitet;   // bei Ändern: das Original in der Liste
        private bool _istNeu;

        /// <summary>True sobald mindestens eine Änderung übernommen wurde --
        /// der Bridge-Aufrufer synchronisiert dann zurück + SetModifiedFlag.</summary>
        public bool Geaendert { get; private set; }

        public string Titel { get; }

        public DauerbuchungenViewModel(BuchungsDocument doc, string dateiname = null)
        {
            _doc = doc;
            Titel = string.IsNullOrEmpty(dateiname)
                ? "Dauerbuchungen"
                : "Dauerbuchungen für " + dateiname;

            LadeMwstOptionen();
            LadeKontenUndPresets();
            LadeBetriebeUndBestandskonten();
            BaueListeAuf(null);
            SetzeEditorDefaults();
        }

        // ------------------------------------------------------------------
        // Liste
        // ------------------------------------------------------------------

        public ObservableCollection<DauerbuchungZeileVM> Zeilen { get; }
            = new ObservableCollection<DauerbuchungZeileVM>();

        private DauerbuchungZeileVM _auswahl;
        public DauerbuchungZeileVM Auswahl
        {
            get => _auswahl;
            set => SetProperty(ref _auswahl, value);
        }

        private void BaueListeAuf(Dauerbuchung selektieren)
        {
            Zeilen.Clear();
            DauerbuchungZeileVM neuAuswahl = null;
            foreach (var db in _doc.Dauerbuchungen)
            {
                var zeile = new DauerbuchungZeileVM(db);
                Zeilen.Add(zeile);
                if (db == selektieren) neuAuswahl = zeile;
            }
            Auswahl = neuAuswahl;
        }

        // ------------------------------------------------------------------
        // Editor-Zustand
        // ------------------------------------------------------------------

        private bool _bearbeitungAktiv;
        public bool BearbeitungAktiv
        {
            get => _bearbeitungAktiv;
            private set
            {
                if (SetProperty(ref _bearbeitungAktiv, value))
                {
                    OnPropertyChanged(nameof(ListeAktiv));
                    OnPropertyChanged(nameof(AktFelderAktiv));
                }
            }
        }

        public bool ListeAktiv => !_bearbeitungAktiv;

        /// <summary>"Bereits ausgeführt bis" nur beim Ändern editierbar.</summary>
        public bool AktFelderAktiv => _bearbeitungAktiv && !_istNeu;

        /// <summary>Neu anlegen: Editor mit Defaults öffnen.</summary>
        public void NeuStarten()
        {
            _istNeu = true;
            _bearbeitet = null;
            SetzeEditorDefaults();
            BearbeitungAktiv = true;
            OnPropertyChanged(nameof(AktFelderAktiv));
        }

        /// <summary>Ändern: Editor mit den Werten der Auswahl füllen.
        /// False, wenn nichts selektiert ist.</summary>
        public bool AendernStarten()
        {
            var zeile = _auswahl;
            if (zeile == null) return false;
            var db = zeile.Modell;

            _istNeu = false;
            _bearbeitet = db;

            _istEinnahme = db.Art == Buchungsart.Einnahme;
            _intervall = db.Intervall;
            LadeKontenUndPresets();

            VonMonatText = db.VonDatum.Month.ToString();
            VonJahrText  = db.VonDatum.Year.ToString();
            BisMonatText = db.BisDatum.Month.ToString();
            BisJahrText  = db.BisDatum.Year.ToString();
            TagText      = db.Buchungstag.ToString();
            AktMonatText = db.AusgefuehrtBis.Month.ToString();
            AktJahrText  = db.AusgefuehrtBis.Year.ToString();
            Beschreibung = db.Beschreibung;
            Belegnummer  = db.Belegnummer;
            BetragText   = (db.BruttoBetrag.InCent / 100m).ToString("N2", DeDE);
            MwstText     = (db.BruttoBetrag.MwstPromille / 1000m).ToString(DeDE);
            KontoAuswahl         = string.IsNullOrEmpty(db.Konto) ? null : db.Konto;
            BetriebAuswahl       = string.IsNullOrEmpty(db.Betrieb) ? null : db.Betrieb;
            BestandskontoAuswahl = string.IsNullOrEmpty(db.Bestandskonto) ? null : db.Bestandskonto;

            AlleEditorFelderMelden();
            BearbeitungAktiv = true;
            OnPropertyChanged(nameof(AktFelderAktiv));
            return true;
        }

        /// <summary>Löscht die selektierte Dauerbuchung (Liste ist nur
        /// außerhalb der Bearbeitung aktiv). False wenn keine Auswahl.</summary>
        public bool Loeschen()
        {
            var zeile = _auswahl;
            if (zeile == null) return false;
            _doc.Dauerbuchungen.Remove(zeile.Modell);
            Geaendert = true;
            BaueListeAuf(null);
            return true;
        }

        /// <summary>Bearbeitung abbrechen, Eingaben verwerfen.</summary>
        public void Verwerfen()
        {
            _istNeu = false;
            SetzeEditorDefaults();
            BearbeitungAktiv = false;
        }

        // ------------------------------------------------------------------
        // Editor-Felder
        // ------------------------------------------------------------------

        private bool _istEinnahme;
        public bool IstEinnahme
        {
            get => _istEinnahme;
            set
            {
                if (SetProperty(ref _istEinnahme, value))
                {
                    OnPropertyChanged(nameof(IstAusgabe));
                    LadeKontenUndPresets();
                }
            }
        }
        public bool IstAusgabe
        {
            get => !_istEinnahme;
            set => IstEinnahme = !value;
        }

        private Intervall _intervall = Intervall.Monatlich;
        private void SetzeIntervall(Intervall wert, bool gesetzt)
        {
            if (!gesetzt || _intervall == wert) return;
            _intervall = wert;
            OnPropertyChanged(nameof(IntervallMonatlich));
            OnPropertyChanged(nameof(IntervallZweiMonatlich));
            OnPropertyChanged(nameof(IntervallQuartal));
            OnPropertyChanged(nameof(IntervallHalbjahr));
            OnPropertyChanged(nameof(IntervallJahr));
        }
        public bool IntervallMonatlich     { get => _intervall == Intervall.Monatlich;     set => SetzeIntervall(Intervall.Monatlich, value); }
        public bool IntervallZweiMonatlich { get => _intervall == Intervall.ZweiMonatlich; set => SetzeIntervall(Intervall.ZweiMonatlich, value); }
        public bool IntervallQuartal       { get => _intervall == Intervall.Quartalsweise; set => SetzeIntervall(Intervall.Quartalsweise, value); }
        public bool IntervallHalbjahr      { get => _intervall == Intervall.Halbjaehrlich; set => SetzeIntervall(Intervall.Halbjaehrlich, value); }
        public bool IntervallJahr          { get => _intervall == Intervall.Jaehrlich;     set => SetzeIntervall(Intervall.Jaehrlich, value); }

        private string _vonMonatText = "";
        public string VonMonatText { get => _vonMonatText; set => SetProperty(ref _vonMonatText, value); }
        private string _vonJahrText = "";
        public string VonJahrText { get => _vonJahrText; set => SetProperty(ref _vonJahrText, value); }
        private string _bisMonatText = "";
        public string BisMonatText { get => _bisMonatText; set => SetProperty(ref _bisMonatText, value); }
        private string _bisJahrText = "";
        public string BisJahrText { get => _bisJahrText; set => SetProperty(ref _bisJahrText, value); }
        private string _tagText = "1";
        public string TagText { get => _tagText; set => SetProperty(ref _tagText, value); }
        private string _aktMonatText = "1";
        public string AktMonatText { get => _aktMonatText; set => SetProperty(ref _aktMonatText, value); }
        private string _aktJahrText = "2000";
        public string AktJahrText { get => _aktJahrText; set => SetProperty(ref _aktJahrText, value); }

        private string _betragText = "";
        public string BetragText { get => _betragText; set => SetProperty(ref _betragText, value); }

        private string _beschreibung = "";
        /// <summary>Beschreibungs-Template, max. 27 Zeichen (wie LimitText(27)
        /// im Original; die ComboBox kennt kein MaxLength, daher hier gekappt).
        /// Platzhalter: $J $j $q $h $2 $m $M $# sowie Monatsmodifikatoren
        /// $+m $++m $-m usw. -- die Auflösung passiert weiterhin nativ in
        /// CEasyCashView::DauerbuchungenAusfuehren.</summary>
        public string Beschreibung
        {
            get => _beschreibung;
            set
            {
                var neu = value ?? "";
                if (neu.Length > 27) neu = neu.Substring(0, 27);
                SetProperty(ref _beschreibung, neu);
            }
        }

        private string _belegnummer = "";
        public string Belegnummer { get => _belegnummer; set => SetProperty(ref _belegnummer, value ?? ""); }

        private string _mwstText = "19";
        public string MwstText { get => _mwstText; set => SetProperty(ref _mwstText, value); }

        public ObservableCollection<string> MwstOptionen { get; }
            = new ObservableCollection<string>();

        private string _kontoAuswahl;
        public string KontoAuswahl { get => _kontoAuswahl; set => SetProperty(ref _kontoAuswahl, value); }
        public ObservableCollection<string> KontenOptionen { get; }
            = new ObservableCollection<string>();

        private string _betriebAuswahl;
        public string BetriebAuswahl { get => _betriebAuswahl; set => SetProperty(ref _betriebAuswahl, value); }
        public ObservableCollection<string> BetriebeOptionen { get; }
            = new ObservableCollection<string>();

        private string _bestandskontoAuswahl;
        public string BestandskontoAuswahl { get => _bestandskontoAuswahl; set => SetProperty(ref _bestandskontoAuswahl, value); }
        public ObservableCollection<string> BestandskontenOptionen { get; }
            = new ObservableCollection<string>();

        // ------------------------------------------------------------------
        // Beschreibungs-Presets (Buchungsposten)
        // ------------------------------------------------------------------

        public ObservableCollection<DauerbuchungPresetOption> PresetOptionen { get; }
            = new ObservableCollection<DauerbuchungPresetOption>();

        private DauerbuchungPresetOption _gewaehltesPreset;
        /// <summary>Auswahl eines Buchungs-Presets übernimmt wie das Original
        /// Beschreibung, MWSt-Satz und E/Ü-Konto in die Felder.</summary>
        public DauerbuchungPresetOption GewaehltesPreset
        {
            get => _gewaehltesPreset;
            set
            {
                if (!SetProperty(ref _gewaehltesPreset, value) || value == null)
                    return;
                var p = value.Preset;
                Beschreibung = p.Text.Length > 27 ? p.Text.Substring(0, 27) : p.Text;
                MwstText = (p.Mwst / 1000m).ToString(DeDE);
                KontoAuswahl = string.IsNullOrEmpty(p.Konto) ? null : p.Konto;
            }
        }

        // ------------------------------------------------------------------
        // Übernehmen (Validierung wie DauerbuchungenDlg::OnOK)
        // ------------------------------------------------------------------

        /// <summary>Validiert die Eingaben und übernimmt sie in die (neue
        /// oder bearbeitete) Dauerbuchung. Liefert null bei Erfolg, sonst
        /// den Fehlertext (die View zeigt ihn als MessageBox, wie das
        /// Original).</summary>
        public string Uebernehmen()
        {
            if (!ParseMonat(_vonMonatText, out int mv))
                return "Eingabefehler: Wert für Monat von 1 bis 12!";
            int jv = NormalisiereJahr(ParseInt(_vonJahrText));
            if (!ParseMonat(_bisMonatText, out int mb))
                return "Eingabefehler: Wert für Monat von 1 bis 12!";
            int jb = NormalisiereJahr(ParseInt(_bisJahrText));

            // CTime-Beschränkung des nativen Datenmodells (wie das Original;
            // die Bridge clamp't DateTime > 3000 sonst still auf 2000-01-01)
            if (jv > 3000)
                return "Betriebssystem-Beschränkung: Die Jahreszahl darf nicht größer als 3000 sein - sorry!";
            if (jb > 3000) { jb = 3000; BisJahrText = "3000"; }
            if (jv < 1971) jv = 1971;   // CTime beginnt 1970 (+1 Puffer für Zeitzonen)
            if (jb < 1971) jb = 1971;

            var von = new DateTime(jv, mv, 1);
            var bis = new DateTime(jb, mb, DateTime.DaysInMonth(jb, mb));
            if (von > bis)
                return "Von-Datum muss kleiner sein als Bis-Datum!";

            // "Bereits ausgeführt bis": beim Ändern editierbar/zurücksetzbar
            DateTime ausgefuehrtBis;
            if (!_istNeu)
            {
                if (!ParseMonat(_aktMonatText, out int ma))
                    return "Eingabefehler: Wert für Monat von 1 bis 12!";
                int ja = NormalisiereJahr(ParseInt(_aktJahrText));
                if (ja > 3000) ja = 3000;
                if (ja < 1971) ja = 1971;
                ausgefuehrtBis = new DateTime(ja, ma, 1);
                if (ausgefuehrtBis > bis)
                    return "Hinweis: Das Aktualisiert-Bis-Datum liegt nach dem Bis-Datum. " +
                        "Diese Dauerbuchung wird deshalb nicht dazu führen, dass reale Buchungen erzeugt werden.";
            }
            else
                ausgefuehrtBis = new DateTime(2000, 1, 1);

            if (string.IsNullOrWhiteSpace(_betragText))
                return "Keinen Betrag angegeben!";
            int cent = ParseBetragInCent(_betragText);

            if (string.IsNullOrWhiteSpace(_beschreibung))
                return "Keine Beschreibung angegeben!";

            if (!ParseMwstPromille(_mwstText, out int promille))
                return "Keinen gültigen MWSt-Satz angegeben!";

            int tag = ParseInt(_tagText);
            if (tag < 1) tag = 1;
            if (tag > 28) tag = 28;   // wie das Original (28 = in jedem Monat gültig)

            var db = _istNeu ? new Dauerbuchung() : _bearbeitet;
            db.Art = _istEinnahme ? Buchungsart.Einnahme : Buchungsart.Ausgabe;
            db.Intervall = _intervall;
            db.VonDatum = von;
            db.BisDatum = bis;
            db.Buchungstag = tag;
            db.AusgefuehrtBis = ausgefuehrtBis;
            db.BruttoBetrag = Betrag.AusCent(cent, promille);
            db.Beschreibung = _beschreibung.Trim();
            db.Belegnummer = _belegnummer.Trim();
            db.Konto = _kontoAuswahl ?? "";
            db.Betrieb = _betriebAuswahl ?? "";
            db.Bestandskonto = _bestandskontoAuswahl ?? "";

            if (_istNeu)
                _doc.Dauerbuchungen.Add(db);

            Geaendert = true;
            _istNeu = false;
            _bearbeitet = null;
            BaueListeAuf(db);
            BearbeitungAktiv = false;
            return null;
        }

        // ------------------------------------------------------------------
        // Defaults + Listen-Beschaffung
        // ------------------------------------------------------------------

        private void SetzeEditorDefaults()
        {
            var jetzt = DateTime.Now;
            _istEinnahme = false;   // Default Ausgaben (wie InitCtrls)
            _intervall = Intervall.Monatlich;
            _vonMonatText = jetzt.Month.ToString();
            _vonJahrText  = jetzt.Year.ToString();
            _bisMonatText = "12";
            _bisJahrText  = (jetzt.Year + 50).ToString();
            _tagText      = "1";
            _aktMonatText = "1";
            _aktJahrText  = "2000";
            _beschreibung = "";
            _belegnummer  = "";
            _betragText   = "";
            _mwstText     = _defaultMwst;
            _kontoAuswahl = null;
            _betriebAuswahl = null;
            _bestandskontoAuswahl = null;
            _gewaehltesPreset = null;
            LadeKontenUndPresets();
            AlleEditorFelderMelden();
        }

        private void AlleEditorFelderMelden() => RaiseAllPropertiesChanged();

        private string _defaultMwst = "19";

        /// <summary>MWSt-Vorschläge aus den Einstellungen (vat1..vat4 in
        /// [Persoenliche_Daten]), wie CEasyCashView::GetVATs. vat1 ist der
        /// Default-Satz.</summary>
        private void LadeMwstOptionen()
        {
            MwstOptionen.Clear();
            for (int i = 1; i <= 4; i++)
            {
                var v = Einstellungen.Hole("[Persoenliche_Daten]vat" + i);
                if (!string.IsNullOrWhiteSpace(v) && !MwstOptionen.Contains(v))
                    MwstOptionen.Add(v);
            }
            if (MwstOptionen.Count == 0)
                foreach (var v in new[] { "0", "7", "19" })
                    MwstOptionen.Add(v);
            _defaultMwst = MwstOptionen[0];
        }

        /// <summary>E/Ü-Konten + Beschreibungs-Presets passend zur
        /// Buchungsart (wie UpdateCombo/UpdateBeschreibungCombo).</summary>
        private void LadeKontenUndPresets()
        {
            KontenOptionen.Clear();
            var konten = _istEinnahme
                ? Einstellungen.EinnahmenKonten
                : Einstellungen.AusgabenKonten;
            foreach (var k in konten)
                KontenOptionen.Add(k);

            PresetOptionen.Clear();
            _gewaehltesPreset = null;
            var presets = Einstellungen.Presets;
            for (int i = 0; i < presets.Count; i++)
            {
                var p = presets[i];
                if (p.IstLeer || p.Ausgabe == _istEinnahme) continue;
                PresetOptionen.Add(new DauerbuchungPresetOption
                {
                    Anzeige = i.ToString("D2") + " " + p.Text,
                    Preset = p
                });
            }
            OnPropertyChanged(nameof(GewaehltesPreset));
        }

        private void LadeBetriebeUndBestandskonten()
        {
            BetriebeOptionen.Clear();
            foreach (var b in Einstellungen.Betriebe)
                BetriebeOptionen.Add(b.Name);
            BestandskontenOptionen.Clear();
            foreach (var k in Einstellungen.Bestandskonten)
                BestandskontenOptionen.Add(k.Name);
        }

        // ------------------------------------------------------------------
        // Parsen/Normalisieren (wie die alten atoi-basierten Helfer)
        // ------------------------------------------------------------------

        private static int ParseInt(string s)
        {
            if (string.IsNullOrWhiteSpace(s)) return 0;
            return int.TryParse(s.Trim(), NumberStyles.Integer,
                CultureInfo.InvariantCulture, out int n) ? n : 0;
        }

        private static bool ParseMonat(string s, out int monat)
        {
            monat = ParseInt(s);
            return monat >= 1 && monat <= 12;
        }

        /// <summary>Zweistellige Jahreszahlen fensterln wie das Original:
        /// 38-99 -> 19xx, 0-37 -> 20xx.</summary>
        public static int NormalisiereJahr(int jahr)
        {
            if (jahr < 100 && jahr > 37) return jahr + 1900;
            if (jahr <= 37 && jahr >= 0) return jahr + 2000;
            return jahr;
        }

        /// <summary>Geldbetrag-Eingabe in Cent (Logik wie BuchungViewModel:
        /// Komma = deutsches Format, sonst Punkt als Dezimaltrenner).</summary>
        private static int ParseBetragInCent(string text)
        {
            if (string.IsNullOrWhiteSpace(text)) return 0;
            string s = text.Trim();
            decimal d;
            if (s.Contains(","))
            {
                string s2 = s.Replace(".", "");
                if (decimal.TryParse(s2, NumberStyles.Number, DeDE, out d))
                    return (int)decimal.Round(d * 100m, 0, MidpointRounding.AwayFromZero);
            }
            else
            {
                if (decimal.TryParse(s, NumberStyles.Number,
                        CultureInfo.InvariantCulture, out d))
                    return (int)decimal.Round(d * 100m, 0, MidpointRounding.AwayFromZero);
            }
            return 0;
        }

        private static bool ParseMwstPromille(string text, out int promille)
        {
            promille = 0;
            if (string.IsNullOrWhiteSpace(text)) return false;
            string s = text.Trim().TrimEnd('%').Trim();
            decimal d;
            bool ok = s.Contains(",")
                ? decimal.TryParse(s, NumberStyles.Number, DeDE, out d)
                : decimal.TryParse(s, NumberStyles.Number, CultureInfo.InvariantCulture, out d);
            if (!ok || d < 0 || d > 100) return false;
            // "Promille" im Engine-Sinn: Prozent x1000 (19 % -> 19000),
            // wie Betrag.MwstPromille und Preset.Mwst
            promille = (int)decimal.Round(d * 1000m, 0, MidpointRounding.AwayFromZero);
            return true;
        }
    }
}
