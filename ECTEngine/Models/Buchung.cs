using System;

namespace ECTEngine.Models
{
    /// <summary>
    /// Repräsentiert eine einzelne Buchung (Transaktion)
    /// </summary>
    public class Buchung : Betrag
    {
        public int AbschreibungNr { get; set; }
        public int AbschreibungJahre { get; set; }
        public string Beschreibung { get; set; } = "";
        public DateTime Datum { get; set; }
        public string Konto { get; set; } = "";
        public string Belegnummer { get; set; } = "";
        public string Erweiterung { get; set; } = "";
        public long AbschreibungRestwert { get; set; }
        public bool AbschreibungDegressiv { get; set; }
        public int AbschreibungSatz { get; set; }
        public AbschreibungsGenauigkeit AbschreibungGenauigkeit { get; set; }
        public string Bestandskonto { get; set; } = "";
        public string Betrieb { get; set; } = "";

        public Buchung()
        {
            Wert = 0;
            MWSt = 0;
            AbschreibungNr = 1;
            AbschreibungJahre = 1;
            Beschreibung = "";
            Datum = new DateTime(2000, 1, 1);
            Konto = "";
            Belegnummer = "";
            Erweiterung = "";
            AbschreibungRestwert = 0;
            AbschreibungDegressiv = false;
            AbschreibungSatz = 0;
            AbschreibungGenauigkeit = AbschreibungsGenauigkeit.EntsprechendEinstellungen;
            Bestandskonto = "";
            Betrieb = "";
        }

        /// <summary>
        /// Berechnet die Abschreibung für das aktuelle Buchungsjahr
        /// </summary>
        public long GetBuchungsjahrNetto(EasyCashDocument? doc = null)
        {
            var genauigkeit = AbschreibungGenauigkeit;
            if (genauigkeit == AbschreibungsGenauigkeit.EntsprechendEinstellungen)
            {
                if (doc == null)
                    genauigkeit = AbschreibungsGenauigkeit.MonatsgenauAfa;
                else
                    genauigkeit = doc.AbschreibungGenauigkeit;
            }

            return GetBuchungsjahrNetto(genauigkeit);
        }

        private long GetBuchungsjahrNetto(AbschreibungsGenauigkeit genauigkeit)
        {
            long netto = GetNetto();

            if (AbschreibungJahre <= 1)
                return netto;

            if (AbschreibungDegressiv)
            {
                return GetBuchungsjahrNettoDegressiv(netto, genauigkeit);
            }
            else
            {
                return GetBuchungsjahrNettoLinear(netto, genauigkeit);
            }
        }

        private long GetBuchungsjahrNettoDegressiv(long netto, AbschreibungsGenauigkeit genauigkeit)
        {
            // Spezialfall: 75% AfA für Elektroautos
            if (AbschreibungSatz == 75 && AbschreibungGenauigkeit == AbschreibungsGenauigkeit.GanzjahresAfa)
            {
                int[] eautoAfa = { 75, 10, 5, 5, 3, 2 };
                if (AbschreibungNr >= 1 && AbschreibungNr <= 6)
                    return GetNetto() * eautoAfa[AbschreibungNr - 1] / 100;
                else
                    return 0;
            }

            // Im letzten Jahr den Restwert zurückgeben
            if (AbschreibungNr == AbschreibungJahre)
            {
                if (genauigkeit == AbschreibungsGenauigkeit.GanzjahresAfa ||
                    (genauigkeit == AbschreibungsGenauigkeit.HalbjahresAfa && Datum.Month <= 6) ||
                    (genauigkeit == AbschreibungsGenauigkeit.MonatsgenauAfa && Datum.Month <= 1))
                {
                    return AbschreibungRestwert;
                }
            }
            else if (AbschreibungNr == 1)
            {
                return RoundenUndDurch10Dividieren(
                    BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen(
                        10 * AbschreibungRestwert * AbschreibungSatz / 100, genauigkeit));
            }

            if (AbschreibungNr > AbschreibungJahre)
                return AbschreibungRestwert;

            return RoundenUndDurch10Dividieren(10 * AbschreibungRestwert * AbschreibungSatz / 100);
        }

