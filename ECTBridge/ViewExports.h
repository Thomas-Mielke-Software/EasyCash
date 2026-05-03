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

// ──────────────────────────────────────────────
// Listen-Initialisierung (Betrieb + Bestandskonto)
// ──────────────────────────────────────────────
//
// EasyCash hält die Betriebs-/Bestandskonten-Daten in den Structs
// CBetrieb (m_Betriebe) und CBestandskonto (m_Bestandskonten) in der View.
//
// Diese Funktion übergibt eine Schnappschuss-Kopie an den ViewHost,
// der ihn beim nächsten Dialog-/Journal-Aufruf nutzt. Wenn die Listen
// leer sind, werden die zugehörigen UI-Elemente ausgeblendet.

/// <summary>
/// Setzt die Betriebs- und Bestandskonten-Listen für nachfolgende
/// Buchungsdialog- und Journal-Aufrufe.
///
/// Icons werden als int-Indizes übergeben (kein atoi mehr nötig).
/// pBestandskontenSalden enthält Vorjahres-Anfangssalden in Cent.
/// Die Bridge kopiert die Inhalte intern, der Aufrufer kann die
/// Arrays nach dem Aufruf verwerfen.
///
/// Übergibt man nCount = 0, werden die Listen ausgeblendet.
/// </summary>
ECTBRIDGE_API void ECT_SetzeBetriebeUndBestandskonten(
    LPCSTR* pBetriebeNamen, const int* pBetriebeIcons, int nBetriebeCount,
    LPCSTR* pBestandskontenNamen, const int* pBestandskontenIcons,
    const int* pBestandskontenSalden, int nBestandskontenCount);

// ──────────────────────────────────────────────
// Buchungsjournal
// ──────────────────────────────────────────────

/// <summary>
/// Öffnet das Buchungsjournal-Fenster (modeless) als WPF-View.
/// Replikat von DrawToDC_Datum/DrawToDC_Konten aus easycashview.cpp.
///
/// Der Aufrufer sollte vorher per ECT_SetzeBetriebeUndBestandskonten
/// die Listen für die Icons gesetzt haben.
///
/// Rückgabe:
///   TRUE wenn das Fenster geöffnet werden konnte.
/// </summary>
ECTBRIDGE_API BOOL ECT_ZeigeJournal(void* pDocBridge, HWND hWndOwner);

/// <summary>
/// Aktualisiert das offene Journal-Fenster — z.B. nach
/// Buchungsänderungen. Wenn kein Fenster offen ist, passiert nichts.
///
/// Parameter:
///   nAnzeigeModus      - 0 = nach Datum, 1 = nach Konten
///   pszKontenFilter    - "" oder "<alle Konten>" für kein Filter
///   nMonatsFilter      - 0=alle, 1-12=Monat, 13-16=Quartal
///   pszBetriebFilter   - "" für kein Filter
///   pszBestandskontoFilter - "" für kein Filter
///   dSchriftgroesse    - Zoom (z.B. 13.0 = Standard)
/// </summary>
ECTBRIDGE_API void ECT_AktualisiereJournal(
    int nAnzeigeModus,
    LPCSTR pszKontenFilter,
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    LPCSTR pszBestandskontoFilter,
    double dSchriftgroesse);

#ifdef __cplusplus
}
#endif
