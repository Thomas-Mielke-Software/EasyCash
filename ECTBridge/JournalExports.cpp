// JournalExports.cpp - Implementierung der Journal-Embed-Exports.
//
// Diese Datei muss mit /clr kompiliert werden (gemischter Modus).
// Sie ruft die managed Klasse ECTViews::Journal::JournalEmbed auf,
// die wiederum HwndSource-basiertes WPF-Hosting im uebergebenen
// Parent-HWND macht.
//
// Compile-Voraussetzungen:
//   - /clr Switch fuer dieses File (NICHT pure native)
//   - ECTViews.dll und ECTEngine.dll als Referenzen
//   - Kein PCH (PrecompiledHeader=NotUsing)

#include "stdafx.h"
#include "JournalExports.h"
#include "EasyCashDocBridge.h"   // CEasyCashDocBridge + GetEngine(bridge)

#using "ECTEngine.dll"
#using "ECTViews.dll"

using namespace System;

// ----------------------------------------------------------
// Helper: native LPCSTR -> managed String^
// ----------------------------------------------------------
static System::String^ ToManagedString(LPCSTR psz)
{
    if (!psz) return System::String::Empty;
    return gcnew System::String(psz);
}

// ----------------------------------------------------------
// ECT_JournalEinbetten
// ----------------------------------------------------------
HWND ECT_JournalEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge,
    int nAnzeigeModus,
    double dSchriftgroesse)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge) return NULL;

        // Engine-Stand garantieren
        bridge->SyncNativeToManaged();
        auto engine = GetEngine(bridge);

        // Initialfilter aufbauen
        auto filter = gcnew ECTViews::Journal::JournalFilter();
        filter->AnzeigeModus = (nAnzeigeModus == 1)
            ? ECTViews::Journal::JournalAnzeigeModus::Konten
            : ECTViews::Journal::JournalAnzeigeModus::Datum;
        filter->Schriftgroesse = (dSchriftgroesse > 0) ? dSchriftgroesse : 13.0;

        IntPtr hParent(hwndParent);
        IntPtr hKind = ECTViews::Journal::JournalEmbed::Einbetten(
            hParent, x, y, width, height, engine, filter);

        return (HWND)hKind.ToPointer();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_JournalEinbetten: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return NULL;
    }
}

// ----------------------------------------------------------
// ECT_JournalAbloesen
// ----------------------------------------------------------
void ECT_JournalAbloesen(HWND hwndJournal)
{
    try
    {
        IntPtr h(hwndJournal);
        ECTViews::Journal::JournalEmbed::Abloesen(h);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_JournalAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_JournalAlleAbloesen
// ----------------------------------------------------------
void ECT_JournalAlleAbloesen()
{
    try
    {
        ECTViews::Journal::JournalEmbed::AlleAbloesen();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_JournalAlleAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_JournalAktualisiere
// ----------------------------------------------------------
void ECT_JournalAktualisiere(
    int nAnzeigeModus,
    LPCSTR pszKontenFilter,
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    LPCSTR pszBestandskontoFilter,
    double dSchriftgroesse)
{
    try
    {
        auto filter = gcnew ECTViews::Journal::JournalFilter();
        filter->AnzeigeModus = (nAnzeigeModus == 1)
            ? ECTViews::Journal::JournalAnzeigeModus::Konten
            : ECTViews::Journal::JournalAnzeigeModus::Datum;
        filter->KontenFilter        = ToManagedString(pszKontenFilter);
        filter->MonatsFilter        = nMonatsFilter;
        filter->BetriebFilter       = ToManagedString(pszBetriebFilter);
        filter->BestandskontoFilter = ToManagedString(pszBestandskontoFilter);
        filter->Schriftgroesse      = (dSchriftgroesse > 0) ? dSchriftgroesse : 13.0;

        ECTViews::Journal::JournalEmbed::AktualisiereAlle(filter);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_JournalAktualisiere: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_JournalSetzeZoom
// ----------------------------------------------------------
void ECT_JournalSetzeZoom(double dSchriftgroesse)
{
    try
    {
        if (dSchriftgroesse <= 0) dSchriftgroesse = 13.0;
        ECTViews::Journal::JournalEmbed::SetzeZoom(dSchriftgroesse);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_JournalSetzeZoom: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_NavigationEinbetten
// ----------------------------------------------------------
HWND ECT_NavigationEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    HWND hwndJournal)
{
    try
    {
        IntPtr hParent(hwndParent);
        IntPtr hJ(hwndJournal);
        IntPtr hN = ECTViews::Journal::JournalEmbed::NavigationEinbetten(
            hParent, x, y, width, height, hJ);
        return (HWND)hN.ToPointer();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_NavigationEinbetten: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return NULL;
    }
}

// ----------------------------------------------------------
// ECT_NavigationAbloesen
// ----------------------------------------------------------
void ECT_NavigationAbloesen(HWND hwndNav)
{
    try
    {
        IntPtr h(hwndNav);
        ECTViews::Journal::JournalEmbed::NavigationAbloesen(h);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_NavigationAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}
