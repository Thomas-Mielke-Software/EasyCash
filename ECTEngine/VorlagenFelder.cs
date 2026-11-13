// VorlagenFelder.cs -- Zusatz-Felder und Original-Feld-Regeln der
// Buchungsvorlagen
//
// Eine Buchungsvorlage kann den Buchen-Dialog erweitern:
//
//   - ZUSATZ-FELDER: benannte Eingabefelder ("$anteil2"), die die Vorlage
//     deklariert (Platzhalter, Bezeichnung, Typ, Ziel-Box). Ihr Wert
//     fliesst als Variable in dieselben Formeln und Text-Templates wie
//     $brutto, $rest oder $beschreibung -- die Zusatz-Zeilen und die
//     Basis-Templates der Vorlage verbrauchen sie.
//
//   - ORIGINAL-FELD-REGELN: die festen Felder des Dialogs (Betrag, MWSt,
//     Konto, ...) lassen sich pro Vorlage sperren, ausblenden oder
//     umbenennen. Ein gesperrtes/ausgeblendetes Feld bekommt seinen Wert
//     aus der Vorlage.
//
// Diese Datei enthält nur Daten und Auswertung (ohne UI testbar):
// Definitionen, das Parsen/Validieren eingegebener Werte, die
// Variablen-Lieferung an den FormelParser und die Prüfung einer
// Vorlagen-Definition auf Widersprüche. Persistenz: Einstellungen.cs
// (ini) und PresetXml.cs (Austauschformat); Rechnen: Buchungsgruppen.cs.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

namespace ECTEngine
{
    // ======================================================================
    // Zusatz-Felder
    // ======================================================================

    /// <summary>Typ eines Zusatz-Felds -- bestimmt Eingabe-Control,
    /// Prüfung und die Form der gelieferten Variable.</summary>
    public enum ZusatzFeldTyp
    {
        /// <summary>Währungsbetrag (2 Nachkommastellen).</summary>
        Betrag,
        /// <summary>Beliebige Zahl (km, Tage, Faktor).</summary>
        Zahl,
        /// <summary>Prozentwert 0..100 (wie $mwstsatz: 19 für 19 %).</summary>
        Prozent,
        /// <summary>MWSt-Satz in Prozent, Auswahl aus vat1..vat4 + frei.</summary>
        Mwst,
        /// <summary>Einzeiliger Text.</summary>
        Text,
        /// <summary>Feste Auswahl ("Anzeige=Wert;Anzeige=Wert").</summary>
        Auswahl,
        /// <summary>Häkchen; liefert 1 oder 0.</summary>
        JaNein,
        /// <summary>Konto aus der Einnahmen- bzw. Ausgaben-Kontenliste.</summary>
        Konto,
        /// <summary>Betrieb (Icon-Liste wie das Original-Feld).</summary>
        Betrieb,
        /// <summary>Bestandskonto (Icon-Liste wie das Original-Feld).</summary>
        Bestandskonto,
        /// <summary>Datum (tt.mm.jjjj).</summary>
        Datum
    }

    /// <summary>Eine Option eines Auswahl-Felds.</summary>
    public sealed class FeldOption
    {
        public string Anzeige { get; }
        public string Wert { get; }
        public FeldOption(string anzeige, string wert)
        {
            Anzeige = anzeige ?? "";
            Wert = wert ?? "";
        }
    }

    /// <summary>
    /// Definition eines Zusatz-Felds. Alle Angaben sind rohe Strings aus
    /// ini/XML; unbekannte Typen/Boxen werden tolerant gelesen (Typ wird
    /// zu Text, Box zur Standard-Box des Typs), damit eine neuere Vorlage
    /// ein älteres Programm nicht aus dem Tritt bringt.
    /// ini-Keys pro Slot NN und Feld k: NNF&lt;k&gt;Name, NNF&lt;k&gt;Bez,
    /// NNF&lt;k&gt;Typ, NNF&lt;k&gt;Box, NNF&lt;k&gt;Vorgabe,
    /// NNF&lt;k&gt;Pflicht, NNF&lt;k&gt;Merken, NNF&lt;k&gt;Optionen.
    /// </summary>
    public sealed class PresetFeld
    {
        /// <summary>Platzhalter-Name OHNE '$' ("anteil2").</summary>
        public string Name { get; }
        /// <summary>Beschriftung im Buchen-Dialog.</summary>
        public string Bezeichnung { get; }
        /// <summary>Typ-Name wie gespeichert ("prozent").</summary>
        public string TypName { get; }
        public ZusatzFeldTyp Typ { get; }
        /// <summary>False, wenn TypName unbekannt war (Typ = Text).</summary>
        public bool TypBekannt { get; }
        /// <summary>Ziel-Box im Buchen-Dialog (normalisiert, siehe
        /// <see cref="VorlagenFelder.Boxen"/>).</summary>
        public string Box { get; }
        /// <summary>Vorbelegung (Konstante; bei Zahl-Typen auch eine Formel
        /// aus Konstanten und $vat1..$vat4).</summary>
        public string Vorbelegung { get; }
        /// <summary>True: Buchen nur mit ausgefülltem Feld.</summary>
        public bool Pflicht { get; }
        /// <summary>True: der zuletzt gebuchte Wert wird die Vorbelegung.</summary>
        public bool Merken { get; }
        /// <summary>Typ-abhängig: Auswahl = "Anzeige=Wert;...",
        /// Konto = "E"/"A" (leer = Buchungsart der Vorlage).</summary>
        public string Optionen { get; }

