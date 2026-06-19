// EinstellungenEmbed.cs - WPF-Einstellungen als Kindfenster eines nativen
// MFC-Parent (HWND), analog zu JournalEmbed.
//
// Anders als das Journal braucht die Einstellungs-Welt KEINE zweite
// Navigations-Pane: die Navigation ist Teil von EinstellungenView selbst
// (interner GridSplitter). Es gibt also pro Einbettung genau einen
// HwndSource.
//
// Der Aufrufer positioniert das zurückgegebene HWND via SetWindowPos und
// ruft beim Schliessen/View-Wechsel Abloesen() bzw. AlleAbloesen() auf.

using System;
using System.Collections.Generic;
using System.Windows.Interop;

namespace ECTViews.EinstellungenUi
{
    public static class EinstellungenEmbed
    {
        private class Eintrag
        {
            public HwndSource Source;
            public EinstellungenView View;
            public EinstellungenViewModel ViewModel;
            public IntPtr HwndKind;
            public IntPtr HwndParent;
        }

        private static readonly List<Eintrag> _aktiveHosts = new List<Eintrag>();

        /// <summary>
        /// Erzeugt die WPF-Einstellungen als Kindfenster des angegebenen HWND.
        /// Liefert das HWND des HwndSource zurück (vom Aufrufer per
        /// SetWindowPos zu positionieren), oder IntPtr.Zero bei Fehler.
        /// </summary>
        /// <param name="hatDokument">true, wenn ein Buchungsdokument offen ist
        /// (steuert die "Aktuelles Dokument"-Gruppe in der Navigation).</param>
        public static IntPtr Einbetten(
            IntPtr parentHwnd, int x, int y, int width, int height,
            bool hatDokument)
        {
            if (parentHwnd == IntPtr.Zero) return IntPtr.Zero;

            EnsureWpfApplication();

            var vm = new EinstellungenViewModel(hatDokument);
            var view = new EinstellungenView { DataContext = vm };

            var hwndParams = new HwndSourceParameters("ECT_Einstellungen")
            {
                ParentWindow = parentHwnd,
                WindowStyle = unchecked((int)0x40000000) | 0x10000000, // WS_CHILD | WS_VISIBLE
                PositionX = x,
                PositionY = y,
                Width = width,
                Height = height
            };

            var source = new HwndSource(hwndParams) { RootVisual = view };

            _aktiveHosts.Add(new Eintrag
            {
                Source = source,
                View = view,
                ViewModel = vm,
                HwndKind = source.Handle,
                HwndParent = parentHwnd
            });

            return source.Handle;
        }

        public static void Abloesen(IntPtr hwndKind)
        {
            for (int i = _aktiveHosts.Count - 1; i >= 0; i--)
            {
                if (_aktiveHosts[i].HwndKind == hwndKind)
                {
                    var e = _aktiveHosts[i];
                    _aktiveHosts.RemoveAt(i);
                    try { e.Source.Dispose(); } catch { }
                    return;
                }
            }
        }

        public static void AlleAbloesen()
        {
            foreach (var e in _aktiveHosts)
            {
                try { e.Source.Dispose(); } catch { }
            }
            _aktiveHosts.Clear();
        }

        public static int AktiveAnzahl => _aktiveHosts.Count;

        // ----------------------------------------------------------
        // WPF-Application-Initialisierung (wie JournalEmbed)
        // ----------------------------------------------------------
        private static bool _wpfInitialized;

        private static void EnsureWpfApplication()
        {
            if (_wpfInitialized) return;
            if (System.Windows.Application.Current == null)
                new System.Windows.Application();
            System.Windows.Application.Current.ShutdownMode =
                System.Windows.ShutdownMode.OnExplicitShutdown;
            _wpfInitialized = true;
        }
    }
}
