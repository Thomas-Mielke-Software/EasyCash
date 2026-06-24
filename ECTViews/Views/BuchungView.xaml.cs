using System;
using System.Windows;
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

            // Auch beim erstmaligen Oeffnen die "Weiterbuchen-Verhalten"-
            // Einstellung zum Cursor beachten (nur bei neuer Buchung, nicht
            // beim Bearbeiten/Kopieren).
            Loaded += OnInitialFokus;
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
