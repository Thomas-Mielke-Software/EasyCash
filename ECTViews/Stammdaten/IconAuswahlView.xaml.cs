// IconAuswahlView.xaml.cs -- Code-Behind der reinen Icon-Auswahl.
//
// Ergebnis über GewaehlterIndex abfragen (-1 = abgebrochen), wie
// m_nSelected des alten CIconAuswahl.

using System.Windows;
using System.Windows.Input;

namespace ECTViews.Stammdaten
{
    public partial class IconAuswahlView : Window
    {
        private readonly IconAuswahlViewModel _vm;

        /// <summary>Index des gewählten Icons, -1 wenn abgebrochen.</summary>
        public int GewaehlterIndex { get; private set; } = -1;

        public IconAuswahlView(IconAuswahlViewModel vm)
        {
            InitializeComponent();
            _vm = vm;
            DataContext = vm;

            // Vorausgewaehltes Icon (z.B. bei "Icon aendern") sichtbar machen.
            Loaded += (s, e) =>
            {
                if (_vm.Auswahl != null)
                    IconListe.ScrollIntoView(_vm.Auswahl);
            };
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            if (_vm.Auswahl == null) return;
            GewaehlterIndex = _vm.Auswahl.IconIndex;
            DialogResult = true;
        }

        private void OnDoppelklick(object sender, MouseButtonEventArgs e)
        {
            OnOk(sender, null);
        }
    }
}