        public PresetFeld(string name, string bezeichnung, string typName,
            string box = "", string vorbelegung = "", bool pflicht = false,
            bool merken = false, string optionen = "")
        {
            Name = (name ?? "").Trim().TrimStart('$');
            Bezeichnung = (bezeichnung ?? "").Trim();
            TypName = (typName ?? "").Trim().ToLowerInvariant();
            Typ = VorlagenFelder.TypAusName(TypName, out bool bekannt);
            TypBekannt = bekannt;
            Box = VorlagenFelder.NormalisiereBox(box, Typ);
            Vorbelegung = vorbelegung ?? "";
            Pflicht = pflicht;
            Merken = merken;
            Optionen = optionen ?? "";
        }

        /// <summary>Beschriftung; fällt auf den Platzhalter zurück.</summary>
        public string AnzeigeBezeichnung =>
            Bezeichnung.Length > 0 ? Bezeichnung : "$" + Name;

        /// <summary>Liefert das Feld eine Zahl-Variable für Formeln?</summary>
        public bool IstZahlTyp =>
            Typ == ZusatzFeldTyp.Betrag || Typ == ZusatzFeldTyp.Zahl
            || Typ == ZusatzFeldTyp.Prozent || Typ == ZusatzFeldTyp.Mwst
            || Typ == ZusatzFeldTyp.JaNein;

        /// <summary>Optionen eines Auswahl-Felds ("Anzeige=Wert;..."; ohne
        /// '=' ist die Anzeige zugleich der Wert).</summary>
        public IReadOnlyList<FeldOption> AuswahlOptionen
        {
            get
            {
                var liste = new List<FeldOption>();
                foreach (var teil in Optionen.Split(';'))
                {
                    var t = teil.Trim();
                    if (t.Length == 0) continue;
                    int gleich = t.IndexOf('=');
                    liste.Add(gleich < 0
                        ? new FeldOption(t, t)
                        : new FeldOption(t.Substring(0, gleich).Trim(),
                                         t.Substring(gleich + 1).Trim()));
                }
                return liste;
            }
        }
    }

    /// <summary>
    /// Ein im Dialog eingegebener (oder gespeicherter) Feldwert, geprüft
    /// und in Variablen-Form gebracht. Ungültige Eingaben liefern
    /// <see cref="Fehler"/> UND einen neutralen Ersatzwert (0 bzw. ""),
    /// damit die Live-Vorschau während des Tippens weiterrechnen kann --
    /// gebucht wird erst, wenn kein Feld mehr einen Fehler meldet.
    /// </summary>
    public sealed class FeldWert
    {
        public PresetFeld Feld { get; internal set; }
        /// <summary>Wert in Speicherform (wie eingegeben bzw. kanonisch).</summary>
        public string Roh { get; internal set; } = "";
        /// <summary>True, wenn das Feld eine Zahl-Variable liefert.</summary>
        public bool HatZahl { get; internal set; }
        public decimal Zahl { get; internal set; }
        /// <summary>Wert für Text-Templates.</summary>
        public string Text { get; internal set; } = "";
        /// <summary>Prüf-Fehler ("" = ok).</summary>
        public string Fehler { get; internal set; } = "";
    }

    // ======================================================================
    // Original-Felder
    // ======================================================================

    public enum FeldZustand
    {
        /// <summary>Sichtbar und editierbar (Standard).</summary>
        Normal,
        /// <summary>Sichtbar, aber read-only; Wert kommt aus der Vorlage.</summary>
        Gesperrt,
        /// <summary>Im Dialog nicht sichtbar; Wert kommt aus der Vorlage.</summary>
        Ausgeblendet
    }

    /// <summary>Beschreibung eines Original-Felds des Buchen-Dialogs.</summary>
    public sealed class OriginalFeldInfo
    {
        /// <summary>Schlüssel (ini-Key-Bestandteil, XML-Attribut).</summary>
        public string Schluessel { get; }
        /// <summary>Beschriftung im Dialog ohne Vorlagen-Regel.</summary>
        public string Standardbezeichnung { get; }
        /// <summary>False: das Feld lässt sich nur umbenennen.</summary>
        public bool Sperrbar { get; }

