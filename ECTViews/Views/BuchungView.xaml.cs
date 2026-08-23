using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Threading;
using ECTViews.ViewModels;

namespace ECTViews.Views
{
    public partial class BuchungView : Window
    {
        // Timer, der den Notiz-Balloon nach ein paar Sekunden wieder schliesst.
        private readonly DispatcherTimer _notizTimer;

        public BuchungView()
        {
            InitializeComponent();

            _notizTimer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(6)
            };
            _notizTimer.Tick += (s, e) =>
            {
                _notizTimer.Stop();
                NotizBalloon.IsOpen = false;
            };

            // Balloon schliessen, wenn das Fenster geschlossen wird.
            Closed += (s, e) => { _notizTimer.Stop(); NotizBalloon.IsOpen = false; };
        }

        public BuchungView(BuchungViewModel viewModel) : this()
        {
            DataContext = viewModel;
            viewModel.RequestClose += () => Close();
            viewModel.PresetNotizAnzeigen += ZeigeNotizBalloon;
            viewModel.RequestFokus += OnRequestFokus;

            // Ad-hoc-Kontoselektor: fehlt das per Feld-Spezifikation
            // ("$de:Formular=Id|...||") geforderte Konto, fragt der
            // Anlage-Dialog nach dem Kontonamen.
            viewModel.KontoAnlegenAbfrage = (bedarf, nameVorschlag) =>
                Stammdaten.KontoAnlegenView.ZeigeDialog(bedarf, owner: this,
                    nameVorschlag: nameVorschlag);

            // DSGVO-Einwilligung vor der ersten Online-Kursabfrage.
            viewModel.ApiEinwilligungAbfrage = WaehrungApiEinwilligung.Sicherstellen;

            // Sicherheitsabfrage, wenn die Wahl einer Vorlage im Bearbeiten-
            // Modus die Buchung in eine Buchungsgruppe umwandelt, eine Gruppe
            // umstellt oder auflöst.
            viewModel.UmwandlungBestaetigen = (text, titel) =>
                MessageBox.Show(this, text, titel,
                    MessageBoxButton.YesNo, MessageBoxImage.Question)
                == MessageBoxResult.Yes;

            // Auch beim erstmaligen Oeffnen die "Weiterbuchen-Verhalten"-
            // Einstellung zum Cursor beachten (nur bei neuer Buchung, nicht
            // beim Bearbeiten/Kopieren).
            Loaded += OnInitialFokus;

            // Ueber das Ribbon-Dropdown vorgewaehlte Buchungsvorlage laden --
            // erst wenn das Fenster steht, damit ein evtl. noetiger
            // "Konto anlegen"-Dialog einen sichtbaren Owner hat.
            Loaded += OnVorwahlLaden;
        }

        /// <summary>Laedt eine ueber das Ribbon-Dropdown vorgewaehlte Vorlage,
        /// sobald das Fenster geladen ist (siehe
        /// <see cref="BuchungViewModel.VorgewaehltesPreset"/>).</summary>
        private void OnVorwahlLaden(object sender, RoutedEventArgs e)
        {
            Loaded -= OnVorwahlLaden;   // nur einmal
            if (DataContext is BuchungViewModel vm)
                vm.LadeVorgewaehlteVorlage();
        }

        /// <summary>Setzt beim Oeffnen des Dialogs den Anfangsfokus gemaess der
        /// Einstellung "Tagesdatum einfügen und Cursor ins Betragsfeld": ins
        /// Betrag- oder sonst ins Tag-Feld -- aber nur bei einer neuen Buchung.</summary>
        private void OnInitialFokus(object sender, RoutedEventArgs e)
        {
            Loaded -= OnInitialFokus;   // nur einmal
            if (DataContext is BuchungViewModel vm && vm.WeiterbuchenSichtbar)
                OnRequestFokus(vm.CursorInsBetragsfeld);
        }

