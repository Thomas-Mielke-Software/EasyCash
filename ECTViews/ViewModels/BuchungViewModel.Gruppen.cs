// BuchungViewModel.Gruppen.cs -- Buchungsgruppen-Teil des Buchen-Dialogs
//
// Wird aktiv, wenn ein mehrzeiliges Preset (Buchungsgruppen-Vorlage)
// geladen ist: hält die Zusatz-Zeilen-ViewModels, rechnet sie bei jeder
// relevanten Basis-Änderung über BuchungsgruppenRechner (Engine) neu
// und baut beim Buchen die komplette, per Gruppen-UUID verknüpfte
// Buchungsliste. Die Basis-Zeile bleibt der bestehende Dialog-Feldsatz
// (kein Umbau des 1800-Zeilen-Haupt-ViewModels).

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using ECTEngine;

namespace ECTViews.ViewModels
{
    public partial class BuchungViewModel
    {
        // ------------------------------------------------------------------
        // Zustand
        // ------------------------------------------------------------------

        /// <summary>Zusatz-Zeilen der geladenen Gruppen-Vorlage (leer, wenn
        /// ein klassisches Einzel-Preset bzw. keines geladen ist).</summary>
        public ObservableCollection<ZusatzzeileViewModel> Zusatzzeilen { get; }
            = new ObservableCollection<ZusatzzeileViewModel>();

        private Preset _gruppenVorlage;
        private int _gruppenVorlageSlot = -1;
        private GruppenBerechnung _letzteBerechnung;

        // Beim Bearbeiten einer bestehenden Gruppe: deren UUID -- die neu
        // gebauten Buchungen behalten sie, damit die Gruppe ihre Identität
        // nicht verliert.
        private string _bestehendeGruppenUuid;

        /// <summary>True wenn eine Buchungsgruppen-Vorlage geladen ist
        /// (steuert die Sichtbarkeit des Zeilen-Bereichs).</summary>
        public bool HatZusatzzeilen => _gruppenVorlage != null;

        public string GruppenVorlageName => _gruppenVorlage?.Text ?? "";

        private string _gruppenSummeText = "";
        /// <summary>Summenzeile: gebuchter Basis-Betrag, Summe der
        /// Zusatz-Zeilen, verbleibender Rest.</summary>
        public string GruppenSummeText
        {
            get => _gruppenSummeText;
            private set => SetProperty(ref _gruppenSummeText, value);
        }

        private string _gruppenFehlerText = "";
        public string GruppenFehlerText
        {
            get => _gruppenFehlerText;
            private set => SetProperty(ref _gruppenFehlerText, value);
        }

        // ------------------------------------------------------------------
        // Laden / Entfernen der Vorlage
        // ------------------------------------------------------------------

        private void UebernimmGruppenVorlage(Preset p, int slot)
        {
            _gruppenVorlage = p;
            _gruppenVorlageSlot = slot;
            Zusatzzeilen.Clear();
            for (int i = 0; i < p.Zeilen.Count; i++)
                Zusatzzeilen.Add(new ZusatzzeileViewModel(
                    p.Zeilen[i], i + 1, AktualisiereZusatzzeilen));
            LoeseZeilenKontoSpezifikationen(p);
            OnPropertyChanged(nameof(HatZusatzzeilen));
            OnPropertyChanged(nameof(GruppenVorlageName));
            AktualisiereZusatzzeilen();
        }

        // ------------------------------------------------------------------
        // Ad-hoc-Kontoselektor ($de:Formular=Id|...|| im Konto-Feld)
        // ------------------------------------------------------------------

        /// <summary>
        /// Fragt den Nutzer, ein Konto für den Feld-Bedarf anzulegen
        /// (KontoAnlegenView). Vom BuchungView-Code-Behind gesetzt; liefert
        /// den Namen des angelegten Kontos oder null (abgebrochen). Null
        /// (nicht verdrahtet) = keine Anlage möglich, die Zeile behält dann
        /// ihren "kein Konto"-Fehler.
        /// </summary>
        /// <remarks>Zweites Argument: Kontoname-Vorschlag der Spezifikation
        /// ("$name=...", "" wenn keiner).</remarks>
        public Func<IReadOnlyList<KontoFeldBedarf>, string, string> KontoAnlegenAbfrage { get; set; }

