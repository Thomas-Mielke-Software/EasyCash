using System.Globalization;

namespace ECTEngine
{
    /// <summary>
    /// Typisierte Fassade über den <see cref="Einstellungen"/>-Cache für die
    /// "einfachen" globalen Einstellungs-Seiten (Allgemein, Unternehmer,
    /// Finanzamt, MwSt-Sätze). Zentralisiert das Wissen über die
    /// KANONISCHEN Cache-Schlüssel, damit Lesen und Schreiben sauber
    /// round-trippen.
    ///
    /// WICHTIG -- Schlüsselformat (siehe KuerzelFuerCache in
    /// ECTBridge/EinstellungenExports.cpp):
    ///   - Präfix-Sektionen (Finanzamt) -> Kurzform "f" + IniKey  ("fname").
    ///   - alle anderen Sektionen       -> Bracket-Form "[Sektion]IniKey".
    /// ECT_LadeEinstellungen legt die Werte genau in dieser Form im Cache ab;
    /// <see cref="Einstellungen.Hole"/> macht einen ROHEN Lookup -- deshalb
    /// MUSS von C# aus exakt dieselbe Form verwendet werden (ein nackter
    /// Schlüssel wie "vat1" würde ins Leere greifen).
    ///
    /// Setter schreiben über <see cref="Einstellungen.Speichere"/>, was das
    /// WertGeaendert-Event auslöst; die Bridge schreibt dann synchron in die
    /// easyct.ini (Live-Save). Da der Cache prozessweit/statisch ist, sehen
    /// alle offenen MDI-Dokumente die Änderung sofort.
    /// </summary>
    public static class GlobaleEinstellungen
    {
        // ---------------------------------------------------------------------
        // Finanzamt  ([Finanzamt], Präfix 'f')
        // ---------------------------------------------------------------------
        public static string FinanzamtName        { get => Get("fname");            set => Set("fname", value); }
        public static string FinanzamtName2        { get => Get("fname2");           set => Set("fname2", value); }
        public static string FinanzamtStrasse      { get => Get("fstrasse");         set => Set("fstrasse", value); }
        public static string FinanzamtPlz          { get => Get("fplz");             set => Set("fplz", value); }
        public static string FinanzamtOrt          { get => Get("fort");             set => Set("fort", value); }
        public static string FinanzamtSteuernummer { get => Get("fsteuernummer");    set => Set("fsteuernummer", value); }
        public static string FinanzamtWirtschaftsId { get => Get("fwirtschaftsidnr"); set => Set("fwirtschaftsidnr", value); }

        // ---------------------------------------------------------------------
        // Unternehmer  ([Persoenliche_Daten])
        // ---------------------------------------------------------------------
        private const string PD = "[Persoenliche_Daten]";
        public static string UnternehmerName        { get => Get(PD + "name");             set => Set(PD + "name", value); }
        public static string UnternehmerVorname     { get => Get(PD + "vorname");          set => Set(PD + "vorname", value); }
        public static string Unternehmensart1       { get => Get(PD + "unternehmensart1"); set => Set(PD + "unternehmensart1", value); }
        public static string Unternehmensart2       { get => Get(PD + "unternehmensart2"); set => Set(PD + "unternehmensart2", value); }
        public static string UnternehmerStrasse     { get => Get(PD + "strasse");          set => Set(PD + "strasse", value); }
        public static string UnternehmerPlz         { get => Get(PD + "plz");              set => Set(PD + "plz", value); }
        public static string UnternehmerOrt         { get => Get(PD + "ort");              set => Set(PD + "ort", value); }
        public static int    Land                   { get => GetInt(PD + "land");          set => SetInt(PD + "land", value); }

        // MwSt-Sätze (konfigurierbare Steuersätze) -- als Strings, weil das
        // Format "7,5" o.ä. erlaubt ist (deutsches Dezimalkomma).
        public static string Vat1 { get => Get(PD + "vat1"); set => Set(PD + "vat1", value); }
        public static string Vat2 { get => Get(PD + "vat2"); set => Set(PD + "vat2", value); }
        public static string Vat3 { get => Get(PD + "vat3"); set => Set(PD + "vat3", value); }
        public static string Vat4 { get => Get(PD + "vat4"); set => Set(PD + "vat4", value); }

