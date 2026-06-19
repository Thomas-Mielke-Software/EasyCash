// EinstellungenExports.h -- Bridge-API zum globalen Einstellungs-Cache
// =====================================================================

#pragma once

#ifndef ECTBRIDGE_API
  #ifdef ECTBRIDGE_EXPORTS
    #define ECTBRIDGE_API __declspec(dllexport)
  #else
    #define ECTBRIDGE_API __declspec(dllimport)
  #endif
#endif

// -----------------------------------------------------------------------------
// Datenstrukturen
// -----------------------------------------------------------------------------

struct ECT_Preset
{
    char text[512];     // 00Text
    BOOL ausgabe;       // 00Ausg: FALSE=Einnahmen, TRUE=Ausgaben
    int  mwst;          // 00MWSt: Festkomma x1000 (19000 = 19 %)
    int  afaj;          // 00AfAJ
    char konto[512];    // 00Rech
    char notiz[1024];   // 00Notiz: Freitext, im Buchen-Dialog als Balloon
};

struct ECT_Betrieb
{
    char name[512];             // Betrieb??Name
    char unternehmensart[1024]; // Betrieb??Unternehmensart (kann Tabs enthalten)
    int  icon;                  // Betrieb??Icon
};

struct ECT_BestandskontoInfo
{
    char name[512]; // Bestandskonto??Name
    int  icon;      // Bestandskonto??Icon
    // Saldo wird jahresweise über ECT_HoleBestandskontoSaldo abgefragt
};

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Cache-Lifecycle
// -----------------------------------------------------------------------------

/// Liest die aktuelle easyct.ini komplett in den Engine-Cache ein.
/// Baut dabei auch alle Listen (EinnahmenKonten, Betriebe usw.) auf.
ECTBRIDGE_API void ECT_LadeEinstellungen();

// -----------------------------------------------------------------------------
// Einfache Key-Value-Einstellungen
// -----------------------------------------------------------------------------

ECTBRIDGE_API LPCSTR ECT_HoleEinstellung(LPCSTR key);
ECTBRIDGE_API void   ECT_SpeichereEinstellung(LPCSTR key, LPCSTR value);
ECTBRIDGE_API int    ECT_HoleEinstellungInt (LPCSTR key, int  defaultValue);
ECTBRIDGE_API BOOL   ECT_HoleEinstellungBool(LPCSTR key, BOOL defaultValue);
ECTBRIDGE_API void   ECT_SpeichereEinstellungInt (LPCSTR key, int  value);
ECTBRIDGE_API void   ECT_SpeichereEinstellungBool(LPCSTR key, BOOL value);

// -----------------------------------------------------------------------------
// EinnahmenKonten / AusgabenKonten
// -----------------------------------------------------------------------------

ECTBRIDGE_API int   ECT_AnzahlEinnahmenKonten();
ECTBRIDGE_API LPCSTR ECT_HoleEinnahmenKonto(int index);
ECTBRIDGE_API void  ECT_SpeichereEinnahmenKonto(int index, LPCSTR name);

ECTBRIDGE_API int   ECT_AnzahlAusgabenKonten();
ECTBRIDGE_API LPCSTR ECT_HoleAusgabenKonto(int index);
ECTBRIDGE_API void  ECT_SpeichereAusgabenKonto(int index, LPCSTR name);

// -----------------------------------------------------------------------------
// Presets (Buchungsposten)
// -----------------------------------------------------------------------------

/// index: 0-basiert, immer 100 Eintraege im Cache (Lücken = IstLeer).
/// Gibt FALSE nur bei ungueltigem index oder outPreset==NULL zurück.
ECTBRIDGE_API BOOL ECT_HolePreset(int index, ECT_Preset* outPreset);
ECTBRIDGE_API void ECT_SpeicherePreset(int index, const ECT_Preset* preset);

// -----------------------------------------------------------------------------
// Betriebe
// -----------------------------------------------------------------------------

ECTBRIDGE_API int  ECT_AnzahlBetriebe();
ECTBRIDGE_API BOOL ECT_HoleBetrieb(int index, ECT_Betrieb* outBetrieb);
ECTBRIDGE_API void ECT_SpeichereBetrieb(int index, const ECT_Betrieb* betrieb);

// -----------------------------------------------------------------------------
// Bestandskonten
// -----------------------------------------------------------------------------

ECTBRIDGE_API int  ECT_AnzahlBestandskonten();
ECTBRIDGE_API BOOL ECT_HoleBestandskonto(int index, ECT_BestandskontoInfo* outInfo);
ECTBRIDGE_API void ECT_SpeichereBestandskonto(int index, const ECT_BestandskontoInfo* info);

/// Liefert den Anfangssaldo in Cent für ein bestimmtes Bestandskonto und Jahr.
/// Gibt FALSE zurück wenn kein Eintrag für dieses Jahr existiert.
ECTBRIDGE_API BOOL ECT_HoleBestandskontoSaldo(int index, int jahr, int* centOut);
ECTBRIDGE_API void ECT_SpeichereBestandskontoSaldo(int index, int jahr, int cent);

// -----------------------------------------------------------------------------
// Statusleiste (managed Pendant zu CMainFrame::SetStatus)
// -----------------------------------------------------------------------------

// Der Host (EasyCash.exe) registriert eine native Funktion, die den Text an
// CMainFrame::SetStatus weiterreicht -- die Bridge kennt CMainFrame nicht.
typedef void (*ECT_StatusCallback)(const char* text);
ECTBRIDGE_API void ECT_SetzeStatusCallback(ECT_StatusCallback cb);

#ifdef __cplusplus
}
#endif