// JournalExports.cpp - Implementierung der Journal-Embed-Exports.
//
// Diese Datei muss mit /clr kompiliert werden (gemischter Modus).
// Sie ruft die managed Klasse ECTViews::Journal::JournalEmbed auf,
// die wiederum HwndSource-basiertes WPF-Hosting im übergebenen
// Parent-HWND macht.
//
// Compile-Voraussetzungen:
//   - /clr Switch für dieses File (NICHT pure native)
//   - ECTViews.dll und ECTEngine.dll als Referenzen
//   - Kein PCH (PrecompiledHeader=NotUsing)

#include "stdafx.h"
#include "JournalExports.h"
#include "ViewExports.h"            // ECT_ShowBuchungBearbeitenDialog
#include "EasyCashDocBridge.h"      // CEasyCashDocBridge + GetEngine(bridge)
#include "Marshalling.h"            // ECTBridge::ToNative / ToManaged

#using "ECTEngine.dll"
#using "ECTViews.dll"
#using <System.dll>
#using <WindowsBase.dll>

// Aus ECTBridge/ectifacemisc.cpp (früher ECTIFace): liefert den
// Kontonamen, der mit dem angegebenen EUER- bzw. UVA-Formularfeld
// verknüpft ist, oder NULL, wenn keine Verknuepfung existiert. Wird
// von OnAfaAbgang benötigt, um das Restwert-Konto zu finden.
extern "C" AFX_EXT_CLASS char* HoleKontoFuerFeld(
    char ea, LPCSTR eurech_feld, LPCSTR uva_feld);

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
// JournalEventHandler - hält Native-Pointer als IntPtr-Felder
// und liefert die Methoden, die als Delegate-Targets für die
// ViewModel-Events dienen. C++/CLI-Lambdas können keine managed
// Variablen capturen und Delegates brauchen managed Method-Pointers,
// daher dieser Umweg über eine ref class.
// ----------------------------------------------------------

