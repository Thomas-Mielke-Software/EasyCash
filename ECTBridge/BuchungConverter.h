// BuchungConverter.h — Konvertierung zwischen CBuchung ↔ ECTEngine.Buchung
//                      und zwischen CDauerbuchung ↔ ECTEngine.Dauerbuchung
//
// Nur für /clr-kompilierte Dateien.

#pragma once

#ifndef __cplusplus_cli
#error "BuchungConverter.h darf nur in /clr-kompilierten Dateien included werden."
#endif

#include "EasyCashDoc.h"
#include "Marshalling.h"

#using "ECTEngine.dll"

namespace ECTBridge
{
    // ─────────────────────────────────────────────────
    // CBuchung ↔ ECTEngine.Buchung
    // ─────────────────────────────────────────────────

    /// <summary>
    /// Konvertiert eine native CBuchung in eine managed ECTEngine.Buchung.
    /// Die Buchungsart muss explizit übergeben werden, da sie im Original
    /// implizit durch die Zugehörigkeit zur Einnahmen/Ausgaben-Linked-List
    /// bestimmt ist.
    /// </summary>
    ECTEngine::Buchung^ ConvertToManaged(
        const CBuchung& native,
        ECTEngine::Buchungsart art);

    /// <summary>
    /// Füllt eine native CBuchung mit Daten aus einer managed ECTEngine.Buchung.
    /// Der next-Zeiger wird nicht berührt (entspricht CBuchung::operator=).
    /// </summary>
    void FillFromManaged(
        CBuchung& native,
        ECTEngine::Buchung^ managed);

    /// <summary>
    /// Erstellt eine neue native CBuchung aus einer managed Instanz.
    /// Der Aufrufer ist für die Speicherverwaltung verantwortlich.
    /// </summary>
    CBuchung* CreateNative(ECTEngine::Buchung^ managed);

    // ─────────────────────────────────────────────────
    // CDauerbuchung ↔ ECTEngine.Dauerbuchung
    // ─────────────────────────────────────────────────

    ECTEngine::Dauerbuchung^ ConvertToManaged(const CDauerbuchung& native);
    void FillFromManaged(CDauerbuchung& native, ECTEngine::Dauerbuchung^ managed);
    CDauerbuchung* CreateNative(ECTEngine::Dauerbuchung^ managed);

    // ─────────────────────────────────────────────────
    // Linked-List ↔ List<T>
    // ─────────────────────────────────────────────────

    /// <summary>
    /// Gibt die komplette Einnahmen/Ausgaben/Dauerbuchungen-Linked-List frei.
    /// Nach dem Aufruf ist der übergebene Pointer ungültig.
    /// Nutzt die im CBuchung-Destruktor bereits implementierte rekursive Freigabe.
    /// </summary>
    template<typename T>
    inline void FreeLinkedList(T*& pHead)
    {
        if (pHead)
        {
            delete pHead;  // rekursiver delete via ~T() { if(next) delete next; }
            pHead = nullptr;
        }
    }
}
