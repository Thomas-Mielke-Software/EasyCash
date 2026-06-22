using System;
using System.Windows.Controls;
using ECTEngine;

namespace ECTViews.EinstellungenUi.Pages
{
    public partial class DokumentPage : UserControl
    {
        /// <param name="dokument">Das aktuell geöffnete Buchungsdokument
        /// (dieselbe Engine-Instanz, die die Bridge hält).</param>
        /// <param name="onGeaendert">Callback, der bei jeder tatsächlichen
        /// Änderung eines Dokumentwerts gerufen wird (native Seite setzt
        /// darüber das Modified-Flag).</param>
        public DokumentPage(BuchungsDocument dokument, Action onGeaendert)
        {
            InitializeComponent();
            DataContext = new DokumentPageViewModel(dokument, onGeaendert);
        }
    }
}
