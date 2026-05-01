// JournalView.xaml.cs - Code-Behind
//
// Aufgaben:
//   1. Doppelklick-Routing -> BearbeitenCommand
//   2. SizeChanged-Handler: aktualisiert ViewModel.BelegMaxBreite auf
//      1/4 der ListBox-Breite, sodass die adaptive Belegspalte den
//      4. Teil der Gesamtbreite niemals ueberschreitet.
//   3. ScrollIntoViewRequest-Event vom ViewModel: scrollt die
//      angeforderte Zeile in den sichtbaren Bereich (wird vom
//      Navigations-Klick ausgeloest).

using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ECTViews.Journal
{
    public partial class JournalView : UserControl
    {
        private JournalViewModel _vmSubscribed;

        public JournalView()
        {
            InitializeComponent();
            DataContextChanged += OnDataContextChanged;
        }

        private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            // Vorherigen Subscriber lösen
            if (_vmSubscribed != null)
                _vmSubscribed.ScrollIntoViewRequest -= OnScrollIntoViewRequest;

            _vmSubscribed = e.NewValue as JournalViewModel;
            if (_vmSubscribed != null)
                _vmSubscribed.ScrollIntoViewRequest += OnScrollIntoViewRequest;
        }

        private void OnScrollIntoViewRequest(JournalRow row)
        {
            if (row == null) return;
            // ScrollIntoView funktioniert nur, nachdem das Layout
            // aufgebaut wurde. In Edge-Cases (z.B. direkt nach
            // Aktualisiere()) verzögert sich das auf den nächsten
            // Dispatcher-Zyklus.
            Dispatcher.BeginInvoke(new System.Action(() =>
            {
                lstZeilen.ScrollIntoView(row);
            }), System.Windows.Threading.DispatcherPriority.Background);
        }

        private void OnZeilenDoppelklick(object sender, MouseButtonEventArgs e)
        {
            if (DataContext is JournalViewModel vm
                && vm.SelektierteZeile != null
                && vm.BearbeitenCommand.CanExecute(null))
            {
                vm.BearbeitenCommand.Execute(null);
                e.Handled = true;
            }
        }

        private void OnControlSizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (!(DataContext is JournalViewModel vm)) return;

            // Belegspalte darf maximal 1/4 der Listenbreite belegen.
            double listWidth = lstZeilen.ActualWidth;
            if (listWidth <= 0) listWidth = ActualWidth;
            double max = System.Math.Max(60, listWidth / 4);
            vm.BelegMaxBreite = max;
        }
    }
}
