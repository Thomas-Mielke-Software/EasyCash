// EasyCashDocBridge.cpp — Implementierung der Bridge-Dokumentenklasse
//
// Kompilierflag: /clr (Common Language Runtime Support)
//
// Diese Datei koordiniert die Synchronisation zwischen den nativen
// Linked-Lists des CEasyCashDoc und der managed ECTEngine-Instanz.

#include "stdafx.h"
#include "EasyCashDocBridge.h"
#include "EngineHost.h"
#include "BuchungConverter.h"
#include "Marshalling.h"

using namespace ECTBridge;

// ─────────────────────────────────────────────────
// MFC-Serialisierung: muss auf v143 registriert werden,
// damit der DocTemplate die Klasse kennt
// ─────────────────────────────────────────────────

IMPLEMENT_SERIAL(CEasyCashDocBridge, CEasyCashDoc, VERSIONABLE_SCHEMA | VERSION)

// ─────────────────────────────────────────────────
// Konstruktor / Destruktor
// ─────────────────────────────────────────────────

CEasyCashDocBridge::CEasyCashDocBridge()
    : CEasyCashDoc()
    , m_pEngineHost(nullptr)
{
    m_pEngineHost = new EngineHost();
}

CEasyCashDocBridge::~CEasyCashDocBridge()
{
    delete m_pEngineHost;
    m_pEngineHost = nullptr;
    // Linked-Lists werden von ~CEasyCashDoc() freigegeben
}

// ─────────────────────────────────────────────────
// OnNewDocument — Engine für neues Dokument initialisieren
// ─────────────────────────────────────────────────

BOOL CEasyCashDocBridge::OnNewDocument()
{
    if (!CEasyCashDoc::OnNewDocument())
        return FALSE;

    // Frische Engine für das neue Dokument
    m_pEngineHost->Engine = gcnew ECTEngine::BuchungsDocument();
    m_pEngineHost->Engine->Jahr = this->nJahr;
    m_pEngineHost->Engine->Waehrung = ToManaged(this->csWaehrung);
    m_pEngineHost->Engine->UrspruenglicheWaehrung =
        ToManaged(this->csUrspruenglicheWaehrung);
    m_pEngineHost->Engine->GlobaleAfaGenauigkeit =
        (ECTEngine::AfaGenauigkeit)this->AbschreibungGenauigkeit;
    m_pEngineHost->Engine->DokumentVersion = this->Version;

    return TRUE;
}

// ─────────────────────────────────────────────────
// OnOpenDocument — nach dem Laden synchronisieren
// ─────────────────────────────────────────────────

BOOL CEasyCashDocBridge::OnOpenDocument(LPCTSTR lpszPathName)
{
    // CEasyCashDoc::OnOpenDocument ruft intern Serialize() auf.
    // Unser überschriebenes Serialize() synchronisiert automatisch.
    if (!CEasyCashDoc::OnOpenDocument(lpszPathName))
        return FALSE;

    // Nach dem Öffnen ist die Engine über Serialize() bereits befüllt.
    return TRUE;
}

// ─────────────────────────────────────────────────
// Serialize — das Herzstück der Bridge
// ─────────────────────────────────────────────────

void CEasyCashDocBridge::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // 1. Vor dem Schreiben: managed Daten in die nativen
        //    Linked-Lists zurückspielen.
        ManagedToNative();
    }

    // 2. Der existierende native Serialize-Code liest/schreibt wie bisher.
    //    Er kennt das CArchive-Binärformat inkl. aller Versionsweichen
    //    (v1 bis v13) und sollte nicht angefasst werden.
    CEasyCashDoc::Serialize(ar);

    if (ar.IsLoading())
    {
        // 3. Nach dem Laden: Engine aus den nativen Linked-Lists befüllen.
        NativeToManaged();
    }
}

void CEasyCashDocBridge::OnCloseDocument()
{
    // Engine freigeben — gcroot<> gibt die Wurzelreferenz frei,
    // der GC kümmert sich um den Rest.
    if (m_pEngineHost)
    {
        m_pEngineHost->Engine = nullptr;
    }

    CEasyCashDoc::OnCloseDocument();
}

