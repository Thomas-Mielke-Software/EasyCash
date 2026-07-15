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

        /// <summary>
        /// Schriftfamilie des Journals = konfigurierte Bildschirmschrift
        /// (GlobaleEinstellungen). Leere/fehlende Einstellung -> "Segoe UI".
        /// Wird in <see cref="Aktualisiere"/> neu gemeldet, damit eine in den
        /// Einstellungen geaenderte Schrift beim naechsten Refresh greift.
        /// </summary>
        public string Schriftart
        {
            get
            {
                var s = GlobaleEinstellungen.Bildschirmschrift;
                return string.IsNullOrWhiteSpace(s) ? "Segoe UI" : s;
            }
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
                    SkalierungAktualisieren();
                }
            }
        }

        // ----------------------------------------------------------
        // Skalierung: alle festen Spaltenbreiten sind auf die Basis-
        // Schriftgroesse 13 ausgelegt und wachsen/schrumpfen linear mit
        // der tatsaechlichen Schriftgroesse (Einstellungen bzw. Zoom).
        // Ohne diese Kopplung wird der Text in den Fixbreiten-Spalten
        // bei groesseren Schriften abgeschnitten.
        // ----------------------------------------------------------
        private const double BasisSchriftgroesse = 13.0;
        private double Skala => Schriftgroesse / BasisSchriftgroesse;

        public double IconSpaltenBreite   => 28.0  * Skala;
        public double DatumSpaltenBreite  => 90.0  * Skala;
        public double BruttoSpaltenBreite => 110.0 * Skala;
        public double AfaSpaltenBreite    => 60.0  * Skala;

        // Abschnittstitel: 15/17 bei Basis-Schriftgroesse 13, zoomen mit.
        public double TitelSchriftgroesse      => 15.0 * Skala;
        public double HauptTitelSchriftgroesse => 17.0 * Skala;

        // Icon-Groesse = Hoehe EINER Textzeile der aktuellen Schrift
        // (FontSize x LineSpacing der FontFamily). Dadurch bestimmt die
        // Textzeile die Zeilenhoehe -- ein groesseres Icon (frueher fix
        // 20px) wuerde die Zeile aufblaehen und der oben ausgerichtete
        // Text saesse bei kleinen Schriften sichtbar zu hoch.
        private string _lineSpacingSchrift;
        private double _lineSpacing = 4.0 / 3.0;
        public double IconGroesse
        {
            get
            {
                if (_lineSpacingSchrift != Schriftart)
                {
                    try
                    {
                        _lineSpacing = new System.Windows.Media
                            .FontFamily(Schriftart).LineSpacing;
                    }
                    catch { _lineSpacing = 4.0 / 3.0; }
                    _lineSpacingSchrift = Schriftart;
                }
                return Schriftgroesse * _lineSpacing;
            }
        }

        /// <summary>
        /// Meldet alle von der Schriftgroesse abgeleiteten Layout-Properties
        /// neu und rechnet die gespeicherten (sichtbarkeitsabhaengigen)
        /// Spaltenbreiten auf die neue Skala um. Wird vom Schriftgroesse-
        /// Setter (Zoom) und von Aktualisiere() gerufen.
        /// </summary>
        private void SkalierungAktualisieren()
        {
            OnPropertyChanged(nameof(IconSpaltenBreite));
            OnPropertyChanged(nameof(IconGroesse));
            OnPropertyChanged(nameof(DatumSpaltenBreite));
            OnPropertyChanged(nameof(BruttoSpaltenBreite));
            OnPropertyChanged(nameof(AfaSpaltenBreite));
            OnPropertyChanged(nameof(TitelSchriftgroesse));
            OnPropertyChanged(nameof(HauptTitelSchriftgroesse));

            SaldoSpaltenBreite     = _zeigeSaldo  ? SaldoBreite         * Skala : 0.0;
            NettoSpaltenBreite     = _zeigeSteuer ? SteuerNettoBreite   * Skala : 0.0;
            UStSatzSpaltenBreite   = _zeigeSteuer ? SteuerSatzBreite    * Skala : 0.0;
            UStBetragSpaltenBreite = _zeigeSteuer ? SteuerBetragBreite  * Skala : 0.0;
            BelegSpaltenBreite     = BerechneBelegSpaltenBreite();
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

        // Breiten der drei Steuer-Spalten (Netto, USt-Satz, USt-Betrag).
        // Wie bei der Saldo-Spalte an MinWidth+MaxWidth der ColumnDefinitions
        // in den Header-/Buchung-/Footer-Templates gebunden. Werden in
        // Aktualisiere() auf 0 gesetzt, wenn keine angezeigte Buchung einen
        // USt-Betrag != 0 hat -- dann faellt der frei werdende Platz der
        // (Star-)Beschreibungsspalte zu, statt verschenkt zu werden.
        private const double SteuerNettoBreite = 100.0;
        private const double SteuerSatzBreite  = 60.0;
        private const double SteuerBetragBreite = 100.0;
        private const double SaldoBreite = 110.0;

        // Sichtbarkeits-Entscheidung der Saldo-Spalte (Bestandskonten-Modus),
        // festgehalten fuer die Neu-Skalierung beim Zoomen.
        private bool _zeigeSaldo;

        private double _nettoSpaltenBreite = SteuerNettoBreite;
        public double NettoSpaltenBreite
        {
            get => _nettoSpaltenBreite;
            set => SetProperty(ref _nettoSpaltenBreite, value);
        }

        private double _ustSatzSpaltenBreite = SteuerSatzBreite;
        public double UStSatzSpaltenBreite
        {
            get => _ustSatzSpaltenBreite;
            set => SetProperty(ref _ustSatzSpaltenBreite, value);
        }

        private double _ustBetragSpaltenBreite = SteuerBetragBreite;
        public double UStBetragSpaltenBreite
        {
            get => _ustBetragSpaltenBreite;
            set => SetProperty(ref _ustBetragSpaltenBreite, value);
        }

        // Globale Entscheidung pro Aktualisiere(): werden die Steuer-Spalten
        // ueberhaupt angezeigt? Steuert sowohl die Spaltenbreiten als auch die
        // Text-Sichtbarkeit (ZeigeSteuer) der Header-/Footer-Zeilen.
        private bool _zeigeSteuer = true;

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
            set
            {
                // Manuelle Selektion (Klick/Tastatur) löst den
                // Mehrfach-Selektions-Merker -- programmgesteuerte Setzer
                // (SelektiereBuchungen, Rebuild-Restore) setzen die Guard.
                if (SetProperty(ref _selektierteZeile, value) && !_selektionIntern)
                    _mehrfachMerker = null;
            }
        }

        // Merker der letzten programmatischen Mehrfach-Selektion (z.B.
        // Buchungsgruppe nach dem Buchen/Bearbeiten). Wird beim Neuaufbau
        // der Zeilen re-appliziert, damit die Gruppen-Markierung den
        // nachfolgenden Journal-Refresh des nativen Aufrufers überlebt.
        private List<Guid> _mehrfachMerker;
        private bool _selektionIntern;

        /// <summary>
        /// Setzt/löst die interne Selektions-Guard von aussen. Die View ruft
        /// das um ihre programmatische SelectedItems-Manipulation herum auf
        /// (OnMehrfachSelektion): das Clear/Add feuert die SelectedItem-
        /// Bindung zurück in <see cref="SelektierteZeile"/>, was ohne Guard
        /// den Mehrfach-Merker löschen und die Gruppen-Expansion erneut
        /// anstossen würde (Endlosschleife).
        /// </summary>
        internal void SetzeSelektionsGuard(bool aktiv) => _selektionIntern = aktiv;

        /// <summary>
        /// Erweitert die Selektion auf die komplette Buchungsgruppe der
        /// angeklickten Zeile (Phase D: Klick auf ein Mitglied markiert die
        /// ganze Gruppe). Kein-Op, wenn die Zeile kein Gruppen-Mitglied ist
        /// oder die Gruppe nur aus einer Buchung besteht.
        /// </summary>
        public void SelektiereGruppeVon(JournalBuchungRow zeile)
        {
            var gruppenUuid = zeile?.Buchung?.GruppenUuid;
            if (string.IsNullOrEmpty(gruppenUuid)) return;

            var uuids = _doc.Buchungen
                .Where(b => b.GruppenUuid == gruppenUuid)
                .Select(b => b.Uuid)
                .ToList();
            if (uuids.Count < 2) return;
            SelektiereBuchungen(uuids);
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

            // Merker VOR der Treffer-Suche setzen: sind die Zeilen noch nicht
            // (neu) aufgebaut, greift die Selektion beim nächsten
            // Aktualisiere() -- so kann die Bridge direkt nach dem Buchen
            // selektieren, bevor der native Refresh läuft.
            _mehrfachMerker = new List<Guid>(uuids);

            var set = new HashSet<Guid>(uuids);
            var treffer = Zeilen.OfType<JournalBuchungRow>()
                .Where(r => r.Buchung != null && set.Contains(r.Buchung.Uuid))
                .ToList();
            if (treffer.Count == 0) return;

            // Primärselektion (für Kontextmenü/Tastatur) auf die letzte setzen.
            _selektionIntern = true;
            try { SelektierteZeile = treffer[treffer.Count - 1]; }
            finally { _selektionIntern = false; }
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
        public ICommand GruppeLoeschenCommand { get; }
        public ICommand KopierenCommand { get; }
        public ICommand KopierenMitNeuerBelegnummerCommand { get; }
        public ICommand AfaAbgangCommand { get; }

        // Events
        /// <summary>
        /// Zoom-Aenderungswunsch aus der View (Strg-'+'/'-' bzw. Strg-Mausrad),
        /// Delta in Prozentpunkten (+25/-25). Wird von der Bridge an den
        /// nativen Zoom-Mechanismus weitergereicht (SetzeZoomfaktor), damit
        /// Profil-Persistenz, Statuszeile und alle Journals synchron bleiben.
        /// </summary>
        public event Action<int> ZoomAendern;
        public void MeldeZoomAenderung(int deltaProzent) => ZoomAendern?.Invoke(deltaProzent);

        public event Action<Buchung> BuchungBearbeiten;
        /// <summary>Loescht alle uebergebenen Buchungen (eine oder mehrere).</summary>
        public event Action<System.Collections.Generic.IList<Buchung>> BuchungenLoeschen;
        public event Action<Buchung> BuchungKopieren;
        public event Action<Buchung> BuchungKopierenMitNeuerBelegnummer;
        public event Action<Buchung> BuchungAfaAbgang;

        // Genau eine Buchung selektiert?
        private bool GenauEine => _selektierteZeilen.Count == 1
                                  && _selektierteZeilen[0].Buchung != null;

        // True wenn ALLE selektierten Zeilen zur selben Buchungsgruppe
        // gehören (mindestens zwei). Weil ein Klick auf ein Gruppen-Mitglied
        // die ganze Gruppe markiert, müssen die Einzel-Kommandos (Ändern/
        // Kopieren/AfA-Abgang) diesen Zustand wie eine Einzelselektion
        // behandeln -- sonst wären sie für Gruppen-Mitglieder unerreichbar.
        private bool SelektionIstEineGruppe
        {
            get
            {
                if (_selektierteZeilen.Count < 2) return false;
                string uuid = _selektierteZeilen[0].Buchung?.GruppenUuid;
                if (string.IsNullOrEmpty(uuid)) return false;
                return _selektierteZeilen.All(
                    z => z.Buchung?.GruppenUuid == uuid);
            }
        }

        // Ziel-Buchung für die Einzel-Kommandos: genau eine Selektion ODER
        // eine komplett selektierte Buchungsgruppe (dann die Primärzeile,
        // Fallback erste Zeile). Null wenn kein eindeutiges Ziel existiert.
        private Buchung EinzelZiel
        {
            get
            {
                if (GenauEine) return _selektierteZeilen[0].Buchung;
                if (SelektionIstEineGruppe)
                {
                    var prim = _selektierteZeile;
                    if (prim?.Buchung != null && _selektierteZeilen.Contains(prim))
                        return prim.Buchung;
                    return _selektierteZeilen[0].Buchung;
                }
                return null;
            }
        }

        /// <summary>
        /// Alle Buchungen der Gruppen, die in der aktuellen Selektion
        /// vertreten sind (frisch aus dem Dokument aufgelöst) -- Basis für
        /// "Buchungsgruppe löschen".
        /// </summary>
        private List<Buchung> GruppenMitgliederDerSelektion()
        {
            var gruppen = new HashSet<string>(_selektierteZeilen
                .Select(z => z.Buchung?.GruppenUuid)
                .Where(u => !string.IsNullOrEmpty(u)));
            if (gruppen.Count == 0) return new List<Buchung>();
            return _doc.Buchungen
                .Where(b => b.GruppenUuid != null && gruppen.Contains(b.GruppenUuid))
                .ToList();
        }

        public JournalViewModel(BuchungsDocument doc)
        {
            _doc = doc ?? throw new ArgumentNullException(nameof(doc));

            BearbeitenCommand = new RelayCommand(
                () => { var z = EinzelZiel; if (z != null) BuchungBearbeiten?.Invoke(z); },
                () => EinzelZiel != null);
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
            // Buchungsgruppe löschen: alle Mitglieder der Gruppe(n) der
            // Selektion auf einmal (ein Sync, ein SetModifiedFlag in der
            // Bridge). Nur aktiv, wenn die Selektion Gruppen-Mitglieder hat.
            GruppeLoeschenCommand = new RelayCommand(
                () =>
                {
                    var liste = GruppenMitgliederDerSelektion();
                    if (liste.Count > 0) BuchungenLoeschen?.Invoke(liste);
                },
                () => _selektierteZeilen.Any(
                    z => !string.IsNullOrEmpty(z.Buchung?.GruppenUuid)));
            KopierenCommand = new RelayCommand(
                () => { var z = EinzelZiel; if (z != null) BuchungKopieren?.Invoke(z); },
                () => EinzelZiel != null);
            KopierenMitNeuerBelegnummerCommand = new RelayCommand(
                () => { var z = EinzelZiel; if (z != null) BuchungKopierenMitNeuerBelegnummer?.Invoke(z); },
                () => EinzelZiel != null);
            AfaAbgangCommand = new RelayCommand(
                () => { var z = EinzelZiel; if (z != null) BuchungAfaAbgang?.Invoke(z); },
                // Nur bei einer noch laufenden Anlage -- Abgang-Buchungen
                // (AfaJahre==1) können nicht nochmal ausgeschieden werden.
                () => EinzelZiel != null && EinzelZiel.AfaJahre > 1);
        }

        /// <summary>
        /// Baut die Zeilenliste neu auf Basis des aktuellen Filters auf.
        /// </summary>
        public void Aktualisiere(JournalFilter filter = null)
        {
            if (filter != null) AktuellerFilter = filter;

            // Schrift/Groesse koennen sich (ueber die Einstellungen bzw. den
            // neuen Filter) geaendert haben -- Bindings neu benachrichtigen.
            OnPropertyChanged(nameof(Schriftart));
            OnPropertyChanged(nameof(Schriftgroesse));

            // Vor dem Clear die Uuid der aktuellen Selektion festhalten -
            // die Buchung^-Referenz darf sich beim nächsten SyncNativeToManaged
            // ändern, die Uuid wird über das native Feld stabil gehalten.
            var alteUuid = SelektierteZeile?.Buchung?.Uuid ?? Guid.Empty;
            Zeilen.Clear();

            // Saldo-Spalte nur im Bestandskonten-Modus sichtbar machen.
            // In den anderen Modi auf 0 - die Spalte ist dann ein
            // Null-Pixel-Strich und stört das Layout nicht.
            _zeigeSaldo =
                AktuellerFilter.AnzeigeModus == JournalAnzeigeModus.Bestandskonten;

            // Steuer-Spalten (Netto, USt-Satz, USt-Betrag) nur einblenden, wenn
            // mindestens eine angezeigte Buchung tatsaechlich einen USt-Betrag
            // hat (in den Modi Datum/Konten). Sonst die Spalten auf 0 Pixel
            // kollabieren, damit die Beschreibungsspalte den Platz bekommt.
            _zeigeSteuer = BerechneZeigeSteuer();

            // Spaltenbreiten (Saldo/Steuer/Beleg) passend zur aktuellen
            // Schriftgroesse setzen; skaliert auch alle Fixbreiten-Properties.
            SkalierungAktualisieren();

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
            // Mehrfach-Selektion (Buchungsgruppe) hat Vorrang: der Merker
            // re-appliziert die Gruppen-Markierung nach jedem Neuaufbau,
            // bis der Anwender manuell eine andere Zeile wählt.
            if (_mehrfachMerker != null && _mehrfachMerker.Count > 0)
            {
                SelektiereBuchungen(_mehrfachMerker);
            }
            else if (alteUuid != Guid.Empty)
            {
                _selektionIntern = true;
                try
                {
                    SelektierteZeile = Zeilen.OfType<JournalBuchungRow>()
                        .FirstOrDefault(r => r.Buchung != null
                                           && r.Buchung.Uuid == alteUuid);
                }
                finally { _selektionIntern = false; }
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
                        ZeigeSteuer = _zeigeSteuer,
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
                        ZeigeSteuer = _zeigeSteuer,
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
                        ZeigeSteuer = _zeigeSteuer
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
                        ZeigeSteuer = _zeigeSteuer,
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
        /// Parst eine Währungs-Zeichenkette nach Cent -- tolerant für
        /// deutsche ("1.234,56"), schweizerische ("1'234.56") und englische
        /// ("1,234.56") Schreibweisen (Waehrungsformat.TryParse).
        /// Liefert 0 bei Parse-Fehler.
        /// </summary>
        private static long ParseCurrencyToCent(string s)
        {
            return Waehrungsformat.TryParse(s, out decimal d)
                ? (long)Math.Round(d * 100m)
                : 0;
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

        /// <summary>
        /// Berechnet den im Journal angezeigten USt-/VSt-Betrag (in Cent) einer
        /// Buchung -- exakt nach derselben Logik wie <see cref="BaueBuchungZeile"/>.
        /// Wird zusaetzlich von <see cref="BerechneZeigeSteuer"/> genutzt, um zu
        /// entscheiden, ob die Steuer-Spalten ueberhaupt eingeblendet werden.
        /// </summary>
        private static long BerechneUstBetragCent(Buchung b, bool istAusgabe)
        {
            long bruttoCent = b.BruttoBetrag.InCent;
            if (istAusgabe)
            {
                if (b.Konto == "VST-Beträge separat")
                    return bruttoCent;
                return (b.AfaNr == 1)
                    ? bruttoCent - b.BruttoBetrag.NettoInCent
                    : 0;
            }
            return bruttoCent - b.BruttoBetrag.NettoInCent;
        }

        /// <summary>
        /// Entscheidet, ob die Steuer-Spalten (Netto, USt-Satz, USt-Betrag)
        /// angezeigt werden. Nur in den Modi Datum/Konten und nur, wenn der
        /// externe Schalter es erlaubt UND mindestens eine angezeigte Buchung
        /// einen USt-Betrag != 0 hat. Bewusst NICHT an "MwstFeldAktiviert"
        /// gekoppelt -- massgeblich sind die tatsaechlichen Buchungen.
        /// </summary>
        private bool BerechneZeigeSteuer()
        {
            var f = AktuellerFilter;
            switch (f.AnzeigeModus)
            {
                case JournalAnzeigeModus.Datum:
                case JournalAnzeigeModus.Konten:
                    if (!f.ZeigeSteuerspalte) return false;
                    return FilterBuchungen(_doc.Einnahmen, true)
                               .Any(b => BerechneUstBetragCent(b, false) != 0)
                        || FilterBuchungen(_doc.Ausgaben, false)
                               .Any(b => BerechneUstBetragCent(b, true) != 0);
                default:
                    // Bestandskonten/Anlagenverzeichnis: keine Steuer-Spalten.
                    return false;
            }
        }

        private JournalBuchungRow BaueBuchungZeile(Buchung b, bool istAusgabe, int zebraIdx)
        {
            long bruttoCent = b.BruttoBetrag.InCent;
            long nettoCent;
            long mwstBetragCent = BerechneUstBetragCent(b, istAusgabe);
            long anzeigeBruttoCent;

            if (istAusgabe)
            {
                if (b.Konto == "VST-Beträge separat")
                {
                    nettoCent = 0;
                    anzeigeBruttoCent = bruttoCent;
                }
                else
                {
                    nettoCent = AfaCalculator.GetBuchungsjahrNetto(b);
                    anzeigeBruttoCent = nettoCent + mwstBetragCent;
                }
            }
            else
            {
                nettoCent = b.BruttoBetrag.NettoInCent;
                anzeigeBruttoCent = bruttoCent;
            }

            return new JournalBuchungRow
            {
                Buchung = b,
                IstAusgabe = istAusgabe,
                ZebraIndex = zebraIdx,
                GruppenUuid = b.GruppenUuid,
                GruppenTooltip = BaueGruppenTooltip(b),
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

        /// <summary>
        /// Tooltip für das "[G]"-Symbol eines Gruppen-Mitglieds; null wenn
        /// die Buchung zu keiner Buchungsgruppe gehört.
        /// </summary>
        private static string BaueGruppenTooltip(Buchung b)
        {
            if (string.IsNullOrEmpty(b.GruppenUuid)) return null;
            int rolle = b.GruppenRolle;
            if (rolle == 0)
                return "Teil einer Buchungsgruppe (Basisbuchung)";
            if (rolle > 0)
                return "Teil einer Buchungsgruppe (Zusatzbuchung " + rolle + ")";
            return "Teil einer Buchungsgruppe";
        }

        private JournalFooterRow BaueFooter(
            bool isAusgabe, long netto, long steuer, long brutto)
        {
            return new JournalFooterRow
            {
                IsAusgabe = isAusgabe,
                ZeigeSteuer = _zeigeSteuer,
                NettoSummeText = FormatBetrag(netto),
                SteuerSummeText = FormatBetrag(steuer),
                BruttoSummeText = FormatBetrag(brutto),
                Waehrung = _doc.Waehrung ?? "EUR"
            };
        }

        private string FormatBetrag(long cent)
            => Waehrungsformat.Betrag(cent / 100m);

        private string FormatMwstSatz(int promille)
        {
            if (promille == 0) return "";
            return Waehrungsformat.Zahl(promille / 1000m) + "%";
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
