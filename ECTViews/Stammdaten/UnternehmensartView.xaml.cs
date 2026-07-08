// UnternehmensartView.xaml.cs -- Code-Behind des Unternehmensart-Dialogs.
//
// Die W-IdNr-Validierung ist advisory (rote Zeile) und blockiert das OK
// nicht -- wie die Hinweis-MessageBoxen des alten CUnternehmensartDlg.

using System.Windows;

namespace ECTViews.Stammdaten
{
    public partial class UnternehmensartView : Window
    {
        public UnternehmensartView(UnternehmensartViewModel vm)
        {
            InitializeComponent();
            DataContext = vm;
            Loaded += (s, e) => Unternehmensart1Box.Focus();
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }
    }
}
