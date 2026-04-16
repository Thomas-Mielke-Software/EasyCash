// Betrag.cs — Cent-genaue Währungsbetragsdarstellung
//
// Ersetzt: CBetrag (EasyCashDoc.h)
// Original: int Betrag (Cent), int MWSt (Promille)
// Verbesserung: decimal statt int/float-Hybrid, kein Moduswechsel ab 10.000€
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

using System;
using System.Globalization;

namespace ECTEngine
{
    /// <summary>
    /// Werttyp für Währungsbeträge mit MWSt-Information.
    /// Ersetzt die C++-Klasse CBetrag.
    ///
    /// Designentscheidungen:
    /// - decimal statt int (Cent): decimal bietet 28-29 signifikante Stellen
    ///   und eliminiert den Moduswechsel int→float bei |Betrag| > 10.000 Cent.
    /// - MWSt als decimal-Prozent statt int-Promille: lesbarerer Code.
    /// - Struct (Werttyp) statt Class: Beträge sind immutable Werte.
    /// </summary>
    public readonly struct Betrag : IEquatable<Betrag>, IComparable<Betrag>
    {
        private static readonly CultureInfo DeDE = new CultureInfo("de-DE");

        /// <summary>Bruttobetrag in Euro.</summary>
        public decimal BruttoWert { get; }

        /// <summary>MWSt-Satz in Prozent (z.B. 19.0m, 7.0m, 0m).</summary>
        public decimal MwstProzent { get; }

        public Betrag(decimal brutto, decimal mwstProzent = 0m)
        {
            BruttoWert = decimal.Round(brutto, 2, MidpointRounding.AwayFromZero);
            MwstProzent = mwstProzent;
        }

        // ──────────────────────────────────────────────
        // Legacy-Kompatibilität (für Bridge-Konvertierung)
        // ──────────────────────────────────────────────

        /// <summary>Bruttobetrag in Cent (int), wie CBetrag::Betrag.</summary>
        public int InCent => (int)decimal.Round(BruttoWert * 100m, 0, MidpointRounding.AwayFromZero);

        /// <summary>MWSt als Promille (int), wie CBetrag::MWSt.</summary>
        public int MwstPromille => (int)decimal.Round(MwstProzent * 1000m, 0);

        /// <summary>
        /// Erzeugt einen Betrag aus den Legacy-Formaten (int Cent, int Promille).
        /// Nutzung in der Bridge beim Konvertieren von nativen CBuchung-Objekten.
        /// </summary>
        public static Betrag AusCent(int cent, int mwstPromille = 0) =>
            new Betrag(cent / 100m, mwstPromille / 1000m);

        // ──────────────────────────────────────────────
        // Berechnungen
        // ──────────────────────────────────────────────

        /// <summary>
        /// Nettobetrag. Reimplementiert CBetrag::GetNetto() mit decimal-Präzision.
        ///
        /// Original-Algorithmus (C++):
        ///   if |Betrag| ≤ 1.000.000 UND MWSt ganzzahlig:
        ///     netto = Betrag * 1000 / (100 + MWSt/1000), dann (±5)/10
        ///   else:
        ///     double-Arithmetik mit round
        ///
        /// Mit decimal brauchen wir keinen Moduswechsel mehr.
        /// </summary>
        public decimal NettoWert
        {
            get
            {
                if (MwstProzent == 0m) return BruttoWert;
                return decimal.Round(
                    BruttoWert / (1m + MwstProzent / 100m),
                    2,
                    MidpointRounding.AwayFromZero);
            }
        }

        /// <summary>
        /// Nettobetrag in Cent (long). Kompatibel mit dem Rückgabewert
        /// von CBetrag::GetNetto().
        /// </summary>
        public long NettoInCent =>
            (long)decimal.Round(NettoWert * 100m, 0, MidpointRounding.AwayFromZero);

        /// <summary>MWSt-Betrag (Brutto - Netto).</summary>
        public decimal MwstBetrag => BruttoWert - NettoWert;

        // ──────────────────────────────────────────────
        // Arithmetik
        // ──────────────────────────────────────────────

        public static Betrag Null => new Betrag(0m);

        public static Betrag operator +(Betrag a, Betrag b) =>
            new Betrag(a.BruttoWert + b.BruttoWert, a.MwstProzent);

        public static Betrag operator -(Betrag a, Betrag b) =>
            new Betrag(a.BruttoWert - b.BruttoWert, a.MwstProzent);

        public static Betrag operator *(Betrag a, decimal faktor) =>
            new Betrag(a.BruttoWert * faktor, a.MwstProzent);

        public static Betrag operator -(Betrag a) =>
            new Betrag(-a.BruttoWert, a.MwstProzent);

        public static bool operator ==(Betrag a, Betrag b) => a.Equals(b);
        public static bool operator !=(Betrag a, Betrag b) => !a.Equals(b);
        public static bool operator <(Betrag a, Betrag b) => a.BruttoWert < b.BruttoWert;
        public static bool operator >(Betrag a, Betrag b) => a.BruttoWert > b.BruttoWert;
        public static bool operator <=(Betrag a, Betrag b) => a.BruttoWert <= b.BruttoWert;
        public static bool operator >=(Betrag a, Betrag b) => a.BruttoWert >= b.BruttoWert;

        // ──────────────────────────────────────────────
        // Formatierung
        // ──────────────────────────────────────────────

        /// <summary>Formatiert als Währungsbetrag, z.B. "1.234,56".</summary>
        public string AlsWaehrung() => BruttoWert.ToString("N2", DeDE);

        public override string ToString() =>
            $"{AlsWaehrung()} (MWSt {MwstProzent}%)";

        // ──────────────────────────────────────────────
        // IEquatable, IComparable
        // ──────────────────────────────────────────────

        public bool Equals(Betrag other) =>
            BruttoWert == other.BruttoWert && MwstProzent == other.MwstProzent;

        public override bool Equals(object obj) =>
            obj is Betrag other && Equals(other);

//        public override int GetHashCode() =>
//            HashCode.Combine(BruttoWert, MwstProzent);

        public int CompareTo(Betrag other) =>
            BruttoWert.CompareTo(other.BruttoWert);
    }
}
