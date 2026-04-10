using System;
using ECTEngine.Helpers;

namespace ECTEngine.Models
{
    /// <summary>
    /// Repräsentiert einen Geldbetrag mit Mehrwertsteuersatz
    /// Betrag in Cents, MWSt in Promille (z.B. 19000 = 19,0%)
    /// </summary>
    public class Betrag
    {
        /// <summary>
        /// Der Brutto-Betrag in Cents
        /// </summary>
        public long Wert { get; set; }

        /// <summary>
        /// Der MWSt-Satz in Promille (z.B. 19000 = 19,0%)
        /// </summary>
        public int MWSt { get; set; }

        public Betrag()
        {
            Wert = 0;
            MWSt = 0;
        }

        public Betrag(long wert, int mwst = 0)
        {
            Wert = wert;
            MWSt = mwst;
        }

        /// <summary>
        /// Berechnet den Netto-Betrag aus Brutto und MWSt
        /// </summary>
        public long GetNetto()
        {
            long netto = Wert;

            if (Math.Abs(Wert) <= 1000000 && MWSt % 1000 == 0)
            {
                netto *= 1000;
                netto /= (100 + (MWSt / 1000));

                if (Wert >= 0)
                    netto += 5;  // halber Pfennig zum Runden
                else
                    netto -= 5;

                netto /= 10;
            }
            else
            {
                // Bei großen Zahlen mit double rechnen
                double temp = (double)netto * 100.0;
                temp /= (100.0 + ((double)MWSt / 1000.0));

                if (Wert >= 0)
                    netto = (long)(temp + 0.5);
                else
                    netto = (long)(temp - 0.5);
            }

            return netto;
        }

        /// <summary>
        /// Setzt den MWSt-Satz aus einem String (z.B. "19,0" oder "19")
        /// </summary>
        public bool SetMWSt(string s)
        {
            if (string.IsNullOrEmpty(s))
                return false;

            MWSt = 0;
            int i = 0;

            // Ganzzahliger Anteil
            while (i < s.Length && char.IsDigit(s[i]))
            {
                MWSt *= 10;
                MWSt += s[i] - '0';
                i++;
            }

            if (MWSt > 100)
            {
                MWSt = 100;
                return false;
            }

            // Nachkommaanteil
            if (i >= s.Length || (s[i] != ',' && s[i] != '.'))
            {
                MWSt *= 1000;  // Kein Nachkommaanteil
                return true;
            }

            i++;  // Komma/Punkt überspringen
            for (int j = 0; j < 3; j++)
            {
                int n;
                if (i < s.Length && char.IsDigit(s[i]))
                    n = s[i++] - '0';
                else
                    n = 0;

                MWSt *= 10;
                MWSt += n;
            }

            if (MWSt > 100000)
            {
                MWSt = 100000;
                return false;
            }

            return true;
        }

        /// <summary>
        /// Gibt den MWSt-Satz als formatierter String zurück
        /// </summary>
        public string GetMWStString()
        {
            if (MWSt % 1000 == 0)
                return (MWSt / 1000).ToString();
            else if (MWSt % 100 == 0)
                return ((double)MWSt / 1000.0).ToString("F1").Replace(".", ",");
            else if (MWSt % 10 == 0)
                return ((double)MWSt / 1000.0).ToString("F2").Replace(".", ",");
            else
                return ((double)MWSt / 1000.0).ToString("F3").Replace(".", ",");
        }

        /// <summary>
        /// Konvertiert den Betrag von einer Fremdwährung zu Euro
        /// </summary>
        public bool ConvertToEuro(string waehrungsKuerzel)
        {
            double konversionskurs = WaehrungsHelper.GetKonversionskurs(waehrungsKuerzel);
            if (konversionskurs <= 0)
                return false;

            double d = (double)Wert / konversionskurs;
            Wert = (long)(d + 0.5);  // Kaufmännisches Runden
            return true;
        }

        /// <summary>
        /// Konvertiert den Betrag von Euro zu einer Fremdwährung
        /// </summary>
        public bool ConvertFromEuro(string waehrungsKuerzel)
        {
            double konversionskurs = WaehrungsHelper.GetKonversionskurs(waehrungsKuerzel);
            if (konversionskurs <= 0)
                return false;

            double d = (double)Wert * konversionskurs;
            Wert = (long)(d + 0.5);  // Kaufmännisches Runden
            return true;
        }
    }
}