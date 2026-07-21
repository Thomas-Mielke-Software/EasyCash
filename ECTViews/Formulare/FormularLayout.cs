// FormularLayout.cs -- gemeinsame Promille->DIP-Umrechnung fuer die
// Formular-ANSICHT (FormularViewModel) und den Formular-DRUCK
// (FormularDruckBauer). EINE Klasse fuer beide -> WYSIWYG per Konstruktion.
//
// Referenz ist der native DRUCK-Pfad (der die korrekte Geometrie hat;
// die Bildschirm-Querformat-Naeherung querformat_faktor=2 in
// easycashview.cpp:4741 wird bewusst NICHT nachgebaut):
//   OnPrint (easycashview.cpp:6127-6137):
//     printer_charheight = SeitenHoehe / VCHARS   (VCHARS = 68)
//   DrawFormularToDC (easycashview.cpp:4894-4899):
//     x = horizontal * SeitenBreite / 1000
//     y = vertikal * SeitenHoehe / 1414 - printer_charheight/2
//   (vertikal adressiert also die MITTE der Textzeile)
//
// Die Seitenmasse kommen aus DruckDokument (A4 in DIP, 96 dpi):
// hochkant 794x1123, quer getauscht.

using ECTViews.Druck;

namespace ECTViews.Formulare
{
    public static class FormularLayout
    {
        /// <summary>Vertikale Zeichenzellen pro Seite (VCHARS,
        /// easycashview.h:502) -- Basis der nativen Geometrie.</summary>
        public const double VChars = 68.0;

        /// <summary>Abstand zwischen zwei Seiten in der Bildschirm-Ansicht
        /// (DIP, unskaliert).</summary>
        public const double SeitenAbstand = 16.0;

        public static double SeitenBreite(bool querformat)
            => querformat ? DruckDokument.SeitenHoehe : DruckDokument.SeitenBreite;

        public static double SeitenHoehe(bool querformat)
            => querformat ? DruckDokument.SeitenBreite : DruckDokument.SeitenHoehe;

        /// <summary>x-Position in DIP (Promille der Seitenbreite; Werte
        /// ueber 1000 kommen vor und duerfen nicht geclippt werden).</summary>
        public static double X(int horizontalPromille, double seitenBreite)
            => horizontalPromille / 1000.0 * seitenBreite;

        /// <summary>Vertikale MITTE der Textzeile in DIP (Promille von 1414).</summary>
        public static double YTextMitte(int vertikalPromille, double seitenHoehe)
            => vertikalPromille / 1414.0 * seitenHoehe;

        /// <summary>Oberkante der Textzeile in DIP (Pendant zum nativen
        /// "- charheight/2" vor dem TA_TOP-TextOut).</summary>
        public static double YTextOben(int vertikalPromille, double seitenHoehe)
            => YTextMitte(vertikalPromille, seitenHoehe) - seitenHoehe / (VChars * 2.0);

        // -----------------------------------------------------------------
        // Fontgroesse. Nativ ist lfHeight = schriftgroesse-Attribut
        // (Bildschirm-Pixel bei Zoom 100); die Relation Schrift/Seite haengt
        // damit von der eingestellten Journal-Bildschirmschrift ab
        // (tmHeight = charheight) -- ein Alt-Quirk. Hier wird stattdessen
        // eine deterministische Relation fixiert:
        //   fontDip = attr * (SeitenHoehe/VCHARS) / ReferenzCharHeight / GdiZelleZuEm
        // ReferenzCharHeight = typische tmHeight der Journal-Schrift in px
        // (Kalibrierwert -- beim Sichtvergleich mit dem nativen Druck
        // festgezurrt), GdiZelleZuEm rechnet die GDI-Zellenhoehe (positives
        // lfHeight = Zellenhoehe inkl. Innenabstand) auf die WPF-em-Groesse um.
        // -----------------------------------------------------------------
        private const double ReferenzCharHeightPx = 16.0;
        private const double GdiZelleZuEm = 1.17;

        public static double FontDip(int schriftgroesseAttribut, double seitenHoehe)
        {
            double zelleDip = seitenHoehe / VChars;
            if (schriftgroesseAttribut <= 0)
                return zelleDip / GdiZelleZuEm;   // Fallback wie nativ (charheight)
            return schriftgroesseAttribut * zelleDip / ReferenzCharHeightPx / GdiZelleZuEm;
        }
    }
}
