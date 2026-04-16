// Enums.cs — Aufzählungstypen für ECTEngine
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

namespace ECTEngine
{
    /// <summary>
    /// Art der Buchung. Entspricht der Zugehörigkeit zur
    /// Einnahmen- bzw. Ausgaben-Linked-List im Original.
    /// </summary>
    public enum Buchungsart
    {
        Einnahme = 0,   // BUCHUNGSTYP_EINNAHMEN
        Ausgabe = 1     // BUCHUNGSTYP_AUSGABEN
    }

    /// <summary>
    /// Abschreibungsgenauigkeit.
    /// Direkt aus EasyCashDoc.h: #define GANZJAHRES_AFA 0 .. 3
    /// </summary>
    public enum AfaGenauigkeit
    {
        Ganzjahr = 0,                   // GANZJAHRES_AFA
        Halbjahr = 1,                   // HALBJAHRES_AFA
        Monatsgenau = 2,                // MONATSGENAUE_AFA
        EntsprechendEinstellungen = 3   // ENTSPRECHEND_EINSTELLUNGEN_AFA
    }

    /// <summary>
    /// Intervall für Dauerbuchungen.
    /// Direkt aus EasyCashDoc.h: #define INTERVALL_MONAT 1 .. 5
    /// </summary>
    public enum Intervall
    {
        Monatlich = 1,      // INTERVALL_MONAT
        Quartalsweise = 2,  // INTERVALL_QUARTAL
        Halbjaehrlich = 3,  // INTERVALL_HALBJAHR
        Jaehrlich = 4,      // INTERVALL_JAHR
        ZweiMonatlich = 5   // INTERVALL_2MONATE
    }
}
