// EinstellungenExports.h — Bridge-API zum globalen Einstellungs-Cache
// =====================================================================
//
// Loest die alte Pro-Dokument-Verwaltung (CEinstellungen1..5 in
// CEasyCashView) ab. Cache lebt im ECTEngine-Modul (Einstellungen.cs),
// die Bridge ist zustaendig fuer:
//   - Erst-Befuellung beim App-Start / Mandantenwechsel
//   - Sofortiges Schreiben in die easyct.ini bei jedem Speichere-Aufruf
//
// Schluessel-Format: wie Plugin-API (Kurzform "fname" oder explizit
// "[Sektion]Key"). Aufloesung Kurzform → Sektion via IniSektion()
// aus ectifacemisc.cpp.

#pragma once

#ifndef ECTBRIDGE_API
  #ifdef ECTBRIDGE_EXPORTS
    #define ECTBRIDGE_API __declspec(dllexport)
  #else
    #define ECTBRIDGE_API __declspec(dllimport)
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Liest die aktuelle easyct.ini (Pfad via GetIniFileName()) komplett
/// in den Engine-Cache ein. Wird automatisch bei SetIniFileName-Wechseln
/// aufgerufen — sollte aber auch beim App-Start einmal explizit aufgerufen
/// werden, falls noch kein Mandantenwechsel stattgefunden hat.
/// </summary>
ECTBRIDGE_API void ECT_LadeEinstellungen();

/// <summary>
/// Liefert den gecachten Wert zum Schluessel.
/// Rueckgabe zeigt auf einen statischen Buffer (nicht thread-safe,
/// muss vor dem naechsten ECT_HoleEinstellung-Aufruf konsumiert werden).
/// Leerstring bei Cache-Miss.
/// </summary>
ECTBRIDGE_API LPCSTR ECT_HoleEinstellung(LPCSTR key);

/// <summary>
/// Speichert den Wert im Cache und schreibt ihn sofort in die ini-Datei.
/// </summary>
ECTBRIDGE_API void ECT_SpeichereEinstellung(LPCSTR key, LPCSTR value);

/// <summary>
/// Convenience-Wrapper.
/// </summary>
ECTBRIDGE_API int  ECT_HoleEinstellungInt (LPCSTR key, int  defaultValue);
ECTBRIDGE_API BOOL ECT_HoleEinstellungBool(LPCSTR key, BOOL defaultValue);
ECTBRIDGE_API void ECT_SpeichereEinstellungInt (LPCSTR key, int  value);
ECTBRIDGE_API void ECT_SpeichereEinstellungBool(LPCSTR key, BOOL value);

#ifdef __cplusplus
}
#endif