// Liefert den Index einer Buchung in der aktuellen Liste, mit Fallback
// auf Uuid-Match. Notwendig, weil ECT_ShowBuchungBearbeitenDialog am
// Anfang SyncNativeToManaged ruft und damit alle managed Buchung^-
// Referenzen austauscht. Nach einem Cancel hat das Journal noch die
// alten Referenzen, IndexOf liefert dann -1.
static int FindeBuchungIdx(ECTEngine::BuchungsDocument^ eng, ECTEngine::Buchung^ b)
{
    if (eng == nullptr || b == nullptr) return -1;
    int idx = eng->Buchungen->IndexOf(b);
    if (idx >= 0) return idx;
    for (int i = 0; i < eng->Buchungen->Count; i++)
        if (eng->Buchungen[i]->Uuid == b->Uuid)
            return i;
    return -1;
}

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

        int idx = FindeBuchungIdx(GetEngine(bridge), b);
        if (idx >= 0)
        {
            // Bei Cancel (Dialog gibt FALSE zurück) wird kein Rebuild nötig --
            // spart Sortierung. Die nächste Bearbeiten-Anfrage findet den
            // Index dank Uuid-Fallback in FindeBuchungIdx auch über stale
            // Buchung^-Referenzen, die durch das eingangs gerufene
            // SyncNativeToManaged entstanden sind.
            if (ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd))
                ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
        }
    }

    // Loescht eine ODER mehrere Buchungen (Mehrfachauswahl im Journal).
    // HINWEIS Encoding: Diese Datei ist cp1252-kodiert (wie ViewExports.cpp); AfxMessageBox ist
    // MBCS (cp1252). Umlaute als UTF-8-Literal wuerden vermurkst ("loeschen").
    // Darum den ASCII-unabhaengigen Oktal-Escape \366 (= 0xF6 = cp1252 'ö')
    // verwenden -- erscheint korrekt, egal wie die Datei gespeichert ist.
    void OnLoeschenMehrere(System::Collections::Generic::IList<ECTEngine::Buchung^>^ buchungen)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        if (!bridge || buchungen == nullptr || buchungen->Count == 0) return;
        auto eng = GetEngine(bridge);

        CString frage;
        if (buchungen->Count == 1)
        {
            int idx = FindeBuchungIdx(eng, buchungen[0]);
            CString desc = (idx >= 0)
                ? CString(eng->Buchungen[idx]->Beschreibung)
                : CString(buchungen[0]->Beschreibung);
            frage.Format("Buchung '%s' wirklich l\366schen?", (LPCTSTR)desc);
        }
        else
        {
            frage.Format("%d Buchungen wirklich l\366schen?", buchungen->Count);
        }
        if (AfxMessageBox(frage, MB_YESNO | MB_DEFBUTTON2) != IDYES)
            return;

        // Alle selektierten Buchungen per Uuid finden und entfernen. Der
        // Index wird pro Buchung neu ermittelt, weil RemoveAt die Liste
        // umnummeriert.
        for each (ECTEngine::Buchung^ b in buchungen)
        {
            int i = FindeBuchungIdx(eng, b);
            if (i >= 0) eng->Buchungen->RemoveAt(i);
        }
        bridge->SyncManagedToNative();
        bridge->SetModifiedFlag("Buchungen ueber Journal geloescht");
        ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
    }

    void OnKopieren(ECTEngine::Buchung^ b)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
        if (!bridge || !b) return;

        auto eng = GetEngine(bridge);
        auto klon = b->Clone();
        klon->Uuid = System::Guid::NewGuid();   // neue Identitaet für den Klon
        eng->Buchungen->Add(klon);
        eng->Sort();
        bridge->SyncManagedToNative();
        int idx = FindeBuchungIdx(eng, klon);

        if (ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd))
        {
            ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
        }
        else
        {
            // Cancel: den vorbereiteten Klon wieder zurücknehmen, sonst
            // bliebe ein leerer/identischer Eintrag im Dokument hängen.
            // Klon-Referenz ist nach SyncNativeToManaged stale, deshalb
            // über Uuid suchen.
            int klonIdx = FindeBuchungIdx(eng, klon);
            if (klonIdx >= 0)
                eng->Buchungen->RemoveAt(klonIdx);
            bridge->SyncManagedToNative();
        }
    }

    void OnKopierenMitNeuerBelegnummer(ECTEngine::Buchung^ b)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
        if (!bridge || !b) return;

        auto eng = GetEngine(bridge);
        auto klon = b->Clone();
        klon->Uuid = System::Guid::NewGuid();   // neue Identitaet für den Klon
        klon->Belegnummer = (klon->Art == ECTEngine::Buchungsart::Einnahme)
            ? eng->LaufendeBelegnrEinnahmen.ToString()
            : eng->LaufendeBelegnrAusgaben.ToString();
        eng->Buchungen->Add(klon);
        eng->Sort();
        bridge->SyncManagedToNative();
        int idx = FindeBuchungIdx(eng, klon);

        if (ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd))
        {
            ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
        }
        else
        {
            // Cancel: den vorbereiteten Klon wieder zurücknehmen.
            int klonIdx = FindeBuchungIdx(eng, klon);
            if (klonIdx >= 0)
                eng->Buchungen->RemoveAt(klonIdx);
            bridge->SyncManagedToNative();
        }
    }

    void OnAfaAbgang(ECTEngine::Buchung^ b)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        if (!bridge || !b) return;
        if (b->AfaJahre <= 1) return;  // nur Anlagen mit laufender AfA

        auto eng = GetEngine(bridge);

        // Stale-Referenz absichern: nach SyncNativeToManaged-Zyklen kann
        // sich die managed Buchung^-Instanz geändert haben. Wir suchen die
        // aktuelle Instanz per Uuid.
        int idx = FindeBuchungIdx(eng, b);
        if (idx < 0) return;
        ECTEngine::Buchung^ aktuelle = eng->Buchungen[idx];

        CString frage;
        frage.Format("Anlagengegenstand '%s' aus dem Betriebsvermögen ausscheiden lassen?\n\n"
                     "Die AfA-Buchung wird dabei in eine einfache Ausgaben-Buchung"
                     " über den Restwert umgewandelt.",
                     (LPCTSTR)CString(aktuelle->Beschreibung));
        if (AfxMessageBox(frage, MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;

        // Restbuchwert-Konto aus den Einstellungen ermitteln. Logik 1:1 aus
        // dem nativen AfAAbgang (easycashview.cpp:7015-7028).
        int land = ECTEngine::Einstellungen::HoleInt("land", 0);
        LPCSTR feldNr = (land == 1) ? "9210" : "1135";
        char* pKonto = HoleKontoFuerFeld('A', feldNr, NULL);
        CString csKonto;
        if (pKonto)
        {
            csKonto = pKonto;
        }
        else
        {
            csKonto = "Restbuchwert abgegangener Anlagegüter";
            CString hinweis;
            hinweis.Format(
                "Es wurde kein Konto gefunden, das mit dem Formularfeld %s verknüpft ist. "
                "Deshalb wurde in der Buchung provisorisch das Konto '%s' eingetragen. "
                "Wenn Sie Formulare benutzen, sollten Sie dieses Ausgabenkonto in den "
                "Einstellungen -> E/Über-Konten anlegen und dem %s-Formularfeld %s zuweisen.",
                (LPCTSTR)CString(feldNr), (LPCTSTR)csKonto,
                (land == 1) ? "E1a" : "EUR",
                (LPCTSTR)CString(feldNr));
            AfxMessageBox(hinweis, MB_ICONINFORMATION);
        }

        // Engine-Methode macht die eigentliche Mutation
        if (!eng->AfaAbgang(aktuelle,
            gcnew System::String((LPCTSTR)csKonto)))
            return;

        eng->Sort();
        bridge->SyncManagedToNative();
        bridge->SetModifiedFlag(
            (CString)"Anlagengut '" +
            ECTBridge::ToNative(aktuelle->Erweiterungen->Hole(
                "EasyCash", "UrspruenglichesKonto", "")) +
            "' aus dem Betriebsvermögen entnommen");
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
            vm->BuchungenLoeschen += gcnew System::Action<
                System::Collections::Generic::IList<ECTEngine::Buchung^>^>(
                handler, &JournalEventHandler::OnLoeschenMehrere);
            vm->BuchungKopieren += gcnew System::Action<ECTEngine::Buchung^>(
                handler, &JournalEventHandler::OnKopieren);
            vm->BuchungKopierenMitNeuerBelegnummer += gcnew System::Action<ECTEngine::Buchung^>(
                handler, &JournalEventHandler::OnKopierenMitNeuerBelegnummer);
            vm->BuchungAfaAbgang += gcnew System::Action<ECTEngine::Buchung^>(
                handler, &JournalEventHandler::OnAfaAbgang);
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
        filter->AnzeigeModus =
            (ECTViews::Journal::JournalAnzeigeModus)nAnzeigeModus;
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
// ECT_JournalSelektiere - mehrere Buchungen per Uuid markieren
// ----------------------------------------------------------
void ECT_JournalSelektiere(LPCSTR pszUuids)
{
    try
    {
        if (!pszUuids || !*pszUuids) return;

        auto uuids = gcnew System::Collections::Generic::List<System::Guid>();
        System::String^ alle = gcnew System::String(pszUuids);
        array<wchar_t>^ trenner = { L';' };
        for each (System::String^ teil in alle->Split(
            trenner, System::StringSplitOptions::RemoveEmptyEntries))
        {
            System::Guid g;
            if (System::Guid::TryParse(teil->Trim(), g))
                uuids->Add(g);
        }
        if (uuids->Count > 0)
            ECTViews::Journal::JournalEmbed::SelektiereBuchungen(uuids);
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_JournalSelektiere: ";
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

// ----------------------------------------------------------
// ECT_JournalSendKey - Sendet Navigations-Tasten an das Journal
// ----------------------------------------------------------
extern "C" ECTBRIDGE_API void ECT_JournalSendKey(UINT nChar)
{
    try
    {
        System::Windows::Input::Key key;
        switch (nChar)
        {
            case VK_UP:    key = System::Windows::Input::Key::Up;       break;
            case VK_DOWN:  key = System::Windows::Input::Key::Down;     break;
            case VK_PRIOR: key = System::Windows::Input::Key::PageUp;   break;
            case VK_NEXT:  key = System::Windows::Input::Key::PageDown; break;
            case VK_HOME:  key = System::Windows::Input::Key::Home;     break;
            case VK_END:   key = System::Windows::Input::Key::End;      break;
            default: return;
        }
        ECTViews::Journal::JournalEmbed::NavigiereScroll(key);
    }
    catch (System::Exception^ ex)
    {
        AfxTrace("ECT_JournalSendKey: Exception - %S\r\n", 
            (LPCTSTR)CString(ex->Message));
    }
}
