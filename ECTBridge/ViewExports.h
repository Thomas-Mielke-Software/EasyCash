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

// Forward-Deklaration für die Pointer-basierten Funktionen.
// Die View-Dateien inkludieren EasyCashDoc.h ohnehin und haben
// dann die volle Definition; die forward decl hier hält den
// Header leichtgewichtig und funktioniert auch in reinen C-APIs.
class CBuchung;

#ifdef __cplusplus
extern "C" {
#endif

// ──────────────────────────────────────────────
// Index-basierte API (Grundfunktionen)
// ──────────────────────────────────────────────

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

// ──────────────────────────────────────────────
// Pointer-basierte API (komfortabel für bestehenden View-Code)
// ──────────────────────────────────────────────
//
// Die folgenden Funktionen nehmen einen nativen CBuchung*-Pointer
// entgegen, wie er in der existierenden MFC-View-Logik (z.B.
// ppPosBuchungsliste, *ppb) schon vorhanden ist. Die Bridge
// identifiziert intern die korrespondierende managed Buchung,
// führt die Operation aus und synchronisiert native ↔ managed.
//
// WICHTIG: Nach jedem dieser Aufrufe muss der View sein Display
// neu zeichnen, weil sich native Linked-List-Positionen verschoben
// haben können (Sort, Neueinfügen, Löschen).

/// <summary>
/// Zeigt den Bearbeiten-Dialog für eine bestehende Buchung,
/// die per CBuchung*-Pointer identifiziert ist.
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungBearbeitenDialogFuerPointer(
    void* pDocBridge, CBuchung* pNativeBuchung, HWND hWndOwner);

/// <summary>
/// Zeigt den Buchungsdialog zum Kopieren einer bestehenden Buchung.
/// Der Dialog startet mit den Feldwerten der Vorlage; beim OK-Klick
/// wird eine NEUE Buchung angelegt (nicht die Vorlage überschrieben).
///
/// Parameter:
///   bNeueBelegnummer - TRUE: Belegnummer wird auf die nächste freie
///                      Nummer gesetzt. FALSE: Belegnummer wird aus
///                      der Vorlage übernommen (inkl. evtl. Suffix).
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungKopierenDialog(
    void* pDocBridge, CBuchung* pNativeBuchung,
    BOOL bNeueBelegnummer, HWND hWndOwner);

/// <summary>
/// Löscht eine Buchung, die per CBuchung*-Pointer identifiziert ist.
/// Nach der Löschung sind alle zuvor gehaltenen CBuchung*-Pointer
/// ungültig (auch der übergebene!), weil SyncManagedToNative die
/// nativen Linked Lists komplett neu aufbaut.
///
/// Rückgabe:
///   TRUE wenn gelöscht, FALSE wenn der Pointer nicht gefunden wurde.
/// </summary>
ECTBRIDGE_API BOOL ECT_LoescheBuchungPerPointer(
    void* pDocBridge, CBuchung* pNativeBuchung);

#ifdef __cplusplus
}
#endif
