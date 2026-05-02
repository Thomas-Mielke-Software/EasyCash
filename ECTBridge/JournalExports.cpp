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
#include "ViewExports.h"

#using "ECTEngine.dll"
#using "ECTViews.dll"

using namespace System;
#using <System.dll>

// ----------------------------------------------------------
// Helper: native LPCSTR -> managed String^
// ----------------------------------------------------------
static System::String^ ToManagedString(LPCSTR psz)
{
    if (!psz) return System::String::Empty;
    return gcnew System::String(psz);
}

// ----------------------------------------------------------
// JournalEventHandler - haelt Native-Pointer als IntPtr-Felder
// und liefert die Methoden, die als Delegate-Targets fuer die
// ViewModel-Events dienen. C++/CLI-Lambdas koennen keine managed
// Variablen capturen und Delegates brauchen managed Method-Pointers,
// daher dieser Umweg ueber eine ref class.
// ----------------------------------------------------------

ref class JournalEventHandler
{
public:
    System::IntPtr m_pBridge;
    System::IntPtr m_hwnd;

    void OnBearbeiten(ECTEngine::Buchung^ b)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
        if (!bridge || !b) return;

        int idx = GetEngine(bridge)->Buchungen->IndexOf(b);
        if (idx >= 0)
        {
            ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd);
            ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
        }
    }

    void OnLoeschen(ECTEngine::Buchung^ b)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        if (!bridge || !b) return;
        auto eng = GetEngine(bridge);

        CString frage;
        frage.Format("Buchung '%s' wirklich loeschen?",
            (LPCTSTR)CString(b->Beschreibung));
        if (AfxMessageBox(frage, MB_YESNO | MB_DEFBUTTON2) != IDYES)
            return;

        eng->Buchungen->Remove(b);
        bridge->SyncManagedToNative();
        bridge->SetModifiedFlag("Buchung ueber Journal geloescht");
        ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
    }

    void OnKopieren(ECTEngine::Buchung^ b)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
        if (!bridge || !b) return;

        auto eng = GetEngine(bridge);
        auto klon = b->Clone();
        eng->Buchungen->Add(klon);
        int idx = eng->Buchungen->IndexOf(klon);
        bridge->SyncManagedToNative();

        ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd);
        ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
    }

    void OnKopierenMitNeuerBelegnummer(ECTEngine::Buchung^ b)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
        if (!bridge || !b) return;

        auto eng = GetEngine(bridge);
        auto klon = b->Clone();
        klon->Belegnummer = (klon->Art == ECTEngine::Buchungsart::Einnahme)
            ? eng->LaufendeBelegnrEinnahmen.ToString()
            : eng->LaufendeBelegnrAusgaben.ToString();
        eng->Buchungen->Add(klon);
        int idx = eng->Buchungen->IndexOf(klon);
        bridge->SyncManagedToNative();

        ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd);
        ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
    }
};

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
        filter->AnzeigeModus = (ECTViews::Journal::JournalAnzeigeModus)nAnzeigeModus;
        filter->Schriftgroesse = (dSchriftgroesse > 0) ? dSchriftgroesse : 13.0;

        IntPtr hParent(hwndParent);
        IntPtr hKind = ECTViews::Journal::JournalEmbed::Einbetten(
            hParent, x, y, width, height, engine, filter);

        if (hKind == IntPtr::Zero) return NULL;

        // Eventhandler an das ViewModel binden, damit Doppelklick und
        // Kontextmenue funktionieren. Ohne diese Verdrahtung feuern die
        // Events ins Leere.
        auto vm = ECTViews::Journal::JournalEmbed::FindeViewModel(hKind);
        if (vm != nullptr)
        {
            auto handler = gcnew JournalEventHandler();
            handler->m_pBridge = IntPtr(pDocBridge);
            handler->m_hwnd = IntPtr((void*)hwndParent);

            vm->BuchungBearbeiten += gcnew System::Action<ECTEngine::Buchung^>(
                handler, &JournalEventHandler::OnBearbeiten);
            vm->BuchungLoeschen += gcnew System::Action<ECTEngine::Buchung^>(
                handler, &JournalEventHandler::OnLoeschen);
            vm->BuchungKopieren += gcnew System::Action<ECTEngine::Buchung^>(
                handler, &JournalEventHandler::OnKopieren);
            vm->BuchungKopierenMitNeuerBelegnummer += gcnew System::Action<ECTEngine::Buchung^>(
                handler, &JournalEventHandler::OnKopierenMitNeuerBelegnummer);
        }

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
