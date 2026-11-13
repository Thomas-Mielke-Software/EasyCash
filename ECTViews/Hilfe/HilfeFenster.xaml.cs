// HilfeFenster.xaml.cs -- Fenster für die Benutzer-Anleitungen
//
// Die Anleitungen liegen als Markdown-Dateien in ECTViews/Hilfe und sind
// als Ressourcen in ECTViews.dll eingebettet (wie die Vorlagen-Bibliothek
// in ECTEngine: sie gehören zur Programmversion, es gibt keinen
// Deployment-Pfad, der fehlschlagen könnte). Aufruf:
//
//   Hilfe.HilfeFenster.Zeige("Buchungsvorlagen.md", "Titel", dieseView);

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Threading;

namespace ECTViews.Hilfe
{
    public partial class HilfeFenster : Window
    {
        // Pro Anleitung höchstens ein Fenster: ein zweiter Klick auf
        // "Anleitung..." holt das offene nach vorn.
        private static readonly Dictionary<string, HilfeFenster> _offen =
            new Dictionary<string, HilfeFenster>(StringComparer.OrdinalIgnoreCase);

        private HilfeFenster()
        {
            InitializeComponent();
            // Der Bildlaufbereich des FlowDocumentReader entsteht erst mit
            // seinem Template -- also nach dem Laden suchen.
            Loaded += (s, e) => Dispatcher.BeginInvoke(
                new Action(VerbindeBildlauf), DispatcherPriority.Loaded);

            // Kopieren aus dem Text: WPF ginge über OleFlushClipboard, das
            // auf manchen Systemen scheitert -- dann wäre der kopierte Text
            // nach dem Beenden von EasyCash weg (siehe TextBoxZwischenablage).
            // Preview, weil die eigene Behandlung des Viewers sonst zuerst
            // greift; das Suchfeld (TextBox) erledigt die Klassenbindung.
            System.Windows.Input.CommandManager.AddPreviewExecutedHandler(this, (s, e) =>
            {
                if (e.Command != System.Windows.Input.ApplicationCommands.Copy
                    || e.OriginalSource is TextBox) return;
                var text = Leser.Selection?.Text;
                if (string.IsNullOrEmpty(text)) return;
                TextBoxZwischenablage.SchreibeText(text);
                e.Handled = true;
            });
        }

        /// <summary>
        /// Zeigt die Anleitung <paramref name="datei"/> (Dateiname der
        /// eingebetteten Markdown-Ressource) in einem eigenen Fenster.
        /// </summary>
        /// <remarks>
        /// Bewusst OHNE Besitzer-Fenster und mit Eintrag in der Taskleiste:
        /// ein Fenster mit Besitzer liegt immer ÜBER dem Hauptfenster und
        /// fehlt in Taskleiste und Alt+Tab -- wer die Beispiele nachbaut, will
        /// aber zwischen Anleitung und Programm wechseln oder beide
        /// nebeneinander andocken (Windows+Pfeil). Damit greift auch
        /// DialogBesitzer hier nicht; die Startposition kommt aus dem
        /// Profil bzw. liegt mittig über der aufrufenden View.
        /// </remarks>
        /// <param name="kontext">Die aufrufende View (nur für die erste
        /// Position).</param>
        public static void Zeige(string datei, string titel, Visual kontext)
        {
            if (_offen.TryGetValue(datei, out var vorhanden))
            {
                if (vorhanden.WindowState == WindowState.Minimized)
                    vorhanden.WindowState = WindowState.Normal;
                vorhanden.Activate();
                return;
            }

            var fenster = new HilfeFenster { Title = titel };
            fenster.Lade(LiesAnleitung(datei)
                ?? "# " + titel + "\n\nDie Anleitung \"" + datei
                   + "\" konnte nicht geladen werden (Ressource fehlt).");

            StelleMittigAufBildschirmVon(fenster, kontext);
            FensterPosition.Verwalte(fenster,
                "Anleitung" + System.IO.Path.GetFileNameWithoutExtension(datei),
                mitGroesse: true);
            _offen[datei] = fenster;
            fenster.Closed += (s, e) => _offen.Remove(datei);
            fenster.Show();
        }

        /// <summary>
        /// Erste Position (solange keine gemerkt ist): mittig auf dem
        /// Bildschirm, auf dem das Programm steht. "CenterScreen" nähme
        /// immer den Hauptbildschirm, "CenterOwner" braucht einen Besitzer.
        /// </summary>
        private static void StelleMittigAufBildschirmVon(Window fenster, Visual kontext)
        {
            fenster.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            IntPtr hwnd = DialogBesitzer.Hwnd(kontext);
            if (hwnd == IntPtr.Zero || kontext == null) return;
            try
            {
                var bereich = System.Windows.Forms.Screen.FromHandle(hwnd).WorkingArea;
                var dpi = VisualTreeHelper.GetDpi(kontext);
                double links = bereich.Left / dpi.DpiScaleX, oben = bereich.Top / dpi.DpiScaleY;
                double breite = bereich.Width / dpi.DpiScaleX, hoehe = bereich.Height / dpi.DpiScaleY;
                fenster.Width = Math.Min(fenster.Width, breite);
                fenster.Height = Math.Min(fenster.Height, hoehe);
                fenster.WindowStartupLocation = WindowStartupLocation.Manual;
                fenster.Left = links + (breite - fenster.Width) / 2;
                fenster.Top = oben + (hoehe - fenster.Height) / 2;
            }
            catch { /* dann eben mittig auf dem Hauptbildschirm */ }
        }

