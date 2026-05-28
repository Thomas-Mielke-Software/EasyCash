// EasyCashDocBridge.cpp — Bridge-Dokumentenklasse, Implementierung
//
// Diese Datei wird MIT /clr kompiliert (Projektstandard).
// KEIN Precompiled Header (weil managed #using nicht mit nativem PCH geht).
// stdafx.h wird aber manuell inkludiert für MFC-Typen.
//
// Dateieigenschaften in vcxproj:
//   PrecompiledHeader = NotUsing
//   CompileAsManaged  = (Standard, d.h. /clr vom Projekt)

#include "stdafx.h"              // MFC-Typen — muss zuerst kommen
#include "EasyCashDocBridge.h"
#include "EngineHost.h"
#include "Marshalling.h"
#include "BuchungConverter.h"

#using "ECTEngine.dll"

using namespace System;
using namespace ECTEngine;
using namespace ECTBridge;

// ══════════════════════════════════════════════════════════
// MFC-Infrastruktur
// ══════════════════════════════════════════════════════════

// IMPLEMENT_DYNCREATE statt IMPLEMENT_SERIAL, weil CEasyCashDoc
// bereits IMPLEMENT_SERIAL hat und die Serialize-Logik erbt.
// CEasyCashDocBridge braucht nur DYNCREATE für die Doc-Template-
// Registrierung in EasyCash.cpp.
IMPLEMENT_DYNCREATE(CEasyCashDocBridge, CEasyCashDoc)

// ══════════════════════════════════════════════════════════
// Konstruktor / Destruktor
// ══════════════════════════════════════════════════════════

// ══════════════════════════════════════════════════════════
// Process-weite Bridge-Liste
// ══════════════════════════════════════════════════════════
//
// Eine fixe-Größe-Array in einer anonymen Namespace innerhalb von
// ECTBridge.dll. Da ECTBridge eine DLL ist (kein static lib), lebt
// dieses Array in dem einen Datensegment, das alle Aufrufer-Module
// (EasyCT.exe, EasyCTX.ocx, Plugins) gemeinsam sehen.
//
// Ersetzt das frühere m_pOwnerBridge-Feld auf CBuchung, das einen
// CBuchung-Layout-Mismatch zwischen ECTBridge.dll (neu) und der
// installierten EasyCTX.ocx (alt) erzeugt hatte.

namespace {
    const int MAX_BRIDGES = 64;
    CEasyCashDocBridge* g_bridges[MAX_BRIDGES] = { NULL };

    void RegisterBridge(CEasyCashDocBridge* p)
    {
        for (int i = 0; i < MAX_BRIDGES; i++)
            if (g_bridges[i] == NULL) { g_bridges[i] = p; return; }
        TRACE0("ECTBridge: Bridge-Slot voll -- MAX_BRIDGES erhoehen.\n");
    }

    void UnregisterBridge(CEasyCashDocBridge* p)
    {
        for (int i = 0; i < MAX_BRIDGES; i++)
            if (g_bridges[i] == p) { g_bridges[i] = NULL; return; }
    }
}

CEasyCashDocBridge::CEasyCashDocBridge()
    : CEasyCashDoc()
{
    m_pEngineHost = new ECTBridge::EngineHost();
    RegisterBridge(this);
    TRACE0("CEasyCashDocBridge: Engine-Host erstellt\n");
}

CEasyCashDocBridge::~CEasyCashDocBridge()
{
    UnregisterBridge(this);
    delete m_pEngineHost;
    m_pEngineHost = NULL;
    TRACE0("CEasyCashDocBridge: Engine-Host freigegeben\n");
}

// ══════════════════════════════════════════════════════════
// CDocument Overrides
// ══════════════════════════════════════════════════════════

void CEasyCashDocBridge::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // VOR dem Speichern: Managed → Native synchronisieren,
        // damit Änderungen aus der Engine in die nativen Linked Lists
        // zurückfließen, bevor CEasyCashDoc::Serialize sie schreibt.
        SyncManagedToNative();

        TRACE0("CEasyCashDocBridge::Serialize -- speichere (nach Sync M->N)\n");
    }

    // Das eigentliche Serialize — der gesamte native Code aus
    // CEasyCashDoc::Serialize() läuft hier unverändert durch.
    // Er liest/schreibt Magic Key, Version, Linked Lists, Zähler, etc.
    CEasyCashDoc::Serialize(ar);

    if (!ar.IsStoring())
    {
        // NACH dem Laden: Native → Managed synchronisieren,
        // damit die Engine die gerade gelesenen Daten hat.
        SyncNativeToManaged();

        TRACE("CEasyCashDocBridge::Serialize — geladen und synchronisiert\n");
    }
}

