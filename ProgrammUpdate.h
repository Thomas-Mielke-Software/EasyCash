// ProgrammUpdate.h: Update-Prüfung für das Hauptprogramm (v4 Preview)
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2026  Thomas Mielke
//
// Parallel zum Plugin-Manager (update.txt, Auswahl einzelner Module) prüft
// EC&T bei jedem Start in einem eigenen Thread, ob auf dem Server eine neuere
// Programmversion liegt:
//
//   https://easyct.de/update_v4.txt   -- "v4.0.28:<letzte Änderung>" (Windows-1252,
//                                        erzeugt von bump-version.ps1)
//   https://easyct.de/ECTSetup4.exe   -- das Setup selbst
//
// Heruntergeladen wird nach C:\ProgramData\EasyCash&Tax\download (wie beim
// Plugin-Manager). Ablauf:
//   - Liegt dort schon ein Setup mit der neuen Version: Ja/Nein/Abbrechen-Frage
//     (Ja = EC&T beenden + Setup starten, Nein = Setup-Datei löschen).
//   - Sonst: Meldung in der CaptionBar mit "Installieren" / "Überspringen".
//     Überspringen merkt sich die Version in [Allgemein]SkipVersion.
//   - Ein Setup, das nicht neuer als die installierte Version ist, wird
//     stillschweigend gelöscht (Rest einer erfolgreichen Installation).
//
// Das Setup startet erst in CEasyCashApp::ExitInstance, also nachdem das
// Hauptfenster wirklich geschlossen wurde; bricht der Anwender das Beenden ab
// (ungespeicherte Daten -> Abbrechen), läuft kein Setup.

#pragma once

#define PROGRAMMUPDATE_INFO_URL		"https://easyct.de/update_v4.txt"
#define PROGRAMMUPDATE_SETUP_URL	"https://easyct.de/ECTSetup4.exe"
#define PROGRAMMUPDATE_SETUP_DATEI	"ECTSetup4.exe"

// Nachrichten an das Hauptfenster (CMainFrame leitet sie an die Funktionen unten weiter)
#define WM_PROGRAMMUPDATE_GEPRUEFT			(WM_APP + 0x4E1)	// lParam: Prüf-Ergebnis des Start-Threads
#define WM_PROGRAMMUPDATE_HERUNTERGELADEN	(WM_APP + 0x4E2)	// lParam: Ergebnis des Download-Threads
#define WM_PROGRAMMUPDATE_BEENDEN			(WM_APP + 0x4E3)	// EC&T beenden und danach Setup starten

// Startet den Prüf-Thread; das Ergebnis kommt als WM_PROGRAMMUPDATE_GEPRUEFT an hwndHauptfenster.
void ProgrammUpdatePruefungStarten(HWND hwndHauptfenster);

// Handler für die Nachrichten oben (laufen im UI-Thread).
LRESULT ProgrammUpdateGeprueft(LPARAM lParam);
LRESULT ProgrammUpdateHeruntergeladen(LPARAM lParam);
LRESULT ProgrammUpdateBeenden();

// Knöpfe in der CaptionBar
void ProgrammUpdateInstallieren();
void ProgrammUpdateUeberspringen();

// Aus ExitInstance: startet das Setup, falls das Beenden für ein Update ausgelöst wurde.
void ProgrammUpdateSetupNachProgrammende();
