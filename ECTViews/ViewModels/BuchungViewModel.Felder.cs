// BuchungViewModel.Felder.cs -- Erweiterung des Buchen-Dialogs durch die
// aktive Buchungsvorlage
//
// Zwei Dinge, die eine Vorlage am Dialog ändern kann (Definition und
// Prüfung in ECTEngine/VorlagenFelder.cs):
//
//   ZUSATZ-FELDER     Eingabefelder, die die Vorlage deklariert. Jede Box
//                     des Dialogs hat dafür eine eigene Liste
//                     (FelderBetrag, FelderBeschreibung, ...), an die ein
//                     ItemsControl der Box gebunden ist -- die View baut
//                     also nichts im Code-Behind zusammen.
//
//   ORIGINAL-FELDER   die festen Felder (Betrag, MWSt, Konto, ...) können
//                     gesperrt, ausgeblendet oder umbenannt sein. Je Feld
//                     ein OriginalFeldZustand, an den Label, IsEnabled und
//                     Visibility gebunden sind.
//
// Aktiv ist das alles nur, solange eine erweiterte Vorlage geladen ist
// (BuchungViewModel.Gruppen.cs: UebernimmVorlage / EntferneVorlage).

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using ECTEngine;

namespace ECTViews.ViewModels
{
    public partial class BuchungViewModel
    {
        // ------------------------------------------------------------------
        // Original-Felder
        // ------------------------------------------------------------------

        public OriginalFeldZustand FeldBeschreibung { get; } = new OriginalFeldZustand(OriginalFeld.Beschreibung);
        public OriginalFeldZustand FeldBetrag { get; } = new OriginalFeldZustand(OriginalFeld.Betrag);
        public OriginalFeldZustand FeldMwst { get; } = new OriginalFeldZustand(OriginalFeld.Mwst);
        public OriginalFeldZustand FeldBeleg { get; } = new OriginalFeldZustand(OriginalFeld.Beleg);
        public OriginalFeldZustand FeldKonto { get; } = new OriginalFeldZustand(OriginalFeld.Konto);
        public OriginalFeldZustand FeldBestandskonto { get; } = new OriginalFeldZustand(OriginalFeld.Bestandskonto);
        public OriginalFeldZustand FeldBetrieb { get; } = new OriginalFeldZustand(OriginalFeld.Betrieb);
        public OriginalFeldZustand FeldAfa { get; } = new OriginalFeldZustand(OriginalFeld.Afa);

        private IEnumerable<OriginalFeldZustand> AlleOriginalFelder()
        {
            yield return FeldBeschreibung;
            yield return FeldBetrag;
            yield return FeldMwst;
            yield return FeldBeleg;
            yield return FeldKonto;
            yield return FeldBestandskonto;
            yield return FeldBetrieb;
            yield return FeldAfa;
        }

        /// <summary>Übernimmt die Original-Feld-Regeln der Vorlage
        /// (null = alle Felder zurück auf Standard).</summary>
        private void WendeOriginalFeldRegelnAn(Preset p)
        {
            foreach (var feld in AlleOriginalFelder())
                feld.Setze(p?.Erweiterung.RegelFuer(feld.Schluessel));
            MeldeFeldLayout();
        }

        // Zusammengesetzte Zustände: Vorlagen-Regel UND das, was der Dialog
        // ohnehin schon entscheidet (globale Einstellungen, vorhandene Listen)

        /// <summary>MWSt-Feld editierbar: global eingeschaltet UND von der
        /// Vorlage nicht gesperrt.</summary>
        public bool MwstEingabeAktiv => MwstFeldAktiviert && FeldMwst.Aktiv;

        /// <summary>Tooltip des deaktivierten MWSt-Felds.</summary>
        public string MwstEingabeHinweis => MwstFeldHinweis ?? FeldMwst.SperrHinweis;

        /// <summary>Die Zeile mit den Original-Feldern Betrag und MWSt:
        /// sichtbar, solange die Vorlage nicht beide ausblendet.</summary>
        public bool BetragZeileSichtbar => FeldBetrag.Sichtbar || FeldMwst.Sichtbar;

        public bool BetragBoxSichtbar => BetragZeileSichtbar || FelderBetrag.Count > 0;

        public bool ZuordnungBoxSichtbar =>
            FeldBeleg.Sichtbar || FeldKonto.Sichtbar || FelderZuordnung.Count > 0;

        /// <summary>Original-Liste der Bestandskonten: es gibt welche UND
        /// die Vorlage blendet sie nicht aus.</summary>
        public bool BestandskontoListeSichtbar =>
            BestandskontenAnzeigen && FeldBestandskonto.Sichtbar;

        public bool BestandskontoBoxSichtbar =>
            BestandskontoListeSichtbar || FelderBestandskonto.Any(f => f.Sichtbar);

