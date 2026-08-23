using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ECTViews.EinstellungenUi.Pages
{
    public partial class PresetsPage : UserControl, IEinstellungenLiveZiel
    {
        public PresetsPage()
        {
            InitializeComponent();
            // Verlässt der Tastaturfokus ein Eingabefeld, eine ggf. aufgeschobene
            // externe Änderung nachholen.
            AddHandler(LostKeyboardFocusEvent,
                new KeyboardFocusChangedEventHandler(OnFokusVerloren), true);
            EinstellungenLiveSync.Registriere(this);
        }

        private PresetsPageViewModel VM => DataContext as PresetsPageViewModel;

        // -----------------------------------------------------------------
        // Live-Sync: Änderung aus einem anderen Dokumentfenster übernehmen,
        // aber NICHT während eine Bearbeitung im Editor läuft (sonst würde die
        // Liste/Selektion mitten in der Eingabe neu aufgebaut).
        // -----------------------------------------------------------------
        private bool _aktualisierungAusstehend;

        public void AufExterneEinstellungsaenderung()
        {
            if (InteraktionLaeuft()) { _aktualisierungAusstehend = true; return; }
            VM?.AktualisiereAusCache();
        }

        private bool InteraktionLaeuft()
        {
            // Tastaturfokus in einem Eingabefeld dieser Seite?
            if (Keyboard.FocusedElement is DependencyObject fe
                && fe is System.Windows.Controls.Primitives.TextBoxBase
                && IsAncestorOf(fe))
                return true;
            return false;
        }

        private void OnFokusVerloren(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_aktualisierungAusstehend)
                Dispatcher.BeginInvoke(new Action(VerarbeiteAusstehend),
                    System.Windows.Threading.DispatcherPriority.Background);
        }

        private void VerarbeiteAusstehend()
        {
            if (_aktualisierungAusstehend && !InteraktionLaeuft())
            {
                _aktualisierungAusstehend = false;
                VM?.AktualisiereAusCache();
            }
        }

        // -----------------------------------------------------------------
        // Neu: anlegen + direkt ins Nr.-Feld springen (Nummer vergeben)
        // -----------------------------------------------------------------

        private void OnNeu(object sender, RoutedEventArgs e)
        {
            VM?.NeuAnlegen();
            Dispatcher.BeginInvoke(new Action(() =>
            {
                NummerBox?.Focus();
                NummerBox?.SelectAll();
            }), System.Windows.Threading.DispatcherPriority.Background);
        }

        // -----------------------------------------------------------------
        // Buchungsgruppen-Zeilen (Buttons im Zeilen-Editor)
        // -----------------------------------------------------------------

        private void OnZeileHinzufuegen(object sender, RoutedEventArgs e)
        {
            VM?.Ausgewaehlt?.ZeileHinzufuegen();
        }

        private static PresetsPageViewModel.PresetZeileItem ZeileVon(object sender)
            => (sender as FrameworkElement)?.DataContext
                as PresetsPageViewModel.PresetZeileItem;

        private void OnZeileEntfernen(object sender, RoutedEventArgs e)
        {
            VM?.Ausgewaehlt?.ZeileEntfernen(ZeileVon(sender));
        }

        private void OnZeileHoch(object sender, RoutedEventArgs e)
        {
            VM?.Ausgewaehlt?.ZeileVerschieben(ZeileVon(sender), -1);
        }

        private void OnZeileRunter(object sender, RoutedEventArgs e)
        {
            VM?.Ausgewaehlt?.ZeileVerschieben(ZeileVon(sender), +1);
        }

        // -----------------------------------------------------------------
        // XML-Export/Import (Vorlagen teilen)
        // -----------------------------------------------------------------

        /// <summary>
        /// Fehlermeldung -- mit Besitzer-Fenster nur dort, wo es eines gibt.
        /// Die Einstellungs-Seiten laufen im MFC-Host in einer HwndSource und
        /// haben dann KEINEN Window-Vorfahren: Window.GetWindow(this) liefert
        /// null, und MessageBox.Show(null, ...) wirft eine
        /// ArgumentNullException (der WindowInteropHelper-Konstruktor
        /// verträgt kein null). Ohne Besitzer nimmt WPF selbst das aktive
        /// Fenster -- im Host also den MFC-Rahmen. Die CommonDialog-Aufrufe
        /// daneben brauchen das nicht, ShowDialog(null) ist dort zulässig.
        /// </summary>
        private void ZeigeFehler(string text, string titel)
        {
            var besitzer = Window.GetWindow(this);
            if (besitzer != null)
                MessageBox.Show(besitzer, text, titel,
                    MessageBoxButton.OK, MessageBoxImage.Error);
            else
                MessageBox.Show(text, titel,
                    MessageBoxButton.OK, MessageBoxImage.Error);
        }

        private void OnExportieren(object sender, RoutedEventArgs e)
        {
            var vm = VM;
            if (vm?.Ausgewaehlt == null) return;

            var xml = vm.ExportiereAusgewaehlt();
            if (xml == null) return;

            // Ungültige Dateinamen-Zeichen aus der Beschreibung entfernen
            var name = vm.Ausgewaehlt.Beschreibung ?? "Buchungsvorlage";
            foreach (var c in System.IO.Path.GetInvalidFileNameChars())
                name = name.Replace(c, '_');

            var dlg = new Microsoft.Win32.SaveFileDialog
            {
                Title = "Buchungsvorlage exportieren",
                FileName = name + ".ectvorlage.xml",
                Filter = "EasyCash&Tax-Buchungsvorlage (*.ectvorlage.xml)|*.ectvorlage.xml|Alle Dateien (*.*)|*.*",
                DefaultExt = ".xml"
            };
            if (dlg.ShowDialog(Window.GetWindow(this)) != true) return;

            try
            {
                System.IO.File.WriteAllText(dlg.FileName, xml,
                    new System.Text.UTF8Encoding(false));
            }
            catch (Exception ex)
            {
                ZeigeFehler(
                    "Die Vorlage konnte nicht gespeichert werden:\n" + ex.Message,
                    "Buchungsvorlage exportieren");
            }
        }

        private void OnImportieren(object sender, RoutedEventArgs e)
        {
            var vm = VM;
            if (vm == null) return;

            var dlg = new Microsoft.Win32.OpenFileDialog
            {
                Title = "Buchungsvorlage importieren",
                Filter = "EasyCash&Tax-Buchungsvorlage (*.ectvorlage.xml;*.xml)|*.ectvorlage.xml;*.xml|Alle Dateien (*.*)|*.*"
            };
            if (dlg.ShowDialog(Window.GetWindow(this)) != true) return;

            string fehler;
            try
            {
                fehler = vm.ImportiereXml(System.IO.File.ReadAllText(dlg.FileName));
            }
            catch (Exception ex)
            {
                fehler = ex.Message;
            }

            if (fehler != null)
                ZeigeFehler(
                    "Die Vorlage konnte nicht importiert werden:\n" + fehler,
                    "Buchungsvorlage importieren");
        }
    }
}
