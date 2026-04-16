// EasyCashDocBridge.h — Bridge-Dokumentenklasse für die C#-Engine
//
// Diese Header-Datei ist PUR NATIV — sie kann von bestehenden Views und
// Dialogen des EasyCash-Hauptprojekts ohne /clr-Umschaltung eingebunden
// werden. Die managed Engine wird über einen opaken Pointer (PIMPL) gekapselt.
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

#if !defined(AFX_EASYCASHDOCBRIDGE_H__INCLUDED_)
#define AFX_EASYCASHDOCBRIDGE_H__INCLUDED_

#include "EasyCashDoc.h"  // Die originale CEasyCashDoc-Klasse (aus ehemals ECTIFace)

#if !defined(AFX_EXT_CLASS)
#define AFX_EXT_CLASS __declspec(dllimport)
#endif

// Forward-Deklaration der internen Engine-Host-Klasse.
// Die tatsächliche Definition liegt in EngineHost.h (/clr).
// Native Clients sehen nur einen opaken Pointer.
struct EngineHost;

/////////////////////////////////////////////////////////////////////////////
// CEasyCashDocBridge
//
// Erbt von CEasyCashDoc und erweitert diese um eine synchronisierte
// managed Engine-Instanz (ECTEngine.BuchungsDocument in C#).
//
// Lebenszyklus:
//   - OnNewDocument()/OnOpenDocument(): nach dem nativen Serialize-Einlesen
//     wird die Engine aus den Linked-Lists befüllt (NativeToManaged).
//   - OnFileSave()/OnFileSaveAs(): vor dem nativen Serialize-Schreiben
//     werden die Linked-Lists aus der Engine synchronisiert (ManagedToNative).
//   - Während des Betriebs sind die Linked-Lists die primäre Datenquelle
//     für bestehende Views. Die Engine wird bei Bedarf synchronisiert.
//
// Bestehende Views sehen über pDoc->Einnahmen, pDoc->Ausgaben und
// pDoc->Dauerbuchungen exakt dieselben Strukturen wie bisher.
/////////////////////////////////////////////////////////////////////////////

class AFX_EXT_CLASS CEasyCashDocBridge : public CEasyCashDoc
{
public:
    DECLARE_SERIAL(CEasyCashDocBridge)

    CEasyCashDocBridge();
    virtual ~CEasyCashDocBridge();

    // ─────────────────────────────────────────────────
    // MFC-Overrides
    // ─────────────────────────────────────────────────

    virtual BOOL OnNewDocument();
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual void Serialize(CArchive& ar);
    virtual void OnCloseDocument();

    // ─────────────────────────────────────────────────
    // Synchronisation nativ ↔ managed
    // ─────────────────────────────────────────────────

    /// Kopiert alle Daten aus den nativen Linked-Lists (Einnahmen, Ausgaben,
    /// Dauerbuchungen) in die managed ECTEngine-Instanz. Wird nach dem
    /// Laden (Serialize IsLoading) aufgerufen.
    void NativeToManaged();

    /// Kopiert alle Daten aus der managed Engine zurück in die nativen
    /// Linked-Lists. Wird vor dem Speichern (Serialize IsStoring) aufgerufen.
    /// Stellt sicher, dass der native Serialize-Code die aktuellen Daten sieht.
    void ManagedToNative();

    /// Gibt TRUE zurück, wenn die Engine initialisiert ist.
    BOOL IsEngineReady() const;

    // ─────────────────────────────────────────────────
    // Opaker Engine-Zugriff
    // Für native Clients reicht dieser Pointer. Wer die managed API
    // direkt nutzen möchte, includiert EngineHost.h in einer /clr-Datei.
    // ─────────────────────────────────────────────────

    EngineHost* GetEngineHost() { return m_pEngineHost; }

    // ─────────────────────────────────────────────────
    // Convenience-API für bestehende Views (ohne /clr)
    // Diese Methoden delegieren an die managed Engine, ohne dass der
    // aufrufende Code /clr aktivieren muss.
    // ─────────────────────────────────────────────────

    /// Summe der Einnahmen in Cent. Nutzt die managed Engine.
    /// Falls Engine nicht bereit: Fallback auf native Berechnung.
    int GetEinnahmenSumme(int nMonatsFilter = 0, LPCTSTR lpszKontoFilter = "");
    int GetAusgabenSumme(int nMonatsFilter = 0, LPCTSTR lpszKontoFilter = "");
    int GetEinnahmenSummeNetto(int nMonatsFilter = 0, LPCTSTR lpszKontoFilter = "");
    int GetAusgabenSummeNetto(int nMonatsFilter = 0, LPCTSTR lpszKontoFilter = "");

private:
    // Opaker Pointer auf die managed Engine-Host-Struktur.
    // In einer /clr-Datei (EasyCashDocBridge.cpp) wird auf die echten Typen
    // zugegriffen. Native Clients sehen nur diesen Pointer.
    EngineHost* m_pEngineHost;

    // Kopier-/Zuweisungs-Konstruktor unterbinden (wegen PIMPL und gcroot)
    CEasyCashDocBridge(const CEasyCashDocBridge&);
    CEasyCashDocBridge& operator=(const CEasyCashDocBridge&);
};

#endif // !defined(AFX_EASYCASHDOCBRIDGE_H__INCLUDED_)