        /// <summary>
        /// Löst eine Feld-Spezifikation im BASIS-Konto-Feld eines Presets
        /// zum verknüpften Konto auf; existiert keines, wird die Anlage
        /// angeboten. Normale Konto-Namen laufen unverändert durch.
        /// </summary>
        private string LoeseKontoFeld(string kontoFeld)
        {
            var a = KontoFeldSelektor.LoeseAuf(kontoFeld);
            if (!a.IstSpezifikation) return kontoFeld;
            if (a.Konto != null) return a.Konto;
            if (a.Fehler.Length > 0)
            {
                Statusleiste.Melde(a.Fehler);
                return "";
            }

            var neu = KontoAnlegenAbfrage?.Invoke(a.Bedarf, a.NameVorschlag);
            if (neu == null) return "";
            LadeKonten();   // das neue Konto in die Auswahl-Liste holen
            return neu;
        }

        /// <summary>
        /// Bietet beim Laden einer Gruppen-Vorlage die Anlage der Konten an,
        /// deren Zeilen-Spezifikation (noch) kein verknüpftes Konto findet.
        /// Bricht der Nutzer ab, bleiben weitere Dialoge aus -- die
        /// betroffenen Zeilen melden ihren Fehler dann in der Live-Anzeige
        /// und blockieren das Buchen.
        /// </summary>
        private void LoeseZeilenKontoSpezifikationen(Preset p)
        {
            if (KontoAnlegenAbfrage == null) return;
            bool angelegt = false;
            foreach (var z in p.Zeilen)
            {
                // Nach jeder Anlage neu auflösen -- mehrere Zeilen können
                // denselben Bedarf teilen.
                var a = KontoFeldSelektor.LoeseAuf(z.Konto);
                if (!a.IstSpezifikation || a.Konto != null || a.Fehler.Length > 0)
                    continue;
                if (KontoAnlegenAbfrage(a.Bedarf, a.NameVorschlag) != null)
                    angelegt = true;
                else
                    break;
            }
            if (angelegt) LadeKonten();
        }

        private RelayCommand _gruppeEntfernenCommand;
        /// <summary>Kommando des "x"-Knopfs im Zeilen-Bereich.</summary>
        public RelayCommand GruppeEntfernenCommand =>
            _gruppeEntfernenCommand ??
            (_gruppeEntfernenCommand = new RelayCommand(EntferneGruppenVorlage));

        /// <summary>Entfernt die geladene Gruppen-Vorlage samt Zeilen
        /// (Kommando des "x"-Knopfs im Zeilen-Bereich; auch beim Laden eines
        /// Einzel-Presets und beim Weiterbuchen gerufen).</summary>
        public void EntferneGruppenVorlage()
        {
            if (_gruppenVorlage == null && Zusatzzeilen.Count == 0) return;
            _gruppenVorlage = null;
            _gruppenVorlageSlot = -1;
            _letzteBerechnung = null;
            Zusatzzeilen.Clear();
            GruppenSummeText = "";
            GruppenFehlerText = "";
            OnPropertyChanged(nameof(HatZusatzzeilen));
            OnPropertyChanged(nameof(GruppenVorlageName));
        }

        // ------------------------------------------------------------------
        // Neuberechnung (live bei Basis-/Zeilen-Änderungen)
        // ------------------------------------------------------------------

        private bool _zusatzzeilenRechnenAktiv;   // Re-Entrancy-Schutz

