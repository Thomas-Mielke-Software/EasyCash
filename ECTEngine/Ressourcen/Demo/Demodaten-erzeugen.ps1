# Demodaten-erzeugen.ps1 -- erzeugt die Beispieldaten des Demo-Modus neu
#
# Schreibt zwei eingebettete Ressourcen von ECTEngine:
#   * easyct.ini      -- ersetzt alles ab der Marke ";@VORLAGEN-AUS-DER-BIBLIOTHEK@"
#                        durch die Buchungsvorlagen 10-17 (aus der
#                        Vorlagen-Bibliothek bzw. hier definiert)
#   * Demodaten.xml   -- das Buchungsdokument als Sammeldatei (BuchungsdateiXml)
#
# Die Buchungsgruppen rechnet der echte BuchungsgruppenRechner aus -- sie
# sehen also genau so aus, als wären sie im Buchen-Dialog entstanden, und
# lassen sich im Demo-Modus mit ihrer Vorlage bearbeiten. Ids sind aus
# einem Zähler abgeleitet: ein erneuter Lauf ändert die Dateien nur dort,
# wo sich die Daten wirklich ändern.
#
# Aufruf (Windows PowerShell 5.1, ECTEngine vorher bauen):
#   powershell -ExecutionPolicy Bypass -File ECTEngine\Ressourcen\Demo\Demodaten-erzeugen.ps1
# Danach ECTEngine neu bauen (die Dateien sind eingebettet) und die Tests
# laufen lassen (DemoModusTests).

$ErrorActionPreference = 'Stop'
$hier = Split-Path -Parent $MyInvocation.MyCommand.Path
$dll  = Join-Path $hier '..\..\bin\Debug\net48\ECTEngine.dll' | Resolve-Path
# Kopie laden, damit die DLL beim nächsten Build nicht gesperrt ist
$kopie = Join-Path $env:TEMP ("ECTEngine-Demodaten-" + [guid]::NewGuid().ToString('N') + '.dll')
Copy-Item $dll $kopie
[void][System.Reflection.Assembly]::LoadFrom($kopie)

[System.Threading.Thread]::CurrentThread.CurrentCulture   = 'de-DE'
[System.Threading.Thread]::CurrentThread.CurrentUICulture = 'de-DE'

$quelle = @'
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using ECTEngine;

public static class DemodatenBauer
{
    public const int Jahr = 2025;   // Jahr, für das die Sammeldatei geschrieben wird

    // ------------------------------------------------------------------
    // Vorlagen 10-17
    // ------------------------------------------------------------------

