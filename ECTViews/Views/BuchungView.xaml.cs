using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Threading;
using ECTViews.ViewModels;

namespace ECTViews.Views
{
    public partial class BuchungView : Window
    {
        // Timer, der den Notiz-Balloon nach ein paar Sekunden wieder schliesst.
        private readonly DispatcherTimer _notizTimer;

        public BuchungView()
        {
            InitializeComponent();

            _notizTimer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(6)
            };
            _notizTimer.Tick += (s, e) =>
            {
                _notizTimer.Stop();
                NotizBalloon.IsOpen = false;
            };

            // Balloon schliessen, wenn das Fenster geschlossen wird.
            Closed += (s, e) => { _notizTimer.Stop(); NotizBalloon.IsOpen = false; };

            // Alt-Kuerzel der Icon-Listen (siehe OnFensterPreviewKeyDown).
            PreviewKeyDown += OnFensterPreviewKeyDown;
            PreviewKeyUp += OnFensterPreviewKeyUp;
            // Mit gedruecktem Alt weggeschaltet (Alt+Tab): das KeyUp kommt
            // dann nicht mehr an, die Schilder blieben sonst stehen.
            Deactivated += (s, e) => VersteckeAltHinweise();
        }

        public BuchungView(BuchungViewModel viewModel) : this()
        {
            DataContext = viewModel;
            viewModel.RequestClose += () => Close();
            viewModel.PresetNotizAnzeigen += ZeigeNotizBalloon;
            viewModel.RequestFokus += OnRequestFokus;

            // Ad-hoc-Kontoselektor: fehlt das per Feld-Spezifikation
            // ("$de:Formular=Id|...||") geforderte Konto, fragt der
            // Anlage-Dialog nach dem Kontonamen.
            viewModel.KontoAnlegenAbfrage = (bedarf, nameVorschlag) =>
                Stammdaten.KontoAnlegenView.ZeigeDialog(bedarf, owner: this,
                    nameVorschlag: nameVorschlag);

            // DSGVO-Einwilligung vor der ersten Online-Kursabfrage.
            viewModel.ApiEinwilligungAbfrage = WaehrungApiEinwilligung.Sicherstellen;

            // Sicherheitsabfrage, wenn die Wahl einer Vorlage im Bearbeiten-
            // Modus die Buchung in eine Buchungsgruppe umwandelt, eine Gruppe
            // umstellt oder auflöst.
            viewModel.UmwandlungBestaetigen = (text, titel) =>
                MessageBox.Show(this, text, titel,
                    MessageBoxButton.YesNo, MessageBoxImage.Question)
                == MessageBoxResult.Yes;

            // Auch beim erstmaligen Oeffnen die "Weiterbuchen-Verhalten"-
            // Einstellung zum Cursor beachten (nur bei neuer Buchung, nicht
            // beim Bearbeiten/Kopieren).
            Loaded += OnInitialFokus;

            // Ueber das Ribbon-Dropdown vorgewaehlte Buchungsvorlage laden --
            // erst wenn das Fenster steht, damit ein evtl. noetiger
            // "Konto anlegen"-Dialog einen sichtbaren Owner hat.
            Loaded += OnVorwahlLaden;
        }

        /// <summary>Laedt eine ueber das Ribbon-Dropdown vorgewaehlte Vorlage,
        /// sobald das Fenster geladen ist (siehe
        /// <see cref="BuchungViewModel.VorgewaehltesPreset"/>).</summary>
        private void OnVorwahlLaden(object sender, RoutedEventArgs e)
        {
            Loaded -= OnVorwahlLaden;   // nur einmal
            if (DataContext is BuchungViewModel vm)
                vm.LadeVorgewaehlteVorlage();
        }

        /// <summary>Setzt beim Oeffnen des Dialogs den Anfangsfokus gemaess der
        /// Einstellung "Tagesdatum einfügen und Cursor ins Betragsfeld": ins
        /// Betrag- oder sonst ins Tag-Feld -- aber nur bei einer neuen Buchung.</summary>
        private void OnInitialFokus(object sender, RoutedEventArgs e)
        {
            Loaded -= OnInitialFokus;   // nur einmal
            if (DataContext is BuchungViewModel vm && vm.WeiterbuchenSichtbar)
                OnRequestFokus(vm.CursorInsBetragsfeld);
        }

