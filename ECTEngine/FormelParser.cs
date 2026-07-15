// FormelParser.cs -- Ausdrucks-Engine für Buchungsgruppen-Vorlagen
//
// Zwei Aufgaben:
//  1. Berechne(): arithmetische Formeln für Betragsfelder,
//     z.B. "$brutto*0,7" oder "($netto-$rest)/2".
//     Grammatik: + - * / Klammern, unäres Vorzeichen, Dezimalzahlen
//     (deutsch UND englisch), $variablen.
//  2. Interpoliere(): String-Templates für Textfelder (Beschreibung,
//     Belegnummer, Konto, Betrieb, Bestandskonto), z.B.
//     "$beleg-$vorlagenzeile" -> "R2026-042-2". Unbekannte $Platzhalter
//     bleiben unangetastet, damit die Datums-Platzhalter der
//     Dauerbuchungen ($J, $M, $+m, ...) in einem NACHGELAGERTEN Schritt
//     aufgelöst werden können.
//
// Bewusst KEIN C#-Scripting (CodeDom/CS-Script/Roslyn): Vorlagen sind
// über XML-Export teilbar und dürfen keine Code-Ausführung erlauben.
//
// Fehler werden als Ergebnis-Typ geliefert (kein Throw in UI-Pfaden).

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

namespace ECTEngine
{
    /// <summary>Ergebnis einer Formel-Auswertung.</summary>
    public sealed class FormelErgebnis
    {
        public bool Ok { get; }
        public decimal Wert { get; }
        /// <summary>Fehlertext (deutsch, endnutzertauglich); leer wenn Ok.</summary>
        public string Fehler { get; }

        private FormelErgebnis(bool ok, decimal wert, string fehler)
        {
            Ok = ok; Wert = wert; Fehler = fehler ?? "";
        }

        public static FormelErgebnis Erfolg(decimal wert) =>
            new FormelErgebnis(true, wert, "");
        public static FormelErgebnis Fehlgeschlagen(string fehler) =>
            new FormelErgebnis(false, 0m, fehler);
    }

    /// <summary>
    /// Rekursiver Abstiegsparser für die Betrags-Formeln und Interpolation
    /// für die Text-Templates der Buchungsgruppen-Vorlagen.
    /// </summary>
    public static class FormelParser
    {
        private static readonly CultureInfo DeDE = new CultureInfo("de-DE");

        // ------------------------------------------------------------------
        // Arithmetik
        // ------------------------------------------------------------------

        /// <summary>
        /// Wertet eine arithmetische Formel aus. Variablen-Namen ohne '$'
        /// im Dictionary, Lookup case-insensitiv.
        /// </summary>
        public static FormelErgebnis Berechne(string formel,
            IReadOnlyDictionary<string, decimal> variablen)
        {
            if (string.IsNullOrWhiteSpace(formel))
                return FormelErgebnis.Fehlgeschlagen("Leere Formel.");
            try
            {
                var parser = new Parser(formel, variablen);
                decimal wert = parser.ParseAusdruck();
                parser.ErwarteEnde();
                return FormelErgebnis.Erfolg(wert);
            }
            catch (FormelFehler f)
            {
                return FormelErgebnis.Fehlgeschlagen(f.Message);
            }
            catch (DivideByZeroException)
            {
                return FormelErgebnis.Fehlgeschlagen("Division durch 0.");
            }
            catch (OverflowException)
            {
                return FormelErgebnis.Fehlgeschlagen("Zahlenbereich überschritten.");
            }
        }

        /// <summary>Nur Syntax/Variablen prüfen (für Live-Validierung im
        /// Vorlagen-Editor). Liefert null wenn ok, sonst den Fehlertext.</summary>
        public static string Pruefe(string formel,
            IReadOnlyDictionary<string, decimal> variablen)
        {
            var e = Berechne(formel, variablen);
            return e.Ok ? null : e.Fehler;
        }

        private sealed class FormelFehler : Exception
        {
            public FormelFehler(string msg) : base(msg) { }
        }

        private sealed class Parser
        {
            private readonly string _s;
            private readonly IReadOnlyDictionary<string, decimal> _variablen;
            private int _pos;

            public Parser(string s, IReadOnlyDictionary<string, decimal> variablen)
            {
                _s = s;
                _variablen = variablen ?? new Dictionary<string, decimal>();
            }

            // Ausdruck := Term (('+'|'-') Term)*
            public decimal ParseAusdruck()
            {
                decimal wert = ParseTerm();
                while (true)
                {
                    UeberspringeLeerraum();
                    if (Passt('+')) wert += ParseTerm();
                    else if (Passt('-')) wert -= ParseTerm();
                    else return wert;
                }
            }

            // Term := Faktor (('*'|'/') Faktor)*
            private decimal ParseTerm()
            {
                decimal wert = ParseFaktor();
                while (true)
                {
                    UeberspringeLeerraum();
                    if (Passt('*')) wert *= ParseFaktor();
                    else if (Passt('/'))
                    {
                        decimal teiler = ParseFaktor();
                        if (teiler == 0m) throw new DivideByZeroException();
                        wert /= teiler;
                    }
                    else return wert;
                }
            }

