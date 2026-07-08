// DauerbuchungenView.xaml.cs -- Code-Behind der Dauerbuchungs-Verwaltung.
//
// Validierungsfehler kommen als Text aus dem ViewModel und werden wie
// im MFC-Original als MessageBox angezeigt. Ob etwas geändert wurde,
// fragt der Aufrufer nach dem (modalen) Dialog über das ViewModel ab.

using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using ECTEngine;

namespace ECTViews.Dauerbuchungen
{
    public partial class DauerbuchungenView : Window
    {
        private readonly DauerbuchungenViewModel _vm;

        public DauerbuchungenView(DauerbuchungenViewModel vm)
        {
            InitializeComponent();
            _vm = vm;
            DataContext = vm;
        }

        private void OnNeu(object sender, RoutedEventArgs e)
        {
            _vm.NeuStarten();
            VonMonatBox.Focus();
        }

        private void OnAendern(object sender, RoutedEventArgs e)
        {
            if (_vm.AendernStarten())
                VonMonatBox.Focus();
            else
                Hinweis("Bitte erst eine Dauerbuchung in der Liste auswählen.");
        }

        private void OnZeileDoppelklick(object sender, MouseButtonEventArgs e)
        {
            if (_vm.ListeAktiv && _vm.Auswahl != null)
                OnAendern(sender, null);
        }

        private void OnLoeschen(object sender, RoutedEventArgs e)
        {
            var zeile = _vm.Auswahl;
            if (zeile == null)
            {
                Hinweis("Bitte erst eine Dauerbuchung in der Liste auswählen.");
                return;
            }
            if (MessageBox.Show(this,
                    $"Dauerbuchung \"{zeile.Beschreibung}\" wirklich löschen?\n\n" +
                    "Bereits erzeugte Buchungen bleiben erhalten.",
                    Title, MessageBoxButton.YesNo, MessageBoxImage.Question,
                    MessageBoxResult.No) != MessageBoxResult.Yes)
                return;
            _vm.Loeschen();
        }

        private void OnVerwerfen(object sender, RoutedEventArgs e)
        {
            _vm.Verwerfen();
        }

        private void OnUebernehmen(object sender, RoutedEventArgs e)
        {
            string fehler = _vm.Uebernehmen();
            if (fehler != null)
                MessageBox.Show(this, fehler, Title,
                    MessageBoxButton.OK, MessageBoxImage.Stop);
        }

        /// <summary>Zweistellige Jahre beim Verlassen des Feldes fensterln
        /// (wie OnKillfocusDatumVonJahr/BisJahr im Original).</summary>
        private void OnJahrFeldLostFocus(object sender, RoutedEventArgs e)
        {
            var box = (TextBox)sender;
            if (int.TryParse(box.Text?.Trim(), out int jahr))
            {
                int normalisiert = DauerbuchungenViewModel.NormalisiereJahr(jahr);
                if (normalisiert != jahr)
                {
                    box.Text = normalisiert.ToString();
                    box.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
                }
            }
        }

        private void Hinweis(string text)
        {
            MessageBox.Show(this, text, Title,
                MessageBoxButton.OK, MessageBoxImage.Information);
        }
    }
}
