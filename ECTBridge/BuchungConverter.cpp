// BuchungConverter.cpp — Konvertierung native CBuchung ↔ managed Buchung
//
// Diese Datei wird MIT /clr kompiliert (Projektstandard).
// KEIN Precompiled Header (weil #using nicht mit nativem PCH geht).
//
// Dateieigenschaften in vcxproj:
//   PrecompiledHeader = NotUsing
//   CompileAsManaged  = (Standard, d.h. /clr vom Projekt)

#include "stdafx.h"
#include "Marshalling.h"
#include "BuchungConverter.h"

#using "ECTEngine.dll"

using namespace System;
using namespace System::Collections::Generic;
using namespace ECTEngine;
using namespace ECTBridge;

// ══════════════════════════════════════════════════════════
// Native → Managed
// ══════════════════════════════════════════════════════════

ECTEngine::Buchung^ ECTBridge::NativeToManaged(
    CBuchung* p, ECTEngine::Buchungsart art)
{
    if (!p) return nullptr;

    ECTEngine::Buchung^ b = gcnew ECTEngine::Buchung();

    // Betrag + MWSt (aus CBetrag-Basisklasse)
    b->BruttoBetrag = ECTEngine::Betrag::AusCent(p->Betrag, p->MWSt);

    // Buchungsart (war implizit durch Einnahmen/Ausgaben-Liste)
    b->Art = art;

    // Kernfelder
    b->Datum              = ToManagedDateTime(p->Datum);
    b->Beschreibung       = ToManaged(p->Beschreibung);
    b->Konto              = ToManaged(p->Konto);
    b->Belegnummer        = ToManaged(p->Belegnummer);

    // Abschreibung
    b->AfaNr              = p->AbschreibungNr;
    b->AfaJahre           = p->AbschreibungJahre;
    b->AfaRestwertCent    = p->AbschreibungRestwert;
    b->AfaDegressiv       = (p->AbschreibungDegressiv != 0);
    b->AfaSatz            = p->AbschreibungSatz;
    b->AfaGenauigkeit     = (ECTEngine::AfaGenauigkeit)p->AbschreibungGenauigkeit;

    // Zuordnungen (seit v11)
    b->Bestandskonto      = ToManaged(p->Bestandskonto);
    b->Betrieb            = ToManaged(p->Betrieb);

    // Erweiterungen: Pipe-Format parsen
    b->Erweiterungen = ECTEngine::ErweiterungStore::AusPipeFormat(
        ToManaged(p->Erweiterung));

    return b;
}

ECTEngine::Dauerbuchung^ ECTBridge::NativeToManaged(CDauerbuchung* p)
{
    if (!p) return nullptr;

    ECTEngine::Dauerbuchung^ db = gcnew ECTEngine::Dauerbuchung();

    // Betrag
    db->BruttoBetrag      = ECTEngine::Betrag::AusCent(p->Betrag, p->MWSt);

    // Kernfelder
    db->Art               = (ECTEngine::Buchungsart)p->Buchungstyp;
    db->Beschreibung      = ToManaged(p->Beschreibung);
    db->Intervall         = (ECTEngine::Intervall)p->Intervall;
    db->VonDatum          = ToManagedDateTime(p->VonDatum);
    db->BisDatum          = ToManagedDateTime(p->BisDatum);
    db->Buchungstag       = p->Buchungstag;
    db->AusgefuehrtBis    = ToManagedDateTime(p->AktualisiertBisDatum);
    db->Konto             = ToManaged(p->Konto);
    db->Belegnummer       = ToManaged(p->Belegnummer);

    // Zuordnungen (seit v12)
    db->Bestandskonto     = ToManaged(p->Bestandskonto);
    db->Betrieb           = ToManaged(p->Betrieb);

    // Erweiterungen
    db->Erweiterungen = ECTEngine::ErweiterungStore::AusPipeFormat(
        ToManaged(p->Erweiterung));

    return db;
}

// ══════════════════════════════════════════════════════════
// Managed → Native
// ══════════════════════════════════════════════════════════

