// EinstellungenExports.h -- Bridge-API zum globalen Einstellungs-Cache
// =====================================================================
//
// Loest die alte Pro-Dokument-Verwaltung (CEinstellungen1..5 in
// CEasyCashView) ab. Cache lebt im ECTEngine-Modul (Einstellungen.cs),
// die Bridge ist zustaendig fuer:
//   - Erst-Befuellung beim App-Start / Mandantenwechsel
//   - Sofortiges Schreiben in die easyct.ini bei jedem Speichere-Aufruf
//
// Schluessel-Format: wie Plugin-API (Kurzform "fname" oder explizit
// "[Sektion]Key"). Aufloesung Kurzform => Sektion via IniSektion()
// aus ectifacemisc.cpp.

#pragma once

#ifndef ECTBRIDGE_API
  #ifdef ECTBRIDGE_EXPORTS
    #define ECTBRIDGE_API __declspec(dllexport)
  #else
    #define ECTBRIDGE_API __declspec(dllimport)
  #endif
#endif

// ---- Preset-Struktur (Buchungsposten aus [Buchungsposten]-Sektion) ----------

struct ECT_Preset
{
    char  text[512];    // 00Text
    BOOL  ausgabe;      // 00Ausg: 0 = Einnahmen, 1 = Ausgaben
    int   mwst;         // 00MWSt: Festkomma x1000 (19000 = 19 %)
    int   afaj;         // 00AfAJ
    char  konto[512];   // 00Rech
};

#ifdef __cplusplus
extern "C" {
#endif

/// Liest die aktuelle easyct.ini komplett in den Engine-Cache ein.
/// Wird automatisch bei SetIniFileName-Wechseln aufgerufen.
ECTBRIDGE_API void ECT_LadeEinstellungen();

/// Liefert den gecachten Wert zum Schluessel.
/// Rueckgabe zeigt auf einen rotierenden statischen Buffer (8 Slots).
/// Leerstring bei Cache-Miss.
ECTBRIDGE_API LPCSTR ECT_HoleEinstellung(LPCSTR key);

/// Speichert den Wert im Cache und schreibt ihn sofort in die ini-Datei.
ECTBRIDGE_API void ECT_SpeichereEinstellung(LPCSTR key, LPCSTR value);

/// Convenience-Wrapper fuer int/bool.
ECTBRIDGE_API int  ECT_HoleEinstellungInt (LPCSTR key, int  defaultValue);
ECTBRIDGE_API BOOL ECT_HoleEinstellungBool(LPCSTR key, BOOL defaultValue);
ECTBRIDGE_API void ECT_SpeichereEinstellungInt (LPCSTR key, int  value);
ECTBRIDGE_API void ECT_SpeichereEinstellungBool(LPCSTR key, BOOL value);

// ---- Listen-Zugriffe --------------------------------------------------------

/// Liefert den index-ten Einnahmen-Kontonamen (aus [EinnahmenRechnungsposten]).
/// Leerstring wenn index ausserhalb der gespeicherten Liste.
ECTBRIDGE_API LPCSTR ECT_HoleEinnahmenKonto(int index);

/// Liefert den index-ten Ausgaben-Kontonamen (aus [AusgabenRechnungsposten]).
/// Leerstring wenn index ausserhalb der gespeicherten Liste.
ECTBRIDGE_API LPCSTR ECT_HoleAusgabenKonto(int index);

/// Befuellt *outPreset mit dem index-ten Buchungsposten-Preset (0-basiert).
/// Gibt FALSE zurueck wenn index ungueltig oder outPreset == NULL,
/// ansonsten TRUE (auch wenn das Preset leer ist).
ECTBRIDGE_API BOOL ECT_HolePreset(int index, ECT_Preset* outPreset);

/// Anzahl der gueltigen EinnahmenKonten / AusgabenKonten im Cache.
ECTBRIDGE_API int ECT_AnzahlEinnahmenKonten();
ECTBRIDGE_API int ECT_AnzahlAusgabenKonten();

#ifdef __cplusplus
}
#endif