        // ---------------------------------------------------------------------
        // Allgemein  ([Allgemein])
        // ---------------------------------------------------------------------
        private const string AG = "[Allgemein]";
        public static int  MonatlicheVoranmeldung { get => GetInt(AG + "monatliche_voranmeldung", 1); set => SetInt(AG + "monatliche_voranmeldung", value); }
        public static bool TaeglichBuchen         { get => GetBool(AG + "taeglich_buchen");          set => SetBool(AG + "taeglich_buchen", value); }
        public static bool BuchungsdatumBelassen  { get => GetBool(AG + "BuchungsdatumBelassen");    set => SetBool(AG + "BuchungsdatumBelassen", value); }
        // Weiterbuchen: Betrieb/Bestandskonto der letzten Buchung beibehalten
        // (Default an).
        public static bool BetriebBelassen        { get => GetBool(AG + "betrieb_belassen", true);        set => SetBool(AG + "betrieb_belassen", value); }
        public static bool BestandskontoBelassen  { get => GetBool(AG + "bestandskonto_belassen", true);  set => SetBool(AG + "bestandskonto_belassen", value); }
        public static bool ErzeugeBelegnrEinnahmen { get => GetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerEinnahmen"); set => SetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerEinnahmen", value); }
        public static bool ErzeugeBelegnrAusgaben  { get => GetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerAusgaben");  set => SetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerAusgaben", value); }
        public static bool ErzeugeBelegnrBank      { get => GetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerBank");      set => SetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerBank", value); }
        public static bool ErzeugeBelegnrKasse     { get => GetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerKasse");     set => SetBool(AG + "ErzeugeLaufendeBuchungsnummernFuerKasse", value); }

        // Buchen-Dialog-Steuerung: Sichtbarkeit von Jahres- und MwSt-Feld.
        // Default MwSt-Feld = an (1), Jahresfeld = aus (0) -- wie LoadProfile.
        public static bool JahresfeldAktiviert { get => GetBool(AG + "JahresfeldAktiviert", false); set => SetBool(AG + "JahresfeldAktiviert", value); }
        public static bool MwstFeldAktiviert   { get => GetBool(AG + "MwstFeldAktiviert",  true);   set => SetBool(AG + "MwstFeldAktiviert", value); }

        public static int  AbschreibungGenauigkeit { get => GetInt(AG + "AbschreibungGenauigkeit", 2); set => SetInt(AG + "AbschreibungGenauigkeit", value); }

        // ---------------------------------------------------------------------
        // Druck  ([Druck])  -- Schriften
        // ---------------------------------------------------------------------
        private const string DR = "[Druck]";
        public static string Bildschirmschrift       { get => Get(DR + "Bildschirmschrift");          set => Set(DR + "Bildschirmschrift", value); }
        public static int    Bildschirmschriftgroesse { get => GetInt(DR + "Bildschirmschriftgroesse"); set => SetInt(DR + "Bildschirmschriftgroesse", value); }
        public static int    BildschirmschriftFett   { get => GetInt(DR + "BildschirmschriftFett", 400); set => SetInt(DR + "BildschirmschriftFett", value); }
        public static bool   BildschirmschriftKursiv  { get => GetBool(DR + "BildschirmschriftKursiv"); set => SetBool(DR + "BildschirmschriftKursiv", value); }
        public static string Druckerschrift          { get => Get(DR + "Druckerschrift");             set => Set(DR + "Druckerschrift", value); }
        public static int    Druckerschriftgroesse   { get => GetInt(DR + "Druckerschriftgroesse");    set => SetInt(DR + "Druckerschriftgroesse", value); }
        public static int    DruckerschriftFett      { get => GetInt(DR + "DruckerschriftFett", 400);  set => SetInt(DR + "DruckerschriftFett", value); }
        public static bool   DruckerschriftKursiv     { get => GetBool(DR + "DruckerschriftKursiv");   set => SetBool(DR + "DruckerschriftKursiv", value); }

        // ---------------------------------------------------------------------
        // Low-level-Zugriff
        // ---------------------------------------------------------------------
        private static string Get(string key) => Einstellungen.Hole(key);
        private static void   Set(string key, string value) => Einstellungen.Speichere(key, value ?? "");

        private static int  GetInt(string key, int def = 0)   => Einstellungen.HoleInt(key, def);
        private static void SetInt(string key, int value)     => Einstellungen.Speichere(key, value.ToString(CultureInfo.InvariantCulture));

        private static bool GetBool(string key, bool def = false) => Einstellungen.HoleBool(key, def);
        private static void SetBool(string key, bool value)       => Einstellungen.Speichere(key, value ? "1" : "0");
    }
}
