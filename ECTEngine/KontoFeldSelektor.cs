// KontoFeldSelektor.cs -- Ad-hoc-Kontoselektor über Formularfeld-Verknüpfungen
//
// Löst im Konto-Feld einer Buchungsgruppen-Vorlage (und über die Plugin-API
// HoleKontoMitFeldern) eine Feld-Spezifikation der Form
//
//   $de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=48||at:Beilage E1a=9040|Umsatzsteuer=1020||
//
// zum ERSTBESTEN Konto auf, das mit genau dieser Kombination von
// Formularfeldern verknüpft ist. Aufbau der Spezifikation:
//   - '$' + Land-Blöcke, getrennt durch "||" (abschließendes "||" optional)
//   - Land-Block: <kürzel>:<paar>|<paar>|...   Kürzel: de/at/ch
//     (Zuordnung zur Land-Einstellung: 0=de, 1=at, 2=ch)
//   - Paar: <Formularname>=<Feld-Id> -- ALLE Paare des aktiven Landes
//     müssen am Konto verknüpft sein (UND-Kombination).
//   - '|', '=' dürfen nicht in Formularnamen/Feld-Ids vorkommen (dieselbe
//     Einschränkung wie beim ErweiterungStore-Pipe-Format).
//
// Findet sich kein Konto, liefert ErmittleFeldInfo() die Feldnamen und den
// E/A-Typ aus den .ecf-Formularen, damit die UI (KontoAnlegenView bzw. der
// native Dialog der V3-Plugin-API) ein passendes Konto anlegen kann;
// ErzeugeKonto() persistiert es samt Feldzuweisungen über die
// EUKonten-Mechanik. Vorläufer/Spezialfall: HoleKontoFuerFeld (ectifacemisc,
// fest auf E/Ü-Rechnung + USt-Voranmeldung, ohne Länder und ohne Anlage).
//
// Bewusst OHNE UI -- Dialoge liegen in ECTViews (V4) bzw. ECTIFace (V3).

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;

namespace ECTEngine
{
    /// <summary>Eine benötigte Feld-Verknüpfung: Formularname + Feld-Id.</summary>
    public sealed class KontoFeldBedarf
    {
        public string Formular { get; }
        public string FeldId   { get; }

        public KontoFeldBedarf(string formular, string feldId)
        {
            Formular = (formular ?? "").Trim();
            FeldId   = (feldId   ?? "").Trim();
        }
    }

    /// <summary>Geparste Feld-Spezifikation: pro Land-Kürzel die Liste der
    /// benötigten Feld-Verknüpfungen.</summary>
    public sealed class KontoFeldSpezifikation
    {
        private readonly Dictionary<string, List<KontoFeldBedarf>> _proLand;

        private KontoFeldSpezifikation(Dictionary<string, List<KontoFeldBedarf>> proLand)
        {
            _proLand = proLand;
        }

        public IReadOnlyCollection<string> Laender => _proLand.Keys;

        /// <summary>Bedarfsliste für ein Land-Kürzel ("de"/"at"/"ch");
        /// null wenn die Spezifikation das Land nicht kennt.</summary>
        public IReadOnlyList<KontoFeldBedarf> FuerLand(string kuerzel)
        {
            if (string.IsNullOrEmpty(kuerzel)) return null;
            return _proLand.TryGetValue(kuerzel.ToLowerInvariant(), out var liste)
                ? liste : null;
        }

        /// <summary>
        /// Heuristik: Sieht der Konto-Feld-Inhalt wie eine Feld-Spezifikation
        /// aus? ('$' + zwei Buchstaben + ':' -- kollidiert nicht mit den
        /// Text-Template-Variablen wie $beschreibung/$beleg oder den
        /// Datums-Platzhaltern der Dauerbuchungen).
        /// </summary>
        public static bool IstSpezifikation(string kontoFeld)
        {
            var s = (kontoFeld ?? "").TrimStart();
            return s.Length >= 4 && s[0] == '$'
                && char.IsLetter(s[1]) && char.IsLetter(s[2]) && s[3] == ':';
        }

