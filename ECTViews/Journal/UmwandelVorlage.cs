// UmwandelVorlage.cs -- ein Eintrag des Journal-Kontextmenues "Umwandeln in"
//
// Beschreibt eine Buchungsgruppen-Vorlage (mehrzeiliges Preset), in die die
// selektierte Buchung umgewandelt werden kann. Das Kommando steckt im Eintrag
// selbst, damit die dynamisch erzeugten Submenue-Eintraege keinen
// RelativeSource-Umweg zum ViewModel brauchen (Popup-Grenze).

using System.Windows.Input;
using ECTEngine;

namespace ECTViews.Journal
{
    public sealed class UmwandelVorlage
    {
        public UmwandelVorlage(int nummer, Preset vorlage, ICommand kommando)
        {
            Nummer = nummer;
            Text = vorlage.Text ?? "";
            AnzahlZeilen = vorlage.Zeilen.Count;
            Notiz = vorlage.Notiz ?? "";
            UmwandelnCommand = kommando;
        }

        /// <summary>Slot 0-99 im Einstellungs-Cache.</summary>
        public int Nummer { get; }
        public string Text { get; }
        /// <summary>Anzahl der Zusatz-Zeilen der Vorlage.</summary>
        public int AnzahlZeilen { get; }
        public string Notiz { get; }
        public ICommand UmwandelnCommand { get; }

        /// <summary>Menue-Text: "NN  Vorlagentext" wie im Beschreibungs-
        /// Dropdown des Buchen-Dialogs.</summary>
        public string Anzeige => Nummer.ToString("00") + "  " + Text;

        public string Tooltip
        {
            get
            {
                string t = AnzahlZeilen == 1
                    ? "Legt beim Speichern 1 zusätzliche Buchung an."
                    : "Legt beim Speichern " + AnzahlZeilen
                      + " zusätzliche Buchungen an.";
                return Notiz.Length > 0 ? Notiz + "\n" + t : t;
            }
        }
    }
}
