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

        public ICommand NeuCommand { get; }
        public ICommand LoeschenCommand { get; }

        public PresetsPageViewModel()
        {
            NeuCommand      = new RelayCommand(Neu);
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
                    Presets.Add(new PresetItem(i, alle[i]));
            Ausgewaehlt = Presets.FirstOrDefault();
        }

        private void Neu()
        {
            // Ersten freien Slot (IstLeer) suchen.
            var alle = Einstellungen.Presets;
            int frei = -1;
            for (int i = 0; i < alle.Count; i++)
                if (alle[i].IstLeer && Presets.All(p => p.Index != i)) { frei = i; break; }
            if (frei < 0) return;  // alle 100 belegt

            var neu = new PresetItem(frei, new Preset("Neues Preset", true, 0, 1, "", ""));
            neu.Persistiere();            // sofort anlegen
            Presets.Add(neu);
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
        // Editierbares Preset-Item (ein Slot)
        // -----------------------------------------------------------------
        public class PresetItem : ViewModelBase
        {
            private bool _laden = true;

            public int Index { get; }

            public PresetItem(int index, Preset p)
            {
                Index        = index;
                _beschreibung = p.Text;
                _istAusgabe   = p.Ausgabe;
                _mwstProzent  = MwstNachProzent(p.Mwst);
                _afaJahre     = p.AfaJ;
                _konto        = p.Konto;
                _notiz        = p.Notiz;
                _istDegressiv = p.Degressiv;
                _degressivSatz = p.AfaSatz;
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

            /// <summary>Anzeigetext in der Master-Liste.</summary>
            public string Anzeige => string.IsNullOrWhiteSpace(_beschreibung)
                ? "(ohne Beschreibung)" : _beschreibung;

            public void Persistiere()
            {
                if (_laden) return;
                Einstellungen.SpeicherePreset(Index, new Preset(
                    _beschreibung, _istAusgabe, ProzentNachMwst(_mwstProzent),
                    _afaJahre, _konto, _notiz, _istDegressiv, _degressivSatz));
            }

            // MwSt-Festkomma (x1000) <-> Prozent-Text
            private static string MwstNachProzent(int mwstX1000)
            {
                decimal prozent = mwstX1000 / 1000m;
                return prozent.ToString("0.###", De);
            }

            private static int ProzentNachMwst(string prozentText)
            {
                if (string.IsNullOrWhiteSpace(prozentText)) return 0;
                if (decimal.TryParse(prozentText.Replace('.', ','), NumberStyles.Number, De, out var p))
                    return (int)decimal.Round(p * 1000m);
                return 0;
            }
        }
    }
}
