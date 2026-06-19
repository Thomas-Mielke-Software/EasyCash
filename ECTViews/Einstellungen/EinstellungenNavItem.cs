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

        /// <summary>Die zugehörige WPF-Seite (UserControl), wird beim Klick
        /// im Content-Bereich angezeigt. Wird einmal beim Aufbau erzeugt.</summary>
        public UserControl Seite { get; set; }
    }
}
