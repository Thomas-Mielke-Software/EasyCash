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