        private void AktualisiereZusatzzeilen()
        {
            if (_gruppenVorlage == null || _zusatzzeilenRechnenAktiv) return;
            _zusatzzeilenRechnenAktiv = true;
            try
            {
                var basis = BaueBasisFuerGruppenBerechnung();
                var b = BuchungsgruppenRechner.Berechne(
                    _gruppenVorlage, basis, ManuelleBruttos(), ManuelleMwstWerte());
                _letzteBerechnung = b;

                for (int i = 0; i < Zusatzzeilen.Count && i < b.Zeilen.Count; i++)
                    Zusatzzeilen[i].UebernimmBerechnung(b.Zeilen[i]);

                decimal summeZeilen = b.Zeilen.Sum(z => z.Brutto);
                GruppenSummeText =
                    "Basis bucht " + Waehrungsformat.Betrag(b.BasisGebuchtBrutto)
                    + " + Zusatz-Zeilen " + Waehrungsformat.Betrag(summeZeilen)
                    + " (Rest " + Waehrungsformat.Betrag(b.Rest) + ")";

                GruppenFehlerText = SammleGruppenFehler(b, pruefeManuelle: _validierungAktiv);
            }
            finally
            {
                _zusatzzeilenRechnenAktiv = false;
            }
        }

        /// <summary>Basis-Buchung für die LIVE-Berechnung -- tolerant, weil
        /// die Felder während des Tippens unvollständig sein können
        /// (ungültiges Datum fällt auf heute zurück).</summary>
        private Buchung BaueBasisFuerGruppenBerechnung()
        {
            DateTime datum;
            try { datum = new DateTime(ExpandiereJahr(DatumJahr), DatumMonat, DatumTag); }
            catch { datum = DateTime.Today; }

            return new Buchung
            {
                Art = IstAusgabe ? Buchungsart.Ausgabe : Buchungsart.Einnahme,
                BruttoBetrag = Betrag.AusCent(BetragInCent, MwstPromille),
                Datum = datum,
                Beschreibung = Beschreibung,
                Belegnummer = Belegnummer,
                Konto = SelectedKonto ?? "",
                Bestandskonto = BestandskontenAnzeigen
                    ? (SelectedBestandskonto?.Name ?? "") : _bestandskontoFallback,
                Betrieb = BetriebeAnzeigen
                    ? (SelectedBetrieb?.Name ?? "") : _betriebFallback
            };
        }

        private IReadOnlyList<decimal?> ManuelleBruttos()
            => Zusatzzeilen.Select(z => z.ManuellerBrutto).ToList();

        private IReadOnlyList<int?> ManuelleMwstWerte()
            => Zusatzzeilen.Select(z => z.ManuelleMwstPromille).ToList();

        private string SammleGruppenFehler(GruppenBerechnung b, bool pruefeManuelle)
        {
            if (b.BasisFehler.Length > 0) return b.BasisFehler;
            foreach (var z in b.Zeilen)
                if (z.Fehler.Length > 0)
                    return $"Zeile {z.Index}: {z.Fehler}";
            if (pruefeManuelle)
                for (int i = 0; i < Zusatzzeilen.Count; i++)
                    if (Zusatzzeilen[i].BetragEditierbar && Zusatzzeilen[i].ManuellerBrutto == null)
                        return $"Zeile {i + 1}: Bitte einen Betrag eingeben.";
            return "";
        }

        // ------------------------------------------------------------------
        // Validierung + Ergebnis
        // ------------------------------------------------------------------

        /// <summary>Validiert die Gruppen-Zeilen (Formelfehler, fehlende
        /// manuelle Beträge). True wenn keine Vorlage geladen oder alles ok.</summary>
        private bool ValidiereGruppe()
        {
            if (_gruppenVorlage == null) { GruppenFehlerText = ""; return true; }
            AktualisiereZusatzzeilen();
            var fehler = _letzteBerechnung == null
                ? "" : SammleGruppenFehler(_letzteBerechnung, pruefeManuelle: true);
            GruppenFehlerText = fehler;
            return fehler.Length == 0;
        }

