// ECTIFaceStub.cpp — Minimale Forwarder-DLL
//
// Diese DLL enthält KEINEN eigenen Code. Sie existiert nur, damit
// bestehende Plugins, die gegen ECTIFace.lib gelinkt sind, beim
// Laden ihre Imports auflösen können.
//
// Alle exportierten Funktionen werden über die DEF-Datei direkt
// an ECTBridge.dll weitergeleitet (DLL Export Forwarding).
// Der Windows-Loader löst die Weiterleitung automatisch auf —
// es gibt keinen Laufzeit-Overhead.

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
