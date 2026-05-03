// JournalViewModel.cs - Hauptlogik des Buchungsjournals
//
// Reimplementiert DrawToDC_Datum() und DrawToDC_Konten() aus
// easycashview.cpp als ViewModel, das eine ObservableCollection
// von JournalRow-Items aufbaut.
//
// Der Aufrufer:
//   1) erzeugt das ViewModel mit Engine + Icon-Sprites + Listen
//   2) ruft Aktualisiere(filter) bei Filteraenderungen auf
//   3) abonniert die Events (BuchungBearbeiten, BuchungLoeschen, etc.)
//      fuer die User-Aktionen

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Globalization;
using System.Linq;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.Journal
{
    public class JournalViewModel : ViewModelBase
    {
        private static readonly CultureInfo DeDE = new CultureInfo("de-DE");

        private readonly BuchungsDocument _doc;
        /// <summary>Internal access to the underlying document for related ViewModels (e.g. Navigation).</summary>
        public BuchungsDocument Doc => _doc;

        // Icon-Daten
        private readonly IList<string> _betriebeNamen;
        private readonly IList<string> _betriebeIcons;
        private readonly IList<string> _bestandskontenNamen;
        private readonly IList<string> _bestandskontenIcons;
        private readonly BitmapSource _spriteBetriebe;
        private readonly BitmapSource _spriteBestandskonten;

        // Cache fuer ausgeschnittene Icons
        private readonly Dictionary<string, BitmapSource> _iconCache =
            new Dictionary<string, BitmapSource>();

        public ObservableCollection<JournalRow> Zeilen { get; } =
            new ObservableCollection<JournalRow>();

        private JournalFilter _aktuellerFilter = new JournalFilter();
        public JournalFilter AktuellerFilter
        {
            get => _aktuellerFilter;
            private set => SetProperty(ref _aktuellerFilter, value);
        }

        // Zoom-Property
        public double Schriftgroesse
        {
            get => _aktuellerFilter.Schriftgroesse;
            set
            {
                if (Math.Abs(_aktuellerFilter.Schriftgroesse - value) > 0.01)
                {
                    _aktuellerFilter.Schriftgroesse = value;
                    OnPropertyChanged();
                }
            }
        }

        // Maximale Breite der Belegspalte (1/4 der ListBox-Breite). Wird
        // vom JournalView-Code-Behind in OnSizeChanged aktualisiert.
        // Die XAML-ColumnDefinitions binden ihr MaxWidth an diese Property,
        // sodass die Belegspalte sich an den Inhalt anpasst, aber nie
        // breiter als 1/4 der Gesamtbreite wird.
        private double _belegMaxBreite = 200;
        public double BelegMaxBreite
        {
            get => _belegMaxBreite;
            set => SetProperty(ref _belegMaxBreite, value);
        }

        // Breite der Saldo-Spalte. 0 in den meisten Modi (Spalte ist
        // dann praktisch nicht da), 110 im Bestandskonten-Modus.
        // Gebunden an MinWidth + MaxWidth der ColumnDefinition in
        // allen drei Templates (Header/Buchung/Footer).
        private double _saldoSpaltenBreite = 0;
        public double SaldoSpaltenBreite
        {
            get => _saldoSpaltenBreite;
            set => SetProperty(ref _saldoSpaltenBreite, value);
        }

        // Selektion
        private JournalBuchungRow _selektierteZeile;
        public JournalBuchungRow SelektierteZeile
        {
            get => _selektierteZeile;
            set => SetProperty(ref _selektierteZeile, value);
        }

        // Event, das der View abonniert. Liefert die Zeile, die in den
        // sichtbaren Bereich gescrollt werden soll. Wird von den
        // ScrolleZu*-Methoden ausgeloest.
        public event Action<JournalRow> ScrollIntoViewRequest;

        /// <summary>
        /// Sucht die erste Buchungs-Zeile mit passendem Monat in der
        /// gewuenschten Buchungsart und scrollt sie in den sichtbaren
        /// Bereich. Wenn kein exakter Monatstreffer existiert, wird zur
        /// zeitlich naechsten Buchung gescrollt (analog OnNMClick im
        /// Original-CNavigation, Zeile 109 ff.).
        /// </summary>
        public void ScrolleZuMonat(int monat, bool istEinnahme)
        {
            JournalBuchungRow exakter = null;
            JournalBuchungRow naehester = null;
            int besteDistanz = 13;

            foreach (var z in Zeilen.OfType<JournalBuchungRow>())
            {
                if (z.Buchung == null) continue;
                bool zIstEinnahme = z.Buchung.Art == Buchungsart.Einnahme;
                if (zIstEinnahme != istEinnahme) continue;

                int zMonat = z.Buchung.Datum.Month;
                if (zMonat == monat)
                {
                    exakter = z;
                    break;
                }
                int dist = Math.Abs(zMonat - monat);
                if (dist < besteDistanz)
                {
                    besteDistanz = dist;
                    naehester = z;
                }
            }

            var ziel = exakter ?? naehester;
            if (ziel != null)
            {
                SelektierteZeile = ziel;
                ScrollIntoViewRequest?.Invoke(ziel);
            }
        }

        /// <summary>
        /// Scrollt zur ersten Buchung mit passendem Konto in der
        /// gewuenschten Buchungsart. Leerer kontoName = "unzugewiesen".
        /// </summary>
        public void ScrolleZuKonto(string kontoName, bool istEinnahme)
        {
            foreach (var z in Zeilen.OfType<JournalBuchungRow>())
            {
                if (z.Buchung == null) continue;
                bool zIstEinnahme = z.Buchung.Art == Buchungsart.Einnahme;
                if (zIstEinnahme != istEinnahme) continue;
                if ((z.Buchung.Konto ?? "") != (kontoName ?? "")) continue;

                SelektierteZeile = z;
                ScrollIntoViewRequest?.Invoke(z);
                return;
            }
        }

        /// <summary>
        /// Scrollt zur ersten Buchung im angegebenen Bestandskonto und
        /// Monat. Wenn der Monat nicht existiert, wird zur ersten Buchung
        /// im Bestandskonto gescrollt.
        /// </summary>
        public void ScrolleZuBestandskontoMonat(string bestandskonto, int monat)
        {
            JournalBuchungRow exakter = null;
            JournalBuchungRow ersterImKonto = null;

            foreach (var z in Zeilen.OfType<JournalBuchungRow>())
            {
                if (z.Buchung == null) continue;
                if ((z.Buchung.Bestandskonto ?? "") != (bestandskonto ?? ""))
                    continue;

                if (ersterImKonto == null) ersterImKonto = z;
                if (z.Buchung.Datum.Month == monat)
                {
                    exakter = z;
                    break;
                }
            }

            var ziel = exakter ?? ersterImKonto;
            if (ziel != null)
            {
                SelektierteZeile = ziel;
                ScrollIntoViewRequest?.Invoke(ziel);
            }
        }

        // Commands
        public ICommand BearbeitenCommand { get; }
        public ICommand LoeschenCommand { get; }
        public ICommand KopierenCommand { get; }
        public ICommand KopierenMitNeuerBelegnummerCommand { get; }
        public ICommand AfaAbgangCommand { get; }

        // Events
        public event Action<Buchung> BuchungBearbeiten;
        public event Action<Buchung> BuchungLoeschen;
        public event Action<Buchung> BuchungKopieren;
        public event Action<Buchung> BuchungKopierenMitNeuerBelegnummer;
        public event Action<Buchung> BuchungAfaAbgang;

        public JournalViewModel(
            BuchungsDocument doc,
            IList<string> betriebeNamen = null,
            IList<string> betriebeIcons = null,
            IList<string> bestandskontenNamen = null,
            IList<string> bestandskontenIcons = null,
            BitmapSource spriteBetriebe = null,
            BitmapSource spriteBestandskonten = null)
        {
            _doc = doc ?? throw new ArgumentNullException(nameof(doc));
            _betriebeNamen = betriebeNamen ?? new List<string>();
            _betriebeIcons = betriebeIcons ?? new List<string>();
            _bestandskontenNamen = bestandskontenNamen ?? new List<string>();
            _bestandskontenIcons = bestandskontenIcons ?? new List<string>();
            _spriteBetriebe = spriteBetriebe;
            _spriteBestandskonten = spriteBestandskonten;

            BearbeitenCommand = new RelayCommand(
                () => BuchungBearbeiten?.Invoke(SelektierteZeile?.Buchung),
                () => SelektierteZeile != null);
            LoeschenCommand = new RelayCommand(
                () => BuchungLoeschen?.Invoke(SelektierteZeile?.Buchung),
                () => SelektierteZeile != null);
            KopierenCommand = new RelayCommand(
                () => BuchungKopieren?.Invoke(SelektierteZeile?.Buchung),
                () => SelektierteZeile != null);
            KopierenMitNeuerBelegnummerCommand = new RelayCommand(
                () => BuchungKopierenMitNeuerBelegnummer?.Invoke(SelektierteZeile?.Buchung),
                () => SelektierteZeile != null);
            AfaAbgangCommand = new RelayCommand(
                () => BuchungAfaAbgang?.Invoke(SelektierteZeile?.Buchung),
                () => SelektierteZeile != null);
        }

        /// <summary>
        /// Baut die Zeilenliste neu auf Basis des aktuellen Filters auf.
        /// </summary>
        public void Aktualisiere(JournalFilter filter = null)
        {
            if (filter != null) AktuellerFilter = filter;

            var alteBuchung = SelektierteZeile?.Buchung;
            Zeilen.Clear();

            // Saldo-Spalte nur im Bestandskonten-Modus sichtbar machen.
            // In den anderen Modi auf 0 - die Spalte ist dann ein
            // Null-Pixel-Strich und stoert das Layout nicht.
            SaldoSpaltenBreite =
                AktuellerFilter.AnzeigeModus == JournalAnzeigeModus.Bestandskonten
                ? 110.0 : 0.0;

            switch (AktuellerFilter.AnzeigeModus)
            {
                case JournalAnzeigeModus.Datum:
                    BaueAnzeigeNachDatum();
                    break;
                case JournalAnzeigeModus.Konten:
                    BaueAnzeigeNachKonten();
                    break;
                case JournalAnzeigeModus.Bestandskonten:
                    BaueAnzeigeBestandskonten();
                    break;
                case JournalAnzeigeModus.Anlagenverzeichnis:
                    BaueAnzeigeAnlagenverzeichnis();
                    break;
            }

            // Selektion wiederherstellen
            if (alteBuchung != null)
            {
                SelektierteZeile = Zeilen.OfType<JournalBuchungRow>()
                    .FirstOrDefault(r => ReferenceEquals(r.Buchung, alteBuchung));
            }
        }

        // Modus 1: Anzeige nach Datum
        private void BaueAnzeigeNachDatum()
        {
            var f = AktuellerFilter;

            bool zeigeEinnahmen =
                !f.KontenFilter.StartsWith("Ausgaben: ") &&
                f.KontenFilter != "--- [noch zu keinem Konto zugewiesene Ausgaben] ---";

            if (zeigeEinnahmen)
            {
                var einnahmen = FilterBuchungen(_doc.Einnahmen, true).ToList();
                if (einnahmen.Count > 0 || !f.IstKontenFilterAktiv)
                {
                    Zeilen.Add(new JournalSectionTitle
                    {
                        Text = TitelEinnahmenAusgaben(true),
                        IsMain = true,
                        IsEinnahme = true
                    });

                    Zeilen.Add(new JournalHeaderRow
                    {
                        IsAusgabe = false,
                        ZeigeBelegnummer = f.ZeigeBelegnummernspalte,
                        ZeigeSteuer = f.ZeigeSteuerspalte,
                        ZeigeAfaNr = false
                    });

                    long brutto = 0, netto = 0, ust = 0;
                    int idx = 0;
                    foreach (var b in einnahmen)
                    {
                        Zeilen.Add(BaueBuchungZeile(b, false, idx++));

                        long bNetto = b.BruttoBetrag.NettoInCent;
                        long bMwst = b.BruttoBetrag.InCent - bNetto;
                        netto += bNetto;
                        ust += bMwst;
                        brutto += b.BruttoBetrag.InCent;
                    }

                    if (einnahmen.Count > 0)
                    {
                        Zeilen.Add(BaueFooter(false, netto, ust, brutto));
                    }

                    Zeilen.Add(new JournalSpacerRow());
                }
            }

            bool zeigeAusgaben =
                !f.KontenFilter.StartsWith("Einnahmen: ") &&
                f.KontenFilter != "--- [noch zu keinem Konto zugewiesene Einnahmen] ---";

            if (zeigeAusgaben)
            {
                var ausgaben = FilterBuchungen(_doc.Ausgaben, false).ToList();
                if (ausgaben.Count > 0 || !f.IstKontenFilterAktiv)
                {
                    Zeilen.Add(new JournalSectionTitle
                    {
                        Text = TitelEinnahmenAusgaben(false),
                        IsMain = true,
                        IsEinnahme = false
                    });

                    Zeilen.Add(new JournalHeaderRow
                    {
                        IsAusgabe = true,
                        ZeigeBelegnummer = f.ZeigeBelegnummernspalte,
                        ZeigeSteuer = f.ZeigeSteuerspalte,
                        ZeigeAfaNr = true
                    });

                    long brutto = 0, netto = 0, vst = 0;
                    int idx = 0;
                    foreach (var b in ausgaben)
                    {
                        Zeilen.Add(BaueBuchungZeile(b, true, idx++));

                        if (b.Konto == "VST-Beträge separat")
                        {
                            vst += b.BruttoBetrag.InCent;
                            brutto += b.BruttoBetrag.InCent;
                        }
                        else
                        {
                            long jahresnetto = AfaCalculator.GetBuchungsjahrNetto(b);
                            long mwstAnteil = (b.AfaNr == 1)
                                ? b.BruttoBetrag.InCent - b.BruttoBetrag.NettoInCent
                                : 0;
                            netto += jahresnetto;
                            vst += mwstAnteil;
                            brutto += jahresnetto + mwstAnteil;
                        }
                    }

                    if (ausgaben.Count > 0)
                    {
                        Zeilen.Add(BaueFooter(true, netto, vst, brutto));
                    }
                }
            }
        }

        // Modus 2: Anzeige nach Konten
        private void BaueAnzeigeNachKonten()
        {
            var f = AktuellerFilter;

            var einnahmenKonten = SammleKonten(_doc.Einnahmen,
                _doc.EinnahmenKonten, true);
            if (einnahmenKonten.Count > 0)
            {
                Zeilen.Add(new JournalSectionTitle
                {
                    Text = "EINNAHMEN",
                    IsMain = true,
                    IsEinnahme = true
                });

                foreach (var konto in einnahmenKonten)
                {
                    var buchungen = FilterBuchungen(_doc.Einnahmen, true)
                        .Where(b => (b.Konto ?? "") == konto)
                        .ToList();
                    if (buchungen.Count == 0) continue;

                    Zeilen.Add(new JournalSectionTitle
                    {
                        Text = string.IsNullOrEmpty(konto)
                            ? "[noch zu keinem Konto zugewiesene Einnahmen]"
                            : "[" + konto + "]",
                        IsMain = false,
                        IsEinnahme = true
                    });

                    Zeilen.Add(new JournalHeaderRow
                    {
                        IsAusgabe = false,
                        ZeigeBelegnummer = f.ZeigeBelegnummernspalte,
                        ZeigeSteuer = f.ZeigeSteuerspalte
                    });

                    long brutto = 0, netto = 0, ust = 0;
                    int idx = 0;
                    foreach (var b in buchungen)
                    {
                        Zeilen.Add(BaueBuchungZeile(b, false, idx++));
                        long bNetto = b.BruttoBetrag.NettoInCent;
                        netto += bNetto;
                        ust += b.BruttoBetrag.InCent - bNetto;
                        brutto += b.BruttoBetrag.InCent;
                    }
                    Zeilen.Add(BaueFooter(false, netto, ust, brutto));
                    Zeilen.Add(new JournalSpacerRow());
                }
            }

            var ausgabenKonten = SammleKonten(_doc.Ausgaben,
                _doc.AusgabenKonten, false);
            if (ausgabenKonten.Count > 0)
            {
                Zeilen.Add(new JournalSectionTitle
                {
                    Text = "AUSGABEN",
                    IsMain = true,
                    IsEinnahme = false
                });

                foreach (var konto in ausgabenKonten)
                {
                    var buchungen = FilterBuchungen(_doc.Ausgaben, false)
                        .Where(b => (b.Konto ?? "") == konto)
                        .ToList();
                    if (buchungen.Count == 0) continue;

                    Zeilen.Add(new JournalSectionTitle
                    {
                        Text = string.IsNullOrEmpty(konto)
                            ? "[noch zu keinem Konto zugewiesene Ausgaben]"
                            : "[" + konto + "]",
                        IsMain = false,
                        IsEinnahme = false
                    });

                    Zeilen.Add(new JournalHeaderRow
                    {
                        IsAusgabe = true,
                        ZeigeBelegnummer = f.ZeigeBelegnummernspalte,
                        ZeigeSteuer = f.ZeigeSteuerspalte,
                        ZeigeAfaNr = true
                    });

                    long brutto = 0, netto = 0, vst = 0;
                    int idx = 0;
                    foreach (var b in buchungen)
                    {
                        Zeilen.Add(BaueBuchungZeile(b, true, idx++));
                        if (b.Konto == "VST-Beträge separat")
                        {
                            vst += b.BruttoBetrag.InCent;
                            brutto += b.BruttoBetrag.InCent;
                        }
                        else
                        {
                            long jahresnetto = AfaCalculator.GetBuchungsjahrNetto(b);
                            long mwstAnteil = (b.AfaNr == 1)
                                ? b.BruttoBetrag.InCent - b.BruttoBetrag.NettoInCent
                                : 0;
                            netto += jahresnetto;
                            vst += mwstAnteil;
                            brutto += jahresnetto + mwstAnteil;
                        }
                    }
                    Zeilen.Add(BaueFooter(true, netto, vst, brutto));
                    Zeilen.Add(new JournalSpacerRow());
                }
            }
        }

        // Modus 3: Bestandskonten
        // Pro Bestandskonto eine Tabelle mit Anfangssaldo, Buchungen
        // (Einnahmen + Ausgaben gemischt), Endsaldo.
        // Reimplementiert DrawToDC_Bestandskonten aus easycashview.cpp.
        private void BaueAnzeigeBestandskonten()
        {
            var f = AktuellerFilter;

            // Liste aller Bestandskonten, fuer die es Buchungen gibt
            var bestandskonten = SammleBestandskonten();
            if (bestandskonten.Count == 0) return;

            Zeilen.Add(new JournalSectionTitle
            {
                Text = "BESTANDSKONTEN",
                IsMain = true,
                IsEinnahme = null
            });

            foreach (var bk in bestandskonten)
            {
                // Anfangssaldo aus den Doc-Properties holen, falls vorhanden.
                // Fallback 0, wenn die Engine den Saldo nicht kennt.
                long anfangssaldoCent = HoleAnfangssaldoCent(bk);

                // Buchungen fuer dieses Bestandskonto sammeln.
                // Filter dabei beachten (Monat, Betrieb, Konto), aber NICHT
                // BestandskontoFilter - der wird hier durch das Konto selbst
                // ueberschrieben.
                var alleBuchungen = new List<Buchung>();
                alleBuchungen.AddRange(FilterBuchungenOhneBestandskonto(_doc.Einnahmen, true)
                    .Where(b => (b.Bestandskonto ?? "") == bk));
                alleBuchungen.AddRange(FilterBuchungenOhneBestandskonto(_doc.Ausgaben, false)
                    .Where(b => (b.Bestandskonto ?? "") == bk));

                // Buchungen chronologisch sortieren
                alleBuchungen = alleBuchungen.OrderBy(b => b.Datum).ToList();

                // Wenn kein Anfangssaldo und keine Buchungen - Konto ueberspringen
                if (anfangssaldoCent == 0 && alleBuchungen.Count == 0) continue;

                // Filter: einzelnes Bestandskonto
                if (!string.IsNullOrEmpty(f.BestandskontoFilter)
                    && bk != f.BestandskontoFilter)
                    continue;

                Zeilen.Add(new JournalSectionTitle
                {
                    Text = string.IsNullOrEmpty(bk)
                        ? "[Bestandskonto: leer]"
                        : "[" + bk + "]",
                    IsMain = false,
                    IsEinnahme = null
                });

                // "Anfangssaldo"-Pseudozeile als Header-Ersatz.
                // Im Bestandskonten-Modus werden die Steuer-Spalten (USt,
                // USt-Betr) nicht angezeigt. Stattdessen ist die Saldo-Spalte aktiv.
                Zeilen.Add(new JournalHeaderRow
                {
                    IsAusgabe = false,
                    ZeigeBelegnummer = f.ZeigeBelegnummernspalte,
                    ZeigeSteuer = false,
                    ZeigeSaldo = true
                });

                // Lauf-Saldo
                long saldoCent = anfangssaldoCent;
                int idx = 0;

                // Pseudozeile fuer Anfangssaldo - als JournalBuchungRow
                // ohne Buchung-Referenz (keine Bearbeiten-Aktion).
                // Brutto bleibt leer, Saldo enthaelt den Anfangswert.
                Zeilen.Add(new JournalBuchungRow
                {
                    Buchung = null,
                    IstAusgabe = false,
                    ZebraIndex = idx++,
                    DatumText = $"01.01.{_doc.Jahr}",
                    BelegText = "",
                    BeschreibungText = "Anfangssaldo",
                    NettoText = "",
                    MwstSatzText = "",
                    MwstBetragText = "",
                    BruttoText = "",
                    SaldoText = FormatBetrag(saldoCent),
                    AfaNrText = ""
                });

                long bruttoSumme = 0;
                foreach (var b in alleBuchungen)
                {
                    bool istEinnahme = b.Art == Buchungsart.Einnahme;
                    long bruttoCent = istEinnahme
                        ? b.BruttoBetrag.InCent
                        : -b.BruttoBetrag.InCent;
                    saldoCent += bruttoCent;
                    bruttoSumme += bruttoCent;

                    var zeile = BaueBuchungZeile(b, !istEinnahme, idx++);
                    // Vorzeichen sichtbar machen
                    if (!istEinnahme)
                        zeile.BruttoText = "-" + zeile.BruttoText;
                    // Im Bestandskonten-Modus die Steuer-Spalten leeren -
                    // Saldo ist eine reine Brutto-Groesse.
                    zeile.NettoText = "";
                    zeile.MwstSatzText = "";
                    zeile.MwstBetragText = "";
                    // AfA-Nr auch nicht zeigen (die Spalte bleibt fuer
                    // die Waehrung im Footer reserviert).
                    zeile.AfaNrText = "";
                    // Laufender Saldo
                    zeile.SaldoText = FormatBetrag(saldoCent);
                    Zeilen.Add(zeile);
                }

                // Footer: Endsaldo. Brutto-Summe leer, Saldo gefuellt.
                Zeilen.Add(new JournalFooterRow
                {
                    IsAusgabe = false,
                    ZeigeSteuer = false,
                    LinkesLabel = "Endsaldo",
                    NettoSummeText = "",
                    SteuerSummeText = "",
                    BruttoSummeText = "",
                    SaldoSummeText = FormatBetrag(saldoCent),
                    Waehrung = _doc.Waehrung ?? "EUR"
                });
                Zeilen.Add(new JournalSpacerRow());
            }
        }

        // Modus 4: Anlagenverzeichnis
        // Pro AfA-Konto eine Tabelle mit den Anlageguetern + AfA-Status.
        // Im Original-MFC zeigt es Buchungen mit AfaJahre>1 an.
        private void BaueAnzeigeAnlagenverzeichnis()
        {
            var f = AktuellerFilter;

            // Nur Ausgaben mit AfA (Jahre > 1)
            var anlageBuchungen = FilterBuchungen(_doc.Ausgaben, false)
                .Where(b => b.AfaJahre > 1)
                .ToList();
            if (anlageBuchungen.Count == 0) return;

            Zeilen.Add(new JournalSectionTitle
            {
                Text = "ANLAGENVERZEICHNIS",
                IsMain = true,
                IsEinnahme = false
            });

            // Gruppiere nach Konto
            var byKonto = anlageBuchungen
                .GroupBy(b => b.Konto ?? "")
                .OrderBy(g => g.Key);

            foreach (var grp in byKonto)
            {
                Zeilen.Add(new JournalSectionTitle
                {
                    Text = string.IsNullOrEmpty(grp.Key)
                        ? "[noch zu keinem Konto zugewiesene Anlagegueter]"
                        : "[" + grp.Key + "]",
                    IsMain = false,
                    IsEinnahme = false
                });

                Zeilen.Add(new JournalHeaderRow
                {
                    IsAusgabe = true,
                    ZeigeBelegnummer = f.ZeigeBelegnummernspalte,
                    ZeigeSteuer = f.ZeigeSteuerspalte,
                    ZeigeAfaNr = true
                });

                long bruttoSumme = 0, nettoSumme = 0, vstSumme = 0;
                int idx = 0;
                foreach (var b in grp.OrderBy(b => b.Datum))
                {
                    Zeilen.Add(BaueBuchungZeile(b, true, idx++));

                    // Anschaffungsbrutto in der Summe (nicht AfA-Jahresanteil)
                    long bruttoCent = b.BruttoBetrag.InCent;
                    long nettoCent = b.BruttoBetrag.NettoInCent;
                    long vstCent = bruttoCent - nettoCent;
                    bruttoSumme += bruttoCent;
                    nettoSumme += nettoCent;
                    vstSumme += vstCent;
                }

                Zeilen.Add(BaueFooter(true, nettoSumme, vstSumme, bruttoSumme));
                Zeilen.Add(new JournalSpacerRow());
            }
        }

        // Hilfsmethode: Bestandskonten sammeln, die in Buchungen vorkommen
        // ODER einen Anfangssaldo haben.
        private List<string> SammleBestandskonten()
        {
            var ergebnis = new List<string>();
            var seen = new HashSet<string>();

            // Erst die in den Einstellungen definierten Bestandskonten
            // (Reihenfolge wie im Original)
            var konfigKonten = HoleKonfigurierteBestandskonten();
            foreach (var k in konfigKonten)
            {
                if (string.IsNullOrEmpty(k)) continue;
                if (seen.Add(k)) ergebnis.Add(k);
            }

            // Dann zusaetzliche aus Buchungen
            foreach (var b in _doc.Einnahmen.Concat(_doc.Ausgaben))
            {
                var bk = b.Bestandskonto ?? "";
                if (!string.IsNullOrEmpty(bk) && seen.Add(bk))
                    ergebnis.Add(bk);
            }

            return ergebnis;
        }

        // Stub: liefert die in den Einstellungen definierten Bestandskonten.
        // Wird beim Erzeugen des ViewModels mitgegeben (siehe Konstruktor).
        // Falls leer, werden die Bestandskonten allein aus den Buchungen
        // ermittelt.
        private IList<string> HoleKonfigurierteBestandskonten()
        {
            return _bestandskontenNamen ?? (IList<string>)new List<string>();
        }

        // Stub: liefert den konfigurierten Anfangssaldo eines Bestandskontos
        // in Cent. Aktuell 0, weil die Engine keine Saldo-Property hat -
        // der Saldo aus der MFC-Welt steht in m_csaBestandskontenSalden,
        // muesste also ueber eine zusaetzliche Setze-API ans ECTViews-
        // Modul uebergeben werden. TODO.
        private long HoleAnfangssaldoCent(string bestandskonto)
        {
            // TODO: Engine-Property oder Setze-API fuer Anfangssalden
            return 0;
        }

        // Wie FilterBuchungen, aber ohne Bestandskonto-Filter (der wird
        // beim Bestandskonten-Modus durch das Konto selbst gesetzt).
        private IEnumerable<Buchung> FilterBuchungenOhneBestandskonto(
            IEnumerable<Buchung> input, bool istEinnahme)
        {
            var f = AktuellerFilter;

            foreach (var b in input)
            {
                // Konten-Filter
                if (f.KontenFilter != "" && f.KontenFilter != "<alle Konten>")
                {
                    string praefix = istEinnahme ? "Einnahmen: " : "Ausgaben: ";
                    string nichtZugewiesenLabel = istEinnahme
                        ? "--- [noch zu keinem Konto zugewiesene Einnahmen] ---"
                        : "--- [noch zu keinem Konto zugewiesene Ausgaben] ---";

                    bool kontoLeer = string.IsNullOrEmpty(b.Konto);
                    if (kontoLeer)
                    {
                        if (f.KontenFilter != nichtZugewiesenLabel) continue;
                    }
                    else
                    {
                        if (f.KontenFilter == nichtZugewiesenLabel) continue;
                        if (praefix + b.Konto != f.KontenFilter) continue;
                    }
                }

                // Monats-Filter
                if (f.MonatsFilter > 0)
                {
                    int monat = b.Datum.Month;
                    if (f.MonatsFilter <= 12)
                    {
                        if (monat != f.MonatsFilter) continue;
                    }
                    else
                    {
                        int quartal = (monat - 1) / 3 + 1;
                        if (quartal != f.MonatsFilter - 12) continue;
                    }
                }

                // Betriebs-Filter
                if (!string.IsNullOrEmpty(f.BetriebFilter)
                    && b.Betrieb != f.BetriebFilter)
                    continue;

                // KEIN Bestandskonto-Filter hier!

                yield return b;
            }
        }

        // Filter-Logik
        private IEnumerable<Buchung> FilterBuchungen(
            IEnumerable<Buchung> input, bool istEinnahme)
        {
            var f = AktuellerFilter;

            foreach (var b in input)
            {
                // Konten-Filter
                if (f.KontenFilter != "" && f.KontenFilter != "<alle Konten>")
                {
                    string praefix = istEinnahme ? "Einnahmen: " : "Ausgaben: ";
                    string nichtZugewiesenLabel = istEinnahme
                        ? "--- [noch zu keinem Konto zugewiesene Einnahmen] ---"
                        : "--- [noch zu keinem Konto zugewiesene Ausgaben] ---";

                    bool kontoLeer = string.IsNullOrEmpty(b.Konto);

                    if (kontoLeer)
                    {
                        if (f.KontenFilter != nichtZugewiesenLabel)
                            continue;
                    }
                    else
                    {
                        if (f.KontenFilter == nichtZugewiesenLabel)
                            continue;
                        if (praefix + b.Konto != f.KontenFilter)
                            continue;
                    }
                }

                // Monats-Filter
                if (f.MonatsFilter > 0)
                {
                    int monat = b.Datum.Month;
                    if (f.MonatsFilter <= 12)
                    {
                        if (monat != f.MonatsFilter) continue;
                    }
                    else
                    {
                        int quartal = (monat - 1) / 3 + 1;
                        if (quartal != f.MonatsFilter - 12) continue;
                    }
                }

                // Betriebs-Filter
                if (!string.IsNullOrEmpty(f.BetriebFilter)
                    && b.Betrieb != f.BetriebFilter)
                    continue;

                // Bestandskonto-Filter
                if (!string.IsNullOrEmpty(f.BestandskontoFilter)
                    && b.Bestandskonto != f.BestandskontoFilter)
                    continue;

                yield return b;
            }
        }

        private string TitelEinnahmenAusgaben(bool einnahme)
        {
            string typ = einnahme ? "EINNAHMEN" : "AUSGABEN";
            int jahr = _doc.Jahr;
            var f = AktuellerFilter;

            if (f.MonatsFilter > 0)
            {
                if (f.MonatsFilter > 12)
                    return $"{typ} fuer {f.MonatsFilter - 12}. Quartal {jahr}";
                return $"{typ} fuer Monat {f.MonatsFilter:D2}/{jahr}";
            }
            return $"{typ} fuer {jahr}";
        }

        private List<string> SammleKonten(IEnumerable<Buchung> buchungen,
            string[] vorgegebeneKonten, bool istEinnahme)
        {
            var alle = new List<string>();
            var seen = new HashSet<string>();

            if (vorgegebeneKonten != null)
            {
                foreach (var k in vorgegebeneKonten)
                {
                    if (string.IsNullOrEmpty(k)) continue;
                    if (seen.Add(k)) alle.Add(k);
                }
            }
            bool unzugewiesenVorhanden = false;
            foreach (var b in buchungen)
            {
                if (string.IsNullOrEmpty(b.Konto))
                    unzugewiesenVorhanden = true;
                else if (seen.Add(b.Konto))
                    alle.Add(b.Konto);
            }
            if (unzugewiesenVorhanden) alle.Add("");
            return alle;
        }

        private JournalBuchungRow BaueBuchungZeile(Buchung b, bool istAusgabe, int zebraIdx)
        {
            long bruttoCent = b.BruttoBetrag.InCent;
            long nettoCent;
            long mwstBetragCent;
            long anzeigeBruttoCent;

            if (istAusgabe)
            {
                if (b.Konto == "VST-Beträge separat")
                {
                    nettoCent = 0;
                    mwstBetragCent = bruttoCent;
                    anzeigeBruttoCent = bruttoCent;
                }
                else
                {
                    nettoCent = AfaCalculator.GetBuchungsjahrNetto(b);
                    mwstBetragCent = (b.AfaNr == 1)
                        ? bruttoCent - b.BruttoBetrag.NettoInCent
                        : 0;
                    anzeigeBruttoCent = nettoCent + mwstBetragCent;
                }
            }
            else
            {
                nettoCent = b.BruttoBetrag.NettoInCent;
                mwstBetragCent = bruttoCent - nettoCent;
                anzeigeBruttoCent = bruttoCent;
            }

            return new JournalBuchungRow
            {
                Buchung = b,
                IstAusgabe = istAusgabe,
                ZebraIndex = zebraIdx,
                DatumText = b.Datum.ToString("dd.MM.yyyy", DeDE),
                BelegText = b.Belegnummer ?? "",
                BeschreibungText = b.Beschreibung ?? "",
                NettoText = FormatBetrag(nettoCent),
                MwstSatzText = FormatMwstSatz(b.BruttoBetrag.MwstPromille),
                MwstBetragText = mwstBetragCent != 0
                    ? FormatBetrag(mwstBetragCent) : "",
                BruttoText = FormatBetrag(anzeigeBruttoCent),
                AfaNrText = (istAusgabe && b.AfaJahre > 1)
                    ? $"{b.AfaNr}/{b.AfaJahre}" : "",
                BetriebIcon = HoleIcon(_betriebeNamen, _betriebeIcons,
                    _spriteBetriebe, b.Betrieb),
                BestandskontoIcon = HoleIcon(_bestandskontenNamen, _bestandskontenIcons,
                    _spriteBestandskonten, b.Bestandskonto)
            };
        }

        private JournalFooterRow BaueFooter(
            bool isAusgabe, long netto, long steuer, long brutto)
        {
            return new JournalFooterRow
            {
                IsAusgabe = isAusgabe,
                ZeigeSteuer = AktuellerFilter.ZeigeSteuerspalte,
                NettoSummeText = FormatBetrag(netto),
                SteuerSummeText = FormatBetrag(steuer),
                BruttoSummeText = FormatBetrag(brutto),
                Waehrung = _doc.Waehrung ?? "EUR"
            };
        }

        private string FormatBetrag(long cent)
            => (cent / 100m).ToString("N2", DeDE);

        private string FormatMwstSatz(int promille)
        {
            if (promille == 0) return "";
            decimal prozent = promille / 1000m;
            return prozent.ToString("0.##", DeDE) + "%";
        }

        private BitmapSource HoleIcon(IList<string> namen, IList<string> indizes,
            BitmapSource sprite, string name)
        {
            if (sprite == null || string.IsNullOrEmpty(name)) return null;
            int pos = -1;
            for (int i = 0; i < namen.Count; i++)
                if (namen[i] == name) { pos = i; break; }
            if (pos < 0 || pos >= indizes.Count) return null;
            if (!int.TryParse(indizes[pos], out int idx)) return null;

            string cacheKey = $"{(sprite == _spriteBetriebe ? "B" : "K")}:{idx}";
            if (_iconCache.TryGetValue(cacheKey, out var cached))
                return cached;

            var icon = IconSpriteSplitter.Crop(sprite, idx) as BitmapSource;
            if (icon != null) _iconCache[cacheKey] = icon;
            return icon;
        }
    }
}
