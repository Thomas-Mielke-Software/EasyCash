// JournalEmbed.cs - WPF-Embedding in einen nativen MFC-Parent (HWND).
//
// Statt eines schwebenden Top-Level-Window legen wir einen HwndSource an,
// dessen Parent ein vom Aufrufer uebergebenes HWND (z.B. der ChildFrame
// oder ein Splitter) ist. Der HwndSource exponiert sein eigenes HWND
// als Handle, das der Aufrufer mit SetWindowPos positionieren kann -
// genau wie das ActiveX-Plugin-Fenster in easycashview.cpp.
//
// Architektur:
//   CChildFrame (MFC HWND, native)
//       |
//       +-- HwndSource (von WPF erzeugtes HWND, Child)
//                |
//                +-- JournalView (UserControl, RootVisual)
//
// Der Aufrufer kontrolliert Position und Groesse via SetWindowPos auf
// das von ECT_JournalEinbetten zurueckgegebene HWND. Beim Schliessen
// (oder Wechsel auf eine andere View-Modus) ruft der Aufrufer
// ECT_JournalAbloesen auf, das den HwndSource freigibt.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Interop;
using ECTEngine;

namespace ECTViews.Journal
{
    public static class JournalEmbed
    {
        // Ein Eintrag pro eingebettetes Journal-Fenster. Wir halten alle
        // aktiven Hosts in einer Liste, sodass AktualisiereAlle() bei
        // Buchungsaenderungen alle Instanzen frisch zeichnen kann.
        private class Eintrag
        {
            public HwndSource Source;
            public JournalView View;
            public JournalViewModel ViewModel;
            public IntPtr HwndKind;     // HWND des HwndSource (= an MFC zurueckgegeben)
            public IntPtr HwndParent;   // HWND des MFC-Parent

            // Navigation, wenn sie zu diesem Journal gehoert
            public HwndSource NavSource;
            public NavigationView NavView;
            public NavigationViewModel NavViewModel;
            public IntPtr NavHwndKind;
        }

        private static readonly List<Eintrag> _aktiveHosts = new List<Eintrag>();

        /// <summary>
        /// Erzeugt ein WPF-Journal als Kindfenster des angegebenen HWND.
        /// Liefert das HWND des HwndSource zurueck, das der Aufrufer mit
        /// SetWindowPos positionieren kann.
        ///
        /// Der ViewModel wird in _aktiveHosts gespeichert, sodass
        /// AktualisiereAlle()/AktualisiereFilter() die Anzeige refreshen
        /// koennen, wenn sich Buchungen oder Filter aendern.
        /// </summary>
        /// <param name="parentHwnd">HWND des MFC-Parents (z.B. ChildFrame).</param>
        /// <param name="x">X-Position relativ zum Parent.</param>
        /// <param name="y">Y-Position relativ zum Parent.</param>
        /// <param name="width">Breite in Pixeln.</param>
        /// <param name="height">Hoehe in Pixeln.</param>
        /// <param name="doc">Das BuchungsDocument, das angezeigt werden soll.</param>
        /// <param name="initialFilter">Startfilter (oder null fuer Defaults).</param>
        /// <returns>HWND des erzeugten WPF-Fensters, IntPtr.Zero bei Fehler.</returns>
        public static IntPtr Einbetten(
            IntPtr parentHwnd,
            int x, int y, int width, int height,
            BuchungsDocument doc,
            JournalFilter initialFilter)
        {
            if (parentHwnd == IntPtr.Zero || doc == null)
                return IntPtr.Zero;

            EnsureWpfApplication();

            // ViewModel mit Listen+Sprites aus dem ViewHost-Cache befuellen
            var vm = new JournalViewModel(doc,
                ViewHost.BetriebeNamen,
                ViewHost.BetriebeIcons,
                ViewHost.BestandskontenNamen,
                ViewHost.BestandskontenIcons,
                ViewHost.SpriteBetriebe,
                ViewHost.SpriteBestandskonten);
            vm.Aktualisiere(initialFilter);

            var view = new JournalView { DataContext = vm };

            // HwndSource-Parameter: WS_CHILD + WS_VISIBLE, damit das
            // Fenster Teil der Eltern-Hierarchie wird und Maus-/Tastatur-
            // Eingaben korrekt empfaengt.
            var hwndParams = new HwndSourceParameters("ECT_Journal")
            {
                ParentWindow = parentHwnd,
                WindowStyle = unchecked((int)0x40000000) | 0x10000000, // WS_CHILD | WS_VISIBLE
                PositionX = x,
                PositionY = y,
                Width = width,
                Height = height
            };

            var source = new HwndSource(hwndParams)
            {
                RootVisual = view
            };

            var eintrag = new Eintrag
            {
                Source = source,
                View = view,
                ViewModel = vm,
                HwndKind = source.Handle,
                HwndParent = parentHwnd
            };
            _aktiveHosts.Add(eintrag);

            return source.Handle;
        }

        /// <summary>
        /// Ein eingebettetes Journal-Fenster wieder aufloesen. Wird
        /// vom MFC-Code aufgerufen, wenn der View-Modus gewechselt wird
        /// oder das Dokument geschlossen wird.
        /// </summary>
        public static void Abloesen(IntPtr hwndKind)
        {
            for (int i = _aktiveHosts.Count - 1; i >= 0; i--)
            {
                if (_aktiveHosts[i].HwndKind == hwndKind)
                {
                    var e = _aktiveHosts[i];
                    _aktiveHosts.RemoveAt(i);
                    try { e.NavSource?.Dispose(); } catch { }
                    try { e.Source.Dispose(); } catch { }
                    return;
                }
            }
        }

