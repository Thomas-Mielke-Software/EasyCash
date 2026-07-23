using System.Windows.Controls;

namespace ECTViews.EinstellungenUi.Pages
{
    public partial class WaehrungenPage : UserControl
    {
        public WaehrungenPage()
        {
            InitializeComponent();

            // DSGVO-Einwilligungs-Dialog vor dem ersten API-Abruf verdrahten.
            if (DataContext is WaehrungenPageViewModel vm)
                vm.ApiEinwilligungAbfrage = WaehrungApiEinwilligung.Sicherstellen;
        }
    }
}
