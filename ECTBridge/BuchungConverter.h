// BuchungConverter.h — Konvertierung zwischen nativen und managed Buchungstypen
//
// Wird NUR aus /clr-kompilierten Dateien inkludiert.
// Konvertiert:
//   CBuchung*       (native Linked List)  ↔  List<Buchung^>
//   CDauerbuchung*  (native Linked List)  ↔  List<Dauerbuchung^>
//   CString Erweiterung (Pipe-Format)     ↔  ErweiterungStore^

#pragma once
#pragma managed(push, on)

#using "ECTEngine.dll"

#include "EasyCashDoc.h"  // für CBuchung, CDauerbuchung, CBetrag

namespace ECTBridge
{
    // ──────────────────────────────────────────────
    // Native → Managed
    // ──────────────────────────────────────────────

    /// <summary>
    /// Konvertiert eine native CBuchung in eine managed Buchung^.
    /// Die Art (Einnahme/Ausgabe) wird als Parameter übergeben,
    /// weil sie im Original implizit durch die Listenzugehörigkeit
    /// bestimmt war.
    /// </summary>
    ECTEngine::Buchung^ NativeToManaged(CBuchung* pNative, ECTEngine::Buchungsart art);

    /// <summary>
    /// Konvertiert eine native CDauerbuchung in eine managed Dauerbuchung^.
    /// </summary>
    ECTEngine::Dauerbuchung^ NativeToManaged(CDauerbuchung* pNative);

    // ──────────────────────────────────────────────
    // Managed → Native
    // ──────────────────────────────────────────────

    /// <summary>
    /// Konvertiert eine managed Buchung^ in eine neue native CBuchung*.
    /// Der Aufrufer ist für delete verantwortlich.
    /// next-Zeiger wird auf NULL gesetzt.
    /// </summary>
    CBuchung* ManagedToNative(ECTEngine::Buchung^ managed);

    /// <summary>
    /// Konvertiert eine managed Dauerbuchung^ in eine neue native CDauerbuchung*.
    /// Der Aufrufer ist für delete verantwortlich.
    /// next-Zeiger wird auf NULL gesetzt.
    /// </summary>
    CDauerbuchung* ManagedToNative(ECTEngine::Dauerbuchung^ managed);

    // ──────────────────────────────────────────────
    // Linked List ↔ List<T> (Bulk-Konvertierung)
    // ──────────────────────────────────────────────

    /// <summary>
    /// Traversiert die native Linked List und fügt alle Buchungen
    /// als managed Objekte zur List hinzu.
    /// </summary>
    void LinkedListToManagedList(
        CBuchung* pHead,
        ECTEngine::Buchungsart art,
        System::Collections::Generic::List<ECTEngine::Buchung^>^ ziel);

    /// <summary>
    /// Traversiert die native Dauerbuchungs-Linked-List und fügt alle
    /// als managed Objekte zur List hinzu.
    /// </summary>
    void LinkedListToManagedList(
        CDauerbuchung* pHead,
        System::Collections::Generic::List<ECTEngine::Dauerbuchung^>^ ziel);

    /// <summary>
    /// Baut aus der managed List eine native Linked List auf.
    /// Gibt den Head-Pointer zurück (oder NULL bei leerer Liste).
    /// Der Aufrufer ist für delete des Head-Pointers verantwortlich
    /// (der Destruktor von CBuchung löscht rekursiv via next).
    /// </summary>
    CBuchung* ManagedListToLinkedList(
        System::Collections::Generic::IEnumerable<ECTEngine::Buchung^>^ quelle);

    /// <summary>
    /// Baut aus der managed Dauerbuchungs-List eine native Linked List auf.
    /// </summary>
    CDauerbuchung* ManagedListToLinkedList(
        System::Collections::Generic::List<ECTEngine::Dauerbuchung^>^ quelle);
}

#pragma managed(pop)