// ─────────────────────────────────────────────────
// Engine-Zugriff
// ─────────────────────────────────────────────────

BOOL CEasyCashDocBridge::IsEngineReady() const
{
    return m_pEngineHost != nullptr
        && static_cast<ECTEngine::BuchungsDocument^>(m_pEngineHost->Engine) != nullptr;
}

// ─────────────────────────────────────────────────
// NativeToManaged — nach dem Laden Linked-Lists → List<T>
// ─────────────────────────────────────────────────

void CEasyCashDocBridge::NativeToManaged()
{
    ECTEngine::BuchungsDocument^ engine = m_pEngineHost->Engine;
    if (engine == nullptr)
    {
        engine = gcnew ECTEngine::BuchungsDocument();
        m_pEngineHost->Engine = engine;
    }

    engine->Buchungen->Clear();
    engine->Dauerbuchungen->Clear();

    // Dokumentmetadaten
    engine->Jahr                    = this->nJahr;
    engine->Waehrung                = ToManaged(this->csWaehrung);
    engine->UrspruenglicheWaehrung  = ToManaged(this->csUrspruenglicheWaehrung);
    engine->GlobaleAfaGenauigkeit   = (ECTEngine::AfaGenauigkeit)this->AbschreibungGenauigkeit;
    engine->DokumentVersion         = this->Version;
    engine->Buchungszaehler         = this->Buchungszaehler;

    engine->LaufendeBelegnrEinnahmen = this->nLaufendeBuchungsnummerFuerEinnahmen;
    engine->LaufendeBelegnrAusgaben  = this->nLaufendeBuchungsnummerFuerAusgaben;
    engine->LaufendeBelegnrBank      = this->nLaufendeBuchungsnummerFuerBank;
    engine->LaufendeBelegnrKasse     = this->nLaufendeBuchungsnummerFuerKasse;

    engine->BackupNachfrageIntervallTage = this->nNachfrageIntervall;
    engine->BackupNachfrageTermin        = ToManaged(this->ctNachfrageTermin);

    // Dokument-Ebene Erweiterungen
    engine->Erweiterungen = ECTEngine::ErweiterungStore::AusPipeFormat(
        ToManaged(this->Erweiterung));

    // Einnahmen-Linked-List → List<Buchung>
    for (CBuchung* p = this->Einnahmen; p != nullptr; p = p->next)
    {
        engine->Buchungen->Add(
            ConvertToManaged(*p, ECTEngine::Buchungsart::Einnahme));
    }

    // Ausgaben-Linked-List → List<Buchung>
    for (CBuchung* p = this->Ausgaben; p != nullptr; p = p->next)
    {
        engine->Buchungen->Add(
            ConvertToManaged(*p, ECTEngine::Buchungsart::Ausgabe));
    }

    // Dauerbuchungen-Linked-List → List<Dauerbuchung>
    for (CDauerbuchung* p = this->Dauerbuchungen; p != nullptr; p = p->next)
    {
        engine->Dauerbuchungen->Add(ConvertToManaged(*p));
    }

    // Sortierung herstellen
    engine->Sort();
}

// ─────────────────────────────────────────────────
// ManagedToNative — vor dem Speichern List<T> → Linked-Lists
// ─────────────────────────────────────────────────

