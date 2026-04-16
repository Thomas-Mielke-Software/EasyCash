// EngineHost.h — Interner /clr-only Wrapper für die managed Engine
//
// Diese Header-Datei DARF NUR von /clr-kompilierten .cpp-Dateien eingebunden
// werden, niemals von nativen Client-Code. Sie enthält msclr::gcroot, das
// /clr-Kontext benötigt.
//
// Externer Zugriff geht über den opaken Pointer EngineHost* aus
// EasyCashDocBridge.h.

#pragma once

#ifndef __cplusplus_cli
#error "EngineHost.h darf nur in /clr-kompilierten Dateien included werden."
#endif

#include <msclr/gcroot.h>
#include <msclr/marshal_cppstd.h>

#using "ECTEngine.dll"

/// <summary>
/// Host-Struktur, die ein gcroot-Handle auf die managed
/// ECTEngine::BuchungsDocument-Instanz hält. Aus nativem Code wird
/// diese Struktur nur als opaker Pointer (struct EngineHost*) gesehen.
/// </summary>
struct EngineHost
{
    msclr::gcroot<ECTEngine::BuchungsDocument^> Engine;

    EngineHost()
    {
        Engine = gcnew ECTEngine::BuchungsDocument();
    }
};
