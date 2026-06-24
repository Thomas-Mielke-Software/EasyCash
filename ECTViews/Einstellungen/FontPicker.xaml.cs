using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace ECTViews.EinstellungenUi
{
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
        }

        // Installierte Schriftfamilien, alphabetisch -- einmalig berechnet.
        private static readonly IReadOnlyList<string> Familien =
            Fonts.SystemFontFamilies
                 .Select(f => f.Source)
                 .OrderBy(s => s, StringComparer.CurrentCultureIgnoreCase)
                 .ToList();

        public IEnumerable<string> FontFamilien => Familien;

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
