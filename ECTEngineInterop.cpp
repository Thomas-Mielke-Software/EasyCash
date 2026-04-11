//
// ECTEngineInterop.cpp - C++/CLI Interop für ECTEngine.dll
// Ermöglicht die Nutzung von .NET ECTEngine-Klassen im C++ EasyCash-Projekt
//

#include "stdafx.h"
#include "ECTEngineInterop.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace ECTEngine::Models;
using namespace ECTEngine::Calculations;

// Globale verwaltete Referenzen
static gcroot<YearTransitionEngine^> g_yearTransitionEngine = nullptr;

namespace ECTEngineInterop {

    // Initialisiert die ECTEngine
    bool Initialize()
    {
        try
        {
            if (g_yearTransitionEngine == nullptr)
            {
                g_yearTransitionEngine = gcnew YearTransitionEngine();
            }
            return true;
        }
        catch (Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("ECTEngine Init Error: " + ex->Message);
            return false;
        }
    }

    // Erstellt ein neues Dokument für das Folgejahr
    // Der Input und Output sind verwaltete GCHandles
    IntPtr CreateNextYearDocument(IntPtr managedDocumentHandle)
    {
        try
        {
            if (g_yearTransitionEngine == nullptr && !Initialize())
                return IntPtr::Zero;

            // Hole die verwaltete Referenz
            GCHandle inputHandle = GCHandle::FromIntPtr(managedDocumentHandle);
            EasyCashDocument^ currentYear = safe_cast<EasyCashDocument^>(inputHandle.Target);

            if (currentYear == nullptr)
                return IntPtr::Zero;

            // Rufe die .NET-Methode auf
            EasyCashDocument^ nextYear = g_yearTransitionEngine->CreateNextYearDocument(currentYear);

            // Erstelle einen neuen Handle für das Ergebnis
            GCHandle outputHandle = GCHandle::Alloc(nextYear);
            return GCHandle::ToIntPtr(outputHandle);
        }
        catch (Exception^ ex)
        {
            System::Diagnostics::Debug::WriteLine("CreateNextYearDocument Error: " + ex->Message);
            return IntPtr::Zero;
        }
    }

    // Gibt einen verwalteten Handle frei
    void FreeHandle(IntPtr handle)
    {
        try
        {
            if (handle != IntPtr::Zero)
            {
                GCHandle gcHandle = GCHandle::FromIntPtr(handle);
                if (gcHandle.IsAllocated)
                    gcHandle.Free();
            }
        }
        catch (...)
        {
        }
    }

    // Cleanup
    void Cleanup()
    {
        g_yearTransitionEngine = nullptr;
    }
}