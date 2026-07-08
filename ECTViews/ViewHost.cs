// ViewHost.cs -- Statischer Einstiegspunkt zum Anzeigen von WPF-Dialogen
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

        // ----------------------------------------------
        // Sprite-Bitmaps für Betrieb/Bestandskonto-Icons
        // Werden lazily aus den Embedded Resources geladen.
        // ----------------------------------------------

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
        /// Vom Aufrufer übergebene Listen -- werden bei jedem Dialog-Aufruf
        /// in das ViewModel kopiert. Zentrale Stelle, damit der C++/CLI-
        /// Aufrufer diese nur einmal setzen muss.
        /// </summary>
        public static System.Collections.Generic.IList<string> BetriebeNamen { get; set; }
        public static System.Collections.Generic.IList<int>    BetriebeIcons { get; set; }
        public static System.Collections.Generic.IList<string> BestandskontenNamen { get; set; }
        public static System.Collections.Generic.IList<int>    BestandskontenIcons { get; set; }
        public static System.Collections.Generic.IList<int>    BestandskontenSalden { get; set; }

        private static void BefuelleListen(BuchungViewModel vm)
        {
            vm.LadeBetriebe(BetriebeNamen, BetriebeIcons, SpriteBetriebe);
            vm.LadeBestandskonten(BestandskontenNamen, BestandskontenIcons,
                SpriteBestandskonten);
        }

        /// <summary>
        /// Stellt sicher, dass ein WPF Application-Objekt existiert.
        /// Muss vor dem ersten WPF-Fenster aufgerufen werden.
        /// In einer MFC-Hostanwendung gibt es kein WPF App.xaml --
        /// daher erzeugen wir die Application manuell.
        /// </summary>
        private static void EnsureWpfInitialized()
        {
            if (_wpfInitialized) return;

            if (Application.Current == null)
            {
                // Minimale WPF-Application erzeugen.
                new Application();
            }

            // ShutdownMode IMMER auf OnExplicitShutdown ziehen, auch wenn
            // jemand anderes (z.B. JournalEmbed.EnsureWpfApplication) die
            // Application bereits mit dem Default OnLastWindowClose erzeugt
            // hat. Sonst beendet WPF die App, sobald der erste Dialog
            // geschlossen wird, und der nächste ShowDialog crasht mit
            // "Das Anwendungsobjekt wird beendet".
            Application.Current.ShutdownMode = ShutdownMode.OnExplicitShutdown;

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
            BuchungsDocument doc, bool ausgaben, IntPtr ownerHwnd = default,
            Action<Buchung> onWeiterbuchen = null)
        {
            EnsureWpfInitialized();

            var vm = new BuchungViewModel(doc, ausgaben);

            // "Weiterbuchen": jeder Klick persistiert die Buchung ueber
            // diesen Callback (nativer Aufrufer), der Dialog bleibt offen.
            if (onWeiterbuchen != null)
                vm.GebuchtUndWeiter += onWeiterbuchen;

            BefuelleListen(vm);
            var view = new BuchungView(vm);

            // Owner-Fenster setzen (MFC HWND --> WPF WindowInteropHelper)
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
        /// Zeigt den Dialog zur Bearbeitung einer bestehenden Buchung
        /// (ohne "Abgang buchen"-Button -- z.B. beim Kopieren).
        /// </summary>
        public static Buchung ZeigeBuchungBearbeitenDialog(
            BuchungsDocument doc, Buchung buchung, IntPtr ownerHwnd = default)
        {
            var vm = ZeigeBearbeitenInternal(doc, buchung, ownerHwnd, abgangErlaubt: false);
            return vm.Bestaetigt ? vm.Ergebnis : null;
        }

        /// <summary>
        /// Wie ZeigeBuchungBearbeitenDialog, aber mit aktivem
        /// "Abgang buchen"-Button. Das Ergebnis sagt zusaetzlich, ob der
        /// Benutzer den AfA-Abgang ausgeloest hat -- dann ist Buchung == null
        /// und der native Aufrufer fuehrt den Abgang aus.
        /// </summary>
        public static BuchungBearbeitenErgebnis ZeigeBuchungBearbeitenDialogMitAbgang(
            BuchungsDocument doc, Buchung buchung, IntPtr ownerHwnd = default)
        {
            var vm = ZeigeBearbeitenInternal(doc, buchung, ownerHwnd, abgangErlaubt: true);
            return new BuchungBearbeitenErgebnis
            {
                Buchung = vm.Bestaetigt ? vm.Ergebnis : null,
                AbgangGewuenscht = vm.AbgangGewuenscht
            };
        }

        private static BuchungViewModel ZeigeBearbeitenInternal(
            BuchungsDocument doc, Buchung buchung, IntPtr ownerHwnd, bool abgangErlaubt)
        {
            EnsureWpfInitialized();

            var vm = new BuchungViewModel(doc, buchung) { AbgangErlaubt = abgangErlaubt };
            BefuelleListen(vm);
            var view = new BuchungView(vm);

            if (ownerHwnd != IntPtr.Zero)
            {
                new WindowInteropHelper(view) { Owner = ownerHwnd };
            }

            view.ShowDialog();
            return vm;
        }

        /// <summary>
        /// Zeigt den "Buchungsjahr wählen"-Dialog beim Anlegen eines neuen
        /// Dokuments. Scannt das Datenverzeichnis nach bestehenden
        /// JahrXXXX.eca-Dateien (Jahreswechsel-Quellen) und liefert die vom
        /// Benutzer gewählte Aktion zurück.
        /// </summary>
        /// <param name="datenverzeichnis">Verzeichnis ohne abschließenden Backslash.</param>
        /// <param name="defaultJahr">Vorbelegung des Jahr-Feldes.</param>
        /// <param name="defaultWaehrung">Vorbelegung des Währungs-Feldes.</param>
        /// <param name="ownerHwnd">HWND des MFC-Elternfensters (optional).</param>
        public static BuchungsjahrWaehlenErgebnis ZeigeBuchungsjahrWaehlenDialog(
            string datenverzeichnis, int defaultJahr, string defaultWaehrung,
            IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new BuchungsjahrWaehlenViewModel(defaultJahr, defaultWaehrung);
            vm.SetzeDateien(SucheBuchungsdateien(datenverzeichnis));

            var view = new BuchungsjahrWaehlenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();

            return new BuchungsjahrWaehlenErgebnis
            {
                Aktion = vm.Aktion,
                Jahr = vm.Jahr,
                Waehrung = vm.Waehrung ?? string.Empty,
                QuelldateiPfad = vm.QuelldateiPfad ?? string.Empty
            };
        }

        /// <summary>
        /// Zeigt den Verwaltungs-/Auswahl-Dialog für Betriebe (WPF-Ersatz für
        /// CIconAuswahlBetrieb im Modus 1). Änderungen werden sofort über den
        /// Einstellungs-Cache in die ini geschrieben.
        /// </summary>
        /// <returns>Index des gewählten Betriebs ("Sel. anzeigen"), oder -1
        /// für "Alle anzeigen"/Abbruch.</returns>
        public static int ZeigeBetriebeVerwaltenDialog(IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new Stammdaten.BetriebeVerwaltenViewModel();
            var view = new Stammdaten.StammdatenVerwaltenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            return view.GewaehlterIndex;
        }

        /// <summary>
        /// Zeigt den Verwaltungs-/Auswahl-Dialog für Bestandskonten (WPF-Ersatz
        /// für CIconAuswahlBestandskonto im Modus 1).
        /// </summary>
        /// <param name="buchungsjahr">Buchungsjahr des aktiven Dokuments --
        /// der Anfangssaldo-Dialog bearbeitet den Saldo des Vorjahres.</param>
        /// <returns>Index des gewählten Bestandskontos, oder -1.</returns>
        public static int ZeigeBestandskontenVerwaltenDialog(
            int buchungsjahr, IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new Stammdaten.BestandskontenVerwaltenViewModel(buchungsjahr);
            var view = new Stammdaten.StammdatenVerwaltenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            return view.GewaehlterIndex;
        }

        /// <summary>
        /// Listet *.eca-Dateien im Verzeichnis auf, aufsteigend sortiert
        /// (jüngste zuletzt -- entspricht dem alten LBS_SORT der MFC-Liste).
        /// </summary>
        private static System.Collections.Generic.IEnumerable<BuchungsdateiItem>
            SucheBuchungsdateien(string datenverzeichnis)
        {
            var liste = new System.Collections.Generic.List<BuchungsdateiItem>();
            if (string.IsNullOrEmpty(datenverzeichnis) ||
                !System.IO.Directory.Exists(datenverzeichnis))
                return liste;

            try
            {
                foreach (var pfad in System.IO.Directory.GetFiles(datenverzeichnis, "*.eca"))
                {
                    liste.Add(new BuchungsdateiItem
                    {
                        Name = System.IO.Path.GetFileName(pfad),
                        VollerPfad = pfad
                    });
                }
            }
            catch (System.IO.IOException)
            {
                // Verzeichnis nicht lesbar -- leere Liste, Dialog zeigt Hinweis.
            }

            liste.Sort((a, b) => string.Compare(
                a.Name, b.Name, StringComparison.OrdinalIgnoreCase));
            return liste;
        }
    }

    /// <summary>
    /// Ergebnis von ViewHost.ZeigeBuchungsjahrWaehlenDialog.
    /// </summary>
    public sealed class BuchungsjahrWaehlenErgebnis
    {
        public BuchungsjahrAktion Aktion { get; set; }
        public int Jahr { get; set; }
        public string Waehrung { get; set; }
        public string QuelldateiPfad { get; set; }
    }

    /// <summary>
    /// Ergebnis von ViewHost.ZeigeBuchungBearbeitenDialogMitAbgang.
    /// </summary>
    public sealed class BuchungBearbeitenErgebnis
    {
        /// <summary>Geaenderte Buchung, oder null (abgebrochen oder Abgang).</summary>
        public Buchung Buchung { get; set; }

        /// <summary>True wenn "Abgang buchen" geklickt wurde.</summary>
        public bool AbgangGewuenscht { get; set; }
    }
}