        /// <summary>
        /// Parst die Spezifikation. Liefert null + Fehlertext bei
        /// Syntaxfehlern (Fehlertexte deutsch, endnutzertauglich --
        /// erscheinen in der Live-Validierung des Vorlagen-Editors).
        /// </summary>
        public static KontoFeldSpezifikation Parse(string spez, out string fehler)
        {
            fehler = "";
            var s = (spez ?? "").Trim();
            if (s.StartsWith("$", StringComparison.Ordinal)) s = s.Substring(1);
            if (s.Length == 0)
            {
                fehler = "Leere Feld-Spezifikation.";
                return null;
            }

            var proLand = new Dictionary<string, List<KontoFeldBedarf>>(StringComparer.OrdinalIgnoreCase);

            // Land-Blöcke sind durch "||" getrennt; ein abschließendes "||"
            // erzeugt einen leeren letzten Block, der ignoriert wird.
            foreach (var blockRoh in s.Split(new[] { "||" }, StringSplitOptions.None))
            {
                var block = blockRoh.Trim();
                if (block.Length == 0) continue;

                int doppelpunkt = block.IndexOf(':');
                if (doppelpunkt <= 0)
                {
                    fehler = $"Land-Kürzel fehlt (erwartet z.B. \"de:\") in \"{Kuerze(block)}\".";
                    return null;
                }
                var land = block.Substring(0, doppelpunkt).Trim().ToLowerInvariant();
                if (land.Length != 2 || !land.All(char.IsLetter))
                {
                    fehler = $"\"{land}\" ist kein gültiges Land-Kürzel (de/at/ch).";
                    return null;
                }

                var paare = new List<KontoFeldBedarf>();
                foreach (var paarRoh in block.Substring(doppelpunkt + 1)
                             .Split(new[] { '|' }, StringSplitOptions.None))
                {
                    var paar = paarRoh.Trim();
                    if (paar.Length == 0) continue;
                    int gleich = paar.IndexOf('=');
                    if (gleich <= 0 || gleich == paar.Length - 1)
                    {
                        fehler = $"\"{Kuerze(paar)}\" ist kein gültiges Paar (erwartet Formularname=Feld-Id).";
                        return null;
                    }
                    paare.Add(new KontoFeldBedarf(
                        paar.Substring(0, gleich), paar.Substring(gleich + 1)));
                }
                if (paare.Count == 0)
                {
                    fehler = $"Land-Block \"{land}:\" enthält keine Feld-Zuordnung.";
                    return null;
                }

                if (proLand.TryGetValue(land, out var vorhanden))
                    vorhanden.AddRange(paare);   // zweiter Block gleichen Landes ergänzt
                else
                    proLand[land] = paare;
            }

            if (proLand.Count == 0)
            {
                fehler = "Die Spezifikation enthält keinen Land-Block.";
                return null;
            }
            return new KontoFeldSpezifikation(proLand);
        }

        private static string Kuerze(string s) =>
            s.Length <= 40 ? s : s.Substring(0, 37) + "...";
    }

    /// <summary>Ergebnis von <see cref="KontoFeldSelektor.LoeseAuf"/>.</summary>
    public sealed class KontoFeldAufloesung
    {
        /// <summary>False: der Konto-Feld-Inhalt ist gar keine Spezifikation
        /// (normales Konto/Template) -- die übrigen Felder sind dann leer.</summary>
        public bool IstSpezifikation { get; internal set; }

        /// <summary>Name des gefundenen Kontos; null wenn keines passt.</summary>
        public string Konto { get; internal set; }

        /// <summary>Parse-/Land-Fehler ("" = ok). Bei Fehler ist Konto null
        /// und Bedarf leer.</summary>
        public string Fehler { get; internal set; } = "";

        /// <summary>Benötigte Feld-Verknüpfungen für das AKTIVE Land
        /// (Grundlage der Konto-Anlage, wenn Konto == null).</summary>
        public IReadOnlyList<KontoFeldBedarf> Bedarf { get; internal set; }
            = Array.Empty<KontoFeldBedarf>();
    }

    /// <summary>Feld-Infos aus den .ecf-Formularen für die Konto-Anlage.</summary>
    public sealed class KontoFeldInfo
    {
        /// <summary>Feld-Bezeichnungen in Bedarfs-Reihenfolge.</summary>
        public IReadOnlyList<string> Feldnamen { get; internal set; }
            = Array.Empty<string>();
        /// <summary>True = Einnahmenkonto anlegen, false = Ausgabenkonto.</summary>
        public bool IstEinnahme { get; internal set; }
        /// <summary>"" = ok; sonst Grund, warum keine Anlage möglich ist
        /// (Formular fehlt, Feld unbekannt, E/A gemischt).</summary>
        public string Fehler { get; internal set; } = "";
    }

    public static class KontoFeldSelektor
    {
        /// <summary>Kürzel des in den Einstellungen gewählten Landes
        /// ([Persoenliche_Daten]land: 0=de, 1=at, 2=ch).</summary>
        public static string AktivesLandKuerzel()
        {
            switch (GlobaleEinstellungen.Land)
            {
                case 1:  return "at";
                case 2:  return "ch";
                default: return "de";
            }
        }

