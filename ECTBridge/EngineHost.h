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

namespace ECTBridge
{
    class EngineHost
    {
    public:
        EngineHost()
        {
            m_engine = gcnew ECTEngine::BuchungsDocument();
        }

        ~EngineHost()
        {
            m_engine = nullptr;
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

    private:
        gcroot<ECTEngine::BuchungsDocument^> m_engine;
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
