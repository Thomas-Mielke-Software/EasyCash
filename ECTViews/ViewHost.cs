// ViewHost.cs — Statischer Einstiegspunkt zum Anzeigen von WPF-Dialogen
//
// Wird von der C++/CLI-Bridge (ECTBridge) aufgerufen. Stellt sicher,
// dass ein WPF Application-Objekt und ein Dispatcher existieren,
// bevor ein WPF-Fenster erzeugt wird.
//
// Aufruf aus C++/CLI:
//   ECTViews::ViewHost::ZeigeBuchungDialog(engine, true);

using System;
using System.Windows;
using System.Windows.Interop;
using ECTEngine;
using ECTViews.ViewModels;
using ECTViews.Views;

namespace ECTViews
{
    /// <summary>
    /// Statischer Host für WPF-Dialoge. Kümmert sich um:
    /// - WPF Application-Initialisierung (falls noch keine existiert)
    /// - Fenster-Erzeugung mit korrektem Owner (MFC HWND)
    /// - Rückgabe der Ergebnis-Buchung
    /// - Befüllen der Bestandskonto-/Betrieb-Listen mit Icons
    /// </summary>
    public static class ViewHost
    {
        private static bool _wpfInitialized;

        // ──────────────────────────────────────────────
        // Sprite-Bitmaps für Betrieb/Bestandskonto-Icons
        // Werden lazily aus den Embedded Resources geladen.
        // ──────────────────────────────────────────────

        private static System.Windows.Media.Imaging.BitmapSource _spriteBetriebe;
        private static System.Windows.Media.Imaging.BitmapSource _spriteBestandskonten;

        /// <summary>
        /// Sprite-Bitmap für Betrieb-Icons. Standardmäßig aus
        /// "Resources/icons.bmp" der ECTViews-Assembly geladen.
        /// Kann vom Aufrufer überschrieben werden.
        /// </summary>
        public static System.Windows.Media.Imaging.BitmapSource SpriteBetriebe
        {
            get
            {
                if (_spriteBetriebe == null)
                    _spriteBetriebe = IconSpriteSplitter.LoadFromResource(
                        "icons.bmp");
                return _spriteBetriebe;
            }
            set => _spriteBetriebe = value;
        }

        /// <summary>
        /// Sprite-Bitmap für Bestandskonto-Icons. Standardmäßig aus
        /// "Resources/icons_bestandskonten.bmp".
        /// </summary>
        public static System.Windows.Media.Imaging.BitmapSource SpriteBestandskonten
        {
            get
            {
                if (_spriteBestandskonten == null)
                    _spriteBestandskonten = IconSpriteSplitter.LoadFromResource(
                        "icons_bestandskonten.bmp");
                return _spriteBestandskonten;
            }
            set => _spriteBestandskonten = value;
        }

        /// <summary>
        /// Vom Aufrufer übergebene Listen — werden bei jedem Dialog-Aufruf
        /// in das ViewModel kopiert. Zentrale Stelle, damit der C++/CLI-
        /// Aufrufer diese nur einmal setzen muss.
        /// </summary>
        public static System.Collections.Generic.IList<string> BetriebeNamen { get; set; }
        public static System.Collections.Generic.IList<string> BetriebeIcons { get; set; }
        public static System.Collections.Generic.IList<string> BestandskontenNamen { get; set; }
        public static System.Collections.Generic.IList<string> BestandskontenIcons { get; set; }

        private static void BefuelleListen(BuchungViewModel vm)
        {
            vm.LadeBetriebe(BetriebeNamen, BetriebeIcons, SpriteBetriebe);
            vm.LadeBestandskonten(BestandskontenNamen, BestandskontenIcons,
                SpriteBestandskonten);
        }

        /// <summary>
        /// Stellt sicher, dass ein WPF Application-Objekt existiert.
        /// Muss vor dem ersten WPF-Fenster aufgerufen werden.
        /// In einer MFC-Hostanwendung gibt es kein WPF App.xaml —
        /// daher erzeugen wir die Application manuell.
        /// </summary>
        private static void EnsureWpfInitialized()
        {
            if (_wpfInitialized) return;

            if (Application.Current == null)
            {
                // Minimale WPF-Application erzeugen.
                // ShutdownMode=OnExplicitShutdown verhindert, dass
                // WPF die App beendet wenn ein Fenster geschlossen wird.
                var app = new Application
                {
                    ShutdownMode = ShutdownMode.OnExplicitShutdown
                };
            }

            _wpfInitialized = true;
        }

        /// <summary>
        /// Zeigt den Buchungseingabe-Dialog als modales Fenster.
        ///
        /// Aufruf aus C++/CLI (Exports.cpp oder EasyCashDocBridge.cpp):
        ///   ECTViews::ViewHost::ZeigeBuchungDialog(engine, true);
        ///
        /// Aufruf mit Owner-HWND (damit der Dialog vor dem MFC-Fenster bleibt):
        ///   ECTViews::ViewHost::ZeigeBuchungDialog(engine, true, hwnd);
        /// </summary>
        /// <param name="doc">Das BuchungsDocument aus der Engine.</param>
        /// <param name="ausgaben">True für Ausgaben, False für Einnahmen.</param>
        /// <param name="ownerHwnd">
        /// HWND des MFC-Elternfensters (optional). Wenn angegeben,
        /// wird der Dialog modal zu diesem Fenster.
        /// </param>
        /// <returns>
        /// Die neue/geänderte Buchung, oder null wenn abgebrochen.
        /// </returns>
        public static Buchung ZeigeBuchungDialog(
            BuchungsDocument doc, bool ausgaben, IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new BuchungViewModel(doc, ausgaben);
            BefuelleListen(vm);
            var view = new BuchungView(vm);

            // Owner-Fenster setzen (MFC HWND → WPF WindowInteropHelper)
            if (ownerHwnd != IntPtr.Zero)
            {
                var helper = new WindowInteropHelper(view)
                {
                    Owner = ownerHwnd
                };
            }

            view.ShowDialog();

            return vm.Bestaetigt ? vm.Ergebnis : null;
        }

        /// <summary>
        /// Zeigt den Dialog zur Bearbeitung einer bestehenden Buchung.
        /// </summary>
        public static Buchung ZeigeBuchungBearbeitenDialog(
            BuchungsDocument doc, Buchung buchung, IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new BuchungViewModel(doc, buchung);
            BefuelleListen(vm);
            var view = new BuchungView(vm);

            if (ownerHwnd != IntPtr.Zero)
            {
                new WindowInteropHelper(view) { Owner = ownerHwnd };
            }

            view.ShowDialog();

            return vm.Bestaetigt ? vm.Ergebnis : null;
        }
    }
}
