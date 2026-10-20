// FensterPosition.cs -- merkt sich die Position von WPF-Dialogen im
// App-Profil, so wie die nativen Dialoge es tun
// (buchendlg.cpp: [Fenster]BuchenPosX/-Y, dauerbuchungendlg.cpp: ...).
//
// Das App-Profil (Registry unter "Thomas Mielke Softwareentwicklung",
// per theApp.Get/WriteProfileInt) ist nur nativ erreichbar. Die Bridge
// haengt darum beim Einbetten der ersten Ansicht zwei Delegates ein
// (ProfilIntLesen/ProfilIntSchreiben). Fehlen sie -- Tests, Designer,
// alte Bridge --, oeffnet der Dialog einfach an seiner Standardposition.
//
// Bewusst Profil statt easyct.ini: die ini gehoert zum Datenverzeichnis
// (evtl. Netzlaufwerk, mehrere Rechner mit anderen Monitoren), eine
// Fensterposition dagegen zum Arbeitsplatz.
//
// Einheit: WPF-DIP (Window.Left/Top). Die Schluessel sind eigene, darum
// keine Verwechslung mit den Pixel-Werten der nativen Dialoge.

using System;
using System.Windows;

namespace ECTViews
{
    public static class FensterPosition
    {
        /// <summary>(Sektion, Schluessel, Vorgabe) -> Wert; gesetzt von der Bridge.</summary>
        public static Func<string, string, int, int> ProfilIntLesen { get; set; }

        /// <summary>(Sektion, Schluessel, Wert); gesetzt von der Bridge.</summary>
        public static Action<string, string, int> ProfilIntSchreiben { get; set; }

        private const string Sektion = "Fenster";
        private const int NichtGesetzt = int.MinValue;

        /// <summary>
        /// Stellt die gemerkte Position wieder her (falls vorhanden und noch
        /// auf einem Bildschirm) und merkt sie sich beim Schliessen. Im
        /// Konstruktor des Dialogs aufrufen, VOR ShowDialog.
        /// </summary>
        /// <param name="schluessel">Praefix der Profil-Keys, z.B.
        /// "Formularfeld" -> FormularfeldPosX/FormularfeldPosY.</param>
        public static void Verwalte(Window fenster, string schluessel)
        {
            if (fenster == null || string.IsNullOrEmpty(schluessel)) return;

            string keyX = schluessel + "PosX", keyY = schluessel + "PosY";
            var lesen = ProfilIntLesen;
            if (lesen != null)
            {
                try
                {
                    int x = lesen(Sektion, keyX, NichtGesetzt);
                    int y = lesen(Sektion, keyY, NichtGesetzt);
                    if (x != NichtGesetzt && y != NichtGesetzt && AufBildschirm(x, y))
                    {
                        fenster.WindowStartupLocation = WindowStartupLocation.Manual;
                        fenster.Left = x;
                        fenster.Top = y;
                    }
                }
                catch { /* Profil nicht lesbar: Standardposition */ }
            }

            fenster.Closed += (s, e) =>
            {
                var schreiben = ProfilIntSchreiben;
                if (schreiben == null || fenster.WindowState != WindowState.Normal) return;
                if (double.IsNaN(fenster.Left) || double.IsNaN(fenster.Top)) return;
                try
                {
                    schreiben(Sektion, keyX, (int)Math.Round(fenster.Left));
                    schreiben(Sektion, keyY, (int)Math.Round(fenster.Top));
                }
                catch { /* Profil nicht schreibbar: Position eben nicht gemerkt */ }
            };
        }

        /// <summary>Liegt die Titelleiste noch greifbar auf dem virtuellen
        /// Bildschirm? (Monitor abgesteckt, Aufloesung verkleinert -- sonst
        /// oeffnete der Dialog unerreichbar im Off.)</summary>
        private static bool AufBildschirm(int x, int y)
        {
            double links = SystemParameters.VirtualScreenLeft;
            double oben = SystemParameters.VirtualScreenTop;
            double rechts = links + SystemParameters.VirtualScreenWidth;
            double unten = oben + SystemParameters.VirtualScreenHeight;
            // mind. 100 DIP der Titelleiste sichtbar
            return x + 100 <= rechts && x >= links - 400
                && y >= oben && y + 30 <= unten;
        }
    }
}