            // Faktor := Zahl | '$'Variable | '(' Ausdruck ')' | ('+'|'-') Faktor
            private decimal ParseFaktor()
            {
                UeberspringeLeerraum();
                if (Passt('-')) return -ParseFaktor();
                if (Passt('+')) return ParseFaktor();
                if (Passt('('))
                {
                    decimal wert = ParseAusdruck();
                    UeberspringeLeerraum();
                    if (!Passt(')'))
                        throw new FormelFehler("Schließende Klammer ')' fehlt.");
                    return wert;
                }
                if (_pos < _s.Length && _s[_pos] == '$')
                    return ParseVariable();
                if (_pos < _s.Length && (char.IsDigit(_s[_pos]) || _s[_pos] == ',' || _s[_pos] == '.'))
                    return ParseZahl();

                throw new FormelFehler(_pos >= _s.Length
                    ? "Formel endet unerwartet."
                    : $"Unerwartetes Zeichen '{_s[_pos]}' an Position {_pos + 1}.");
            }

            private decimal ParseVariable()
            {
                _pos++;   // '$'
                int start = _pos;
                while (_pos < _s.Length && (char.IsLetterOrDigit(_s[_pos]) || _s[_pos] == '_'))
                    _pos++;
                if (_pos == start)
                    throw new FormelFehler("'$' ohne Variablennamen.");
                string name = _s.Substring(start, _pos - start);

                // case-insensitiver Lookup
                foreach (var kv in _variablen)
                    if (string.Equals(kv.Key, name, StringComparison.OrdinalIgnoreCase))
                        return kv.Value;
                throw new FormelFehler($"Unbekannte Variable '${name}'.");
            }

            private decimal ParseZahl()
            {
                int start = _pos;
                while (_pos < _s.Length &&
                       (char.IsDigit(_s[_pos]) || _s[_pos] == ',' || _s[_pos] == '.'))
                    _pos++;
                string token = _s.Substring(start, _pos - start);

                // Komma vorhanden -> deutsches Format (Punkt = Tausender);
                // sonst Punkt als Dezimaltrenner (Invariant) -- gleiche
                // Heuristik wie die Betragsfelder im Buchen-Dialog.
                decimal d;
                bool ok = token.Contains(",")
                    ? decimal.TryParse(token.Replace(".", ""), NumberStyles.Number, DeDE, out d)
                    : decimal.TryParse(token, NumberStyles.Number, CultureInfo.InvariantCulture, out d);
                if (!ok)
                    throw new FormelFehler($"'{token}' ist keine gültige Zahl.");
                return d;
            }

            private bool Passt(char c)
            {
                UeberspringeLeerraum();
                if (_pos < _s.Length && _s[_pos] == c) { _pos++; return true; }
                return false;
            }

            private void UeberspringeLeerraum()
            {
                while (_pos < _s.Length && char.IsWhiteSpace(_s[_pos])) _pos++;
            }

            public void ErwarteEnde()
            {
                UeberspringeLeerraum();
                if (_pos < _s.Length)
                    throw new FormelFehler(
                        $"Unerwartetes Zeichen '{_s[_pos]}' an Position {_pos + 1}.");
            }
        }

        // ------------------------------------------------------------------
        // String-Interpolation
        // ------------------------------------------------------------------

        /// <summary>
        /// Ersetzt $Variablen in einem Text-Template. Längster Variablenname
        /// gewinnt ("$beschreibung" vor "$b"), Lookup case-insensitiv.
        /// UNBEKANNTE $Platzhalter bleiben unverändert stehen -- die
        /// Datums-Platzhalter der Dauerbuchungen werden nachgelagert
        /// aufgelöst.
        /// </summary>
        public static string Interpoliere(string template,
            IReadOnlyDictionary<string, string> variablen)
        {
            if (string.IsNullOrEmpty(template) || variablen == null || variablen.Count == 0)
                return template ?? "";

            // Namen nach Länge absteigend, damit "$beschreibung" nicht
            // fälschlich als "$b" + "eschreibung" gelesen wird.
            var namen = variablen.Keys
                .Where(n => !string.IsNullOrEmpty(n))
                .OrderByDescending(n => n.Length)
                .ToList();

            var sb = new StringBuilder(template.Length + 16);
            int pos = 0;
            while (pos < template.Length)
            {
                char c = template[pos];
                if (c != '$') { sb.Append(c); pos++; continue; }

                string treffer = null;
                foreach (var name in namen)
                {
                    if (pos + 1 + name.Length <= template.Length &&
                        string.Compare(template, pos + 1, name, 0, name.Length,
                            StringComparison.OrdinalIgnoreCase) == 0)
                    {
                        // Nicht mitten in einem längeren Bezeichner abschneiden:
                        // nach dem Treffer darf kein weiteres Wortzeichen folgen,
                        // es sei denn, es gibt keine längere Variable (dann ist es
                        // ein unbekannter Platzhalter und bleibt stehen).
                        int ende = pos + 1 + name.Length;
                        bool wortgrenze = ende >= template.Length
                            || !(char.IsLetterOrDigit(template[ende]) || template[ende] == '_');
                        if (wortgrenze) { treffer = name; break; }
                    }
                }

                if (treffer != null)
                {
                    sb.Append(variablen.First(kv => string.Equals(
                        kv.Key, treffer, StringComparison.OrdinalIgnoreCase)).Value ?? "");
                    pos += 1 + treffer.Length;
                }
                else
                {
                    sb.Append('$');   // unbekannt -> stehen lassen
                    pos++;
                }
            }
            return sb.ToString();
        }

