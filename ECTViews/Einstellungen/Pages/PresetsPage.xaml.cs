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
    }
}