        /// <summary>
        /// Löst einen Konto-Feld-Inhalt auf: keine Spezifikation -> Ergebnis
        /// mit IstSpezifikation=false; sonst Parsen, Bedarf des aktiven
        /// Landes bestimmen und das erstbeste verknüpfte Konto suchen.
        /// Rein lesend (kein Dialog, keine Anlage) -- darf live bei jeder
        /// Neuberechnung der Gruppen-Zeilen laufen.
        /// </summary>
        public static KontoFeldAufloesung LoeseAuf(string kontoFeld)
        {
            var ergebnis = new KontoFeldAufloesung();
            if (!KontoFeldSpezifikation.IstSpezifikation(kontoFeld))
                return ergebnis;
            ergebnis.IstSpezifikation = true;

            var spez = KontoFeldSpezifikation.Parse(kontoFeld, out string fehler);
            if (spez == null)
            {
                ergebnis.Fehler = fehler;
                return ergebnis;
            }

            string land = AktivesLandKuerzel();
            var bedarf = spez.FuerLand(land);
            if (bedarf == null)
            {
                ergebnis.Fehler = $"Die Vorlage enthält keine Feld-Verknüpfung für das eingestellte Land (\"{land}\").";
                return ergebnis;
            }

            ergebnis.Bedarf = bedarf;
            ergebnis.Konto = FindeKonto(bedarf);
            return ergebnis;
        }

        /// <summary>
        /// Sucht das erstbeste Konto (Slot-Reihenfolge, Einnahmen vor
        /// Ausgaben), das ALLE geforderten Feld-Verknüpfungen trägt.
        /// Liest direkt aus dem Einstellungs-Cache (Feldzuweisungs-Blobs im
        /// ErweiterungStore-Pipe-Format) -- kein EUKonten.Lade() nötig.
        /// </summary>
        public static string FindeKonto(IReadOnlyList<KontoFeldBedarf> bedarf)
        {
            if (bedarf == null || bedarf.Count == 0) return null;
            return FindeKontoInGruppe(Einstellungen.EinnahmenKonten, "[EinnahmenFeldzuweisungen]", bedarf)
                ?? FindeKontoInGruppe(Einstellungen.AusgabenKonten,  "[AusgabenFeldzuweisungen]",  bedarf);
        }

        private static string FindeKontoInGruppe(IReadOnlyList<string> namen,
            string fzSektion, IReadOnlyList<KontoFeldBedarf> bedarf)
        {
            for (int i = 0; i < namen.Count; i++)
            {
                var blob = Einstellungen.Hole(
                    fzSektion + i.ToString("D2", CultureInfo.InvariantCulture));
                if (string.IsNullOrEmpty(blob)) continue;

                var store = ErweiterungStore.AusPipeFormat(blob);
                bool passt = true;
                foreach (var b in bedarf)
                {
                    // Feldzuweisungs-Werte sind reine Feld-Ids (siehe
                    // EUKonten.SpeichereFeldzuweisungen); Trim für
                    // handeditierte ini-Bestände.
                    var wert = store.Hole("ECT", b.Formular, "").Trim();
                    if (!string.Equals(wert, b.FeldId, StringComparison.Ordinal))
                    {
                        passt = false;
                        break;
                    }
                }
                if (passt) return namen[i];
            }
            return null;
        }

        /// <summary>
        /// Ermittelt aus den installierten .ecf-Formularen die
        /// Feld-Bezeichnungen und den E/A-Typ des Bedarfs -- Grundlage für
        /// Hinweistext und Vorgabe-Name des Anlage-Dialogs. Durchsucht alle
        /// Jahres-Varianten des Formulars (neueste zuerst).
        /// </summary>
        public static KontoFeldInfo ErmittleFeldInfo(IReadOnlyList<KontoFeldBedarf> bedarf)
        {
            var info = new KontoFeldInfo();
            if (bedarf == null || bedarf.Count == 0)
            {
                info.Fehler = "Keine Feld-Verknüpfung angegeben.";
                return info;
            }

            EUKonten.Lade();   // Formular-Gruppen einmalig aus .ecf (gecacht)

            var namen = new List<string>();
            bool? einnahme = null;
            foreach (var b in bedarf)
            {
                var gruppe = EUKonten.FormularGruppen.FirstOrDefault(
                    g => string.Equals(g.Name, b.Formular, StringComparison.OrdinalIgnoreCase));
                if (gruppe == null)
                {
                    info.Fehler = $"Das Formular \"{b.Formular}\" ist nicht installiert (keine passende .ecf-Datei).";
                    return info;
                }

                Formularfeld feld = null;
                foreach (var variante in gruppe.Varianten)   // neueste zuerst
                {
                    feld = variante.Felder.FirstOrDefault(
                        f => string.Equals(f.Id, b.FeldId, StringComparison.Ordinal));
                    if (feld != null) break;
                }
                if (feld == null)
                {
                    info.Fehler = $"Das Formular \"{b.Formular}\" hat kein zuweisbares Feld mit der Id \"{b.FeldId}\".";
                    return info;
                }

                namen.Add(string.IsNullOrEmpty(feld.Bezeichnung)
                    ? b.Formular + " Feld " + b.FeldId : feld.Bezeichnung);
                if (einnahme == null)
                    einnahme = feld.IstEinnahme;
                else if (einnahme != feld.IstEinnahme)
                {
                    info.Fehler = "Die geforderten Felder mischen Einnahmen- und Ausgaben-Typ -- ein Konto kann nur einem Typ angehören.";
                    return info;
                }
            }

            info.Feldnamen = namen;
            info.IstEinnahme = einnahme ?? true;
            return info;
        }

