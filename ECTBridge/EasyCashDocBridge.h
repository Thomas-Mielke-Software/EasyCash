// EasyCashDocBridge.h — Bridge-Dokumentenklasse
//
// CEasyCashDocBridge erbt von CEasyCashDoc und ergänzt die managed
// Synchronisierung mit ECTEngine::BuchungsDocument.
//
// Designprinzip:
//   CEasyCashDoc         = native Geschäftslogik (bleibt unverändert)
//   CEasyCashDocBridge   = synchronisiert native Daten ↔ managed Engine
//   ECTEngine            = neue C#-Geschäftslogik
//
// Die Views und Dialoge arbeiten weiterhin mit den nativen CBuchung*-
// Pointern über die geerbten Members (Einnahmen, Ausgaben, etc.).
// Parallel dazu steht die managed Engine über GetEngine() bereit,
// um neue Funktionalität in C# zu implementieren.
//
// Für rein native Übersetzungseinheiten: Dieser Header kann inkludiert
// werden, weil EngineHost.h im nativen Kontext nur eine opake Klasse
// deklariert. GetEngine() ist dann allerdings nicht aufrufbar.

#pragma once

#include "EasyCashDoc.h"
#include "EngineHost.h"

class AFX_EXT_CLASS CEasyCashDocBridge : public CEasyCashDoc
{
    DECLARE_DYNCREATE(CEasyCashDocBridge)

public:
    CEasyCashDocBridge();
    virtual ~CEasyCashDocBridge();

    // ──────────────────────────────────────────────
    // CDocument Overrides
    // ──────────────────────────────────────────────

    virtual void Serialize(CArchive& ar) override;
    virtual BOOL OnNewDocument() override;
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;

    // ──────────────────────────────────────────────
    // Synchronisierung: Native ↔ Managed
    // ──────────────────────────────────────────────

    /// <summary>
    /// Überträgt die nativen Linked Lists (Einnahmen, Ausgaben,
    /// Dauerbuchungen) und alle Doc-Felder in die managed Engine.
    /// Aufzurufen NACH dem nativen Serialize-Load.
    /// </summary>
    void SyncNativeToManaged();

    /// <summary>
    /// Überträgt die managed Engine-Daten zurück in die nativen
    /// Linked Lists und Doc-Felder.
    /// Aufzurufen VOR dem nativen Serialize-Store.
    /// </summary>
    void SyncManagedToNative();

    // ──────────────────────────────────────────────
    // EngineHost — public, weil die freien /clr-Hilfsfunktionen
    // (siehe unten) darauf zugreifen müssen. Das ist sauberer als
    // friend-Deklarationen für die einzelnen Funktionen.
    // ──────────────────────────────────────────────
public:
    ECTBridge::EngineHost* m_pEngineHost;
};

// ══════════════════════════════════════════════════════════
// Freie Inline-Funktionen für /clr-Code
// ══════════════════════════════════════════════════════════
//
// Diese Funktionen sind NICHT __declspec(dllexport), weil sie
// managed Typen zurückgeben und damit __clrcall sind. Beide
// Aufrufkonventionen sind inkompatibel.
//
// Stattdessen sind sie inline definiert, sodass jede /clr-Datei,
// die EasyCashDocBridge.h inkludiert, eine eigene Instanz bekommt.
// Da AFX_EXT_CLASS auf die Klasse selbst dort kein dllexport mehr
// erzwingt, gibt es keinen Konflikt.
//
// Wichtig: Der #ifdef __cplusplus_cli-Guard sorgt dafür, dass
// diese Funktionen nur in /clr-Übersetzungseinheiten existieren —
// rein native Quellen (z.B. dllmain.cpp) dürfen den Header
// problemlos inkludieren.

#ifdef __cplusplus_cli

inline ECTEngine::BuchungsDocument^ GetEngine(CEasyCashDocBridge* pBridge)
{
    return pBridge->m_pEngineHost->GetEngine();
}

inline ECTEngine::Buchung^ FindManagedFor(
    CEasyCashDocBridge* pBridge, CBuchung* pNative)
{
    if (!pBridge || !pNative || !pBridge->m_pEngineHost) return nullptr;
    return pBridge->m_pEngineHost->Lookup(pNative);
}

inline int FindManagedIndexFor(
    CEasyCashDocBridge* pBridge, CBuchung* pNative)
{
    auto managed = FindManagedFor(pBridge, pNative);
    if (managed == nullptr) return -1;
    return GetEngine(pBridge)->Buchungen->IndexOf(managed);
}

#endif // __cplusplus_cli
