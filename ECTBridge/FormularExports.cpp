// FormularExports.cpp - Implementierung der Formular-Ansicht-Exports.
//
// Muster wie BerichtExports.cpp: /clr, kein PCH, ruft die managed
// Klasse ECTViews::Formulare::FormularEmbed (HwndSource-Hosting).
// Die Callbacks (Zoom, Strg+P) werden von easycashview.cpp registriert
// und laufen ueber eine ref class als Delegate-Target (C++/CLI-Lambdas
// koennen keine managed Variablen capturen).

#include "stdafx.h"
#include "FormularExports.h"
#include "EasyCashDocBridge.h"      // CEasyCashDocBridge + GetEngine(bridge)

#using "ECTEngine.dll"
#using "ECTViews.dll"
#using <System.dll>
#using <WindowsBase.dll>

using namespace System;

// ----------------------------------------------------------
// Callbacks (Zoom + Druckwunsch aus der Formular-Ansicht)
// ----------------------------------------------------------
static ECT_JournalZoomAenderungCallback     g_pfnFormularZoom  = NULL;
static ECT_JournalDruckAnforderungCallback  g_pfnFormularDruck = NULL;

void ECT_FormularRegistriereCallbacks(
    ECT_JournalZoomAenderungCallback pfnZoom,
    ECT_JournalDruckAnforderungCallback pfnDruck)
{
    g_pfnFormularZoom  = pfnZoom;
    g_pfnFormularDruck = pfnDruck;
}

// Delegate-Targets fuer die ViewModel-Events
ref class FormularEventHandler
{
public:
    void OnZoomAenderung(int deltaProzent)
    {
        if (g_pfnFormularZoom)
            g_pfnFormularZoom(deltaProzent);
    }

    void OnDruckAnforderung()
    {
        if (g_pfnFormularDruck)
            g_pfnFormularDruck();
    }
};

// ----------------------------------------------------------
// ECT_FormularEinbetten
// ----------------------------------------------------------
HWND ECT_FormularEinbetten(
    HWND hwndParent,
    int x, int y, int width, int height,
    void* pDocBridge,
    LPCSTR pszEcfPfad,
    LPCSTR pszBetriebFilter,
    int nZoomProzent)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge) return NULL;

        // Engine-Stand garantieren (der Rechner arbeitet auf den managed Buchungen)
        bridge->SyncNativeToManaged();
        auto engine = GetEngine(bridge);

        IntPtr hParent(hwndParent);
        IntPtr hKind = ECTViews::Formulare::FormularEmbed::Einbetten(
            hParent, x, y, width, height,
            engine,
            (pszEcfPfad ? gcnew String(pszEcfPfad) : String::Empty),
            (pszBetriebFilter ? gcnew String(pszBetriebFilter) : String::Empty),
            nZoomProzent);

        if (hKind == IntPtr::Zero) return NULL;

        // Zoom + Strg+P an den nativen Mechanismus verdrahten
        auto vm = ECTViews::Formulare::FormularEmbed::FindeViewModel(hKind);
        if (vm != nullptr)
        {
            auto handler = gcnew FormularEventHandler();
            vm->ZoomAendern += gcnew System::Action<int>(
                handler, &FormularEventHandler::OnZoomAenderung);
            vm->DruckAnfordern += gcnew System::Action(
                handler, &FormularEventHandler::OnDruckAnforderung);
        }

        return (HWND)hKind.ToPointer();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_FormularEinbetten: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return NULL;
    }
}

