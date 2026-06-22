// EinstellungenViewExports.h - Native Exports fuer das eingebettete
// Einstellungs-View (WPF). Analog zu JournalExports.h, aber einfacher:
// die Navigation ist Teil von EinstellungenView selbst, deshalb gibt es
// pro Einbettung nur EIN HwndSource-Fenster (keine separate Nav-Pane).
//
// NICHT zu verwechseln mit EinstellungenExports.h -- das ist der
// Key-Value-Cache (ini-I/O). Diese Datei hier ist nur das View-Hosting.
//
// Verwendungsmuster (aus easycashview.cpp):
//
//     CRect rc;
//     GetParent()->GetClientRect(&rc);
//     m_hwndEinstellungenWpf = ECT_EinstellungenEinbetten(
//         GetParent()->m_hWnd,
//         rc.left, rc.top, rc.Width(), rc.Height(),
//         GetDocument());   // CEasyCashDocBridge* oder NULL
//     ShowWindow(SW_HIDE);
//
// Beim Verlassen des Modus: ECT_EinstellungenAlleAbloesen().

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
/// Bettet das Einstellungs-View als Kind des angegebenen Parent-HWND ein.
/// Liefert das HWND des erzeugten WPF-Fensters zurueck (vom Aufrufer per
/// SetWindowPos zu positionieren), NULL bei Fehler.
/// </summary>
/// <param name="hwndParent">HWND des Parents (z.B. ChildFrame/Splitter).</param>
/// <param name="x">X-Position relativ zum Parent.</param>
/// <param name="y">Y-Position relativ zum Parent.</param>
/// <param name="width">Breite in Pixeln.</param>
/// <param name="height">Hoehe in Pixeln.</param>
/// <param name="pDocBridge">Zeiger auf das aktuelle CEasyCashDocBridge
/// (in der Praxis GetDocument()) oder NULL. Bei != NULL erscheint die
/// "Aktuelles Dokument"-Gruppe (Buchungsjahr, laufende Belegnummern), die
/// direkt in die managed Engine dieses Dokuments schreibt.</param>
ECTBRIDGE_API HWND ECT_EinstellungenEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge);

/// <summary>Loest ein einzelnes eingebettetes Einstellungs-Fenster ab.</summary>
ECTBRIDGE_API void ECT_EinstellungenViewAbloesen(HWND hwndEinstellungen);

/// <summary>Loest ALLE aktiven Einstellungs-Fenster ab.</summary>
ECTBRIDGE_API void ECT_EinstellungenAlleAbloesen();

#ifdef __cplusplus
}
#endif