    public static List<string> ErzeugeVorlagenZeilen()
    {
        var bibliothek = VorlagenBibliothek.Lade().SelectMany(a => a.AlleEintraege())
            .Select(e => PresetXml.Importiere(e.VorlagenXml)).ToList();
        Func<string, Preset> lib = name =>
        {
            var p = bibliothek.FirstOrDefault(x => x.Text == name);
            if (p == null) throw new InvalidOperationException("Bibliothek: " + name + " fehlt");
            return p;
        };

        var telefon = new Preset("Telefon mit Privatanteil", true, 19000, 1, "Telefon und Internet",
            "Gemischt genutzter Telefon- und Internetanschluss: der private Anteil (Vorbelegung 20 %) "
            + "läuft ohne Vorsteuer über das neutrale Konto /Privatentnahmen, der Rest ist "
            + "Betriebsausgabe mit anteiliger Vorsteuer.",
            false, 0,
            new[] { new PresetZeile("A", "/Privatentnahmen", "0", "$brutto*$privat/100",
                                    "$beschreibung (Privatanteil $privat %)") },
            "$rest",
            new VorlagenErweiterung(
                new[] { new PresetFeld("privat", "Privater Anteil", "prozent", "betrag", "20", true, true, "") },
                null, "", "", "", "", ""));

        var vorlagen = new List<KeyValuePair<int, Preset>>
        {
            new KeyValuePair<int, Preset>(10, lib("Reverse Charge EU-Eingangsrechnung")),
            new KeyValuePair<int, Preset>(11, lib("Bewirtungskosten")),
            new KeyValuePair<int, Preset>(12, lib("Umbuchung zwischen Bestandskonten")),
            new KeyValuePair<int, Preset>(13, lib("Ausgabe auf zwei Betriebe")),
            new KeyValuePair<int, Preset>(14, telefon),
            new KeyValuePair<int, Preset>(15, lib("Rechnung mit zwei Steuersätzen")),
            new KeyValuePair<int, Preset>(16, lib("Verpflegungsmehraufwand")),
            new KeyValuePair<int, Preset>(17, lib("Reverse Charge EU-Ausgangsrechnung")),
        };

        var zeilen = new List<string>();
        Action<string, string> merke = (k, v) =>
        {
            const string p = "[Buchungsposten]";
            if (k.StartsWith(p)) zeilen.Add(k.Substring(p.Length) + "=" + (v ?? ""));
        };
        Einstellungen.WertGeaendert += merke;
        try
        {
            foreach (var kv in vorlagen)
            {
                var p = kv.Value;
                // feste Kennung, damit die Buchungen ihre Vorlage wiederfinden
                var mitId = new Preset(p.Text, p.Ausgabe, p.Mwst, p.AfaJ, p.Konto, p.Notiz,
                    p.Degressiv, p.AfaSatz, p.Zeilen, p.BasisBetragFormel, p.Erweiterung,
                    DemoId("vorlage-" + kv.Key).ToString("N"));
                Einstellungen.SpeicherePreset(kv.Key, mitId);
            }
        }
        finally { Einstellungen.WertGeaendert -= merke; }
        return zeilen;
    }

    // ------------------------------------------------------------------
    // Buchungsdokument
    // ------------------------------------------------------------------

    private static int _zaehler;
    private static BuchungsDocument _doc;

