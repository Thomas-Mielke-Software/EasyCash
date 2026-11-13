// BuchungViewModel.Gruppen.cs -- die AKTIVE BUCHUNGSVORLAGE im Buchen-Dialog
//
// Eine "erweiterte" Vorlage (Preset.IstErweitert: Zusatz-Zeilen,
// Zusatz-Felder, Original-Feld-Regeln oder Formeln/Templates für die
// Basis) belegt die Felder nicht nur einmalig vor, sondern bleibt bis zum
// Buchen aktiv: der Dialog rechnet bei jeder relevanten Änderung über
// BuchungsgruppenRechner (Engine) neu, zeigt die entstehenden
// Zusatz-Buchungen als Vorschau und baut beim Buchen die komplette, per
// Gruppen-UUID verknüpfte Buchungsliste. Die Basis-Zeile bleibt der
// bestehende Dialog-Feldsatz (kein Umbau des Haupt-ViewModels); die
// Zusatz-Felder und Original-Feld-Regeln stehen in
// BuchungViewModel.Felder.cs.
//
// Eingaben PRO ZUSATZ-ZEILE gibt es nicht mehr (früher: Betrag und MWSt
// im Zeilen-Bereich) -- was eine Vorlage abfragen will, deklariert sie als
// Zusatz-Feld. Der Zeilen-Bereich ist reine Vorschau.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Text;
using ECTEngine;

namespace ECTViews.ViewModels
{
    public partial class BuchungViewModel
    {
        // ------------------------------------------------------------------
        // Zustand
        // ------------------------------------------------------------------

        /// <summary>Vorschau der Zusatz-Zeilen der aktiven Vorlage (leer,
        /// wenn sie keine hat bzw. keine Vorlage aktiv ist).</summary>
        public ObservableCollection<ZusatzzeileViewModel> Zusatzzeilen { get; }
            = new ObservableCollection<ZusatzzeileViewModel>();

        // Die aktive (erweiterte) Vorlage; null = keine. Ein klassisches
        // Einzel-Preset wird nie aktiv, es belegt nur die Felder vor.
        private Preset _aktiveVorlage;
        private int _aktiveVorlageSlot = -1;
        private GruppenBerechnung _letzteBerechnung;

        // Beim Bearbeiten einer bestehenden Gruppe: deren UUID -- die neu
        // gebauten Buchungen behalten sie, damit die Gruppe ihre Identität
        // nicht verliert.
        private string _bestehendeGruppenUuid;

        // Die bearbeitete Buchung wurde MIT ihrer Vorlage geöffnet (siehe
        // LadeVorlageFuerBearbeitung). Entfernt der Anwender die Vorlage,
        // muss auch der Vorlagen-Bezug aus der Buchung verschwinden.
        private bool _mitVorlageGeoeffnet;

        /// <summary>True wenn eine erweiterte Vorlage aktiv ist (steuert
        /// die Sichtbarkeit des Vorlagen-Bereichs).</summary>
        public bool HatAktiveVorlage => _aktiveVorlage != null;

        /// <summary>True wenn die aktive Vorlage Zusatz-Zeilen hat
        /// (Vorschau-Liste sichtbar).</summary>
        public bool HatZusatzzeilen => Zusatzzeilen.Count > 0;

        public string AktiveVorlageName => _aktiveVorlage?.Text ?? "";

        /// <summary>Überschrift des Vorlagen-Bereichs.</summary>
        public string VorlagenBereichTitel =>
            _aktiveVorlage != null && _aktiveVorlage.IstMehrzeilig
                ? "Buchungsgruppe: " : "Buchungsvorlage: ";

        private string _beschreibungVorschau = "";
        /// <summary>Die Beschreibung, die tatsächlich gebucht wird, wenn die
        /// Vorlage sie aus einem Template zusammensetzt ("" = der Inhalt
        /// des Beschreibungsfelds wird unverändert gebucht).</summary>
        public string BeschreibungVorschau
        {
            get => _beschreibungVorschau;
            private set => SetProperty(ref _beschreibungVorschau, value ?? "");
        }

        private string _vorlagenHinweis = "";
        /// <summary>Hinweis über den Feldern, z.B. wenn eine Buchung nicht
        /// mehr mit ihrer Vorlage geöffnet werden konnte (vom ViewHost
        /// gesetzt).</summary>
        public string VorlagenHinweis
        {
            get => _vorlagenHinweis;
            set => SetProperty(ref _vorlagenHinweis, value ?? "");
        }