void CEasyCashDocBridge::ManagedToNative()
{
    ECTEngine::BuchungsDocument^ engine = m_pEngineHost->Engine;
    if (engine == nullptr)
        return;  // nichts zu synchronisieren

    // Dokumentmetadaten zurückschreiben
    this->nJahr                   = engine->Jahr;
    this->csWaehrung              = ToNative(engine->Waehrung);
    this->csUrspruenglicheWaehrung = ToNative(engine->UrspruenglicheWaehrung);
    this->AbschreibungGenauigkeit = (int)engine->GlobaleAfaGenauigkeit;
    this->Version                 = engine->DokumentVersion;
    this->Buchungszaehler         = engine->Buchungszaehler;

    this->nLaufendeBuchungsnummerFuerEinnahmen = engine->LaufendeBelegnrEinnahmen;
    this->nLaufendeBuchungsnummerFuerAusgaben  = engine->LaufendeBelegnrAusgaben;
    this->nLaufendeBuchungsnummerFuerBank      = engine->LaufendeBelegnrBank;
    this->nLaufendeBuchungsnummerFuerKasse     = engine->LaufendeBelegnrKasse;

    this->nNachfrageIntervall = engine->BackupNachfrageIntervallTage;
    this->ctNachfrageTermin   = ToNative(engine->BackupNachfrageTermin);

    this->Erweiterung = ToNative(engine->Erweiterungen->ZuPipeFormat());

    // Alte Linked-Lists freigeben
    FreeLinkedList(this->Einnahmen);
    FreeLinkedList(this->Ausgaben);
    FreeLinkedList(this->Dauerbuchungen);

    // Buchungen neu aufbauen: Einnahmen getrennt von Ausgaben
    CBuchung** ppEinn = &this->Einnahmen;
    CBuchung** ppAusg = &this->Ausgaben;

    for each (ECTEngine::Buchung^ b in engine->Buchungen)
    {
        CBuchung* pNeu = CreateNative(b);
        if (b->Art == ECTEngine::Buchungsart::Einnahme)
        {
            *ppEinn = pNeu;
            ppEinn  = &pNeu->next;
        }
        else
        {
            *ppAusg = pNeu;
            ppAusg  = &pNeu->next;
        }
    }

    // Dauerbuchungen-Linked-List neu aufbauen
    CDauerbuchung** ppDauer = &this->Dauerbuchungen;
    for each (ECTEngine::Dauerbuchung^ d in engine->Dauerbuchungen)
    {
        CDauerbuchung* pNeu = CreateNative(d);
        *ppDauer = pNeu;
        ppDauer  = &pNeu->next;
    }
}

// ─────────────────────────────────────────────────
// Convenience-API für native Views
// ─────────────────────────────────────────────────

int CEasyCashDocBridge::GetEinnahmenSumme(int nMonatsFilter, LPCTSTR lpszKontoFilter)
{
    if (!IsEngineReady())
        return CEasyCashDoc::EinnahmenSumme(nMonatsFilter, lpszKontoFilter);

    System::String^ filter = (lpszKontoFilter && *lpszKontoFilter)
        ? gcnew System::String(lpszKontoFilter)
        : System::String::Empty;

    return m_pEngineHost->Engine->EinnahmenSumme(nMonatsFilter, filter);
}

int CEasyCashDocBridge::GetAusgabenSumme(int nMonatsFilter, LPCTSTR lpszKontoFilter)
{
    if (!IsEngineReady())
        return CEasyCashDoc::AusgabenSumme(nMonatsFilter, lpszKontoFilter);

    System::String^ filter = (lpszKontoFilter && *lpszKontoFilter)
        ? gcnew System::String(lpszKontoFilter)
        : System::String::Empty;

    return m_pEngineHost->Engine->AusgabenSumme(nMonatsFilter, filter);
}

int CEasyCashDocBridge::GetEinnahmenSummeNetto(int nMonatsFilter, LPCTSTR lpszKontoFilter)
{
    if (!IsEngineReady())
        return CEasyCashDoc::EinnahmenSummeNetto(nMonatsFilter, lpszKontoFilter);

    System::String^ filter = (lpszKontoFilter && *lpszKontoFilter)
        ? gcnew System::String(lpszKontoFilter)
        : System::String::Empty;

    return m_pEngineHost->Engine->EinnahmenSummeNetto(nMonatsFilter, filter);
}

int CEasyCashDocBridge::GetAusgabenSummeNetto(int nMonatsFilter, LPCTSTR lpszKontoFilter)
{
    if (!IsEngineReady())
        return CEasyCashDoc::AusgabenSummeNetto(nMonatsFilter, lpszKontoFilter);

    System::String^ filter = (lpszKontoFilter && *lpszKontoFilter)
        ? gcnew System::String(lpszKontoFilter)
        : System::String::Empty;

    return m_pEngineHost->Engine->AusgabenSummeNetto(nMonatsFilter, filter);
}