        public bool BetriebListeSichtbar => BetriebeAnzeigen && FeldBetrieb.Sichtbar;

        public bool BetriebBoxSichtbar =>
            BetriebListeSichtbar || FelderBetrieb.Any(f => f.Sichtbar);

        /// <summary>Die Zeile mit den beiden Listen-Boxen.</summary>
        public bool ListenBereichSichtbar => BestandskontoBoxSichtbar || BetriebBoxSichtbar;

        /// <summary>AfA-Felder bedienbar: nur bei Ausgaben UND von der
        /// Vorlage nicht gesperrt.</summary>
        public bool AfaBereichAktiv => IstAusgabe && FeldAfa.Aktiv;

        public bool AfaBoxSichtbar => FeldAfa.Sichtbar || FelderAfa.Count > 0;

        /// <summary>Tooltip des deaktivierten AfA-Bereichs.</summary>
        public string AfaBereichHinweis => AfaFeldHinweis ?? FeldAfa.SperrHinweis;

        /// <summary>Meldet alle zusammengesetzten Layout-Zustände neu
        /// (nach Vorlagen-Wechsel und nach dem Befüllen der Listen).</summary>
        private void MeldeFeldLayout()
        {
            OnPropertyChanged(nameof(MwstEingabeAktiv));
            OnPropertyChanged(nameof(MwstEingabeHinweis));
            OnPropertyChanged(nameof(BetragZeileSichtbar));
            OnPropertyChanged(nameof(BetragBoxSichtbar));
            OnPropertyChanged(nameof(ZuordnungBoxSichtbar));
            OnPropertyChanged(nameof(BestandskontoListeSichtbar));
            OnPropertyChanged(nameof(BestandskontoBoxSichtbar));
            OnPropertyChanged(nameof(BetriebListeSichtbar));
            OnPropertyChanged(nameof(BetriebBoxSichtbar));
            OnPropertyChanged(nameof(ListenBereichSichtbar));
            OnPropertyChanged(nameof(AfaBereichAktiv));
            OnPropertyChanged(nameof(AfaBoxSichtbar));
            OnPropertyChanged(nameof(AfaBereichHinweis));
        }

        // ------------------------------------------------------------------
        // Zusatz-Felder
        // ------------------------------------------------------------------

        /// <summary>Alle Zusatz-Felder der aktiven Vorlage, in der
        /// Reihenfolge der Vorlage.</summary>
        public ObservableCollection<ZusatzFeldViewModel> Zusatzfelder { get; }
            = new ObservableCollection<ZusatzFeldViewModel>();

        // Dieselben Felder, verteilt auf die Boxen des Dialogs.
        public ObservableCollection<ZusatzFeldViewModel> FelderDatum { get; } = new ObservableCollection<ZusatzFeldViewModel>();
        public ObservableCollection<ZusatzFeldViewModel> FelderBeschreibung { get; } = new ObservableCollection<ZusatzFeldViewModel>();
        public ObservableCollection<ZusatzFeldViewModel> FelderBetrag { get; } = new ObservableCollection<ZusatzFeldViewModel>();
        public ObservableCollection<ZusatzFeldViewModel> FelderZuordnung { get; } = new ObservableCollection<ZusatzFeldViewModel>();
        public ObservableCollection<ZusatzFeldViewModel> FelderBestandskonto { get; } = new ObservableCollection<ZusatzFeldViewModel>();
        public ObservableCollection<ZusatzFeldViewModel> FelderBetrieb { get; } = new ObservableCollection<ZusatzFeldViewModel>();
        public ObservableCollection<ZusatzFeldViewModel> FelderAfa { get; } = new ObservableCollection<ZusatzFeldViewModel>();

        private ObservableCollection<ZusatzFeldViewModel> BoxListe(string box)
        {
            switch (box)
            {
                case "datum":         return FelderDatum;
                case "betrag":        return FelderBetrag;
                case "zuordnung":     return FelderZuordnung;
                case "bestandskonto": return FelderBestandskonto;
                case "betrieb":       return FelderBetrieb;
                case "afa":           return FelderAfa;
                default:              return FelderBeschreibung;
            }
        }

        private void LeereZusatzfelder()
        {
            Zusatzfelder.Clear();
            FelderDatum.Clear();
            FelderBeschreibung.Clear();
            FelderBetrag.Clear();
            FelderZuordnung.Clear();
            FelderBestandskonto.Clear();
            FelderBetrieb.Clear();
            FelderAfa.Clear();
        }