    public static string ErzeugeSammeldatei()
    {
        _zaehler = 0;
        _doc = new BuchungsDocument
        {
            Jahr = Jahr, Waehrung = "EUR", GlobaleAfaGenauigkeit = AfaGenauigkeit.Monatsgenau,
            BackupNachfrageIntervallTage = 7, BackupNachfrageTermin = new DateTime(Jahr, 12, 31)
        };

        const string Giro = "Girokonto", Kasse = "Kasse", PayPal = "PayPal";
        const string Ill = "Illustration", Laden = "Druckladen";

        // ---- Einnahmen: Illustration ----
        E( 1, 14, "Kinderbuch \"Der kleine Leuchtturm\", 1. Rate -- Verlag Wellenreiter", 2140.00m, 7, "Honorare Illustration", "R-01", Ill, Giro);
        E( 2, 20, "Editorial-Illustration Stadtmagazin, Februar-Ausgabe", 595.00m, 19, "Honorare Illustration", "R-02", Ill, Giro);
        E( 3, 12, "Kinderbuch \"Der kleine Leuchtturm\", 2. Rate -- Verlag Wellenreiter", 2140.00m, 7, "Honorare Illustration", "R-03", Ill, Giro);
        E( 4,  3, "Logo und Geschäftsausstattung Bäckerei Krume", 1428.00m, 19, "Honorare Illustration", "R-04", Ill, Giro);
        Gruppe(17, 4, 28, "Kampagnen-Illustrationen -- Agentur Feinstrich, Wien", 1800.00m, 0, null, "R-05", Ill, Giro, null);
        E( 5, 22, "Plakat Stadtfest Tintenfeld", 892.50m, 19, "Honorare Illustration", "R-06", Ill, Giro);
        E( 6, 16, "Buchcover -- Verlag Bergsicht, Luzern", 950.00m, 0, "Leistungen in Drittländer", "R-07", Ill, Giro);
        E( 7,  9, "Kinderbuch \"Der kleine Leuchtturm\", 3. Rate -- Verlag Wellenreiter", 2140.00m, 7, "Honorare Illustration", "R-08", Ill, Giro);
        E( 8, 18, "Illustrationen Museumsführer -- Stadtmuseum Tintenfeld", 1712.00m, 7, "Honorare Illustration", "R-09", Ill, Giro);
        E( 9, 25, "Editorial-Illustration Stadtmagazin, Oktober-Ausgabe", 595.00m, 19, "Honorare Illustration", "R-10", Ill, Giro);
        Gruppe(17, 10, 14, "Folgeauftrag Kampagne -- Agentur Feinstrich, Wien", 1200.00m, 0, null, "R-11", Ill, Giro, null);
        E(11,  6, "Weihnachtskarten-Motiv Gärtnerei Grünspan", 714.00m, 19, "Honorare Illustration", "R-12", Ill, Giro);
        E(12, 10, "Kinderbuch \"Der kleine Leuchtturm\", Schlussrate -- Verlag Wellenreiter", 1070.00m, 7, "Honorare Illustration", "R-13", Ill, Giro);
        E(12, 18, "Editorial-Illustration Stadtmagazin, Januar-Ausgabe", 595.00m, 19, "Honorare Illustration", "R-14", Ill, Giro);

        // ---- Einnahmen: Druckladen (Online-Shop über PayPal, Märkte bar) ----
        var shop = new[] { 214.60m, 168.90m, 241.30m, 312.00m, 286.40m, 198.70m,
                           176.50m, 205.90m, 264.80m, 358.20m, 512.60m, 689.30m };
        var monate = new[] { "Januar", "Februar", "März", "April", "Mai", "Juni", "Juli",
                             "August", "September", "Oktober", "November", "Dezember" };
        for (int m = 1; m <= 12; m++)
            E(m, Math.Min(28, DateTime.DaysInMonth(Jahr, m)), "Shop-Verkäufe " + monate[m - 1] + " (Kunstdrucke und Karten)",
              shop[m - 1], 19, "Verkauf Kunstdrucke und Karten", "S-" + m.ToString("00"), Laden, PayPal);
        E( 5, 10, "Kunsthandwerkermarkt Tintenfeld, Barverkauf", 386.00m, 19, "Verkauf Kunstdrucke und Karten", "K-01", Laden, Kasse);
        E(12,  6, "Weihnachtsmarkt Tintenfeld, 1. Wochenende", 742.50m, 19, "Verkauf Kunstdrucke und Karten", "K-02", Laden, Kasse);
        E(12, 13, "Weihnachtsmarkt Tintenfeld, 2. Wochenende", 598.00m, 19, "Verkauf Kunstdrucke und Karten", "K-03", Laden, Kasse);
        E( 1,  2, "Wechselgeld für die Kasse", 100.00m, 0, "/Privateinlagen", "", Laden, Kasse);

        // ---- Dauerbuchungen ----
        Dauer("Atelier-Miete $M/$J", 450.00m, 0, Intervall.Monatlich, 1, "Raumkosten Atelier", Ill, Giro);
        Dauer("Kontoführung Girokonto $M/$J", 7.90m, 0, Intervall.Monatlich, 30, "Kontoführung und sonstige Kosten", Ill, Giro);
        Dauer("Cloud-Speicher und Website $M/$J", 11.90m, 19, Intervall.Monatlich, 5, "Software und EDV", Ill, Giro);
        Dauer("Berufshaftpflicht $J", 186.40m, 0, Intervall.Jaehrlich, 15, "Versicherungen und Beiträge", Ill, Giro);

        // ---- Telefon und Internet mit Privatanteil (Buchungsgruppe) ----
        for (int m = 1; m <= 12; m++)
            Gruppe(14, m, 3, "Telefon und Internet " + m.ToString("00") + "/" + Jahr, 44.95m, 19, null, "",
                   Ill, Giro, new Dictionary<string, string> { { "privat", "20" } });

        // ---- Ausgaben Illustration ----
        Gruppe(10, 1, 8, "Adobe Creative Cloud, Jahresabo (Adobe Ireland)", 791.40m, 0, null, "E-101", Ill, Giro, null);
        A( 1, 22, "Aquarellfarben und Pinsel -- Künstlerbedarf Farbtopf", 142.80m, 19, "Arbeitsmittel und Fachliteratur", "E-102", Ill, Giro);
        Gruppe(15, 2, 7, "Buchhandlung Seitensprung: Fachbücher und Skizzenbücher", 84.70m, 19, "Arbeitsmittel und Fachliteratur", "E-103", Ill, Kasse,
               new Dictionary<string, string> { { "betrag2", "59,90" }, { "satz2", "7" } });
        A( 2, 18, "Flyer und Visitenkarten -- Druckhaus Punkt", 89.25m, 19, "Werbung", "E-104", Ill, Giro);
        Afa(2, 26, "Zeichentisch, höhenverstellbar -- Atelierbedarf Lindgrün", 1249.50m, 19, 13, 1, 105000, "E-105", Ill, Giro);
        A( 3, 11, "Online-Kurs Kinderbuch-Illustration", 249.00m, 19, "Fortbildung", "E-106", Ill, Giro);
        Gruppe(13, 3, 24, "Nebenkostenabrechnung Atelier Vorjahr", 312.60m, 0, "Raumkosten Atelier", "E-107", Ill, Giro,
               new Dictionary<string, string> { { "betrieb2", Laden }, { "anteil2", "30" } });
        A( 4, 15, "Steuerberatung Jahresabschluss " + (Jahr - 1) + " -- Kanzlei Zahlenwerk", 595.00m, 19, "Steuerberatung", "E-108", Ill, Giro);
        A( 5,  5, "Grafiktablett -- Elektro Pixel", 699.00m, 19, "Geringwertige Wirtschaftsgüter", "E-109", Ill, Giro);
        A( 6,  3, "Papier und Tusche -- Künstlerbedarf Farbtopf", 96.40m, 19, "Arbeitsmittel und Fachliteratur", "E-110", Ill, Kasse);
        Gruppe(10, 6, 12, "Schriftlizenz -- Letterhuis Type Foundry, Utrecht", 180.00m, 0, null, "E-111", Ill, Giro, null);
        A( 7, 21, "Lektorat Begleittext Ausstellung -- Wortwerk", 357.00m, 19, "Fremdleistungen", "E-112", Ill, Giro);
        Gruppe(11, 8, 21, "Bewirtung", 92.40m, 19, null, "E-113", Ill, Giro,
               new Dictionary<string, string> { { "anlass", "Projektbesprechung Museumsführer" },
                                                { "personen", "Dr. Elke Pergament (Stadtmuseum), Franziska Federkiel" },
                                                { "trinkgeld", "7,40" } });
        A( 9,  9, "Skizzenbücher und Fineliner -- Künstlerbedarf Farbtopf", 118.90m, 19, "Arbeitsmittel und Fachliteratur", "E-114", Ill, Kasse);
        A(10, 15, "Bahnfahrt zur Buchmesse Frankfurt", 129.90m, 7, "Reisekosten", "E-115", Ill, Giro);
        A(10, 15, "Fachbesucher-Ticket Buchmesse", 58.00m, 19, "Reisekosten", "E-116", Ill, Giro);
        A(10, 17, "Hotel Buchmesse, 2 Nächte", 238.00m, 7, "Reisekosten", "E-117", Ill, Giro);
        Gruppe(16, 10, 17, "Buchmesse Frankfurt", 0m, 0, null, "", Ill, "",
               new Dictionary<string, string> { { "tage8", "2" }, { "tage24", "1" } });

        // ---- Ausgaben Druckladen ----
        A( 3,  3, "Giclée-Drucke -- Druckerei Stempel & Söhne", 478.00m, 19, "Material und Druckkosten", "E-201", Laden, Giro);
        A( 9,  1, "Giclée-Drucke und Postkarten -- Druckerei Stempel & Söhne", 612.00m, 19, "Material und Druckkosten", "E-202", Laden, Giro);
        A(11,  3, "Weihnachtskarten und Kalender -- Druckerei Stempel & Söhne", 890.00m, 19, "Material und Druckkosten", "E-203", Laden, Giro);
        A( 4, 22, "Versandkartons und Seidenpapier -- Packwerk", 74.90m, 19, "Material und Druckkosten", "E-204", Laden, PayPal);
        A(11, 10, "Standgebühr Weihnachtsmarkt", 180.00m, 19, "Kontoführung und sonstige Kosten", "E-205", Laden, Giro);
        for (int q = 1; q <= 4; q++)
        {
            A(q * 3, 28, "Porto Shop-Versand " + q + ". Quartal", new[] { 42.60m, 51.35m, 47.90m, 96.15m }[q - 1], 0,
              "Porto und Versand", "", Laden, Giro);
            A(q * 3, 30, "PayPal-Gebühren " + q + ". Quartal", new[] { 21.84m, 25.33m, 20.62m, 52.41m }[q - 1], 0,
              "Kontoführung und sonstige Kosten", "", Laden, PayPal);
        }
        // Lastenrad: im Vorjahr angeschafft, im Demojahr zweites AfA-Jahr
        Afa(6, 14, "Lastenrad für Auslieferungen und Märkte", 3570.00m, 19, 7, 2, 275000, "E-17/" + (Jahr - 1), Laden, Giro);

        // ---- Geldverkehr ----
        foreach (var t in new[] { new { M = 4, T = 2, B = 520.00m }, new { M = 7, T = 1, B = 680.00m },
                                  new { M = 10, T = 1, B = 540.00m }, new { M = 12, T = 30, B = 1350.00m } })
            Gruppe(12, t.M, t.T, "Auszahlung PayPal-Guthaben", t.B, 0, null, "", Laden, PayPal,
                   new Dictionary<string, string> { { "zielkonto", Giro } });
        Gruppe(12, 12, 20, "Bareinzahlung Markterlöse", 1200.00m, 0, null, "", Laden, Kasse,
               new Dictionary<string, string> { { "zielkonto", Giro } });
        A(12, 22, "Privatentnahme", 1500.00m, 0, "/Privatentnahmen", "", Ill, Giro);

        // ---- Umsatzsteuer ----
        A(1, 10, "USt-Vorauszahlung 4. Quartal " + (Jahr - 1), 386.20m, 0, "Umsatzsteuer-Vorauszahlungen", "", Ill, Giro);
        for (int q = 1; q <= 4; q++)
        {
            decimal zahllast = Zahllast(q);
            UstVorauszahlungen.Setze(_doc, UstVorauszahlungen.QuartalZeitraum(q),
                zahllast.ToString("0.00", System.Globalization.CultureInfo.GetCultureInfo("de-DE")));
            if (q == 4) continue;   // wird erst im Januar des Folgejahres gezahlt
            if (zahllast >= 0)
                A(q * 3 + 1, 10, "USt-Vorauszahlung " + q + ". Quartal " + Jahr, zahllast, 0, "Umsatzsteuer-Vorauszahlungen", "", Ill, Giro);
            else
                E(q * 3 + 1, 10, "USt-Erstattung " + q + ". Quartal " + Jahr, -zahllast, 0, "Erstattete Umsatzsteuer", "", Ill, Giro);
        }

        _doc.Buchungszaehler = _doc.Buchungen.Count;
        _doc.Sort();
        return BuchungsdateiXml.SchreibeSammeldatei(_doc);
    }

