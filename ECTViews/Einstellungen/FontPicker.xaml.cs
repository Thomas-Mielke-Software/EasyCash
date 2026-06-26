using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Threading;

namespace ECTViews.EinstellungenUi
{
    /// <summary>
    /// Liefert die Liste der installierten Schriftfamilien -- EINMAL pro Prozess
    /// und ASYNCHRON im Hintergrund aufgebaut. Grund: <see cref="Fonts.SystemFontFamilies"/>
    /// dauert beim ersten Zugriff (kalt) einige Sekunden; früher lief das im
    /// statischen Initializer des FontPickers und blockierte damit das Öffnen
    /// der Einstellungen. Die Collection ist anfangs leer und füllt sich, sobald
    /// der Hintergrund-Task fertig ist -- der aktuell gewählte Schriftname steht
    /// derweil schon im editierbaren Feld.
    /// </summary>
    public static class FontFamilienProvider
    {
        private static readonly ObservableCollection<string> _familien =
            new ObservableCollection<string>();
        private static bool _gestartet;

        public static ObservableCollection<string> Familien => _familien;

        /// <summary>Startet (einmalig) das Hintergrund-Laden der Schriftliste.
        /// Muss vom UI-Thread gerufen werden (merkt sich dessen Dispatcher).</summary>
        public static void Vorwaermen()
        {
            if (_gestartet) return;
            _gestartet = true;

            var dispatcher = Dispatcher.CurrentDispatcher;   // UI-Thread
            Task.Run(() =>
            {
                // Enumeration + Sortierung auf einem Hintergrund-Thread (die
                // FontFamily.Source-Strings sind unkritisch off-thread).
                var liste = Fonts.SystemFontFamilies
                    .Select(f => f.Source)
                    .OrderBy(s => s, StringComparer.CurrentCultureIgnoreCase)
                    .ToList();

                // Befüllen MUSS auf dem UI-Thread passieren (ObservableCollection
                // gehört dem Binding-Thread).
                dispatcher.BeginInvoke(new Action(() =>
                {
                    if (_familien.Count > 0) return;
                    foreach (var s in liste) _familien.Add(s);
                }));
            });
        }
    }

    /// <summary>
    /// Einfacher Schrift-Auswahl-Control: eine editierbare Liste aller
    /// installierten Schriftfamilien (jeder Eintrag in seiner eigenen Schrift
    /// als Vorschau) plus ein Größen-Auswahlfeld. Wird in den Einstellungen
    /// fuer Bildschirm- und Druckerschrift verwendet.
    ///
    /// SchriftName und SchriftGroesse sind TwoWay-bindbar; eine bereits
    /// gespeicherte, aber nicht installierte Schrift bleibt dank des
    /// editierbaren Felds erhalten.
    /// </summary>
    public partial class FontPicker : UserControl
    {
        public FontPicker()
        {
            InitializeComponent();
            // Schriftliste im Hintergrund anstoßen (blockiert das Öffnen nicht).
            FontFamilienProvider.Vorwaermen();
        }

        /// <summary>Tooltip-Hinweis für die Schriftgrößen-Felder: das Ausgabe-
        /// Layout skaliert nicht mit der Schrift, daher manuell passend wählen.</summary>
        public const string GroessenHinweis =
            "Hinweis: Das Layout der Ausgaben passt sich bewusst nicht an die Schrift an, " +
            "deshalb bitte die Schriftgröße manuell so anpassen, dass z.B. im " +
            "Journal nichts abgeschnitten wird.";

        /// <summary>Installierte Schriftfamilien (alphabetisch). Wird vom
        /// <see cref="FontFamilienProvider"/> asynchron befüllt.</summary>
        public IEnumerable<string> FontFamilien => FontFamilienProvider.Familien;

        // Uebliche Schriftgrößen zur Auswahl; 0 = automatische Größe.
        public IEnumerable<int> Groessen { get; } =
            new[] { 0, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 24, 28, 36 };

        /// <summary>Beschriftung links neben der Auswahl (z.B. "Bildschirm").</summary>
        public string Beschriftung
        {
            get => (string)GetValue(BeschriftungProperty);
            set => SetValue(BeschriftungProperty, value);
        }
        public static readonly DependencyProperty BeschriftungProperty =
            DependencyProperty.Register(nameof(Beschriftung), typeof(string),
                typeof(FontPicker), new PropertyMetadata(""));

        /// <summary>Gewählter Schriftfamilien-Name.</summary>
        public string SchriftName
        {
            get => (string)GetValue(SchriftNameProperty);
            set => SetValue(SchriftNameProperty, value);
        }
        public static readonly DependencyProperty SchriftNameProperty =
            DependencyProperty.Register(nameof(SchriftName), typeof(string),
                typeof(FontPicker),
                new FrameworkPropertyMetadata("",
                    FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

        /// <summary>Gewählte Schriftgröße (0 = automatisch).</summary>
        public int SchriftGroesse
        {
            get => (int)GetValue(SchriftGroesseProperty);
            set => SetValue(SchriftGroesseProperty, value);
        }
        public static readonly DependencyProperty SchriftGroesseProperty =
            DependencyProperty.Register(nameof(SchriftGroesse), typeof(int),
                typeof(FontPicker),
                new FrameworkPropertyMetadata(0,
                    FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));
    }
}
