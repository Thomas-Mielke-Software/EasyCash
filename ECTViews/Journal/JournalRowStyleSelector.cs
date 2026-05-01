// JournalRowStyleSelector.cs - Waehlt das passende ListBoxItem-Style
// pro Zeilentyp (nur Buchungs-Zeilen sind selektierbar mit Zebra).
//
// ZebraConverter.cs - Liefert true wenn ZebraIndex gerade ist
// (wird im DataTrigger-Style genutzt, weil DataTrigger keinen
// arithmetischen Vergleich erlaubt).

using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace ECTViews.Journal
{
    public class JournalRowStyleSelector : StyleSelector
    {
        public static readonly JournalRowStyleSelector Instance =
            new JournalRowStyleSelector();

        public override Style SelectStyle(object item, DependencyObject container)
        {
            if (container is FrameworkElement fe)
            {
                string key = item is JournalBuchungRow
                    ? "BuchungRowStyle"
                    : "JournalRowStyle";
                return fe.TryFindResource(key) as Style;
            }
            return null;
        }
    }

    public class ZebraConverter : IValueConverter
    {
        public static readonly ZebraConverter Instance = new ZebraConverter();

        public object Convert(object value, Type targetType,
            object parameter, CultureInfo culture)
        {
            if (value is JournalBuchungRow row)
                return (row.ZebraIndex % 2) == 1;
            return false;
        }

        public object ConvertBack(object value, Type targetType,
            object parameter, CultureInfo culture)
            => throw new NotSupportedException();
    }
}
