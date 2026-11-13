// ZusatzFeldViewModel.cs -- Bausteine für die Erweiterung des Buchen-
// Dialogs durch eine Buchungsvorlage
//
//   ZusatzFeldViewModel   ein Zusatz-Feld der Vorlage (Eingabe-Control in
//                         einer der Boxen des Dialogs)
//   OriginalFeldZustand   Sichtbarkeit/Sperre/Bezeichnung eines der festen
//                         Dialog-Felder laut Vorlage
//   ZusatzzeileViewModel  Vorschau einer Zusatz-Zeile (reine Anzeige)
//
// Definition, Prüfung und Rechnen liegen in der Engine (VorlagenFelder.cs,
// Buchungsgruppen.cs) -- hier wird nur gebunden.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using ECTEngine;

namespace ECTViews.ViewModels
{
    /// <summary>Welches Control ein Zusatz-Feld im Dialog bekommt (wählt
    /// das DataTemplate, siehe ZusatzFeldTemplateSelector).</summary>
    public enum ZusatzFeldSteuerelement
    {
        /// <summary>Textfeld über die volle Breite.</summary>
        Text,
        /// <summary>Schmales, rechtsbündiges Textfeld (Betrag, Zahl,
        /// Prozent, Datum).</summary>
        Zahl,
        /// <summary>Editierbare Auswahl der MWSt-Sätze.</summary>
        Mwst,
        /// <summary>Auswahlliste (Auswahl-Feld, Konto).</summary>
        Auswahl,
        Haekchen,
        /// <summary>Icon-Liste (Betrieb, Bestandskonto).</summary>
        IconListe
    }

    // ======================================================================
    // Zusatz-Feld
    // ======================================================================

    public class ZusatzFeldViewModel : ViewModelBase
    {
        private readonly Action _onGeaendert;

        // Unterdrückt die Änderungs-Meldung, solange das Feld programmatisch
        // befüllt wird (Vorbelegung, gespeicherter Wert).
        private bool _still;

        public PresetFeld Feld { get; }

        public ZusatzFeldViewModel(PresetFeld feld, Action onGeaendert)
        {
            Feld = feld ?? throw new ArgumentNullException(nameof(feld));
            _onGeaendert = onGeaendert;

            switch (feld.Typ)
            {
                case ZusatzFeldTyp.Betrag:
                case ZusatzFeldTyp.Zahl:
                case ZusatzFeldTyp.Prozent:
                case ZusatzFeldTyp.Datum:
                    Steuerelement = ZusatzFeldSteuerelement.Zahl; break;
                case ZusatzFeldTyp.Mwst:
                    Steuerelement = ZusatzFeldSteuerelement.Mwst; break;
                case ZusatzFeldTyp.Auswahl:
                case ZusatzFeldTyp.Konto:
                    Steuerelement = ZusatzFeldSteuerelement.Auswahl; break;
                case ZusatzFeldTyp.JaNein:
                    Steuerelement = ZusatzFeldSteuerelement.Haekchen; break;
                case ZusatzFeldTyp.Betrieb:
                case ZusatzFeldTyp.Bestandskonto:
                    Steuerelement = ZusatzFeldSteuerelement.IconListe; break;
                default:
                    Steuerelement = ZusatzFeldSteuerelement.Text; break;
            }

            if (feld.Typ == ZusatzFeldTyp.Auswahl)
                SetzeOptionen(feld.AuswahlOptionen.Select(o => o.Anzeige));
        }

        public ZusatzFeldSteuerelement Steuerelement { get; }

        /// <summary>Beschriftung ohne Doppelpunkt (Häkchen, Listen-Titel).</summary>
        public string Bezeichnung => Feld.AnzeigeBezeichnung;

        /// <summary>Beschriftung vor einem Eingabefeld ("Anteil Betrieb 2:").</summary>
        public string Beschriftung =>
            Bezeichnung.EndsWith(":", StringComparison.Ordinal) ? Bezeichnung : Bezeichnung + ":";

        /// <summary>Einheit hinter dem Feld ("%").</summary>
        public string Einheit =>
            Feld.Typ == ZusatzFeldTyp.Prozent || Feld.Typ == ZusatzFeldTyp.Mwst ? "%" : "";

        /// <summary>Tooltip: der Platzhalter, unter dem die Vorlage den
        /// Wert verwendet.</summary>
        public string Hinweis => "Zusatz-Feld der Buchungsvorlage (Platzhalter $" + Feld.Name + ")";

        private void Geaendert()
        {
            OnPropertyChanged(nameof(Rohwert));
            if (!_still) _onGeaendert?.Invoke();
        }

        // ------------------------------------------------------------------
        // Textfeld / MWSt-Auswahl
        // ------------------------------------------------------------------

