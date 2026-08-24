// DialogBesitzerKonventionTests.cs -- Konventions-Wächter für das
// Besitzer-Fenster gehosteter Dialoge.
//
// Hintergrund: Views, die per HwndSource im MFC-Rahmen hängen (Journal,
// Bericht, Formular, Einstellungen), haben KEINEN WPF-Window-Vorfahren.
// Window.GetWindow(this) liefert dort null -- einmal als Absturz
// (MessageBox.Show(null, ...) wirft ArgumentNullException), einmal als
// Dialog, der hinter dem Hauptfenster verschwindet. Beides ist in der
// Praxis passiert.
//
// Die Herleitung des Besitzers gehört deshalb an GENAU EINE Stelle:
// ECTViews/DialogBesitzer.cs. Dieser Test hält die Regel mechanisch fest,
// damit sie nicht beim nächsten neuen Dialog wieder aufgeweicht wird.
//
// Kein Ersatz für Nachdenken, sondern eine Erinnerung an der richtigen
// Stelle: schlägt er an, ist die Frage "brauche ich das wirklich selbst
// oder reicht DialogBesitzer?" -- und wenn ja, gehört die Datei in die
// Ausnahmeliste unten, mit Begründung.

using System;
using System.IO;
using System.Linq;
using Xunit;

namespace ECTEngine.Tests
{
    public class DialogBesitzerKonventionTests
    {
        /// <summary>Dateien, die den Besitzer selbst herleiten dürfen.</summary>
        private static readonly string[] Ausnahmen =
        {
            "DialogBesitzer.cs"   // genau dafür da
        };

        [Fact]
        public void NurDialogBesitzer_LeitetDenBesitzerSelbstHer()
        {
            string views = FindeViewsVerzeichnis();
            if (views == null) return;   // außerhalb des Repos gebaut

            var treffer = Directory
                .GetFiles(views, "*.cs", SearchOption.AllDirectories)
                .Where(d => !Ausnahmen.Contains(Path.GetFileName(d), StringComparer.OrdinalIgnoreCase))
                .Where(d => File.ReadAllText(d).Contains("PresentationSource.FromVisual"))
                .Select(d => d.Substring(views.Length).TrimStart('\\', '/'))
                .ToList();

            Assert.True(treffer.Count == 0,
                "Diese Dateien ermitteln das Besitzer-Fenster selbst statt über "
                + "DialogBesitzer (siehe CLAUDE.md, Win32-Hosting-Stolperfallen):"
                + Environment.NewLine + string.Join(Environment.NewLine, treffer));
        }

        [Fact]
        public void DialogBesitzer_GehtUeberDasTopLevelFenster()
        {
            string views = FindeViewsVerzeichnis();
            if (views == null) return;

            string pfad = Path.Combine(views, "DialogBesitzer.cs");
            Assert.True(File.Exists(pfad), "ECTViews/DialogBesitzer.cs fehlt.");

            // Der eigentliche Kniff: die HwndSource ist ein KIND-Fenster,
            // Besitzer kann nur ein Top-Level-Fenster sein. Fällt GetAncestor
            // weg, ist der Fehler zurück -- und zwar unsichtbar, weil der
            // Dialog erst beim Fokuswechsel nach hinten rutscht.
            string quelle = File.ReadAllText(pfad);
            Assert.Contains("GetAncestor", quelle);
            Assert.Contains("GA_ROOT", quelle);
        }

        private static string FindeViewsVerzeichnis()
        {
            var dir = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
            for (int i = 0; i < 8 && dir != null; i++, dir = dir.Parent)
            {
                string kandidat = Path.Combine(dir.FullName, "ECTViews");
                if (Directory.Exists(kandidat)
                    && File.Exists(Path.Combine(kandidat, "ViewHost.cs")))
                    return kandidat;
            }
            return null;
        }
    }
}
