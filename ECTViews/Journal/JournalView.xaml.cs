// JournalView.xaml.cs - Code-Behind
//
// Aufgaben:
//   1. Doppelklick-Routing -> BearbeitenCommand
//   2. SizeChanged-Handler: aktualisiert ViewModel.BelegMaxBreite auf
//      1/4 der ListBox-Breite, sodass die adaptive Belegspalte den
//      4. Teil der Gesamtbreite niemals überschreitet.
//   3. ScrollIntoViewRequest-Event vom ViewModel: scrollt die
//      angeforderte Zeile in den sichtbaren Bereich (wird vom
//      Navigations-Klick ausgelöst).

using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ECTViews.Journal
{
    public partial class JournalView : UserControl
    {
        private JournalViewModel _vmSubscribed;

        // True, während OnMehrfachSelektion die SelectedItems programmatisch
        // umbaut -- verhindert, dass die dabei feuernden Selektions-Events
        // die Gruppen-Expansion erneut anstossen (Endlosschleife).
        private bool _programmatischeSelektion;

        public JournalView()
        {
            InitializeComponent();
            DataContextChanged += OnDataContextChanged;
        }

        private void OnDataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            // Vorherigen Subscriber lösen
            if (_vmSubscribed != null)
            {
                _vmSubscribed.ScrollIntoViewRequest -= OnScrollIntoViewRequest;
                _vmSubscribed.MehrfachSelektionRequest -= OnMehrfachSelektion;
            }

            _vmSubscribed = e.NewValue as JournalViewModel;
            if (_vmSubscribed != null)
            {
                _vmSubscribed.ScrollIntoViewRequest += OnScrollIntoViewRequest;
                _vmSubscribed.MehrfachSelektionRequest += OnMehrfachSelektion;
            }
        }

        /// <summary>
        /// Selektiert mehrere Buchungszeilen auf einmal und scrollt so, dass
        /// moeglichst viele davon sichtbar sind (Buchungsgruppe als Block).
        /// Verwendet die SelectedItems-API direkt (nicht bindbar), was bei
        /// SelectionMode=Extended der robuste Weg für programmatische
        /// Mehrfachauswahl ist.
        /// </summary>
        private void OnMehrfachSelektion(
            System.Collections.Generic.IReadOnlyList<JournalBuchungRow> rows)
        {
            if (rows == null || rows.Count == 0) return;
            // Asynchron, damit die Container nach einem evtl. vorausgegangenen
            // Aktualisiere() schon aufgebaut sind. Während des Umbaus beide
            // Guards setzen: die View-Guard stoppt die Klick-Expansion, die
            // ViewModel-Guard schützt den Mehrfach-Merker vor dem Binding-
            // Echo der SelectedItem-Änderungen (Clear/Add).
            Dispatcher.BeginInvoke(new System.Action(() =>
            {
                var vm = DataContext as JournalViewModel;
                _programmatischeSelektion = true;
                vm?.SetzeSelektionsGuard(true);
                try
                {
                    lstZeilen.SelectedItems.Clear();
                    foreach (var r in rows)
                        lstZeilen.SelectedItems.Add(r);
                    ZeigeBereichVertikal(rows[0], rows[rows.Count - 1]);
                }
                finally
                {
                    _programmatischeSelektion = false;
                    vm?.SetzeSelektionsGuard(false);
                }
            }), System.Windows.Threading.DispatcherPriority.Background);
        }

        /// <summary>
        /// Scrollt so, dass der Bereich von <paramref name="erste"/> bis
        /// <paramref name="letzte"/> moeglichst komplett sichtbar ist:
        /// passt der Block in den Viewport, wird er zentriert; ist er
        /// groesser, wird die erste Zeile (mit kleinem Rand) oben
        /// ausgerichtet. Fallback: letzte Zeile zentrieren.
        /// </summary>
        private void ZeigeBereichVertikal(JournalRow erste, JournalRow letzte)
        {
            if (ReferenceEquals(erste, letzte))
            {
                ZentriereVertikal(erste);
                return;
            }

            var scrollViewer = FindeScrollViewer(lstZeilen);
            if (scrollViewer == null) return;

            double hErste, hLetzte;
            double topErste = AbsoluterOffsetVon(erste, scrollViewer, out hErste);
            double topLetzte = AbsoluterOffsetVon(letzte, scrollViewer, out hLetzte);
            if (double.IsNaN(topErste) || double.IsNaN(topLetzte))
            {
                ZentriereVertikal(letzte);
                return;
            }
            if (topLetzte < topErste)   // Sicherheit bei unerwarteter Reihenfolge
            {
                var t = topErste; topErste = topLetzte; topLetzte = t;
                var h = hErste; hErste = hLetzte; hLetzte = h;
            }

            double blockHoehe = (topLetzte + hLetzte) - topErste;
            double ziel = blockHoehe <= scrollViewer.ViewportHeight
                ? topErste - (scrollViewer.ViewportHeight - blockHoehe) / 2
                : topErste - 8;   // Block groesser als Viewport: Anfang oben

            ziel = System.Math.Max(0,
                System.Math.Min(ziel, scrollViewer.ScrollableHeight));
            scrollViewer.ScrollToVerticalOffset(ziel);
        }

        /// <summary>Absolute Y-Position einer Zeile im Scroll-Inhalt.
        /// Der Container wird via ScrollIntoView erzwungen
        /// (UI-Virtualisierung); NaN wenn keiner erzeugt werden konnte.</summary>
        private double AbsoluterOffsetVon(JournalRow row,
            System.Windows.Controls.ScrollViewer scrollViewer, out double hoehe)
        {
            hoehe = 0;
            lstZeilen.ScrollIntoView(row);
            lstZeilen.UpdateLayout();

            var item = lstZeilen.ItemContainerGenerator
                .ContainerFromItem(row) as System.Windows.Controls.ListBoxItem;
            if (item == null) return double.NaN;

            hoehe = item.ActualHeight;
            var transform = item.TransformToAncestor(scrollViewer);
            return transform.Transform(new System.Windows.Point(0, 0)).Y
                   + scrollViewer.VerticalOffset;
        }

        private void OnScrollIntoViewRequest(JournalRow row)
        {
            if (row == null) return;
            // Asynchron, damit das Layout fertig aufgebaut ist (sonst
            // funktioniert ContainerFromItem nicht zuverlässig).
            Dispatcher.BeginInvoke(new System.Action(() =>
            {
                ZentriereVertikal(row);
            }), System.Windows.Threading.DispatcherPriority.Background);
        }

        /// <summary>
        /// Scrollt die ListBox so, dass die übergebene Zeile vertikal
        /// in der Mitte des sichtbaren Bereichs steht. Falls nötig,
        /// wird ScrollIntoView vorab aufgerufen, um den Container überhaupt
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

        /// <summary>
        /// Fängt Tasten ab, die wir selbst behandeln moechten (PageUp/Down,
        /// Home/End, Up/Down). Der Default-Handler der ListBox scrollt bei
        /// PageUp/Down nur den Viewport ohne die Selektion mitzunehmen, und
        /// bei Up/Down hat er keine Klammerung an die erste/letzte
        /// Buchungs-Zeile (er stoppt vorher in einem JournalSpacerRow oder
        /// JournalSectionTitle). Wir leiten alles auf NavigiereZeile um.
        /// </summary>
        private void OnListBoxPreviewKeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.PageUp:
                case Key.PageDown:
                case Key.Home:
                case Key.End:
                    NavigiereZeile(e.Key);
                    e.Handled = true;
                    break;

                // Zoom mit (Strg-)'+' / '-'. Der alte native Pfad
                // (CEasyCashView::OnKeyDown) bekommt keine Tastatur mehr,
                // sobald die WPF-Liste den Fokus hat -- deshalb hier
                // abfangen und ueber das ViewModel-Event an den nativen
                // Zoom-Mechanismus melden. Wie im Original wird der
                // Strg-Modifier nicht verlangt.
                case Key.Add:
                case Key.OemPlus:
                    (DataContext as JournalViewModel)?.MeldeZoomAenderung(+25);
                    e.Handled = true;
                    break;
                case Key.Subtract:
                case Key.OemMinus:
                    (DataContext as JournalViewModel)?.MeldeZoomAenderung(-25);
                    e.Handled = true;
                    break;

                // Strg+P druckt das Journal. Wie beim Zoom erreicht der
                // MFC-Accelerator die native View nicht mehr, sobald die
                // WPF-Liste den Fokus hat -- deshalb hier abfangen und
                // ueber das ViewModel-Event an den nativen Druckbefehl
                // melden.
                case Key.P:
                    if ((Keyboard.Modifiers & ModifierKeys.Control) != 0)
                    {
                        (DataContext as JournalViewModel)?.MeldeDruckAnforderung();
                        e.Handled = true;
                    }
                    break;
            }
        }

        /// <summary>
        /// Strg-Mausrad zoomt das Journal (analog zu Strg-'+'/'-').
        /// Ohne Strg laeuft das Rad normal als Scroll weiter.
        /// </summary>
        private void OnListBoxPreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            if ((Keyboard.Modifiers & ModifierKeys.Control) == 0) return;
            if (DataContext is JournalViewModel vm && e.Delta != 0)
            {
                vm.MeldeZoomAenderung(e.Delta > 0 ? +25 : -25);
                e.Handled = true;
            }
        }

        /// <summary>
        /// Meldet die komplette Mehrfachauswahl an das ViewModel, das daraus
        /// die Verfuegbarkeit der Kontextmenue-Befehle ableitet.
        /// </summary>
        private void OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (DataContext is JournalViewModel vm)
                vm.SetzeSelektion(lstZeilen.SelectedItems.OfType<JournalBuchungRow>());
        }

        /// <summary>
        /// Füllt vor dem Aufklappen des Kontextmenüs das Submenü
        /// "Umwandeln in" mit den Buchungsgruppen-Vorlagen, die zur
        /// selektierten Buchung passen. Erst hier (statt bei jeder
        /// Selektionsänderung), damit auch in den Einstellungen frisch
        /// angelegte Vorlagen sofort auftauchen.
        /// </summary>
        private void OnKontextmenueOeffnet(object sender, ContextMenuEventArgs e)
        {
            if (DataContext is JournalViewModel vm)
                vm.AktualisiereUmwandelVorlagen();
        }

        /// <summary>
        /// Klick auf ein Buchungsgruppen-Mitglied markiert die ganze Gruppe
        /// (Phase D). Nur bei einfachem Linksklick ohne Modifier -- Ctrl/
        /// Shift erlaubt weiterhin die Einzelauswahl innerhalb einer Gruppe
        /// (z.B. um gezielt EIN Mitglied zu löschen oder zu kopieren).
        /// </summary>
        private void OnZeilenMausKlick(object sender, MouseButtonEventArgs e)
        {
            if (_programmatischeSelektion) return;
            if ((Keyboard.Modifiers & (ModifierKeys.Control | ModifierKeys.Shift)) != 0)
                return;
            if (!(DataContext is JournalViewModel vm)) return;

            // Nur reagieren, wenn wirklich eine Zeile angeklickt wurde --
            // Klicks auf Scrollbar/Leerbereich würden sonst die Ansicht
            // zurück zur selektierten Gruppe springen lassen.
            var element = e.OriginalSource as System.Windows.DependencyObject;
            if (element == null) return;
            var container = ItemsControl.ContainerFromElement(lstZeilen, element)
                as ListBoxItem;
            if (container != null && container.IsSelected
                && container.Content is JournalBuchungRow row)
            {
                vm.SelektiereGruppeVon(row);
            }
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

        /// <summary>
        /// Scrollt das Journal per Tastendruck -- wird aus NavigationView und
        /// CMainFrame::PreTranslateMessage (via ECT_JournalSendKey) aufgerufen.
        /// </summary>
        public void NavigiereZeile(Key key)
        {
            if (!(DataContext is JournalViewModel vm)) return;
            var scroll = FindeScrollViewer(lstZeilen);

            switch (key)
            {
                case Key.Up:
                case Key.Down:
                {
                    var buchungen = vm.Zeilen.OfType<JournalBuchungRow>().ToList();
                    if (buchungen.Count == 0) return;
                    int idx = vm.SelektierteZeile != null
                        ? buchungen.IndexOf(vm.SelektierteZeile) : -1;
                    int neu = key == Key.Up
                        ? Math.Max(0, idx > 0 ? idx - 1 : 0)
                        : Math.Min(buchungen.Count - 1, idx < 0 ? 0 : idx + 1);
                    if (buchungen[neu] != vm.SelektierteZeile)
                    {
                        vm.SelektierteZeile = buchungen[neu];
                        ZentriereVertikal(buchungen[neu]);
                    }
                    break;
                }
                case Key.PageUp:
                case Key.PageDown:
                {
                    var buchungen = vm.Zeilen.OfType<JournalBuchungRow>().ToList();
                    if (buchungen.Count == 0) return;
                    int idx = vm.SelektierteZeile != null
                        ? buchungen.IndexOf(vm.SelektierteZeile) : -1;
                    if (idx < 0) idx = key == Key.PageUp ? buchungen.Count - 1 : 0;

                    int pageSize = BerechnePageSizeBuchungen(buchungen);
                    int neu = key == Key.PageUp
                        ? Math.Max(0, idx - pageSize)
                        : Math.Min(buchungen.Count - 1, idx + pageSize);

                    if (buchungen[neu] != vm.SelektierteZeile)
                    {
                        vm.SelektierteZeile = buchungen[neu];
                        ZentriereVertikal(buchungen[neu]);
                    }
                    break;
                }
                case Key.Home:
                {
                    var buchungen = vm.Zeilen.OfType<JournalBuchungRow>().ToList();
                    if (buchungen.Count > 0)
                    {
                        vm.SelektierteZeile = buchungen[0];
                        ZentriereVertikal(buchungen[0]);
                    }
                    else
                    {
                        scroll?.ScrollToTop();
                    }
                    break;
                }
                case Key.End:
                {
                    var buchungen = vm.Zeilen.OfType<JournalBuchungRow>().ToList();
                    if (buchungen.Count > 0)
                    {
                        var letzte = buchungen[buchungen.Count - 1];
                        vm.SelektierteZeile = letzte;
                        ZentriereVertikal(letzte);
                    }
                    else
                    {
                        scroll?.ScrollToBottom();
                    }
                    break;
                }
            }
        }

        /// <summary>
        /// Schätzt die Anzahl Buchungs-Zeilen, die in den sichtbaren
        /// Listenbereich passen. Basis: durchschnittliche Hoehe der aktuell
        /// realisierten Buchungs-Container (UI-Virtualisierung hält nur die
        /// sichtbaren plus etwas Puffer im Speicher).
        /// </summary>
        private int BerechnePageSizeBuchungen(
            System.Collections.Generic.IList<JournalBuchungRow> buchungen)
        {
            var scroll = FindeScrollViewer(lstZeilen);
            if (scroll == null || scroll.ViewportHeight <= 0 || buchungen.Count == 0)
                return 10;

            double summe = 0;
            int count = 0;
            foreach (var row in buchungen)
            {
                var c = lstZeilen.ItemContainerGenerator.ContainerFromItem(row)
                        as ListBoxItem;
                if (c != null && c.ActualHeight > 0)
                {
                    summe += c.ActualHeight;
                    count++;
                }
            }
            if (count == 0) return 10;

            double avg = summe / count;
            int n = (int)(scroll.ViewportHeight / avg);
            return System.Math.Max(1, n);
        }
    }
}
