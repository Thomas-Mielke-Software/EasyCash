// EinstellungenViewExports.cpp - Implementierung der Einstellungs-Embed-
// Exports. Muss mit /clr kompiliert werden (gemischter Modus), kein PCH.
//
// Ruft die managed Klasse ECTViews::EinstellungenUi::EinstellungenEmbed
// auf, die HwndSource-basiertes WPF-Hosting im uebergebenen Parent-HWND
// macht.

#include "stdafx.h"
#include "EinstellungenViewExports.h"

#using "ECTEngine.dll"
#using "ECTViews.dll"
#using <System.dll>
#using <WindowsBase.dll>

using namespace System;

// ----------------------------------------------------------
// ECT_EinstellungenEinbetten
// ----------------------------------------------------------
HWND ECT_EinstellungenEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    BOOL hatDokument)
{
    try
    {
        IntPtr hParent(hwndParent);
        IntPtr hKind = ECTViews::EinstellungenUi::EinstellungenEmbed::Einbetten(
            hParent, x, y, width, height, hatDokument ? true : false);

        if (hKind == IntPtr::Zero) return NULL;
        return (HWND)hKind.ToPointer();
    }
    catch (Exception^ ex)
    {
        CString msg = "Fehler in ECT_EinstellungenEinbetten: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return NULL;
    }
}

// ----------------------------------------------------------
// ECT_EinstellungenViewAbloesen
// ----------------------------------------------------------
void ECT_EinstellungenViewAbloesen(HWND hwndEinstellungen)
{
    try
    {
        IntPtr h(hwndEinstellungen);
        ECTViews::EinstellungenUi::EinstellungenEmbed::Abloesen(h);
    }
    catch (Exception^ ex)
    {
        CString msg = "Fehler in ECT_EinstellungenViewAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_EinstellungenAlleAbloesen
// ----------------------------------------------------------
void ECT_EinstellungenAlleAbloesen()
{
    try
    {
        ECTViews::EinstellungenUi::EinstellungenEmbed::AlleAbloesen();
    }
    catch (Exception^ ex)
    {
        CString msg = "Fehler in ECT_EinstellungenAlleAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}
