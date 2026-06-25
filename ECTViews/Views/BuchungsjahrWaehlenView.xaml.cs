using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;
using ECTViews.ViewModels;

namespace ECTViews.Views
{
    /// <summary>
    /// "Buchungsjahr wählen"-Dialog. Wird beim Anlegen eines neuen Dokuments
    /// gezeigt (Migration von ECTIFace/buchungsjahrwaehlen.cpp).
    /// </summary>
    public partial class BuchungsjahrWaehlenView : Window
    {
        public BuchungsjahrWaehlenView()
        {
            InitializeComponent();
        }

        public BuchungsjahrWaehlenView(BuchungsjahrWaehlenViewModel viewModel) : this()
        {
            DataContext = viewModel;
            viewModel.RequestClose += () => Close();
        }

        /// <summary>Doppelklick auf eine Datei löst den Jahreswechsel aus.</summary>
        private void OnDateiDoppelklick(object sender, MouseButtonEventArgs e)
        {
            if (DataContext is BuchungsjahrWaehlenViewModel vm &&
                vm.AusgewaehlteDatei != null)
            {
                vm.FuehreJahreswechselAus();
            }
        }

        /// <summary>Beim Öffnen die vorausgewählte (Vorjahres-)Datei vertikal
        /// in der Liste zentrieren.</summary>
        private void OnWindowLoaded(object sender, RoutedEventArgs e)
        {
            // Verzögert, damit ItemContainer und ScrollViewer-Layout stehen.
            Dispatcher.BeginInvoke(new Action(ZentriereAuswahl),
                DispatcherPriority.Loaded);
        }

        private void ZentriereAuswahl()
        {
            if (!(DataContext is BuchungsjahrWaehlenViewModel vm) ||
                vm.AusgewaehlteDatei == null)
                return;

            // Container realisieren und Layout aktualisieren.
            DateienListe.ScrollIntoView(vm.AusgewaehlteDatei);
            DateienListe.UpdateLayout();

            var sv = FindeScrollViewer(DateienListe);
            var item = DateienListe.ItemContainerGenerator
                .ContainerFromItem(vm.AusgewaehlteDatei) as ListBoxItem;
            if (sv == null || item == null) return;

            // Position des Items relativ zum ScrollViewer-Inhalt. Dank
            // ScrollViewer.CanContentScroll="False" sind alle Offsets in Pixeln.
            GeneralTransform t = item.TransformToAncestor(sv);
            Rect rect = t.TransformBounds(
                new Rect(0, 0, item.ActualWidth, item.ActualHeight));

            double ziel = sv.VerticalOffset + rect.Top
                          - (sv.ViewportHeight - item.ActualHeight) / 2.0;
            if (ziel < 0) ziel = 0;
            sv.ScrollToVerticalOffset(ziel);
        }

        /// <summary>Sucht den ScrollViewer im Visual Tree der ListBox.</summary>
        private static ScrollViewer FindeScrollViewer(DependencyObject root)
        {
            if (root is ScrollViewer sv) return sv;
            int n = VisualTreeHelper.GetChildrenCount(root);
            for (int i = 0; i < n; i++)
            {
                var treffer = FindeScrollViewer(VisualTreeHelper.GetChild(root, i));
                if (treffer != null) return treffer;
            }
            return null;
        }
    }
}
