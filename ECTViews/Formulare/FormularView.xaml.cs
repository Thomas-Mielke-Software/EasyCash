// FormularView.xaml.cs - Code-Behind der Formular-Ansicht.
//
// Tastatur-/Maus-Verdrahtung wie BerichtView (Strg+P, (Strg-)'+'/'-',
// Strg-Mausrad -> ViewModel-Events -> Bridge-Callbacks) plus die
// Abschnitts-/Seiten-Navigation (Klick scrollt an die vorberechnete
// Position, Pendant m_cuiaScrollPos) und der Feld-DESIGNER:
//
//   - Kontextmenue "Felder anzeigen/bearbeiten" schaltet den Modus
//     (Pendant m_bFormularfelderAnzeigen, easycashview.cpp:7497).
//   - Linke Maustaste zieht das Feld unterm Cursor (Strg-Klick =
//     Mehrfachauswahl; nativ uebernahm das die mittlere Maustaste).
//     Live-Vorschau ueber RenderTransform, beim Loslassen
//     Promille-Delta -> .ecf (Pendant MoveFormularfeld,
//     easycashview.cpp:7839ff).
//   - Pfeiltasten: Feinjustage um 1 Promille (Shift: 10).
//   - Doppelklick/Kontextmenue: Eigenschaften-Dialog (FormularFeldDialog).

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using ECTEngine;

namespace ECTViews.Formulare
{
    public partial class FormularView : UserControl
    {
        public FormularView()
        {
            InitializeComponent();
            Loaded += (s, e) => Focus();
        }

        private FormularViewModel Vm => DataContext as FormularViewModel;

        // ----------------------------------------------------------
        // Tastatur + Mausrad (Zoom/Druck wie BerichtView; Pfeiltasten
        // = Designer-Feinjustage)
        // ----------------------------------------------------------

        private void OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            var vm = Vm;

            // Designer: Pfeiltasten verschieben die Selektion in Promille
            if (vm != null && vm.DesignerAktiv && vm.Selektion.Count > 0)
            {
                int schritt = (Keyboard.Modifiers & ModifierKeys.Shift) != 0 ? 10 : 1;
                switch (e.Key)
                {
                    case Key.Left: vm.VerschiebeSelektion(-schritt, 0); e.Handled = true; return;
                    case Key.Right: vm.VerschiebeSelektion(+schritt, 0); e.Handled = true; return;
                    case Key.Up: vm.VerschiebeSelektion(0, -schritt); e.Handled = true; return;
                    case Key.Down: vm.VerschiebeSelektion(0, +schritt); e.Handled = true; return;
                    case Key.Escape: vm.LeereSelektion(); e.Handled = true; return;
                }
            }

            switch (e.Key)
            {
                // Zoom wie im Journal: Strg-Modifier wird nicht verlangt
                case Key.Add:
                case Key.OemPlus:
                    vm?.MeldeZoomAenderung(+25);
                    e.Handled = true;
                    break;
                case Key.Subtract:
                case Key.OemMinus:
                    vm?.MeldeZoomAenderung(-25);
                    e.Handled = true;
                    break;

                case Key.P:
                    if ((Keyboard.Modifiers & ModifierKeys.Control) != 0)
                    {
                        vm?.MeldeDruckAnforderung();
                        e.Handled = true;
                    }
                    break;
            }
        }

