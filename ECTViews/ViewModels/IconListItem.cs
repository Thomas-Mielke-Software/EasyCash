// IconListItem.cs — Item-Klasse für ListViews mit Icon + Name
//
// Wird sowohl für Betriebe als auch für Bestandskonten genutzt.
// Das Icon ist ein zugeschnittener Ausschnitt aus dem Sprite-Bitmap
// (icons.bmp bzw. icons_bestandskonten.bmp).

using System.Windows.Media;

namespace ECTViews.ViewModels
{
    public class IconListItem
    {
        /// <summary>Name des Betriebs/Bestandskontos (wird in der Buchung gespeichert).</summary>
        public string Name { get; set; }

        /// <summary>Index in der zugehörigen Sprite-Bitmap (0-basiert).</summary>
        public int IconIndex { get; set; }

        /// <summary>Das geschnittene 32x32-Icon zum Anzeigen in der ListView.</summary>
        public ImageSource Icon { get; set; }

        public override string ToString() => Name;
    }
}
