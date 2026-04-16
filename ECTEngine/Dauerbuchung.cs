// Dauerbuchung.cs — Wiederkehrende Buchung
//
// Ersetzt: CDauerbuchung : CBetrag : CObject (EasyCashDoc.h)
// Feldmapping: siehe Kommentare in der Klasse
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

using System;
using System.Collections.Generic;

namespace ECTEngine
{
    /// <summary>
    /// Wiederkehrende Buchung (Dauerauftrag).
    ///
    /// Feld-für-Feld-Mapping zur C++-Klasse CDauerbuchung:
    ///
    ///   C++ (CDauerbuchung)           → C# Property
    ///   int Betrag (Cent)             → BruttoBetrag.InCent
    ///   int MWSt (Promille)           → BruttoBetrag.MwstPromille
    ///   CString Beschreibung          → Beschreibung (Template mit $M, $m, ...)
    ///   int Buchungstyp               → Art (0=Einnahme, 1=Ausgabe)
    ///   int Intervall                 → Intervall (1..5)
    ///   CTime VonDatum                → VonDatum
    ///   CTime BisDatum                → BisDatum
    ///   int Buchungstag               → Buchungstag
    ///   CTime AktualisiertBisDatum    → AusgefuehrtBis
    ///   CString Konto                 → Konto                    (seit v3)
    ///   CString Belegnummer           → Belegnummer (Template)   (seit v7)
    ///   CString Erweiterung           → Erweiterungen (parsed)   (seit v7)
    ///   CString Bestandskonto         → Bestandskonto            (seit v12)
    ///   CString Betrieb               → Betrieb                  (seit v12)
    ///   CDauerbuchung *next           → [entfällt – List statt Linked List]
    /// </summary>
    public class Dauerbuchung
    {
        // ──────────────────────────────────────────────
        // Betrag (aus CBetrag-Basisklasse)
        // ──────────────────────────────────────────────

        public Betrag BruttoBetrag { get; set; }

        // ──────────────────────────────────────────────
        // Kernfelder
        // ──────────────────────────────────────────────

        /// <summary>Einnahme oder Ausgabe (int Buchungstyp).</summary>
        public Buchungsart Art { get; set; }

        /// <summary>
        /// Beschreibungstext-Template. Unterstützt Platzhalter:
        /// $M=Monat 2-stellig, $m=Monat, $q=Quartal, $h=Halbjahr,
        /// $J=Jahr 4-stellig, $j=Jahr 2-stellig, $2=2-Monatszeitraum
        /// </summary>
        public string Beschreibung { get; set; } = "";

        /// <summary>Wiederholungsintervall.</summary>
        public Intervall Intervall { get; set; } = Intervall.Monatlich;

        /// <summary>Startdatum (CTime VonDatum).</summary>
        public DateTime VonDatum { get; set; }

        /// <summary>Enddatum (CTime BisDatum).</summary>
        public DateTime BisDatum { get; set; }

        /// <summary>Tag im Monat für die Buchung (int Buchungstag).</summary>
        public int Buchungstag { get; set; } = 1;

        /// <summary>
        /// Datum, bis zu dem die Dauerbuchung zuletzt ausgeführt wurde
        /// (CTime AktualisiertBisDatum). Kann manuell zurückgesetzt werden.
        /// </summary>
        public DateTime AusgefuehrtBis { get; set; }

        /// <summary>E/Ü-Kontoname.</summary>
        public string Konto { get; set; } = "";

        /// <summary>
        /// Belegnummer-Template. Seit Version 7.
        /// Unterstützt dieselben Platzhalter wie Beschreibung.
        /// </summary>
        public string Belegnummer { get; set; } = "";

        /// <summary>Bestandskonto. Seit Version 12.</summary>
        public string Bestandskonto { get; set; } = "";

        /// <summary>Betriebszuordnung. Seit Version 12.</summary>
        public string Betrieb { get; set; } = "";

        /// <summary>Plugin-Erweiterungsdaten. Seit Version 7.</summary>
        public ErweiterungStore Erweiterungen { get; set; } = new ErweiterungStore();

