// FormularFeldDialog.xaml.cs - Code-Behind des Feld-Eigenschaften-Dialogs
// (Pendant CFormularfeld/IDD_FORMULARFELD).
//
// Der Dialog arbeitet auf einer Kopie der Werte; erst bei "Speichern"
// werden sie in die uebergebene FormularFeldDef geschrieben (der Aufrufer
// persistiert dann ueber FormularViewModel.SpeichereFeld).

using System.Windows;
using ECTEngine;

namespace ECTViews.Formulare
{
    public partial class FormularFeldDialog : Window
    {
        private readonly FormularFeldDef _feld;

        private static readonly string[] BekannteTypen =
        {
            "Einnahmen", "Ausgaben", "Summe",
            "Einstellungsdaten", "Dokumentdaten", "Freitext"
        };

        public FormularFeldDialog(FormularFeldDef feld)
        {
            InitializeComponent();
            _feld = feld;

            foreach (var typ in BekannteTypen)
                TypBox.Items.Add(typ);

            IdBox.Text = _feld.Id.ToString();
            TypBox.Text = _feld.TypRoh;
            NameBox.Text = _feld.Name;
            ErweiterungBox.Text = _feld.Erweiterung;
            SeiteBox.Text = _feld.Seite.ToString();
            HorizontalBox.Text = _feld.Horizontal.ToString();
            VertikalBox.Text = _feld.Vertikal.ToString();
            AusrichtungBox.SelectedIndex = _feld.RechtsBuendig ? 0 : 1;
            AnteilBox.SelectedIndex =
                _feld.Anteil == FormularFeldAnteil.Netto ? 1
                : _feld.Anteil == FormularFeldAnteil.Mwst ? 2 : 0;
            OhneNachkommaBox.IsChecked = _feld.NachkommaanteilOhne;
            NullwertBox.IsChecked = _feld.NullwertAnzeigen;
            VeraltetBox.IsChecked = _feld.Veraltet;
            ElsterBox.Text = _feld.Elsterfeldname;

            Loaded += (s, e) => { IdBox.Focus(); IdBox.SelectAll(); };
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            int id = FormularDefinition.Atoi(IdBox.Text);
            if (id <= 0 || id >= 10000)
            {
                ZeigeFehler("Die Feld-Id muss zwischen 1 und 9999 liegen.");
                return;
            }
            if (string.IsNullOrWhiteSpace(TypBox.Text))
            {
                ZeigeFehler("Bitte einen Feldtyp angeben.");
                return;
            }
            int seite = FormularDefinition.Atoi(SeiteBox.Text);
            if (seite < 1)
            {
                ZeigeFehler("Die Seite muss mindestens 1 sein.");
                return;
            }

            _feld.Id = id;
            FormularDefinition.SetzeTyp(_feld, TypBox.Text.Trim());
            _feld.Name = NameBox.Text ?? "";
            _feld.Erweiterung = ErweiterungBox.Text ?? "";
            _feld.Seite = seite;
            _feld.Horizontal = FormularDefinition.Atoi(HorizontalBox.Text);
            _feld.Vertikal = FormularDefinition.Atoi(VertikalBox.Text);
            _feld.RechtsBuendig = AusrichtungBox.SelectedIndex != 1;
            _feld.Anteil = AnteilBox.SelectedIndex == 1 ? FormularFeldAnteil.Netto
                : AnteilBox.SelectedIndex == 2 ? FormularFeldAnteil.Mwst
                : FormularFeldAnteil.Brutto;
            _feld.NachkommaanteilOhne = OhneNachkommaBox.IsChecked == true;
            _feld.NullwertAnzeigen = NullwertBox.IsChecked == true;
            _feld.Veraltet = VeraltetBox.IsChecked == true;
            _feld.Elsterfeldname = ElsterBox.Text ?? "";

            DialogResult = true;
        }

        private void ZeigeFehler(string text)
        {
            FehlerText.Text = text;
            FehlerText.Visibility = Visibility.Visible;
        }
    }
}