        /// <summary>Setzt nach "Weiterbuchen" den Fokus: auf das Betrag-Feld
        /// (Einstellung "Tagesdatum einfügen und Cursor ins Betragsfeld") oder
        /// sonst auf das Tag-Feld. Verzoegert, damit das Re-Init der Maske
        /// abgeschlossen ist, bevor der Fokus gesetzt wird.</summary>
        private void OnRequestFokus(bool aufBetrag)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                var box = aufBetrag ? BetragBox : TagBox;
                box.Focus();
                box.SelectAll();
            }), DispatcherPriority.Background);
        }

        /// <summary>Selektiert beim Fokus-Erhalt den vorhandenen Feldinhalt
        /// (Datum, Betrag, Beschreibung, Beleg), damit Tippen ihn direkt
        /// ersetzt. Greift nur bei nicht-leerem Feld.</summary>
        private void OnFeldGotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (sender is TextBox box && box.Text.Length > 0)
                box.SelectAll();
        }

        /// <summary>Sorgt dafuer, dass die Vorselektion auch beim Fokussieren
        /// per Mausklick greift: der erste Klick setzt nur den Fokus (WPF
        /// wuerde sonst sofort den Cursor platzieren und die Selektion
        /// aufheben); ein weiterer Klick positioniert dann normal.</summary>
        private void OnFeldPreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is TextBox box && !box.IsKeyboardFocusWithin)
            {
                box.Focus();
                e.Handled = true;
            }
        }

        /// <summary>Dropdown-Knopf neben dem Beschreibungsfeld: klappt die
        /// ungefilterte Vorlagen-Liste auf bzw. wieder zu (Pendant zum
        /// ComboBox-Pfeil des MFC-Originals). Der Knopf ist Focusable=False,
        /// darum bleibt der Fokus im Beschreibungsfeld bzw. wird dorthin
        /// gesetzt -- so schliesst das Popup wie gewohnt ueber dessen
        /// LostFocus, wenn der Anwender woanders hinklickt.</summary>
        private void OnVorschlaegeKnopf(object sender, RoutedEventArgs e)
        {
            if (!(DataContext is BuchungViewModel vm)) return;
            if (vm.VorschlaegeOffen)
            {
                vm.VorschlaegeOffen = false;
            }
            else
            {
                vm.OeffneAlleVorschlaege();
                BeschreibungBox.Focus();
            }
        }

        /// <summary>Schliesst die Vorschlagsliste, wenn das Beschreibungsfeld
        /// den Fokus verliert. Verzoegert, damit ein Klick auf einen Vorschlag
        /// noch verarbeitet wird (Selection -> Preset laden), bevor das Popup
        /// zugeht.</summary>
        private void OnBeschreibungLostFocus(object sender, RoutedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                if (DataContext is BuchungViewModel vm)
                    vm.VorschlaegeOffen = false;
            }), DispatcherPriority.Background);
        }

        /// <summary>Waehrungsrechner-Knopf neben dem Betragsfeld: klappt ein
        /// Menue mit den in den Einstellungen ausgewaehlten Waehrungen auf. Die
        /// Auswahl einer Waehrung startet die (asynchrone) Kursabfrage und
        /// Umrechnung. Baut das Menue programmatisch wie das MFC-Original
        /// (OnBnClickedWaehrungsrechner), gespeist aber aus der Whitelist.</summary>
        private void OnWaehrungsrechnerKnopf(object sender, RoutedEventArgs e)
        {
            if (!(DataContext is BuchungViewModel vm)) return;
            if (!(sender is Button knopf)) return;

            var menue = new ContextMenu
            {
                PlacementTarget = knopf,
                Placement = PlacementMode.Bottom
            };

            var aktive = vm.WaehrungenFuerMenue();
            if (aktive == null || aktive.Count == 0)
            {
                menue.Items.Add(new MenuItem
                {
                    Header = "Keine Währungen ausgewählt – bitte unter "
                             + "Einstellungen › Währungsumrechnung auswählen",
                    IsEnabled = false
                });
            }
            else
            {
                foreach (var w in aktive)
                {
                    string code = w.Code;   // fuer die Closure festhalten
                    var item = new MenuItem { Header = w.Code + "  –  " + w.Name };
                    item.Click += async (s2, e2) => await UmrechnungAusfuehren(vm, code);
                    menue.Items.Add(item);
                }
            }

            menue.IsOpen = true;
        }

        /// <summary>Fuehrt die Umrechnung aus und faengt JEDEN Fehler ab, damit
        /// die Buchungsmaske nicht abstuerzt (Issue #21, Befund Darkwing371):
        /// Netz-/Server-/Eingabefehler landen in einer freundlichen Meldung,
        /// die UI bleibt waehrend des asynchronen Abrufs bedienbar.</summary>
        private async Task UmrechnungAusfuehren(BuchungViewModel vm, string code)
        {
            try
            {
                await vm.WaehrungUmrechnenAsync(code);
            }
            catch (Exception ex)
            {
                try
                {
                    MessageBox.Show(this,
                        "Die Währungsumrechnung ist nicht möglich:\n\n" + ex.Message,
                        "Währungsumrechnung",
                        MessageBoxButton.OK, MessageBoxImage.Warning);
                }
                catch { /* Fenster evtl. schon geschlossen -- ignorieren */ }
            }
        }

        /// <summary>Zeigt die Preset-Notiz als Balloon und (re)startet den
        /// Auto-Schliess-Timer.</summary>
        private void ZeigeNotizBalloon(string notiz)
        {
            if (string.IsNullOrWhiteSpace(notiz)) return;
            NotizText.Text = notiz;
            // Neu oeffnen erzwingt das Repositionieren/Fade-In.
            NotizBalloon.IsOpen = false;
            NotizBalloon.IsOpen = true;
            _notizTimer.Stop();
            _notizTimer.Start();
        }
    }
}
