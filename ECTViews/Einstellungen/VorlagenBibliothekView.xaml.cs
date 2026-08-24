// VorlagenBibliothekView.xaml.cs -- Auswahldialog der mitgelieferten
// Vorlagen-Bibliothek.
//
// Der Dialog waehlt nur aus; das Uebernehmen in einen freien Vorlagen-Platz
// macht der Aufrufer ueber denselben Weg wie der Datei-Import
// (PresetsPageViewModel.ImportiereXml). Ergebnis ist deshalb schlicht das
// XML der gewaehlten Vorlage.

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace ECTViews.EinstellungenUi
{
    public partial class VorlagenBibliothekView : Window
    {
        private readonly VorlagenBibliothekViewModel _vm;

        /// <summary>XML der uebernommenen Vorlage; null bei Abbruch.</summary>
        public string GewaehltesVorlagenXml { get; private set; }

        /// <summary>Gewaehlter Ziel-Platz; -1 = erster freier.</summary>
        public int GewaehlterSlot { get; private set; } = -1;

        private VorlagenBibliothekView(IReadOnlyList<int> freieSlots)
        {
            InitializeComponent();
            _vm = new VorlagenBibliothekViewModel(freieSlots);
            DataContext = _vm;
            Loaded += (s, e) => SuchBox.Focus();
        }

        /// <summary>
        /// Zeigt die Bibliothek. Liefert das XML der gewaehlten Vorlage oder
        /// null (abgebrochen); <paramref name="zielSlot"/> traegt den im
        /// Dialog gewaehlten Vorlagen-Platz. <paramref name="kontext"/> ist
        /// die aufrufende View -- daraus ermittelt DialogBesitzer den
        /// Besitzer, auch wenn die View im MFC-Rahmen gehostet ist.
        /// </summary>
        public static string ZeigeDialog(Visual kontext,
            IReadOnlyList<int> freieSlots, out int zielSlot)
        {
            var dlg = new VorlagenBibliothekView(freieSlots);
            DialogBesitzer.Setze(dlg, kontext);
            dlg.ShowDialog();
            zielSlot = dlg.GewaehlterSlot;
            return dlg.GewaehltesVorlagenXml;
        }

        // ------------------------------------------------------------------
        // Handler
        // ------------------------------------------------------------------

        private void OnAuswahlGeaendert(object sender,
            RoutedPropertyChangedEventArgs<object> e)
        {
            _vm.Auswahl = e.NewValue as BibliothekKnoten;
        }

        private void OnBaumDoppelklick(object sender, MouseButtonEventArgs e)
        {
            // Nur der Doppelklick AUF eine Vorlage uebernimmt; auf einem
            // Abschnitt bleibt es beim Auf-/Zuklappen durch die TreeView.
            if (!(e.OriginalSource is DependencyObject quelle)) return;
            var item = SucheTreeViewItem(quelle);
            if (item?.DataContext is BibliothekKnoten k && k.IstEintrag)
            {
                e.Handled = true;
                Uebernimm();
            }
        }

        private void OnUebernehmen(object sender, RoutedEventArgs e) => Uebernimm();

        private void Uebernimm()
        {
            var xml = _vm.GewaehltesVorlagenXml;
            if (xml == null) return;
            GewaehltesVorlagenXml = xml;
            GewaehlterSlot = _vm.GewaehlterSlot;
            DialogResult = true;
        }

        private static TreeViewItem SucheTreeViewItem(DependencyObject d)
        {
            while (d != null && !(d is TreeViewItem))
                d = System.Windows.Media.VisualTreeHelper.GetParent(d);
            return d as TreeViewItem;
        }
    }
}