BOOL CEasyCashDocBridge::OnNewDocument()
{
    if (!CEasyCashDoc::OnNewDocument())
        return FALSE;

    // Nach dem nativen Init: leeres Dokument in die Engine übernehmen
    SyncNativeToManaged();

    TRACE("CEasyCashDocBridge::OnNewDocument — Jahr %d, Engine synchronisiert\n", nJahr);
    return TRUE;
}

BOOL CEasyCashDocBridge::OnOpenDocument(LPCTSTR lpszPathName)
{
    // CEasyCashDoc::OnOpenDocument ruft intern Serialize() auf,
    // das wiederum SyncNativeToManaged() aufruft.
    // Also brauchen wir hier keinen zusätzlichen Sync.
    return CEasyCashDoc::OnOpenDocument(lpszPathName);
}

// ══════════════════════════════════════════════════════════
// Synchronisierung: Native → Managed
// ══════════════════════════════════════════════════════════

void CEasyCashDocBridge::SyncNativeToManaged()
{
    auto engine = GetEngine(this);

    // Listen leeren + Pointer-Map leeren
    engine->Buchungen->Clear();
    engine->Dauerbuchungen->Clear();
    m_pEngineHost->ClearPointerMap();

    // Einnahmen-Linked-List → managed List + Pointer-Map
    for (CBuchung* p = Einnahmen; p != NULL; p = p->next)
    {
        ECTEngine::Buchung^ b = NativeToManaged(p, Buchungsart::Einnahme);
        engine->Buchungen->Add(b);
        m_pEngineHost->RegisterPointer(p, b);
    }

    // Ausgaben-Linked-List → managed List + Pointer-Map
    for (CBuchung* p = Ausgaben; p != NULL; p = p->next)
    {
        ECTEngine::Buchung^ b = NativeToManaged(p, Buchungsart::Ausgabe);
        engine->Buchungen->Add(b);
        m_pEngineHost->RegisterPointer(p, b);
    }

    // Dauerbuchungen-Linked-List → managed List
    LinkedListToManagedList(Dauerbuchungen, engine->Dauerbuchungen);

    // Dokument-Felder synchronisieren
    engine->Buchungszaehler             = Buchungszaehler;
    engine->LaufendeBelegnrEinnahmen    = nLaufendeBuchungsnummerFuerEinnahmen;
    engine->LaufendeBelegnrAusgaben     = nLaufendeBuchungsnummerFuerAusgaben;
    engine->LaufendeBelegnrBank         = nLaufendeBuchungsnummerFuerBank;
    engine->LaufendeBelegnrKasse        = nLaufendeBuchungsnummerFuerKasse;
    engine->Jahr                        = nJahr;
    engine->Waehrung                    = ToManaged(csWaehrung);
    engine->UrspruenglicheWaehrung      = ToManaged(csUrspruenglicheWaehrung);
    engine->GlobaleAfaGenauigkeit       = (AfaGenauigkeit)AbschreibungGenauigkeit;
    engine->DokumentVersion             = Version;
    engine->BackupNachfrageIntervallTage = nNachfrageIntervall;
    engine->BackupNachfrageTermin       = ToManagedDateTime(ctNachfrageTermin);

    // Dokument-Erweiterungen
    engine->Erweiterungen = ErweiterungStore::AusPipeFormat(
        ToManaged(Erweiterung));

    // Sortieren — die Pointer-Map bleibt korrekt, weil sie auf
    // Buchung^-Referenzen zeigt, nicht auf Indices.
    engine->Sort();

    TRACE("SyncNativeToManaged: %d Buchungen, %d Dauerbuchungen → Engine (Pointer-Map gefüllt)\n",
          engine->Buchungen->Count, engine->Dauerbuchungen->Count);
}

// ══════════════════════════════════════════════════════════
// Synchronisierung: Managed → Native
// ══════════════════════════════════════════════════════════

