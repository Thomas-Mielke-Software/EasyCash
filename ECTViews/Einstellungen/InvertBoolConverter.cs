using System;
using System.Globalization;
using System.Windows.Data;

namespace ECTViews.EinstellungenUi
{
    /// <summary>Invertiert einen bool-Wert (für gegenläufige RadioButtons,
    /// z.B. Einnahme = !Ausgabe).</summary>
    public class InvertBoolConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
            => value is bool b ? !b : value;

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            => value is bool b ? !b : value;
    }
}
