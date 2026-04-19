// NativeInit.cpp — MFC-Extension-Initialisierung für ECTBridge (C++/CLI)
//
// Warum diese Datei existiert:
// ─────────────────────────────
// ECTBridge ist eine Mixed-Mode-DLL (/clr) und gleichzeitig eine MFC-Extension-DLL.
// Die CLR installiert ihre eigene DllMain und warnt ausdrücklich davor, sie zu
// überschreiben (Loader-Lock-Gefahr). Eine eigene DllMain würde außerdem beim
// Linken mit _DllMain@12-Mehrfachdefinitionen kollidieren.
//
// Lösung: Wir initialisieren das MFC-Extension-Modul nicht in DllMain, sondern
// im Konstruktor einer globalen Instanz, die in einer rein nativen
// Übersetzungseinheit liegt. Der C++-Runtime führt globale Konstruktoren nach
// Abschluss der Loader-Lock-Phase aus — exakt der sichere Zeitpunkt, der auch
// für "DllMain-Ersatz" in /clr-DLLs empfohlen wird.
//
// Wichtig: Die gesamte Datei muss nativ kompiliert werden, sonst schlagen
// MFC-Makros wie DECLARE_DYNCREATE im managed Kontext fehl. Deshalb der
// #pragma managed(push, off)-Block.

#include "stdafx.h"
#include <afxdllx.h>

HINSTANCE hInst;
char LocaleDezimalkomma = ',';	// für Dezimalpunkt und Tausendertrenner aus Betriebssystem geholt
char LocaleTausendertrenner = '.';
char LocaleZifferngruppierung = '3';
char LocaleMinuszeichen = '-';

namespace ECTBridgeNative
{
    // Das MFC-Extension-Modul. Muss global (bzw. static file-scope) bleiben,
    // weil AfxInitExtensionModule/AfxTermExtensionModule es referenzieren
    // und MFC sich die Adresse merkt.
    static AFX_EXTENSION_MODULE g_ExtensionModule = { FALSE, nullptr };

    /// <summary>
    /// Globaler Initializer. Der Konstruktor läuft automatisch, wenn die
    /// CRT die DLL lädt und ihre globalen Konstruktoren aufruft — das passiert
    /// NACH der CLR-DllMain und NACH der Freigabe des Loader-Locks.
    ///
    /// Der Destruktor räumt beim DLL-Unload auf.
    /// </summary>
    class BridgeInitializer
    {
    public:
        BridgeInitializer()
        {
            // Modul-Handle bestimmen. GetModuleHandleEx mit Adresse einer eigenen
            // Funktion/Variable ist zuverlässiger als GetModuleHandle mit einem
            // festverdrahteten DLL-Namen, weil der Modulname sich bei Rename
            // des Binaries ändern könnte.
            HMODULE hModule = nullptr;
            ::GetModuleHandleExW(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCWSTR>(&g_ExtensionModule),
                &hModule);

            if (!hModule)
            {
                // Sollte nie passieren — defensiv behandeln
                ::OutputDebugStringA(
                    "ECTBridge: Konnte eigenes Modul-Handle nicht ermitteln.\n");
                return;
            }

            TRACE0("ECTBridge.DLL: Initialisiere MFC-Extension-Modul\n");

            if (!AfxInitExtensionModule(g_ExtensionModule, hModule))
            {
                ::OutputDebugStringA(
                    "ECTBridge: AfxInitExtensionModule fehlgeschlagen.\n");
                return;
            }

            hInst = hModule;
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, &LocaleDezimalkomma, 1);
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONTHOUSANDSEP, &LocaleTausendertrenner, 1);
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONGROUPING, &LocaleZifferngruppierung, 1);
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SNEGATIVESIGN, &LocaleMinuszeichen, 1);

            // Neue CDynLinkLibrary registrieren, damit der aufrufende MFC-
            // Prozess (EasyCT.exe) unsere seriellen Klassen per Klassenname
            // auflösen kann — essentiell für CArchive-Deserialisierung.
            new CDynLinkLibrary(g_ExtensionModule);

            // Hier können weitere native Init-Schritte folgen, z.B.:
            //   - Registrieren eigener Plugin-Hooks
            //   - Vorbereiten nativer Datenstrukturen
            //   - Logging-Setup
        }

        ~BridgeInitializer()
        {
            TRACE0("ECTBridge.DLL: Terminiere MFC-Extension-Modul\n");
            AfxTermExtensionModule(g_ExtensionModule);
        }
    };

    // Die eine globale Instanz. Ihr Konstruktor/Destruktor triggert die
    // Initialisierung/Aufräumung. Sie ist `static`, damit der Name die
    // Übersetzungseinheit nicht verlässt.
    static BridgeInitializer g_Initializer;
}