    // ------------------------------------------------------------------

    private static decimal Zahllast(int quartal)
    {
        decimal ust = 0m, vst = 0m;
        foreach (var b in _doc.Buchungen)
        {
            if ((b.Datum.Month - 1) / 3 + 1 != quartal) continue;
            if (b.Konto == "Umsatzsteuer-Vorauszahlungen" || b.Konto == "Erstattete Umsatzsteuer") continue;
            if (b.Art == Buchungsart.Einnahme) ust += b.MwstBetrag;
            else if (b.AfaNr == 1) vst += b.MwstBetrag;   // fortgeführte AfA: Vorsteuer schon im Vorjahr
        }
        return ust - vst;
    }

    private static Buchung Neu(Buchungsart art, int monat, int tag, string text, decimal brutto, decimal mwst,
                               string konto, string beleg, string betrieb, string bestandskonto)
    {
        return new Buchung
        {
            Uuid = NaechsteId(), Art = art, Datum = new DateTime(Jahr, monat, tag),
            Beschreibung = text, BruttoBetrag = new Betrag(brutto, mwst), Konto = konto ?? "",
            Belegnummer = beleg ?? "", Betrieb = betrieb ?? "", Bestandskonto = bestandskonto ?? ""
        };
    }

    private static void E(int m, int t, string text, decimal brutto, decimal mwst, string konto,
                          string beleg, string betrieb, string bk)
    {
        _doc.Buchungen.Add(Neu(Buchungsart.Einnahme, m, t, text, brutto, mwst, konto, beleg, betrieb, bk));
    }

