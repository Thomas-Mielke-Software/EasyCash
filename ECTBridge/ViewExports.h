// ViewExports.h — Native C-Exportschnittstelle für ECTViews
//
// Wird von EasyCash.exe inkludiert, um WPF-Dialoge aus nativem
// MFC-Code aufzurufen. Die Implementierung in ViewExports.cpp
// (C++/CLI) ruft intern ECTViews::ViewHost auf.
//
// Einbindung in EasyCash:
//   #include "ViewExports.h"
//   // in einem Menü-Handler oder View:
//   ECT_ShowBuchungDialog(pDoc, TRUE, AfxGetMainWnd()->GetSafeHwnd());

#pragma once

#ifdef ECTBRIDGE_EXPORTS
#define ECTBRIDGE_API __declspec(dllexport)
#else
#define ECTBRIDGE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Zeigt den WPF-Buchungseingabe-Dialog.
///
/// Parameter:
///   pDocBridge - Pointer auf CEasyCashDocBridge (wird intern zu
///                BuchungsDocument^ aufgelöst)
///   bAusgaben  - TRUE für Ausgaben, FALSE für Einnahmen
///   hWndOwner  - HWND des MFC-Elternfensters (für modale Darstellung)
///
/// Rückgabe:
///   TRUE wenn eine Buchung erstellt wurde (sie wurde bereits in das
///   Dokument eingefügt und die Engine synchronisiert).
///   FALSE wenn der Dialog abgebrochen wurde.
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungDialog(
    void* pDocBridge, BOOL bAusgaben, HWND hWndOwner);

/// <summary>
/// Zeigt den WPF-Dialog zur Bearbeitung einer bestehenden Buchung.
///
/// Parameter:
///   pDocBridge  - Pointer auf CEasyCashDocBridge
///   nBuchungIdx - Index der Buchung in der Engine-Buchungsliste
///   hWndOwner   - HWND des MFC-Elternfensters
///
/// Rückgabe:
///   TRUE wenn die Buchung geändert wurde.
///   FALSE wenn abgebrochen.
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungBearbeitenDialog(
    void* pDocBridge, int nBuchungIdx, HWND hWndOwner);

#ifdef __cplusplus
}
#endif
