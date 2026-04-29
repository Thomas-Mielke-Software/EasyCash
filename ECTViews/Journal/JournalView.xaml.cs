// JournalView.xaml.cs — Code-Behind
//
// Hauptaufgabe: Doppelklick-Routing (löst BearbeitenCommand aus, wenn
// auf eine Buchungs-Zeile geklickt wurde — ListBox unterscheidet das
// nicht von alleine).

using System.Windows.Controls;
using System.Windows.Input;
using ECTViews.Journal;

namespace ECTViews.Journal
{
    public partial class JournalView : UserControl
    {
        public JournalView()
        {
            InitializeComponent();
        }

        private void OnZeilenDoppelklick(object sender, MouseButtonEventArgs e)
        {
            if (DataContext is JournalViewModel vm
                && vm.SelektierteZeile != null
                && vm.BearbeitenCommand.CanExecute(null))
            {
                vm.BearbeitenCommand.Execute(null);
                e.Handled = true;
            }
        }
    }
}
