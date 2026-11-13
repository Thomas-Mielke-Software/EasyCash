// ZusatzFeldTemplateSelector.cs -- wählt das Eingabe-Control eines
// Zusatz-Felds der Buchungsvorlage
//
// Jede Box des Buchen-Dialogs hat ein ItemsControl für die Zusatz-Felder,
// die die aktive Vorlage dort platziert. Welches Control ein Feld bekommt,
// entscheidet sein Typ (ZusatzFeldViewModel.Steuerelement); die Templates
// selbst stehen in den Resources von BuchungView.xaml.

using System.Windows;
using System.Windows.Controls;
using ECTViews.ViewModels;

namespace ECTViews.Views
{
    public class ZusatzFeldTemplateSelector : DataTemplateSelector
    {
        public DataTemplate Text { get; set; }
        public DataTemplate Zahl { get; set; }
        public DataTemplate Mwst { get; set; }
        public DataTemplate Auswahl { get; set; }
        public DataTemplate Haekchen { get; set; }
        public DataTemplate IconListe { get; set; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (!(item is ZusatzFeldViewModel feld))
                return base.SelectTemplate(item, container);

            switch (feld.Steuerelement)
            {
                case ZusatzFeldSteuerelement.Zahl:      return Zahl;
                case ZusatzFeldSteuerelement.Mwst:      return Mwst;
                case ZusatzFeldSteuerelement.Auswahl:   return Auswahl;
                case ZusatzFeldSteuerelement.Haekchen:  return Haekchen;
                case ZusatzFeldSteuerelement.IconListe: return IconListe;
                default:                                return Text;
            }
        }
    }
}
