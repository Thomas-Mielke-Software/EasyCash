using System;
using System.IO;
using System.Text;
using ECTEngine.Models;

namespace ECTEngine.Serialization
{
    /// <summary>
    /// Serialisiert und deserialisiert EasyCashDocument-Objekte
    /// </summary>
    public class DocumentSerializer
    {
        private const string MAGIC_KEY = "ECDo";
        private const int CURRENT_VERSION = 13;

        public void SaveDocument(string filePath, EasyCashDocument document)
        {
            using (var stream = new FileStream(filePath, FileMode.Create, FileAccess.Write))
            using (var writer = new BinaryWriter(stream, Encoding.UTF8))
            {
                WriteMagicKey(writer);
                WriteVersion(writer);
                WriteDocument(writer, document);
            }
        }

        public EasyCashDocument LoadDocument(string filePath)
        {
            using (var stream = new FileStream(filePath, FileMode.Open, FileAccess.Read))
            using (var reader = new BinaryReader(stream, Encoding.UTF8))
            {
                if (!VerifyMagicKey(reader))
                    throw new InvalidOperationException("Fehler beim Öffnen: Kein EasyCash-Dokument!");

                int version = (int)reader.ReadUInt32();
                if (version > CURRENT_VERSION)
                    throw new InvalidOperationException(
                        "Fehler beim Öffnen: Diese Version des Programms ist zu veraltet um das EasyCash-Dokument einzulesen.");

                return ReadDocument(reader, version);
            }
        }

        private void WriteMagicKey(BinaryWriter writer)
        {
            writer.Write(MAGIC_KEY[0]);
            writer.Write(MAGIC_KEY[1]);
            writer.Write(MAGIC_KEY[2]);
            writer.Write(MAGIC_KEY[3]);
        }

        private void WriteVersion(BinaryWriter writer)
        {
            writer.Write((uint)CURRENT_VERSION);
        }

        private void WriteDocument(BinaryWriter writer, EasyCashDocument doc)
        {
            // Einnahmen
            writer.Write(doc.Einnahmen.Count > 0);
            if (doc.Einnahmen.Count > 0)
            {
                foreach (var buchung in doc.Einnahmen)
                    WriteBuchung(writer, buchung);
            }

            // Ausgaben
            writer.Write(doc.Ausgaben.Count > 0);
            if (doc.Ausgaben.Count > 0)
            {
                foreach (var buchung in doc.Ausgaben)
                    WriteBuchung(writer, buchung);
            }

            writer.Write(doc.Buchungszaehler);

            // Dauerbuchungen
            writer.Write(doc.Dauerbuchungen.Count > 0);
            if (doc.Dauerbuchungen.Count > 0)
            {
                foreach (var dauerBuchung in doc.Dauerbuchungen)
                    WriteDauerbuchung(writer, dauerBuchung);
            }

            // ab VERSION 2
            writer.Write(doc.LaufendeBuchungsnummerFuerEinnahmen);
            writer.Write(doc.LaufendeBuchungsnummerFuerAusgaben);

            // ab VERSION 4
            writer.Write(doc.Jahr);

            // ab VERSION 6
            writer.Write(doc.Waehrung);
            writer.Write(doc.UrspruenglicheWaehrung);

            // ab VERSION 7
            writer.Write(doc.Erweiterung);
            writer.Write((int)doc.AbschreibungGenauigkeit);

            // ab VERSION 8
            writer.Write(doc.LaufendeBuchungsnummerFuerBank);
            writer.Write(doc.LaufendeBuchungsnummerFuerKasse);

            // ab VERSION 13
            writer.Write(doc.NachfrageIntervall);
            writer.Write(doc.NachfrageTermin.Ticks);
        }

        private void WriteBuchung(BinaryWriter writer, Buchung buchung)
        {
            writer.Write(buchung.Wert);
            writer.Write(buchung.MWSt);
            writer.Write(buchung.AbschreibungNr);
            writer.Write(buchung.AbschreibungJahre);
            writer.Write(buchung.Beschreibung);
            writer.Write(buchung.Datum.Ticks);
            writer.Write(buchung.Konto);
            writer.Write(buchung.Belegnummer);
            writer.Write(buchung.Erweiterung);
            writer.Write(buchung.AbschreibungRestwert);
            writer.Write(buchung.AbschreibungDegressiv);
            writer.Write(buchung.AbschreibungSatz);
            writer.Write((int)buchung.AbschreibungGenauigkeit);
            writer.Write(buchung.Bestandskonto);
            writer.Write(buchung.Betrieb);
        }

        private void WriteDauerbuchung(BinaryWriter writer, Dauerbuchung dauerBuchung)
        {
            writer.Write(dauerBuchung.Wert);
            writer.Write(dauerBuchung.MWSt);
            writer.Write(dauerBuchung.Beschreibung);
            writer.Write(dauerBuchung.Buchungstyp);
            writer.Write(dauerBuchung.Intervall);
            writer.Write(dauerBuchung.VonDatum.Ticks);
            writer.Write(dauerBuchung.BisDatum.Ticks);
            writer.Write(dauerBuchung.Buchungstag);
            writer.Write(dauerBuchung.AktualisiertBisDatum.Ticks);
            writer.Write(dauerBuchung.Konto);
            writer.Write(dauerBuchung.Belegnummer);
            writer.Write(dauerBuchung.Erweiterung);
            writer.Write(dauerBuchung.Bestandskonto);
            writer.Write(dauerBuchung.Betrieb);
        }

        private bool VerifyMagicKey(BinaryReader reader)
        {
            char[] magic = new char[4];
            magic[0] = reader.ReadChar();
            magic[1] = reader.ReadChar();
            magic[2] = reader.ReadChar();
            magic[3] = reader.ReadChar();
            return new string(magic) == MAGIC_KEY;
        }

        private EasyCashDocument ReadDocument(BinaryReader reader, int version)
        {
            var doc = new EasyCashDocument();

            // Einnahmen
            if (reader.ReadBoolean())
            {
                // TODO: Implementierung für verschiedene Versionen
            }

            // Ausgaben
            if (reader.ReadBoolean())
            {
                // TODO: Implementierung für verschiedene Versionen
            }

            doc.Buchungszaehler = reader.ReadInt32();

            // Dauerbuchungen
            if (reader.ReadBoolean())
            {
                // TODO: Implementierung für verschiedene Versionen
            }

            // ab VERSION 2
            if (version >= 2)
            {
                doc.LaufendeBuchungsnummerFuerEinnahmen = reader.ReadInt32();
                doc.LaufendeBuchungsnummerFuerAusgaben = reader.ReadInt32();
            }

            // ab VERSION 4
            if (version >= 4)
            {
                doc.Jahr = reader.ReadInt32();
            }

            // ab VERSION 6
            if (version >= 6)
            {
                doc.Waehrung = reader.ReadString();
                doc.UrspruenglicheWaehrung = reader.ReadString();
            }

            // ab VERSION 7
            if (version >= 7)
            {
                doc.Erweiterung = reader.ReadString();
                doc.AbschreibungGenauigkeit = (AbschreibungsGenauigkeit)reader.ReadInt32();
            }

            // ab VERSION 8
            if (version >= 8)
            {
                doc.LaufendeBuchungsnummerFuerBank = reader.ReadInt32();
                doc.LaufendeBuchungsnummerFuerKasse = reader.ReadInt32();
            }

            // ab VERSION 13
            if (version >= 13)
            {
                doc.NachfrageIntervall = reader.ReadInt32();
                doc.NachfrageTermin = new DateTime(reader.ReadInt64());
            }

            return doc;
        }
    }
}