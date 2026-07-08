// AnfangssaldoView.xaml.cs -- Code-Behind des Anfangssaldo-Dialogs.

using System.Windows;

namespace ECTViews.Stammdaten
{
    public partial class AnfangssaldoView : Window
    {
        private readonly AnfangssaldoViewModel _vm;

        public AnfangssaldoView(AnfangssaldoViewModel vm)
        {
            InitializeComponent();
            _vm = vm;
            DataContext = vm;
            Loaded += (s, e) => { WertBox.Focus(); WertBox.SelectAll(); };
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            if (!_vm.IstGueltig) return;
            DialogResult = true;
        }
    }
}