        private string _text = "";
        public string Text
        {
            get => _text;
            set { if (SetProperty(ref _text, value ?? "")) Geaendert(); }
        }

        // ------------------------------------------------------------------
        // Auswahlliste (Auswahl-Feld: Anzeige-Texte; Konto: Kontonamen;
        // MWSt: Vorschläge des editierbaren Felds)
        // ------------------------------------------------------------------

        public ObservableCollection<string> Optionen { get; }
            = new ObservableCollection<string>();

        /// <summary>Befüllt die Auswahlliste. Nicht-Pflicht-Felder bekommen
        /// einen leeren ersten Eintrag ("keine Angabe").</summary>
        public void SetzeOptionen(IEnumerable<string> optionen)
        {
            string gemerkt = Rohwert;
            Optionen.Clear();
            if (Steuerelement == ZusatzFeldSteuerelement.Auswahl && !Feld.Pflicht)
                Optionen.Add("");
            if (optionen != null)
                foreach (var o in optionen)
                    if (o != null && !Optionen.Contains(o)) Optionen.Add(o);
            if (Steuerelement == ZusatzFeldSteuerelement.Auswahl)
                SetzeRohwertStill(gemerkt);
        }

        private string _gewaehlteOption = "";
        public string GewaehlteOption
        {
            get => _gewaehlteOption;
            set { if (SetProperty(ref _gewaehlteOption, value ?? "")) Geaendert(); }
        }

        // ------------------------------------------------------------------
        // Häkchen
        // ------------------------------------------------------------------

        private bool _istAngehakt;
        public bool IstAngehakt
        {
            get => _istAngehakt;
            set { if (SetProperty(ref _istAngehakt, value)) Geaendert(); }
        }

        // ------------------------------------------------------------------
        // Icon-Liste (Betrieb / Bestandskonto)
        // ------------------------------------------------------------------

        public ObservableCollection<IconListItem> Eintraege { get; }
            = new ObservableCollection<IconListItem>();

        // Wert, der gesetzt wurde, bevor die Liste befüllt war (die Listen
        // kommen erst nach dem Laden der Vorlage, siehe ViewHost.BefuelleListen)
        private string _ausstehenderEintrag = "";

        /// <summary>Befüllt die Icon-Liste aus der Stammliste des Dialogs
        /// (Kopien ohne Tastenkürzel -- die Alt-Kürzel gehören den
        /// Original-Listen) und stellt die Auswahl wieder her.</summary>
        public void SetzeEintraege(IEnumerable<IconListItem> stammliste)
        {
            string gemerkt = Rohwert;
            Eintraege.Clear();
            if (stammliste != null)
                foreach (var e in stammliste)
                    Eintraege.Add(new IconListItem
                    {
                        Name = e.Name, IconIndex = e.IconIndex, Icon = e.Icon
                    });
            OnPropertyChanged(nameof(Sichtbar));
            SetzeRohwertStill(gemerkt);
        }

        private IconListItem _gewaehlterEintrag;
        public IconListItem GewaehlterEintrag
        {
            get => _gewaehlterEintrag;
            set
            {
                if (!SetProperty(ref _gewaehlterEintrag, value)) return;
                if (value != null) _ausstehenderEintrag = "";
                Geaendert();
            }
        }

        // ------------------------------------------------------------------
        // Wert
        // ------------------------------------------------------------------

        /// <summary>
        /// False, wenn das Feld im Dialog nichts anzuzeigen hat: eine
        /// Betriebs-/Bestandskonto-Liste, obwohl gar keine Betriebe bzw.
        /// Bestandskonten angelegt sind. Ein solches Feld wird nicht
        /// geprüft (auch nicht als Pflichtfeld) und liefert "".
        /// </summary>
        public bool Sichtbar =>
            Steuerelement != ZusatzFeldSteuerelement.IconListe || Eintraege.Count > 0;

        /// <summary>Der Wert in Speicherform (so landet er in der Buchung
        /// und in den Formeln, siehe VorlagenFelder.Werte).</summary>
        public string Rohwert
        {
            get
            {
                switch (Steuerelement)
                {
                    case ZusatzFeldSteuerelement.Haekchen:
                        return _istAngehakt ? "1" : "0";
                    case ZusatzFeldSteuerelement.IconListe:
                        return _gewaehlterEintrag?.Name ?? _ausstehenderEintrag;
                    case ZusatzFeldSteuerelement.Auswahl:
                        if (Feld.Typ != ZusatzFeldTyp.Auswahl) return _gewaehlteOption;
                        foreach (var o in Feld.AuswahlOptionen)
                            if (o.Anzeige == _gewaehlteOption) return o.Wert;
                        return "";
                    default:
                        return _text;
                }
            }
        }

