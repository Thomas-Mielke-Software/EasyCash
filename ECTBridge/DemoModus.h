// DemoModus.h - Demo-Modus: Programmstart mit Beispieldaten (Kommandozeile /demo)
//
// Der Demo-Modus laeuft in einem Sandkasten, damit echte Nutzerdaten unter
// keinen Umstaenden beruehrt werden:
//
//   1. Eigenes Datenverzeichnis: ein frisch angelegter Ordner unter
//      %TEMP%\EasyCT-Demo\<PID>-<Zeit>. Dort entsteht die easyct.ini aus der
//      eingebetteten Ressource (ECTEngine DemoModus.SchreibeIni, FileMode.
//      CreateNew -- eine vorhandene Datei wuerde nie ueberschrieben). Jeder
//      Schreibzugriff ueber GetIniFileName() landet damit im Wegwerf-Ordner,
//      der beim Beenden geloescht wird.
//   2. Eigener Registry-Zweig: InitInstance haengt " Demo" an den
//      Profilnamen, BEVOR irgendetwas gelesen wird (Mandanten, MRU,
//      Datenverzeichnis, Fensterlagen). Der Zweig wird bei jedem Demo-Start
//      und beim Beenden geloescht.
//   3. Gesperrte Funktionen: Speichern (jeder Weg bis hinunter zu
//      Serialize), Wiederherstellungsdatei, Neu/Oeffnen/MRU/Drag&Drop/DDE,
//      Jahreswechsel, Mandanten und Datenverzeichnis -- mit der Rueckfrage
//      "Im Demo-Modus ist diese Funktion nicht erlaubt. Demo-Modus
//      beenden?" (ECT_DemoFunktionGesperrt).
//
// Das Buchungsdokument entsteht ohne "Neu"-Dialog direkt aus den
// Beispieldaten (CEasyCashDocBridge::OnNewDocument --> ECT_DemoDokumentFuellen),
// verschoben auf das Demojahr = aeltester gemeinsamer Stand der
// installierten EUeR-, Voranmeldungs- und Erklaerungsformulare.

#pragma once

#include "JournalExports.h"   // ECTBRIDGE_API

// Kommandos fuer den Neustart (Ribbon Optionen --> Programm, Startoptionen,
// Rueckfrage "Demo-Modus beenden?"). Behandelt in CEasyCashApp.
#define ECT_ID_DEMO_UMSCHALTEN        40950   // Ribbon: Demo starten bzw. beenden (mit Rueckfrage)
#define ECT_ID_DEMO_STARTEN_DIREKT    40951   // Startoptionen: ohne weitere Rueckfrage in den Demo-Modus
#define ECT_ID_DEMO_BEENDEN_DIREKT    40952   // nach "Demo-Modus beenden? Ja": normal neu starten

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>TRUE, wenn das Programm im Demo-Modus laeuft.</summary>
ECTBRIDGE_API BOOL ECT_IstDemoModus();

/// <summary>TRUE, wenn die Kommandozeile den Schalter /demo (oder -demo,
/// Gross-/Kleinschreibung egal) enthaelt.</summary>
ECTBRIDGE_API BOOL ECT_DemoParameterVorhanden();

/// <summary>
/// Schaltet den Demo-Modus ein -- als ALLERERSTES in InitInstance, sobald
/// /demo erkannt ist. Vor allem anderen, damit alles, was beim Aufbau des
/// Hauptfensters nachfragt (z.B. die Beschriftung "Demo beenden" im Ribbon),
/// und alle Sperren den Modus von Anfang an kennen.
/// </summary>
ECTBRIDGE_API void ECT_DemoModusEinschalten();

/// <summary>
/// Bereitet den Demo-Modus vor: legt das Temp-Datenverzeichnis an,
/// schreibt die easyct.ini fuer das Demojahr hinein und erlaubt genau EIN
/// neues Dokument (das Demo-Dokument). Liefert das Verzeichnis (ohne
/// abschliessenden Backslash). FALSE bei Fehler (Meldung in szFehler).
/// </summary>
ECTBRIDGE_API BOOL ECT_DemoVorbereiten(char* szVerzeichnis, int nGroesse,
                                       char* szFehler, int nFehlerGroesse);

/// <summary>Demojahr (nach ECT_DemoVorbereiten), sonst 0.</summary>
ECTBRIDGE_API int ECT_DemoJahr();

/// <summary>
/// Fragt "Im Demo-Modus ist diese Funktion nicht erlaubt. Demo-Modus
/// beenden?" und stoesst bei Ja den normalen Neustart an
/// (ECT_ID_DEMO_BEENDEN_DIREKT an das Hauptfenster). Liefert immer TRUE,
/// damit Aufrufer schreiben koennen: if (ECT_IstDemoModus() &&
/// ECT_DemoFunktionGesperrt()) return ...;
/// </summary>
ECTBRIDGE_API BOOL ECT_DemoFunktionGesperrt();

/// <summary>Loescht das Temp-Datenverzeichnis des Demo-Modus (nur das
/// selbst angelegte). Aufruf aus ExitInstance.</summary>
ECTBRIDGE_API void ECT_DemoAufraeumen();

#ifdef __cplusplus
}

class CEasyCashDoc;

/// <summary>
/// Interne Bridge-Funktionen (nicht exportiert): erlaubt die Dokumentvorlage
/// das naechste neue Dokument? (verbraucht die einmalige Erlaubnis) bzw.
/// fuellt das frische Dokument mit den Beispieldaten.
/// </summary>
BOOL ECT_DemoDokumentErlaubt();
BOOL ECT_DemoDokumentFuellen(CEasyCashDoc* pDoc);
#endif