        /// <summary>
        /// Baut die Zusatz-Felder der Vorlage auf und belegt sie vor.
        /// Rangfolge der Startwerte: der in der Buchung gespeicherte Wert
        /// (Bearbeiten) -- sonst der zuletzt gebuchte, wenn das Feld sich
        /// Werte merkt -- sonst die Vorbelegung der Vorlage.
        /// </summary>
        /// <param name="gespeicherteWerte">Beim Bearbeiten die Feldwerte der
        /// Buchung (auch leer: dann gilt für jedes Feld die Vorbelegung,
        /// nicht der zuletzt gebuchte Wert); null bei einer neuen Buchung.</param>
        private void BaueZusatzfelder(Preset p, int slot,
            IReadOnlyDictionary<string, string> gespeicherteWerte)
        {
            LeereZusatzfelder();
            foreach (var feld in p.Felder)
            {
                var vm = new ZusatzFeldViewModel(feld, AktualisiereVorlage);
                VersorgeFeldMitListen(vm);

                string wert = null;
                if (gespeicherteWerte != null)
                    gespeicherteWerte.TryGetValue(feld.Name, out wert);
                else if (feld.Merken)
                {
                    wert = Einstellungen.HoleGemerktenFeldwert(slot, feld.Name);
                    if (wert.Length == 0) wert = null;
                }
                vm.SetzeRohwertStill(wert ?? VorlagenFelder.StandardWert(feld));

                Zusatzfelder.Add(vm);
                BoxListe(feld.Box).Add(vm);
            }
            MeldeFeldLayout();
        }

        /// <summary>Gibt einem Feld die Listen, aus denen es wählen lässt
        /// (MWSt-Sätze, Konten, Betriebe, Bestandskonten).</summary>
        private void VersorgeFeldMitListen(ZusatzFeldViewModel vm)
        {
            switch (vm.Feld.Typ)
            {
                case ZusatzFeldTyp.Mwst:
                    vm.SetzeOptionen(MwstOptionen);
                    break;
                case ZusatzFeldTyp.Konto:
                    // Optionen "E"/"A" legen die Kontenliste fest; sonst
                    // die der Buchungsart des Dialogs
                    string art = vm.Feld.Optionen.Trim().ToUpperInvariant();
                    bool ausgaben = art == "A" || (art != "E" && IstAusgabe);
                    vm.SetzeOptionen((ausgaben
                        ? Einstellungen.AusgabenKonten
                        : Einstellungen.EinnahmenKonten)
                        .Where(k => !string.IsNullOrEmpty(k)));
                    break;
                case ZusatzFeldTyp.Betrieb:
                    vm.SetzeEintraege(Betriebe);
                    break;
                case ZusatzFeldTyp.Bestandskonto:
                    vm.SetzeEintraege(Bestandskonten);
                    break;
            }
        }

        /// <summary>Reicht frisch geladene Betriebe/Bestandskonten an die
        /// Zusatz-Felder weiter. Nötig, weil der Aufrufer die Listen erst
        /// NACH dem Laden einer Vorlage füllen kann (Bearbeiten einer
        /// Buchung, siehe ViewHost.BefuelleListen).</summary>
        private void AktualisiereFeldListen()
        {
            foreach (var vm in Zusatzfelder)
                if (vm.Feld.Typ == ZusatzFeldTyp.Betrieb
                    || vm.Feld.Typ == ZusatzFeldTyp.Bestandskonto)
                    VersorgeFeldMitListen(vm);
            MeldeFeldLayout();
        }

        /// <summary>Geprüfte Werte aller Zusatz-Felder (Futter für den
        /// Rechner der Engine).</summary>
        private List<FeldWert> FeldWerte()
            => Zusatzfelder.Select(f => f.Wert).ToList();

        /// <summary>Aktualisiert die Fehleranzeigen der Zusatz-Felder.
        /// True, wenn kein Feld einen Fehler hat.</summary>
        private bool PruefeZusatzfelder(bool allesZeigen)
        {
            bool ok = true;
            foreach (var f in Zusatzfelder)
                ok &= f.Pruefe(allesZeigen);
            return ok;
        }

        /// <summary>Merkt die gebuchten Werte der Felder, die sich ihren
        /// letzten Wert als Vorbelegung merken sollen. Nur bei NEUEN
        /// Buchungen -- das Nachbearbeiten einer alten Buchung soll die
        /// Vorbelegung nicht verstellen.</summary>
        private void MerkeFeldWerte()
        {
            if (IstBearbeitung || _aktiveVorlage == null) return;
            foreach (var f in Zusatzfelder)
                if (f.Feld.Merken && f.Sichtbar)
                    Einstellungen.MerkeFeldwert(_aktiveVorlageSlot, f.Feld.Name, f.Rohwert);
        }

        // ------------------------------------------------------------------
        // Werte der Vorlage für die Original-Felder
        // ------------------------------------------------------------------

