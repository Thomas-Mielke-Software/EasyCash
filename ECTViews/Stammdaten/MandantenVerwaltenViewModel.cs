// MandantenVerwaltenViewModel.cs -- Mandanten-Verwaltung/-Auswahl
// (WPF-Ersatz für CIconAuswahlMandant im Modus 1, mainfrm.cpp
// OnFileMandanten).
//
// Anders als Betriebe/Bestandskonten liegen die Mandanten NICHT in der
// mandantenspezifischen easyct.ini, sondern im Anwendungsprofil
// (theApp.GetProfileString("Mandanten", ...)). Der globale Einstellungs-
// Cache greift hier also nicht: dieses ViewModel arbeitet auf einer vom
// nativen Aufrufer übergebenen In-Memory-Liste; der Aufrufer schreibt
// die (ggf. geänderte) Liste nach Dialogende komplett zurück -- auch
// bei Abbruch, damit Verwaltungs-Änderungen wie im Original erhalten
// bleiben.
//
// Das Property ist das Datenverzeichnis des Mandanten; gewählt wird es
// über den WinForms-FolderBrowserDialog (WPF/.NET 4.8 hat keinen
// eigenen Ordner-Picker) -- Ersatz für SelectFolder/XFolderDialog.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows.Media.Imaging;
using ECTEngine;

namespace ECTViews.Stammdaten
{
    public sealed class MandantenVerwaltenViewModel : StammdatenVerwaltenViewModel
    {
        private readonly List<StammdatenEintrag> _startEintraege
            = new List<StammdatenEintrag>();

        // Aktuelles Daten-/Mandantenverzeichnis (Verzeichnis der easyct.ini) --
        // Vorauswahl-Fallback im Ordner-Picker, wenn der Eintrag selbst noch
        // kein (existierendes) Verzeichnis hat.
        private readonly string _aktuellesDatenverzeichnis;

        public MandantenVerwaltenViewModel(
            IList<string> namen, IList<int> icons, IList<string> datenverzeichnisse,
            string aktuellesDatenverzeichnis = null)
        {
            _aktuellesDatenverzeichnis = aktuellesDatenverzeichnis ?? "";
            int anzahl = namen?.Count ?? 0;
            for (int i = 0; i < anzahl; i++)
            {
                var e = new StammdatenEintrag
                {
                    Name = namen[i],
                    Icon = icons != null && i < icons.Count ? icons[i] : 0
                };
                if (datenverzeichnisse != null && i < datenverzeichnisse.Count
                    && !string.IsNullOrEmpty(datenverzeichnisse[i]))
                    e.Werte[PropertySuffix] = datenverzeichnisse[i];
                _startEintraege.Add(e);
            }
        }

        // Gesetzt, sobald der LETZTE Mandant gelöscht wurde: das
        // Datenverzeichnis dieses Mandanten, das der native Aufrufer im
        // Nicht-Mandanten-Modus als Datenverzeichnis übernehmen soll (sonst
        // null). Wird vom Ergebnis-Objekt an die Bridge weitergereicht.
        public string NichtMandantenModusDatenverzeichnis { get; private set; }

        public override string Titel => "Mandant auswählen";
        public override string NeuKnopfText => "Neuer Mandant...";
        public override string PropertyKnopfText => "Datenverzeichnis...";
        public override string OkKnopfText => "Auswählen";
        public override string AbbrechenKnopfText => "Abbrechen";
        public override string OkKnopfToolTip =>
            "Zum gewählten Mandanten wechseln";
        public override string AbbrechenKnopfToolTip =>
            "Dialog schließen ohne Mandantenwechsel (Änderungen an der Liste bleiben erhalten)";

        // Nur der Vollständigkeit halber (Lade/Persistiere sind überschrieben).
        internal override string Sektion => "Mandanten";
        internal override string Praefix => "Mandant";

        // Mandanten teilen sich Sprite + Icon-Namen mit den Betrieben
        // (IconAuswahlMandant.cpp enthält dieselbe Liste wie IconAuswahlBetrieb.cpp).
        internal override BitmapSource Sprite => ViewHost.SpriteBetriebe;
        internal override string[] IconNamen => IconKatalog.Betriebe;
        internal override string PropertySuffix => "Datenverzeichnis";

        // Wie CIconAuswahl::GetIconText (Default, nicht überschrieben beim
        // Mandanten): Name des gewählten Icons
        internal override string DefaultName(int iconIndex, int listenIndex)
            => iconIndex >= 0 && iconIndex < IconNamen.Length
                ? IconNamen[iconIndex]
                : "Mandant " + (listenIndex + 1).ToString(CultureInfo.InvariantCulture);

        public override void Lade()
        {
            Eintraege.Clear();
            foreach (var e in _startEintraege)
                Eintraege.Add(new StammdatenEintragVM(e, this));
        }

        internal override void Persistiere()
        {
            // Bewusst leer: die Mandanten liegen im App-Profil (Registry),
            // das nur der native Aufrufer erreicht. Er liest die Liste nach
            // Dialogende aus Eintraege/HoleProperty aus und schreibt sie
            // komplett zurück.
        }