        /// <summary>Baut alle zu buchenden Buchungen: ohne Gruppen-Vorlage
        /// genau die Basis-Buchung, sonst Basis + Zusatz-Zeilen mit
        /// gemeinsamer Gruppen-UUID (beim Bearbeiten bleibt die UUID der
        /// bestehenden Gruppe erhalten).</summary>
        private List<Buchung> BaueAlleBuchungen()
        {
            var basis = BaueBuchungAusFeldern();
            if (_gruppenVorlage == null)
            {
                // Wurde die Gruppe während der Bearbeitung aufgelöst
                // ("Vorlage entfernen"), darf die Basis die alten
                // Gruppen-Keys nicht behalten.
                if (_bestehendeGruppenUuid != null)
                    basis.EntferneGruppe();
                return new List<Buchung> { basis };
            }

            var berechnung = BuchungsgruppenRechner.Berechne(
                _gruppenVorlage, basis, ManuelleBruttos(), ManuelleMwstWerte());
            return BuchungsgruppenRechner.BaueBuchungen(
                _gruppenVorlage, _gruppenVorlageSlot, basis, berechnung,
                _bestehendeGruppenUuid);
        }

        // ------------------------------------------------------------------
        // Bearbeiten einer bestehenden Gruppe
        // ------------------------------------------------------------------

        /// <summary>
        /// Lädt beim BEARBEITEN eines Gruppenmitglieds die komplette Gruppe
        /// in den Dialog: Vorlagen-Zeilen wie bei der Neuerfassung, manuelle
        /// Betrag-/MWSt-Felder aus den bestehenden Zusatz-Buchungen
        /// vorbelegt (Zuordnung über die Gruppen-Rolle). Bei belegweiser
        /// Aufteilung (BasisBetragFormel gesetzt) wird der ursprünglich
        /// eingegebene Gesamtbetrag als Basis-Brutto plus Summe der
        /// Zusatz-Zeilen rekonstruiert.
        /// Aufrufer: ViewHost.ZeigeBuchungBearbeitenKombiDialog.
        /// </summary>
        public void LadeGruppeFuerBearbeitung(Preset vorlage, int slot,
            IReadOnlyList<Buchung> zusatzBuchungen, string gruppenUuid)
        {
            _bestehendeGruppenUuid = gruppenUuid;
            UebernimmGruppenVorlage(vorlage, slot);

            // Eingabe-Gesamtbetrag rekonstruieren (exakt für "$rest";
            // für andere Basis-Formeln eine Näherung)
            if (!string.IsNullOrEmpty(vorlage.BasisBetragFormel)
                && zusatzBuchungen != null && zusatzBuchungen.Count > 0)
            {
                decimal gesamt = (BetragInCent / 100m)
                    + zusatzBuchungen.Sum(b => b.BruttoBetrag.BruttoWert);
                BetragText = Waehrungsformat.BetragOhneGruppierung(gesamt);
            }

            // Manuelle Felder aus den bestehenden Buchungen vorbelegen
            if (zusatzBuchungen != null)
            {
                foreach (var m in zusatzBuchungen)
                {
                    int i = m.GruppenRolle - 1;
                    if (i < 0 || i >= Zusatzzeilen.Count) continue;
                    var z = Zusatzzeilen[i];
                    if (z.BetragEditierbar)
                        z.BetragText = Waehrungsformat.BetragOhneGruppierung(
                            m.BruttoBetrag.BruttoWert);
                    if (z.MwstEditierbar)
                        z.MwstText = Waehrungsformat.Zahl(
                            m.BruttoBetrag.MwstPromille / 1000m);
                }
            }

            AktualisiereZusatzzeilen();
        }

        // ------------------------------------------------------------------
        // Umwandeln einer bestehenden Buchung in eine Buchungsgruppe
        //
        // Zwei Einstiege, eine Mechanik:
        //   - Journal-Kontextmenue "Umwandeln in <Vorlage>": die Vorlage
        //     kommt als VorgewaehltesPreset in den Bearbeiten-Dialog, der
        //     sie beim Oeffnen anwendet. Dort ist keine Rueckfrage noetig --
        //     der Dialog zeigt das Ergebnis und speichert erst auf Knopfdruck.
        //   - Vorlagen-Dropdown des Beschreibungsfelds im Bearbeiten-Modus:
        //     hier ist die Auswahl schnell mal versehentlich passiert, darum
        //     die Sicherheitsabfrage in BestaetigeGruppenWechsel.
        // ------------------------------------------------------------------

