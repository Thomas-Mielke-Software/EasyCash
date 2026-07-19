// BerichtView.xaml.cs - Code-Behind der Formlos-Ansicht.
//
// Nur Tastatur-/Maus-Verdrahtung: Strg+P (Druck), (Strg-)'+'/'-' und
// Strg-Mausrad (Zoom) -- gleiche Muster wie JournalView. Die native View
// hat keinen Tastatur-Fokus mehr, sobald das WPF-HWND aktiv ist, deshalb
// werden die Tasten hier abgefangen und über ViewModel-Events an den
// nativen Mechanismus gemeldet.

using System.Windows.Controls;
using System.Windows.Input;

namespace ECTViews.Berichte
{
    public partial class BerichtView : UserControl
    {
        public BerichtView()
        {
            InitializeComponent();
            Loaded += (s, e) => Focus();
        }

        private void OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                // Zoom wie im Journal: Strg-Modifier wird nicht verlangt
                case Key.Add:
                case Key.OemPlus:
                    (DataContext as BerichtViewModel)?.MeldeZoomAenderung(+25);
                    e.Handled = true;
                    break;
                case Key.Subtract:
                case Key.OemMinus:
                    (DataContext as BerichtViewModel)?.MeldeZoomAenderung(-25);
                    e.Handled = true;
                    break;

                case Key.P:
                    if ((Keyboard.Modifiers & ModifierKeys.Control) != 0)
                    {
                        (DataContext as BerichtViewModel)?.MeldeDruckAnforderung();
                        e.Handled = true;
                    }
                    break;
            }
        }

        private void OnPreviewMouseWheel(object sender, MouseWheelEventArgs e)
        {
            if ((Keyboard.Modifiers & ModifierKeys.Control) == 0) return;
            if (DataContext is BerichtViewModel vm && e.Delta != 0)
            {
                vm.MeldeZoomAenderung(e.Delta > 0 ? +25 : -25);
                e.Handled = true;
            }
        }
    }
}
