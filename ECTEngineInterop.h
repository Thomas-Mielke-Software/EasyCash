//
// ECTEngineInterop.h - C++/CLI Interop Header
//

#pragma once

#using <System.dll>
using namespace System;

namespace ECTEngineInterop {

    /// <summary>
    /// Initialisiert die ECTEngine
    /// </summary>
    bool Initialize();

    /// <summary>
    /// Erstellt ein Dokument für das Folgejahr
    /// </summary>
    /// <param name="managedDocumentHandle">GCHandle zum aktuellen Jahres-Dokument</param>
    /// <returns>GCHandle zum neuen Jahres-Dokument (oder IntPtr::Zero bei Fehler)</returns>
    IntPtr CreateNextYearDocument(IntPtr managedDocumentHandle);

    /// <summary>
    /// Gibt einen verwalteten Handle frei
    /// </summary>
    void FreeHandle(IntPtr handle);

    /// <summary>
    /// Cleanup
    /// </summary>
    void Cleanup();
}

// Initialisierung beim Programmstart
ECTEngineInterop::Initialize();

// Verwendung beim Jahreswechsel
IntPtr nextYearHandle = ECTEngineInterop::CreateNextYearDocument(currentYearHandle);

// Cleanup beim Programmende
ECTEngineInterop::Cleanup();