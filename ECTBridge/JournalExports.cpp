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
#include "AfaAbgangShared.h"     // ECTBridge_FuehreAfaAbgang (geteilt mit ViewExports)
#include "BuchungenLoeschenShared.h" // gemeinsames Löschen mit Buchungsgruppen-Abfrage

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
// Zoom-Aenderungs-Callback (Strg-'+'/'-' bzw. Strg-Mausrad im
// WPF-Journal). Wird von easycashview.cpp registriert; im
// OCX-Kontext bleibt er NULL und die Zoom-Tasten sind wirkungslos.
// ----------------------------------------------------------
static ECT_JournalZoomAenderungCallback g_pfnZoomAenderung = NULL;

void ECT_JournalRegistriereZoomAenderung(ECT_JournalZoomAenderungCallback pfn)
{
    g_pfnZoomAenderung = pfn;
}

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

    // Löscht eine ODER mehrere Buchungen (Mehrfachauswahl im Journal).
    // Bestätigung und Kaskadenlösch-Abfrage für Buchungsgruppen stecken
    // in der geteilten Funktion (BuchungenLoeschenShared.h, Definition
    // weiter unten in dieser Datei).
    void OnLoeschenMehrere(System::Collections::Generic::IList<ECTEngine::Buchung^>^ buchungen)
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
        if (!bridge) return;
        if (ECTBridge_LoescheBuchungenMitGruppenAbfrage(bridge, buchungen))
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
        auto eng = GetEngine(bridge);
        // Stale-Referenz absichern (Uuid-Fallback nach Sync-Zyklen),
        // dann die gemeinsame Abgang-Logik aufrufen.
        int idx = FindeBuchungIdx(eng, b);
        if (idx < 0) return;
        ECTBridge_FuehreAfaAbgang(bridge, eng->Buchungen[idx]);
    }

    // Reicht den Zoom-Wunsch (Delta in Prozentpunkten) an den nativen
    // Mechanismus weiter -- der setzt m_zoomfaktor um und verteilt die
    // neue Schriftgroesse ueber ECT_JournalSetzeZoom an alle Journals.
    void OnZoomAenderung(int deltaProzent)
    {
        if (g_pfnZoomAenderung)
            g_pfnZoomAenderung(deltaProzent);
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
            vm->ZoomAendern += gcnew System::Action<int>(
                handler, &JournalEventHandler::OnZoomAenderung);
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

// ----------------------------------------------------------
// Gemeinsame AfA-Abgang-Logik (Journal-Kontextmenue UND Dialog-Button).
// Deklaration in AfaAbgangShared.h. 'aktuelle' muss die aktuelle managed
// Instanz aus eng->Buchungen sein (nicht stale).
// ----------------------------------------------------------
bool ECTBridge_FuehreAfaAbgang(CEasyCashDocBridge* bridge, ECTEngine::Buchung^ aktuelle)
{
    if (!bridge || aktuelle == nullptr) return false;
    if (aktuelle->AfaJahre <= 1) return false;  // nur Anlagen mit laufender AfA
    auto eng = GetEngine(bridge);

        CString frage;
        frage.Format("Anlagengegenstand '%s' aus dem Betriebsvermögen ausscheiden lassen?\n\n"
                     "Die AfA-Buchung wird dabei in eine einfache Ausgaben-Buchung"
                     " über den Restwert umgewandelt.",
                     (LPCTSTR)CString(aktuelle->Beschreibung));
        if (AfxMessageBox(frage, MB_YESNO | MB_ICONQUESTION) != IDYES)
            return false;

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
            return false;

        eng->Sort();
        bridge->SyncManagedToNative();
        bridge->SetModifiedFlag(
            (CString)"Anlagengut '" +
            ECTBridge::ToNative(aktuelle->Erweiterungen->Hole(
                "EasyCash", "UrspruenglichesKonto", "")) +
            "' aus dem Betriebsvermögen entnommen");
        ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);
    return true;
}

