using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    /// <summary>Stammdaten des zuständigen Finanzamts ([Finanzamt]).</summary>
    public class FinanzamtPageViewModel : ViewModelBase
    {
        public string Name          { get => GlobaleEinstellungen.FinanzamtName;          set { GlobaleEinstellungen.FinanzamtName = Getrimmt(value);          OnPropertyChanged(); } }
        public string Name2         { get => GlobaleEinstellungen.FinanzamtName2;         set { GlobaleEinstellungen.FinanzamtName2 = Getrimmt(value);         OnPropertyChanged(); } }
        public string Strasse       { get => GlobaleEinstellungen.FinanzamtStrasse;       set { GlobaleEinstellungen.FinanzamtStrasse = Getrimmt(value);       OnPropertyChanged(); } }
        public string Plz           { get => GlobaleEinstellungen.FinanzamtPlz;           set { GlobaleEinstellungen.FinanzamtPlz = Getrimmt(value);           OnPropertyChanged(); } }
        public string Ort           { get => GlobaleEinstellungen.FinanzamtOrt;           set { GlobaleEinstellungen.FinanzamtOrt = Getrimmt(value);           OnPropertyChanged(); } }
        public string Steuernummer  { get => GlobaleEinstellungen.FinanzamtSteuernummer;  set { GlobaleEinstellungen.FinanzamtSteuernummer = Getrimmt(value);  OnPropertyChanged(); } }
        public string WirtschaftsId
        {
            get => GlobaleEinstellungen.FinanzamtWirtschaftsId;
            set
            {
                GlobaleEinstellungen.FinanzamtWirtschaftsId = Getrimmt(value);
                OnPropertyChanged();
                OnPropertyChanged(nameof(WirtschaftsIdFehler));  // Fehlertext mit aktualisieren
            }
        }

        // -----------------------------------------------------------------
        // Validierung (advisory, blockiert das Speichern nicht -- wie der
        // Hinweis-Dialog im alten MFC-Code). Nur die W-IdNr hat eine
        // Format-Pruefung; der Fehlertext wird wie im Buchen-Dialog als rote
        // Zeile unter dem Feld angezeigt (leer = kein Fehler -> Zeile
        // verschwindet ueber den TextBlock-Trigger).
        // -----------------------------------------------------------------
        public string WirtschaftsIdFehler => PruefeWirtschaftsId(WirtschaftsId) ?? "";

        /// <summary>
        /// Prueft die Wirtschafts-Identifikationsnummer (W-IdNr.) auf das
        /// vom BZSt vergebene Format: Laenderkuerzel (2 Buchstaben) + 9 Ziffern
        /// + Bindestrich + 5-stelliges Unterscheidungsmerkmal, z.B.
        /// "DE123456789-00001" (17 Zeichen). Ein leeres Feld ist zulaessig.
        /// Logik gespiegelt aus IconAuswahlBetrieb.cpp (betriebsbezogene W-IdNr).
        /// </summary>
        private static string PruefeWirtschaftsId(string wert)
        {
            if (string.IsNullOrEmpty(wert))
                return null;  // leer ist erlaubt

            if (wert.Length != 17)
                return "Die Wirtschafts-Identifikationsnummer muss 17 Zeichen lang sein (z.B. DE123456789-00001).";
            if (!IstBuchstabe(wert[0]) || !IstBuchstabe(wert[1]))
                return "Die Wirtschafts-Identifikationsnummer muss mit zwei Buchstaben für das Länderkürzel beginnen, z.B. 'DE' oder 'AT'.";
            if (wert[11] != '-')
                return "Die Wirtschafts-Identifikationsnummer muss an der 12. Position einen Bindestrich enthalten.";
            for (int i = 2; i < 17; i++)
            {
                if (i == 11) continue;  // den Bindestrich ueberspringen
                if (!IstZiffer(wert[i]))
                    return string.Format(
                        "Die Wirtschafts-Identifikationsnummer muss an der {0}. Position eine Ziffer enthalten.", i + 1);
            }

            // Unterscheidungsmerkmal (die letzten 5 Ziffern) laeuft laut BZSt
            // ab 00001 -- 00000 wird nie vergeben. Reine Plausibilitaet, da das
            // Merkmal selbst keine Pruefziffer hat.
            if (wert.Substring(12, 5) == "00000")
                return "Das Unterscheidungsmerkmal (die letzten fünf Ziffern) beginnt bei 00001; 00000 wird nicht vergeben.";

            // Pruefziffer der 9-stelligen Kernnummer -- aber nur fuer deutsche
            // Nummern (DE). Das MOD-11,10-Verfahren gilt fuer die deutsche
            // USt-IdNr/W-IdNr; auslaendische Laenderkuerzel nutzen andere
            // Verfahren und werden hier nicht geprueft.
            if ((wert[0] == 'D' || wert[0] == 'd') && (wert[1] == 'E' || wert[1] == 'e')
                && !PruefzifferStimmt(wert.Substring(2, 9)))
                return "Die Prüfziffer (9. Ziffer der Kernnummer) stimmt nicht -- bitte auf Tippfehler prüfen.";

            return null;
        }

        /// <summary>
        /// Prueft die 9-stellige Kernnummer (8 Nutzziffern + Pruefziffer an
        /// 9. Stelle) nach ISO/IEC 7064 MOD 11,10 -- dem Verfahren der deutschen
        /// USt-IdNr/W-IdNr. Erwartet genau 9 Ziffern (vom Aufrufer geprueft).
        /// </summary>
        private static bool PruefzifferStimmt(string kern)
        {
            int summand = 10;
            for (int i = 0; i < 8; i++)
            {
                int m = (summand + (kern[i] - '0')) % 10;
                if (m == 0) m = 10;
                summand = (m * 2) % 11;
            }
            int pruef = 11 - summand;
            if (pruef == 10) pruef = 0;
            return pruef == (kern[8] - '0');
        }

        private static bool IstBuchstabe(char c) => (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        private static bool IstZiffer(char c)    => c >= '0' && c <= '9';

        /// <summary>Schneidet fuehrende/abschliessende Leerzeichen weg (kein
        /// hartes Kuerzen auf die Maximallaenge -- das erledigt MaxLength).</summary>
        private static string Getrimmt(string wert) => wert?.Trim();
    }
}