        /// <summary>Setzt den Wert, ohne die Änderungs-Meldung auszulösen
        /// (Vorbelegung, gespeicherter Wert beim Bearbeiten).</summary>
        public void SetzeRohwertStill(string roh)
        {
            roh = roh ?? "";
            _still = true;
            try
            {
                switch (Steuerelement)
                {
                    case ZusatzFeldSteuerelement.Haekchen:
                        IstAngehakt = VorlagenFelder.Werte(Feld, roh).Zahl != 0m;
                        break;
                    case ZusatzFeldSteuerelement.IconListe:
                        var eintrag = Eintraege.FirstOrDefault(e => e.Name == roh);
                        _ausstehenderEintrag = eintrag == null ? roh : "";
                        GewaehlterEintrag = eintrag;
                        break;
                    case ZusatzFeldSteuerelement.Auswahl:
                        if (Feld.Typ == ZusatzFeldTyp.Auswahl)
                        {
                            var option = Feld.AuswahlOptionen.FirstOrDefault(o => o.Wert == roh);
                            GewaehlteOption = option?.Anzeige ?? "";
                        }
                        else
                        {
                            // Konto: ein gespeichertes Konto, das es nicht
                            // mehr gibt, bleibt als Eintrag wählbar
                            if (roh.Length > 0 && !Optionen.Contains(roh)) Optionen.Add(roh);
                            GewaehlteOption = roh;
                        }
                        break;
                    default:
                        Text = roh;
                        break;
                }
            }
            finally { _still = false; }
        }

        /// <summary>Geprüfter Wert für die Engine.</summary>
        public FeldWert Wert => VorlagenFelder.Werte(Feld, Sichtbar ? Rohwert : "");

        private string _fehler = "";
        public string Fehler
        {
            get => _fehler;
            private set => SetProperty(ref _fehler, value ?? "");
        }

        /// <summary>
        /// Aktualisiert die Fehleranzeige am Feld. Unlesbare Zahlen werden
        /// sofort gemeldet (die Vorschau rechnet sonst stillschweigend mit
        /// 0); fehlende Pflichtangaben und halb getippte Datumswerte erst,
        /// nachdem einmal versucht wurde zu buchen
        /// (<paramref name="allesZeigen"/>).
        /// </summary>
        /// <returns>True, wenn das Feld (auch still) keinen Fehler hat.</returns>
        public bool Pruefe(bool allesZeigen)
        {
            if (!Sichtbar) { Fehler = ""; return true; }
            var w = Wert;
            bool sofort = w.Roh.Length > 0 && Steuerelement != ZusatzFeldSteuerelement.Text
                && Feld.Typ != ZusatzFeldTyp.Datum;
            Fehler = allesZeigen || sofort ? w.Fehler : "";
            return w.Fehler.Length == 0;
        }
    }

    // ======================================================================
    // Original-Feld des Dialogs
    // ======================================================================

    /// <summary>
    /// Zustand eines festen Dialog-Felds laut aktiver Vorlage. Ohne Vorlage
    /// (oder ohne Regel) ist das Feld sichtbar, editierbar und trägt seine
    /// Standard-Bezeichnung.
    /// </summary>
    public sealed class OriginalFeldZustand : ViewModelBase
    {
        private readonly string _standardTitel;

        public string Schluessel { get; }

        public OriginalFeldZustand(string schluessel)
        {
            Schluessel = schluessel;
            _standardTitel = OriginalFeld.Finde(schluessel)?.Standardbezeichnung ?? schluessel;
            _titel = _standardTitel;
        }

        private FeldZustand _zustand = FeldZustand.Normal;
        public FeldZustand Zustand => _zustand;

        /// <summary>False = die Vorlage blendet das Feld aus.</summary>
        public bool Sichtbar => _zustand != FeldZustand.Ausgeblendet;

        /// <summary>False = die Vorlage sperrt das Feld (oder blendet es
        /// aus); sein Wert kommt dann aus der Vorlage.</summary>
        public bool Aktiv => _zustand == FeldZustand.Normal;

        private string _titel;
        /// <summary>Bezeichnung ohne Doppelpunkt (Box-Überschriften).</summary>
        public string Titel => _titel;

        /// <summary>Bezeichnung vor einem Eingabefeld ("Brutto:").</summary>
        public string Beschriftung =>
            _titel.EndsWith(":", StringComparison.Ordinal) ? _titel : _titel + ":";

        /// <summary>Tooltip eines gesperrten Felds (null = kein Tooltip).</summary>
        public string SperrHinweis =>
            _zustand == FeldZustand.Gesperrt
                ? "Dieses Feld ist durch die Buchungsvorlage gesperrt -- der Wert "
                  + "kommt aus der Vorlage."
                : null;