void CEasyCashDocBridge::SyncManagedToNative()
{
    auto engine = GetEngine(this);

    // ══════════════════════════════════════════════════════════
    // Pointer-stabiler Rebuild:
    //
    // FRÜHER: delete der gesamten Einnahmen/Ausgaben-Ketten und neue
    // CBuchungs allozieren. Das hat Plugin-/OCX-Pointer (CBuchungCtrl
    // m_ID = CBuchung*) invalidiert -- klassisches 0xDDDDDDDD beim
    // nächsten Zugriff.
    //
    // JETZT: für jede managed Buchung^ in der Engine zuerst per
    // inverser Pointer-Map prüfen, ob es schon eine native CBuchung
    // für sie gibt. Wenn ja: dieselbe Instanz wiederverwenden und
    // nur die Felder aktualisieren. Wenn nein: frisch allozieren.
    // Am Ende werden nur die nicht-mehr-referenzierten alten
    // Natives gedeletet.
    //
    // Folge: alle nativen CBuchung-Pointer, die ein Plugin gerade
    // hält, bleiben über den Sync-Zyklus hinweg gültig (sofern die
    // entsprechende managed Buchung^ noch existiert).
    // ══════════════════════════════════════════════════════════

    // 1) Inverse Map (managed Buchung^ -> native IntPtr) aus dem
    //    aktuellen Stand der Pointer-Map snapshotten.
    auto invMap = m_pEngineHost->BuildInverseMap();

    // 2) Alle alten Native-Pointer einsammeln, BEVOR der Rebuild ihre
    //    next-Felder ueberschreibt. Walken via p->next nach dem Rebuild
    //    waere unzuverlaessig, weil reused-Buchungen jetzt in die NEUE
    //    Kette zeigen statt in die alte.
    auto oldNatives = gcnew System::Collections::Generic::List<System::IntPtr>();
    for (CBuchung* p = Einnahmen; p != NULL; p = p->next)
        oldNatives->Add(System::IntPtr((void*)p));
    for (CBuchung* p = Ausgaben;  p != NULL; p = p->next)
        oldNatives->Add(System::IntPtr((void*)p));

    // Set der wiederverwendeten Natives (nicht zu deleten).
    // Dictionary<,> ist in mscorlib -- HashSet<> waere in System.Core.dll,
    // brauchten wir extra #using fuer. Bool-Wert wird nicht ausgewertet.
    auto reused = gcnew System::Collections::Generic::Dictionary<
        System::IntPtr, bool>();

    // Pointer-Map jetzt leeren -- wird im Lauf neu befuellt.
    m_pEngineHost->ClearPointerMap();

    // ── Einnahmen aufbauen ──
    {
        CBuchung* pHead = NULL;
        CBuchung* pTail = NULL;
        for each (ECTEngine::Buchung^ b in engine->Einnahmen)
        {
            // managed Buchung^ -> native CBuchung*
            // (existierend wiederverwenden, sonst frisch erzeugen).
            System::IntPtr existingPtr;
            CBuchung* p;
            if (invMap->TryGetValue(b, existingPtr))
            {
                p = (CBuchung*)existingPtr.ToPointer();
                FillNativeFromManaged(p, b);
                reused[existingPtr] = true;
            }
            else
            {
                p = ManagedToNative(b);   // setzt p->next = NULL
            }
            p->next = NULL;
            m_pEngineHost->RegisterPointer(p, b);

            if (pHead == NULL) { pHead = p; pTail = p; }
            else               { pTail->next = p; pTail = p; }
        }
        Einnahmen = pHead;
    }

    // ── Ausgaben aufbauen (identisches Muster) ──
    {
        CBuchung* pHead = NULL;
        CBuchung* pTail = NULL;
        for each (ECTEngine::Buchung^ b in engine->Ausgaben)
        {
            System::IntPtr existingPtr;
            CBuchung* p;
            if (invMap->TryGetValue(b, existingPtr))
            {
                p = (CBuchung*)existingPtr.ToPointer();
                FillNativeFromManaged(p, b);
                reused[existingPtr] = true;
            }
            else
            {
                p = ManagedToNative(b);
            }
            p->next = NULL;
            m_pEngineHost->RegisterPointer(p, b);

            if (pHead == NULL) { pHead = p; pTail = p; }
            else               { pTail->next = p; pTail = p; }
        }
        Ausgaben = pHead;
    }

    // 4) Alle alten Natives, die nicht wiederverwendet wurden, einzeln
    //    deleten. Iteration ueber die VORHER eingesammelte Liste, weil
    //    die next-Pointer der reused-Buchungen jetzt in die NEUE Kette
    //    zeigen. WICHTIG: next vor delete auf NULL setzen, damit der
    //    ~CBuchung()-Destruktor nicht noch reused-Nachbarn mitloescht.
    for each (System::IntPtr key in oldNatives)
    {
        if (!reused->ContainsKey(key))
        {
            CBuchung* p = (CBuchung*)key.ToPointer();
            p->next = NULL;
            delete p;
        }
    }

    // ── Dauerbuchungen weiterhin komplett neu aufbauen ──
    // (Plugin-Interface reicht keine CDauerbuchung*-Pointer raus,
    //  also kein Stabilitaets-Erfordernis -- siehe Konvention.)
    if (Dauerbuchungen) { delete Dauerbuchungen; Dauerbuchungen = NULL; }
    Dauerbuchungen = ManagedListToLinkedList(engine->Dauerbuchungen);

    // ── Dokument-Felder zurückschreiben ──
    Buchungszaehler                      = engine->Buchungszaehler;
    nLaufendeBuchungsnummerFuerEinnahmen  = engine->LaufendeBelegnrEinnahmen;
    nLaufendeBuchungsnummerFuerAusgaben   = engine->LaufendeBelegnrAusgaben;
    nLaufendeBuchungsnummerFuerBank       = engine->LaufendeBelegnrBank;
    nLaufendeBuchungsnummerFuerKasse      = engine->LaufendeBelegnrKasse;
    nJahr                                = engine->Jahr;
    csWaehrung                           = ToNative(engine->Waehrung);
    csUrspruenglicheWaehrung             = ToNative(engine->UrspruenglicheWaehrung);
    AbschreibungGenauigkeit              = (int)engine->GlobaleAfaGenauigkeit;
    nNachfrageIntervall                  = engine->BackupNachfrageIntervallTage;
    ctNachfrageTermin                    = ToNativeTime(engine->BackupNachfrageTermin);

    // Erweiterungen zurück ins Pipe-Format
    Erweiterung = ToNative(engine->Erweiterungen->ZuPipeFormat());

    TRACE("SyncManagedToNative: %d Buchungen aus Engine → native Linked Lists (Pointer-Map gefüllt)\n",
          engine->Buchungen->Count);
}

