// FormularAbschnittDialog.xaml.cs - Code-Behind des Abschnitts-Dialogs
// (Pendant CFormularabschnitt). Arbeitet wie der Feld-Dialog auf der
// uebergebenen Definition; der Aufrufer persistiert.

using System.Windows;
using ECTEngine;

namespace ECTViews.Formulare
{
    public partial class FormularAbschnittDialog : Window
    {
        private readonly FormularAbschnittDef _abschnitt;

        public FormularAbschnittDialog(FormularAbschnittDef abschnitt)
        {
            InitializeComponent();
            _abschnitt = abschnitt;

            NameBox.Text = _abschnitt.Name;
            SeiteBox.Text = _abschnitt.Seite.ToString();
            VertikalBox.Text = _abschnitt.Vertikal.ToString();

            Loaded += (s, e) => { NameBox.Focus(); NameBox.SelectAll(); };
        }

        private void OnOk(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(NameBox.Text))
            {
                FehlerText.Text = "Bitte einen Namen angeben.";
                FehlerText.Visibility = Visibility.Visible;
                return;
            }
            int seite = FormularDefinition.Atoi(SeiteBox.Text);
            if (seite < 1)
            {
                FehlerText.Text = "Die Seite muss mindestens 1 sein.";
                FehlerText.Visibility = Visibility.Visible;
                return;
            }

            _abschnitt.Name = NameBox.Text.Trim();
            _abschnitt.Seite = seite;
            _abschnitt.Vertikal = FormularDefinition.Atoi(VertikalBox.Text);
            DialogResult = true;
        }
    }
}
