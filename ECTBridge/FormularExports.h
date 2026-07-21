// FormularExports.h - Native Exports fuer die eingebettete WPF-Formular-
// Ansicht (.ecf-Formulare: EUeR, USt-Erklaerung, UVA/U30, AT-Formulare).
//
// Muster wie BerichtExports.h: die Ansicht wird als Kind-HWND eines
// nativen Parents (Splitter des ChildFrame) eingebettet und ueberlagert
// als Vollflaeche Journal + Navigation (die der Aufrufer per SW_HIDE
// versteckt, siehe ZeigeFormularWpf in easycashview.cpp).
//
// Das gewaehlte Formular kommt als .ecf-Dateipfad (m_csaFormulare[i]) plus
// Betriebsfilter (m_csaFormularfilter[i], leer = kein Betrieb) herein; der
// Zeitraum steckt im Formular selbst (voranmeldungszeitraum-Attribut).
// Der Zoom ist der native Prozent-Zoomfaktor (m_zoomfaktor) -- die Ansicht
// skaliert per ScaleTransform, damit Feldwerte und Formularscan
// deckungsgleich bleiben.

#pragma once

#include "JournalExports.h"   // ECTBRIDGE_API + Callback-Typedefs

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Bettet die Formular-Ansicht als Kind des angegebenen Parent-HWND ein.
/// Rueckgabe: HWND des WPF-Fensters (mit SetWindowPos positionierbar),
/// NULL bei Fehler.
/// </summary>
ECTBRIDGE_API HWND ECT_FormularEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge,
    LPCSTR pszEcfPfad,
    LPCSTR pszBetriebFilter,
    int nZoomProzent);

/// <summary>Loest eine eingebettete Formular-Ansicht ab.</summary>
ECTBRIDGE_API void ECT_FormularAbloesen(HWND hwndFormular);

/// <summary>Loest ALLE eingebetteten Formular-Ansichten ab.</summary>
ECTBRIDGE_API void ECT_FormularAlleAbloesen();

/// <summary>
/// Rechnet die aktive(n) Formular-Ansicht(en) neu (Buchungs- oder
/// Einstellungs-Aenderung). Synchronisiert vorher die native Buchungsliste
/// in die Engine (pDocBridge = CEasyCashDoc der aktiven View).
/// </summary>
ECTBRIDGE_API void ECT_FormularAktualisiere(void* pDocBridge);

/// <summary>
/// Wechselt das angezeigte Formular bei offener Ansicht (Ribbon-Menue),
/// ohne Re-Embedding.
/// </summary>
ECTBRIDGE_API void ECT_FormularWechsle(
    void* pDocBridge,
    LPCSTR pszEcfPfad,
    LPCSTR pszBetriebFilter);

/// <summary>Setzt den Zoomfaktor in Prozent (wie m_zoomfaktor).</summary>
ECTBRIDGE_API void ECT_FormularSetzeZoom(int nZoomProzent);

/// <summary>
/// Druckt das aktive Formular (WYSIWYG, FixedPages mit Formularscan).
/// bVorschau=TRUE oeffnet die Seitenansicht. Rueckgabe FALSE, wenn keine
/// Formular-Ansicht aktiv ist.
/// </summary>
ECTBRIDGE_API BOOL ECT_FormularDrucken(BOOL bVorschau);

/// <summary>
/// Registriert die Callbacks fuer Zoom-Tasten und Strg+P aus der
/// Formular-Ansicht (gleiche Typedefs wie beim Journal). NULL
/// deregistriert den jeweiligen Callback.
/// </summary>
ECTBRIDGE_API void ECT_FormularRegistriereCallbacks(
    ECT_JournalZoomAenderungCallback pfnZoom,
    ECT_JournalDruckAnforderungCallback pfnDruck);

/// <summary>
/// Golden-Master-Vergleich nativ/managed (nur fuer den Debug-Build
/// gedacht): laesst den managed FormularRechner auf demselben Dokument
/// laufen und vergleicht Feld fuer Feld mit den nativen Werten
/// (pszNativWerte = m_csaFormularfeldwerte, mit '\n' verbunden).
/// Differenzen gehen mit Feld-Id und beiden Werten an OutputDebugString.
/// Rueckgabe: Anzahl der Differenzen (0 = identisch), -1 bei Fehler.
/// </summary>
ECTBRIDGE_API int ECT_FormularVergleichsdump(
    void* pDocBridge,
    LPCSTR pszEcfPfad,
    LPCSTR pszBetriebFilter,
    LPCSTR pszNativWerte);

#ifdef __cplusplus
}
#endif
