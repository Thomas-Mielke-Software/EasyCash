// EinstellungenViewExports.cpp - Implementierung der Einstellungs-Embed-
// Exports. Muss mit /clr kompiliert werden (gemischter Modus), kein PCH.
//
// Ruft die managed Klasse ECTViews::EinstellungenUi::EinstellungenEmbed
// auf, die HwndSource-basiertes WPF-Hosting im uebergebenen Parent-HWND
// macht.

#include "stdafx.h"
#include "EinstellungenViewExports.h"
#include "EasyCashDocBridge.h"      // CEasyCashDocBridge + GetEngine(bridge)

#using "ECTEngine.dll"
#using "ECTViews.dll"
#using <System.dll>
#using <WindowsBase.dll>

using namespace System;

// ----------------------------------------------------------
// Handler, der die WPF-Aenderung eines Dokumentwerts (Buchungsjahr,
// laufende Belegnummern) sofort als Modified-Flag am nativen Dokument
// vermerkt. So fragt MFC beim Schliessen "Speichern?" -- unabhaengig
// davon, ueber welchen Teardown-Pfad das View verlassen wird. Die Daten
// selbst wandern beim Speichern ueber Serialize -> SyncManagedToNative.
//
// Eine /clr-Lambda kann keine managed Variablen capturen und nicht direkt
// einem System::Action-Delegate zugewiesen werden; darum eine ref class
// mit IntPtr-Feld und Member-Methode (vgl. JournalEventHandler).
// ----------------------------------------------------------
ref class EinstellungenDokumentHandler
{
public:
    IntPtr m_pDocBridge;   // CEasyCashDocBridge* als IntPtr

    void OnGeaendert()
    {
        if (m_pDocBridge == IntPtr::Zero) return;
        CEasyCashDocBridge* bridge = (CEasyCashDocBridge*)m_pDocBridge.ToPointer();
        bridge->SetModifiedFlag();
    }
};

// ----------------------------------------------------------
// ECT_EinstellungenEinbetten
// ----------------------------------------------------------
HWND ECT_EinstellungenEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge,
    LPCSTR szStartSeite)
{
    try
    {
        // Falls ein Dokument offen ist: native Daten in die managed Engine
        // spiegeln und die BuchungsDocument^-Instanz an die WPF-Seite geben.
        // Die "Aktuelles Dokument"-Seite schreibt dann direkt in diese Engine.
        ECTEngine::BuchungsDocument^ dokument = nullptr;
        System::Action^ onGeaendert = nullptr;
        if (pDocBridge)
        {
            CEasyCashDocBridge* bridge = (CEasyCashDocBridge*)pDocBridge;
            bridge->SyncNativeToManaged();
            dokument = GetEngine(bridge);

            auto handler = gcnew EinstellungenDokumentHandler();
            handler->m_pDocBridge = IntPtr(pDocBridge);
            onGeaendert = gcnew System::Action(
                handler, &EinstellungenDokumentHandler::OnGeaendert);
        }

        System::String^ startSeite = (szStartSeite && *szStartSeite)
            ? gcnew System::String(szStartSeite) : nullptr;

        IntPtr hParent(hwndParent);
        IntPtr hKind = ECTViews::EinstellungenUi::EinstellungenEmbed::Einbetten(
            hParent, x, y, width, height, dokument, onGeaendert, startSeite);

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
