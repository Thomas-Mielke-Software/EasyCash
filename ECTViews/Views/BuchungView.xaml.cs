using System;
using System.Windows;
using System.Windows.Controls;
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
            viewModel.KontoAnlegenAbfrage = bedarf =>
                Stammdaten.KontoAnlegenView.ZeigeDialog(bedarf, owner: this);

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
