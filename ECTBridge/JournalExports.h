// JournalExports.h - Native Exports fuer das eingebettete Journal-View.
//
// Diese Funktionen sind als Ergaenzung zu ViewExports.h gedacht. Sie
// erzeugen das Journal NICHT als schwebendes Fenster, sondern als
// Kind-HWND eines vom Aufrufer angegebenen Parent-Fensters - typisch
// das CChildFrame eines MFC-MDI-Programms.
//
// Verwendungsmuster (Pseudocode aus easycashview.cpp):
//
//     CRect rc;
//     GetParent()->GetClientRect(&rc);
//     m_hwndJournal = ECT_JournalEinbetten(
//         GetParent()->m_hWnd,    // ChildFrame als Parent
//         rc.left, rc.top, rc.Width(), rc.Height(),
//         GetDocument(),           // pDocBridge
//         0,                       // 0=Datum, 1=Konten
//         13.0);                   // Zoom
//
//     // Eigenes View verstecken:
//     ShowWindow(SW_HIDE);
//
// Bei jeder Aenderung von Filter oder Buchungen rufst du
// ECT_JournalAktualisiere() auf - das wirkt auf alle aktiven
// Journal-Instanzen gleichzeitig (typischerweise nur eine).
//
// Beim Wechsel zurueck zur alten View-Modus oder beim Schliessen
// des Dokuments rufst du ECT_JournalAlleAbloesen() auf.

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
/// Bettet ein Journal-View als Kind des angegebenen Parent-HWND ein.
/// Liefert das HWND des erzeugten WPF-Fensters zurueck, das der
/// Aufrufer mit SetWindowPos positionieren kann (z.B. bei OnSize).
///
/// Rueckgabe: HWND des Journal-Fensters, NULL bei Fehler.
/// </summary>
/// <param name="hwndParent">HWND des Parents (z.B. ChildFrame).</param>
/// <param name="x">X-Position relativ zum Parent.</param>
/// <param name="y">Y-Position relativ zum Parent.</param>
/// <param name="width">Breite in Pixeln.</param>
/// <param name="height">Hoehe in Pixeln.</param>
/// <param name="pDocBridge">Pointer auf CEasyCashDocBridge.</param>
/// <param name="nAnzeigeModus">0=Datum, 1=Konten.</param>
/// <param name="dSchriftgroesse">Anfaengliche Schriftgroesse (Standard 13.0).</param>
ECTBRIDGE_API HWND ECT_JournalEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge,
    int nAnzeigeModus,
    double dSchriftgroesse);

/// <summary>
/// Loest ein einzelnes eingebettetes Journal-Fenster wieder ab.
/// Das HWND ist danach ungueltig.
/// </summary>
ECTBRIDGE_API void ECT_JournalAbloesen(HWND hwndJournal);

/// <summary>
/// Loest ALLE aktiven Journal-Fenster ab. Wird typischerweise beim
/// Wechsel zu einer anderen View-Modus oder beim Schliessen des
/// Dokuments aufgerufen.
/// </summary>
ECTBRIDGE_API void ECT_JournalAlleAbloesen();

/// <summary>
/// Aktualisiert ALLE aktiven Journal-Fenster mit einem neuen Filter.
/// Wirkt sofort - die Anzeige wird neu aufgebaut.
///
/// Parameter sind dieselben wie bei der ehemaligen
/// ECT_AktualisiereJournal-Funktion:
///   nAnzeigeModus      - 0=Datum, 1=Konten
///   pszKontenFilter    - "" oder "<alle Konten>" fuer kein Filter
///   nMonatsFilter      - 0=alle, 1-12=Monat, 13-16=Quartal
///   pszBetriebFilter   - "" fuer kein Filter
///   pszBestandskontoFilter - "" fuer kein Filter
///   dSchriftgroesse    - Zoom-Wert
/// </summary>
ECTBRIDGE_API void ECT_JournalAktualisiere(
    int nAnzeigeModus,
    LPCSTR pszKontenFilter,
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    LPCSTR pszBestandskontoFilter,
    double dSchriftgroesse);

/// <summary>
/// Setzt nur die Schriftgroesse (Zoom) auf allen aktiven Journals.
/// Effizienter als ECT_JournalAktualisiere, weil der Filter
/// unveraendert bleibt - nur die Schrift wird groesser/kleiner.
/// </summary>
ECTBRIDGE_API void ECT_JournalSetzeZoom(double dSchriftgroesse);

#ifdef __cplusplus
}
#endif