        private long GetBuchungsjahrNettoLinear(long netto, AbschreibungsGenauigkeit genauigkeit)
        {
            int gesamtMonate = 12 * AbschreibungJahre;
            int verbleibendeMonate = CalculateVerbleibendeMonate(gesamtMonate, genauigkeit);

            if (AbschreibungNr > AbschreibungJahre)
                return AbschreibungRestwert;

            long jaehrlicheRate = RoundenUndDurch10Dividieren(10 * AbschreibungRestwert * 12 / verbleibendeMonate);
            if (jaehrlicheRate > AbschreibungRestwert)
                jaehrlicheRate = AbschreibungRestwert;

            return BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen(jaehrlicheRate, genauigkeit);
        }

        private int CalculateVerbleibendeMonate(int gesamtMonate, AbschreibungsGenauigkeit genauigkeit)
        {
            return genauigkeit switch
            {
                AbschreibungsGenauigkeit.GanzjahresAfa =>
                    (AbschreibungJahre - AbschreibungNr + 1) * 12,

                AbschreibungsGenauigkeit.HalbjahresAfa =>
                    AbschreibungNr == 1
                        ? AbschreibungJahre * 12
                        : Datum.Month < 7
                            ? (AbschreibungJahre - AbschreibungNr + 1) * 12
                            : (AbschreibungJahre - AbschreibungNr + 2) * 12 - 6,

                AbschreibungsGenauigkeit.MonatsgenauAfa =>
                    AbschreibungNr == 1
                        ? AbschreibungJahre * 12
                        : gesamtMonate - (AbschreibungNr - 2) * 12 - (13 - Datum.Month),

                _ => (AbschreibungJahre - AbschreibungNr + 1) * 12
            };
        }

        private long BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen(
            long jaehrlicheRate, AbschreibungsGenauigkeit genauigkeit)
        {
            if (AbschreibungNr == 1)
            {
                return genauigkeit switch
                {
                    AbschreibungsGenauigkeit.GanzjahresAfa => jaehrlicheRate,
                    AbschreibungsGenauigkeit.HalbjahresAfa =>
                        Datum.Month < 7 ? jaehrlicheRate : jaehrlicheRate / 2,
                    AbschreibungsGenauigkeit.MonatsgenauAfa =>
                        jaehrlicheRate * (13 - Datum.Month) / 12,
                    _ => jaehrlicheRate
                };
            }
            else if (AbschreibungNr <= AbschreibungJahre)
            {
                return jaehrlicheRate;
            }
            else
            {
                return genauigkeit switch
                {
                    AbschreibungsGenauigkeit.GanzjahresAfa => 0,
                    AbschreibungsGenauigkeit.HalbjahresAfa =>
                        Datum.Month < 7 ? 0 : jaehrlicheRate / 2,
                    AbschreibungsGenauigkeit.MonatsgenauAfa =>
                        jaehrlicheRate * (13 - Datum.Month) / 12,
                    _ => 0
                };
            }
        }

        private long RoundenUndDurch10Dividieren(long betrag)
        {
            if (betrag >= 0)
                betrag += 5;
            else
                betrag -= 5;
            return betrag / 10;
        }

        public void CopyFrom(Buchung quelle)
        {
            Wert = quelle.Wert;
            MWSt = quelle.MWSt;
            AbschreibungNr = quelle.AbschreibungNr;
            AbschreibungJahre = quelle.AbschreibungJahre;
            Beschreibung = quelle.Beschreibung;
            Datum = quelle.Datum;
            Konto = quelle.Konto;
            Belegnummer = quelle.Belegnummer;
            Erweiterung = quelle.Erweiterung;
            AbschreibungRestwert = quelle.AbschreibungRestwert;
            AbschreibungDegressiv = quelle.AbschreibungDegressiv;
            AbschreibungSatz = quelle.AbschreibungSatz;
            AbschreibungGenauigkeit = quelle.AbschreibungGenauigkeit;
            Bestandskonto = quelle.Bestandskonto;
            Betrieb = quelle.Betrieb;
        }
    }
}