// ----------------------------------------------------------
// ECT_FormularAbloesen / ECT_FormularAlleAbloesen
// ----------------------------------------------------------
void ECT_FormularAbloesen(HWND hwndFormular)
{
    try
    {
        IntPtr h(hwndFormular);
        ECTViews::Formulare::FormularEmbed::Abloesen(h);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_FormularAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

void ECT_FormularAlleAbloesen()
{
    try
    {
        ECTViews::Formulare::FormularEmbed::AlleAbloesen();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_FormularAlleAbloesen: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_FormularAktualisiere / ECT_FormularWechsle / ECT_FormularSetzeZoom
// ----------------------------------------------------------
void ECT_FormularAktualisiere(void* pDocBridge)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (bridge)
            bridge->SyncNativeToManaged();
        ECTViews::Formulare::FormularEmbed::AktualisiereAlle();
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_FormularAktualisiere: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

void ECT_FormularWechsle(
    void* pDocBridge,
    LPCSTR pszEcfPfad,
    LPCSTR pszBetriebFilter)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (bridge)
            bridge->SyncNativeToManaged();
        ECTViews::Formulare::FormularEmbed::WechsleFormular(
            (pszEcfPfad ? gcnew String(pszEcfPfad) : String::Empty),
            (pszBetriebFilter ? gcnew String(pszBetriebFilter) : String::Empty));
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_FormularWechsle: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

void ECT_FormularSetzeZoom(int nZoomProzent)
{
    try
    {
        if (nZoomProzent <= 0) nZoomProzent = 100;
        ECTViews::Formulare::FormularEmbed::SetzeZoom(nZoomProzent);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_FormularSetzeZoom: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// ECT_FormularDrucken
// ----------------------------------------------------------
BOOL ECT_FormularDrucken(BOOL bVorschau)
{
    try
    {
        return ECTViews::Formulare::FormularEmbed::DruckeAktives(
            bVorschau != FALSE) ? TRUE : FALSE;
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_FormularDrucken: ";
        msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
    return FALSE;
}

// ----------------------------------------------------------
// ECT_FormularVergleichsdump -- Golden-Master-Vergleich nativ/managed.
// Der Aufrufer (easycashview.cpp, Debug-Pfad) hat die nativen Werte
// bereits mit BerechneFormularfeldwerte() berechnet und liefert sie
// '\n'-verbunden; hier laeuft der managed FormularRechner auf demselben
// Dokument, verglichen wird Feld fuer Feld in Dateireihenfolge.
// ----------------------------------------------------------
int ECT_FormularVergleichsdump(
    void* pDocBridge,
    LPCSTR pszEcfPfad,
    LPCSTR pszBetriebFilter,
    LPCSTR pszNativWerte)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge || !pszEcfPfad) return -1;

        bridge->SyncNativeToManaged();
        auto engine = GetEngine(bridge);

        auto def = ECTEngine::FormularDefinition::Lade(
            gcnew String(pszEcfPfad));
        auto werte = ECTEngine::FormularRechner::Berechne(engine, def,
            (pszBetriebFilter ? gcnew String(pszBetriebFilter) : String::Empty));

        auto nativ = (pszNativWerte ? gcnew String(pszNativWerte) : String::Empty)
            ->Split(L'\n');

        int nDiff = 0;
        System::Diagnostics::Debug::WriteLine(String::Format(
            "ECT_FormularVergleichsdump: '{0}' Filter '{1}' -- {2} Felder managed, {3} nativ",
            def->Anzeigename,
            (pszBetriebFilter ? gcnew String(pszBetriebFilter) : String::Empty),
            werte->Count, nativ->Length));

        for (int i = 0; i < werte->Count; i++)
        {
            String^ managedWert = werte[i]->Text;
            String^ nativWert = i < nativ->Length ? nativ[i] : "<fehlt>";
            if (!String::Equals(managedWert, nativWert))
            {
                nDiff++;
                System::Diagnostics::Debug::WriteLine(String::Format(
                    "  DIFF Feld {0} ({1}): nativ '{2}' managed '{3}'",
                    werte[i]->Feld->Id, werte[i]->Feld->Name,
                    nativWert, managedWert));
            }
        }
        if (nativ->Length > werte->Count)
        {
            nDiff += nativ->Length - werte->Count;
            System::Diagnostics::Debug::WriteLine(String::Format(
                "  DIFF: nativ hat {0} Felder mehr", nativ->Length - werte->Count));
        }

        System::Diagnostics::Debug::WriteLine(String::Format(
            "ECT_FormularVergleichsdump: {0} Differenzen", nDiff));
        return nDiff;
    }
    catch (Exception^ ex)
    {
        System::Diagnostics::Debug::WriteLine(
            "ECT_FormularVergleichsdump: Fehler " + ex->Message);
        return -1;
    }
}