        /// <summary>
        /// Sicherheitsabfrage vor dem Umwandeln/Umstellen/Aufloesen einer
        /// Buchungsgruppe im Bearbeiten-Modus (Ja/Nein). Wird vom
        /// BuchungView-Code-Behind gesetzt, damit die MessageBox den Dialog
        /// als Owner bekommt. Args: (Text, Titel); nicht verdrahtet =
        /// im Zweifel nichts aendern.
        /// </summary>
        public Func<string, string, bool> UmwandlungBestaetigen { get; set; }

        // Gesetzt, waehrend eine ueber das Journal ausdruecklich angeforderte
        // Umwandlung laeuft -- dann entfaellt die Rueckfrage.
        private bool _umwandlungOhneRueckfrage;

        /// <summary>Slot der geladenen Gruppen-Vorlage (-1 = keine).</summary>
        public int GruppenVorlagenSlot => _gruppenVorlageSlot;

        /// <summary>
        /// Wandelt die bearbeitete Buchung in eine Buchungsgruppe um bzw.
        /// stellt eine bestehende Gruppe auf die angegebene Vorlage um --
        /// ohne Rueckfrage, weil der Aufrufer die Vorlage ausdruecklich
        /// gewaehlt hat (Journal-Kontextmenue). Die Zusatz-Buchungen
        /// entstehen erst beim Speichern.
        /// </summary>
        /// <returns>False, wenn der Slot keine zur Buchungsart passende
        /// Gruppen-Vorlage enthaelt (Buchung bleibt dann unveraendert).</returns>
        public bool WandleInGruppeUm(int presetSlot)
        {
            var presets = Einstellungen.Presets;
            if (presetSlot < 0 || presetSlot >= presets.Count) return false;
            var p = presets[presetSlot];
            if (p.IstLeer || !p.IstMehrzeilig || p.Ausgabe != IstAusgabe)
                return false;

            _umwandlungOhneRueckfrage = true;
            try { LadePresetInFelder(p, presetSlot, beschreibungErhalten: true); }
            finally { _umwandlungOhneRueckfrage = false; }
            return true;
        }

        /// <summary>
        /// Rueckfrage, wenn die Wahl einer Vorlage im Bearbeiten-Modus die
        /// Gruppen-Zugehoerigkeit der Buchung aendert (umwandeln, umstellen,
        /// aufloesen). True = weitermachen. Aendert die Vorlage an der
        /// Gruppe nichts (klassische Vorlage auf einer normalen Buchung),
        /// wird nicht gefragt.
        /// </summary>
        private bool BestaetigeGruppenWechsel(Preset p)
        {
            if (_umwandlungOhneRueckfrage) return true;

            bool hatGruppe = _gruppenVorlage != null;
            if (!hatGruppe && !p.IstMehrzeilig) return true;

            string text;
            if (!hatGruppe)
            {
                text = "Diese Buchung in eine Buchungsgruppe umwandeln?\n\n"
                     + "Vorlage: \"" + p.Text + "\" (" + ZeilenAnzahlText(p) + ")\n\n"
                     + "Konto, MWSt. und AfA werden aus der Vorlage übernommen; "
                     + "Datum, Betrag und Belegnummer bleiben erhalten.\n"
                     + BeschreibungsHinweis(p) + "\n\n"
                     + "Die zusätzlichen Buchungen entstehen erst beim Speichern.";
            }
            else if (p.IstMehrzeilig)
            {
                text = "Die Buchungsgruppe auf die Vorlage \"" + p.Text
                     + "\" umstellen?\n\n"
                     + "Die bisherigen Zusatz-Buchungen werden beim Speichern "
                     + "durch " + ZeilenAnzahlText(p) + " ersetzt.\n"
                     + BeschreibungsHinweis(p);
            }
            else
            {
                text = "Die Buchungsgruppe auflösen?\n\n"
                     + "\"" + p.Text + "\" ist eine einfache Buchungsvorlage. "
                     + "Beim Speichern bleibt nur diese eine Buchung übrig, "
                     + "die Zusatz-Buchungen der Gruppe werden entfernt.";
            }

            if (UmwandlungBestaetigen == null) return false;
            return UmwandlungBestaetigen(text, "Buchungsgruppe");
        }

