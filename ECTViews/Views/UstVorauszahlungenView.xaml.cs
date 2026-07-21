// UstVorauszahlungenView.xaml.cs -- Code-Behind des
// USt-Vorauszahlungen-Dialogs (Muster AnfangssaldoView).

using System.Windows;
using ECTViews.ViewModels;

namespace ECTViews.Views
{
    public partial class UstVorauszahlungenView : Window
    {
        private readonly UstVorauszahlungenViewModel _vm;

        public bool Bestaetigt { get; private set; }

        public UstVorauszahlungenView(UstVorauszahlungenViewModel vm)
        {
            InitializeComponent();
            _vm = vm;
            DataContext = vm;
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            if (!_vm.AlleGueltig) return;
            Bestaetigt = true;
            DialogResult = true;
        }
    }
}