// ══════════════════════════════════════════════════════════
// Hinweis: GetEngine, FindManagedFor, FindManagedIndexFor sind
// inline freie Funktionen im Header (EasyCashDocBridge.h),
// nicht Member-Funktionen — wegen der dllexport/__clrcall-
// Inkompatibilität bei Methoden, die managed Typen zurückgeben.
// ══════════════════════════════════════════════════════════

// ══════════════════════════════════════════════════════════
// Mirror Native -> Managed (für OCX-Setter aus dem Plugin)
// ══════════════════════════════════════════════════════════

BOOL CEasyCashDocBridge::MirrorBuchungInEngine(CBuchung* pNative)
{
    if (!pNative || !m_pEngineHost) return FALSE;
    ECTEngine::Buchung^ b = m_pEngineHost->Lookup(pNative);
    if (b == nullptr) return FALSE;
    FillManagedFromNative(pNative, b);
    return TRUE;
}

extern "C" AFX_EXT_CLASS void ECT_SpiegleNativeBuchungInEngine(CBuchung* pNative)
{
    if (!pNative)
    {
        TRACE0("ECT_Spiegle: pNative == NULL -- skip\n");
        return;
    }
    // Process-weite Bridge-Liste iterieren -- frueher Back-Pointer auf CBuchung,
    // aber das hat das CBuchung-Layout veraendert und mit der installierten
    // EasyCTX.ocx einen Heap-Overflow erzeugt. Stattdessen jetzt die kleine
    // statische Liste hier in ECTBridge.dll.
    for (int i = 0; i < MAX_BRIDGES; i++)
    {
        CEasyCashDocBridge* pBridge = g_bridges[i];
        if (pBridge && pBridge->MirrorBuchungInEngine(pNative))
        {
            TRACE1("ECT_Spiegle: CBuchung* 0x%p gespiegelt\n", pNative);
            return;
        }
    }
    TRACE1("ECT_Spiegle: CBuchung* 0x%p in keiner Bridge gefunden -- skip\n",
           pNative);
}