        private static string ZeilenAnzahlText(Preset p)
            => p.Zeilen.Count == 1
                ? "1 zusätzliche Buchung"
                : p.Zeilen.Count + " zusätzliche Buchungen";

        /// <summary>Sagt in der Rueckfrage an, was mit dem vorhandenen
        /// Beschreibungstext passiert (siehe BeschreibungIstAutomatisch).</summary>
        private string BeschreibungsHinweis(Preset p)
            => BeschreibungIstAutomatisch()
                ? "Als Beschreibung wird \"" + p.Text + "\" eingesetzt."
                : "Die Beschreibung \"" + Beschreibung + "\" bleibt erhalten.";

        /// <summary>
        /// True, wenn der aktuelle Beschreibungstext NICHT von Hand stammt:
        /// leer oder wortgleich mit dem Text einer Buchungsvorlage (dann hat
        /// ihn eine fruehere Vorlagen-Wahl eingesetzt). Nur solche Texte darf
        /// eine Vorlage beim Bearbeiten ueberschreiben -- ein selbst
        /// getippter Text ("Hotel Berlin, 3 Naechte") ist der eigentliche
        /// Inhalt der Buchung und ueberlebt die Umwandlung. Er ist ausserdem
        /// die bessere Grundlage fuer die Zeilen-Templates der Vorlage
        /// ($B liefert dann "Hotel Berlin, 3 Naechte" statt des
        /// Vorlagen-Namens).
        /// </summary>
        private bool BeschreibungIstAutomatisch()
        {
            var t = (Beschreibung ?? "").Trim();
            if (t.Length == 0) return true;
            foreach (var vorlage in Einstellungen.Presets)
                if (!vorlage.IstLeer && !string.IsNullOrEmpty(vorlage.Text)
                    && string.Equals(vorlage.Text.Trim(), t,
                        StringComparison.OrdinalIgnoreCase))
                    return true;
            return false;
        }
    }

    // ======================================================================
    // ViewModel einer einzelnen Zusatz-Zeile
    // ======================================================================

    public class ZusatzzeileViewModel : ViewModelBase
    {
        private readonly Action _onGeaendert;

        public PresetZeile Vorlage { get; }
        /// <summary>1-basierter Zeilenindex (= spätere Gruppen-Rolle).</summary>
        public int Index { get; }

        public ZusatzzeileViewModel(PresetZeile vorlage, int index, Action onGeaendert)
        {
            Vorlage = vorlage;
            Index = index;
            _onGeaendert = onGeaendert;
        }

        // ------------------------------------------------------------------
        // Editierbare Felder (nur wo die Vorlage sie offen lässt)
        // ------------------------------------------------------------------

        public bool BetragEditierbar => Vorlage.BetragManuell;
        public bool MwstEditierbar => string.IsNullOrEmpty(Vorlage.MwstAusdruck);

        private string _betragText = "";
        public string BetragText
        {
            get => _betragText;
            set
            {
                if (SetProperty(ref _betragText, value ?? ""))
                    _onGeaendert?.Invoke();
            }
        }

        /// <summary>Geparster manueller Brutto-Betrag; null wenn leer oder
        /// nicht parsbar (Validierung meldet das beim Buchen).</summary>
        public decimal? ManuellerBrutto
        {
            get
            {
                if (!BetragEditierbar) return null;
                return Waehrungsformat.TryParse(_betragText, out decimal d)
                    ? d : (decimal?)null;
            }
        }

