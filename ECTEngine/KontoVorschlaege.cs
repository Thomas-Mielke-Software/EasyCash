// KontoVorschlaege.cs -- vorgeschlagene Konto-Feld-Spezifikationen für das
// Konto-Feld im Vorlagen-Editor
//
// Der Standard-Kontenrahmen von EasyCash&Tax entsteht aus der EÜR: es gibt
// ein Konto für "umsatzsteuerpflichtige Betriebseinnahmen", eines für
// "Waren", usw. Manche häufigen Geschäftsvorfälle müssen aber ZUSÄTZLICH
// eine bestimmte Kennzahl der Umsatzsteuer-Voranmeldung (DE) bzw. der U30
// (AT) beschicken -- eine Ausfuhrlieferung landet in der EÜR bei den
// steuerfreien Einnahmen (1103) UND in der Voranmeldung in KZ 43. Für
// solche Kombinationen gibt es im Standard-Kontenrahmen noch kein Konto.
//
// Statt den Anwender auf die Suche nach dem richtigen Formularfeld zu
// schicken, bietet die Konto-Auswahl des Vorlagen-Editors diese
// Spezifikationen fertig an ($de:Formular=Feld|...|$name=...||at:...||,
// siehe KontoFeldSelektor). Beim ersten Buchen mit der Vorlage findet der
// Buchen-Dialog das passende Konto bzw. bietet seine Anlage an.
//
// Abgrenzung zur Vorlagen-Bibliothek: hier stehen nur EINZELNE Konten für
// einzeilige Vorlagen. Was mehrere Buchungen braucht (Reverse Charge,
// steuerpflichtiger i.g. Erwerb), steht in der Bibliothek.
//
// Alle Spezifikationen prüft VorlagenBibliothekFelderTests gegen die
// echten .ecf-Formulare des Repos (Feld existiert, ist bebuchbar, Richtung
// passt). Kennzahlen ändern sich mit den Formular-Jahrgängen -- schlägt
// der Test an, hier nachziehen.

using System;
using System.Collections.Generic;
using System.Linq;

namespace ECTEngine
{
    /// <summary>Ein vorgeschlagenes Konto (als Feld-Spezifikation).</summary>
    public sealed class KontoVorschlag
    {
        public string Spezifikation { get; }
        /// <summary>True = Einnahmen-Konto, false = Ausgaben-Konto.</summary>
        public bool IstEinnahme { get; }

        internal KontoVorschlag(bool istEinnahme, string spezifikation)
        {
            IstEinnahme = istEinnahme;
            Spezifikation = spezifikation;
        }

        /// <summary>Gilt der Vorschlag für das Land (Kürzel de/at/ch)?</summary>
        public bool GiltFuer(string land)
        {
            var spez = KontoFeldSpezifikation.Parse(Spezifikation, out _);
            return spez?.FuerLand(land) != null;
        }

        /// <summary>Kontoname des Landes ("$name=..."), "" ohne Angabe.</summary>
        public string Name(string land)
            => KontoFeldSpezifikation.Parse(Spezifikation, out _)?.NameFuerLand(land) ?? "";

        /// <summary>Die verknüpften Felder des Landes, lesbar
        /// ("E/Ü-Rechnung 1103, Umsatzsteuer-Voranmeldung 43").</summary>
        public string Felder(string land)
        {
            var bedarf = KontoFeldSpezifikation.Parse(Spezifikation, out _)?.FuerLand(land);
            return bedarf == null ? ""
                : string.Join(", ", bedarf.Select(b => b.Formular + " " + b.FeldId));
        }
    }