        /// <summary>
        /// Loest ALLE eingebetteten Journal-Fenster ab. Wird beim
        /// Anwendungsende oder beim Wechsel zu einem nicht-Journal-View
        /// aufgerufen.
        /// </summary>
        public static void AlleAbloesen()
        {
            foreach (var e in _aktiveHosts)
            {
                try { e.NavSource?.Dispose(); } catch { }
                try { e.Source.Dispose(); } catch { }
            }
            _aktiveHosts.Clear();
        }

        /// <summary>
        /// Aktualisiert ALLE eingebetteten Journals - z.B. wenn eine
        /// Buchung geaendert wurde. Aufrufer braucht das einzelne
        /// HWND nicht zu kennen. Die Navigation wird automatisch
        /// mit-aktualisiert.
        /// </summary>
        public static void AktualisiereAlle(JournalFilter filter = null)
        {
            foreach (var e in _aktiveHosts)
            {
                e.ViewModel.Aktualisiere(filter);
                e.NavViewModel?.Aktualisiere();
            }
        }

        /// <summary>
        /// Setzt nur die Schriftgroesse (Zoom) auf allen aktiven Journals,
        /// ohne die Filter zu beruehren.
        /// </summary>
        public static void SetzeZoom(double schriftgroesse)
        {
            foreach (var e in _aktiveHosts)
            {
                e.ViewModel.Schriftgroesse = schriftgroesse;
            }
        }

        /// <summary>
        /// Bettet einen NavigationView als zweites Kind-HWND ein und
        /// verknuepft ihn mit dem Journal-Eintrag. Beim AktualisiereAlle()
        /// wird die Navigation automatisch mit-aktualisiert.
        ///
        /// Findet das Journal anhand seines HWND und haengt sich daran an.
        /// </summary>
        /// <param name="parentHwnd">HWND des MFC-Parents fuer die Nav-Pane.</param>
        /// <param name="hwndJournal">HWND des bereits eingebetteten Journals
        /// (Rueckgabe von Einbetten).</param>
        public static IntPtr NavigationEinbetten(
            IntPtr parentHwnd, int x, int y, int width, int height,
            IntPtr hwndJournal)
        {
            if (parentHwnd == IntPtr.Zero) return IntPtr.Zero;

            var eintrag = _aktiveHosts.FirstOrDefault(e => e.HwndKind == hwndJournal);
            if (eintrag == null) return IntPtr.Zero;

            EnsureWpfApplication();

            var navVm = new NavigationViewModel(
                eintrag.ViewModel.Doc, eintrag.ViewModel);
            navVm.Aktualisiere();

            var navView = new NavigationView { DataContext = navVm };

            var hwndParams = new HwndSourceParameters("ECT_Navigation")
            {
                ParentWindow = parentHwnd,
                WindowStyle = unchecked((int)0x40000000) | 0x10000000, // WS_CHILD | WS_VISIBLE
                PositionX = x,
                PositionY = y,
                Width = width,
                Height = height
            };
            var navSource = new HwndSource(hwndParams)
            {
                RootVisual = navView
            };

            eintrag.NavSource = navSource;
            eintrag.NavView = navView;
            eintrag.NavViewModel = navVm;
            eintrag.NavHwndKind = navSource.Handle;
            return navSource.Handle;
        }

        /// <summary>
        /// Loest das zu diesem Journal gehoerende Navigations-Fenster ab.
        /// </summary>
        public static void NavigationAbloesen(IntPtr hwndNav)
        {
            foreach (var e in _aktiveHosts)
            {
                if (e.NavHwndKind == hwndNav)
                {
                    try { e.NavSource?.Dispose(); } catch { }
                    e.NavSource = null;
                    e.NavView = null;
                    e.NavViewModel = null;
                    e.NavHwndKind = IntPtr.Zero;
                    return;
                }
            }
        }

        /// <summary>
        /// Liefert das JournalViewModel, das zu einem eingebetteten
        /// Journal-HWND gehoert. Wird vom Bridge-Code verwendet, um
        /// Eventhandler (BuchungBearbeiten etc.) an das ViewModel zu
        /// binden, nachdem ECT_JournalEinbetten aufgerufen wurde.
        /// </summary>
        public static JournalViewModel FindeViewModel(IntPtr hwndKind)
        {
            foreach (var e in _aktiveHosts)
            {
                if (e.HwndKind == hwndKind) return e.ViewModel;
            }
            return null;
        }

        /// <summary>
        /// Liefert die Anzahl der aktuell aktiven Journal-Hosts.
        /// (Fuer Logging/Debugging.)
        /// </summary>
        public static int AktiveAnzahl => _aktiveHosts.Count;

        // ----------------------------------------------------------
        // WPF-Application-Initialisierung
        // ----------------------------------------------------------
        // WPF verlangt, dass eine System.Windows.Application-Instanz
        // existiert, bevor irgendwelche Visual-Trees aufgebaut werden.
        // In einer hosted-WPF-in-MFC-Umgebung muessen wir die selbst
        // anlegen, falls sie noch nicht existiert.

        private static bool _wpfInitialized;

        private static void EnsureWpfApplication()
        {
            if (_wpfInitialized) return;
            if (System.Windows.Application.Current == null)
            {
                // Trick: einfach instanziieren - der Konstruktor
                // setzt Application.Current automatisch.
                new System.Windows.Application();
            }
            _wpfInitialized = true;
        }
    }
}
