// EngineHost.h — Managed-Engine-Handle für CEasyCashDocBridge
//
// Kapselt den gcroot<BuchungsDocument^> so, dass der Header auch aus
// nativen Übersetzungseinheiten inkludiert werden kann (dort wird
// die Engine-Klasse als opaker Pointer behandelt).
//
// Nutzung:
//   #include "EngineHost.h"
//   // In managed Code:
//   auto doc = host.GetEngine();
//   // In nativem Code:
//   EngineHost host; // kompiliert, aber GetEngine() nicht aufrufbar

#pragma once

#ifdef __cplusplus_cli
// ── Managed-Kontext: voller Zugriff ──

#using "ECTEngine.dll"
#include <vcclr.h>

// Forward decl — vermeidet das Inkludieren der MFC-Header in
// rein managed Quellen. Die Bridge-Klasse setzt die Map später
// per Templatmethoden.
class CBuchung;

namespace ECTBridge
{
    class EngineHost
    {
    public:
        EngineHost()
        {
            m_engine = gcnew ECTEngine::BuchungsDocument();
            m_pointerMap = gcnew System::Collections::Generic::Dictionary<
                System::IntPtr, ECTEngine::Buchung^>();
            m_pointerOrder = gcnew System::Collections::Generic::List<System::IntPtr>();
        }

        ~EngineHost()
        {
            m_engine = nullptr;
            m_pointerMap = nullptr;
            m_pointerOrder = nullptr;
        }

        /// <summary>Zugriff auf die managed BuchungsDocument-Instanz.</summary>
        ECTEngine::BuchungsDocument^ GetEngine()
        {
            return m_engine;
        }

        /// <summary>Engine-Instanz austauschen (z.B. nach Jahreswechsel).</summary>
        void SetEngine(ECTEngine::BuchungsDocument^ newDoc)
        {
            m_engine = newDoc;
        }

        // ──────────────────────────────────────────────
        // Pointer-Map: native CBuchung* → managed Buchung^
        // Wird in SyncManagedToNative beim Aufbau der Linked Lists
        // gefüllt. Erlaubt O(1)-Lookup unabhängig von der Reihenfolge.
        // ──────────────────────────────────────────────

        void ClearPointerMap()
        {
            m_pointerMap->Clear();
            m_pointerOrder->Clear();
        }

        void RegisterPointer(CBuchung* pNative, ECTEngine::Buchung^ managed)
        {
            System::IntPtr key((void*)pNative);
            // gcroot<T^> hat operator->, das T^ zurückgibt.
            // Auf Dictionary den Indexer per Methode aufrufen, weil
            // gcroot kein operator[] hat.
            System::Collections::Generic::Dictionary<
                System::IntPtr, ECTEngine::Buchung^>^ map = m_pointerMap;
            map[key] = managed;
            m_pointerOrder->Add(key);
        }

        ECTEngine::Buchung^ Lookup(CBuchung* pNative)
        {
            System::IntPtr key((void*)pNative);
            ECTEngine::Buchung^ result;
            if (m_pointerMap->TryGetValue(key, result))
                return result;
            return nullptr;
        }

    private:
        gcroot<ECTEngine::BuchungsDocument^> m_engine;
        gcroot<System::Collections::Generic::Dictionary<
            System::IntPtr, ECTEngine::Buchung^>^> m_pointerMap;
        gcroot<System::Collections::Generic::List<System::IntPtr>^> m_pointerOrder;
    };
}

#else
// ── Nativer Kontext: opake Deklaration ──
// Damit EasyCashDocBridge.h auch aus rein nativem Code inkludiert
// werden kann (z.B. wenn Views den Bridge-Header brauchen, aber
// selbst nicht mit /clr kompiliert werden).

namespace ECTBridge
{
    class EngineHost;  // opak — nur als Pointer verwendbar
}

#endif // __cplusplus_cli