    private static void A(int m, int t, string text, decimal brutto, decimal mwst, string konto,
                          string beleg, string betrieb, string bk)
    {
        _doc.Buchungen.Add(Neu(Buchungsart.Ausgabe, m, t, text, brutto, mwst, konto, beleg, betrieb, bk));
    }

    private static void Afa(int m, int t, string text, decimal brutto, decimal mwst, int jahre, int nr,
                            int restwertCent, string beleg, string betrieb, string bk)
    {
        var b = Neu(Buchungsart.Ausgabe, m, t, text, brutto, mwst, "AfA auf bewegliche Wirtschaftsgüter",
                    beleg, betrieb, bk);
        b.AfaJahre = jahre; b.AfaNr = nr; b.AfaRestwertCent = restwertCent;
        _doc.Buchungen.Add(b);
    }

    private static void Dauer(string text, decimal brutto, decimal mwst, Intervall intervall, int tag,
                              string konto, string betrieb, string bk)
    {
        var d = new Dauerbuchung
        {
            Art = Buchungsart.Ausgabe, Beschreibung = text, BruttoBetrag = new Betrag(brutto, mwst),
            Intervall = intervall, Buchungstag = tag, Konto = konto, Betrieb = betrieb, Bestandskonto = bk,
            VonDatum = new DateTime(Jahr - 2, 1, 1), BisDatum = new DateTime(2037, 12, 31),
            AusgefuehrtBis = new DateTime(Jahr - 1, 12, 31)
        };
        var stichtag = new DateTime(Jahr, 12, 31);
        foreach (var termin in d.FaelligeTermine(stichtag).ToList())
        {
            var b = d.ErzeugeBuchung(termin);
            b.Uuid = NaechsteId();
            _doc.Buchungen.Add(b);
        }
        d.AusgefuehrtBis = stichtag;
        _doc.Dauerbuchungen.Add(d);
    }

