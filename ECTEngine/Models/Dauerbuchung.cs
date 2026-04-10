using System;

namespace ECTEngine.Models
{
    /// <summary>
    /// Repräsentiert eine wiederkehrende Buchung (Dauerauftrag)
    /// </summary>
    public class Dauerbuchung : Betrag
    {
        public string Beschreibung { get; set; } = "";
        public int Buchungstyp { get; set; }
        public int Intervall { get; set; }
        public DateTime VonDatum { get; set; }
        public DateTime BisDatum { get; set; }
        public int Buchungstag { get; set; }
        public DateTime AktualisiertBisDatum { get; set; }
        public string Konto { get; set; } = "";
        public string Belegnummer { get; set; } = "";
        public string Erweiterung { get; set; } = "";
        public string Bestandskonto { get; set; } = "";
        public string Betrieb { get; set; } = "";

        public Dauerbuchung()
        {
            Wert = 0;
            MWSt = 0;
            Beschreibung = "";
            Buchungstyp = 0;
            Intervall = 0;
            VonDatum = new DateTime(2000, 1, 1);
            BisDatum = new DateTime(2000, 1, 1);
            Buchungstag = 1;
            AktualisiertBisDatum = new DateTime(2000, 1, 1);
            Konto = "";
            Belegnummer = "";
            Erweiterung = "";
            Bestandskonto = "";
            Betrieb = "";
        }

        public void CopyFrom(Dauerbuchung quelle)
        {
            Wert = quelle.Wert;
            MWSt = quelle.MWSt;
            Beschreibung = quelle.Beschreibung;
            Buchungstyp = quelle.Buchungstyp;
            Intervall = quelle.Intervall;
            VonDatum = quelle.VonDatum;
            BisDatum = quelle.BisDatum;
            Buchungstag = quelle.Buchungstag;
            AktualisiertBisDatum = quelle.AktualisiertBisDatum;
            Konto = quelle.Konto;
            Belegnummer = quelle.Belegnummer;
            Erweiterung = quelle.Erweiterung;
            Bestandskonto = quelle.Bestandskonto;
            Betrieb = quelle.Betrieb;
        }
    }
}