        private List<HilfeUeberschrift> _ueberschriften = new List<HilfeUeberschrift>();

        private void Lade(string markdown)
        {
            _ueberschriften = new List<HilfeUeberschrift>();
            Leser.Document = MarkdownDokument.Baue(markdown, _ueberschriften);
            InhaltListe.ItemsSource = _ueberschriften;
        }

        // ------------------------------------------------------------------
        // Inhaltsverzeichnis <-> Text
        //
        // Ein Klick im Inhaltsverzeichnis rollt die Überschrift an den
        // oberen Rand; umgekehrt markiert das Verzeichnis beim Rollen den
        // Abschnitt, in dem man gerade liest (der letzte, dessen Überschrift
        // schon oben herausgerollt ist).
        //
        // Positionen kommen aus TextPointer.GetCharacterRect. Das liefert
        // (gemessen) Koordinaten RELATIV ZUM SICHTBAREN AUSSCHNITT, die mit
        // dem Zoom skalieren: bei 150 % bewegt sich die Überschrift um 750
        // Einheiten, wenn der Bildlauf um 500 weiterrückt. Deshalb wird
        // durch Zoom/100 geteilt.
        // ------------------------------------------------------------------

        private ScrollViewer _bildlauf;

        // Gesetzt, solange die Markierung im Verzeichnis vom Bildlauf kommt
        // -- dann darf die Auswahl nicht zurückspringen.
        private bool _markiereVomBildlauf;

        /// <summary>Abstand vom oberen Rand, ab dem eine Überschrift als
        /// "erreicht" gilt (in Bildlauf-Einheiten).</summary>
        private const double Lesekante = 40;

        private void VerbindeBildlauf()
        {
            if (_bildlauf != null) return;
            _bildlauf = FindeBildlauf(Leser);
            if (_bildlauf == null) return;
            _bildlauf.ScrollChanged += (s, e) => MarkiereAktuellenAbschnitt();
            MarkiereAktuellenAbschnitt();
        }

        private static ScrollViewer FindeBildlauf(DependencyObject d)
        {
            if (d is ScrollViewer sv) return sv;
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(d); i++)
            {
                var treffer = FindeBildlauf(VisualTreeHelper.GetChild(d, i));
                if (treffer != null) return treffer;
            }
            return null;
        }

        /// <summary>Abstand der Überschrift vom oberen Rand des sichtbaren
        /// Ausschnitts in Bildlauf-Einheiten (negativ = schon
        /// herausgerollt); null, wenn das Layout sie nicht kennt.</summary>
        private double? AbstandVonOben(HilfeUeberschrift u)
        {
            if (u?.Block == null) return null;
            var rect = u.Block.ContentStart.GetCharacterRect(LogicalDirection.Forward);
            if (rect.IsEmpty || double.IsInfinity(rect.Y) || double.IsNaN(rect.Y)) return null;
            double zoom = Leser.Zoom > 0 ? Leser.Zoom / 100.0 : 1.0;
            return rect.Y / zoom;
        }

        private void MarkiereAktuellenAbschnitt()
        {
            if (_bildlauf == null || _ueberschriften.Count == 0) return;

            HilfeUeberschrift aktuell = null;
            bool amEnde = _bildlauf.ScrollableHeight > 0
                && _bildlauf.VerticalOffset >= _bildlauf.ScrollableHeight - 1;
            double grenze = amEnde ? _bildlauf.ViewportHeight - Lesekante : Lesekante;

            foreach (var u in _ueberschriften)
            {
                double? abstand = AbstandVonOben(u);
                if (abstand == null) continue;
                if (abstand.Value <= grenze) aktuell = u;
                else break;
            }
            aktuell = aktuell ?? _ueberschriften[0];

            if (ReferenceEquals(InhaltListe.SelectedItem, aktuell)) return;
            _markiereVomBildlauf = true;
            try
            {
                InhaltListe.SelectedItem = aktuell;
                InhaltListe.ScrollIntoView(aktuell);
            }
            finally { _markiereVomBildlauf = false; }
        }

        /// <summary>Rollt den gewählten Abschnitt an den oberen Rand.</summary>
        private void OnInhaltGewaehlt(object sender, SelectionChangedEventArgs e)
        {
            if (_markiereVomBildlauf) return;
            if (!(InhaltListe.SelectedItem is HilfeUeberschrift u)) return;

            VerbindeBildlauf();
            double? abstand = AbstandVonOben(u);
            if (_bildlauf != null && abstand != null)
                _bildlauf.ScrollToVerticalOffset(
                    Math.Max(0, _bildlauf.VerticalOffset + abstand.Value - 4));
            else
                u.Block?.BringIntoView();
        }

        /// <summary>Liest eine eingebettete Anleitung; null, wenn es sie
        /// nicht gibt. Gesucht wird per Namens-Endung, damit der
        /// Standard-Namensraum des Projekts keine Rolle spielt. Public, damit
        /// auch Tests und Werkzeuge an den Text kommen.</summary>
        public static string LiesAnleitung(string datei)
        {
            var asm = typeof(HilfeFenster).Assembly;
            string name = asm.GetManifestResourceNames().FirstOrDefault(n =>
                n.EndsWith("." + datei, StringComparison.OrdinalIgnoreCase)
                || n.Equals(datei, StringComparison.OrdinalIgnoreCase));
            if (name == null) return null;

            using (var strom = asm.GetManifestResourceStream(name))
            {
                if (strom == null) return null;
                using (var leser = new StreamReader(strom, Encoding.UTF8))
                    return leser.ReadToEnd();
            }
        }
    }
}