        internal OriginalFeldInfo(string schluessel, string bezeichnung, bool sperrbar)
        {
            Schluessel = schluessel;
            Standardbezeichnung = bezeichnung;
            Sperrbar = sperrbar;
        }
    }

    /// <summary>Katalog der Original-Felder, die eine Vorlage steuern darf.
    /// Das Datum fehlt bewusst (immer sichtbar und editierbar); die
    /// Beschreibung trägt die Vorlagen-Auswahl und ist nur umbenennbar.</summary>
    public static class OriginalFeld
    {
        public const string Beschreibung  = "Beschreibung";
        public const string Betrag        = "Betrag";
        public const string Mwst          = "MWSt";
        public const string Beleg         = "Beleg";
        public const string Konto         = "Konto";
        public const string Bestandskonto = "Bestandskonto";
        public const string Betrieb       = "Betrieb";
        public const string Afa           = "AfA";

        public static readonly IReadOnlyList<OriginalFeldInfo> Alle = new[]
        {
            new OriginalFeldInfo(Beschreibung,  "Beschreibung",       sperrbar: false),
            new OriginalFeldInfo(Betrag,        "Brutto",             sperrbar: true),
            new OriginalFeldInfo(Mwst,          "MWSt %",             sperrbar: true),
            new OriginalFeldInfo(Beleg,         "Beleg-Nr.",          sperrbar: true),
            new OriginalFeldInfo(Konto,         "Konto",              sperrbar: true),
            new OriginalFeldInfo(Bestandskonto, "Bestandskonto",      sperrbar: true),
            new OriginalFeldInfo(Betrieb,       "Betrieb",            sperrbar: true),
            new OriginalFeldInfo(Afa,           "Abschreibung (AfA)", sperrbar: true),
        };

        public static OriginalFeldInfo Finde(string schluessel)
            => Alle.FirstOrDefault(f => string.Equals(f.Schluessel, schluessel,
                StringComparison.OrdinalIgnoreCase));
    }

    /// <summary>Regel einer Vorlage für ein Original-Feld.
    /// ini-Keys: NNO&lt;Feld&gt; ("gesperrt"/"ausgeblendet") und
    /// NNO&lt;Feld&gt;Bez (abweichende Bezeichnung).</summary>
    public sealed class OriginalFeldRegel
    {
        /// <summary>Schlüssel aus <see cref="OriginalFeld"/>.</summary>
        public string Feld { get; }
        public FeldZustand Zustand { get; }
        /// <summary>Abweichende Bezeichnung ("" = Standard).</summary>
        public string Bezeichnung { get; }

        public OriginalFeldRegel(string feld, FeldZustand zustand, string bezeichnung = "")
        {
            var info = OriginalFeld.Finde(feld);
            Feld = info?.Schluessel ?? (feld ?? "");
            // Nicht sperrbare Felder (Beschreibung) bleiben immer normal --
            // auch wenn eine von Hand editierte Vorlage etwas anderes sagt.
            Zustand = info != null && !info.Sperrbar ? FeldZustand.Normal : zustand;
            Bezeichnung = (bezeichnung ?? "").Trim();
        }

        /// <summary>True, wenn die Regel nichts ändert.</summary>
        public bool IstStandard =>
            Zustand == FeldZustand.Normal && Bezeichnung.Length == 0;

        public static string ZustandName(FeldZustand z) =>
            z == FeldZustand.Gesperrt ? "gesperrt"
            : z == FeldZustand.Ausgeblendet ? "ausgeblendet" : "";

        public static FeldZustand ZustandAusName(string s)
        {
            s = (s ?? "").Trim().ToLowerInvariant();
            return s == "gesperrt" ? FeldZustand.Gesperrt
                 : s == "ausgeblendet" ? FeldZustand.Ausgeblendet
                 : FeldZustand.Normal;
        }
    }

    // ======================================================================
    // Erweiterung einer Vorlage (alles, was über die klassischen
    // Preset-Felder und die Zusatz-Zeilen hinausgeht)
    // ======================================================================

    public sealed class VorlagenErweiterung
    {
        public static readonly VorlagenErweiterung Leer = new VorlagenErweiterung();

        public IReadOnlyList<PresetFeld> Felder { get; }
        /// <summary>Nur die vom Standard abweichenden Regeln.</summary>
        public IReadOnlyList<OriginalFeldRegel> OriginalFelder { get; }

