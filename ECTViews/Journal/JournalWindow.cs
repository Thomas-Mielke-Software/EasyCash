// JournalWindow.cs — Window-Wrapper für JournalView, plus statische
// Host-Methoden analog zu BuchungView, damit das Journal aus C++/CLI
// einfach geöffnet werden kann.

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using ECTEngine;

namespace ECTViews.Journal
{
    public partial class JournalWindow : Window
    {
        private JournalView _view;
        private JournalViewModel _vm;

        public JournalWindow(JournalViewModel vm)
        {
            Title = "Buchungsjournal";
            Width = 1200;
            Height = 800;
            MinWidth = 800;
            MinHeight = 500;
            WindowStartupLocation = WindowStartupLocation.CenterOwner;
            Background = System.Windows.Media.Brushes.White;

            _vm = vm;
            _view = new JournalView { DataContext = vm };
            Content = _view;
        }

        /// <summary>Liefert das ViewModel — der Aufrufer kann Aktualisiere() aufrufen.</summary>
        public JournalViewModel ViewModel => _vm;
    }

    /// <summary>
    /// Statische Host-Methoden für die Integration aus C++/CLI.
    /// </summary>
    public static class JournalHost
    {
        private static JournalWindow _aktuellesFenster;

        /// <summary>
        /// Öffnet das Journal-Fenster (modeless, damit der User
        /// parallel den Buchungsdialog öffnen kann).
        /// </summary>
        public static JournalViewModel ZeigeJournal(
            BuchungsDocument doc, IntPtr ownerHwnd = default)
        {
            var vm = new JournalViewModel(doc,
                ViewHost.BetriebeNamen,
                ViewHost.BetriebeIcons,
                ViewHost.BestandskontenNamen,
                ViewHost.BestandskontenIcons,
                ViewHost.SpriteBetriebe,
                ViewHost.SpriteBestandskonten);
            vm.Aktualisiere();

            var window = new JournalWindow(vm);

            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(window) { Owner = ownerHwnd };

            window.Closed += (s, e) =>
            {
                if (_aktuellesFenster == window) _aktuellesFenster = null;
            };

            _aktuellesFenster = window;
            window.Show();
            return vm;
        }

        /// <summary>
        /// Aktualisiert das offene Journal-Fenster (falls eines existiert)
        /// nach Buchungsänderungen. Optional kann ein neuer Filter übergeben
        /// werden.
        /// </summary>
        public static void AktualisiereOffenesJournal(JournalFilter filter = null)
        {
            _aktuellesFenster?.ViewModel?.Aktualisiere(filter);
        }
    }
}