        /// <summary>
        /// True, wenn die bearbeitete Buchung auf eine Vorlage verweist, die
        /// es so nicht mehr gibt (Platz leer oder anders belegt). Die
        /// Buchung wird dann einzeln bearbeitet; beim Speichern verliert
        /// eine Einzelbuchung den toten Vorlagen-Bezug.
        /// </summary>
        public bool VorlagenBezugVeraltet { get; set; }

        private string _gruppenSummeText = "";
        /// <summary>Summenzeile der Gruppe: die entstehenden Buchungen,
        /// zusammengefasst nach dem Merkmal, in dem sie sich unterscheiden
        /// (nie über Richtungen hinweg addiert) -- siehe BaueSummeText.</summary>
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

        /// <summary>Macht <paramref name="p"/> zur aktiven Vorlage: Vorschau-
        /// Zeilen, Original-Feld-Regeln und Zusatz-Felder aufbauen, dann
        /// rechnen.</summary>
        /// <param name="gespeicherteWerte">Beim Bearbeiten die in der
        /// Buchung gemerkten Feldwerte; null bei einer neuen Buchung (siehe
        /// BaueZusatzfelder).</param>
        private void UebernimmVorlage(Preset p, int slot,
            IReadOnlyDictionary<string, string> gespeicherteWerte = null)
        {
            _aktiveVorlage = p;
            _aktiveVorlageSlot = slot;
            Zusatzzeilen.Clear();
            for (int i = 0; i < p.Zeilen.Count; i++)
                Zusatzzeilen.Add(new ZusatzzeileViewModel(i + 1));
            WendeOriginalFeldRegelnAn(p);
            BaueZusatzfelder(p, slot, gespeicherteWerte);
            LoeseZeilenKontoSpezifikationen(p);
            MeldeVorlagenWechsel();
            AktualisiereVorlage();
        }