        /// <summary>
        /// Formel für den EINGABE-Betrag, wenn das Betragsfeld gesperrt
        /// oder ausgeblendet ist (ini-Key NNEingabeBetrag), z.B.
        /// "$tage*14 + $km*0,30". Rechnet nur mit Zusatz-Feldern,
        /// Konstanten und $vat1..$vat4; das Ergebnis ist das $brutto der
        /// Vorlage. Bei editierbarem Betragsfeld ohne Bedeutung.
        /// </summary>
        public string EingabeBetragFormel { get; }
        /// <summary>Template für die Beschreibung der BASIS-Buchung
        /// (NNBasisText), z.B. "Bewirtung $anlass ($personen)". Leer = der
        /// Inhalt des Beschreibungsfelds wird unverändert gebucht.</summary>
        public string BasisTextTemplate { get; }
        /// <summary>Template für die Belegnummer der Basis (NNBasisBeleg).</summary>
        public string BasisBelegTemplate { get; }
        /// <summary>Bestandskonto der Basis (NNBasisBestandskonto); wird
        /// beim Laden der Vorlage in der Liste ausgewählt.</summary>
        public string BasisBestandskonto { get; }
        /// <summary>Betrieb der Basis (NNBasisBetrieb).</summary>
        public string BasisBetrieb { get; }

        public VorlagenErweiterung(
            IReadOnlyList<PresetFeld> felder = null,
            IReadOnlyList<OriginalFeldRegel> originalFelder = null,
            string eingabeBetragFormel = "", string basisTextTemplate = "",
            string basisBelegTemplate = "", string basisBestandskonto = "",
            string basisBetrieb = "")
        {
            Felder = felder ?? Array.Empty<PresetFeld>();
            OriginalFelder = (originalFelder ?? Array.Empty<OriginalFeldRegel>())
                .Where(r => r != null && !r.IstStandard).ToList();
            EingabeBetragFormel = (eingabeBetragFormel ?? "").Trim();
            BasisTextTemplate = basisTextTemplate ?? "";
            BasisBelegTemplate = basisBelegTemplate ?? "";
            BasisBestandskonto = (basisBestandskonto ?? "").Trim();
            BasisBetrieb = (basisBetrieb ?? "").Trim();
        }

        /// <summary>True, wenn die Vorlage den Dialog nicht erweitert.</summary>
        public bool IstLeer =>
            Felder.Count == 0 && OriginalFelder.Count == 0
            && EingabeBetragFormel.Length == 0 && BasisTextTemplate.Length == 0
            && BasisBelegTemplate.Length == 0 && BasisBestandskonto.Length == 0
            && BasisBetrieb.Length == 0;

        /// <summary>Regel für ein Original-Feld (Standard, wenn die Vorlage
        /// nichts festlegt).</summary>
        public OriginalFeldRegel RegelFuer(string feld)
        {
            foreach (var r in OriginalFelder)
                if (string.Equals(r.Feld, feld, StringComparison.OrdinalIgnoreCase))
                    return r;
            return new OriginalFeldRegel(feld, FeldZustand.Normal);
        }

        public FeldZustand ZustandVon(string feld) => RegelFuer(feld).Zustand;
    }

    // ======================================================================
    // Auswertung
    // ======================================================================

    public static class VorlagenFelder
    {
        /// <summary>Maximale Zahl der Zusatz-Felder pro Vorlage (F1..F12).</summary>
        public const int MaxFelder = 12;

        // ------------------------------------------------------------------
        // Kataloge (Name wie gespeichert, Anzeige für den Editor)
        // ------------------------------------------------------------------

        public static readonly IReadOnlyList<KeyValuePair<string, string>> Typen = new[]
        {
            new KeyValuePair<string, string>("betrag",        "Betrag"),
            new KeyValuePair<string, string>("zahl",          "Zahl"),
            new KeyValuePair<string, string>("prozent",       "Prozent"),
            new KeyValuePair<string, string>("mwst",          "MWSt-Satz"),
            new KeyValuePair<string, string>("text",          "Text"),
            new KeyValuePair<string, string>("auswahl",       "Auswahl"),
            new KeyValuePair<string, string>("janein",        "Häkchen (ja/nein)"),
            new KeyValuePair<string, string>("konto",         "Konto"),
            new KeyValuePair<string, string>("betrieb",       "Betrieb"),
            new KeyValuePair<string, string>("bestandskonto", "Bestandskonto"),
            new KeyValuePair<string, string>("datum",         "Datum"),
        };

        public static readonly IReadOnlyList<KeyValuePair<string, string>> Boxen = new[]
        {
            new KeyValuePair<string, string>("datum",         "Datum"),
            new KeyValuePair<string, string>("beschreibung",  "Beschreibung"),
            new KeyValuePair<string, string>("betrag",        "Betrag"),
            new KeyValuePair<string, string>("zuordnung",     "Zuordnung"),
            new KeyValuePair<string, string>("bestandskonto", "Bestandskonto"),
            new KeyValuePair<string, string>("betrieb",       "Betrieb"),
            new KeyValuePair<string, string>("afa",           "Abschreibung (AfA)"),
        };

