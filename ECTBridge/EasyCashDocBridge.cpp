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

CEasyCashDocBridge::CEasyCashDocBridge()
    : CEasyCashDoc()
{
    m_pEngineHost = new ECTBridge::EngineHost();
    TRACE0("CEasyCashDocBridge: Engine-Host erstellt\n");
}

CEasyCashDocBridge::~CEasyCashDocBridge()
{
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

        TRACE0("CEasyCashDocBridge::Serialize — speichere (nach Sync M→N)\n");
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

    // ── Alte Linked Lists freigeben + Map leeren ──
    // CBuchung-Destruktor löscht rekursiv über next
    if (Einnahmen) { delete Einnahmen; Einnahmen = NULL; }
    if (Ausgaben)  { delete Ausgaben;  Ausgaben  = NULL; }
    if (Dauerbuchungen) { delete Dauerbuchungen; Dauerbuchungen = NULL; }
    m_pEngineHost->ClearPointerMap();

    // ── Einnahmen aufbauen + jeden Pointer in der Map registrieren ──
    {
        CBuchung* pTail = NULL;
        for each (ECTEngine::Buchung^ b in engine->Einnahmen)
        {
            CBuchung* pNeu = ManagedToNative(b);
            m_pEngineHost->RegisterPointer(pNeu, b);

            if (Einnahmen == NULL) { Einnahmen = pNeu; pTail = pNeu; }
            else                   { pTail->next = pNeu; pTail = pNeu; }
        }
    }

    // ── Ausgaben aufbauen + Map registrieren ──
    {
        CBuchung* pTail = NULL;
        for each (ECTEngine::Buchung^ b in engine->Ausgaben)
        {
            CBuchung* pNeu = ManagedToNative(b);
            m_pEngineHost->RegisterPointer(pNeu, b);

            if (Ausgaben == NULL) { Ausgaben = pNeu; pTail = pNeu; }
            else                  { pTail->next = pNeu; pTail = pNeu; }
        }
    }

    // ── Dauerbuchungen ohne Map (werden nicht über Pointer-API angesprochen) ──
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