// ----------------------------------------------------------
// Gemeinsame Lösch-Logik für das Journal-Kontextmenü (beide
// Journal-Hosts: JournalEmbed und JournalHost). Berücksichtigt
// Buchungsgruppen: sind Mitglieder einer Gruppe selektiert, deren
// übrige Mitglieder NICHT selektiert sind, wird per Ja/Nein/Abbrechen
// gefragt, ob die ganze Gruppe gelöscht werden soll (Kaskadenlöschen).
// Deklaration in BuchungenLoeschenShared.h.
//
// HINWEIS Encoding: AfxMessageBox ist MBCS (cp1252). Umlaute als
// UTF-8-Literal würden vermurkst ("loeschen"). Darum in den Strings
// die ASCII-unabhängigen Oktal-Escapes \366 (=0xF6, kleines oe) und
// \344 (=0xE4, kleines ae) verwenden -- erscheint korrekt, egal wie
// die Datei gespeichert ist.
// ----------------------------------------------------------
bool ECTBridge_LoescheBuchungenMitGruppenAbfrage(
    CEasyCashDocBridge* bridge,
    System::Collections::Generic::IList<ECTEngine::Buchung^>^ buchungen)
{
    if (!bridge || buchungen == nullptr || buchungen->Count == 0)
        return false;
    auto eng = GetEngine(bridge);

    // 1) Selektion auf aktuelle managed Instanzen auflösen (Referenzen
    //    können nach Sync-Zyklen stale sein) und Duplikate entfernen.
    auto selUuids    = gcnew System::Collections::Generic::List<System::Guid>();
    auto loeschListe = gcnew System::Collections::Generic::List<ECTEngine::Buchung^>();
    for each (ECTEngine::Buchung^ b in buchungen)
    {
        int i = FindeBuchungIdx(eng, b);
        if (i < 0) continue;
        auto cur = eng->Buchungen[i];
        if (selUuids->Contains(cur->Uuid)) continue;
        selUuids->Add(cur->Uuid);
        loeschListe->Add(cur);
    }
    if (loeschListe->Count == 0) return false;

    // 2) Beteiligte Buchungsgruppen und deren NICHT selektierte
    //    Mitglieder ermitteln.
    auto gruppen = gcnew System::Collections::Generic::List<System::String^>();
    for each (ECTEngine::Buchung^ cur in loeschListe)
    {
        System::String^ g = cur->GruppenUuid;
        if (g != nullptr && !gruppen->Contains(g)) gruppen->Add(g);
    }
    auto fehlende = gcnew System::Collections::Generic::List<ECTEngine::Buchung^>();
    if (gruppen->Count > 0)
    {
        for each (ECTEngine::Buchung^ bd in eng->Buchungen)
        {
            System::String^ g = bd->GruppenUuid;
            if (g != nullptr && gruppen->Contains(g)
                && !selUuids->Contains(bd->Uuid))
                fehlende->Add(bd);
        }
    }

    bool bestaetigt = false;
    if (fehlende->Count > 0)
    {
        // Kaskadenlösch-Angebot: Ja = ganze Gruppe(n), Nein = nur die
        // Selektion, Abbrechen = nichts (ersetzt die alte bloße Warnung
        // des MFC-Pfads). Default = Abbrechen.
        int gesamt = loeschListe->Count + fehlende->Count;
        CString frage;
        if (loeschListe->Count == 1)
            frage.Format(
                "Die Buchung '%s' geh\366rt zu einer Buchungsgruppe.\n\n"
                "Ja = ganze Gruppe l\366schen (%d Buchungen)\n"
                "Nein = nur diese Buchung l\366schen",
                (LPCTSTR)CString(loeschListe[0]->Beschreibung), gesamt);
        else
            frage.Format(
                "Die Auswahl enth\344lt Mitglieder von Buchungsgruppen.\n\n"
                "Ja = Gruppen komplett l\366schen (%d Buchungen)\n"
                "Nein = nur die %d ausgew\344hlten Buchungen l\366schen",
                gesamt, loeschListe->Count);
        int antwort = AfxMessageBox(frage,
            MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON3);
        if (antwort == IDCANCEL) return false;
        if (antwort == IDYES) loeschListe->AddRange(fehlende);
        bestaetigt = true;   // Ja/Nein war bereits die Bestätigung
    }

    if (!bestaetigt)
    {
        // Ist die Selektion exakt eine komplette Gruppe (z.B. über das
        // Kontextmenü "Buchungsgruppe löschen"), das auch so benennen.
        bool eineKompletteGruppe = (gruppen->Count == 1);
        if (eineKompletteGruppe)
            for each (ECTEngine::Buchung^ cur in loeschListe)
                if (cur->GruppenUuid == nullptr)
                {
                    eineKompletteGruppe = false;
                    break;
                }

        CString frage;
        if (eineKompletteGruppe)
            frage.Format("Buchungsgruppe (%d Buchungen) wirklich l\366schen?",
                loeschListe->Count);
        else if (loeschListe->Count == 1)
            frage.Format("Buchung '%s' wirklich l\366schen?",
                (LPCTSTR)CString(loeschListe[0]->Beschreibung));
        else
            frage.Format("%d Buchungen wirklich l\366schen?",
                loeschListe->Count);
        if (AfxMessageBox(frage, MB_YESNO | MB_DEFBUTTON2) != IDYES)
            return false;
    }

    // 3) Entfernen (Index pro Buchung neu ermitteln, weil RemoveAt die
    //    Liste umnummeriert), ein Sync, ein SetModifiedFlag.
    for each (ECTEngine::Buchung^ b in loeschListe)
    {
        int i = FindeBuchungIdx(eng, b);
        if (i >= 0) eng->Buchungen->RemoveAt(i);
    }
    bridge->SyncManagedToNative();
    bridge->SetModifiedFlag("Buchungen ueber Journal geloescht");
    return true;
}
