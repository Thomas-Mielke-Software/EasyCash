// AfaAbgangShared.h -- gemeinsame AfA-Abgang-Logik fuer Journal + Dialog.
//
// Definition in JournalExports.cpp, Aufrufer sind JournalExports.cpp
// (Kontextmenue "AfA-Abgang buchen") und ViewExports.cpp (Button
// "Abgang buchen" im Buchungsdialog).
//
// WICHTIG: Nur in /clr-Uebersetzungseinheiten einbinden, und zwar NACH
// EasyCashDocBridge.h -- dann sind sowohl CEasyCashDocBridge als auch die
// ECTEngine-Typen (Projekt-Referenz / forced #using) bereits bekannt.
// ECTEngine::Buchung wird hier bewusst NICHT vorwaertsdeklariert: eine
// ref-class-Forward-Deklaration wuerde einen Phantom-Typ in der eigenen
// Assembly erzeugen statt den importierten ECTEngine-Typ zu treffen.

#pragma once

class CEasyCashDocBridge;

// Fuehrt den AfA-Abgang fuer eine bereits aufgeloeste, aktuelle Buchung aus:
// Sicherheitsabfrage (Ja/Nein), Restbuchwert-Konto ermitteln, Engine-Mutation
// (BuchungsDocument::AfaAbgang), Sort, SyncManagedToNative, SetModifiedFlag
// und Journal-Refresh. 'aktuelle' muss die aktuelle managed Instanz aus
// eng->Buchungen sein (nicht eine stale Referenz).
//
// Rueckgabe: true wenn der Abgang durchgefuehrt wurde; false bei Abbruch
// (Nein-Antwort), ungeeigneter Buchung (AfaJahre <= 1) oder Fehler.
bool ECTBridge_FuehreAfaAbgang(CEasyCashDocBridge* bridge, ECTEngine::Buchung^ aktuelle);
