// NavigationItem.cs - Datenmodell fuer das Navigationsfenster.
//
// Replikat von CNavigation, aber als WPF-Daten ohne List-Control.
// Eine Liste von NavigationGruppe (Header + Items), jedes Item ist ein
// klickbarer Eintrag, dessen Aktivierung das Journal an die passende
// Stelle scrollt.
//
// Designentscheidung: ItemsControl mit GroupStyle vs TreeView vs
// ItemsControl mit gruppierten Sektionen in flacher Liste.
//
// Gewaehlt: ItemsControl mit ItemsControl-Verschachtelung -
// Gruppen oben mit Header-Text in Fett, Items darunter klickbar.
// Keine Auf-/Zuklappbarkeit (anders als TreeView), weil das im
// Original auch nicht ging - die Gruppen sind immer sichtbar.

using System.Collections.ObjectModel;

namespace ECTViews.Journal
{
    /// <summary>Gruppe in der Navigation (z.B. "Einnahmen", "Ausgaben",
    /// oder ein Bestandskontoname).</summary>
    public class NavigationGruppe
    {
        public string Header { get; set; }
        public ObservableCollection<NavigationItem> Items { get; }
            = new ObservableCollection<NavigationItem>();
    }

    /// <summary>Klickbarer Eintrag in der Navigation.</summary>
    public class NavigationItem
    {
        /// <summary>Anzeigetext (z.B. "Januar", "Honorar", "Sparkonto").</summary>
        public string Text { get; set; }

        /// <summary>
        /// Sprung-Ziel-Kennung. Inhalt haengt vom Modus ab:
        ///   - Datum:           "E:1".."E:12" / "A:1".."A:12"
        ///   - Konten/Anlagen:  "E:Honorar" / "A:Bueromaterial" /
        ///                      "E:" (unzugewiesene E.) / "A:" (unzug. A.)
        ///   - Bestandskonten:  "BK:Sparkonto:1".."BK:Sparkonto:12"
        /// Der NavigationViewModel wertet das aus und ruft die passende
        /// ScrolleZu*-Methode am JournalViewModel auf.
        /// </summary>
        public string SprungZiel { get; set; }
    }
}