        // ──────────────────────────────────────────────
        // Platzhalter-Auflösung
        // ──────────────────────────────────────────────

        /// <summary>
        /// Löst Platzhalter in einem Template-String auf.
        /// Platzhalter: $M, $m, $q, $h, $J, $j, $2
        /// </summary>
        public static string ResolvePlatzhalter(string template, DateTime stichtag)
        {
            if (string.IsNullOrEmpty(template)) return template ?? "";

            return template
                .Replace("$M", stichtag.Month.ToString("D2"))
                .Replace("$m", stichtag.Month.ToString())
                .Replace("$q", ((stichtag.Month - 1) / 3 + 1).ToString())
                .Replace("$h", (stichtag.Month <= 6 ? 1 : 2).ToString())
                .Replace("$J", stichtag.Year.ToString("D4"))
                .Replace("$j", (stichtag.Year % 100).ToString("D2"))
                .Replace("$2", ((stichtag.Month - 1) / 2 + 1).ToString());
        }

        // ──────────────────────────────────────────────
        // Buchungserzeugung
        // ──────────────────────────────────────────────

        /// <summary>
        /// Erzeugt aus dieser Dauerbuchung eine konkrete Buchung
        /// für den gegebenen Stichtag.
        /// </summary>
        public Buchung ErzeugeBuchung(DateTime stichtag)
        {
            return new Buchung
            {
                Datum = stichtag,
                BruttoBetrag = BruttoBetrag,
                Beschreibung = ResolvePlatzhalter(Beschreibung, stichtag),
                Belegnummer = ResolvePlatzhalter(Belegnummer, stichtag),
                Art = Art,
                Konto = Konto,
                Bestandskonto = Bestandskonto,
                Betrieb = Betrieb,
                Erweiterungen = Erweiterungen.Clone()
            };
        }

        /// <summary>
        /// Gibt alle fälligen Termine zurück, die nach AusgefuehrtBis
        /// liegen und bis zum angegebenen Stichtag fällig sind.
        /// </summary>
        public IEnumerable<DateTime> FaelligeTermine(DateTime bis)
        {
            DateTime current = NaechsterTerminNach(AusgefuehrtBis);

            while (current <= bis && current <= BisDatum)
            {
                if (current >= VonDatum)
                    yield return current;
                current = NaechsterTerminNach(current);
            }
        }

        private DateTime NaechsterTerminNach(DateTime nach)
        {
            int jahr = nach.Year;
            int monat = nach.Month;

            switch (Intervall)
            {
                case Intervall.Monatlich:     monat += 1; break;
                case Intervall.ZweiMonatlich: monat += 2; break;
                case Intervall.Quartalsweise: monat += 3; break;
                case Intervall.Halbjaehrlich: monat += 6; break;
                case Intervall.Jaehrlich:     monat += 12; break;
                default:                      monat += 1; break;
            }

            while (monat > 12) { monat -= 12; jahr++; }

            int tag = Math.Min(Buchungstag, DateTime.DaysInMonth(jahr, monat));
            return new DateTime(jahr, monat, tag);
        }

        // ──────────────────────────────────────────────
        // Kopie
        // ──────────────────────────────────────────────

        /// <summary>
        /// Erstellt eine tiefe Kopie. Entspricht CDauerbuchung::operator=.
        /// </summary>
        public Dauerbuchung Clone()
        {
            return new Dauerbuchung
            {
                BruttoBetrag = BruttoBetrag,
                Art = Art,
                Beschreibung = Beschreibung,
                Intervall = Intervall,
                VonDatum = VonDatum,
                BisDatum = BisDatum,
                Buchungstag = Buchungstag,
                AusgefuehrtBis = AusgefuehrtBis,
                Konto = Konto,
                Belegnummer = Belegnummer,
                Bestandskonto = Bestandskonto,
                Betrieb = Betrieb,
                Erweiterungen = Erweiterungen.Clone()
            };
        }
    }
}
