// BerichtExports.cpp - Implementierung der Formlos-Ansicht-Exports.
//
// Muster wie JournalExports.cpp: /clr, kein PCH, ruft die managed
// Klasse ECTViews::Berichte::BerichtEmbed (HwndSource-Hosting).
// Die Callbacks (Zoom, Strg+P) werden von easycashview.cpp registriert
// und laufen ueber eine ref class als Delegate-Target (C++/CLI-Lambdas
// koennen keine managed Variablen capturen).

#include "stdafx.h"
#include "BerichtExports.h"
#include "EasyCashDocBridge.h"      // CEasyCashDocBridge + GetEngine(bridge)

#using "ECTEngine.dll"
#using "ECTViews.dll"
#using <System.dll>
#using <WindowsBase.dll>

using namespace System;

// ----------------------------------------------------------
// Callbacks (Zoom + Druckwunsch aus der Formlos-Ansicht)
// ----------------------------------------------------------
static ECT_JournalZoomAenderungCallback     g_pfnBerichtZoom  = NULL;
static ECT_JournalDruckAnforderungCallback  g_pfnBerichtDruck = NULL;

void ECT_BerichtRegistriereCallbacks(
    ECT_JournalZoomAenderungCallback pfnZoom,
    ECT_JournalDruckAnforderungCallback pfnDruck)
{
    g_pfnBerichtZoom  = pfnZoom;
    g_pfnBerichtDruck = pfnDruck;
}

// Delegate-Targets fuer die ViewModel-Events
ref class BerichtEventHandler
{
public:
    void OnZoomAenderung(int deltaProzent)
    {
        if (g_pfnBerichtZoom)
            g_pfnBerichtZoom(deltaProzent);
    }

    void OnDruckAnforderung()
    {
        if (g_pfnBerichtDruck)
            g_pfnBerichtDruck();
    }
};

// ----------------------------------------------------------
// ECT_BerichtEinbetten
// ----------------------------------------------------------
HWND ECT_BerichtEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge,
    int nBerichtTyp,
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    double dSchriftgroesse)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge) return NULL;

        // Engine-Stand garantieren (Berichte rechnen auf den managed Buchungen)
        bridge->SyncNativeToManaged();
        auto engine = GetEngine(bridge);

        IntPtr hParent(hwndParent);
        IntPtr hKind = ECTViews::Berichte::BerichtEmbed::Einbetten(
            hParent, x, y, width, height,
            engine,
            (ECTViews::Berichte::BerichtTyp)nBerichtTyp,
            nMonatsFilter,
            (pszBetriebFilter ? gcnew String(pszBetriebFilter) : String::Empty),
            dSchriftgroesse);

        if (hKind == IntPtr::Zero) return NULL;

        // Zoom + Strg+P an den nativen Mechanismus verdrahten
        auto vm = ECTViews::Berichte::BerichtEmbed::FindeViewModel(hKind);
        if (vm != nullptr)
        {
            auto handler = gcnew BerichtEventHandler();
            vm->ZoomAendern += gcnew System::Action<int>(
                handler, &BerichtEventHandler::OnZoomAenderung);
            vm->DruckAnfordern += gcnew System::Action(
                handler, &BerichtEventHandler::OnDruckAnforderung);
        }

        return (HWND)hKind.ToPointer();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_BerichtEinbetten: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return NULL;
    }
}

// ----------------------------------------------------------
// ECT_BerichtAbloesen / ECT_BerichtAlleAbloesen
// ----------------------------------------------------------
void ECT_BerichtAbloesen(HWND hwndBericht)
{
    try
    {
        IntPtr h(hwndBericht);
        ECTViews::Berichte::BerichtEmbed::Abloesen(h);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_BerichtAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

void ECT_BerichtAlleAbloesen()
{
    try
    {
        ECTViews::Berichte::BerichtEmbed::AlleAbloesen();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_BerichtAlleAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_BerichtAktualisiere / ECT_BerichtWechsleTyp / ECT_BerichtSetzeZoom
// ----------------------------------------------------------
void ECT_BerichtAktualisiere(
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    double dSchriftgroesse)
{
    try
    {
        ECTViews::Berichte::BerichtEmbed::AktualisiereAlle(
            nMonatsFilter,
            (pszBetriebFilter ? gcnew String(pszBetriebFilter) : String::Empty),
            dSchriftgroesse);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_BerichtAktualisiere: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

void ECT_BerichtWechsleTyp(int nBerichtTyp)
{
    try
    {
        ECTViews::Berichte::BerichtEmbed::WechsleTyp(
            (ECTViews::Berichte::BerichtTyp)nBerichtTyp);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_BerichtWechsleTyp: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

void ECT_BerichtSetzeZoom(double dSchriftgroesse)
{
    try
    {
        if (dSchriftgroesse <= 0) dSchriftgroesse = 13.0;
        ECTViews::Berichte::BerichtEmbed::SetzeZoom(dSchriftgroesse);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_BerichtSetzeZoom: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_BerichtDrucken
// ----------------------------------------------------------
BOOL ECT_BerichtDrucken(BOOL bVorschau)
{
    try
    {
        return ECTViews::Berichte::BerichtEmbed::DruckeAktives(
            bVorschau != FALSE) ? TRUE : FALSE;
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_BerichtDrucken: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
    return FALSE;
}