        /// <summary>Vorgabe für den Kontonamen: Feldnamen mit " / " verbunden.</summary>
        public static string VorgabeName(IReadOnlyList<string> feldnamen)
            => string.Join(" / ", feldnamen ?? (IReadOnlyList<string>)Array.Empty<string>());

        /// <summary>
        /// Hinweistext des Anlage-Dialogs: "Diese Vorlage benötigt ein Konto,
        /// das mit dem Feld ... / den Feldern ..., ... und ... verknüpft ist."
        /// </summary>
        public static string HinweisText(IReadOnlyList<string> feldnamen)
        {
            var namen = feldnamen ?? (IReadOnlyList<string>)Array.Empty<string>();
            var sb = new StringBuilder("Diese Vorlage benötigt ein Konto, das mit ");
            if (namen.Count <= 1)
            {
                sb.Append("dem Feld \"").Append(namen.Count == 1 ? namen[0] : "?").Append("\"");
            }
            else
            {
                sb.Append("den Feldern ");
                for (int i = 0; i < namen.Count; i++)
                {
                    if (i > 0) sb.Append(i == namen.Count - 1 ? " und " : ", ");
                    sb.Append("\"").Append(namen[i]).Append("\"");
                }
            }
            sb.Append(" verknüpft ist. Den Kontonamen bitte ggf. geeignet kürzen, "
                    + "sodass der Zweck des Kontos aber noch eindeutig zu erkennen ist.");
            return sb.ToString();
        }

        /// <summary>
        /// Legt das Konto am Ende seiner Gruppe an (erster freier Slot) und
        /// persistiert es samt Feldzuweisungen über die EUKonten-Mechanik
        /// (Cache + ini synchron, Kontenlisten neu aufgebaut).
        /// </summary>
        /// <returns>false + Fehlertext, wenn alle 100 Slots belegt sind, der
        /// Name leer/vergeben ist oder kein Bedarf vorliegt.</returns>
        public static bool ErzeugeKonto(string name, bool istEinnahme,
            IReadOnlyList<KontoFeldBedarf> bedarf, out string fehler)
        {
            fehler = "";
            name = (name ?? "").Trim();
            if (name.Length == 0)
            {
                fehler = "Der Kontoname darf nicht leer sein.";
                return false;
            }
            if (bedarf == null || bedarf.Count == 0)
            {
                fehler = "Keine Feld-Verknüpfung angegeben.";
                return false;
            }

            EUKonten.Lade();
            var einnahmen = EUKonten.EinnahmenKonten.ToList();
            var ausgaben  = EUKonten.AusgabenKonten.ToList();
            var gruppe    = istEinnahme ? einnahmen : ausgaben;

            if (gruppe.Count >= 100)
            {
                fehler = "Alle 100 Plätze für "
                    + (istEinnahme ? "Einnahmenkonten" : "Ausgabenkonten")
                    + " sind bereits belegt.";
                return false;
            }
            if (gruppe.Any(k => string.Equals(k.Name, name, StringComparison.OrdinalIgnoreCase)))
            {
                fehler = $"Ein Konto \"{name}\" existiert in dieser Gruppe bereits.";
                return false;
            }

            var zuweisungen = new Dictionary<string, string>();
            foreach (var b in bedarf)
                zuweisungen[b.Formular] = b.FeldId;

            gruppe.Add(new EUKonto(name, istEinnahme, gruppe.Count, "", zuweisungen));
            EUKonten.SpeichereReihenfolge(einnahmen, ausgaben);
            return true;
        }
    }
}
