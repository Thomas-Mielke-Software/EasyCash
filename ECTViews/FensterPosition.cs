// FensterPosition.cs -- merkt sich die Position von WPF-Fenstern im
// App-Profil, so wie die nativen Dialoge es tun
// (buchendlg.cpp: [Fenster]BuchenPosX/-Y, dauerbuchungendlg.cpp: ...).
//
// Das App-Profil (Registry unter "Thomas Mielke Softwareentwicklung",
// per theApp.Get/WriteProfileInt) ist nur nativ erreichbar. Die Bridge
// haengt darum beim Einbetten zwei Delegates ein (ProfilIntLesen/
// ProfilIntSchreiben, siehe ECTBridge/AppProfilShared.h). Fehlen sie --
// Tests, Designer, alte Bridge --, oeffnet das Fenster einfach an seiner
// Standardposition.
//
// Bewusst Profil statt easyct.ini: die ini gehoert zum Datenverzeichnis
// (evtl. Netzlaufwerk, mehrere Rechner mit anderen Monitoren), eine
// Fensterposition dagegen zum Arbeitsplatz.
//
// Einheit: WPF-DIP (Window.Left/Top/Width/Height). Die Schluessel sind
// eigene, darum keine Verwechslung mit den Pixel-Werten der nativen
// Dialoge.

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

        /// <summary>Spielraum fuer die unsichtbaren Fensterraender (DIP),
        /// um den ein Fenster groesser als der Bildschirm sein darf.</summary>
        private const double RandToleranz = 32;

        /// <summary>
        /// Stellt die gemerkte Position wieder her (falls vorhanden und noch
        /// auf einem Bildschirm) und merkt sie sich. Im Konstruktor bzw. vor
        /// Show/ShowDialog aufrufen.
        /// </summary>
        /// <param name="schluessel">Praefix der Profil-Keys, z.B.
        /// "Formularfeld" -> FormularfeldPosX/FormularfeldPosY.</param>
        /// <param name="mitGroesse">Auch Breite/Hoehe und den Maximiert-
        /// Zustand merken (...Breite/...Hoehe/...Maximiert) -- fuer Fenster,
        /// deren Groesse der Anwender selbst einstellt.</param>
        public static void Verwalte(Window fenster, string schluessel, bool mitGroesse = false)
        {
            if (fenster == null || string.IsNullOrEmpty(schluessel)) return;

            string keyX = schluessel + "PosX", keyY = schluessel + "PosY";
            string keyB = schluessel + "Breite", keyH = schluessel + "Hoehe";
            string keyM = schluessel + "Maximiert";
            var lesen = ProfilIntLesen;
            if (lesen != null)
            {
                try
                {
                    if (mitGroesse)
                    {
                        int b = lesen(Sektion, keyB, NichtGesetzt);
                        int h = lesen(Sektion, keyH, NichtGesetzt);
                        // Nach oben nur KAPPEN, nicht verwerfen: das
                        // Fensterrechteck enthaelt unter Windows 10/11 die
                        // unsichtbaren Ziehraender (je ~7 px). Ein vertikal
                        // maximiertes Fenster (obere Kante an den Bildschirm-
                        // rand gezogen, Aero Snap) ist darum HOEHER als der
                        // Bildschirm -- die fruehere Pruefung "nicht groesser
                        // als der Bildschirm" verwarf genau diese Groesse.
                        if (b != NichtGesetzt && h != NichtGesetzt && b > 0 && h > 0)
                        {
                            fenster.Width = Math.Max(fenster.MinWidth,
                                Math.Min(b, SystemParameters.VirtualScreenWidth + RandToleranz));
                            fenster.Height = Math.Max(fenster.MinHeight,
                                Math.Min(h, SystemParameters.VirtualScreenHeight + RandToleranz));
                        }
                    }

                    int x = lesen(Sektion, keyX, NichtGesetzt);
                    int y = lesen(Sektion, keyY, NichtGesetzt);
                    bool positionGilt = x != NichtGesetzt && y != NichtGesetzt && AufBildschirm(x, y);
                    if (positionGilt)
                    {
                        fenster.WindowStartupLocation = WindowStartupLocation.Manual;
                        fenster.Left = x;
                        fenster.Top = y;
                    }
                    // Maximiert erst NACH der Position: WPF maximiert auf dem
                    // Bildschirm, auf dem die (normale) Position liegt.
                    if (mitGroesse && positionGilt && lesen(Sektion, keyM, 0) == 1)
                        fenster.WindowState = WindowState.Maximized;
                }
                catch { /* Profil nicht lesbar: Standardposition */ }
            }

            // Gemerkt wird die NORMALE Lage des Fensters. Ist es maximiert
            // (oder minimiert), steht sie in RestoreBounds -- Left/Top/
            // ActualWidth gaeben dann die Bildschirmflaeche bzw. Unsinn wieder.
            // Frueher wurde in diesem Fall gar nichts gespeichert; ein
            // maximiert geschlossenes Fenster oeffnete darum wieder in der
            // alten Groesse an der alten Stelle.
            void Merke()
            {
                var schreiben = ProfilIntSchreiben;
                if (schreiben == null) return;

                Rect lage;
                if (fenster.WindowState == WindowState.Normal)
                    lage = new Rect(fenster.Left, fenster.Top, fenster.ActualWidth, fenster.ActualHeight);
                else
                    lage = fenster.RestoreBounds;
                if (lage.IsEmpty || double.IsNaN(lage.Left) || double.IsNaN(lage.Top)
                    || double.IsInfinity(lage.Left) || double.IsInfinity(lage.Top))
                    return;

                try
                {
                    schreiben(Sektion, keyX, (int)Math.Round(lage.Left));
                    schreiben(Sektion, keyY, (int)Math.Round(lage.Top));
                    if (!mitGroesse) return;
                    if (lage.Width > 0 && lage.Height > 0
                        && !double.IsInfinity(lage.Width) && !double.IsInfinity(lage.Height))
                    {
                        schreiben(Sektion, keyB, (int)Math.Round(lage.Width));
                        schreiben(Sektion, keyH, (int)Math.Round(lage.Height));
                    }
                    // Minimiert sagt nichts darueber, wie es wieder aufgehen
                    // soll -- dann den bisherigen Merker stehen lassen.
                    if (fenster.WindowState != WindowState.Minimized)
                        schreiben(Sektion, keyM,
                            fenster.WindowState == WindowState.Maximized ? 1 : 0);
                }
                catch { /* Profil nicht schreibbar: Position eben nicht gemerkt */ }
            }

            fenster.Closed += (s, e) => Merke();
            // Auch beim Wechsel in ein anderes Fenster: ein nicht-modales
            // Fenster (Anleitung) ist beim Beenden des Programms oft noch
            // offen und bekommt dann kein Closed mehr.
            fenster.Deactivated += (s, e) => Merke();
        }

        /// <summary>Liegt die Titelleiste noch greifbar auf dem virtuellen
        /// Bildschirm? (Monitor abgesteckt, Aufloesung verkleinert -- sonst
        /// oeffnete das Fenster unerreichbar im Off.)</summary>
        private static bool AufBildschirm(int x, int y)
        {
            double links = SystemParameters.VirtualScreenLeft;
            double oben = SystemParameters.VirtualScreenTop;
            double rechts = links + SystemParameters.VirtualScreenWidth;
            double unten = oben + SystemParameters.VirtualScreenHeight;
            // mind. 100 DIP der Titelleiste sichtbar; oben ein paar DIP
            // Spielraum, weil ein angedocktes Fenster mit seinem unsichtbaren
            // Rand leicht ueber den Bildschirmrand ragen kann
            return x + 100 <= rechts && x >= links - 400
                && y >= oben - RandToleranz && y + 30 <= unten;
        }
    }
}