    public static class KontoVorschlaege
    {
        public static readonly IReadOnlyList<KontoVorschlag> Alle = new[]
        {
            // ------------------------------------------------------------
            // Einnahmen
            // ------------------------------------------------------------

            // Ausfuhr in ein Drittland: steuerfrei MIT Vorsteuerabzug
            // (DE §4 Nr. 1a, KZ 43; AT KZ 011)
            new KontoVorschlag(true,
                "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=43|$name=Ausfuhrlieferungen (Drittland)||"
                + "at:Beilage E1a=9040|Umsatzsteuer=1011|$name=Ausfuhrlieferungen (Drittland)||"),

            // Lieferung an Unternehmer mit USt-IdNr. in einem anderen
            // EU-Staat (DE KZ 41; AT KZ 017)
            new KontoVorschlag(true,
                "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=41|$name=Innergemeinschaftliche Lieferungen||"
                + "at:Beilage E1a=9040|Umsatzsteuer=1017|$name=Innergemeinschaftliche Lieferungen||"),

            // Sonstige Leistung an Unternehmer im EU-Ausland, Steuerschuld
            // geht auf den Empfänger über (DE KZ 21, §18b). Die U30 hat dafür
            // keine Kennzahl (nur die ZM) -- darum kein at-Block.
            new KontoVorschlag(true,
                "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=21|$name=Sonstige Leistungen an EU-Unternehmer||"),

            // Übrige nicht steuerbare Umsätze, Leistungsort im Ausland
            // (DE KZ 45, z.B. Leistungen an Drittland-Kunden)
            new KontoVorschlag(true,
                "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=45|$name=Nicht steuerbare Umsätze (Leistungsort im Ausland)||"),

            // Steuerfreie Umsätze OHNE Vorsteuerabzug (DE KZ 48, z.B.
            // Heilbehandlung §4 Nr. 14, Vermietung §4 Nr. 12)
            new KontoVorschlag(true,
                "$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=48|$name=Steuerfreie Umsätze ohne Vorsteuerabzug||"),

            // AT: steuerfreie Grundstücksumsätze (KZ 019). Die übrigen
            // unecht steuerfreien Umsätze haben in der U30 keine eigene
            // Kennzahl mehr; KZ 020 ist dort nur noch die Summe.
            new KontoVorschlag(true,
                "$at:Beilage E1a=9040|Umsatzsteuer=1019|$name=Steuerfreie Grundstücksumsätze||"),

            // DE: Umsätze zum Nullsteuersatz (Photovoltaik, §12 Abs. 3,
            // KZ 87) -- umsatzsteuerPFLICHTIG, darum EÜR 1112 statt 1103
            new KontoVorschlag(true,
                "$de:E/Ü-Rechnung=1112|Umsatzsteuer-Voranmeldung=87|$name=Umsätze zum Steuersatz 0 % (Photovoltaik)||"),

            // ------------------------------------------------------------
            // Ausgaben
            // ------------------------------------------------------------

            // Gezahlte Einfuhrumsatzsteuer laut Zollbeleg (DE: Betriebs-
            // ausgabe 1185 + Vorsteuer KZ 62; AT: KZ 061)
            new KontoVorschlag(false,
                "$de:E/Ü-Rechnung=1185|Umsatzsteuer-Voranmeldung=62|$name=Entrichtete Einfuhrumsatzsteuer||"
                + "at:Umsatzsteuer=1061|$name=Entrichtete Einfuhrumsatzsteuer||"),

            // Steuerfreier innergemeinschaftlicher Erwerb (DE §4b, KZ 91;
            // AT KZ 071). Der STEUERPFLICHTIGE Erwerb braucht Zusatz-Zeilen
            // für Steuer und Vorsteuer -- der steht in der Bibliothek.
            new KontoVorschlag(false,
                "$de:E/Ü-Rechnung=1100|Umsatzsteuer-Voranmeldung=91|$name=Steuerfreie innergemeinschaftliche Erwerbe||"
                + "at:Beilage E1a=9100|Umsatzsteuer=1071|$name=Steuerfreie innergemeinschaftliche Erwerbe||"),
        };

        /// <summary>Die Vorschläge einer Buchungsrichtung, die für das Land
        /// eine Feld-Verknüpfung haben.</summary>
        public static IEnumerable<KontoVorschlag> Fuer(bool istEinnahme, string land)
            => Alle.Where(v => v.IstEinnahme == istEinnahme && v.GiltFuer(land));
    }
}