        /// <summary>Wählt Bestandskonto und Betrieb der Vorlage in den
        /// Listen aus (sofern die Vorlage welche nennt und es sie gibt).</summary>
        private void WaehleBasisListenwerte(Preset p)
        {
            var e = p.Erweiterung;
            if (e.BasisBestandskonto.Length > 0)
            {
                var k = Bestandskonten.FirstOrDefault(x => x.Name == e.BasisBestandskonto);
                if (k != null) SelectedBestandskonto = k;
            }
            if (e.BasisBetrieb.Length > 0)
            {
                var b = Betriebe.FirstOrDefault(x => x.Name == e.BasisBetrieb);
                if (b != null) SelectedBetrieb = b;
            }
        }

        // Marke, mit der sich feststellen lässt, ob und wo ein Template den
        // Inhalt des Beschreibungsfelds verwendet.
        private const string BeschreibungsMarke = "\u0001";

        /// <summary>BasisText-Template der Vorlage, in dem $beschreibung
        /// (bzw. $B) durch die Marke ersetzt ist; "" ohne Template.</summary>
        private static string BasisTextMitMarke(Preset p)
        {
            string t = p.Erweiterung.BasisTextTemplate;
            if (t.Length == 0) return "";
            return FormelParser.Interpoliere(t,
                new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
                {
                    ["beschreibung"] = BeschreibungsMarke, ["B"] = BeschreibungsMarke
                });
        }

        /// <summary>
        /// True, wenn die Vorlage die gebuchte Beschreibung aus dem Inhalt
        /// des Beschreibungsfelds zusammensetzt (BasisText-Template mit
        /// $beschreibung). Der Text im Feld ist dann der INHALT der Buchung
        /// und kein Vorlagenname mehr, den man beim Vorlagen-Wechsel
        /// austauschen dürfte.
        /// </summary>
        private static bool VorlageNutztBeschreibung(Preset p)
            => BasisTextMitMarke(p).IndexOf(BeschreibungsMarke, StringComparison.Ordinal) >= 0;

        /// <summary>
        /// True, wenn der Vorlagenname im Beschreibungsfeld zu einer
        /// Doppelung führte: das Template nennt ihn schon selbst und hängt
        /// den Feldinhalt nur an ("Fahrtkosten $beschreibung" bei der
        /// Vorlage "Fahrtkosten"). Dann startet das Feld leer und nimmt nur
        /// den Inhalt auf. Bei "$beschreibung (Zinsen)" dagegen ist der
        /// Vorlagenname die sinnvolle Vorbelegung.
        /// </summary>
        private static bool VorlagennameStuendeDoppelt(Preset p)
        {
            string mitMarke = BasisTextMitMarke(p);
            if (mitMarke.IndexOf(BeschreibungsMarke, StringComparison.Ordinal) < 0)
                return false;
            string name = (p.Text ?? "").Trim();
            return name.Length > 0
                && mitMarke.Replace(BeschreibungsMarke, "")
                           .IndexOf(name, StringComparison.OrdinalIgnoreCase) >= 0;
        }

        // ------------------------------------------------------------------
        // Fehler ausgeblendeter Felder
        // ------------------------------------------------------------------

        private string _versteckteFehlerText = "";
        /// <summary>
        /// Sammelt die Validierungsfehler der Felder, die die Vorlage
        /// AUSBLENDET. Deren Fehlertext steht an einem unsichtbaren Feld --
        /// ohne diese Zeile bliebe der Buchen-Knopf scheinbar grundlos
        /// wirkungslos (typisch: Betrag ausgeblendet, aber die Vorlage
        /// liefert keine Betrags-Formel).
        /// </summary>
        public string VersteckteFehlerText
        {
            get => _versteckteFehlerText;
            private set => SetProperty(ref _versteckteFehlerText, value ?? "");
        }

        private void AktualisiereVersteckteFehler()
        {
            var teile = new List<string>();
            void Pruefe(OriginalFeldZustand feld, string fehler)
            {
                if (!feld.Sichtbar && !string.IsNullOrEmpty(fehler))
                    teile.Add(feld.Titel + ": " + fehler);
            }
            Pruefe(FeldBetrag, BetragError);
            Pruefe(FeldMwst, MwstError);
            Pruefe(FeldBestandskonto, BestandskontoError);
            Pruefe(FeldBetrieb, BetriebError);
            Pruefe(FeldAfa, AfaError);

            VersteckteFehlerText = teile.Count == 0 ? ""
                : "Die Buchungsvorlage blendet Felder aus, liefert für sie aber keinen "
                  + "gültigen Wert. " + string.Join(" ", teile)
                  + " Bitte die Vorlage in den Einstellungen ergänzen -- oder unten "
                  + "\"Vorlage entfernen\" wählen, um ohne sie zu buchen.";
        }
    }
}
