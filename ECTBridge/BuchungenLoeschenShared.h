// BuchungenLoeschenShared.h -- gemeinsame Lösch-Logik für das
// Journal-Kontextmenü (beide Journal-Hosts: JournalEmbed in
// JournalExports.cpp und JournalHost in ViewExports.cpp).
//
// Definition in JournalExports.cpp.
//
// WICHTIG: Nur in /clr-Übersetzungseinheiten einbinden, und zwar NACH
// EasyCashDocBridge.h -- dann sind sowohl CEasyCashDocBridge als auch die
// ECTEngine-Typen bereits bekannt (vgl. AfaAbgangShared.h).

#pragma once

class CEasyCashDocBridge;

// Löscht die übergebenen Buchungen aus der Engine -- mit Rücksicht auf
// Buchungsgruppen: Gehören selektierte Buchungen zu Gruppen, deren übrige
// Mitglieder NICHT mitselektiert sind, wird per Ja/Nein/Abbrechen gefragt,
// ob die ganzen Gruppen gelöscht werden sollen (Kaskadenlöschen). Sonst
// normale Bestätigungsabfrage; ist die Selektion exakt eine komplette
// Gruppe, wird sie auch so benannt.
//
// Stale Buchung^-Referenzen sind erlaubt (Auflösung per Uuid). Nach dem
// Entfernen: ein SyncManagedToNative + ein SetModifiedFlag.
//
// Rückgabe: true wenn gelöscht wurde (der Aufrufer muss danach das Journal
// aktualisieren); false bei Abbruch oder leerer Liste.
bool ECTBridge_LoescheBuchungenMitGruppenAbfrage(
    CEasyCashDocBridge* bridge,
    System::Collections::Generic::IList<ECTEngine::Buchung^>^ buchungen);