        /// <summary>Übernimmt die Regel der Vorlage (null = Standard).</summary>
        internal void Setze(OriginalFeldRegel regel)
        {
            _zustand = regel?.Zustand ?? FeldZustand.Normal;
            _titel = string.IsNullOrEmpty(regel?.Bezeichnung) ? _standardTitel : regel.Bezeichnung;
            OnPropertyChanged(nameof(Zustand));
            OnPropertyChanged(nameof(Sichtbar));
            OnPropertyChanged(nameof(Aktiv));
            OnPropertyChanged(nameof(Titel));
            OnPropertyChanged(nameof(Beschriftung));
            OnPropertyChanged(nameof(SperrHinweis));
        }
    }

    // ======================================================================
    // Vorschau einer Zusatz-Zeile
    // ======================================================================

    /// <summary>
    /// Anzeige einer Zusatz-Zeile der aktiven Buchungsgruppen-Vorlage: was
    /// beim Buchen entsteht. Rein lesend -- Eingaben kommen aus den
    /// Zusatz-Feldern der Vorlage.
    /// </summary>
    public class ZusatzzeileViewModel : ViewModelBase
    {
        /// <summary>1-basierter Zeilenindex (= spätere Gruppen-Rolle).</summary>
        public int Index { get; }

        public ZusatzzeileViewModel(int index) { Index = index; }

        private string _artText = "";
        public string ArtText { get => _artText; private set => SetProperty(ref _artText, value); }

        private string _kontoText = "";
        public string KontoText { get => _kontoText; private set => SetProperty(ref _kontoText, value); }

        private string _beschreibungText = "";
        public string BeschreibungText { get => _beschreibungText; private set => SetProperty(ref _beschreibungText, value); }

        private string _betragAnzeige = "";
        public string BetragAnzeige { get => _betragAnzeige; private set => SetProperty(ref _betragAnzeige, value); }

        private string _mwstAnzeige = "";
        public string MwstAnzeige { get => _mwstAnzeige; private set => SetProperty(ref _mwstAnzeige, value); }

        private string _ziel = "";
        /// <summary>Betrieb und/oder Bestandskonto der Zeile, soweit sie von
        /// der Basis-Buchung abweichen ("" = wie die Basis).</summary>
        public string Ziel { get => _ziel; private set => SetProperty(ref _ziel, value); }

        private string _details = "";
        /// <summary>Tooltip: Beleg, Betrieb, Bestandskonto der Zeile.</summary>
        public string Details { get => _details; private set => SetProperty(ref _details, value); }

        private bool _uebersprungen;
        /// <summary>True: die Zeile wird nicht gebucht (grau dargestellt).</summary>
        public bool Uebersprungen { get => _uebersprungen; private set => SetProperty(ref _uebersprungen, value); }

        private string _hinweis = "";
        public string Hinweis { get => _hinweis; private set => SetProperty(ref _hinweis, value); }

        private string _zeilenFehler = "";
        public string ZeilenFehler { get => _zeilenFehler; private set => SetProperty(ref _zeilenFehler, value); }

        /// <param name="basis">Die Basis-Buchung -- zum Vergleich, ob die
        /// Zeile auf einen anderen Betrieb bzw. ein anderes Bestandskonto
        /// bucht.</param>
        internal void UebernimmBerechnung(GruppenZeile z, Buchung basis)
        {
            var abweichend = new List<string>();
            if (!string.IsNullOrEmpty(z.Betrieb) && z.Betrieb != basis?.Betrieb)
                abweichend.Add(z.Betrieb);
            if (!string.IsNullOrEmpty(z.Bestandskonto) && z.Bestandskonto != basis?.Bestandskonto)
                abweichend.Add(z.Bestandskonto);
            Ziel = string.Join(", ", abweichend);

            ArtText = z.Art == Buchungsart.Einnahme ? "Einnahme" : "Ausgabe";
            KontoText = z.Konto;
            BeschreibungText = z.Beschreibung;
            BetragAnzeige = Waehrungsformat.Betrag(z.Brutto);
            MwstAnzeige = Waehrungsformat.Zahl(z.MwstPromille / 1000m) + " %";
            Uebersprungen = z.Uebersprungen;
            Hinweis = z.Hinweis;
            ZeilenFehler = z.Fehler;

            var teile = new List<string>();
            if (!string.IsNullOrEmpty(z.Belegnummer)) teile.Add("Beleg: " + z.Belegnummer);
            if (!string.IsNullOrEmpty(z.Betrieb)) teile.Add("Betrieb: " + z.Betrieb);
            if (!string.IsNullOrEmpty(z.Bestandskonto)) teile.Add("Bestandskonto: " + z.Bestandskonto);
            Details = teile.Count == 0 ? null : string.Join("\n", teile);
        }
    }
}
