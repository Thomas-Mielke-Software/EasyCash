// WaehrungApiEinwilligung.cs -- DSGVO-Einwilligung fuer die Online-Kursabfrage.
//
// Vor dem ERSTEN Zugriff auf die ect-forex-API (Liste aktualisieren ODER
// Waehrungsumrechnung im Buchen-Dialog) wird der Anwender einmalig um seine
// Einwilligung gebeten. Die Entscheidung wird persistent gespeichert
// (ECTEngine.Waehrungsliste), sodass der Dialog nur beim ersten Mal erscheint.

using System.Windows;
using ECTEngine;

namespace ECTViews
{
    public static class WaehrungApiEinwilligung
    {
        /// <summary>Stellt sicher, dass eine Einwilligung zur Online-Kursabfrage
        /// vorliegt. Ist sie schon erteilt, kehrt sofort true zurueck; sonst
        /// wird ein Einwilligungs-Dialog gezeigt. Gibt true zurueck, wenn der
        /// Anwender zustimmt (und speichert die Einwilligung), sonst false.
        /// Muss auf dem UI-Thread aufgerufen werden.</summary>
        public static bool Sicherstellen()
        {
            if (Waehrungsliste.ApiEinwilligungErteilt) return true;

            const string titel = "Online-Kursabfrage – Einwilligung gemäß DSGVO";
            var text =
                "Für die Währungsumrechnung ruft EasyCash&Tax aktuelle Wechselkurse "
                + "vom Server easyct.de (ect-forex-API) ab.\n\n"
                + "Dabei werden die gewählte Währung, das Buchungsdatum und – technisch "
                + "bedingt – Ihre IP-Adresse an diesen Server übertragen. "
                + "Andere Buchungs- oder gar Personendaten werden dabei NICHT übermittelt.\n\n"
                + "Es erfolgt keinerlei Weiterverarbeitung dieser Daten über die "
                + "übliche IP-Adress-Protokollierung durch den Webhoster 'Netclusive' hinaus. "
                + "Ich behalte mir jedoch vor, die anonyme Nutzung des Service zukünftig z.B. auf "
                + "registrierte Nutzer*innen einzuschränken, sollte der Service missbraucht werden. "
                + "In dem Fall würde eine erneute Einwilligung eingeholt.\n\n"
                + "Verantwortlicher gemäß DSGVO ist Thomas Mielke (Kontakt über www.easyct.de). "
                + "Es besteht das Recht auf Beschwerde bei der zuständigen Aufsichtsbehörde.\n\n"
                + "Möchten Sie die Online-Kursabfrage aktivieren?";

            var antwort = MessageBox.Show(text, titel,
                MessageBoxButton.YesNo, MessageBoxImage.Question);

            if (antwort == MessageBoxResult.Yes)
            {
                // Vollständigen Wortlaut der angezeigten Dialogbox mitprotokollieren.
                Waehrungsliste.SetzeApiEinwilligung(true, titel + "\r\n\r\n" + text);
                return true;
            }
            return false;
        }
    }
}
