using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;

namespace ECTViews.EinstellungenUi
{
    public partial class EinstellungenView : UserControl
    {
        public EinstellungenView()
        {
            InitializeComponent();

            // ----------------------------------------------------------
            // Eingaben in Textfeldern zuverlaessig uebernehmen.
            //
            // Die Seiten binden Textfelder mit UpdateSourceTrigger=LostFocus
            // (Default von TextBox.Text). LostFocus ist aber LOGISCHER Fokus:
            // klickt man aus einem Textfeld heraus in ein MFC-Fenster (Ribbon,
            // Buchen-Knopf, anderes Dokument) oder schliesst die eingebettete
            // Ansicht, verliert WPF nur den TASTATUR-Fokus -- der logische Fokus
            // bleibt im Feld, die Bindung schreibt nie zurueck. Beispiel: ein
            // umbenanntes Konto kam weder in der Kontenliste des Buchen-Dialogs
            // noch sonstwo an. Darum zusaetzlich bei Tastatur-Fokusverlust und
            // bei Enter (einzeilige Felder) uebernehmen; beim Abloesen der
            // Ansicht ruft EinstellungenEmbed UebernimmOffeneEingabe().
            // ----------------------------------------------------------
            AddHandler(LostKeyboardFocusEvent,
                new KeyboardFocusChangedEventHandler((s, e) => Uebernimm(e.OriginalSource as TextBox)), true);
            AddHandler(PreviewKeyDownEvent, new KeyEventHandler((s, e) =>
            {
                if (e.Key == Key.Enter && e.OriginalSource is TextBox tb && !tb.AcceptsReturn)
                    Uebernimm(tb);
            }), true);
        }

        /// <summary>Schreibt den Inhalt des Textfelds mit dem (logischen)
        /// Fokus zurück -- vor dem Abbau der Ansicht aufrufen.</summary>
        public void UebernimmOffeneEingabe()
        {
            var fokus = Keyboard.FocusedElement as TextBox;
            if (fokus == null || !IsAncestorOf(fokus))
            {
                var bereich = FocusManager.GetFocusScope(this);
                fokus = bereich == null ? null : FocusManager.GetFocusedElement(bereich) as TextBox;
            }
            if (fokus != null && IsAncestorOf(fokus)) Uebernimm(fokus);
        }

        private static void Uebernimm(TextBox tb)
        {
            var be = tb?.GetBindingExpression(TextBox.TextProperty);
            if (be == null) return;
            var trigger = be.ParentBinding.UpdateSourceTrigger;
            // Explicit bleibt dem Besitzer ueberlassen, PropertyChanged ist schon geschrieben.
            if (trigger == UpdateSourceTrigger.LostFocus || trigger == UpdateSourceTrigger.Default)
                be.UpdateSource();
        }
    }
}
