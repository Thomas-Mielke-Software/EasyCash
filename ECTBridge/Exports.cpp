// Exports.cpp — Implementierung der nativen C-Exports
//
// Diese Datei wird MIT /clr kompiliert (Projektstandard), weil sie
// auf die managed ECTEngine.dll zugreift.
//
// KEIN #include "stdafx.h" — diese Datei nutzt KEINEN Precompiled Header,
// weil der PCH MFC-Header enthält, die unter /clr Probleme machen können.
// In den Dateieigenschaften: "Vorkompilierter Header" = "Nicht verwenden".

// #define ECTBRIDGE_EXPORTS -- wird bereits über die Projekteinstellungen definiert
#include "Exports.h"

#include <cstdio>
#include <cstring>

// Managed-Referenz auf die C#-Engine
#using "ECTEngine.dll"

using namespace System;
using namespace System::Runtime::InteropServices;

// ──────────────────────────────────────────────
// Statischer Puffer für den Versionsstring
// (wird einmal befüllt und bleibt gültig bis DLL-Unload)
// ──────────────────────────────────────────────
static char s_versionBuffer[256] = { 0 };

// ──────────────────────────────────────────────
// ECT_BridgeVersion
// ──────────────────────────────────────────────

const char* ECT_BridgeVersion(void)
{
    if (s_versionBuffer[0] != '\0')
        return s_versionBuffer;  // nur einmal berechnen

    try
    {
        // Managed String zusammenbauen
        System::Reflection::Assembly^ engineAsm =
            ECTEngine::Betrag::typeid->Assembly;
        String^ version = engineAsm->GetName()->Version->ToString();
        String^ info = String::Format(
            "ECTBridge 1.0 | ECTEngine {0} | CLR {1}",
            version,
            Environment::Version->ToString());

        // Managed String → nativer char*
        IntPtr pNative = Marshal::StringToHGlobalAnsi(info);
        strncpy(s_versionBuffer, (const char*)pNative.ToPointer(),
                sizeof(s_versionBuffer) - 1);
        s_versionBuffer[sizeof(s_versionBuffer) - 1] = '\0';
        Marshal::FreeHGlobal(pNative);
    }
    catch (Exception^ ex)
    {
        String^ msg = "ECTBridge: Fehler - " + ex->Message;
        IntPtr pNative = Marshal::StringToHGlobalAnsi(msg);
        strncpy(s_versionBuffer, (const char*)pNative.ToPointer(),
                sizeof(s_versionBuffer) - 1);
        Marshal::FreeHGlobal(pNative);
    }

    return s_versionBuffer;
}

// ──────────────────────────────────────────────
// ECT_BridgeSelfTest
// ──────────────────────────────────────────────

int ECT_BridgeSelfTest(void)
{
    try
    {
        // Test 1: Betrag erzeugen und Netto berechnen
        // 119,00 EUR brutto bei 19% MWSt → erwartet 100,00 EUR netto
        ECTEngine::Betrag b = ECTEngine::Betrag::AusCent(11900, 19000);

        if (b.InCent != 11900)
            return 1;  // Brutto-Konvertierung fehlerhaft

        if (b.NettoInCent != 10000)
            return 2;  // Netto-Berechnung fehlerhaft

        // Test 2: ErweiterungStore Roundtrip
        ECTEngine::ErweiterungStore^ store =
            ECTEngine::ErweiterungStore::AusPipeFormat(
                "TestDLL|Key1=Wert1|Key2=Wert2||");

        if (store->Hole("TestDLL", "Key1", "") != "Wert1")
            return 3;  // Pipe-Format-Parsing fehlerhaft

        String^ roundtrip = store->ZuPipeFormat();
        if (!roundtrip->Contains("Key1=Wert1"))
            return 4;  // Pipe-Format-Serialisierung fehlerhaft

        // Test 3: BuchungsDocument erzeugen und Summe berechnen
        ECTEngine::BuchungsDocument^ doc =
            gcnew ECTEngine::BuchungsDocument();
        doc->Jahr = 2024;

        ECTEngine::Buchung^ einnahme = gcnew ECTEngine::Buchung();
        einnahme->Art = ECTEngine::Buchungsart::Einnahme;
        einnahme->BruttoBetrag = ECTEngine::Betrag::AusCent(119000, 19000);
        einnahme->Datum = DateTime(2024, 3, 15);
        einnahme->Konto = "Erlöse";
        doc->Buchungen->Add(einnahme);

        if (doc->EinnahmenSumme(0, "") != 119000)
            return 5;  // Summenberechnung fehlerhaft

        return 0;  // Alles OK
    }
    catch (Exception^)
    {
        return -1;  // Unerwartete Exception
    }
}