        public static ZusatzFeldTyp TypAusName(string name, out bool bekannt)
        {
            bekannt = true;
            switch ((name ?? "").Trim().ToLowerInvariant())
            {
                case "betrag":        return ZusatzFeldTyp.Betrag;
                case "zahl":          return ZusatzFeldTyp.Zahl;
                case "prozent":       return ZusatzFeldTyp.Prozent;
                case "mwst":          return ZusatzFeldTyp.Mwst;
                case "text":          return ZusatzFeldTyp.Text;
                case "auswahl":       return ZusatzFeldTyp.Auswahl;
                case "janein":        return ZusatzFeldTyp.JaNein;
                case "konto":         return ZusatzFeldTyp.Konto;
                case "betrieb":       return ZusatzFeldTyp.Betrieb;
                case "bestandskonto": return ZusatzFeldTyp.Bestandskonto;
                case "datum":         return ZusatzFeldTyp.Datum;
            }
            bekannt = false;
            return ZusatzFeldTyp.Text;
        }

        /// <summary>Box, in der ein Feld dieses Typs ohne Angabe landet.</summary>
        public static string StandardBox(ZusatzFeldTyp typ)
        {
            switch (typ)
            {
                case ZusatzFeldTyp.Betrag:
                case ZusatzFeldTyp.Zahl:
                case ZusatzFeldTyp.Prozent:
                case ZusatzFeldTyp.Mwst:
                case ZusatzFeldTyp.JaNein:        return "betrag";
                case ZusatzFeldTyp.Konto:         return "zuordnung";
                case ZusatzFeldTyp.Betrieb:       return "betrieb";
                case ZusatzFeldTyp.Bestandskonto: return "bestandskonto";
                default:                          return "beschreibung";
            }
        }

        public static string NormalisiereBox(string box, ZusatzFeldTyp typ)
        {
            var b = (box ?? "").Trim().ToLowerInvariant();
            foreach (var kv in Boxen)
                if (kv.Key == b) return b;
            return StandardBox(typ);
        }

        // ------------------------------------------------------------------
        // Platzhalter-Namen
        // ------------------------------------------------------------------

        /// <summary>
        /// Namen, die ein Zusatz-Feld nicht tragen darf: die eingebauten
        /// Variablen der Formeln/Templates und die Länderkürzel der
        /// Konto-Feld-Spezifikation ("$de:..."). Die einbuchstabigen
        /// Datums-Platzhalter ($M, $J, ...) schließt schon die
        /// Mindestlänge von zwei Zeichen aus.
        /// </summary>
        private static readonly HashSet<string> _reserviert =
            new HashSet<string>(StringComparer.OrdinalIgnoreCase)
            {
                "brutto", "netto", "mwst", "mwstsatz", "rest", "vorlagenzeile",
                "vat1", "vat2", "vat3", "vat4",
                "beschreibung", "beleg", "konto", "betrieb", "bestandskonto",
                "datum", "name", "de", "at", "ch"
            };

