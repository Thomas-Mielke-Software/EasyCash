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

        /// <summary>Alle freien Vorlagen-Plätze in aufsteigender Reihenfolge
        /// (Grundlage für die Platz-Auswahl der Bibliothek).</summary>
        public IReadOnlyList<int> FreieSlots()
        {
            var alle = Einstellungen.Presets;
            var frei = new List<int>();
            for (int i = 0; i < alle.Count; i++)
                if (alle[i].IstLeer && Presets.All(x => x.Index != i)) frei.Add(i);
            return frei;
        }

        /// <summary>Importiert eine XML-Vorlage und selektiert sie. Ohne
        /// <paramref name="zielSlot"/> (-1) landet sie im ersten freien Platz.
        /// Liefert null bei Erfolg, sonst den Fehlertext (die Seite zeigt ihn
        /// als MessageBox).</summary>
        public string ImportiereXml(string xml, int zielSlot = -1)
        {
            Preset p;
            try { p = PresetXml.Importiere(xml); }
            catch (FormatException ex) { return ex.Message; }

            var freie = FreieSlots();
            if (freie.Count == 0) return "Alle 100 Vorlagen-Plätze sind belegt.";

            int frei;
            if (zielSlot < 0)
                frei = freie[0];
            else if (freie.Contains(zielSlot))
                frei = zielSlot;
            else
                return $"Platz {zielSlot:D2} ist bereits belegt.";

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

            // Feste Kennung (Preset.Id). Wandert beim Umnummerieren mit, damit
            // gebuchte Buchungen ihre Vorlage wiederfinden; nach jedem
            // Speichern aus dem Cache nachgeführt (eine neue Vorlage bekommt
            // sie erst dort).
            private string _id;

            public PresetItem(int index, Preset p, PresetsPageViewModel besitzer = null)
            {
                _besitzer    = besitzer;
                Index        = index;
                _id          = p.Id;
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

                // Erweiterung des Buchen-Dialogs
                var e = p.Erweiterung;
                _eingabeBetragFormel = e.EingabeBetragFormel;
                _basisTextTemplate = e.BasisTextTemplate;
                _basisBelegTemplate = e.BasisBelegTemplate;
                _basisBestandskonto = e.BasisBestandskonto;
                _basisBetrieb = e.BasisBetrieb;
                foreach (var info in OriginalFeld.Alle)
                    OriginalFelder.Add(new OriginalFeldItem(
                        info, e.RegelFuer(info.Schluessel), this));
                foreach (var f in e.Felder)
                    Felder.Add(new PresetFeldItem(f, this));
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
                    : FormelParser.Pruefe(_basisBetragFormel, PruefZahlVariablen) ?? "";

            // --------------------------------------------------------------
            // Erweiterung des Buchen-Dialogs: Original-Felder, Vorgaben für
            // die Basis-Buchung, Zusatz-Felder (ECTEngine/VorlagenFelder.cs)
            // --------------------------------------------------------------

            /// <summary>Eine Zeile pro Original-Feld des Buchen-Dialogs
            /// (immer alle, auch die ohne abweichende Regel).</summary>
            public ObservableCollection<OriginalFeldItem> OriginalFelder { get; }
                = new ObservableCollection<OriginalFeldItem>();

            public ObservableCollection<PresetFeldItem> Felder { get; }
                = new ObservableCollection<PresetFeldItem>();

            private FeldZustand ZustandVon(string feld)
            {
                foreach (var o in OriginalFelder)
                    if (o.Schluessel == feld) return o.Zustand;
                return FeldZustand.Normal;
            }

            /// <summary>Vermerk hinter einem Wert-Feld des Editors, dessen
            /// Pendant im Buchen-Dialog gesperrt oder ausgeblendet ist --
            /// dann ist der hier eingetragene Wert verbindlich.</summary>
            private string Vermerk(string feld)
            {
                switch (ZustandVon(feld))
                {
                    case FeldZustand.Gesperrt:     return "im Buchen-Dialog gesperrt";
                    case FeldZustand.Ausgeblendet: return "im Buchen-Dialog ausgeblendet";
                    default:                       return "";
                }
            }

            public string KontoVermerk => Vermerk(OriginalFeld.Konto);
            public string MwstVermerk => Vermerk(OriginalFeld.Mwst);
            public string BelegVermerk => Vermerk(OriginalFeld.Beleg);
            public string BestandskontoVermerk => Vermerk(OriginalFeld.Bestandskonto);
            public string BetriebVermerk => Vermerk(OriginalFeld.Betrieb);
            public string AfaVermerk => Vermerk(OriginalFeld.Afa);

            /// <summary>Die Betrags-Formel gibt es nur, wenn der Betrag im
            /// Dialog nicht eingegeben wird.</summary>
            public bool BetragFormelSichtbar =>
                ZustandVon(OriginalFeld.Betrag) != FeldZustand.Normal;

            /// <summary>Felder, die im Dialog ausgeblendet sind UND dort
            /// auch keinen Wert brauchen, verschwinden zur Verdeutlichung
            /// auch hier (Beleg, AfA). Pflichtfelder bleiben stehen: ihren
            /// Wert muss die Vorlage ja gerade liefern.</summary>
            public bool BelegZeileSichtbar =>
                ZustandVon(OriginalFeld.Beleg) != FeldZustand.Ausgeblendet;

            public bool AfaZeilenSichtbar =>
                ZustandVon(OriginalFeld.Afa) != FeldZustand.Ausgeblendet;

            /// <summary>Wird von den Original-Feld-Zeilen bei Änderungen
            /// gerufen.</summary>
            internal void OriginalFeldGeaendert(OriginalFeldItem feld)
            {
                // Eine Vorlage, die die Abschreibung ausblendet, soll auch
                // keine buchen -- sonst hinge an jeder Buchung unsichtbar
                // eine AfA-Dauer, die hier niemand mehr sieht.
                if (feld.Schluessel == OriginalFeld.Afa
                    && feld.Zustand == FeldZustand.Ausgeblendet)
                {
                    _afaJahre = 1;
                    _istDegressiv = false;
                    _degressivSatz = 0;
                    OnPropertyChanged(nameof(AfaJahre));
                    OnPropertyChanged(nameof(IstDegressiv));
                    OnPropertyChanged(nameof(DegressivSatz));
                }
                // Dasselbe für die Belegnummer: ihr Umform-Text verschwindet
                // mit dem Ausblenden aus dem Editor und darf dann auch nicht
                // unsichtbar weiterwirken.
                if (feld.Schluessel == OriginalFeld.Beleg
                    && feld.Zustand == FeldZustand.Ausgeblendet
                    && _basisBelegTemplate.Length > 0)
                {
                    _basisBelegTemplate = "";
                    OnPropertyChanged(nameof(BasisBelegTemplate));
                }
                Persistiere();
            }

            private string _eingabeBetragFormel;
            /// <summary>Formel für den Betrag, wenn das Betragsfeld im
            /// Dialog gesperrt oder ausgeblendet ist.</summary>
            public string EingabeBetragFormel
            {
                get => _eingabeBetragFormel;
                set { if (SetProperty(ref _eingabeBetragFormel, value ?? "")) Persistiere(); }
            }

            public string EingabeBetragFormelFehler =>
                string.IsNullOrWhiteSpace(_eingabeBetragFormel)
                    ? ""
                    : FormelParser.Pruefe(_eingabeBetragFormel,
                        FormelParser.FeldVariablen(BeispielFeldwerte())) ?? "";

            private string _basisTextTemplate;
            /// <summary>Template für die gebuchte Beschreibung der Basis.</summary>
            public string BasisTextTemplate
            {
                get => _basisTextTemplate;
                set { if (SetProperty(ref _basisTextTemplate, value ?? "")) Persistiere(); }
            }

            private string _basisBelegTemplate;
            public string BasisBelegTemplate
            {
                get => _basisBelegTemplate;
                set { if (SetProperty(ref _basisBelegTemplate, value ?? "")) Persistiere(); }
            }

            private string _basisBestandskonto;
            public string BasisBestandskonto
            {
                get => _basisBestandskonto;
                set { if (SetProperty(ref _basisBestandskonto, value ?? "")) Persistiere(); }
            }

            private string _basisBetrieb;
            public string BasisBetrieb
            {
                get => _basisBetrieb;
                set { if (SetProperty(ref _basisBetrieb, value ?? "")) Persistiere(); }
            }

            /// <summary>Vorschläge für die beiden Auswahlfelder.</summary>
            public IReadOnlyList<string> BestandskontoNamen =>
                Einstellungen.Bestandskonten.Select(k => k.Name).ToList();

            public IReadOnlyList<string> BetriebNamen =>
                Einstellungen.Betriebe.Select(b => b.Name).ToList();

            // -- Zusatz-Felder --

            public bool KannFeldHinzufuegen => Felder.Count < VorlagenFelder.MaxFelder;

            public void FeldHinzufuegen()
            {
                if (!KannFeldHinzufuegen) return;
                // Ohne Platzhalter wird das Feld noch nicht gespeichert
                // (Hinweis am Feld-Editor).
                Felder.Add(new PresetFeldItem(new PresetFeld("", "", "betrag"), this));
                OnPropertyChanged(nameof(KannFeldHinzufuegen));
            }

            public void FeldEntfernen(PresetFeldItem feld)
            {
                if (feld == null || !Felder.Remove(feld)) return;
                OnPropertyChanged(nameof(KannFeldHinzufuegen));
                Persistiere();
            }

            public void FeldVerschieben(PresetFeldItem feld, int delta)
            {
                int i = Felder.IndexOf(feld);
                if (i < 0) return;
                int ziel = i + delta;
                if (ziel < 0 || ziel >= Felder.Count) return;
                Felder.Move(i, ziel);
                Persistiere();
            }

            /// <summary>Wird von den Feld-Items bei Änderungen gerufen.</summary>
            internal void PersistiereVonFeld() => Persistiere();

            private IReadOnlyList<PresetFeld> FelderModell() =>
                Felder.Where(f => !string.IsNullOrWhiteSpace(f.Name))
                      .Select(f => f.NachModell()).ToList();

            private IReadOnlyList<FeldWert> BeispielFeldwerte() =>
                VorlagenFelder.Beispielwerte(FelderModell());

            /// <summary>Variablen für die Live-Formelprüfung dieser Vorlage:
            /// die eingebauten plus ihre Zusatz-Felder.</summary>
            internal IReadOnlyDictionary<string, decimal> PruefZahlVariablen =>
                FormelParser.ZahlVariablen(
                    new Buchung { BruttoBetrag = Betrag.AusCent(11900, 19000) },
                    100m, 1, BeispielFeldwerte());

            // -- Prüfhinweise --

            /// <summary>Widersprüche in der Vorlage, die sonst erst beim
            /// Buchen auffielen (ein Satz pro Zeile; "" = alles in Ordnung).
            /// Der Editor speichert trotzdem -- eine halb fertige Vorlage
            /// soll nicht verloren gehen; der Buchen-Dialog blockiert dann
            /// mit derselben Begründung.</summary>
            public string Probleme =>
                string.Join("\n", VorlagenFelder.Pruefe(NachModell()));

            /// <summary>Meldet alles neu, was sich aus mehreren Feldern der
            /// Vorlage ergibt (nach jedem Speichern).</summary>
            private void MeldeAbgeleitetes()
            {
                OnPropertyChanged(nameof(Probleme));
                OnPropertyChanged(nameof(KontoVermerk));
                OnPropertyChanged(nameof(MwstVermerk));
                OnPropertyChanged(nameof(BelegVermerk));
                OnPropertyChanged(nameof(BestandskontoVermerk));
                OnPropertyChanged(nameof(BetriebVermerk));
                OnPropertyChanged(nameof(AfaVermerk));
                OnPropertyChanged(nameof(BetragFormelSichtbar));
                OnPropertyChanged(nameof(BelegZeileSichtbar));
                OnPropertyChanged(nameof(AfaZeilenSichtbar));
                OnPropertyChanged(nameof(EingabeBetragFormelFehler));
                OnPropertyChanged(nameof(BasisBetragFormelFehler));
                foreach (var z in Zeilen) z.MeldeFormelFehler();
                foreach (var f in Felder) f.MeldeFehler();
                AktualisiereKontoAuswahl();
                foreach (var z in Zeilen) z.AktualisiereKontoAuswahl();
            }

            // -- Auswahlliste des Konto-Felds --

            private string _kontoAuswahlSchluessel;
            private IReadOnlyList<KontoAuswahlEintrag> _kontoAuswahl;

            /// <summary>Vorschläge für das Konto-Feld: die Konten der
            /// Buchungsart und Konto-Feld-Spezifikationen für häufige Fälle
            /// (KontoVorschlaege). Freie Eingabe bleibt möglich.</summary>
            public IReadOnlyList<KontoAuswahlEintrag> KontoAuswahl
            {
                get { AktualisiereKontoAuswahl(melden: false); return _kontoAuswahl; }
            }

            /// <summary>Baut die Liste nur neu, wenn sich die Buchungsart
            /// geändert hat -- eine neue ItemsSource unter einer gerade
            /// bearbeiteten ComboBox wäre unnötige Unruhe.</summary>
            private void AktualisiereKontoAuswahl(bool melden = true)
            {
                string schluessel = (_istAusgabe ? "A" : "E") + "|" + KontoFeldSelektor.AktivesLandKuerzel();
                if (schluessel == _kontoAuswahlSchluessel) return;
                _kontoAuswahlSchluessel = schluessel;
                _kontoAuswahl = KontoAuswahlEintrag.Baue(!_istAusgabe, null);
                if (!melden) return;
                OnPropertyChanged(nameof(KontoAuswahl));
                // Fällt der gewählte Eintrag mit der neuen Liste weg, LEERT
                // die editierbare ComboBox ihr Textfeld (gemessen) -- beim
                // nächsten Verlassen des Felds ginge das Konto verloren.
                // Darum den Text danach aus dem Konto neu setzen.
                OnPropertyChanged(nameof(Konto));
            }

            /// <summary>Platzhalter der Zusatz-Felder vom Typ Konto (für die
            /// Konto-Auswahl der Zusatz-Zeilen).</summary>
            internal IEnumerable<KontoAuswahlEintrag> KontoPlatzhalter()
                => Felder.Where(f => !string.IsNullOrWhiteSpace(f.Name)
                                     && f.NachModell().Typ == ZusatzFeldTyp.Konto)
                         .Select(f => new KontoAuswahlEintrag("$" + f.Name,
                             string.IsNullOrWhiteSpace(f.Bezeichnung) ? "$" + f.Name : f.Bezeichnung,
                             "$" + f.Name + " (Zusatz-Feld)"));

            internal bool IstAusgabeIntern => _istAusgabe;

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
                    _basisBetragFormel,
                    new VorlagenErweiterung(
                        FelderModell(),
                        OriginalFelder.Select(o => o.NachModell()).ToList(),
                        _eingabeBetragFormel, _basisTextTemplate, _basisBelegTemplate,
                        _basisBestandskonto, _basisBetrieb),
                    _id);
            }

            public void Persistiere()
            {
                if (_laden) return;
                Einstellungen.SpeicherePreset(Index, NachModell());
                var gespeichert = Einstellungen.Presets;
                if (Index >= 0 && Index < gespeichert.Count) _id = gespeichert[Index].Id;
                MeldeAbgeleitetes();
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

            // -- Auswahlliste des Konto-Felds --

            private string _kontoAuswahlSchluessel;
            private IReadOnlyList<KontoAuswahlEintrag> _kontoAuswahl;

            /// <summary>Vorschläge für das Konto der Zeile: $konto, die
            /// Zusatz-Felder vom Typ Konto, die Konten der Buchungsart der
            /// ZEILE ("wie Basis" = die der Vorlage) und passende
            /// Konto-Feld-Spezifikationen.</summary>
            public IReadOnlyList<KontoAuswahlEintrag> KontoAuswahl
            {
                get { AktualisiereKontoAuswahl(melden: false); return _kontoAuswahl; }
            }

            internal void AktualisiereKontoAuswahl(bool melden = true)
            {
                bool ausgabe = _artIndex == 2
                    || (_artIndex == 0 && (_besitzer?.IstAusgabeIntern ?? true));
                var platzhalter = new List<KontoAuswahlEintrag>
                {
                    new KontoAuswahlEintrag("$konto", "Konto der Basis-Buchung", "$konto")
                };
                if (_besitzer != null) platzhalter.AddRange(_besitzer.KontoPlatzhalter());

                string schluessel = (ausgabe ? "A" : "E") + "|" + KontoFeldSelektor.AktivesLandKuerzel()
                    + "|" + string.Join(";", platzhalter.Select(p => p.Wert + "=" + p.Anzeige));
                if (schluessel == _kontoAuswahlSchluessel) return;
                _kontoAuswahlSchluessel = schluessel;
                _kontoAuswahl = KontoAuswahlEintrag.Baue(!ausgabe, platzhalter);
                if (!melden) return;
                OnPropertyChanged(nameof(KontoAuswahl));
                OnPropertyChanged(nameof(Konto));   // siehe PresetItem.AktualisiereKontoAuswahl
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
            /// <summary>Leer = Satz der Basis; sonst Festwert in Prozent
            /// ("7", "10,7"), x1000-Rohwert, $vat1..$vat4 oder ein
            /// Zusatz-Feld ("$satz2").</summary>
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
                        _mwstAusdruck, Variablen, out var fehler) == null
                        ? fehler : "";
                }
            }

            /// <summary>Prüf-Variablen: die eingebauten plus die Zusatz-Felder
            /// der Vorlage, zu der die Zeile gehört.</summary>
            private IReadOnlyDictionary<string, decimal> Variablen =>
                _besitzer?.PruefZahlVariablen ?? PruefVariablen;

            /// <summary>Vom Besitzer gerufen, wenn sich die Zusatz-Felder der
            /// Vorlage geändert haben (ein Platzhalter kann dadurch gültig
            /// oder ungültig geworden sein).</summary>
            internal void MeldeFormelFehler()
            {
                OnPropertyChanged(nameof(MwstFehler));
                OnPropertyChanged(nameof(BetragFehler));
            }

            private string _betragFormel;
            /// <summary>Formel wie "$brutto*0,3", "$netto*0,19", "$rest" oder
            /// ein Zusatz-Feld ("$skonto"). Ohne Formel wird die Zeile beim
            /// Buchen übersprungen.</summary>
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
                    ? "Ohne Betrags-Formel wird die Zeile beim Buchen übersprungen."
                    : FormelParser.Pruefe(_betragFormel, Variablen) ?? "";

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

            // Darstellungs-Override aus der Zeit der Eingabefelder pro Zeile
            // (0 = automatisch, 1 = kompakt, 2 = Masken-Serie). Ohne Wirkung
            // und ohne Bedienelement -- der Wert einer bestehenden Vorlage
            // wird nur unverändert wieder gespeichert.
            private readonly int _darstellungIndex;

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

        // -----------------------------------------------------------------
        // Eintrag der Konto-Auswahllisten im Editor (Konto der Vorlage und
        // der Zusatz-Zeilen). Die ComboBoxen sind editierbar: übernommen
        // wird "Wert" (TextSearch.TextPath), angezeigt "Anzeige" + "Hinweis".
        // -----------------------------------------------------------------
        public sealed class KontoAuswahlEintrag
        {
            /// <summary>Was ins Konto-Feld übernommen wird.</summary>
            public string Wert { get; }
            /// <summary>Text in der Liste.</summary>
            public string Anzeige { get; }
            /// <summary>Kleiner grauer Zusatz in der Liste ("" = keiner).</summary>
            public string Hinweis { get; }
            /// <summary>Abschnitts-Überschrift (nicht wählbar).</summary>
            public bool IstUeberschrift { get; }
            public bool HatHinweis => Hinweis.Length > 0;

            public KontoAuswahlEintrag(string wert, string anzeige, string hinweis = "",
                bool istUeberschrift = false)
            {
                Wert = wert ?? "";
                Anzeige = anzeige ?? "";
                Hinweis = hinweis ?? "";
                IstUeberschrift = istUeberschrift;
            }

            public override string ToString() => Wert;

            internal const string NeutralHinweis =
                "ergebnisneutral: nur Formulare, kein Gewinn";

            private static KontoAuswahlEintrag Ueberschrift(string text)
                => new KontoAuswahlEintrag("", text, "", istUeberschrift: true);

            /// <summary>
            /// Liste in drei Abschnitten: Platzhalter (nur Zusatz-Zeilen),
            /// die vorhandenen Konten der Buchungsart, Konto-Vorschläge über
            /// Formularfelder für das eingestellte Land.
            /// </summary>
            internal static IReadOnlyList<KontoAuswahlEintrag> Baue(bool istEinnahme,
                IEnumerable<KontoAuswahlEintrag> platzhalter)
            {
                var liste = new List<KontoAuswahlEintrag>();

                var ph = platzhalter?.ToList();
                if (ph != null && ph.Count > 0)
                {
                    liste.Add(Ueberschrift("Platzhalter"));
                    liste.AddRange(ph);
                }

                var konten = (istEinnahme ? Einstellungen.EinnahmenKonten : Einstellungen.AusgabenKonten)
                    .Where(k => !string.IsNullOrEmpty(k)).ToList();
                if (konten.Count > 0)
                {
                    liste.Add(Ueberschrift(istEinnahme ? "Einnahmen-Konten" : "Ausgaben-Konten"));
                    // Konten mit '/' am Anfang speisen nur die Formulare
                    // (z.B. die Umsatzsteuer-Voranmeldung) und bleiben aus
                    // Gewinn und EÜR heraus -- das soll man beim Wählen sehen.
                    liste.AddRange(konten.Select(k => new KontoAuswahlEintrag(k, k,
                        k.StartsWith("/") ? NeutralHinweis : "")));
                }

                string land = KontoFeldSelektor.AktivesLandKuerzel();
                var vorschlaege = KontoVorschlaege.Fuer(istEinnahme, land).ToList();
                if (vorschlaege.Count > 0)
                {
                    liste.Add(Ueberschrift("Über Formularfelder (Konto wird beim Buchen gefunden oder angelegt)"));
                    liste.AddRange(vorschlaege.Select(v =>
                        new KontoAuswahlEintrag(v.Spezifikation, v.Name(land), v.Felder(land))));
                }
                return liste;
            }
        }

        // -----------------------------------------------------------------
        // Regel für ein Original-Feld des Buchen-Dialogs
        // -----------------------------------------------------------------
        public class OriginalFeldItem : ViewModelBase
        {
            private readonly PresetItem _besitzer;
            private readonly OriginalFeldInfo _info;

            public OriginalFeldItem(OriginalFeldInfo info, OriginalFeldRegel regel,
                PresetItem besitzer)
            {
                _info = info;
                _besitzer = besitzer;
                _zustandIndex = regel.Zustand == FeldZustand.Gesperrt ? 1
                              : regel.Zustand == FeldZustand.Ausgeblendet ? 2 : 0;
                _bezeichnung = regel.Bezeichnung;
            }

            public string Schluessel => _info.Schluessel;

            /// <summary>Name des Felds, wie er im Dialog ohne Regel steht.</summary>
            public string Standardbezeichnung => _info.Standardbezeichnung;

            /// <summary>False: das Feld lässt sich nur umbenennen
            /// (Beschreibung -- sie trägt die Vorlagen-Auswahl).</summary>
            public bool Sperrbar => _info.Sperrbar;

            public string ZustandTooltip => Sperrbar
                ? "normal = im Buchen-Dialog sichtbar und editierbar; "
                  + "gesperrt = sichtbar, aber nicht änderbar (der Wert kommt aus der Vorlage); "
                  + "ausgeblendet = im Buchen-Dialog nicht zu sehen."
                : "Dieses Feld trägt die Auswahl der Buchungsvorlage und lässt sich "
                  + "deshalb nur umbenennen.";

            private int _zustandIndex;
            /// <summary>0 = normal, 1 = gesperrt, 2 = ausgeblendet.</summary>
            public int ZustandIndex
            {
                get => _zustandIndex;
                set
                {
                    if (!Sperrbar) value = 0;
                    if (SetProperty(ref _zustandIndex, value))
                        _besitzer?.OriginalFeldGeaendert(this);
                }
            }

            public FeldZustand Zustand =>
                _zustandIndex == 1 ? FeldZustand.Gesperrt
                : _zustandIndex == 2 ? FeldZustand.Ausgeblendet
                : FeldZustand.Normal;

            private string _bezeichnung;
            /// <summary>Abweichende Bezeichnung im Dialog ("" = Standard).</summary>
            public string Bezeichnung
            {
                get => _bezeichnung;
                set
                {
                    if (SetProperty(ref _bezeichnung, (value ?? "").Trim()))
                        _besitzer?.OriginalFeldGeaendert(this);
                }
            }

            internal OriginalFeldRegel NachModell()
                => new OriginalFeldRegel(Schluessel, Zustand, _bezeichnung);
        }

        // -----------------------------------------------------------------
        // Editierbares Zusatz-Feld einer Vorlage
        // -----------------------------------------------------------------
        public class PresetFeldItem : ViewModelBase
        {
            private readonly PresetItem _besitzer;

            public PresetFeldItem(PresetFeld f, PresetItem besitzer)
            {
                _besitzer = besitzer;
                _name = f.Name;
                _bezeichnung = f.Bezeichnung;
                _typIndex = Math.Max(0, IndexVon(VorlagenFelder.Typen, f.TypName));
                _boxIndex = Math.Max(0, IndexVon(VorlagenFelder.Boxen, f.Box));
                _vorbelegung = f.Vorbelegung;
                _pflicht = f.Pflicht;
                _merken = f.Merken;
                _optionen = f.Optionen;
            }

            private static int IndexVon(
                IReadOnlyList<KeyValuePair<string, string>> katalog, string name)
            {
                for (int i = 0; i < katalog.Count; i++)
                    if (katalog[i].Key == name) return i;
                return -1;
            }

            private void Geaendert() => _besitzer?.PersistiereVonFeld();

            /// <summary>Auswahllisten der beiden ComboBoxen.</summary>
            public IReadOnlyList<string> TypAnzeigen { get; } =
                VorlagenFelder.Typen.Select(t => t.Value).ToList();

            public IReadOnlyList<string> BoxAnzeigen { get; } =
                VorlagenFelder.Boxen.Select(b => b.Value).ToList();

            private string _name;
            /// <summary>Platzhalter ohne '$'.</summary>
            public string Name
            {
                get => _name;
                set
                {
                    if (SetProperty(ref _name, (value ?? "").Trim().TrimStart('$')))
                        Geaendert();
                }
            }

            private string _bezeichnung;
            public string Bezeichnung
            {
                get => _bezeichnung;
                set { if (SetProperty(ref _bezeichnung, value ?? "")) Geaendert(); }
            }

            private string TypName => VorlagenFelder.Typen[_typIndex].Key;

            private int _typIndex;
            public int TypIndex
            {
                get => _typIndex;
                set
                {
                    if (value < 0 || value >= VorlagenFelder.Typen.Count) return;
                    // Stand die Box noch auf dem Standard des alten Typs,
                    // zieht sie zum Standard des neuen um (ein Betrag gehört
                    // in die Betrags-Box, ein Betrieb in die Betriebs-Box).
                    string alteStandardBox = VorlagenFelder.StandardBox(
                        VorlagenFelder.TypAusName(TypName, out _));
                    bool boxWarStandard = VorlagenFelder.Boxen[_boxIndex].Key == alteStandardBox;
                    if (!SetProperty(ref _typIndex, value)) return;
                    if (boxWarStandard)
                    {
                        _boxIndex = Math.Max(0, IndexVon(VorlagenFelder.Boxen,
                            VorlagenFelder.StandardBox(
                                VorlagenFelder.TypAusName(TypName, out _))));
                        OnPropertyChanged(nameof(BoxIndex));
                    }
                    OnPropertyChanged(nameof(OptionenSichtbar));
                    OnPropertyChanged(nameof(OptionenTooltip));
                    Geaendert();
                }
            }

            private int _boxIndex;
            public int BoxIndex
            {
                get => _boxIndex;
                set
                {
                    if (value < 0 || value >= VorlagenFelder.Boxen.Count) return;
                    if (SetProperty(ref _boxIndex, value)) Geaendert();
                }
            }

            private string _vorbelegung;
            public string Vorbelegung
            {
                get => _vorbelegung;
                set { if (SetProperty(ref _vorbelegung, value ?? "")) Geaendert(); }
            }

            private bool _pflicht;
            public bool Pflicht
            {
                get => _pflicht;
                set { if (SetProperty(ref _pflicht, value)) Geaendert(); }
            }

            private bool _merken;
            public bool Merken
            {
                get => _merken;
                set { if (SetProperty(ref _merken, value)) Geaendert(); }
            }

            private string _optionen;
            public string Optionen
            {
                get => _optionen;
                set { if (SetProperty(ref _optionen, value ?? "")) Geaendert(); }
            }

            /// <summary>Das Optionen-Feld gibt es nur bei Typen, die es
            /// auswerten.</summary>
            public bool OptionenSichtbar =>
                TypName == "auswahl" || TypName == "konto";

            public string OptionenTooltip => TypName == "konto"
                ? "E = Einnahmen-Konten, A = Ausgaben-Konten; leer = Konten der "
                  + "Buchungsart der Vorlage"
                : "Auswahlmöglichkeiten, mit Semikolon getrennt: Anzeige=Wert;Anzeige=Wert "
                  + "(ohne '=' ist die Anzeige zugleich der Wert). Sind die Werte Zahlen, "
                  + "lässt sich der Platzhalter auch in Formeln verwenden.";

            /// <summary>Erster Fehler dieses Felds ("" = ok).</summary>
            public string Fehler
            {
                get
                {
                    if (string.IsNullOrWhiteSpace(_name))
                        return "Ohne Platzhalter wird das Feld nicht gespeichert.";
                    string f = VorlagenFelder.PruefeName(_name);
                    if (f.Length > 0) return f;
                    var modell = NachModell();
                    if (modell.Typ == ZusatzFeldTyp.Auswahl && modell.AuswahlOptionen.Count == 0)
                        return "Bitte die Auswahlmöglichkeiten angeben (Anzeige=Wert;Anzeige=Wert).";
                    if (_vorbelegung.Trim().Length > 0
                        && VorlagenFelder.Werte(modell,
                               VorlagenFelder.StandardWert(modell)).Fehler.Length > 0)
                        return "Die Vorbelegung passt nicht zum Typ.";
                    return "";
                }
            }

            internal void MeldeFehler() => OnPropertyChanged(nameof(Fehler));

            internal PresetFeld NachModell()
                => new PresetFeld(_name, _bezeichnung, TypName,
                    VorlagenFelder.Boxen[_boxIndex].Key, _vorbelegung,
                    _pflicht, _merken, _optionen);
        }
    }
}