    /// <summary>Bucht wie der Dialog mit einer Vorlage: Eingabe in die
    /// Basis, Zusatz-Felder, Rechner, fertige Buchungen.</summary>
    private static void Gruppe(int slot, int m, int t, string text, decimal brutto, decimal mwst, string konto,
                               string beleg, string betrieb, string bk, Dictionary<string, string> werte)
    {
        var p = Einstellungen.Presets[slot];
        if (p.IstLeer) throw new InvalidOperationException("Vorlage " + slot + " fehlt");

        var felder = p.Felder.Select(f => VorlagenFelder.Werte(f,
            werte != null && werte.ContainsKey(f.Name) ? werte[f.Name] : VorlagenFelder.StandardWert(f))).ToList();
        foreach (var f in felder)
            if (f.Fehler.Length > 0) throw new InvalidOperationException(p.Text + "/" + f.Feld.Name + ": " + f.Fehler);

        var eingabe = BuchungsgruppenRechner.BerechneEingabeBetrag(p, felder);
        if (eingabe != null)
        {
            if (!eingabe.Ok) throw new InvalidOperationException(p.Text + ": " + eingabe.Fehler);
            brutto = eingabe.Wert;
        }

        string basisKonto = konto;
        if (string.IsNullOrEmpty(basisKonto))
        {
            var a = KontoFeldSelektor.LoeseAuf(p.Konto);
            basisKonto = a.IstSpezifikation ? a.Konto : p.Konto;
        }
        if (string.IsNullOrEmpty(basisKonto))
            throw new InvalidOperationException(p.Text + ": Basis-Konto nicht auflösbar");

        var basis = Neu(p.Ausgabe ? Buchungsart.Ausgabe : Buchungsart.Einnahme, m, t, text, brutto,
                        mwst, basisKonto, beleg, betrieb, bk);
        var berechnung = BuchungsgruppenRechner.Berechne(p, basis, felder);
        if (berechnung.HatFehler)
        {
            var z = berechnung.Zeilen.FirstOrDefault(x => x.Fehler.Length > 0);
            throw new InvalidOperationException(p.Text + ": " + (z != null ? z.Fehler : berechnung.BasisFehler));
        }
        var liste = BuchungsgruppenRechner.BaueBuchungen(p, slot, basis, berechnung,
            NaechsteId().ToString(), felder);
        foreach (var b in liste)
        {
            if (b != basis) b.Uuid = NaechsteId();
            _doc.Buchungen.Add(b);
        }
    }

