// BerichtExports.h - Native Exports fuer die eingebettete Formlos-Ansicht
// (Freestyle-EUER, formlose USt-Erklaerung, Kontenplan).
//
// Muster wie JournalExports.h: die Ansicht wird als Kind-HWND eines
// nativen Parents (Splitter des ChildFrame) eingebettet und ueberlagert
// als Vollflaeche Journal + Navigation (die der Aufrufer per SW_HIDE
// versteckt, siehe ZeigeBerichtWpf in easycashview.cpp).
//
// Berichtstypen (muss mit ECTViews.Berichte.BerichtTyp uebereinstimmen):
//   0 = Freestyle-EUER
//   1 = formlose USt-Erklaerung
//   2 = Kontenplan (einfach)
//   3 = Kontenplan mit Formular-Feldverknuepfungen
//
// Filter (WYSIWYG): nMonatsFilter = 0 alle, 1-12 Monat, 13-16 Quartal;
// pszBetriebFilter = "" fuer alle Betriebe. Beide kommen aus den
// Ribbon-Display-Filtern des Aufrufers.

#pragma once

#include "JournalExports.h"   // ECTBRIDGE_API + Callback-Typedefs

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Bettet die Formlos-Ansicht als Kind des angegebenen Parent-HWND ein.
/// Rueckgabe: HWND des WPF-Fensters (mit SetWindowPos positionierbar),
/// NULL bei Fehler.
/// </summary>
ECTBRIDGE_API HWND ECT_BerichtEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge,
    int nBerichtTyp,
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    double dSchriftgroesse);

/// <summary>Loest eine eingebettete Formlos-Ansicht ab.</summary>
ECTBRIDGE_API void ECT_BerichtAbloesen(HWND hwndBericht);

/// <summary>Loest ALLE eingebetteten Formlos-Ansichten ab.</summary>
ECTBRIDGE_API void ECT_BerichtAlleAbloesen();

/// <summary>
/// Baut alle aktiven Berichte mit den aktuellen Ribbon-Filtern neu auf
/// (Filter-Aenderung oder Buchungs-Aenderung). dSchriftgroesse &lt;= 0
/// laesst den Zoom unveraendert.
/// </summary>
ECTBRIDGE_API void ECT_BerichtAktualisiere(
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    double dSchriftgroesse);

/// <summary>
/// Wechselt den Berichtstyp der aktiven Formlos-Ansicht (Auswahl im
/// Formlos-Menue, waehrend die Ansicht bereits offen ist).
/// </summary>
ECTBRIDGE_API void ECT_BerichtWechsleTyp(int nBerichtTyp);

/// <summary>Setzt nur die Schriftgroesse (Zoom) der Formlos-Ansicht.</summary>
ECTBRIDGE_API void ECT_BerichtSetzeZoom(double dSchriftgroesse);

/// <summary>
/// Druckt den aktiven Bericht (WYSIWYG). bVorschau=TRUE oeffnet die
/// Seitenansicht. Rueckgabe FALSE, wenn keine Formlos-Ansicht aktiv ist.
/// </summary>
ECTBRIDGE_API BOOL ECT_BerichtDrucken(BOOL bVorschau);

/// <summary>
/// Registriert die Callbacks fuer Zoom-Tasten und Strg+P aus der
/// Formlos-Ansicht (gleiche Typedefs wie beim Journal). NULL
/// deregistriert den jeweiligen Callback.
/// </summary>
ECTBRIDGE_API void ECT_BerichtRegistriereCallbacks(
    ECT_JournalZoomAenderungCallback pfnZoom,
    ECT_JournalDruckAnforderungCallback pfnDruck);

#ifdef __cplusplus
}
#endif