        private void MeldeVorlagenWechsel()
        {
            OnPropertyChanged(nameof(HatAktiveVorlage));
            OnPropertyChanged(nameof(HatZusatzzeilen));
            OnPropertyChanged(nameof(AktiveVorlageName));
            OnPropertyChanged(nameof(VorlagenBereichTitel));
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

        private RelayCommand _vorlageEntfernenCommand;
        /// <summary>Kommando des Knopfs "Vorlage entfernen".</summary>
        public RelayCommand VorlageEntfernenCommand =>
            _vorlageEntfernenCommand ??
            (_vorlageEntfernenCommand = new RelayCommand(EntferneVorlage));

        /// <summary>Entfernt die aktive Vorlage: Zusatz-Zeilen und
        /// Zusatz-Felder verschwinden, die Original-Felder sind wieder
        /// sichtbar, editierbar und tragen ihre Standard-Bezeichnung. Die
        /// schon vorbelegten Feldwerte bleiben stehen. (Knopf "Vorlage
        /// entfernen"; auch beim Laden eines Einzel-Presets und beim
        /// Weiterbuchen gerufen.)</summary>
        public void EntferneVorlage()
        {
            if (_aktiveVorlage == null && Zusatzzeilen.Count == 0
                && Zusatzfelder.Count == 0) return;
            _aktiveVorlage = null;
            _aktiveVorlageSlot = -1;
            _letzteBerechnung = null;
            Zusatzzeilen.Clear();
            LeereZusatzfelder();
            WendeOriginalFeldRegelnAn(null);
            GruppenSummeText = "";
            GruppenFehlerText = "";
            BeschreibungVorschau = "";
            VersteckteFehlerText = "";
            MeldeVorlagenWechsel();
        }

        // ------------------------------------------------------------------
        // Neuberechnung (live bei Änderungen an Basis- und Zusatz-Feldern)
        // ------------------------------------------------------------------

        private bool _vorlageRechnenAktiv;   // Re-Entrancy-Schutz

        private void AktualisiereVorlage()
        {
            if (_aktiveVorlage == null || _vorlageRechnenAktiv) return;
            _vorlageRechnenAktiv = true;
            try
            {
                var felder = FeldWerte();
                PruefeZusatzfelder(allesZeigen: _validierungAktiv);

                // Gesperrtes/ausgeblendetes Betragsfeld: der Betrag kommt
                // aus der Formel der Vorlage und steht (bei "gesperrt")
                // read-only im Feld. Der BetragText-Setter ruft hierher
                // zurück -- das fängt der Re-Entrancy-Schutz ab.
                string eingabeFehler = "";
                if (!FeldBetrag.Aktiv)
                {
                    var e = BuchungsgruppenRechner.BerechneEingabeBetrag(_aktiveVorlage, felder);
                    if (e != null && e.Ok)
                        BetragText = Waehrungsformat.BetragOhneGruppierung(e.Wert);
                    else if (e != null)
                        eingabeFehler = FeldBetrag.Titel + ": " + e.Fehler;
                }

                var basis = BaueBasisFuerGruppenBerechnung();
                var b = BuchungsgruppenRechner.Berechne(_aktiveVorlage, basis, felder);
                _letzteBerechnung = b;

                for (int i = 0; i < Zusatzzeilen.Count && i < b.Zeilen.Count; i++)
                    Zusatzzeilen[i].UebernimmBerechnung(b.Zeilen[i], basis);

                GruppenSummeText = BaueSummeText(b, basis);

                BeschreibungVorschau =
                    _aktiveVorlage.Erweiterung.BasisTextTemplate.Length > 0
                        ? "Wird gebucht als: " + b.BasisBeschreibung : "";

                GruppenFehlerText = eingabeFehler.Length > 0
                    ? eingabeFehler : SammleVorlagenFehler(b);

                if (_validierungAktiv) AktualisiereVersteckteFehler();
            }
            finally
            {
                _vorlageRechnenAktiv = false;
            }
        }

        /// <summary>
        /// Summenzeile der Gruppe. Statt abstrakt "Basis"/"Zusatz" nennt sie
        /// das Merkmal, in dem sich die entstehenden Buchungen unterscheiden
        /// -- bei einer Aufteilung auf zwei Betriebe also die Betriebe:
        /// "Je Betrieb: Laden 70,00 Ausgabe; Werkstatt 30,00 Ausgabe".
        /// Gesucht wird in der Reihenfolge Betrieb, Bestandskonto, Konto,
        /// Steuersatz (siehe <see cref="SummenMerkmal"/>).
        ///
        /// Leitregel bleibt: NIE über Buchungsrichtungen hinweg addieren.
        /// Eine Gruppe kann Zeilen enthalten, die einander aufheben (Reverse
        /// Charge: geschuldete USt als Einnahme, Vorsteuer als Ausgabe) --
        /// eine Gesamtsumme suggeriert dort eine Bewegung, die es nicht
        /// gibt. Zusammengefasst wird darum nur innerhalb (Merkmal, Richtung).
        /// </summary>
        private string BaueSummeText(GruppenBerechnung b, Buchung basis)
        {
            // Ohne Zusatz-Zeilen und ohne eigene Basis-Formel bucht die
            // Vorlage genau das, was im Betragsfeld steht -- dann sagt die
            // Zeile nichts Neues. Ausnahme: die Vorlage blendet das
            // Betragsfeld aus; hier ist die Summenzeile die einzige Stelle,
            // an der der berechnete Betrag zu sehen ist.
            if (b.Zeilen.Count == 0 && FeldBetrag.Sichtbar
                && string.IsNullOrWhiteSpace(_aktiveVorlage?.BasisBetragFormel))
                return "";

            // Alle Buchungen, die entstehen: die Basis plus die Zeilen, die
            // auch gebucht werden.
            var posten = new List<SummenPosten>
            {
                new SummenPosten(basis.Art, b.BasisGebuchtBrutto, basis.Betrieb,
                                 basis.Bestandskonto, basis.Konto, MwstPromille)
            };
            posten.AddRange(b.GebuchteZeilen.Select(z => new SummenPosten(
                z.Art, z.Brutto, z.Betrieb, z.Bestandskonto, z.Konto, z.MwstPromille)));

            string text;
            if (posten.Count == 1)
            {
                // Nur die Basis bucht (Zusatz-Zeilen alle 0): nichts zu
                // unterscheiden.
                text = "Gebucht wird " + Waehrungsformat.Betrag(posten[0].Brutto)
                     + " " + ArtWort(posten[0].Art);
            }
            else
            {
                var merkmal = SummenMerkmal(posten, out string ueberschrift);
                if (merkmal == null)
                {
                    // Nichts unterscheidet die Buchungen (z.B. zweiter Betrieb
                    // = erster): nur Anzahl und Summe je Richtung.
                    var jeArt = posten.GroupBy(p => p.Art)
                        .Select(g => Waehrungsformat.Betrag(g.Sum(p => p.Brutto)) + " " + ArtWort(g.Key));
                    text = posten.Count + " Buchungen, zusammen " + string.Join("; ", jeArt);
                }
                else
                {
                    var gruppen = posten
                        .GroupBy(p => new { Wert = merkmal(p), p.Art })
                        .Select(g => g.Key.Wert + " " + Waehrungsformat.Betrag(g.Sum(p => p.Brutto))
                                     + " " + ArtWort(g.Key.Art));
                    // Semikolon, nicht Komma: die Beträge selbst enthalten
                    // Kommas ("Laden 70,00 Ausgabe; Werkstatt 30,00 Ausgabe")
                    text = ueberschrift + ": " + string.Join("; ", gruppen);
                }
            }

            return text + RestHinweis(b);
        }

        private sealed class SummenPosten
        {
            public Buchungsart Art { get; }
            public decimal Brutto { get; }
            public string Betrieb { get; }
            public string Bestandskonto { get; }
            public string Konto { get; }
            public int MwstPromille { get; }

            public SummenPosten(Buchungsart art, decimal brutto, string betrieb,
                string bestandskonto, string konto, int mwstPromille)
            {
                Art = art; Brutto = brutto;
                Betrieb = betrieb ?? ""; Bestandskonto = bestandskonto ?? "";
                Konto = konto ?? ""; MwstPromille = mwstPromille;
            }
        }

        private static string ArtWort(Buchungsart art)
            => art == Buchungsart.Ausgabe ? "Ausgabe" : "Einnahme";

        /// <summary>Das erste Merkmal, in dem sich die Buchungen
        /// unterscheiden; null, wenn sie sich in keinem unterscheiden.</summary>
        private static Func<SummenPosten, string> SummenMerkmal(
            List<SummenPosten> posten, out string ueberschrift)
        {
            bool Verschieden(Func<SummenPosten, string> f)
                => posten.Select(f).Distinct(StringComparer.Ordinal).Skip(1).Any();

            Func<SummenPosten, string> betrieb =
                p => p.Betrieb.Length > 0 ? p.Betrieb : "ohne Betrieb";
            Func<SummenPosten, string> bestandskonto =
                p => p.Bestandskonto.Length > 0 ? p.Bestandskonto : "ohne Bestandskonto";
            Func<SummenPosten, string> konto =
                p => p.Konto.Length > 0 ? p.Konto : "ohne Konto";
            Func<SummenPosten, string> mwst =
                p => Waehrungsformat.Zahl(p.MwstPromille / 1000m) + " %";

            if (Verschieden(betrieb))       { ueberschrift = "Je Betrieb";       return betrieb; }
            if (Verschieden(bestandskonto)) { ueberschrift = "Je Bestandskonto"; return bestandskonto; }
            if (Verschieden(konto))         { ueberschrift = "Je Konto";         return konto; }
            if (Verschieden(mwst))          { ueberschrift = "Je Steuersatz";    return mwst; }
            ueberschrift = "";
            return null;
        }

        /// <summary>
        /// "(Rest ...)" nur bei Vorlagen, die den Eingabebetrag AUFTEILEN.
        /// Der Rest ist die Restgröße der Aufteilungs-Mechanik: Eingabe minus
        /// Zusatz-Zeilen, gedacht als $rest für die BasisBetragFormel (Herkunft
        /// Privat-Split). Hat die Vorlage keine solche Formel, bucht die Basis
        /// den vollen Eingabebetrag und die Zeilen kommen OBENDRAUF -- dann
        /// beschreibt der Rest nichts und liest sich wie ein nicht zugeordneter
        /// Betrag (typisch bei Reverse Charge: "Basis bucht 18,00 +
        /// Zusatz-Zeilen 6,84 (Rest 11,16)", obwohl 18,00 gebucht werden).
        /// </summary>
        private string RestHinweis(GruppenBerechnung b)
        {
            if (string.IsNullOrEmpty(_aktiveVorlage?.BasisBetragFormel)) return "";
            // Der Normalfall der Aufteilung ist BasisBetrag = "$rest": dann
            // IST der Rest schon der Basis-Betrag und stünde nur doppelt da.
            // Interessant wird die Zahl erst, wenn die Formel den Rest NICHT
            // vollständig aufbraucht -- dann ist wirklich etwas offen.
            if (b.Rest == b.BasisGebuchtBrutto) return "";
            // in Klammern: "; " trennt in der Summenzeile die Gruppen
            return " (nicht zugeordnet " + Waehrungsformat.Betrag(b.Rest) + ")";
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

        /// <summary>Erster Fehler der Berechnung (Basis vor Zeilen). Fehler
        /// der Zusatz-Felder stehen an den Feldern selbst.</summary>
        private static string SammleVorlagenFehler(GruppenBerechnung b)
        {
            if (b.BasisFehler.Length > 0) return b.BasisFehler;
            foreach (var z in b.Zeilen)
                if (z.Fehler.Length > 0)
                    return $"Zeile {z.Index}: {z.Fehler}";
            return "";
        }

        // ------------------------------------------------------------------
        // Validierung + Ergebnis
        // ------------------------------------------------------------------

        /// <summary>Validiert die aktive Vorlage: Zusatz-Felder (inkl.
        /// Pflichtangaben) und die Berechnung (Formelfehler, aufgezehrter
        /// Basis-Betrag). True wenn keine Vorlage aktiv oder alles ok.</summary>
        private bool ValidiereVorlage()
        {
            if (_aktiveVorlage == null) { GruppenFehlerText = ""; return true; }
            AktualisiereVorlage();
            bool felderOk = PruefeZusatzfelder(allesZeigen: true);
            return felderOk && GruppenFehlerText.Length == 0;
        }

        /// <summary>Baut alle zu buchenden Buchungen: ohne aktive Vorlage
        /// genau die Basis-Buchung, sonst rechnet die Engine -- Basis mit
        /// Vorlagen-Bezug plus die Zusatz-Buchungen mit gemeinsamer
        /// Gruppen-UUID (beim Bearbeiten bleibt die UUID der bestehenden
        /// Gruppe erhalten).</summary>
        private List<Buchung> BaueAlleBuchungen()
        {
            var basis = BaueBuchungAusFeldern();
            if (_aktiveVorlage == null)
            {
                // Wurde die Vorlage während der Bearbeitung entfernt, darf
                // die Buchung weder die alten Gruppen-Keys noch die
                // Eingaben der Vorlage behalten.
                if (_bestehendeGruppenUuid != null || _mitVorlageGeoeffnet)
                    basis.EntferneGruppe();
                // Verweis auf eine Vorlage, die es so nicht mehr gibt: eine
                // Einzelbuchung wird ihn los (ein Gruppen-Mitglied braucht
                // den Platz-Verweis weiter für seine Gruppe).
                else if (VorlagenBezugVeraltet && basis.GruppenUuid == null)
                    basis.EntferneVorlagenBezug();
                return new List<Buchung> { basis };
            }

            var felder = FeldWerte();
            var berechnung = BuchungsgruppenRechner.Berechne(_aktiveVorlage, basis, felder);
            return BuchungsgruppenRechner.BaueBuchungen(
                _aktiveVorlage, _aktiveVorlageSlot, basis, berechnung,
                _bestehendeGruppenUuid, felder);
        }

        // ------------------------------------------------------------------
        // Bearbeiten einer bestehenden Gruppe
        // ------------------------------------------------------------------

        /// <summary>
        /// Lädt beim BEARBEITEN die Vorlage, mit der die Buchung gebucht
        /// wurde, wieder in den Dialog -- samt dem, was damals eingegeben
        /// wurde. Die Basis-Buchung trägt dafür ihren Vorlagen-Bezug
        /// (Buchung.SetzeVorlagenBezug): Werte der Zusatz-Felder und, wo
        /// die Vorlage sie umformt, den eingegebenen Betrag, Beschreibungs-
        /// text und Beleg. Die Zusatz-Buchungen selbst werden nicht
        /// zurückgelesen, sondern beim Speichern neu berechnet.
        /// Aufrufer: ViewHost.ZeigeBuchungBearbeitenKombiDialog.
        /// </summary>
        /// <param name="zusatzBuchungen">Die bestehenden Zusatz-Buchungen
        /// der Gruppe -- nur noch für Buchungen aus der Zeit VOR dem
        /// gemerkten Eingabe-Betrag (Rekonstruktion als Summe).</param>
        /// <param name="gruppenUuid">UUID der bestehenden Gruppe; null bei
        /// einer Einzelbuchung mit Vorlagen-Bezug.</param>
        public void LadeVorlageFuerBearbeitung(Preset vorlage, int slot,
            IReadOnlyList<Buchung> zusatzBuchungen, string gruppenUuid)
        {
            _bestehendeGruppenUuid = gruppenUuid;
            _mitVorlageGeoeffnet = true;
            var e = vorlage.Erweiterung;

            // Die Eingaben wiederherstellen, BEVOR die Vorlage rechnet --
            // in den Feldern der Buchung steht ja das Ergebnis der Templates.
            string eingabeText = _originalBuchung?.VorlagenEingabeText;
            if (eingabeText != null && e.BasisTextTemplate.Length > 0)
            {
                _beschreibung = eingabeText;
                _geschuetzteBeschreibung = eingabeText;
                OnPropertyChanged(nameof(Beschreibung));
            }
            string eingabeBeleg = _originalBuchung?.VorlagenEingabeBeleg;
            if (eingabeBeleg != null && e.BasisBelegTemplate.Length > 0)
            {
                _belegnummer = eingabeBeleg;
                OnPropertyChanged(nameof(Belegnummer));
            }
            if (!string.IsNullOrWhiteSpace(vorlage.BasisBetragFormel))
            {
                decimal? eingabeBetrag = _originalBuchung?.VorlagenEingabeBetrag;
                if (eingabeBetrag.HasValue)
                    BetragText = Waehrungsformat.BetragOhneGruppierung(eingabeBetrag.Value);
                else if (zusatzBuchungen != null && zusatzBuchungen.Count > 0)
                {
                    // Altbestand ohne gemerkte Eingabe: Gesamtbetrag als
                    // Basis-Brutto plus Summe der Zusatz-Buchungen (exakt
                    // für "$rest", für andere Basis-Formeln eine Näherung)
                    decimal gesamt = (BetragInCent / 100m)
                        + zusatzBuchungen.Sum(b => b.BruttoBetrag.BruttoWert);
                    BetragText = Waehrungsformat.BetragOhneGruppierung(gesamt);
                }
            }

            UebernimmVorlage(vorlage, slot,
                _originalBuchung?.VorlagenFeldWerte()
                    ?? new Dictionary<string, string>());
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

        /// <summary>
        /// Hinweis (nur OK), wenn eine Umwandlung im Bearbeiten-Modus nicht
        /// zulaessig ist. Args: (Text, Titel). Ebenfalls vom
        /// BuchungView-Code-Behind gesetzt (Owner der MessageBox).
        /// </summary>
        public Action<string, string> UmwandlungHinweis { get; set; }

        // Gesetzt, waehrend eine ueber das Journal ausdruecklich angeforderte
        // Umwandlung laeuft -- dann entfaellt die Rueckfrage.
        private bool _umwandlungOhneRueckfrage;

        /// <summary>Slot der aktiven Vorlage (-1 = keine).</summary>
        public int GruppenVorlagenSlot => _aktiveVorlageSlot;

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

            // Um die GRUPPE geht es hier: eine aktive Vorlage ohne
            // Zusatz-Zeilen (nur Zusatz-Felder o.ä.) ist keine.
            bool hatGruppe = _aktiveVorlage != null && _aktiveVorlage.IstMehrzeilig;
            if (!hatGruppe && !p.IstMehrzeilig) return true;

            // Gruppen-Mitglied, dessen Gruppe der Dialog nicht laden konnte
            // (Vorlage geloescht/einzeilig geworden, Basis fehlt -- Einzel-
            // Fallback in ViewHost.ZeigeBuchungBearbeitenKombiDialog): auch
            // das ist eine Gruppe, die beim Speichern ersetzt wird.
            bool warGruppenMitglied = !hatGruppe
                && _originalBuchung?.GruppenUuid != null;

            // ... und zwar eine ZUSATZ-Buchung dieser Gruppe: die neue Gruppe
            // entstuende dann aus deren Betrag/Konto (z.B. der Vorsteuer-
            // Zeile), und beim Speichern fiele die alte Basis als "altes
            // Mitglied" weg. Das Journal-Kontextmenue "Umwandeln in" lenkt
            // in diesem Fall auf die Basis um -- hier im Dialog geht das
            // nicht mehr, also ablehnen und den Weg dorthin zeigen.
            if (warGruppenMitglied && p.IstMehrzeilig
                && _originalBuchung.GruppenRolle > 0)
            {
                UmwandlungHinweis?.Invoke(
                    "Diese Buchung ist eine Zusatz-Buchung einer Buchungsgruppe, "
                    + "deren Vorlage nicht mehr gefunden wurde. Sie kann hier "
                    + "nicht in eine andere Buchungsgruppe umgewandelt werden -- "
                    + "die neue Gruppe entstünde sonst aus dieser Zusatz-Buchung, "
                    + "und die eigentliche Basis-Buchung ginge verloren.\n\n"
                    + "Bitte stattdessen die Basis-Buchung der Gruppe bearbeiten "
                    + "oder im Journal über das Kontextmenü \"Umwandeln in\" "
                    + "umwandeln.",
                    "Buchungsgruppe");
                return false;
            }

            string text;
            if (!hatGruppe && !warGruppenMitglied)
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
                text = "Diese Buchungsgruppe in eine andere Buchungsgruppe umwandeln?\n\n"
                     + "Vorlage: \"" + p.Text + "\" (" + ZeilenAnzahlText(p) + ")\n\n"
                     + "Konto, MWSt. und AfA werden aus der Vorlage übernommen; "
                     + "Datum, Betrag und Belegnummer bleiben erhalten.\n"
                     + BeschreibungsHinweis(p) + "\n\n"
                     + "Die bisherigen Zusatz-Buchungen der Gruppe werden beim "
                     + "Speichern entfernt und durch " + ZeilenAnzahlText(p)
                     + " ersetzt.";
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

        /// <summary>
        /// Beschreibung, die eine Vorlagen-Wahl beim Bearbeiten (teilweise)
        /// erhaelt: beim Oeffnen der Text der Buchung, nach jeder
        /// angewendeten Vorlagen-Wahl deren Ergebnis (so ueberlebt der Zusatz
        /// "3Q/2026" auch einen zweiten Vorlagenwechsel). Null bei einer
        /// neuen Buchung -- dort wird der Text immer ersetzt.
        /// </summary>
        private string _geschuetzteBeschreibung;

        /// <summary>Sagt in der Rueckfrage an, wie die Beschreibung danach
        /// lautet (siehe BeschreibungNachVorlage).</summary>
        private string BeschreibungsHinweis(Preset p)
            => "Die Beschreibung lautet dann \""
               + BeschreibungNachVorlage(p) + "\".";

        /// <summary>
        /// Beschreibung nach der Wahl von <paramref name="p"/> im
        /// Bearbeiten-Modus: Vorlagenname am Textanfang austauschen, Rest
        /// (Zusatz wie "3Q/2026" oder eigener Inhalt) erhalten -- Regel siehe
        /// ECTEngine.VorlagenBeschreibung. Ein selbst getippter Text ist
        /// ausserdem die bessere Grundlage fuer die Zeilen-Templates der
        /// Vorlage ($B liefert dann "Hotel Berlin" statt des Vorlagen-Namens).
        /// </summary>
        /// <remarks>Geschuetzt ist NUR die beim Oeffnen geladene Beschreibung
        /// (bzw. das Ergebnis einer vorherigen Vorlagen-Wahl, siehe
        /// <see cref="_geschuetzteBeschreibung"/>). Alles, was danach im
        /// Dialog getippt wurde, ist Suchbegriff der Liste ("F" ->
        /// "Finanzamtsrueckzahlung") und wird komplett ersetzt.</remarks>
        private string BeschreibungNachVorlage(Preset p)
        {
            if (_geschuetzteBeschreibung == null
                || !string.Equals((Beschreibung ?? "").Trim(),
                                  _geschuetzteBeschreibung.Trim(),
                                  StringComparison.Ordinal))
                return (p.Text ?? "").Trim();

            return VorlagenBeschreibung.NachVorlagenwahl(
                Beschreibung, p.Text, p.IstMehrzeilig,
                Einstellungen.Presets.Where(v => !v.IstLeer).Select(v => v.Text));
        }
    }
}