    private static Guid NaechsteId() { return DemoId("buchung-" + (++_zaehler)); }

    public static Guid DemoId(string schluessel)
    {
        using (var md5 = MD5.Create())
            return new Guid(md5.ComputeHash(Encoding.UTF8.GetBytes("EasyCash&Tax-Demo/" + schluessel)));
    }
}
'@

Add-Type -TypeDefinition $quelle -Language CSharp -ReferencedAssemblies $kopie, 'System.Core', 'System.Xml', 'System.Xml.Linq'

# ---- 1) easyct.ini: Vorlagen 10-17 aus der Bibliothek ----
$iniPfad = Join-Path $hier 'easyct.ini'
$marke   = ';@VORLAGEN-AUS-DER-BIBLIOTHEK@'
$ini     = [System.IO.File]::ReadAllText($iniPfad, [System.Text.Encoding]::UTF8)
$pos     = $ini.IndexOf($marke)
if ($pos -lt 0) { throw "Marke $marke fehlt in easyct.ini" }
$basis   = $ini.Substring(0, $pos + $marke.Length)

$jahrText = [string][DemodatenBauer]::Jahr
$vorjahr  = [string]([DemodatenBauer]::Jahr - 1)
function Lade-Cache([string]$text) {
    $t = $text.Replace('{VORJAHR}', $vorjahr).Replace('{JAHR}', $jahrText)
    [ECTEngine.Einstellungen]::LadeAusBridge([ECTEngine.DemoModus]::IniZuCache($t))
}

Lade-Cache $basis
$zeilen = [DemodatenBauer]::ErzeugeVorlagenZeilen()
$neuIni = $basis + "`n" + (($zeilen | ForEach-Object { $_ }) -join "`n") + "`n"
[System.IO.File]::WriteAllText($iniPfad, $neuIni.Replace("`r`n", "`n"), (New-Object System.Text.UTF8Encoding($false)))
Write-Host ("easyct.ini: {0} Vorlagen-Zeilen geschrieben" -f $zeilen.Count)

# ---- 2) Demodaten.xml ----
Lade-Cache $neuIni
$xml = [DemodatenBauer]::ErzeugeSammeldatei()
[System.IO.File]::WriteAllText((Join-Path $hier 'Demodaten.xml'), $xml, (New-Object System.Text.UTF8Encoding($false)))
$anzahl = ([regex]::Matches($xml, '<Buchung ')).Count
Write-Host ("Demodaten.xml: {0} Buchungen geschrieben" -f $anzahl)
