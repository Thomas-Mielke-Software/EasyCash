// IconAuswahlViewModel.cs -- reine Icon-Auswahl aus einem Sprite-Bitmap
// (WPF-Ersatz für CIconAuswahl im Modus 0). Wird von der Stammdaten-
// Verwaltung für "Neu..." und "Icon ändern..." benutzt.

using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Media.Imaging;
using ECTViews.ViewModels;

namespace ECTViews.Stammdaten
{
    public class IconAuswahlViewModel : ViewModelBase
    {
        public ObservableCollection<IconListItem> Icons { get; }
            = new ObservableCollection<IconListItem>();

        private IconListItem _auswahl;
        public IconListItem Auswahl
        {
            get => _auswahl;
            set
            {
                if (SetProperty(ref _auswahl, value))
                    OnPropertyChanged(nameof(IconGewaehlt));
            }
        }

        public bool IconGewaehlt => _auswahl != null;

        public string Titel { get; }

        /// <param name="titel">Fenstertitel, z.B. "Icon wählen".</param>
        /// <param name="sprite">Sprite-Bitmap (horizontal aneinandergereihte Icons).</param>
        /// <param name="namen">Icon-Namen (bestimmen wie im MFC-Original die
        /// Anzahl der angebotenen Icons).</param>
        /// <param name="vorauswahlIndex">Anfangs selektiertes Icon (-1 = keins).</param>
        public IconAuswahlViewModel(string titel, BitmapSource sprite,
            string[] namen, int vorauswahlIndex = -1)
        {
            Titel = titel ?? "Icon wählen";
            namen = namen ?? new string[0];

            for (int i = 0; i < namen.Length; i++)
            {
                Icons.Add(new IconListItem
                {
                    Name = namen[i],
                    IconIndex = i,
                    Icon = IconSpriteSplitter.Crop(sprite, i)
                });
            }

            if (vorauswahlIndex >= 0)
                Auswahl = Icons.FirstOrDefault(x => x.IconIndex == vorauswahlIndex);
        }
    }
}
