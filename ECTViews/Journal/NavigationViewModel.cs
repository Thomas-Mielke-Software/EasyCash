// NavigationViewModel.cs - Hauptlogik des Navigationsfensters.
//
// Refaktoriert die OnViewUpdate-Logik aus easycashview.cpp (Zeilen
// 1051-1267), die eigentlich nicht in den View, sondern in CNavigation
// gehoert. Hier ist das jetzt sauber getrennt.
//
// Aufgaben:
//   - Liste aller Navigations-Gruppen + Items aufbauen, abhaengig vom
//     aktuellen Journal-Modus
//   - Beim Klick auf ein Item: das passende ScrolleZu* am JournalViewModel
//     aufrufen
//
// Kommunikation mit dem JournalViewModel: ueber direkte Referenz (vom
// Konstruktor mitgegeben) - das ist einfacher als ein Mediator-Pattern
// und reicht hier vollstaendig.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Input;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.Journal
{
    public class NavigationViewModel : ViewModelBase
    {
        private static readonly string[] cpMonat = {
            "Januar", "Februar", "Maerz", "April", "Mai", "Juni",
            "Juli", "August", "September", "Oktober", "November", "Dezember"
        };

        private readonly BuchungsDocument _doc;
        private readonly JournalViewModel _journal;

        public ObservableCollection<NavigationGruppe> Gruppen { get; }
            = new ObservableCollection<NavigationGruppe>();

        // Wird vom Item-Click ausgeloest.
        public ICommand NavigiereCommand { get; }

        public NavigationViewModel(BuchungsDocument doc, JournalViewModel journal)
        {
            _doc = doc ?? throw new ArgumentNullException(nameof(doc));
            _journal = journal ?? throw new ArgumentNullException(nameof(journal));

            NavigiereCommand = new RelayCommand<NavigationItem>(NavigiereZu);
        }

        /// <summary>
        /// Baut die Navigation entsprechend dem aktuellen JournalFilter neu auf.
        /// Aufrufen bei Filter-Aenderung oder nach Buchungs-Aenderung.
        /// </summary>
        public void Aktualisiere()
        {
            Gruppen.Clear();

            switch (_journal.AktuellerFilter.AnzeigeModus)
            {
                case JournalAnzeigeModus.Datum:
                    BaueGruppenNachDatum();
                    break;
                case JournalAnzeigeModus.Konten:
                case JournalAnzeigeModus.Anlagenverzeichnis:
                    BaueGruppenNachKonten(
                        _journal.AktuellerFilter.AnzeigeModus
                            == JournalAnzeigeModus.Anlagenverzeichnis);
                    break;
                case JournalAnzeigeModus.Bestandskonten:
                    BaueGruppenBestandskonten();
                    break;
            }
        }

        // Modus 0: Datum -> "Einnahmen" und "Ausgaben" mit je 12 Monatsitems
        private void BaueGruppenNachDatum()
        {
            var einnahmen = new NavigationGruppe { Header = "Einnahmen" };
            var ausgaben = new NavigationGruppe { Header = "Ausgaben" };
            for (int i = 1; i <= 12; i++)
            {
                einnahmen.Items.Add(new NavigationItem
                {
                    Text = cpMonat[i - 1],
                    SprungZiel = $"E:{i}"
                });
                ausgaben.Items.Add(new NavigationItem
                {
                    Text = cpMonat[i - 1],
                    SprungZiel = $"A:{i}"
                });
            }
            Gruppen.Add(einnahmen);
            Gruppen.Add(ausgaben);
        }

        // Modus 1/3: Konten/Anlagen -> "Einnahmen" und "Ausgaben" mit
        // je den Konten als Items
        private void BaueGruppenNachKonten(bool nurAusgabenAfA)
        {
            // Konten bei Einnahmen sammeln
            var einnahmenKonten = new HashSet<string>();
            bool unzugewieseneEinnahmen = false;
            foreach (var b in _doc.Einnahmen)
            {
                if (nurAusgabenAfA) break;  // Anlagenverzeichnis: keine Einnahmen
                if (string.IsNullOrEmpty(b.Konto))
                    unzugewieseneEinnahmen = true;
                else
                    einnahmenKonten.Add(b.Konto);
            }

            // Konten bei Ausgaben sammeln (nur AfA-Buchungen, wenn Anlagenmodus)
            var ausgabenKonten = new HashSet<string>();
            bool unzugewieseneAusgaben = false;
            foreach (var b in _doc.Ausgaben)
            {
                if (nurAusgabenAfA && b.AfaJahre <= 1) continue;
                if (string.IsNullOrEmpty(b.Konto))
                    unzugewieseneAusgaben = true;
                else
                    ausgabenKonten.Add(b.Konto);
            }

            if (einnahmenKonten.Count > 0 || unzugewieseneEinnahmen)
            {
                var grpE = new NavigationGruppe { Header = "Einnahmen" };
                foreach (var k in einnahmenKonten.OrderBy(k => k))
                    grpE.Items.Add(new NavigationItem
                    {
                        Text = k,
                        SprungZiel = "E:" + k
                    });
                if (unzugewieseneEinnahmen)
                    grpE.Items.Add(new NavigationItem
                    {
                        Text = "[noch zu keinem Konto zugewiesene Einnahmen]",
                        SprungZiel = "E:"
                    });
                Gruppen.Add(grpE);
            }

            if (ausgabenKonten.Count > 0 || unzugewieseneAusgaben)
            {
                var grpA = new NavigationGruppe { Header = "Ausgaben" };
                foreach (var k in ausgabenKonten.OrderBy(k => k))
                    grpA.Items.Add(new NavigationItem
                    {
                        Text = k,
                        SprungZiel = "A:" + k
                    });
                if (unzugewieseneAusgaben)
                    grpA.Items.Add(new NavigationItem
                    {
                        Text = "[noch zu keinem Konto zugewiesene Ausgaben]",
                        SprungZiel = "A:"
                    });
                Gruppen.Add(grpA);
            }
        }

        // Modus 2: Bestandskonten -> pro Bestandskonto eine Gruppe mit
        // den 12 Monatsitems
        private void BaueGruppenBestandskonten()
        {
            // Bestandskonten sammeln, die in Buchungen vorkommen
            var bestandskonten = new HashSet<string>();
            foreach (var b in _doc.Einnahmen.Concat(_doc.Ausgaben))
            {
                if (!string.IsNullOrEmpty(b.Bestandskonto))
                    bestandskonten.Add(b.Bestandskonto);
            }

            foreach (var bk in bestandskonten.OrderBy(b => b))
            {
                var grp = new NavigationGruppe { Header = bk };
                for (int i = 1; i <= 12; i++)
                {
                    grp.Items.Add(new NavigationItem
                    {
                        Text = cpMonat[i - 1],
                        SprungZiel = $"BK:{bk}:{i}"
                    });
                }
                Gruppen.Add(grp);
            }
        }

        // Routing der Item-Klicks an die passende Scroll-Methode des Journals
        private void NavigiereZu(NavigationItem item)
        {
            if (item == null || string.IsNullOrEmpty(item.SprungZiel)) return;

            var ziel = item.SprungZiel;

            // Bestandskonten: "BK:<Name>:<Monat>"
            if (ziel.StartsWith("BK:"))
            {
                var rest = ziel.Substring(3);
                int letzteDoppel = rest.LastIndexOf(':');
                if (letzteDoppel < 0) return;
                string bk = rest.Substring(0, letzteDoppel);
                if (!int.TryParse(rest.Substring(letzteDoppel + 1), out int monat))
                    return;
                _journal.ScrolleZuBestandskontoMonat(bk, monat);
                return;
            }

            // Datum/Konten: "E:<Monat>" oder "E:<Konto>" oder "A:..."
            if (ziel.Length < 2 || ziel[1] != ':') return;
            bool istEinnahme = ziel[0] == 'E';
            string nutzlast = ziel.Substring(2);

            // Wenn Modus = Datum, ist die Nutzlast der Monat (1-12)
            if (_journal.AktuellerFilter.AnzeigeModus == JournalAnzeigeModus.Datum
                && int.TryParse(nutzlast, out int monatDt)
                && monatDt >= 1 && monatDt <= 12)
            {
                _journal.ScrolleZuMonat(monatDt, istEinnahme);
                return;
            }

            // Sonst: Nutzlast ist Kontoname (kann leer sein = "unzugewiesen")
            _journal.ScrolleZuKonto(nutzlast, istEinnahme);
        }
    }

    // Generische RelayCommand-Variante mit Parameter, falls die normale
    // RelayCommand nur parameterlos ist. (Falls schon vorhanden im Projekt:
    // diese Klasse einfach loeschen.)
    public class RelayCommand<T> : ICommand
    {
        private readonly Action<T> _execute;
        private readonly Predicate<T> _canExecute;

        public RelayCommand(Action<T> execute, Predicate<T> canExecute = null)
        {
            _execute = execute ?? throw new ArgumentNullException(nameof(execute));
            _canExecute = canExecute;
        }

        public bool CanExecute(object parameter)
            => _canExecute == null || _canExecute((T)parameter);
        public void Execute(object parameter) => _execute((T)parameter);
        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }
    }
}