        // Moduswechsel-Meldungen beim Löschen des vorletzten bzw. letzten
        // Mandanten. Nach dem Löschen des letzten Mandanten fällt die Software
        // in den Nicht-Mandanten-Modus zurück (leere Mandanten-Sektion) und
        // nutzt dessen Datenverzeichnis künftig direkt als Datenverzeichnis --
        // das setzt der native Aufrufer, hier wird es nur gemeldet und über
        // NichtMandantenModusDatenverzeichnis weitergereicht.
        internal override bool NachLoeschen(System.Windows.Window owner,
            StammdatenEintragVM geloeschter)
        {
            const string titel = "Mandant löschen";

            if (Eintraege.Count == 1)
            {
                // Vorletzter Mandant gelöscht -- vorwarnen, was das Löschen des
                // letzten (verbliebenen) Mandanten auslösen würde.
                var letzter = Eintraege[0];
                var verz = HoleProperty(letzter);
                System.Windows.MessageBox.Show(owner,
                    "Es ist nur noch ein Mandant (\"" + letzter.Name + "\") vorhanden.\n\n" +
                    "Wenn Sie auch diesen löschen, wechselt die Software wieder in den " +
                    "Nicht-Mandanten-Modus und nutzt dessen Mandantenverzeichnis" +
                    (string.IsNullOrEmpty(verz) ? "" : " (\"" + verz + "\")") +
                    " künftig direkt als Datenverzeichnis.",
                    titel, System.Windows.MessageBoxButton.OK,
                    System.Windows.MessageBoxImage.Information);
                return false;
            }

            if (Eintraege.Count == 0)
            {
                // Letzter Mandant gelöscht -- zurück in den Nicht-Mandanten-Modus.
                NichtMandantenModusDatenverzeichnis = HoleProperty(geloeschter) ?? "";
                var verz = NichtMandantenModusDatenverzeichnis;
                System.Windows.MessageBox.Show(owner,
                    "Der letzte Mandant (\"" + geloeschter.Name + "\") wurde gelöscht.\n\n" +
                    "Die Software wechselt wieder in den Nicht-Mandanten-Modus. Dessen " +
                    "Mandantenverzeichnis" +
                    (string.IsNullOrEmpty(verz) ? "" : " (\"" + verz + "\")") +
                    " wird künftig direkt als Datenverzeichnis genutzt.",
                    titel, System.Windows.MessageBoxButton.OK,
                    System.Windows.MessageBoxImage.Information);
                return true;   // Dialog schließen (Moduswechsel übernimmt der Aufrufer)
            }

            return false;
        }

        internal override string EintragToolTip(StammdatenEintragVM eintrag)
        {
            var verzeichnis = HoleProperty(eintrag);
            return string.IsNullOrEmpty(verzeichnis)
                ? eintrag.Name
                : eintrag.Name + "\n" + verzeichnis;
        }

        internal override bool FrageProperty(System.Windows.Window owner,
            string name, string aktuellerWert, out string neuerWert)
        {
            using (var dlg = new System.Windows.Forms.FolderBrowserDialog())
            {
                dlg.Description = $"Datenverzeichnis für \"{name}\" wählen " +
                    "(jeder Mandant benötigt ein eigenes Verzeichnis; im Zweifelsfall " +
                    "einfache ein neues Verzeichnis unter 'Dokumente' erstellen)";
                dlg.ShowNewFolderButton = true;
                // Vorauswahl: bestehendes Verzeichnis des Mandanten, sonst das
                // aktuelle Daten-/Mandantenverzeichnis (typisch legt man neue
                // Mandanten daneben an), sonst "Eigene Dateien" (CSIDL_PERSONAL)
                dlg.SelectedPath = ErsterVorhandenerOrdner(
                    aktuellerWert,
                    _aktuellesDatenverzeichnis,
                    Environment.GetFolderPath(Environment.SpecialFolder.Personal));

                if (dlg.ShowDialog(new Win32Fenster(owner))
                    == System.Windows.Forms.DialogResult.OK)
                {
                    // Abschließenden Backslash abschneiden (wie das Original)
                    neuerWert = dlg.SelectedPath.TrimEnd('\\');
                    return true;
                }
            }
            neuerWert = aktuellerWert;
            return false;
        }

        /// <summary>Liefert den ersten Kandidaten, der ein existierendes
        /// Verzeichnis ist (leer, wenn keiner passt -> Dialog-Default).</summary>
        private static string ErsterVorhandenerOrdner(params string[] kandidaten)
        {
            foreach (var k in kandidaten)
                if (!string.IsNullOrEmpty(k) && System.IO.Directory.Exists(k))
                    return k;
            return "";
        }

        /// <summary>WPF-Window als WinForms-Owner (fürs modale Verhalten
        /// des FolderBrowserDialog).</summary>
        private sealed class Win32Fenster : System.Windows.Forms.IWin32Window
        {
            public IntPtr Handle { get; }
            public Win32Fenster(System.Windows.Window fenster)
                => Handle = new System.Windows.Interop.WindowInteropHelper(fenster).Handle;
        }
    }
}
