// TextBoxZwischenablage.cs -- Kopieren und Ausschneiden in WPF-TextBoxen
// ohne OleFlushClipboard
//
// WPF legt markierten Text über OLE in die Zwischenablage: OleSetClipboard,
// danach OleFlushClipboard (damit der Text das Programm überlebt). Auf
// manchen Systemen scheitert der Flush dauerhaft mit 0x800401D0
// (CLIPBRD_E_CANT_OPEN) -- gemessen auf dem Entwicklungsrechner, auch in
// einem reinen WPF-Testprogramm ohne EasyCash. WPF schluckt den Fehler
// (FrameworkCompatibilityPreferences.ShouldThrowOnCopyOrCutFailure = false)
// mit diesen Folgen:
//   * Ausschneiden (Strg+X, Umschalt+Entf, Kontextmenü) bricht nach dem
//     Kopieren ab, bevor die Markierung gelöscht wird: der Text landet in
//     der Zwischenablage, bleibt aber stehen ("kopiert nur").
//   * Kopieren scheint zu klappen, der Text hängt aber nur als lebendes
//     OLE-Objekt an EasyCash und ist nach dem Beenden weg.
//
// Abhilfe: Für TextBox (dazu gehört auch das Editierfeld einer editierbaren
// ComboBox) übernehmen eigene Klassen-Befehlsbindungen Copy und Cut und
// schreiben den Text direkt per Win32 (OpenClipboard/SetClipboardData,
// CF_UNICODETEXT) -- das braucht keinen Flush, der Text gehört sofort
// Windows. Gelöscht wird erst, wenn das Schreiben geklappt hat; über
// TextBox.SelectedText, damit Strg+Z das Ausschneiden rückgängig macht.
//
// Registriert wird per Modul-Initialisierer: er läuft, bevor irgendein Code
// aus ECTViews.dll ausgeführt wird -- egal über welchen Einstieg (Dialog,
// Journal, Einstellungen, Formular ...).

using System;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ECTViews
{
    internal static class TextBoxZwischenablage
    {
        private static int _registriert;

        [System.Runtime.CompilerServices.ModuleInitializer]
        internal static void Registriere()
        {
            if (Interlocked.Exchange(ref _registriert, 1) != 0) return;

            // Klassen-Bindungen der abgeleiteten Klasse TextBox werden vor
            // denen von TextBoxBase (dort hängt WPFs eigene Implementierung)
            // gefunden; e.Handled beendet die Suche.
            CommandManager.RegisterClassCommandBinding(typeof(TextBox),
                new CommandBinding(ApplicationCommands.Copy, OnKopieren, OnKannKopieren));
            CommandManager.RegisterClassCommandBinding(typeof(TextBox),
                new CommandBinding(ApplicationCommands.Cut, OnAusschneiden, OnKannAusschneiden));

            // Die FlowDocument-Viewer (Anleitung) gehen hier NICHT: WPF
            // registriert seine Behandlung auf genau denselben Typen, und die
            // kommt zuerst. Das Hilfefenster fängt Kopieren darum selbst per
            // PreviewExecuted ab (HilfeFenster.xaml.cs).
        }

        private static void OnKannKopieren(object sender, CanExecuteRoutedEventArgs e)
        {
            if (!(sender is TextBox tb)) return;
            e.CanExecute = tb.SelectionLength > 0;
            e.Handled = true;
        }

        private static void OnKannAusschneiden(object sender, CanExecuteRoutedEventArgs e)
        {
            if (!(sender is TextBox tb)) return;
            e.CanExecute = tb.SelectionLength > 0 && !tb.IsReadOnly && tb.IsEnabled;
            e.Handled = true;
        }

        private static void OnKopieren(object sender, ExecutedRoutedEventArgs e)
        {
            if (!(sender is TextBox tb) || tb.SelectionLength == 0) return;
            SchreibeText(tb.SelectedText);
            e.Handled = true;
        }

        private static void OnAusschneiden(object sender, ExecutedRoutedEventArgs e)
        {
            if (!(sender is TextBox tb) || tb.SelectionLength == 0 || tb.IsReadOnly) return;
            e.Handled = true;
            if (SchreibeText(tb.SelectedText))
                tb.SelectedText = "";   // rückgängig machbar (Strg+Z)
        }

        // ------------------------------------------------------------------
        // Win32-Zwischenablage
        // ------------------------------------------------------------------

        private const uint CF_UNICODETEXT = 13;
        private const uint GMEM_MOVEABLE = 0x0002;

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool OpenClipboard(IntPtr hWndNewOwner);
        [DllImport("user32.dll")]
        private static extern bool CloseClipboard();
        [DllImport("user32.dll")]
        private static extern bool EmptyClipboard();
        [DllImport("user32.dll")]
        private static extern IntPtr SetClipboardData(uint uFormat, IntPtr hMem);
        [DllImport("kernel32.dll")]
        private static extern IntPtr GlobalAlloc(uint uFlags, UIntPtr dwBytes);
        [DllImport("kernel32.dll")]
        private static extern IntPtr GlobalLock(IntPtr hMem);
        [DllImport("kernel32.dll")]
        private static extern bool GlobalUnlock(IntPtr hMem);
        [DllImport("kernel32.dll")]
        private static extern IntPtr GlobalFree(IntPtr hMem);

        /// <summary>Legt <paramref name="text"/> als Unicode-Text in die
        /// Zwischenablage (Windows erzeugt CF_TEXT/CF_OEMTEXT selbst).
        /// Ist sie gerade von einem anderen Programm geöffnet, wird kurz
        /// erneut versucht. False, wenn es nicht geklappt hat.</summary>
        internal static bool SchreibeText(string text)
        {
            if (string.IsNullOrEmpty(text)) return false;

            for (int versuch = 0; versuch < 10; versuch++)
            {
                if (versuch > 0) Thread.Sleep(20);
                if (!OpenClipboard(IntPtr.Zero)) continue;
                try
                {
                    if (!EmptyClipboard()) return false;

                    int bytes = (text.Length + 1) * 2;
                    IntPtr h = GlobalAlloc(GMEM_MOVEABLE, (UIntPtr)bytes);
                    if (h == IntPtr.Zero) return false;
                    IntPtr p = GlobalLock(h);
                    if (p == IntPtr.Zero) { GlobalFree(h); return false; }
                    Marshal.Copy(text.ToCharArray(), 0, p, text.Length);
                    Marshal.WriteInt16(p, text.Length * 2, 0);
                    GlobalUnlock(h);

                    // Gehört der Speicher danach Windows, darf er nicht mehr
                    // freigegeben werden -- nur im Fehlerfall.
                    if (SetClipboardData(CF_UNICODETEXT, h) == IntPtr.Zero)
                    {
                        GlobalFree(h);
                        return false;
                    }
                    return true;
                }
                finally
                {
                    CloseClipboard();
                }
            }
            return false;
        }
    }
}

namespace System.Runtime.CompilerServices
{
    /// <summary>In .NET Framework 4.8 nicht enthalten; der Compiler erkennt
    /// das Attribut am Namen (C# 9 Modul-Initialisierer).</summary>
    [AttributeUsage(AttributeTargets.Method, Inherited = false)]
    internal sealed class ModuleInitializerAttribute : Attribute { }
}
