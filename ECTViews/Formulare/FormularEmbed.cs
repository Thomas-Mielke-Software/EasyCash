// FormularEmbed.cs - Hosting der WPF-Formular-Ansicht in einem nativen
// MFC-Parent (HwndSource, Muster BerichtEmbed).
//
// Die Formular-Ansicht ueberlagert als Vollflaeche den Splitter-Bereich
// (Journal + Navigation werden vom nativen Aufrufer per SW_HIDE
// versteckt). Das gewaehlte Formular kommt als .ecf-Pfad + Betriebsfilter
// aus m_csaFormulare/m_csaFormularfilter; Formular-Wechsel bei offener
// Ansicht laufen ueber WechsleFormular (kein Re-Embedding). Gedruckt wird
// exakt die angezeigte Werte-Liste (WYSIWYG, FormularDruckBauer).

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Interop;
using ECTEngine;

namespace ECTViews.Formulare
{
    public static class FormularEmbed
    {
        private class Eintrag
        {
            public HwndSource Source;
            public FormularView View;
            public FormularViewModel ViewModel;
            public IntPtr HwndKind;
            public IntPtr HwndParent;
        }

        private static readonly List<Eintrag> _aktiveHosts = new List<Eintrag>();

        public static IntPtr Einbetten(
            IntPtr parentHwnd,
            int x, int y, int width, int height,
            BuchungsDocument doc,
            string ecfPfad, string betriebFilter,
            int zoomProzent)
        {
            if (parentHwnd == IntPtr.Zero || doc == null)
                return IntPtr.Zero;

            EnsureWpfApplication();

            var vm = new FormularViewModel();
            vm.Initialisiere(doc, ecfPfad, betriebFilter, zoomProzent);

            var view = new FormularView { DataContext = vm };

            var hwndParams = new HwndSourceParameters("ECT_Formular")
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
                try { e.Source.Dispose(); } catch { }
            _aktiveHosts.Clear();
        }

        /// <summary>Rechnet alle aktiven Formular-Ansichten neu
        /// (Buchungs-/Einstellungs-Aenderung).</summary>
        public static void AktualisiereAlle()
        {
            foreach (var e in _aktiveHosts)
                e.ViewModel.Aktualisiere();
        }

        /// <summary>Wechselt das Formular der aktiven Ansicht(en)
        /// (Ribbon-Menue bei bereits offener Formular-Ansicht).</summary>
        public static void WechsleFormular(string ecfPfad, string betriebFilter)
        {
            foreach (var e in _aktiveHosts)
                e.ViewModel.WechsleFormular(ecfPfad, betriebFilter);
        }

        /// <summary>Setzt den Zoomfaktor (Prozent, wie m_zoomfaktor).</summary>
        public static void SetzeZoom(int zoomProzent)
        {
            foreach (var e in _aktiveHosts)
                e.ViewModel.ZoomProzent = zoomProzent;
        }

        /// <summary>Liefert das ViewModel zu einem eingebetteten HWND
        /// (fuer die Event-Verdrahtung in der Bridge).</summary>
        public static FormularViewModel FindeViewModel(IntPtr hwndKind) =>
            _aktiveHosts.FirstOrDefault(e => e.HwndKind == hwndKind)?.ViewModel;

        public static int AktiveAnzahl => _aktiveHosts.Count;

        /// <summary>
        /// Druckt das aktive (erste) Formular. vorschau=true oeffnet die
        /// Seitenansicht. Liefert false, wenn keine Formular-Ansicht aktiv
        /// oder kein Formular geladen ist.
        /// </summary>
        public static bool DruckeAktives(bool vorschau)
        {
            if (_aktiveHosts.Count == 0) return false;
            var eintrag = _aktiveHosts[0];
            var vm = eintrag.ViewModel;
            if (vm.Definition == null || vm.LetzteWerte == null) return false;

            var dokument = Druck.FormularDruckBauer.Baue(
                vm.Definition, vm.LetzteWerte);
            if (dokument == null) return false;

            string titel = vm.Titel;
            if (vorschau)
                Druck.DruckDokument.ZeigeVorschau(dokument, titel, eintrag.HwndParent);
            else
                Druck.DruckDokument.Drucke(dokument, titel, vm.Definition.Querformat);
            return true;
        }

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
