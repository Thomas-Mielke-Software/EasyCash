// NavigationView.xaml.cs - Code-Behind fuer das Navigationsfenster.

using System.Windows.Controls;
using System.Windows.Input;

namespace ECTViews.Journal
{
    public partial class NavigationView : UserControl
    {
        public NavigationView()
        {
            InitializeComponent();
        }

        private void OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Up:
                case Key.Down:
                case Key.PageUp:
                case Key.PageDown:
                case Key.Home:
                case Key.End:
                    JournalEmbed.NavigiereScroll(e.Key);
                    e.Handled = true;
                    break;
            }
        }
    }
}