        /// <summary>Prüft einen Platzhalter-Namen (ohne '$'). Liefert ""
        /// oder den Grund der Ablehnung.</summary>
        public static string PruefeName(string name)
        {
            name = (name ?? "").Trim().TrimStart('$');
            if (name.Length == 0) return "Der Platzhalter fehlt.";
            if (name.Length < 2)
                return "Der Platzhalter braucht mindestens zwei Zeichen.";
            if (!((name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z')))
                return "Der Platzhalter muss mit einem Buchstaben (a-z) beginnen.";
            foreach (char c in name)
            {
                bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                       || (c >= '0' && c <= '9') || c == '_';
                if (!ok)
                    return "Im Platzhalter sind nur Buchstaben a-z, Ziffern und '_' erlaubt.";
            }
            if (_reserviert.Contains(name))
                return "'$" + name + "' ist ein eingebauter Platzhalter und kann nicht "
                     + "für ein Zusatz-Feld verwendet werden.";
            return "";
        }

        // ------------------------------------------------------------------
        // Werte: Eingabe prüfen und in Variablen-Form bringen
        // ------------------------------------------------------------------

        /// <summary>Prüft den rohen Wert eines Felds (Eingabe im Dialog
        /// oder gespeicherter Wert einer Buchung).</summary>
        public static FeldWert Werte(PresetFeld feld, string roh)
        {
            var w = new FeldWert { Feld = feld, Roh = (roh ?? "").Trim() };
            if (feld == null) return w;
            bool leer = w.Roh.Length == 0;
            string was = feld.AnzeigeBezeichnung;

            switch (feld.Typ)
            {
                case ZusatzFeldTyp.Betrag:
                case ZusatzFeldTyp.Zahl:
                    w.HatZahl = true;
                    if (leer)
                    {
                        if (feld.Pflicht) w.Fehler = "Bitte \"" + was + "\" angeben.";
                    }
                    else if (Waehrungsformat.TryParse(w.Roh, out decimal d))
                    {
                        w.Zahl = feld.Typ == ZusatzFeldTyp.Betrag
                            ? decimal.Round(d, 2, MidpointRounding.AwayFromZero) : d;
                    }
                    else
                        w.Fehler = "\"" + was + "\": '" + w.Roh + "' ist "
                            + (feld.Typ == ZusatzFeldTyp.Betrag
                                ? "kein gültiger Betrag." : "keine gültige Zahl.");
                    w.Text = feld.Typ == ZusatzFeldTyp.Betrag
                        ? Waehrungsformat.BetragOhneGruppierung(w.Zahl)
                        : Waehrungsformat.Zahl(w.Zahl, "0.######");
                    break;

                case ZusatzFeldTyp.Prozent:
                case ZusatzFeldTyp.Mwst:
                    w.HatZahl = true;
                    if (leer)
                    {
                        if (feld.Pflicht) w.Fehler = "Bitte \"" + was + "\" angeben.";
                    }
                    else if (!Waehrungsformat.TryParseProzent(w.Roh, out decimal p))
                        w.Fehler = "\"" + was + "\": '" + w.Roh + "' ist keine gültige Zahl.";
                    else if (p < 0 || p > 100)
                        w.Fehler = "\"" + was + "\" muss zwischen 0 und 100 liegen.";
                    else
                        w.Zahl = p;
                    w.Text = Waehrungsformat.Zahl(w.Zahl, "0.######");
                    break;

                case ZusatzFeldTyp.JaNein:
                    w.HatZahl = true;
                    bool ja = IstJa(w.Roh);
                    w.Zahl = ja ? 1m : 0m;
                    w.Roh = ja ? "1" : "0";
                    w.Text = ja ? "ja" : "nein";
                    break;

                case ZusatzFeldTyp.Auswahl:
                    if (leer)
                    {
                        if (feld.Pflicht) w.Fehler = "Bitte \"" + was + "\" auswählen.";
                    }
                    else if (!feld.AuswahlOptionen.Any(o => o.Wert == w.Roh))
                    {
                        w.Fehler = "\"" + was + "\": '" + w.Roh
                            + "' ist keine der vorgesehenen Auswahlmöglichkeiten.";
                    }
                    w.Text = w.Fehler.Length == 0 ? w.Roh : "";
                    // Zahl-Variable nur, wenn der Wert eine Zahl ist
                    // ("ermäßigt=7" -> $satz in Formeln nutzbar)
                    if (!leer && w.Fehler.Length == 0
                        && Waehrungsformat.TryParse(w.Roh, out decimal az))
                    {
                        w.HatZahl = true;
                        w.Zahl = az;
                    }
                    break;

                case ZusatzFeldTyp.Datum:
                    if (leer)
                    {
                        if (feld.Pflicht) w.Fehler = "Bitte \"" + was + "\" angeben.";
                    }
                    else if (TryParseDatum(w.Roh, out DateTime dt))
                    {
                        w.Text = dt.ToString("dd.MM.yyyy", CultureInfo.InvariantCulture);
                        w.Roh = w.Text;
                    }
                    else
                        w.Fehler = "\"" + was + "\": bitte ein Datum in der Form "
                                 + "tt.mm.jjjj angeben.";
                    break;

                case ZusatzFeldTyp.Konto:
                case ZusatzFeldTyp.Betrieb:
                case ZusatzFeldTyp.Bestandskonto:
                    if (leer && feld.Pflicht)
                        w.Fehler = "Bitte \"" + was + "\" auswählen.";
                    w.Text = w.Roh;
                    break;

                default:   // Text
                    if (leer && feld.Pflicht)
                        w.Fehler = "Bitte \"" + was + "\" angeben.";
                    w.Text = w.Roh;
                    break;
            }
            return w;
        }

        private static bool IstJa(string s)
        {
            s = (s ?? "").Trim().ToLowerInvariant();
            return s == "1" || s == "ja" || s == "true" || s == "x";
        }

        private static readonly string[] _datumsFormate =
            { "d.M.yyyy", "d.M.yy", "yyyy-M-d", "d/M/yyyy" };

        private static bool TryParseDatum(string s, out DateTime datum)
            => DateTime.TryParseExact((s ?? "").Trim(), _datumsFormate,
                CultureInfo.InvariantCulture, DateTimeStyles.None, out datum);

        /// <summary>
        /// Startwert eines Felds aus seiner Vorbelegung. Bei Zahl-Typen
        /// darf die Vorbelegung eine Formel aus Konstanten und
        /// $vat1..$vat4 sein ("$vat2" = der zweite MWSt-Satz).
        /// </summary>
        public static string StandardWert(PresetFeld feld)
        {
            if (feld == null) return "";
            var v = feld.Vorbelegung.Trim();
            if (v.Length == 0)
                return feld.Typ == ZusatzFeldTyp.JaNein ? "0" : "";

            switch (feld.Typ)
            {
                case ZusatzFeldTyp.JaNein:
                    return IstJa(v) ? "1" : "0";
                case ZusatzFeldTyp.Betrag:
                case ZusatzFeldTyp.Zahl:
                case ZusatzFeldTyp.Prozent:
                case ZusatzFeldTyp.Mwst:
                    var e = FormelParser.Berechne(v, FormelParser.FeldVariablen(null));
                    if (!e.Ok)
                        // "$vat2", aber der Mandant hat keinen zweiten
                        // MWSt-Satz gepflegt: dann eben keine Vorbelegung.
                        // Alles andere Unlesbare bleibt stehen, damit die
                        // Prüfung der Vorlage es meldet.
                        return v.IndexOf('$') >= 0 ? "" : v;
                    return feld.Typ == ZusatzFeldTyp.Betrag
                        ? Waehrungsformat.BetragOhneGruppierung(e.Wert)
                        : Waehrungsformat.Zahl(e.Wert, "0.######");
                case ZusatzFeldTyp.Auswahl:
                    // Vorbelegung darf den Wert ODER die Anzeige nennen
                    foreach (var o in feld.AuswahlOptionen)
                        if (o.Wert == v || o.Anzeige == v) return o.Wert;
                    return v;
                default:
                    return v;
            }
        }

        // ------------------------------------------------------------------
        // Variablen für FormelParser
        // ------------------------------------------------------------------

        /// <summary>Trägt die Zahl-Variablen der Felder ein. Eingebaute
        /// Variablen gewinnen bei Namensgleichheit (der Editor weist
        /// solche Namen ohnehin ab).</summary>
        public static void ErgaenzeVariablen(Dictionary<string, decimal> ziel,
            IEnumerable<FeldWert> felder)
        {
            if (ziel == null || felder == null) return;
            foreach (var w in felder)
                if (w?.Feld != null && w.HatZahl && w.Feld.Name.Length > 0
                    && !ziel.ContainsKey(w.Feld.Name))
                    ziel[w.Feld.Name] = w.Zahl;
        }

        /// <summary>Trägt die Text-Variablen der Felder ein.</summary>
        public static void ErgaenzeVariablen(Dictionary<string, string> ziel,
            IEnumerable<FeldWert> felder)
        {
            if (ziel == null || felder == null) return;
            foreach (var w in felder)
                if (w?.Feld != null && w.Feld.Name.Length > 0
                    && !ziel.ContainsKey(w.Feld.Name))
                    ziel[w.Feld.Name] = w.Text ?? "";
        }

        /// <summary>Beispielwerte aller Felder einer Vorlage -- für die
        /// Formelprüfung im Editor zählen nur die NAMEN.</summary>
        public static IReadOnlyList<FeldWert> Beispielwerte(IEnumerable<PresetFeld> felder)
        {
            var liste = new List<FeldWert>();
            if (felder == null) return liste;
            foreach (var f in felder)
            {
                if (f == null) continue;
                liste.Add(new FeldWert
                {
                    Feld = f,
                    // Auswahl-Felder liefern nur dann eine Zahl, wenn ihre
                    // Werte Zahlen sind
                    HatZahl = f.IstZahlTyp || (f.Typ == ZusatzFeldTyp.Auswahl
                        && f.AuswahlOptionen.Count > 0
                        && f.AuswahlOptionen.All(o =>
                               Waehrungsformat.TryParse(o.Wert, out _))),
                    Zahl = 1m,
                    Text = "x"
                });
            }
            return liste;
        }

        // ------------------------------------------------------------------
        // Prüfung einer Vorlagen-Definition
        // ------------------------------------------------------------------

        /// <summary>
        /// Sucht Widersprüche in einer Vorlage, die erst beim Buchen
        /// auffielen: doppelte/ungültige Platzhalter, Felder ohne Optionen,
        /// gesperrte oder ausgeblendete Original-Felder ohne Wert aus der
        /// Vorlage, Formeln mit unbekannten Variablen. Liefert endnutzer-
        /// taugliche Sätze (leer = alles in Ordnung).
        /// </summary>
        public static IReadOnlyList<string> Pruefe(Preset p)
        {
            var probleme = new List<string>();
            if (p == null) return probleme;
            var e = p.Erweiterung;

            // Zusatz-Felder
            var gesehen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (var f in e.Felder)
            {
                string nameFehler = PruefeName(f.Name);
                if (nameFehler.Length > 0)
                {
                    probleme.Add("Zusatz-Feld \"" + f.AnzeigeBezeichnung + "\": " + nameFehler);
                    continue;
                }
                if (!gesehen.Add(f.Name))
                    probleme.Add("Der Platzhalter '$" + f.Name + "' ist mehrfach vergeben.");
                if (!f.TypBekannt)
                    probleme.Add("Zusatz-Feld '$" + f.Name + "': unbekannter Typ '"
                        + f.TypName + "' (wird als Text behandelt).");
                if (f.Typ == ZusatzFeldTyp.Auswahl && f.AuswahlOptionen.Count == 0)
                    probleme.Add("Zusatz-Feld '$" + f.Name + "': bitte die "
                        + "Auswahlmöglichkeiten angeben (Anzeige=Wert;Anzeige=Wert).");
                if (f.Vorbelegung.Trim().Length > 0)
                {
                    var w = Werte(f, StandardWert(f));
                    if (w.Fehler.Length > 0)
                        probleme.Add("Zusatz-Feld '$" + f.Name
                            + "': die Vorbelegung passt nicht zum Typ.");
                }
            }

            // Original-Felder: was der Dialog nicht eingeben lässt, muss
            // die Vorlage liefern
            bool nichtEingebbar(string feld) =>
                e.ZustandVon(feld) != FeldZustand.Normal;

            if (nichtEingebbar(OriginalFeld.Betrag) && e.EingabeBetragFormel.Length == 0)
                probleme.Add("Der Betrag ist im Buchen-Dialog nicht eingebbar, "
                    + "aber \"Betrag (Formel)\" ist leer -- so kann nicht gebucht werden.");
            if (nichtEingebbar(OriginalFeld.Konto) && string.IsNullOrWhiteSpace(p.Konto))
                probleme.Add("Das Konto ist im Buchen-Dialog nicht wählbar, "
                    + "aber die Vorlage nennt kein Konto.");
            if (nichtEingebbar(OriginalFeld.Bestandskonto) && e.BasisBestandskonto.Length == 0)
                probleme.Add("Das Bestandskonto ist im Buchen-Dialog nicht wählbar, "
                    + "aber die Vorlage nennt keines (nötig, sobald Bestandskonten "
                    + "angelegt sind).");
            if (nichtEingebbar(OriginalFeld.Betrieb) && e.BasisBetrieb.Length == 0)
                probleme.Add("Der Betrieb ist im Buchen-Dialog nicht wählbar, "
                    + "aber die Vorlage nennt keinen (nötig, sobald Betriebe "
                    + "angelegt sind).");

            // Formeln
            var beispiele = Beispielwerte(e.Felder);
            if (e.EingabeBetragFormel.Length > 0)
            {
                string f = FormelParser.Pruefe(e.EingabeBetragFormel,
                    FormelParser.FeldVariablen(beispiele));
                if (f != null)
                    probleme.Add("Betrag (Formel): " + f
                        + " (hier sind nur Zusatz-Felder, Zahlen und $vat1..$vat4 erlaubt)");
            }
            var zahlVars = FormelParser.ZahlVariablen(
                new Buchung { BruttoBetrag = Betrag.AusCent(11900, 19000) },
                100m, 1, beispiele);
            if (!string.IsNullOrWhiteSpace(p.BasisBetragFormel))
            {
                string f = FormelParser.Pruefe(p.BasisBetragFormel, zahlVars);
                if (f != null) probleme.Add("Basis bucht: " + f);
            }
            for (int i = 0; i < p.Zeilen.Count; i++)
            {
                var z = p.Zeilen[i];
                if (string.IsNullOrWhiteSpace(z.BetragFormel))
                {
                    probleme.Add("Zusatz-Zeile " + (i + 1) + " hat keine Betrags-Formel "
                        + "und wird beim Buchen übersprungen.");
                    continue;
                }
                string f = FormelParser.Pruefe(z.BetragFormel, zahlVars);
                if (f != null) probleme.Add("Zusatz-Zeile " + (i + 1) + ", Betrag: " + f);
            }
            return probleme;
        }

        // ------------------------------------------------------------------
        // Kodierung für den ErweiterungStore der Buchung
        // ------------------------------------------------------------------

        /// <summary>Macht einen Feldwert tauglich für das Pipe-Format der
        /// Buchungs-Erweiterung ('|' trennt dort die Einträge).</summary>
        public static string Kodiere(string wert)
        {
            if (string.IsNullOrEmpty(wert)) return "";
            var sb = new StringBuilder(wert.Length + 8);
            foreach (char c in wert)
            {
                switch (c)
                {
                    case '%':  sb.Append("%25"); break;
                    case '|':  sb.Append("%7C"); break;
                    case '\r': sb.Append("%0D"); break;
                    case '\n': sb.Append("%0A"); break;
                    default:   sb.Append(c);     break;
                }
            }
            return sb.ToString();
        }

        public static string Dekodiere(string wert)
        {
            if (string.IsNullOrEmpty(wert) || wert.IndexOf('%') < 0) return wert ?? "";
            return wert.Replace("%7C", "|").Replace("%0D", "\r")
                       .Replace("%0A", "\n").Replace("%25", "%");
        }
    }
}
