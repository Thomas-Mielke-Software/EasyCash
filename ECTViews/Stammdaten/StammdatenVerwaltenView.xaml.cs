// StammdatenVerwaltenView.xaml.cs -- Code-Behind des Verwaltungs-/
// Auswahl-Dialogs für Betriebe und Bestandskonten.
//
// Ergebnis über GewaehlterIndex abfragen:
//   >= 0  Index des gewählten Eintrags ("Sel. anzeigen" -> Filter setzen)
//   -1    "Alle anzeigen" / Abbruch -> Filter aufheben
// Verwaltungs-Änderungen (Neu/Löschen/Umbenennen/Icon/Property) sind beim
// Schließen bereits persistiert, unabhängig vom Rückgabewert.

using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ECTViews.Stammdaten
{
    public partial class StammdatenVerwaltenView : Window
    {
        private readonly StammdatenVerwaltenViewModel _vm;

        /// <summary>Index des gewählten Eintrags, -1 = alle anzeigen/Abbruch.</summary>
        public int GewaehlterIndex { get; private set; } = -1;

        public StammdatenVerwaltenView(StammdatenVerwaltenViewModel vm)
        {
            InitializeComponent();
            _vm = vm;
            vm.Lade();
            DataContext = vm;
        }

        // ------------------------------------------------------------------
        // Auswahl ("Sel. anzeigen" / Doppelklick)
        // ------------------------------------------------------------------

        private void OnSelAnzeigen(object sender, RoutedEventArgs e)
        {
            if (_vm.Auswahl == null) return;
            GewaehlterIndex = _vm.Eintraege.IndexOf(_vm.Auswahl);
            DialogResult = true;
        }

        private void OnDoppelklick(object sender, MouseButtonEventArgs e)
        {
            OnSelAnzeigen(sender, null);
        }

        // ------------------------------------------------------------------
        // Verwaltung
        // ------------------------------------------------------------------

        /// <summary>Ablauf wie CIconAuswahl::OnNeu: erst Icon wählen, dann den
        /// Property-Dialog (Unternehmensart/Anfangssaldo), dann anlegen.
        /// Abbruch in einem der beiden Schritte legt nichts an.</summary>
        private void OnNeu(object sender, RoutedEventArgs e)
        {
            if (_vm.ListeVoll)
            {
                MessageBox.Show(this,
                    "Oh, sorry: Kann nur 100 Einträge anlegen!",
                    Title, MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            int icon = WaehleIcon(-1);
            if (icon < 0) return;

            if (!_vm.FrageProperty(this,
                    _vm.DefaultName(icon, _vm.Eintraege.Count), "", out string property))
                return;

            _vm.NeuAnlegen(icon, property);

            // Direkt zum Umbenennen ins Namensfeld springen.
            NameBox.Focus();
            NameBox.SelectAll();
        }

        private void OnLoeschen(object sender, RoutedEventArgs e)
        {
            var eintrag = _vm.Auswahl;
            if (eintrag == null) return;

            if (MessageBox.Show(this,
                    $"\"{eintrag.Name}\" wirklich löschen?\n\n" +
                    "Bestehende Buchungen behalten den Namen als Text und werden nicht geändert.",
                    Title, MessageBoxButton.YesNo, MessageBoxImage.Question,
                    MessageBoxResult.No) != MessageBoxResult.Yes)
                return;

            _vm.Loeschen(eintrag);

            // Typspezifische Nachbehandlung (Mandanten: Moduswechsel-Meldungen
            // beim Löschen des vorletzten/letzten Mandanten). Verlangt die VM
            // das Schließen, wird der Dialog wie ein Abbruch beendet -- die
            // (jetzt leere) Liste wird vom nativen Aufrufer trotzdem
            // zurückgeschrieben, GewaehlterIndex bleibt -1.
            if (_vm.NachLoeschen(this, eintrag))
                DialogResult = false;
        }

        private void OnIconAendern(object sender, RoutedEventArgs e)
        {
            var eintrag = _vm.Auswahl;
            if (eintrag == null) return;

            int icon = WaehleIcon(eintrag.IconIndex);
            if (icon < 0) return;
            _vm.SetzeIcon(eintrag, icon);
        }

        private void OnPropertyBearbeiten(object sender, RoutedEventArgs e)
        {
            var eintrag = _vm.Auswahl;
            if (eintrag == null) return;

            if (_vm.FrageProperty(this, eintrag.Name,
                    _vm.HoleProperty(eintrag), out string neuerWert))
                _vm.SetzeProperty(eintrag, neuerWert);
        }

        /// <summary>Öffnet den Icon-Picker modal. -1 = abgebrochen.</summary>
        private int WaehleIcon(int vorauswahl)
        {
            var vm = new IconAuswahlViewModel(
                "Icon wählen", _vm.Sprite, _vm.IconNamen, vorauswahl);
            var dlg = new IconAuswahlView(vm) { Owner = this };
            dlg.ShowDialog();
            return dlg.GewaehlterIndex;
        }

        /// <summary>Enter im Namensfeld übernimmt den Namen sofort (das
        /// Binding aktualisiert sonst erst bei LostFocus).</summary>
        private void OnNameBoxKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key != Key.Enter) return;
            var box = (TextBox)sender;
            box.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
            e.Handled = true;   // nicht als IsDefault-Klick ("Sel. anzeigen") werten
        }
    }
}
