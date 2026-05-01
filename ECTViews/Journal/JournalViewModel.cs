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

        // Selektion
        private JournalBuchungRow _selektierteZeile;
        public JournalBuchungRow SelektierteZeile
        {
            get => _selektierteZeile;
            set => SetProperty(ref _selektierteZeile, value);
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

            switch (AktuellerFilter.AnzeigeModus)
            {
                case JournalAnzeigeModus.Datum:
                    BaueAnzeigeNachDatum();
                    break;
                case JournalAnzeigeModus.Konten:
                    BaueAnzeigeNachKonten();
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