CBuchung* ECTBridge::ManagedToNative(ECTEngine::Buchung^ b)
{
    if (b == nullptr) return NULL;

    CBuchung* p = new CBuchung();

    // Betrag + MWSt
    p->Betrag                   = b->BruttoBetrag.InCent;
    p->MWSt                     = b->BruttoBetrag.MwstPromille;

    // Kernfelder
    p->Datum                    = ToNativeTime(b->Datum);
    p->Beschreibung             = ToNative(b->Beschreibung);
    p->Konto                    = ToNative(b->Konto);
    p->Belegnummer              = ToNative(b->Belegnummer);

    // Abschreibung
    p->AbschreibungNr           = b->AfaNr;
    p->AbschreibungJahre        = b->AfaJahre;
    p->AbschreibungRestwert     = b->AfaRestwertCent;
    p->AbschreibungDegressiv    = b->AfaDegressiv ? TRUE : FALSE;
    p->AbschreibungSatz         = b->AfaSatz;
    p->AbschreibungGenauigkeit  = (int)b->AfaGenauigkeit;

    // Zuordnungen
    p->Bestandskonto            = ToNative(b->Bestandskonto);
    p->Betrieb                  = ToNative(b->Betrieb);

    // Erweiterungen: zurück ins Pipe-Format
    p->Erweiterung              = ToNative(b->Erweiterungen->ZuPipeFormat());

    // next = NULL (wird vom Aufrufer verkettet)
    p->next = NULL;

    return p;
}

CDauerbuchung* ECTBridge::ManagedToNative(ECTEngine::Dauerbuchung^ db)
{
    if (db == nullptr) return NULL;

    CDauerbuchung* p = new CDauerbuchung();

    // Betrag
    p->Betrag                   = db->BruttoBetrag.InCent;
    p->MWSt                     = db->BruttoBetrag.MwstPromille;

    // Kernfelder
    p->Buchungstyp              = (int)db->Art;
    p->Beschreibung             = ToNative(db->Beschreibung);
    p->Intervall                = (int)db->Intervall;
    p->VonDatum                 = ToNativeTime(db->VonDatum);
    p->BisDatum                 = ToNativeTime(db->BisDatum);
    p->Buchungstag              = db->Buchungstag;
    p->AktualisiertBisDatum     = ToNativeTime(db->AusgefuehrtBis);
    p->Konto                    = ToNative(db->Konto);
    p->Belegnummer              = ToNative(db->Belegnummer);

    // Zuordnungen
    p->Bestandskonto            = ToNative(db->Bestandskonto);
    p->Betrieb                  = ToNative(db->Betrieb);

    // Erweiterungen
    p->Erweiterung              = ToNative(db->Erweiterungen->ZuPipeFormat());

    // next = NULL
    p->next = NULL;

    return p;
}

// ══════════════════════════════════════════════════════════
// Bulk: Linked List → List<T>
// ══════════════════════════════════════════════════════════

void ECTBridge::LinkedListToManagedList(
    CBuchung* pHead,
    ECTEngine::Buchungsart art,
    List<ECTEngine::Buchung^>^ ziel)
{
    for (CBuchung* p = pHead; p != NULL; p = p->next)
    {
        ziel->Add(NativeToManaged(p, art));
    }
}

void ECTBridge::LinkedListToManagedList(
    CDauerbuchung* pHead,
    List<ECTEngine::Dauerbuchung^>^ ziel)
{
    for (CDauerbuchung* p = pHead; p != NULL; p = p->next)
    {
        ziel->Add(NativeToManaged(p));
    }
}

// ══════════════════════════════════════════════════════════
// Bulk: List<T> → Linked List
// ══════════════════════════════════════════════════════════

CBuchung* ECTBridge::ManagedListToLinkedList(
    System::Collections::Generic::IEnumerable<ECTEngine::Buchung^>^ quelle)
{
    CBuchung* pHead = NULL;
    CBuchung* pTail = NULL;

    for each (ECTEngine::Buchung^ b in quelle)
    {
        CBuchung* pNeu = ManagedToNative(b);

        if (pHead == NULL)
        {
            pHead = pNeu;
            pTail = pNeu;
        }
        else
        {
            pTail->next = pNeu;
            pTail = pNeu;
        }
    }

    return pHead;
}

CDauerbuchung* ECTBridge::ManagedListToLinkedList(
    List<ECTEngine::Dauerbuchung^>^ quelle)
{
    CDauerbuchung* pHead = NULL;
    CDauerbuchung* pTail = NULL;

    for each (ECTEngine::Dauerbuchung^ db in quelle)
    {
        CDauerbuchung* pNeu = ManagedToNative(db);

        if (pHead == NULL)
        {
            pHead = pNeu;
            pTail = pNeu;
        }
        else
        {
            pTail->next = pNeu;
            pTail = pNeu;
        }
    }

    return pHead;
}
