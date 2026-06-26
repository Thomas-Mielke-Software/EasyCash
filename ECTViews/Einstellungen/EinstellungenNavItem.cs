using System;
using System.Windows.Controls;

namespace ECTViews.EinstellungenUi
{
    /// <summary>Ein anklickbarer Navigations-Eintrag (= eine Seite).</summary>
    public class EinstellungenNavItem
    {
        /// <summary>Gruppen-Überschrift, nach der in der Navigation gruppiert
        /// wird (z.B. "Global" oder "Aktuelles Dokument").</summary>
        public string Gruppe { get; set; }

        public string Titel { get; set; }

        /// <summary>Fabrik für die zugehörige Seite. Wird erst beim ERSTEN
        /// Anzeigen aufgerufen (Lazy) und das Ergebnis danach gecacht. So kostet
        /// das Öffnen der Einstellungen nichts für Seiten, die (noch) nicht
        /// angeklickt wurden -- insbesondere die teure E/Ü-Konten-Seite, deren
        /// ViewModel beim Konstruieren alle .ecf-Formulardateien liest und parst.</summary>
        public Func<UserControl> SeitenFabrik { get; set; }

        private UserControl _seite;

        /// <summary>Die zugehörige WPF-Seite; wird beim ersten Zugriff erzeugt.</summary>
        public UserControl Seite => _seite ?? (_seite = SeitenFabrik?.Invoke());
    }
}
