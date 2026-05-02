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
            // Asynchron, damit das Layout fertig aufgebaut ist (sonst
            // funktioniert ContainerFromItem nicht zuverlaessig).
            Dispatcher.BeginInvoke(new System.Action(() =>
            {
                ZentriereVertikal(row);
            }), System.Windows.Threading.DispatcherPriority.Background);
        }

        /// <summary>
        /// Scrollt die ListBox so, dass die uebergebene Zeile vertikal
        /// in der Mitte des sichtbaren Bereichs steht. Falls noetig,
        /// wird ScrollIntoView vorab aufgerufen, um den Container ueberhaupt
        /// erst zu materialisieren (UI-Virtualisierung).
        /// </summary>
        private void ZentriereVertikal(JournalRow row)
        {
            // Schritt 1: Container erzwingen (UI-Virt. baut sonst gar nichts auf)
            lstZeilen.ScrollIntoView(row);
            lstZeilen.UpdateLayout();

            var item = lstZeilen.ItemContainerGenerator
                .ContainerFromItem(row) as System.Windows.Controls.ListBoxItem;
            if (item == null) return;

            var scrollViewer = FindeScrollViewer(lstZeilen);
            if (scrollViewer == null) return;

            // Y-Position des Items relativ zum ScrollViewer-Inhalt
            var transform = item.TransformToAncestor(scrollViewer);
            double itemTop = transform.Transform(new System.Windows.Point(0, 0)).Y
                             + scrollViewer.VerticalOffset;

            double zielOffset = itemTop
                                - scrollViewer.ViewportHeight / 2
                                + item.ActualHeight / 2;

            zielOffset = System.Math.Max(0,
                System.Math.Min(zielOffset, scrollViewer.ScrollableHeight));

            scrollViewer.ScrollToVerticalOffset(zielOffset);
        }

        private static System.Windows.Controls.ScrollViewer FindeScrollViewer(
            System.Windows.DependencyObject d)
        {
            if (d is System.Windows.Controls.ScrollViewer sv) return sv;
            int n = System.Windows.Media.VisualTreeHelper.GetChildrenCount(d);
            for (int i = 0; i < n; i++)
            {
                var child = System.Windows.Media.VisualTreeHelper.GetChild(d, i);
                var found = FindeScrollViewer(child);
                if (found != null) return found;
            }
            return null;
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
