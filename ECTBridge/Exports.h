// Exports.h — Native C-Exportschnittstelle der ECTBridge
//
// Diese Datei wird von EasyCash.exe inkludiert (#include "ECTBridge\Exports.h").
// Sie deklariert rein native C-Funktionen mit __declspec(dllimport/dllexport),
// die intern auf die managed ECTEngine zugreifen.
//
// Phase 1: Nur eine Testfunktion, um die Integration zu verifizieren.
// Später kommen hier die Ersatzfunktionen für ECTIFace-Exports hinzu.

#pragma once

#ifdef ECTBRIDGE_EXPORTS
#define ECTBRIDGE_API __declspec(dllexport)
#else
#define ECTBRIDGE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ──────────────────────────────────────────────
// Phase 1: Integrationstest
// ──────────────────────────────────────────────

/// Gibt einen Versionsstring der Bridge zurück.
/// Der String ist statisch allokiert und muss NICHT freigegeben werden.
/// Ruft intern die CLR auf und instanziiert ein ECTEngine-Objekt,
/// um zu beweisen, dass die gesamte Kette funktioniert.
ECTBRIDGE_API const char* ECT_BridgeVersion(void);

/// Führt einen schnellen Selbsttest durch:
/// - Erzeugt ein Betrag-Objekt in der C#-Engine
/// - Berechnet Netto aus Brutto
/// - Vergleicht mit dem erwarteten Ergebnis
/// Rückgabe: 0 = OK, != 0 = Fehlercode
ECTBRIDGE_API int ECT_BridgeSelfTest(void);

#ifdef __cplusplus
}
#endif
