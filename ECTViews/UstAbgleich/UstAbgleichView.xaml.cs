// UstAbgleichView.xaml.cs -- Dialog "Zuordnungen für Voranmeldung und
// Umsatzsteuererklärung abgleichen" (WPF-Ersatz für CUStAbgleichDlg).
// Logik im UstAbgleichViewModel; hier nur die Auswahl je Zeile als
// Kontextmenü (wie das Vorschlagsmenü in v3) und Fenstergröße.

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace ECTViews.UstAbgleichUi
{
    public partial class UstAbgleichView : Window
    {
        private readonly UstAbgleichViewModel _vm;

        public UstAbgleichView(UstAbgleichViewModel vm)
        {
            InitializeComponent();
            _vm = vm;
            DataContext = vm;
            BuchungenSpalte.Header = vm.BuchungenKopf;   // Spaltenköpfe erben keinen DataContext
            vm.AuswahlNoetig += z => Dispatcher.BeginInvoke(new Action(() => ZeigeAuswahl(z, null)),
                System.Windows.Threading.DispatcherPriority.Input);

            // Sechs Spalten brauchen Platz, aber nie breiter als der Arbeitsbereich.
            var arbeitsbereich = SystemParameters.WorkArea;
            if (Width > arbeitsbereich.Width - 40) Width = Math.Max(MinWidth, arbeitsbereich.Width - 40);
            if (Height > arbeitsbereich.Height - 40) Height = Math.Max(MinHeight, arbeitsbereich.Height - 40);

            Loaded += (s, e) =>
            {
                if (vm.Zeilen.Count > 0)
                {
                    Liste.SelectedIndex = 0;
                    Liste.Focus();
                }
            };
        }

        private void OnUebernehmen(object sender, RoutedEventArgs e)
        {
            _vm.Uebernehmen();
            DialogResult = true;
        }

        private void OnAendern(object sender, RoutedEventArgs e)
        {
            if (!(Liste.SelectedItem is UstAbgleichZeile z))
            {
                MessageBox.Show(this, "Bitte erst eine Zeile auswählen.", Title,
                    MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }
            ZeigeAuswahl(z, sender as UIElement);
        }

        private void OnDoppelklick(object sender, MouseButtonEventArgs e)
        {
            // Nur Doppelklicks auf eine Zeile (nicht auf Kopf oder Rollbalken)
            var row = ItemsControl.ContainerFromElement(Liste, e.OriginalSource as DependencyObject) as DataGridRow;
            if (row?.Item is UstAbgleichZeile z)
            {
                Liste.SelectedItem = z;
                ZeigeAuswahl(z, null);
                e.Handled = true;
            }
        }

        /// <summary>Auswahl für eine Zeile als Kontextmenü -- an der Maus bzw.
        /// unter dem auslösenden Knopf.</summary>
        private void ZeigeAuswahl(UstAbgleichZeile z, UIElement anker)
        {
            if (z == null) return;
            Liste.SelectedItem = z;

            var menue = new ContextMenu();
            foreach (var o in _vm.Optionen(z))
            {
                if (o.IstTrenner)
                {
                    menue.Items.Add(new Separator());
                    continue;
                }
                var item = new MenuItem
                {
                    Header = new TextBlock { Text = o.Text },   // TextBlock: '_' nicht als Zugriffstaste deuten
                    IsChecked = o.IstGewaehlt,
                    IsEnabled = !o.IstKopf,
                };
                if (o.IstKopf) item.FontWeight = FontWeights.SemiBold;
                var anwenden = o.Anwenden;
                if (anwenden != null) item.Click += (s, e) => anwenden();
                menue.Items.Add(item);
            }

            if (anker != null)
            {
                menue.PlacementTarget = anker;
                menue.Placement = PlacementMode.Bottom;
            }
            else
                menue.Placement = PlacementMode.MousePoint;
            menue.IsOpen = true;
        }
    }
}
