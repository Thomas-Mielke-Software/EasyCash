// JournalViewModel.cs - Hauptlogik des Buchungsjournals
//
// Reimplementiert DrawToDC_Datum() und DrawToDC_Konten() aus
// easycashview.cpp als ViewModel, das eine ObservableCollection
// von JournalRow-Items aufbaut.
//
// Der Aufrufer:
//   1) erzeugt das ViewModel mit Engine + Icon-Sprites + Listen
//   2) ruft Aktualisiere(filter) bei Filteränderungen auf
//   3) abonniert die Events (BuchungBearbeiten, BuchungLoeschen, etc.)
//      für die User-Aktionen

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

        // Icon-Daten kommen NICHT mehr aus gecachten Snapshots, sondern werden
        // bei jedem Aktualisiere() direkt aus den statischen ViewHost-Listen
        // gelesen. Sonst entstände der Bug "Icons fehlen, bis man einmal den
        // Modus umschaltet": Wenn das ViewModel konstruiert wird, bevor
        // ECT_SetzeBetriebeUndBestandskonten das ViewHost befüllt hat, blieben
        // die Snapshot-Felder für immer leer.

        // Cache für ausgeschnittene Icons
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
            set
            {
                if (SetProperty(ref _belegMaxBreite, value))
                {
                    // Belegspalte ggf. an neue Obergrenze anpassen
                    BelegSpaltenBreite = BerechneBelegSpaltenBreite();
                }
            }
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

        // Konstante Breite der Beleg-Spalte für das gesamte Journal --
        // nicht pro Konten-/Bestandskonto-Abschnitt neu berechnet. Wird in
        // Aktualisiere() einmalig aus dem 95-Perzentil aller Belegnummer-
        // Längen ermittelt; längere Belegnummern werden im TextBlock per
        // TextTrimming="CharacterEllipsis" abgekürzt. Vorher war die
        // Spalte ein Auto + SharedSizeGroup, was beim Scrollen in einen
        // Bereich mit neuen Belegnummer-Längen zu Layout-Sprüngen führte.
        private double _belegSpaltenBreite = 60;
        public double BelegSpaltenBreite
        {
            get => _belegSpaltenBreite;
            set => SetProperty(ref _belegSpaltenBreite, value);
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
        // ScrolleZu*-Methoden ausgelöst.
        public event Action<JournalRow> ScrollIntoViewRequest;

        // Event für Mehrfachauswahl: die View setzt damit ListBox.SelectedItems
        // direkt (Code-Behind), weil SelectedItems nicht bindbar ist. Die
        // letzte Zeile der Liste wird zusätzlich zentriert in den Blick gescrollt.
        // Wird z.B. nach dem Ausführen von Dauerbuchungen ausgelöst.
        public event Action<System.Collections.Generic.IReadOnlyList<JournalBuchungRow>> MehrfachSelektionRequest;

        /// <summary>
        /// Selektiert alle Buchungszeilen mit den angegebenen Uuids und scrollt
        /// die zuletzt gefundene zentriert in den sichtbaren Bereich. Zeilen,
        /// die zu keiner Uuid passen, bleiben unselektiert. Muss aufgerufen
        /// werden, NACHDEM die Zeilenliste aufgebaut wurde (nach Aktualisiere).
        /// </summary>
        public void SelektiereBuchungen(IList<Guid> uuids)
        {
            if (uuids == null || uuids.Count == 0) return;
            var set = new HashSet<Guid>(uuids);
            var treffer = Zeilen.OfType<JournalBuchungRow>()
                .Where(r => r.Buchung != null && set.Contains(r.Buchung.Uuid))
                .ToList();
            if (treffer.Count == 0) return;

            // Primärselektion (für Kontextmenü/Tastatur) auf die letzte setzen.
            SelektierteZeile = treffer[treffer.Count - 1];
            MehrfachSelektionRequest?.Invoke(treffer);
        }

        /// <summary>
        /// Sucht die erste Buchungs-Zeile mit passendem Monat in der
        /// gewünschten Buchungsart und scrollt sie in den sichtbaren
        /// Bereich. Wenn kein exakter Monatstreffer existiert, wird zur
        /// zeitlich nächsten Buchung gescrollt (analog OnNMClick im
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
        /// gewünschten Buchungsart. Leerer kontoName = "unzugewiesen".
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

        // ----------------------------------------------
        // Mehrfachauswahl
        //
        // Die View meldet bei jeder SelectionChanged die komplette
        // ListBox-Selektion hierher (SelectedItems ist nicht bindbar).
        // Die Command-CanExecute werten darauf aus: Aendern/Kopieren/
        // Kopieren-mit-Beleg/AfA-Abgang nur bei GENAU EINER Selektion,
        // Loeschen bei einer ODER mehreren (loescht dann alle).
        // ----------------------------------------------
        private readonly List<JournalBuchungRow> _selektierteZeilen =
            new List<JournalBuchungRow>();

        /// <summary>Anzahl aktuell selektierter Buchungszeilen.</summary>
        public int AnzahlSelektiert => _selektierteZeilen.Count;

        /// <summary>
        /// Von der View bei SelectionChanged aufgerufen. Aktualisiert die
        /// interne Mehrfachauswahl und stoesst eine Neubewertung der
        /// Command-Verfuegbarkeit an.
        /// </summary>
        public void SetzeSelektion(IEnumerable<JournalBuchungRow> zeilen)
        {
            _selektierteZeilen.Clear();
            if (zeilen != null)
                _selektierteZeilen.AddRange(zeilen.Where(z => z != null && z.Buchung != null));
            System.Windows.Input.CommandManager.InvalidateRequerySuggested();
        }

        // Commands
        public ICommand BearbeitenCommand { get; }
        public ICommand LoeschenCommand { get; }
        public ICommand KopierenCommand { get; }
        public ICommand KopierenMitNeuerBelegnummerCommand { get; }
        public ICommand AfaAbgangCommand { get; }

        // Events
        public event Action<Buchung> BuchungBearbeiten;
        /// <summary>Loescht alle uebergebenen Buchungen (eine oder mehrere).</summary>
        public event Action<System.Collections.Generic.IList<Buchung>> BuchungenLoeschen;
        public event Action<Buchung> BuchungKopieren;
        public event Action<Buchung> BuchungKopierenMitNeuerBelegnummer;
        public event Action<Buchung> BuchungAfaAbgang;

        // Genau eine Buchung selektiert?
        private bool GenauEine => _selektierteZeilen.Count == 1
                                  && _selektierteZeilen[0].Buchung != null;

        public JournalViewModel(BuchungsDocument doc)
        {
            _doc = doc ?? throw new ArgumentNullException(nameof(doc));

            BearbeitenCommand = new RelayCommand(
                () => { if (GenauEine) BuchungBearbeiten?.Invoke(_selektierteZeilen[0].Buchung); },
                () => GenauEine);
            // Loeschen: eine ODER mehrere -- loescht alle Selektierten.
            LoeschenCommand = new RelayCommand(
                () =>
                {
                    var liste = _selektierteZeilen
                        .Where(z => z.Buchung != null)
                        .Select(z => z.Buchung)
                        .ToList();
                    if (liste.Count > 0) BuchungenLoeschen?.Invoke(liste);
                },
                () => _selektierteZeilen.Count >= 1);
            KopierenCommand = new RelayCommand(
                () => { if (GenauEine) BuchungKopieren?.Invoke(_selektierteZeilen[0].Buchung); },
                () => GenauEine);
            KopierenMitNeuerBelegnummerCommand = new RelayCommand(
                () => { if (GenauEine) BuchungKopierenMitNeuerBelegnummer?.Invoke(_selektierteZeilen[0].Buchung); },
                () => GenauEine);
            AfaAbgangCommand = new RelayCommand(
                () => { if (GenauEine) BuchungAfaAbgang?.Invoke(_selektierteZeilen[0].Buchung); },
                // Nur bei genau einer noch laufenden Anlage -- Abgang-Buchungen
                // (AfaJahre==1) können nicht nochmal ausgeschieden werden.
                () => GenauEine && _selektierteZeilen[0].Buchung.AfaJahre > 1);
        }

        /// <summary>
        /// Baut die Zeilenliste neu auf Basis des aktuellen Filters auf.
        /// </summary>
        public void Aktualisiere(JournalFilter filter = null)
        {
            if (filter != null) AktuellerFilter = filter;

            // Vor dem Clear die Uuid der aktuellen Selektion festhalten -
            // die Buchung^-Referenz darf sich beim nächsten SyncNativeToManaged
            // ändern, die Uuid wird über das native Feld stabil gehalten.
            var alteUuid = SelektierteZeile?.Buchung?.Uuid ?? Guid.Empty;
            Zeilen.Clear();

            // Saldo-Spalte nur im Bestandskonten-Modus sichtbar machen.
            // In den anderen Modi auf 0 - die Spalte ist dann ein
            // Null-Pixel-Strich und stört das Layout nicht.
            SaldoSpaltenBreite =
                AktuellerFilter.AnzeigeModus == JournalAnzeigeModus.Bestandskonten
                ? 110.0 : 0.0;

            // Beleg-Spaltenbreite einmal global berechnen (bleibt während
            // des Scrollens konstant).
            BelegSpaltenBreite = BerechneBelegSpaltenBreite();

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

            // Selektion wiederherstellen via Uuid-Match - überlebt jeden
            // SyncNativeToManaged-Zyklus, weil die Bridge die Uuid native
            // mitführt. Anschliessend zum wiederhergestellten Eintrag
            // zentrieren (ScrollIntoViewRequest), damit die Buchung nach
            // einer Bearbeitung sichtbar bleibt.
            if (alteUuid != Guid.Empty)
            {
                SelektierteZeile = Zeilen.OfType<JournalBuchungRow>()
                    .FirstOrDefault(r => r.Buchung != null
                                       && r.Buchung.Uuid == alteUuid);
                if (SelektierteZeile != null)
                    ScrollIntoViewRequest?.Invoke(SelektierteZeile);
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

            // Liste aller Bestandskonten, für die es Buchungen gibt
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

                // Buchungen für dieses Bestandskonto sammeln.
                // Filter dabei beachten (Monat, Betrieb, Konto), aber NICHT
                // BestandskontoFilter - der wird hier durch das Konto selbst
                // überschrieben.
                var alleBuchungen = new List<Buchung>();
                alleBuchungen.AddRange(FilterBuchungenOhneBestandskonto(_doc.Einnahmen, true)
                    .Where(b => (b.Bestandskonto ?? "") == bk));
                alleBuchungen.AddRange(FilterBuchungenOhneBestandskonto(_doc.Ausgaben, false)
                    .Where(b => (b.Bestandskonto ?? "") == bk));

                // Buchungen chronologisch sortieren
                alleBuchungen = alleBuchungen
                    .OrderBy(b => b.Datum)
                    .ThenBy(b => b.Uuid)   // stabile Reihenfolge bei Datums-Gleichstand
                    .ToList();

                // Wenn kein Anfangssaldo und keine Buchungen - Konto überspringen
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

                // Pseudozeile für Anfangssaldo - als JournalBuchungRow
                // ohne Buchung-Referenz (keine Bearbeiten-Aktion).
                // Brutto bleibt leer, Saldo enthält den Anfangswert.
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
                    // AfA-Nr auch nicht zeigen (die Spalte bleibt für
                    // die Waehrung im Footer reserviert).
                    zeile.AfaNrText = "";
                    // Laufender Saldo
                    zeile.SaldoText = FormatBetrag(saldoCent);
                    Zeilen.Add(zeile);
                }

                // Footer: Endsaldo. Brutto-Summe leer, Saldo gefüllt.
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
        // Pro AfA-Konto eine Tabelle mit den Anlagegütern + AfA-Status.
        // Im Original-MFC zeigt es Buchungen mit AfaJahre>1 an.
        private void BaueAnzeigeAnlagenverzeichnis()
        {
            // 1) Laufende Anlagen (AfaJahre > 1), pro Konto eigene Sub-Sektion
            var anlageBuchungen = FilterBuchungen(_doc.Ausgaben, false)
                .Where(b => b.AfaJahre > 1)
                .ToList();

            // 2) Abgänge des aktuellen Buchungsjahres (siehe AfAAbgang in
            //    easycashview.cpp:7008): AfaJahre==1 + Erweiterung
            //    "UrspruenglichesAnschaffungsdatum" gesetzt + Datum im
            //    aktuellen Jahr.
            var abgangsBuchungen = FilterBuchungen(_doc.Ausgaben, false)
                .Where(b => b.AfaJahre == 1
                         && b.Datum.Year == _doc.Jahr
                         && b.Erweiterungen != null
                         && b.Erweiterungen.Hat("EasyCash", "UrspruenglichesAnschaffungsdatum"))
                .ToList();

            if (anlageBuchungen.Count == 0 && abgangsBuchungen.Count == 0) return;

            Zeilen.Add(new JournalSectionTitle
            {
                Text = "ANLAGENVERZEICHNIS",
                IsMain = true,
                IsEinnahme = null   // neutral: kein Farbtrigger, Standardfarbe schwarz
            });

            // -- Laufende Anlagen --------------------------------------------
            var byKonto = anlageBuchungen
                .GroupBy(b => b.Konto ?? "")
                .OrderBy(g => g.Key);

            foreach (var grp in byKonto)
            {
                Zeilen.Add(new JournalSectionTitle
                {
                    Text = string.IsNullOrEmpty(grp.Key)
                        ? "[noch zu keinem Konto zugewiesene Anlagegüter]"
                        : "[" + grp.Key + "]",
                    IsMain = false,
                    IsEinnahme = null   // neutral: schwarz
                });
                Zeilen.Add(new JournalAnlagenHeaderRow());

                long sumAnsch = 0, sumAfa = 0, sumEnde = 0;
                int idx = 0;
                foreach (var b in grp.OrderBy(b => b.Datum).ThenBy(b => b.Uuid))
                {
                    long ansch  = b.BruttoBetrag.InCent;
                    long beginn = b.AfaRestwertCent;
                    long afa    = AfaCalculator.GetBuchungsjahrNetto(
                                     b, _doc.GlobaleAfaGenauigkeit);
                    long ende   = beginn - afa;
                    if (ende < 0) ende = 0;

                    Zeilen.Add(BaueAnlagenZeile(b, idx++,
                        anschCent: ansch,
                        beginnCent: beginn,
                        afaCent: afa,
                        abgangCent: 0,
                        endeCent: ende));

                    sumAnsch += ansch;
                    sumAfa   += afa;
                    sumEnde  += ende;
                }
                Zeilen.Add(BaueAnlagenFooter(sumAnsch, sumAfa, 0, sumEnde));
                Zeilen.Add(new JournalSpacerRow());
            }

            // -- Abgänge des aktuellen Jahres -------------------------------
            if (abgangsBuchungen.Count > 0)
            {
                Zeilen.Add(new JournalSectionTitle
                {
                    Text = "[Abgänge " + _doc.Jahr + "]",
                    IsMain = false,
                    IsEinnahme = null   // neutral: schwarz
                });
                Zeilen.Add(new JournalAnlagenHeaderRow());

                long sumAnsch = 0, sumAbgang = 0;
                int idx = 0;
                foreach (var b in abgangsBuchungen.OrderBy(b => b.Datum).ThenBy(b => b.Uuid))
                {
                    // Werte aus den Original-Erweiterungs-Keys ablesen
                    string anschStr = b.Erweiterungen.Hole(
                        "EasyCash", "UrspruenglicherBetrag", "");
                    string beginnStr = b.Erweiterungen.Hole(
                        "EasyCash", "UrspruenglicherRestwert", "");
                    string afaNrStr = b.Erweiterungen.Hole(
                        "EasyCash", "UrspruenglicheAbschreibungNr", "");
                    string afaJahreStr = b.Erweiterungen.Hole(
                        "EasyCash", "UrspruenglicheAbschreibungJahre", "");
                    string anschDatumStr = b.Erweiterungen.Hole(
                        "EasyCash", "UrspruenglichesAnschaffungsdatum", "");

                    long anschCent  = ParseCurrencyToCent(anschStr);
                    long abgangCent = b.BruttoBetrag.InCent;

                    Zeilen.Add(new JournalAnlagenRow
                    {
                        Buchung      = b,
                        IstAusgabe   = true,
                        ZebraIndex   = idx++,
                        BeschreibungText = b.Beschreibung ?? "",
                        AnschDatumText   = anschDatumStr,
                        AnschKostenText  = anschStr,
                        BuchwBeginnText  = beginnStr,
                        AfaJahresbetragText = "",
                        AbgangText       = FormatBetrag(abgangCent),
                        BuchwEndeText    = FormatBetrag(0),
                        AfaNrText        = string.IsNullOrEmpty(afaNrStr)
                                            ? ""
                                            : afaNrStr + "/" + afaJahreStr,
                        BetriebIcon = HoleIcon(IconArt.Betrieb, b.Betrieb),
                        BestandskontoIcon = HoleIcon(IconArt.Bestandskonto, b.Erweiterungen.Hole("EasyCash",
                                "UrspruenglichesBestandskonto", b.Bestandskonto ?? ""))
                    });

                    sumAnsch  += anschCent;
                    sumAbgang += abgangCent;
                }
                Zeilen.Add(BaueAnlagenFooter(sumAnsch, 0, sumAbgang, 0));
                Zeilen.Add(new JournalSpacerRow());
            }
        }

        /// <summary>
        /// Baut eine Anlagen-Zeile mit vorberechneten Cent-Betraegen.
        /// </summary>
        private JournalAnlagenRow BaueAnlagenZeile(
            Buchung b, int zebraIdx,
            long anschCent, long beginnCent, long afaCent,
            long abgangCent, long endeCent)
        {
            return new JournalAnlagenRow
            {
                Buchung = b,
                IstAusgabe = true,
                ZebraIndex = zebraIdx,
                BeschreibungText = b.Beschreibung ?? "",
                AnschDatumText = b.Datum.ToString("dd.MM.yyyy", DeDE),
                AnschKostenText = FormatBetrag(anschCent),
                BuchwBeginnText = FormatBetrag(beginnCent),
                AfaJahresbetragText = FormatBetrag(afaCent),
                AbgangText = abgangCent != 0 ? FormatBetrag(abgangCent) : "",
                BuchwEndeText = FormatBetrag(endeCent),
                AfaNrText = b.AfaJahre > 1 ? $"{b.AfaNr}/{b.AfaJahre}" : "",
                BetriebIcon = HoleIcon(IconArt.Betrieb, b.Betrieb),
                BestandskontoIcon = HoleIcon(IconArt.Bestandskonto, b.Bestandskonto)
            };
        }

        private JournalAnlagenFooterRow BaueAnlagenFooter(
            long anschSumme, long afaSumme, long abgangSumme, long endeSumme)
        {
            return new JournalAnlagenFooterRow
            {
                AnschKostenSummeText = FormatBetrag(anschSumme),
                AfaSummeText         = FormatBetrag(afaSumme),
                AbgangSummeText      = FormatBetrag(abgangSumme),
                BuchwEndeSummeText   = FormatBetrag(endeSumme),
                Waehrung             = _doc.Waehrung ?? "EUR"
            };
        }

        /// <summary>
        /// Parst eine deutsche Waehrungs-Zeichenkette ("1234,56" oder
        /// "-1234,56") nach Cent. Toleriert auch Punkte als
        /// Tausendertrenner ("1.234,56"). Liefert 0 bei Parse-Fehler.
        /// </summary>
        private static long ParseCurrencyToCent(string s)
        {
            if (string.IsNullOrWhiteSpace(s)) return 0;
            s = s.Trim().Replace(".", "");  // Tausendertrenner weg
            if (decimal.TryParse(s,
                System.Globalization.NumberStyles.Number,
                DeDE, out var d))
                return (long)Math.Round(d * 100m);
            return 0;
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

            // Dann zusätzliche aus Buchungen
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
            return ViewHost.BestandskontenNamen ?? (IList<string>)new List<string>();
        }

        private long HoleAnfangssaldoCent(string bestandskonto)
        {
            var bks  = Einstellungen.Bestandskonten;
            // Anfangssaldo des laufenden Jahres = gespeicherter Saldo des
            // VORJAHRES (Uebertrag). Die ini speichert unter "Saldo<Y>" den
            // Stand, der ins Jahr Y+1 uebernommen wird -- siehe natives
            // UpdateBestandskonten (easycashview.cpp: ...Saldo%04d, nJahr-1).
            int jahr = _doc.Jahr - 1;
            for (int i = 0; i < bks.Count; i++)
            {
                if (bks[i].Name == bestandskonto)
                {
                    if (bks[i].Saldo.TryGetValue(jahr, out decimal d))
                        return (long)(d * 100m);
                    return 0;
                }
            }
            return 0;
        }

        // Berechnet die globale Belegspalten-Breite einmal pro Aktualisiere:
        // das 95-Perzentil aller Belegnummer-Längen (inkl. leerer Belege)
        // mal einer groben Zeichenbreite, geklammert auf [Mindestbreite für
        // 6 Ziffern, BelegMaxBreite (1/4 der ListBox-Breite)]. Längere
        // Belegnummern werden im TextBlock via TextTrimming="..." abgekürzt.
        private double BerechneBelegSpaltenBreite()
        {
            // grobe Heuristik für Segoe UI: ~0.6 x Schriftgroesse pro Zeichen
            double zeichenBreite = Schriftgroesse * 0.6;
            const double rand = 8.0;  // 4 px links + 4 px rechts (TextBlock Margin)
            double minBreite = 6 * zeichenBreite + rand;

            if (_doc?.Buchungen == null || _doc.Buchungen.Count == 0)
                return minBreite;

            var laengen = _doc.Buchungen
                .Select(b => (b.Belegnummer ?? "").Length)
                .OrderBy(n => n)
                .ToList();

            int p95Idx = (int)Math.Ceiling(laengen.Count * 0.95) - 1;
            if (p95Idx < 0) p95Idx = 0;
            if (p95Idx >= laengen.Count) p95Idx = laengen.Count - 1;
            double berechnet = laengen[p95Idx] * zeichenBreite + rand;

            return Math.Max(minBreite, Math.Min(BelegMaxBreite, berechnet));
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
                    return $"{typ} für {f.MonatsFilter - 12}. Quartal {jahr}";
                return $"{typ} für Monat {f.MonatsFilter:D2}/{jahr}";
            }
            return $"{typ} für {jahr}";
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
                BetriebIcon = HoleIcon(IconArt.Betrieb, b.Betrieb),
                BestandskontoIcon = HoleIcon(IconArt.Bestandskonto, b.Bestandskonto)
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

        /// <summary>
        /// Liefert das Betrieb- oder Bestandskonto-Icon zum angegebenen Namen.
        /// Liest Sprite und Mapping bei jedem Aufruf frisch aus ViewHost - das
        /// ist wichtig, damit Icons auch direkt nach dem ersten Anzeigen
        /// erscheinen (siehe Kommentar oben bei den Feldern).
        /// </summary>
        private enum IconArt { Betrieb, Bestandskonto }

        private BitmapSource HoleIcon(IconArt art, string name)
        {
            if (string.IsNullOrEmpty(name)) return null;

            BitmapSource    sprite;
            IList<string>   namen;
            IList<int>      indizes;
            string          keyPrefix;
            if (art == IconArt.Betrieb)
            {
                sprite    = ViewHost.SpriteBetriebe;
                namen     = ViewHost.BetriebeNamen;
                indizes   = ViewHost.BetriebeIcons;
                keyPrefix = "B";
            }
            else
            {
                sprite    = ViewHost.SpriteBestandskonten;
                namen     = ViewHost.BestandskontenNamen;
                indizes   = ViewHost.BestandskontenIcons;
                keyPrefix = "K";
            }
            if (sprite == null || namen == null || indizes == null) return null;

            int pos = -1;
            for (int i = 0; i < namen.Count; i++)
                if (namen[i] == name) { pos = i; break; }
            if (pos < 0 || pos >= indizes.Count) return null;
            int idx = indizes[pos];

            string cacheKey = $"{keyPrefix}:{idx}";
            if (_iconCache.TryGetValue(cacheKey, out var cached))
                return cached;

            var icon = IconSpriteSplitter.Crop(sprite, idx) as BitmapSource;
            if (icon != null) _iconCache[cacheKey] = icon;
            return icon;
        }
    }
}
