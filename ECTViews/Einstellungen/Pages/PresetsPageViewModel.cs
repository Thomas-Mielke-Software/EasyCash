using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Windows.Input;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>
    /// Verwaltung der Buchungs-Presets ([Buchungsposten]). Master-Detail:
    /// links die Liste der angelegten Presets, rechts der Editor mit
    /// Beschreibung, Konto, MwSt, AfA-Jahre und der neuen Freitext-Notiz.
    ///
    /// Live-Save: jede Änderung an einem Preset-Item wird sofort über
    /// <see cref="Einstellungen.SpeicherePreset"/> in den Cache/ini
    /// geschrieben.
    /// </summary>
    public class PresetsPageViewModel : ViewModelBase
    {
        private static readonly CultureInfo De = new CultureInfo("de-DE");

        public ObservableCollection<PresetItem> Presets { get; }
            = new ObservableCollection<PresetItem>();

        private PresetItem _ausgewaehlt;
        public PresetItem Ausgewaehlt
        {
            get => _ausgewaehlt;
            set { SetProperty(ref _ausgewaehlt, value); OnPropertyChanged(nameof(HatAuswahl)); }
        }

        public bool HatAuswahl => _ausgewaehlt != null;

        public ICommand LoeschenCommand { get; }

        public PresetsPageViewModel()
        {
            LoeschenCommand = new RelayCommand(Loeschen, () => HatAuswahl);
            Laden();
        }

        /// <summary>
        /// Lädt die Preset-Liste aus dem (von außen geänderten) Cache neu und
        /// stellt die Selektion über den Slot-Index wieder her. Aufrufer
        /// (PresetsPage) stellt sicher, dass das NICHT während einer laufenden
        /// Bearbeitung passiert.
        /// </summary>
        public void AktualisiereAusCache()
        {
            int selIndex = _ausgewaehlt?.Index ?? -1;
            Laden();
            if (selIndex >= 0)
            {
                var wieder = Presets.FirstOrDefault(p => p.Index == selIndex);
                if (wieder != null) Ausgewaehlt = wieder;
            }
        }

        private void Laden()
        {
            Presets.Clear();
            var alle = Einstellungen.Presets;
            for (int i = 0; i < alle.Count; i++)
                if (!alle[i].IstLeer)
                    Presets.Add(new PresetItem(i, alle[i], this));
            Ausgewaehlt = Presets.FirstOrDefault();
        }

        /// <summary>Legt eine neue Vorlage im ersten freien Slot an und
        /// selektiert sie -- die Seite springt danach ins Nr.-Feld, damit
        /// die Nummer direkt vergeben werden kann (thematische Sortierung,
        /// z.B. Einnahmen 00-29, Ausgaben 30-99).</summary>
        public void NeuAnlegen()
        {
            // Ersten freien Slot (IstLeer) suchen.
            var alle = Einstellungen.Presets;
            int frei = -1;
            for (int i = 0; i < alle.Count; i++)
                if (alle[i].IstLeer && Presets.All(p => p.Index != i)) { frei = i; break; }
            if (frei < 0) return;  // alle 100 belegt

            var neu = new PresetItem(frei, new Preset("Neue Vorlage", true, 0, 1, "", ""), this);
            neu.Persistiere();            // sofort anlegen
            Presets.Add(neu);
            SortiereEin(neu);
            Ausgewaehlt = neu;
        }

        private void Loeschen()
        {
            var sel = Ausgewaehlt;
            if (sel == null) return;
            // Leeres Preset in den Slot schreiben.
            Einstellungen.SpeicherePreset(sel.Index, new Preset("", false, 0, 1, "", ""));
            Presets.Remove(sel);
            Ausgewaehlt = Presets.FirstOrDefault();
        }

        // -----------------------------------------------------------------
        // XML-Export/Import (Vorlagen teilen, z.B. im Forum)
        // -----------------------------------------------------------------

        /// <summary>XML der aktuellen Auswahl (null wenn keine Auswahl).</summary>
        public string ExportiereAusgewaehlt()
            => _ausgewaehlt == null
                ? null : PresetXml.Exportiere(_ausgewaehlt.NachModell());

        /// <summary>Importiert eine XML-Vorlage in den nächsten freien Slot
        /// und selektiert sie. Liefert null bei Erfolg, sonst den Fehlertext
        /// (die Seite zeigt ihn als MessageBox).</summary>
        public string ImportiereXml(string xml)
        {
            Preset p;
            try { p = PresetXml.Importiere(xml); }
            catch (FormatException ex) { return ex.Message; }

            var alle = Einstellungen.Presets;
            int frei = -1;
            for (int i = 0; i < alle.Count; i++)
                if (alle[i].IstLeer && Presets.All(x => x.Index != i)) { frei = i; break; }
            if (frei < 0) return "Alle 100 Vorlagen-Plätze sind belegt.";

            Einstellungen.SpeicherePreset(frei, p);
            var item = new PresetItem(frei, p, this);
            Presets.Add(item);
            SortiereEin(item);
            Ausgewaehlt = item;
            return null;
        }

        // -----------------------------------------------------------------
        // Vorlagen-Nummer (Slot) -- sichtbar und änderbar
        // -----------------------------------------------------------------

        /// <summary>Verschiebt eine Vorlage in einen anderen (freien) Slot.
        /// Liefert null bei Erfolg, sonst den Fehlertext fürs Nr.-Feld.</summary>
        internal string VerschiebeZuSlot(PresetItem item, int neuerSlot)
        {
            if (neuerSlot == item.Index) return null;
            if (neuerSlot < 0 || neuerSlot > 99)
                return "Bitte eine Nummer von 00 bis 99 eingeben.";
            if (Presets.Any(p => !ReferenceEquals(p, item) && p.Index == neuerSlot))
                return $"Platz {neuerSlot:D2} ist bereits belegt.";

            // alten Slot leeren, unter der neuen Nummer speichern
            Einstellungen.SpeicherePreset(item.Index,
                new Preset("", false, 0, 1, "", ""));
            item.UebernehmeIndex(neuerSlot);
            item.Persistiere();
            SortiereEin(item);
            Statusleiste.Melde($"Vorlage auf Platz {neuerSlot:D2} verschoben.");
            return null;
        }

        /// <summary>Hält die Master-Liste nach Vorlagen-Nummer sortiert.</summary>
        private void SortiereEin(PresetItem item)
        {
            int alt = Presets.IndexOf(item);
            if (alt < 0) return;
            int ziel = 0;
            foreach (var p in Presets)
            {
                if (ReferenceEquals(p, item)) continue;
                if (p.Index < item.Index) ziel++;
            }
            if (ziel != alt) Presets.Move(alt, ziel);
        }

        // -----------------------------------------------------------------
        // Editierbares Preset-Item (ein Slot)
        // -----------------------------------------------------------------
        public class PresetItem : ViewModelBase
        {
            private bool _laden = true;
            private readonly PresetsPageViewModel _besitzer;

            /// <summary>Slot-Nummer in [Buchungsposten] (00-99) -- zugleich
            /// die Kurzwahl im Buchen-Dialog. Änderbar über NummerText.</summary>
            public int Index { get; private set; }

            public PresetItem(int index, Preset p, PresetsPageViewModel besitzer = null)
            {
                _besitzer    = besitzer;
                Index        = index;
                _beschreibung = p.Text;
                _istAusgabe   = p.Ausgabe;
                _mwstProzent  = MwstNachProzent(p.Mwst);
                _afaJahre     = p.AfaJ;
                _konto        = p.Konto;
                _notiz        = p.Notiz;
                _istDegressiv = p.Degressiv;
                _degressivSatz = p.AfaSatz;
                _basisBetragFormel = p.BasisBetragFormel;
                foreach (var z in p.Zeilen)
                    Zeilen.Add(new PresetZeileItem(z, this));
                _laden = false;
            }

            private string _beschreibung;
            public string Beschreibung
            {
                get => _beschreibung;
                set { if (SetProperty(ref _beschreibung, value)) { OnPropertyChanged(nameof(Anzeige)); Persistiere(); } }
            }

            private bool _istAusgabe;
            public bool IstAusgabe
            {
                get => _istAusgabe;
                set { if (SetProperty(ref _istAusgabe, value)) Persistiere(); }
            }

            private string _mwstProzent;
            /// <summary>MwSt in Prozent als Text (z.B. "19" oder "7,5").</summary>
            public string MwstProzent
            {
                get => _mwstProzent;
                set { if (SetProperty(ref _mwstProzent, value)) Persistiere(); }
            }

            private int _afaJahre;
            public int AfaJahre
            {
                get => _afaJahre;
                set { if (SetProperty(ref _afaJahre, value)) Persistiere(); }
            }

            private string _konto;
            public string Konto
            {
                get => _konto;
                set { if (SetProperty(ref _konto, value)) Persistiere(); }
            }

            private string _notiz;
            public string Notiz
            {
                get => _notiz;
                set { if (SetProperty(ref _notiz, value)) Persistiere(); }
            }

            private bool _istDegressiv;
            /// <summary>Degressive AfA statt linear.</summary>
            public bool IstDegressiv
            {
                get => _istDegressiv;
                set { if (SetProperty(ref _istDegressiv, value)) Persistiere(); }
            }

            private int _degressivSatz;
            /// <summary>Degressiver AfA-Satz in Prozent.</summary>
            public int DegressivSatz
            {
                get => _degressivSatz;
                set { if (SetProperty(ref _degressivSatz, value)) Persistiere(); }
            }

            /// <summary>Anzeigetext in der Master-Liste: Vorlagen-Nummer
            /// (Kurzwahl) vorangestellt, mehrzeilige Vorlagen erkennbar am
            /// Gruppen-Präfix.</summary>
            public string Anzeige =>
                Index.ToString("D2") + "  "
                + (Zeilen.Count > 0 ? "[G] " : "")
                + (string.IsNullOrWhiteSpace(_beschreibung)
                    ? "(ohne Beschreibung)" : _beschreibung);

            // --------------------------------------------------------------
            // Nr.-Feld: zeigt den Slot, Ändern verschiebt in einen freien
            // --------------------------------------------------------------

            public string NummerText
            {
                get => Index.ToString("D2");
                set
                {
                    var s = (value ?? "").Trim();
                    string fehler;
                    if (s == Index.ToString("D2") || s == Index.ToString())
                        fehler = null;
                    else if (!int.TryParse(s, out int neu))
                        fehler = "Bitte eine Nummer von 00 bis 99 eingeben.";
                    else
                        fehler = _besitzer?.VerschiebeZuSlot(this, neu);
                    NummerFehler = fehler ?? "";
                    OnPropertyChanged();   // bei Fehler zurück auf alten Wert
                }
            }

            private string _nummerFehler = "";
            public string NummerFehler
            {
                get => _nummerFehler;
                private set => SetProperty(ref _nummerFehler, value);
            }

            /// <summary>Vom Besitzer-VM beim Slot-Wechsel gerufen.</summary>
            internal void UebernehmeIndex(int neuerIndex)
            {
                Index = neuerIndex;
                OnPropertyChanged(nameof(NummerText));
                OnPropertyChanged(nameof(Anzeige));
            }

            // --------------------------------------------------------------
            // Buchungsgruppen-Zeilen (Zusatz-Zeilen der Vorlage)
            // --------------------------------------------------------------

            public ObservableCollection<PresetZeileItem> Zeilen { get; }
                = new ObservableCollection<PresetZeileItem>();

            private string _basisBetragFormel;
            /// <summary>Formel für den Betrag, den die BASIS-Zeile bucht
            /// (z.B. "$rest" bei belegweiser Aufteilung); leer = Eingabe
            /// unverändert buchen.</summary>
            public string BasisBetragFormel
            {
                get => _basisBetragFormel;
                set
                {
                    if (SetProperty(ref _basisBetragFormel, value ?? ""))
                    {
                        OnPropertyChanged(nameof(BasisBetragFormelFehler));
                        Persistiere();
                    }
                }
            }

            public string BasisBetragFormelFehler =>
                string.IsNullOrWhiteSpace(_basisBetragFormel)
                    ? ""
                    : FormelParser.Pruefe(_basisBetragFormel, PruefVariablen) ?? "";

            public bool KannZeileHinzufuegen =>
                Zeilen.Count < Einstellungen.MaxPresetZeilen - 1;

            public void ZeileHinzufuegen()
            {
                if (!KannZeileHinzufuegen) return;
                // Konto bleibt leer -> Zeile wird erst gespeichert, wenn ein
                // Konto gesetzt ist (Hinweis am Zeilen-Editor).
                Zeilen.Add(new PresetZeileItem(
                    new PresetZeile("", "", "", "", ""), this));
                OnPropertyChanged(nameof(Anzeige));
                OnPropertyChanged(nameof(KannZeileHinzufuegen));
            }

            public void ZeileEntfernen(PresetZeileItem zeile)
            {
                if (zeile == null || !Zeilen.Remove(zeile)) return;
                OnPropertyChanged(nameof(Anzeige));
                OnPropertyChanged(nameof(KannZeileHinzufuegen));
                Persistiere();
            }

            /// <summary>Verschiebt die Zeile um delta Positionen (-1/+1).</summary>
            public void ZeileVerschieben(PresetZeileItem zeile, int delta)
            {
                int i = Zeilen.IndexOf(zeile);
                if (i < 0) return;
                int ziel = i + delta;
                if (ziel < 0 || ziel >= Zeilen.Count) return;
                Zeilen.Move(i, ziel);
                Persistiere();
            }

            /// <summary>Wird von den Zeilen-Items bei Feldänderungen gerufen.</summary>
            internal void PersistiereVonZeile() => Persistiere();

            /// <summary>Das aktuelle Preset als Engine-Objekt (für
            /// Persistenz und XML-Export). Zeilen ohne Konto werden
            /// ausgelassen (noch unfertige Editor-Zeilen).</summary>
            internal Preset NachModell()
            {
                return new Preset(
                    _beschreibung, _istAusgabe, ProzentNachMwst(_mwstProzent),
                    _afaJahre, _konto, _notiz, _istDegressiv, _degressivSatz,
                    Zeilen.Where(z => !string.IsNullOrWhiteSpace(z.Konto))
                          .Select(z => z.NachModell()).ToList(),
                    _basisBetragFormel);
            }

            public void Persistiere()
            {
                if (_laden) return;
                Einstellungen.SpeicherePreset(Index, NachModell());
            }

            // MwSt-Festkomma (x1000) <-> Prozent-Text
            // (Anzeige im Systemformat, Parse tolerant/locale-frei)
            private static string MwstNachProzent(int mwstX1000)
                => Waehrungsformat.Zahl(mwstX1000 / 1000m, "0.###");

            private static int ProzentNachMwst(string prozentText)
            {
                return Waehrungsformat.TryParseProzent(prozentText, out var p)
                    ? (int)decimal.Round(p * 1000m)
                    : 0;
            }
        }

        // -----------------------------------------------------------------
        // Beispiel-Variablen für die Live-Formelprüfung im Editor
        // (nur Syntax/Variablennamen zählen, die Werte sind egal)
        // -----------------------------------------------------------------
        internal static readonly IReadOnlyDictionary<string, decimal> PruefVariablen =
            FormelParser.ZahlVariablen(
                new Buchung { BruttoBetrag = Betrag.AusCent(11900, 19000) },
                rest: 100m, vorlagenzeile: 1);

        // -----------------------------------------------------------------
        // Editierbare Zusatz-Zeile einer Buchungsgruppen-Vorlage
        // -----------------------------------------------------------------
        public class PresetZeileItem : ViewModelBase
        {
            private readonly PresetItem _besitzer;

            public PresetZeileItem(PresetZeile z, PresetItem besitzer)
            {
                _besitzer = besitzer;
                _artIndex = z.Art == "E" ? 1 : z.Art == "A" ? 2 : 0;
                _konto = z.Konto;
                _mwstAusdruck = z.MwstAusdruck;
                _betragFormel = z.BetragFormel;
                _textTemplate = z.TextTemplate;
                _belegTemplate = z.BelegTemplate;
                _betriebTemplate = z.BetriebTemplate;
                _bestandskontoTemplate = z.BestandskontoTemplate;
                _darstellungIndex =
                    string.Equals(z.Darstellung, "maske", StringComparison.OrdinalIgnoreCase) ? 2 :
                    string.Equals(z.Darstellung, "kompakt", StringComparison.OrdinalIgnoreCase) ? 1 : 0;
            }

            private void Geaendert() => _besitzer?.PersistiereVonZeile();

            private int _artIndex;
            /// <summary>0 = wie Basis, 1 = Einnahme, 2 = Ausgabe.</summary>
            public int ArtIndex
            {
                get => _artIndex;
                set { if (SetProperty(ref _artIndex, value)) Geaendert(); }
            }

            private string _konto;
            public string Konto
            {
                get => _konto;
                set
                {
                    if (SetProperty(ref _konto, value ?? ""))
                    {
                        OnPropertyChanged(nameof(KontoFehler));
                        Geaendert();
                    }
                }
            }

            /// <summary>Ohne Konto wird die Zeile nicht gespeichert; eine
            /// Feld-Spezifikation ("$de:Formular=Id|...||") wird live auf
            /// Syntaxfehler geprüft.</summary>
            public string KontoFehler
            {
                get
                {
                    if (string.IsNullOrWhiteSpace(_konto))
                        return "Ohne Konto wird die Zeile nicht gespeichert.";
                    if (KontoFeldSpezifikation.IstSpezifikation(_konto)
                        && KontoFeldSpezifikation.Parse(_konto, out var fehler) == null)
                        return fehler;
                    return "";
                }
            }

            private string _mwstAusdruck;
            /// <summary>Leer = Eingabefeld im Buchen-Dialog; sonst Festwert
            /// in Prozent ("7", "10,7"), x1000-Rohwert oder $vat1..$vat4.</summary>
            public string MwstAusdruck
            {
                get => _mwstAusdruck;
                set
                {
                    if (SetProperty(ref _mwstAusdruck, value ?? ""))
                    {
                        OnPropertyChanged(nameof(MwstFehler));
                        Geaendert();
                    }
                }
            }

            public string MwstFehler
            {
                get
                {
                    if (string.IsNullOrWhiteSpace(_mwstAusdruck)) return "";
                    return BuchungsgruppenRechner.WerteMwstAusdruckAus(
                        _mwstAusdruck, PruefVariablen, out var fehler) == null
                        ? fehler : "";
                }
            }

            private string _betragFormel;
            /// <summary>Leer = manuelle Eingabe im Buchen-Dialog; sonst Formel
            /// wie "$brutto*0,3", "$netto*0,19" oder "$rest".</summary>
            public string BetragFormel
            {
                get => _betragFormel;
                set
                {
                    if (SetProperty(ref _betragFormel, value ?? ""))
                    {
                        OnPropertyChanged(nameof(BetragFehler));
                        Geaendert();
                    }
                }
            }

            public string BetragFehler =>
                string.IsNullOrWhiteSpace(_betragFormel)
                    ? ""
                    : FormelParser.Pruefe(_betragFormel, PruefVariablen) ?? "";

            private string _textTemplate;
            public string TextTemplate
            {
                get => _textTemplate;
                set { if (SetProperty(ref _textTemplate, value ?? "")) Geaendert(); }
            }

            private string _belegTemplate;
            public string BelegTemplate
            {
                get => _belegTemplate;
                set { if (SetProperty(ref _belegTemplate, value ?? "")) Geaendert(); }
            }

            private string _betriebTemplate;
            public string BetriebTemplate
            {
                get => _betriebTemplate;
                set { if (SetProperty(ref _betriebTemplate, value ?? "")) Geaendert(); }
            }

            private string _bestandskontoTemplate;
            public string BestandskontoTemplate
            {
                get => _bestandskontoTemplate;
                set { if (SetProperty(ref _bestandskontoTemplate, value ?? "")) Geaendert(); }
            }

            private int _darstellungIndex;
            /// <summary>0 = automatisch, 1 = kompakt, 2 = Masken-Serie.</summary>
            public int DarstellungIndex
            {
                get => _darstellungIndex;
                set { if (SetProperty(ref _darstellungIndex, value)) Geaendert(); }
            }

            internal PresetZeile NachModell()
            {
                return new PresetZeile(
                    _artIndex == 1 ? "E" : _artIndex == 2 ? "A" : "",
                    _konto, _mwstAusdruck, _betragFormel, _textTemplate,
                    _belegTemplate, _betriebTemplate, _bestandskontoTemplate,
                    _darstellungIndex == 2 ? "maske" :
                    _darstellungIndex == 1 ? "kompakt" : "");
            }
        }
    }
}
