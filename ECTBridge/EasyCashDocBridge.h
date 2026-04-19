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

#include "stdafx.h"
#include "EasyCashDoc.h"
#include "EngineHost.h"

class AFX_EXT_CLASS CEasyCashDocBridge : public CEasyCashDoc
{
    DECLARE_DYNCREATE(CEasyCashDocBridge)

public:
    CEasyCashDocBridge();
    virtual ~CEasyCashDocBridge();

    virtual void Serialize(CArchive& ar) override;
    virtual BOOL OnNewDocument() override;
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;

    void SyncNativeToManaged();
    void SyncManagedToNative();

    // Engine-Host ist public, damit managed Code darauf zugreifen kann
    ECTBridge::EngineHost* m_pEngineHost;
};

// ══════════════════════════════════════════════════════════
// Engine-Zugriff
// ══════════════════════════════════════════════════════════

// ── Managed-only Hilfsfunktion (NICHT exportiert, kein dllexport) ──
#ifdef __cplusplus_cli
inline ECTEngine::BuchungsDocument^ GetEngine(CEasyCashDocBridge* pBridge)
{
    return pBridge->m_pEngineHost->GetEngine();
}
#endif