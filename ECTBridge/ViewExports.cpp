// ViewExports.cpp — C++/CLI-Brücke zu den WPF-Dialogen in ECTViews
//
// Diese Datei gehört ins ECTBridge-Projekt.
// Kompiliert mit /clr (Projektstandard), OHNE Precompiled Header.
//
// Dateieigenschaften in ECTBridge.vcxproj:
//   PrecompiledHeader = NotUsing
//   CompileAsManaged  = (Standard, d.h. /clr vom Projekt)

#include "stdafx.h"
#include "ViewExports.h"
#include "EasyCashDocBridge.h"
#include "EngineHost.h"
#include "Marshalling.h"

#using "ECTEngine.dll"
#using "ECTViews.dll"

using namespace System;

// ══════════════════════════════════════════════════════════
// ECT_ShowBuchungDialog
// ══════════════════════════════════════════════════════════

BOOL ECT_ShowBuchungDialog(void* pDocBridge, BOOL bAusgaben, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        auto engine = GetEngine(bridge);

        // Vor dem Dialog: Native → Managed synchronisieren
        // (falls seit dem letzten Sync native Änderungen passiert sind)
        bridge->SyncNativeToManaged();

        // WPF-Dialog anzeigen
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        ECTEngine::Buchung^ ergebnis =
            ECTViews::ViewHost::ZeigeBuchungDialog(
                engine, bAusgaben != 0, hwnd);

        if (ergebnis == nullptr)
            return FALSE;  // Abgebrochen

        // Buchung in die Engine einfügen
        engine->Buchungen->Add(ergebnis);
        engine->InkrementBuchungszaehler();
        engine->Sort();

        // Managed → Native synchronisieren
        // (damit die Views die neue Buchung sehen)
        bridge->SyncManagedToNative();

        // Dokument als geändert markieren
        bridge->SetModifiedFlag(
            (CString)"Buchung '" + ECTBridge::ToNative(ergebnis->Beschreibung) + "' hinzugefügt");

        return TRUE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg.Format("Fehler im Buchungsdialog: %S", ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ══════════════════════════════════════════════════════════
// ECT_ShowBuchungBearbeitenDialog
// ══════════════════════════════════════════════════════════

BOOL ECT_ShowBuchungBearbeitenDialog(
    void* pDocBridge, int nBuchungIdx, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        auto engine = GetEngine(bridge);

        // Sync sicherstellen
        bridge->SyncNativeToManaged();

        if (nBuchungIdx < 0 || nBuchungIdx >= engine->Buchungen->Count)
            return FALSE;

        ECTEngine::Buchung^ original = engine->Buchungen[nBuchungIdx];

        // WPF-Dialog anzeigen
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        ECTEngine::Buchung^ geaendert =
            ECTViews::ViewHost::ZeigeBuchungBearbeitenDialog(
                engine, original, hwnd);

        if (geaendert == nullptr)
            return FALSE;  // Abgebrochen

        // Buchung im Dokument ersetzen
        engine->Buchungen[nBuchungIdx] = geaendert;
        engine->Sort();

        // Zurücksynchronisieren
        bridge->SyncManagedToNative();

        bridge->SetModifiedFlag(
            (CString)"Buchung '" + ECTBridge::ToNative(geaendert->Beschreibung) + "' geändert");

        return TRUE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg.Format("Fehler bei Buchungsbearbeitung: %S", ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}