        private void OnPreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            if ((Keyboard.Modifiers & ModifierKeys.Control) == 0) return;
            if (Vm != null && e.Delta != 0)
            {
                Vm.MeldeZoomAenderung(e.Delta > 0 ? +25 : -25);
                e.Handled = true;
            }
        }

        // ----------------------------------------------------------
        // Navigations-Leiste
        // ----------------------------------------------------------

        private void OnNavigationClick(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count == 0) return;
            var item = e.AddedItems[0] as FormularNavigationItem;
            var vm = Vm;
            if (item == null || vm?.Definition == null) return;

            // Ziel-Offset in SKALIERTEN DIP: Seiten stapeln sich mit
            // SeitenAbstand (+2 = Seitenrahmen).
            double hoehe = FormularLayout.SeitenHoehe(vm.Definition.Querformat);
            double seitenOffset = (item.Seite - 1)
                * (hoehe + FormularLayout.SeitenAbstand + 2);
            double innerhalb = item.IstSeite
                ? 0
                : FormularLayout.YTextMitte(item.Vertikal, hoehe) - 40;
            if (innerhalb < 0) innerhalb = 0;

            SeitenScroller.ScrollToVerticalOffset(
                (seitenOffset + innerhalb) * vm.Skala);
        }

        private void OnNavKontextmenuOeffnet(object sender, RoutedEventArgs e)
        {
            var vm = Vm;
            var item = NavigationListe.SelectedItem as FormularNavigationItem;
            bool abschnitt = vm != null && vm.DesignerAktiv
                && item?.AbschnittDef != null;
            MenuAbschnittBearbeiten.IsEnabled = abschnitt;
            MenuAbschnittLoeschen.IsEnabled = abschnitt;
        }

        private void OnAbschnittBearbeiten(object sender, RoutedEventArgs e)
        {
            var vm = Vm;
            var item = NavigationListe.SelectedItem as FormularNavigationItem;
            if (vm == null || item?.AbschnittDef == null) return;

            var dlg = new FormularAbschnittDialog(item.AbschnittDef);
            SetzeOwner(dlg);
            if (dlg.ShowDialog() == true)
                vm.SpeichereAbschnitt(item.AbschnittDef);
        }

        private void OnAbschnittLoeschen(object sender, RoutedEventArgs e)
        {
            var vm = Vm;
            var item = NavigationListe.SelectedItem as FormularNavigationItem;
            if (vm == null || item?.AbschnittDef == null) return;

            if (MessageBox.Show(
                    "Abschnitt '" + item.AbschnittDef.Name + "' wirklich löschen?",
                    "EasyCash&Tax", MessageBoxButton.YesNo,
                    MessageBoxImage.Question) == MessageBoxResult.Yes)
                vm.LoescheAbschnitt(item.AbschnittDef);
        }

        // ----------------------------------------------------------
        // Designer: Hit-Testing + Drag
        // ----------------------------------------------------------

        // Kontext des letzten Rechtsklicks (fuer "hier erzeugen"/"Feld
        // bearbeiten" im Kontextmenue)
        private FormularFeldVm _kontextFeld;
        private FormularSeiteVm _kontextSeite;
        private int _kontextHorizontal, _kontextVertikal;

        // Drag-Zustand
        private bool _dragAktiv;
        private bool _dragBewegt;
        private Point _dragStart;                     // in Seiten-DIP
        private Grid _dragSeitenGrid;
        private FormularSeiteVm _dragSeite;
        private readonly List<(FormularFeldVm Vm, UIElement Container)> _dragElemente
            = new List<(FormularFeldVm, UIElement)>();

        private static FormularFeldVm FeldVmVon(object quelle)
        {
            var fe = quelle as FrameworkElement;
            return fe?.DataContext as FormularFeldVm;
        }

        private static T SucheElter<T>(DependencyObject start) where T : DependencyObject
        {
            while (start != null && !(start is T))
                start = VisualTreeHelper.GetParent(start);
            return start as T;
        }

        private void OnSeiteMouseDown(object sender, MouseButtonEventArgs e)
        {
            var vm = Vm;
            var seitenGrid = sender as Grid;
            var seiteVm = seitenGrid?.DataContext as FormularSeiteVm;
            if (vm == null || seiteVm == null) return;

            Focus();
            if (!vm.DesignerAktiv) return;

            var feldVm = FeldVmVon(e.OriginalSource);
            bool strg = (Keyboard.Modifiers & ModifierKeys.Control) != 0;

            if (feldVm == null)
            {
                if (!strg) vm.LeereSelektion();
                return;
            }

            // Doppelklick: Eigenschaften-Dialog
            if (e.ClickCount == 2)
            {
                BearbeiteFeld(feldVm.FeldDef);
                e.Handled = true;
                return;
            }

            // Selektion (Strg = Mehrfachauswahl, nativ mittlere Maustaste)
            if (strg)
                feldVm.IstSelektiert = !feldVm.IstSelektiert;
            else if (!feldVm.IstSelektiert)
            {
                vm.LeereSelektion();
                feldVm.IstSelektiert = true;
            }

            // Drag vorbereiten: alle selektierten Felder DIESER Seite
            _dragElemente.Clear();
            var itemsControl = SucheElter<ItemsControl>(e.OriginalSource as DependencyObject);
            if (itemsControl != null)
            {
                foreach (var f in seiteVm.Felder)
                {
                    if (!f.IstSelektiert) continue;
                    var container = itemsControl.ItemContainerGenerator
                        .ContainerFromItem(f) as UIElement;
                    if (container != null)
                        _dragElemente.Add((f, container));
                }
            }
            if (_dragElemente.Count > 0)
            {
                _dragAktiv = true;
                _dragBewegt = false;
                _dragStart = e.GetPosition(seitenGrid);
                _dragSeitenGrid = seitenGrid;
                _dragSeite = seiteVm;
                seitenGrid.CaptureMouse();
            }
            e.Handled = true;
        }

        private void OnSeiteMouseMove(object sender, MouseEventArgs e)
        {
            if (!_dragAktiv || _dragSeitenGrid == null) return;
            var delta = e.GetPosition(_dragSeitenGrid) - _dragStart;
            if (!_dragBewegt
                && Math.Abs(delta.X) < 2 && Math.Abs(delta.Y) < 2) return;
            _dragBewegt = true;

            // Live-Vorschau ueber RenderTransform (billig; die echten
            // Positionen werden erst beim Loslassen geschrieben)
            foreach (var (_, container) in _dragElemente)
                container.RenderTransform = new TranslateTransform(delta.X, delta.Y);
        }

        private void OnSeiteMouseUp(object sender, MouseButtonEventArgs e)
        {
            if (!_dragAktiv) return;
            _dragAktiv = false;
            _dragSeitenGrid?.ReleaseMouseCapture();

            var vm = Vm;
            if (vm?.Definition == null || _dragSeitenGrid == null) return;

            var delta = e.GetPosition(_dragSeitenGrid) - _dragStart;
            foreach (var (_, container) in _dragElemente)
                container.RenderTransform = null;

            if (!_dragBewegt) return;   // nur Klick, kein Drag

            // DIP-Delta -> Promille (Pendant der nativen Rueckrechnung)
            double breite = FormularLayout.SeitenBreite(vm.Definition.Querformat);
            double hoehe = FormularLayout.SeitenHoehe(vm.Definition.Querformat);
            int deltaH = (int)Math.Round(delta.X / breite * 1000.0);
            int deltaV = (int)Math.Round(delta.Y / hoehe * 1414.0);
            if (deltaH != 0 || deltaV != 0)
                vm.VerschiebeSelektion(deltaH, deltaV);

            _dragSeitenGrid = null;
            _dragSeite = null;
            _dragElemente.Clear();
        }

        private void OnSeiteRechtsklick(object sender, MouseButtonEventArgs e)
        {
            // Kontext fuer das Kontextmenue merken (Feld unterm Cursor +
            // Klickposition in Promille)
            var seitenGrid = sender as Grid;
            var seiteVm = seitenGrid?.DataContext as FormularSeiteVm;
            _kontextFeld = FeldVmVon(e.OriginalSource);
            _kontextSeite = seiteVm;
            if (seitenGrid != null && seiteVm != null)
            {
                var p = e.GetPosition(seitenGrid);
                _kontextHorizontal = (int)Math.Round(p.X / seiteVm.Breite * 1000.0);
                _kontextVertikal = (int)Math.Round(p.Y / seiteVm.Hoehe * 1414.0);
            }
        }

        // ----------------------------------------------------------
        // Kontextmenue
        // ----------------------------------------------------------

        private void OnKontextmenuOeffnet(object sender, RoutedEventArgs e)
        {
            var vm = Vm;
            bool designer = vm != null && vm.DesignerAktiv;
            bool feld = designer && _kontextFeld != null;
            MenuDesigner.IsEnabled = vm?.Definition != null;
            MenuFeldBearbeiten.IsEnabled = feld;
            MenuFeldLoeschen.IsEnabled = feld;
            MenuNeuesFeld.IsEnabled = designer && _kontextSeite != null;
            MenuNeuerAbschnitt.IsEnabled = designer && _kontextSeite != null;
            MenuFeldwertKopieren.IsEnabled = _kontextFeld != null;
        }

        private void SetzeOwner(Window dlg)
        {
            // Die View haengt per HwndSource in einem nativen Parent --
            // WPF-Owner gibt es nicht, also das native Top-Level-Fenster.
            // Wichtig: die HwndSource selbst ist ein KIND-Fenster und taugt
            // als Owner nicht; DialogBesitzer geht darum ueber GA_ROOT hoch.
            DialogBesitzer.Setze(dlg, this);
        }

        private void BearbeiteFeld(FormularFeldDef feld)
        {
            var vm = Vm;
            if (vm == null || feld == null) return;
            var dlg = new FormularFeldDialog(feld);
            SetzeOwner(dlg);
            if (dlg.ShowDialog() == true)
                vm.SpeichereFeld(feld);
        }

        private void OnFeldBearbeiten(object sender, RoutedEventArgs e)
        {
            if (_kontextFeld != null)
                BearbeiteFeld(_kontextFeld.FeldDef);
        }

        private void OnFeldLoeschen(object sender, RoutedEventArgs e)
        {
            var vm = Vm;
            if (vm == null || _kontextFeld == null) return;
            var feld = _kontextFeld.FeldDef;
            if (MessageBox.Show(
                    "Feld " + feld.Id + " ('" + feld.Name + "') wirklich löschen?",
                    "EasyCash&Tax", MessageBoxButton.YesNo,
                    MessageBoxImage.Question) == MessageBoxResult.Yes)
                vm.LoescheFeld(feld);
            _kontextFeld = null;
        }

        private void OnNeuesFeld(object sender, RoutedEventArgs e)
        {
            var vm = Vm;
            if (vm == null || _kontextSeite == null) return;
            var feld = vm.NeuesFeldAn(_kontextSeite.Nummer,
                _kontextHorizontal, _kontextVertikal);
            if (feld != null)
                BearbeiteFeld(feld);
        }

        private void OnNeuerAbschnitt(object sender, RoutedEventArgs e)
        {
            var vm = Vm;
            if (vm == null || _kontextSeite == null) return;

            var abschnitt = new FormularAbschnittDef
            {
                Name = "Neuer Abschnitt",
                Seite = _kontextSeite.Nummer,
                Vertikal = _kontextVertikal
            };
            var dlg = new FormularAbschnittDialog(abschnitt);
            SetzeOwner(dlg);
            if (dlg.ShowDialog() == true)
                vm.NeuerAbschnittAn(abschnitt.Name, abschnitt.Seite, abschnitt.Vertikal);
        }

        private void OnFeldwertKopieren(object sender, RoutedEventArgs e)
        {
            if (_kontextFeld == null) return;
            try { Clipboard.SetText(_kontextFeld.Text ?? ""); }
            catch { /* Zwischenablage belegt -- ignorieren */ }
        }

        private void OnDateiOeffnen(object sender, RoutedEventArgs e)
        {
            var pfad = Vm?.Definition?.Quellpfad;
            if (string.IsNullOrEmpty(pfad)) return;
            try
            {
                System.Diagnostics.Process.Start("notepad.exe", "\"" + pfad + "\"");
            }
            catch { }
        }
    }
}
