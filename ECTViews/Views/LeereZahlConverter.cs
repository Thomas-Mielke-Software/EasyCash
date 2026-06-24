using System;
using System.Globalization;
using System.Windows.Data;

namespace ECTViews.Views
{
    /// <summary>
    /// Zeigt eine 0 als leeres Textfeld an und umgekehrt. Wird fuer die
    /// Datums-Eingabefelder (Tag/Monat) verwendet: ein nicht vorbelegter Wert
    /// (0) erscheint als leeres Feld statt als "0", und ein geleertes Feld
    /// liefert wieder 0 zurueck.
    /// </summary>
    public class LeereZahlConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is int i)
                return i == 0 ? string.Empty : i.ToString(culture);
            return value;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var s = value as string;
            if (string.IsNullOrWhiteSpace(s)) return 0;
            return int.TryParse(s, NumberStyles.Integer, culture, out int i) ? i : 0;
        }
    }
}