        // ------------------------------------------------------------------
        // Variablen-Maps aus der Basis-Buchung (EINE Stelle fürs Mapping)
        // ------------------------------------------------------------------

        /// <summary>
        /// Numerische Variablen für Betrags-Formeln einer Vorlagen-Zeile.
        /// </summary>
        /// <param name="basis">Die Basis-Buchung (Zeile 0).</param>
        /// <param name="rest">Basis-Brutto minus Brutto aller bisher
        /// berechneten Zusatz-Zeilen ($rest).</param>
        /// <param name="vorlagenzeile">1-basierter Index der Zusatz-Zeile.</param>
        public static Dictionary<string, decimal> ZahlVariablen(
            Buchung basis, decimal rest, int vorlagenzeile)
        {
            var map = new Dictionary<string, decimal>(StringComparer.OrdinalIgnoreCase)
            {
                ["brutto"] = basis?.BruttoBetrag.BruttoWert ?? 0m,
                ["netto"] = basis?.NettoWert ?? 0m,
                ["mwst"] = basis?.MwstBetrag ?? 0m,
                // MWSt-SATZ der Basis in Prozent (19 fuer 19 %) -- z.B. fuer
                // Gegenbuchungen, die den Steuersatz der Basis übernehmen
                // sollen (Privat-Split-Import: MwstAusdruck "$mwstsatz")
                ["mwstsatz"] = basis?.BruttoBetrag.MwstProzent ?? 0m,
                ["rest"] = rest,
                ["vorlagenzeile"] = vorlagenzeile
            };
            FuegeVatVariablenHinzu(map);
            return map;
        }

        /// <summary>
        /// String-Variablen für die Text-Templates (Beschreibung, Beleg,
        /// Konto, Betrieb, Bestandskonto). Zahlen im Systemformat
        /// (Waehrungsformat). Kanonischer Platzhalter für die
        /// Basis-Beschreibung ist "$beschreibung"; "$B" bleibt als stiller
        /// Alias erhalten (bestehende Vorlagen/frühe Privat-Split-Importe).
        /// </summary>
        public static Dictionary<string, string> TextVariablen(
            Buchung basis, decimal rest, int vorlagenzeile)
        {
            var map = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase)
            {
                ["beschreibung"] = basis?.Beschreibung ?? "",
                ["B"] = basis?.Beschreibung ?? "",   // Alias, s.o.
                ["beleg"] = basis?.Belegnummer ?? "",
                ["konto"] = basis?.Konto ?? "",
                ["betrieb"] = basis?.Betrieb ?? "",
                ["bestandskonto"] = basis?.Bestandskonto ?? "",
                ["datum"] = basis?.Datum.ToString("dd.MM.yyyy") ?? "",
                ["brutto"] = Waehrungsformat.BetragOhneGruppierung(basis?.BruttoBetrag.BruttoWert ?? 0m),
                ["netto"] = Waehrungsformat.BetragOhneGruppierung(basis?.NettoWert ?? 0m),
                ["mwst"] = Waehrungsformat.BetragOhneGruppierung(basis?.MwstBetrag ?? 0m),
                ["rest"] = Waehrungsformat.BetragOhneGruppierung(rest),
                ["vorlagenzeile"] = vorlagenzeile.ToString(CultureInfo.InvariantCulture)
            };
            return map;
        }

        /// <summary>
        /// $vat1..$vat4 aus den Einstellungen ([Persoenliche_Daten]vatN),
        /// als Prozentwert (19 fuer "19"). Nicht konfigurierte Sätze fehlen
        /// in der Map (-> "Unbekannte Variable" statt stiller 0).
        /// </summary>
        private static void FuegeVatVariablenHinzu(Dictionary<string, decimal> map)
        {
            for (int i = 1; i <= 4; i++)
            {
                var roh = Einstellungen.Hole("[Persoenliche_Daten]vat" + i);
                if (string.IsNullOrWhiteSpace(roh)) continue;
                roh = roh.Trim().TrimEnd('%').Trim();
                decimal d;
                bool ok = roh.Contains(",")
                    ? decimal.TryParse(roh, NumberStyles.Number, DeDE, out d)
                    : decimal.TryParse(roh, NumberStyles.Number, CultureInfo.InvariantCulture, out d);
                if (ok) map["vat" + i] = d;
            }
        }
    }
}
