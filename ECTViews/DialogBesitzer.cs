// DialogBesitzer.cs -- Besitzer-Fenster für modale Dialoge aus gehosteten Views
//
// Das Problem, das diese Klasse an EINER Stelle löst:
//
// Grosse Teile der WPF-Oberfläche laufen nicht in einem WPF-Fenster, sondern
// per HwndSource als Kind des MFC-Rahmens (Journal, Bericht, Formular,
// Einstellungen). In diesen Views liefert Window.GetWindow(this) NULL --
// es gibt schlicht keinen Window-Vorfahren. Wer den Rückgabewert
// ungeprüft als Dialog-Besitzer verwendet, bekommt zwei Fehlerbilder:
//
//   * MessageBox.Show(null, ...) wirft eine ArgumentNullException,
//   * ein Window ohne Owner rutscht hinter das Hauptfenster, sobald
//     dieses den Fokus bekommt.
//
// Der Besitzer muss deshalb das native TOP-LEVEL-Fenster sein. Die
// HwndSource selbst ist ein KIND-Fenster und taugt dafür nicht (Owner kann
// nur ein Top-Level-Fenster sein) -- darum GetAncestor(GA_ROOT).

using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;

namespace ECTViews
{
    /// <summary>
    /// Ermittelt das Besitzer-Fenster für modale Dialoge, die aus einer
    /// (möglicherweise in MFC gehosteten) View heraus geöffnet werden.
    /// </summary>
    public static class DialogBesitzer
    {
        private const uint GA_ROOT = 2;

        [DllImport("user32.dll")]
        private static extern IntPtr GetAncestor(IntPtr hwnd, uint gaFlags);

        /// <summary>
        /// Natives Top-Level-Fenster über der View; IntPtr.Zero, wenn die
        /// View (noch) nicht dargestellt wird.
        /// </summary>
        public static IntPtr Hwnd(Visual view)
        {
            if (view == null) return IntPtr.Zero;
            var quelle = PresentationSource.FromVisual(view) as HwndSource;
            if (quelle == null || quelle.Handle == IntPtr.Zero) return IntPtr.Zero;

            var wurzel = GetAncestor(quelle.Handle, GA_ROOT);
            return wurzel != IntPtr.Zero ? wurzel : quelle.Handle;
        }

        /// <summary>
        /// Setzt den Besitzer eines Dialogs: bevorzugt das WPF-Fenster über
        /// der View, sonst das native Top-Level-Fenster. Beides fehlt nur,
        /// wenn die View gar nicht dargestellt wird -- dann bleibt der Dialog
        /// besitzerlos, statt an einem null-Owner zu scheitern.
        /// </summary>
        public static void Setze(Window dialog, Visual view)
        {
            if (dialog == null) return;

            var wpfBesitzer = view == null ? null : Window.GetWindow(view);
            if (wpfBesitzer != null && !ReferenceEquals(wpfBesitzer, dialog))
            {
                dialog.Owner = wpfBesitzer;
                return;
            }

            var hwnd = Hwnd(view);
            if (hwnd != IntPtr.Zero)
                new WindowInteropHelper(dialog) { Owner = hwnd };
        }
    }
}
