// KontoAnlegenView.xaml.cs -- Eingabemaske des Ad-hoc-Kontoselektors
//
// Erscheint, wenn eine Buchungsgruppen-Vorlage (Konto-Feld mit
// "$de:Formular=Id|...||"-Spezifikation) bzw. ein Plugin über
// HoleKontoMitFeldern ein Konto mit bestimmten Formularfeld-Verknüpfungen
// braucht, aber keines existiert. Es muss nur der Kontoname eingegeben
// werden (vorbelegt mit dem "$name="-Vorschlag der Spezifikation, sonst mit
// den Feldnamen, " / "-getrennt); die Feld-Verknüpfungen und der E/A-Typ
// stehen durch die Spezifikation fest.
//
// Die Anlage selbst macht KontoFeldSelektor.ErzeugeKonto (Engine):
// Namens-Duplikate werden inline gemeldet (Dialog bleibt offen), volle
// 100 Slots als Fehlermeldung-MessageBox (Dialog schließt ohne Ergebnis).

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Interop;
using ECTEngine;

namespace ECTViews.Stammdaten
{
    public partial class KontoAnlegenView : Window
    {
        private readonly IReadOnlyList<KontoFeldBedarf> _bedarf;
        private readonly bool _istEinnahme;

        /// <summary>Name des angelegten Kontos (nach erfolgreichem OK).</summary>
        public string ErzeugtesKonto { get; private set; }

        private KontoAnlegenView(IReadOnlyList<KontoFeldBedarf> bedarf,
            KontoFeldInfo info, string nameVorschlag)
        {
            InitializeComponent();
            _bedarf = bedarf;
            _istEinnahme = info.IstEinnahme;
            // Hinweistext mit Feld-Kennzeichen ("... (Feld 47)"), damit
            // erkennbar bleibt, fuer WELCHE Vorlagen-Zeile das Konto gebraucht
            // wird; der Vorgabe-Name bleibt die reine Bezeichnung.
            HinweisText.Text = KontoFeldSelektor.HinweisText(info.FeldnamenMitKennzeichen);
            NameBox.Text = KontoFeldSelektor.VorgabeName(info.Feldnamen, nameVorschlag);
            Loaded += (s, e) => { NameBox.Focus(); NameBox.SelectAll(); };
        }

        // ------------------------------------------------------------------
        // Statische Einstiege
        // ------------------------------------------------------------------

        /// <summary>
        /// Zeigt den Anlage-Dialog für den Feld-Bedarf einer Spezifikation.
        /// Liefert den Namen des angelegten Kontos oder null (abgebrochen,
        /// Feld-Infos nicht ermittelbar oder alle Slots belegt -- die beiden
        /// letzten Fälle mit Fehlermeldung-MessageBox).
        /// <paramref name="nameVorschlag"/> ist der "$name="-Wert der
        /// Spezifikation ("" = keiner, dann Vorbelegung aus den Feldnamen).
        /// </summary>
        public static string ZeigeDialog(IReadOnlyList<KontoFeldBedarf> bedarf,
            Window owner = null, IntPtr ownerHwnd = default,
            string nameVorschlag = null)
        {
            var info = KontoFeldSelektor.ErmittleFeldInfo(bedarf);
            if (info.Fehler.Length > 0)
            {
                MessageBox.Show(info.Fehler, "Konto anlegen",
                    MessageBoxButton.OK, MessageBoxImage.Warning);
                return null;
            }

            var dlg = new KontoAnlegenView(bedarf, info, nameVorschlag);
            if (owner != null)
                dlg.Owner = owner;
            else if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(dlg) { Owner = ownerHwnd };

            dlg.ShowDialog();
            return dlg.ErzeugtesKonto;
        }

        // ------------------------------------------------------------------
        // Handler
        // ------------------------------------------------------------------

        private void OnAnlegen(object sender, RoutedEventArgs e)
        {
            if (KontoFeldSelektor.ErzeugeKonto(
                    NameBox.Text, _istEinnahme, _bedarf, out string fehler))
            {
                ErzeugtesKonto = NameBox.Text.Trim();
                Statusleiste.Melde((_istEinnahme
                        ? "Einnahmenkonto \"" : "Ausgabenkonto \"")
                    + ErzeugtesKonto + "\" mit Feld-Verknüpfungen angelegt.");
                DialogResult = true;
                return;
            }

            // Volle Slots: MessageBox + Dialog erfolglos schließen;
            // alles andere (leerer/vergebener Name) inline melden.
            if (fehler.Contains("100"))
            {
                MessageBox.Show(fehler, "Konto anlegen",
                    MessageBoxButton.OK, MessageBoxImage.Error);
                DialogResult = false;
                return;
            }
            FehlerText.Text = fehler;
            FehlerText.Visibility = Visibility.Visible;
        }

        private void OnNameGeaendert(object sender,
            System.Windows.Controls.TextChangedEventArgs e)
        {
            FehlerText.Visibility = Visibility.Collapsed;
        }
    }
}
