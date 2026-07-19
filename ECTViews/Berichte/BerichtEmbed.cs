// BerichtEmbed.cs - Hosting der Formlos-Ansicht in einem nativen
// MFC-Parent (HwndSource, Muster JournalEmbed/EinstellungenEmbed).
//
// Die Formlos-Ansicht überlagert als Vollfläche den Splitter-Bereich
// (Journal + Navigation werden vom nativen Aufrufer per SW_HIDE
// versteckt). Filter-Änderungen am Ribbon erreichen sie über
// AktualisiereAlle; gedruckt wird der aktuell angezeigte Bericht
// (WYSIWYG, gleiche Bericht-Instanz).

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Interop;
using ECTEngine;

namespace ECTViews.Berichte
{
    public static class BerichtEmbed
    {
        private class Eintrag
        {
            public HwndSource Source;
            public BerichtView View;
            public BerichtViewModel ViewModel;
            public IntPtr HwndKind;
            public IntPtr HwndParent;
        }

        private static readonly List<Eintrag> _aktiveHosts = new List<Eintrag>();

        public static IntPtr Einbetten(
            IntPtr parentHwnd,
            int x, int y, int width, int height,
            BuchungsDocument doc,
            BerichtTyp typ,
            int monatsfilter, string betriebFilter,
            double schriftgroesse)
        {
            if (parentHwnd == IntPtr.Zero || doc == null)
                return IntPtr.Zero;

            EnsureWpfApplication();

            var vm = new BerichtViewModel();
            vm.Initialisiere(doc, typ);
            vm.Aktualisiere(monatsfilter, betriebFilter,
                schriftgroesse > 0 ? schriftgroesse : 13.0);

            var view = new BerichtView { DataContext = vm };

            var hwndParams = new HwndSourceParameters("ECT_Bericht")
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

        /// <summary>
        /// Baut alle aktiven Berichte mit den aktuellen Ribbon-Filtern neu
        /// (Filter-Änderung oder Buchungs-Änderung).
        /// schriftgroesse &lt;= 0 = beibehalten.
        /// </summary>
        public static void AktualisiereAlle(int monatsfilter,
            string betriebFilter, double schriftgroesse = 0)
        {
            foreach (var e in _aktiveHosts)
                e.ViewModel.Aktualisiere(monatsfilter, betriebFilter, schriftgroesse);
        }

        /// <summary>Wechselt den Berichtstyp der aktiven Ansicht
        /// (Formlos-Menü bei bereits offener Formlos-Ansicht).</summary>
        public static void WechsleTyp(BerichtTyp typ)
        {
            foreach (var e in _aktiveHosts)
                e.ViewModel.WechsleTyp(typ);
        }

        /// <summary>Setzt nur den Zoom (Schriftgröße) auf allen Berichten.</summary>
        public static void SetzeZoom(double schriftgroesse)
        {
            foreach (var e in _aktiveHosts)
                e.ViewModel.Schriftgroesse = schriftgroesse;
        }

        /// <summary>Liefert das ViewModel zu einem eingebetteten HWND
        /// (für die Event-Verdrahtung in der Bridge).</summary>
        public static BerichtViewModel FindeViewModel(IntPtr hwndKind) =>
            _aktiveHosts.FirstOrDefault(e => e.HwndKind == hwndKind)?.ViewModel;

        public static int AktiveAnzahl => _aktiveHosts.Count;

        /// <summary>
        /// Druckt den aktiven (ersten) Bericht. vorschau=true öffnet die
        /// Seitenansicht. Liefert false, wenn keine Formlos-Ansicht aktiv ist.
        /// </summary>
        public static bool DruckeAktives(bool vorschau)
        {
            if (_aktiveHosts.Count == 0) return false;
            var eintrag = _aktiveHosts[0];
            var bericht = eintrag.ViewModel.Bericht;
            if (bericht == null) return false;

            var dokument = Druck.BerichtDruckBauer.Baue(
                bericht, eintrag.ViewModel.Schriftart);
            if (dokument == null) return false;

            if (vorschau)
                Druck.DruckDokument.ZeigeVorschau(
                    dokument, bericht.Titel, eintrag.HwndParent);
            else
                Druck.DruckDokument.Drucke(dokument, bericht.Titel);
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