        /// <summary>Setzt nach "Weiterbuchen" den Fokus: auf das Betrag-Feld
        /// (Einstellung "Tagesdatum einfügen und Cursor ins Betragsfeld") oder
        /// sonst auf das Tag-Feld. Verzoegert, damit das Re-Init der Maske
        /// abgeschlossen ist, bevor der Fokus gesetzt wird.</summary>
        private void OnRequestFokus(bool aufBetrag)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                var box = aufBetrag ? BetragBox : TagBox;
                box.Focus();
                box.SelectAll();
            }), DispatcherPriority.Background);
        }

        /// <summary>Selektiert beim Fokus-Erhalt den vorhandenen Feldinhalt
        /// (Datum, Betrag, Beschreibung, Beleg), damit Tippen ihn direkt
        /// ersetzt. Greift nur bei nicht-leerem Feld.</summary>
        private void OnFeldGotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (sender is TextBox box && box.Text.Length > 0)
                box.SelectAll();
        }

        /// <summary>Sorgt dafuer, dass die Vorselektion auch beim Fokussieren
        /// per Mausklick greift: der erste Klick setzt nur den Fokus (WPF
        /// wuerde sonst sofort den Cursor platzieren und die Selektion
        /// aufheben); ein weiterer Klick positioniert dann normal.</summary>
        private void OnFeldPreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (sender is TextBox box && !box.IsKeyboardFocusWithin)
            {
                box.Focus();
                e.Handled = true;
            }
        }

        // ----------------------------------------------------------
        // Alt-Kuerzel fuer die Icon-Listen
        //
        // Alt+1..Alt+9 und Alt+0 waehlen die ersten ZEHN Bestandskonten,
        // Strg+Alt+<Ziffer> die ersten zehn Betriebe (Pendant zu
        // IDC_ALT1..IDC_ALT6 im MFC-Original, dort als kleine Knoepfe unter
        // den Listen). Solange Alt gedrueckt ist, zeigen die Listeneintraege
        // ihr Kuerzel.
        //
        // Alt kommt in WPF nicht als normaler Tastendruck herein: e.Key ist
        // dann Key.System, die eigentliche Taste steht in e.SystemKey.
        // ----------------------------------------------------------

        /// <summary>Die tatsaechlich gedrueckte Taste -- bei gehaltenem Alt
        /// liefert WPF Key.System und die echte Taste in SystemKey.</summary>
        private static Key EchteTaste(KeyEventArgs e)
        {
            return e.Key == Key.System ? e.SystemKey : e.Key;
        }

        /// <summary>Ziffer 0-9 der Taste, sonst -1 (nur die Zifferreihe --
        /// der Ziffernblock bleibt frei fuer Alt+Zahlencode-Eingaben).</summary>
        private static int ZifferAus(Key taste)
        {
            if (taste >= Key.D0 && taste <= Key.D9) return taste - Key.D0;
            return -1;
        }

        /// <summary>
        /// True, wenn die gedrueckte Strg+Alt-Kombination in Wahrheit AltGr
        /// ist. Windows meldet AltGr als linkes Strg PLUS rechtes Alt -- ohne
        /// diese Unterscheidung wuerde die deutsche Tastatur ihre AltGr-Zeichen
        /// verlieren (AltGr+8 = '[', AltGr+7 = '{', AltGr+3 = '³' ...), weil
        /// wir sie als Betriebs-Kuerzel wegfangen.
        /// </summary>
        private static bool IstAltGr()
        {
            return Keyboard.IsKeyDown(Key.RightAlt);
        }

        private void OnFensterPreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (!(DataContext is BuchungViewModel vm)) return;

            var taste = EchteTaste(e);

            if (taste == Key.LeftAlt || taste == Key.RightAlt)
            {
                vm.AltHinweiseSichtbar = true;
                return;
            }

            if ((Keyboard.Modifiers & ModifierKeys.Alt) != ModifierKeys.Alt) return;

            int ziffer = ZifferAus(taste);
            if (ziffer < 0) return;

            bool mitStrg = (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control;
            if (mitStrg && IstAltGr()) return;   // AltGr-Zeichen durchlassen

            vm.WaehleUeberHotkey(ziffer, betrieb: mitStrg);
            // Auch dann als erledigt melden, wenn die Liste den Eintrag nicht
            // hat: sonst quittiert Windows das Kuerzel mit einem Piepton.
            e.Handled = true;
        }

        private void OnFensterPreviewKeyUp(object sender, KeyEventArgs e)
        {
            var taste = EchteTaste(e);
            if (taste != Key.LeftAlt && taste != Key.RightAlt) return;

            VersteckeAltHinweise();

            // Losgelassenes Alt NICHT an DefWindowProc durchreichen: Windows
            // macht daraus SC_KEYMENU und schaltet das Fenster in den
            // Menue-Modus. Weil dieses Fenster keine Menueleiste hat, landet
            // die Auswahl auf dem SYSTEMMENUE -- unsichtbar, bis der naechste
            // Druck auf Return/Leertaste/Pfeil-ab es aufklappt. Genau das
            // passierte nach einem Blick auf die Alt-Kuerzel: das naechste
            // Return im Beschreibungsfeld oeffnete das Fenstermenue statt zu
            // buchen. Alt+Leertaste erreicht das Systemmenue weiterhin.
            e.Handled = true;
        }

        /// <summary>Blendet die Kuerzel-Schilder wieder aus. Auch noetig, wenn
        /// das Fenster mit gedruecktem Alt verlassen wird (Alt+Tab) -- das
        /// KeyUp kommt dann nie an.</summary>
        private void VersteckeAltHinweise()
        {
            if (DataContext is BuchungViewModel vm)
                vm.AltHinweiseSichtbar = false;
        }

        // ----------------------------------------------------------
        // Automatischer Feldwechsel in der Datumszeile
        //
        // Pendant zu BuchenDlg::OnChangeDatumTag/-Monat/-Jahr: sobald das Feld
        // voll getippt ist (2 Ziffern bei Tag/Monat, 4 beim Jahr) und der
        // Cursor am Ende steht, springt der Fokus ins naechste Feld. Anders als
        // im Original ist das naechste Feld nach dem Datum die BESCHREIBUNG --
        // sie steht in dieser Maske vor dem Betrag, weil die Wahl einer
        // Buchungsvorlage die MWSt. setzt.
        //
        // Wichtig: nur reagieren, wenn das Feld auch den Tastaturfokus hat.
        // Sonst wuerde ein programmatisches Befuellen der Datumsfelder
        // (SetzeFrischesDatum beim Oeffnen/Weiterbuchen) den Fokus verschieben.
        // ----------------------------------------------------------

        /// <summary>True, wenn das Feld gerade per Tastatur bis zur vollen
        /// Laenge <paramref name="laenge"/> gefuellt wurde und der Cursor ohne
        /// Selektion am Ende steht (Pendant zu GetSel(m,n) mit m==n==Laenge).</summary>
        private static bool FeldVollGetippt(object sender, int laenge)
        {
            var box = sender as TextBox;
            return box != null
                && box.IsKeyboardFocused
                && box.Text.Length == laenge
                && box.SelectionLength == 0
                && box.CaretIndex == laenge;
        }

        private void OnDatumTagTextChanged(object sender, TextChangedEventArgs e)
        {
            if (FeldVollGetippt(sender, 2))
                MonatBox.Focus();
        }

        private void OnDatumMonatTextChanged(object sender, TextChangedEventArgs e)
        {
            if (!FeldVollGetippt(sender, 2)) return;
            // Ist das Jahresfeld gesperrt, wird es uebersprungen (wie nativ).
            if (JahrBox.IsEnabled) JahrBox.Focus();
            else BeschreibungBox.Focus();
        }

        private void OnDatumJahrTextChanged(object sender, TextChangedEventArgs e)
        {
            if (FeldVollGetippt(sender, 4))
                BeschreibungBox.Focus();
        }

        /// <summary>Dropdown-Knopf neben dem Beschreibungsfeld: klappt die
        /// ungefilterte Vorlagen-Liste auf bzw. wieder zu (Pendant zum
        /// ComboBox-Pfeil des MFC-Originals). Der Knopf ist Focusable=False,
        /// darum bleibt der Fokus im Beschreibungsfeld bzw. wird dorthin
        /// gesetzt -- so schliesst das Popup wie gewohnt ueber dessen
        /// LostFocus, wenn der Anwender woanders hinklickt.</summary>
        private void OnVorschlaegeKnopf(object sender, RoutedEventArgs e)
        {
            if (!(DataContext is BuchungViewModel vm)) return;
            if (vm.VorschlaegeOffen)
            {
                vm.VorschlaegeOffen = false;
            }
            else
            {
                vm.OeffneAlleVorschlaege();
                BeschreibungBox.Focus();
            }
        }

        /// <summary>Schliesst die Vorschlagsliste, wenn das Beschreibungsfeld
        /// den Fokus verliert. Verzoegert, damit ein Klick auf einen Vorschlag
        /// noch verarbeitet wird (Selection -> Preset laden), bevor das Popup
        /// zugeht.</summary>
        private void OnBeschreibungLostFocus(object sender, RoutedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                if (DataContext is BuchungViewModel vm)
                    vm.VorschlaegeOffen = false;
            }), DispatcherPriority.Background);
        }

        /// <summary>Waehrungsrechner-Knopf neben dem Betragsfeld: klappt ein
        /// Menue mit den in den Einstellungen ausgewaehlten Waehrungen auf. Die
        /// Auswahl einer Waehrung startet die (asynchrone) Kursabfrage und
        /// Umrechnung. Baut das Menue programmatisch wie das MFC-Original
        /// (OnBnClickedWaehrungsrechner), gespeist aber aus der Whitelist.</summary>
        private void OnWaehrungsrechnerKnopf(object sender, RoutedEventArgs e)
        {
            if (!(DataContext is BuchungViewModel vm)) return;
            if (!(sender is Button knopf)) return;

            var menue = new ContextMenu
            {
                PlacementTarget = knopf,
                Placement = PlacementMode.Bottom
            };

            var aktive = vm.WaehrungenFuerMenue();
            if (aktive == null || aktive.Count == 0)
            {
                menue.Items.Add(new MenuItem
                {
                    Header = "Keine Währungen ausgewählt – bitte unter "
                             + "Einstellungen › Währungsumrechnung auswählen",
                    IsEnabled = false
                });
            }
            else
            {
                foreach (var w in aktive)
                {
                    string code = w.Code;   // fuer die Closure festhalten
                    var item = new MenuItem { Header = w.Code + "  –  " + w.Name };
                    item.Click += async (s2, e2) => await UmrechnungAusfuehren(vm, code);
                    menue.Items.Add(item);
                }
            }

            menue.IsOpen = true;
        }

        /// <summary>Fuehrt die Umrechnung aus und faengt JEDEN Fehler ab, damit
        /// die Buchungsmaske nicht abstuerzt (Issue #21, Befund Darkwing371):
        /// Netz-/Server-/Eingabefehler landen in einer freundlichen Meldung,
        /// die UI bleibt waehrend des asynchronen Abrufs bedienbar.</summary>
        private async Task UmrechnungAusfuehren(BuchungViewModel vm, string code)
        {
            try
            {
                await vm.WaehrungUmrechnenAsync(code);
            }
            catch (Exception ex)
            {
                try
                {
                    MessageBox.Show(this,
                        "Die Währungsumrechnung ist nicht möglich:\n\n" + ex.Message,
                        "Währungsumrechnung",
                        MessageBoxButton.OK, MessageBoxImage.Warning);
                }
                catch { /* Fenster evtl. schon geschlossen -- ignorieren */ }
            }
        }

        /// <summary>Zeigt die Preset-Notiz als Balloon und (re)startet den
        /// Auto-Schliess-Timer.</summary>
        private void ZeigeNotizBalloon(string notiz)
        {
            if (string.IsNullOrWhiteSpace(notiz)) return;
            NotizText.Text = notiz;
            // Neu oeffnen erzwingt das Repositionieren/Fade-In.
            NotizBalloon.IsOpen = false;
            NotizBalloon.IsOpen = true;
            _notizTimer.Stop();
            _notizTimer.Start();
        }
    }
}