        private string _mwstText = "";
        public string MwstText
        {
            get => _mwstText;
            set
            {
                if (SetProperty(ref _mwstText, value ?? ""))
                    _onGeaendert?.Invoke();
            }
        }

        /// <summary>Manuell gewählte MWSt als x1000-Wert; null = Feld leer
        /// (dann gilt der MWSt-Satz der Basis).</summary>
        public int? ManuelleMwstPromille
        {
            get
            {
                if (!MwstEditierbar) return null;
                if (!Waehrungsformat.TryParseProzent(_mwstText, out decimal d)
                    || d < 0 || d > 100)
                    return null;
                return (int)decimal.Round(d * 1000m, 0, MidpointRounding.AwayFromZero);
            }
        }

        // ------------------------------------------------------------------
        // Anzeige (Ergebnis der Engine-Berechnung)
        // ------------------------------------------------------------------

        private string _artText = "";
        public string ArtText { get => _artText; private set => SetProperty(ref _artText, value); }

        private string _kontoText = "";
        public string KontoText { get => _kontoText; private set => SetProperty(ref _kontoText, value); }

        private string _beschreibungText = "";
        public string BeschreibungText { get => _beschreibungText; private set => SetProperty(ref _beschreibungText, value); }

        private string _belegText = "";
        public string BelegText { get => _belegText; private set => SetProperty(ref _belegText, value); }

        private string _betragAnzeige = "";
        /// <summary>Berechneter Brutto-Betrag (readonly-Anzeige bei Formel-Zeilen).</summary>
        public string BetragAnzeige { get => _betragAnzeige; private set => SetProperty(ref _betragAnzeige, value); }

        private string _mwstAnzeige = "";
        public string MwstAnzeige { get => _mwstAnzeige; private set => SetProperty(ref _mwstAnzeige, value); }

        private string _zeilenFehler = "";
        public string ZeilenFehler { get => _zeilenFehler; private set => SetProperty(ref _zeilenFehler, value); }

        /// <summary>Tooltip der Betrag-Anzeige: die Formel aus der Vorlage.</summary>
        public string BetragFormelTooltip =>
            string.IsNullOrEmpty(Vorlage.BetragFormel) ? null : "Formel: " + Vorlage.BetragFormel;

        public string MwstTooltip =>
            string.IsNullOrEmpty(Vorlage.MwstAusdruck) ? null : "Aus Vorlage: " + Vorlage.MwstAusdruck;

        internal void UebernimmBerechnung(GruppenZeile z)
        {
            ArtText = z.Art == Buchungsart.Einnahme ? "Einnahme" : "Ausgabe";
            KontoText = z.Konto;
            BeschreibungText = z.Beschreibung;
            BelegText = z.Belegnummer;
            if (!BetragEditierbar)
                BetragAnzeige = Waehrungsformat.Betrag(z.Brutto);
            if (!MwstEditierbar)
                MwstAnzeige = Waehrungsformat.Zahl(z.MwstPromille / 1000m);
            ZeilenFehler = z.Fehler;
        }

        // ------------------------------------------------------------------
        // Darstellung (additiv/kompakt vs. Masken-Serie)
        // ------------------------------------------------------------------

        /// <summary>Reine Formelzeilen (keine Eingabefelder) rendern nicht --
        /// sie erscheinen nur in der Summenzeile. Override "maske" macht sie
        /// trotzdem sichtbar.</summary>
        public bool IstSichtbar =>
            BetragEditierbar || MwstEditierbar || IstMaske;

        /// <summary>Volle Zeilen-Maske: per Vorlagen-Override
        /// (Darstellung=maske); die Heuristik "&gt;2 Eingabefelder" kann bei
        /// den aktuellen zwei Eingabefeldern nicht greifen.</summary>
        public bool IstMaske =>
            string.Equals(Vorlage.Darstellung, "maske", StringComparison.OrdinalIgnoreCase);

        /// <summary>Kompakte (additive) Darstellung: nur die Eingabefelder.</summary>
        public bool IstKompakt => IstSichtbar && !IstMaske;
    }
}
