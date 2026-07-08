// DauerbuchungenAusfuehrenView.xaml.cs -- "Dauerbuchungen ausführen bis"
// (WPF-Ersatz für DauBuchAusfuehren, daubuchausfuehren.cpp).
//
// Fragt nur Monat/Jahr ab; die eigentliche Ausführung macht weiterhin
// der native Aufrufer (CEasyCashView::DauerbuchungenAusfuehren) -- dort
// leben die Platzhalter-Auflösung, die Buchungsjahr-Rückfrage und die
// Journal-Selektion der erzeugten Buchungen.

using System.Windows;

namespace ECTViews.Dauerbuchungen
{
    public partial class DauerbuchungenAusfuehrenView : Window
    {
        // Einfache Quell-Properties fuer die TwoWay-Bindings; Werte werden
        // nur initial gesetzt, daher kein INotifyPropertyChanged noetig.
        public string MonatText { get; set; }
        public string JahrText { get; set; }

        /// <summary>True wenn "Ausführen" geklickt wurde; Monat/Jahr dann
        /// in <see cref="Monat"/>/<see cref="Jahr"/>.</summary>
        public bool Bestaetigt { get; private set; }
        public int Monat { get; private set; }
        public int Jahr { get; private set; }

        /// <param name="buchungsjahr">Buchungsjahr des Dokuments. Default wie
        /// das Original: aktueller Monat, wenn das Buchungsjahr das laufende
        /// Jahr ist, sonst Dezember des Buchungsjahres.</param>
        public DauerbuchungenAusfuehrenView(int buchungsjahr)
        {
            InitializeComponent();

            var jetzt = System.DateTime.Now;
            if (jetzt.Year == buchungsjahr)
            {
                MonatText = jetzt.Month.ToString();
                JahrText = jetzt.Year.ToString();
            }
            else
            {
                MonatText = "12";
                JahrText = buchungsjahr.ToString();
            }

            DataContext = this;
            Loaded += (s, e) => { MonatBox.Focus(); MonatBox.SelectAll(); };
        }

        private void OnAusfuehren(object sender, RoutedEventArgs e)
        {
            if (!int.TryParse(MonatText?.Trim(), out int monat)
                || monat < 1 || monat > 12)
            {
                MessageBox.Show(this, "Eingabefehler: Wert für Monat von 1 bis 12!",
                    Title, MessageBoxButton.OK, MessageBoxImage.Stop);
                MonatBox.Focus();
                return;
            }

            int.TryParse(JahrText?.Trim(), out int jahr);
            jahr = DauerbuchungenViewModel.NormalisiereJahr(jahr);
            if (jahr > 3000) jahr = 3000;

            Monat = monat;
            Jahr = jahr;
            Bestaetigt = true;
            DialogResult = true;
        }
    }
}
