using System.Windows;
using ECTViews.ViewModels;

namespace ECTViews.Views
{
    public partial class BuchungView : Window
    {
        public BuchungView()
        {
            InitializeComponent();
        }

        public BuchungView(BuchungViewModel viewModel) : this()
        {
            DataContext = viewModel;
            viewModel.RequestClose += () => Close();
        }
    }
}
