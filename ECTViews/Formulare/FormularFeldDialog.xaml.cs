// FormularFeldDialog.xaml.cs - Code-Behind des Feld-Eigenschaften-Dialogs
// (Pendant CFormularfeld/IDD_FORMULARFELD).
//
// Der Dialog arbeitet auf einer Kopie der Werte; erst bei "Speichern"
// werden sie in die uebergebene FormularFeldDef geschrieben (der Aufrufer
// persistiert dann ueber FormularViewModel.SpeichereFeld).

using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
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

        // Positions-Vorschau in der Formular-Ansicht (Seite, horizontal,
        // vertikal); null = keine Vorschau moeglich
        private readonly Action<int, int, int> _vorschau;

        public FormularFeldDialog(FormularFeldDef feld,
            Action<int, int, int> vorschau = null)
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

            // erst NACH dem Vorbelegen setzen -- die TextChanged-Ereignisse
            // der Initialwerte sollen keine Vorschau ausloesen
            _vorschau = vorschau;

            // Position merken wie die nativen Dialoge -- man schiebt den
            // Dialog einmal neben das Formular und er bleibt dort
            FensterPosition.Verwalte(this, "Formularfeld");

            Loaded += (s, e) => { IdBox.Focus(); IdBox.SelectAll(); };
        }

        // ----------------------------------------------------------
        // Position verschieben (Pendant OnDeltapos*Spin): Pfeilknoepfe
        // und Alt+Pfeiltasten, Umschalt = 10er-Schritte wie die
        // Pfeiltasten im Designer (FormularView.OnPreviewKeyDown).
        // ----------------------------------------------------------

        private static int Schritt =>
            (Keyboard.Modifiers & ModifierKeys.Shift) != 0 ? 10 : 1;

        private static void Verschiebe(TextBox box, int delta, int minimum)
        {
            int wert = FormularDefinition.Atoi(box.Text) + delta;
            if (wert < minimum) wert = minimum;
            box.Text = wert.ToString(CultureInfo.InvariantCulture);
            box.CaretIndex = box.Text.Length;
        }

        private void OnSeitePlus(object sender, RoutedEventArgs e) => Verschiebe(SeiteBox, +1, 1);
        private void OnSeiteMinus(object sender, RoutedEventArgs e) => Verschiebe(SeiteBox, -1, 1);
        private void OnLinks(object sender, RoutedEventArgs e) => Verschiebe(HorizontalBox, -Schritt, 0);
        private void OnRechts(object sender, RoutedEventArgs e) => Verschiebe(HorizontalBox, +Schritt, 0);
        private void OnHoch(object sender, RoutedEventArgs e) => Verschiebe(VertikalBox, -Schritt, 0);
        private void OnRunter(object sender, RoutedEventArgs e) => Verschiebe(VertikalBox, +Schritt, 0);

        // Alt(+Umschalt)+Pfeil statt Strg: Strg+Pfeil bleibt das wortweise
        // Springen in den Textfeldern, und Alt+Pfeil hoch/runter ist aus
        // VS/VS Code/Word als "Zeile/Absatz verschieben" vertraut.
        // Alt kommt in WPF als Key.System herein, die echte Taste steht in
        // SystemKey (siehe BuchungView.EchteTaste).
        private void OnPreviewKeyDown(object sender, KeyEventArgs e)
        {
            var mod = Keyboard.Modifiers & ~ModifierKeys.Shift;
            if (mod != ModifierKeys.Alt) return;   // Strg+Alt waere AltGr

            var taste = e.Key == Key.System ? e.SystemKey : e.Key;

            // In einer ComboBox oeffnet/schliesst Alt+Pfeil hoch/runter die
            // Liste -- das bleibt ihr (F4 ginge zwar auch, ist aber weniger
            // bekannt).
            if ((taste == Key.Up || taste == Key.Down) && FokusInComboBox())
                return;

            switch (taste)
            {
                case Key.Left: OnLinks(sender, e); break;
                case Key.Right: OnRechts(sender, e); break;
                case Key.Up: OnHoch(sender, e); break;
                case Key.Down: OnRunter(sender, e); break;
                default: return;
            }
            e.Handled = true;
        }

        private void OnPreviewKeyUp(object sender, KeyEventArgs e)
        {
            // Losgelassenes Alt NICHT an DefWindowProc durchreichen -- sonst
            // SC_KEYMENU, und der Dialog (ohne Menueleiste) haengt unsichtbar
            // im Systemmenue: das naechste Return klappt es auf statt zu
            // speichern (gleicher Fallstrick wie BuchungView).
            var taste = e.Key == Key.System ? e.SystemKey : e.Key;
            if (taste == Key.LeftAlt || taste == Key.RightAlt)
                e.Handled = true;
        }

        private static bool FokusInComboBox()
        {
            var element = Keyboard.FocusedElement as DependencyObject;
            while (element != null)
            {
                if (element is ComboBox) return true;
                element = element is System.Windows.Media.Visual
                    ? System.Windows.Media.VisualTreeHelper.GetParent(element)
                    : LogicalTreeHelper.GetParent(element);
            }
            return false;
        }

        private void OnPositionGeaendert(object sender, TextChangedEventArgs e)
        {
            _vorschau?.Invoke(
                FormularDefinition.Atoi(SeiteBox.Text),
                FormularDefinition.Atoi(HorizontalBox.Text),
                FormularDefinition.Atoi(VertikalBox.Text));
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
