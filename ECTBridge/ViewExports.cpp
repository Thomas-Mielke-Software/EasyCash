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

// ══════════════════════════════════════════════════════════
// Pointer-basierte API
// ══════════════════════════════════════════════════════════
//
// Lookup über die Pointer-Map in CEasyCashDocBridge. Die Map wird
// bei jedem Sync (in beide Richtungen) befüllt und enthält die
// Zuordnung CBuchung* → Buchung^. O(1)-Lookup, unabhängig von der
// Reihenfolge der Listen oder Sort-Stabilität.

// ──────────────────────────────────────────────
// ECT_ShowBuchungBearbeitenDialogFuerPointer
// ──────────────────────────────────────────────

BOOL ECT_ShowBuchungBearbeitenDialogFuerPointer(
    void* pDocBridge, CBuchung* pNative, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge || !pNative) return FALSE;

        // Direkter Lookup in der Pointer-Map. Die Map ist nach dem
        // letzten Sync aktuell — solange seitdem keine Buchungen
        // hinzugefügt/entfernt wurden, stimmt sie.
        ECTEngine::Buchung^ original = FindManagedFor(bridge, pNative);
        if (original == nullptr)
        {
            TRACE0("ECT_ShowBuchungBearbeitenDialogFuerPointer: "
                   "Pointer in Map nicht gefunden — evtl. veraltet?\n");
            return FALSE;
        }

        auto engine = GetEngine(bridge);

        // WPF-Dialog mit der gefundenen Original-Buchung anzeigen
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        ECTEngine::Buchung^ geaendert =
            ECTViews::ViewHost::ZeigeBuchungBearbeitenDialog(
                engine, original, hwnd);

        if (geaendert == nullptr)
            return FALSE;  // Abgebrochen

        // Original im Dokument durch geänderte Version ersetzen.
        // IndexOf nutzt Reference-Equality auf der Buchung^, also
        // unabhängig vom Sort-Zustand.
        int idx = engine->Buchungen->IndexOf(original);
        if (idx < 0) return FALSE;
        engine->Buchungen[idx] = geaendert;
        engine->Sort();

        bridge->SyncManagedToNative();

        bridge->SetModifiedFlag(
            (CString)"Buchung '" + ECTBridge::ToNative(geaendert->Beschreibung) + "' geändert");

        return TRUE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg.Format("Fehler in ECT_ShowBuchungBearbeitenDialogFuerPointer: %S", ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ──────────────────────────────────────────────
// ECT_ShowBuchungKopierenDialog
// ──────────────────────────────────────────────

BOOL ECT_ShowBuchungKopierenDialog(
    void* pDocBridge, CBuchung* pNative,
    BOOL bNeueBelegnummer, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge || !pNative) return FALSE;

        ECTEngine::Buchung^ originalRef = FindManagedFor(bridge, pNative);
        if (originalRef == nullptr) return FALSE;

        auto engine = GetEngine(bridge);

        // Vorlage klonen — die Original-Buchung bleibt unverändert
        ECTEngine::Buchung^ vorlage = originalRef->Clone();

        if (bNeueBelegnummer)
        {
            vorlage->Belegnummer = (vorlage->Art == ECTEngine::Buchungsart::Einnahme)
                ? engine->LaufendeBelegnrEinnahmen.ToString()
                : engine->LaufendeBelegnrAusgaben.ToString();
        }

        IntPtr hwnd = IntPtr((void*)hWndOwner);
        ECTEngine::Buchung^ neu =
            ECTViews::ViewHost::ZeigeBuchungBearbeitenDialog(
                engine, vorlage, hwnd);

        if (neu == nullptr) return FALSE;

        // Als NEUE Buchung einfügen (nicht ersetzen)
        engine->Buchungen->Add(neu);
        engine->InkrementBuchungszaehler();

        if (bNeueBelegnummer)
        {
            if (neu->Art == ECTEngine::Buchungsart::Einnahme)
                engine->LaufendeBelegnrEinnahmen++;
            else
                engine->LaufendeBelegnrAusgaben++;
        }

        engine->Sort();
        bridge->SyncManagedToNative();

        bridge->SetModifiedFlag(
            (CString)"Buchung '" + ECTBridge::ToNative(neu->Beschreibung) + "' kopiert");

        return TRUE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg.Format("Fehler in ECT_ShowBuchungKopierenDialog: %S", ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ──────────────────────────────────────────────
// ECT_LoescheBuchungPerPointer
// ──────────────────────────────────────────────

BOOL ECT_LoescheBuchungPerPointer(
    void* pDocBridge, CBuchung* pNative)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge || !pNative) return FALSE;

        ECTEngine::Buchung^ managed = FindManagedFor(bridge, pNative);
        if (managed == nullptr)
        {
            TRACE0("ECT_LoescheBuchungPerPointer: Pointer nicht in Map\n");
            return FALSE;
        }

        auto engine = GetEngine(bridge);

        // Beschreibung VOR dem Löschen merken
        CString csBeschreibung = ECTBridge::ToNative(managed->Beschreibung);

        // Aus der Engine entfernen — Reference-Equality, unabhängig vom Index
        if (!engine->Buchungen->Remove(managed))
            return FALSE;

        // Native Linked Lists neu aufbauen.
        // ACHTUNG: pNative wird dadurch ungültig.
        bridge->SyncManagedToNative();

        bridge->SetModifiedFlag(
            (CString)"Buchung '" + csBeschreibung + "' gelöscht");

        return TRUE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg.Format("Fehler in ECT_LoescheBuchungPerPointer: %S", ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ══════════════════════════════════════════════════════════
// Listen-Initialisierung
// ══════════════════════════════════════════════════════════

/// Hilfsfunktion: native LPCSTR-Array → managed List<String^>
static System::Collections::Generic::List<System::String^>^
MachListe(LPCSTR* pArray, int nCount)
{
    auto liste = gcnew System::Collections::Generic::List<System::String^>();
    for (int i = 0; i < nCount; i++)
    {
        if (pArray[i])
            liste->Add(gcnew System::String(pArray[i]));
        else
            liste->Add(System::String::Empty);
    }
    return liste;
}

void ECT_SetzeBetriebeUndBestandskonten(
    LPCSTR* pBetriebeNamen, LPCSTR* pBetriebeIcons, int nBetriebeCount,
    LPCSTR* pBestandskontenNamen, LPCSTR* pBestandskontenIcons, int nBestandskontenCount)
{
    try
    {
        ECTViews::ViewHost::BetriebeNamen =
            MachListe(pBetriebeNamen, nBetriebeCount);
        ECTViews::ViewHost::BetriebeIcons =
            MachListe(pBetriebeIcons, nBetriebeCount);
        ECTViews::ViewHost::BestandskontenNamen =
            MachListe(pBestandskontenNamen, nBestandskontenCount);
        ECTViews::ViewHost::BestandskontenIcons =
            MachListe(pBestandskontenIcons, nBestandskontenCount);

        TRACE("ECT_SetzeBetriebeUndBestandskonten: %d Betriebe, %d Bestandskonten\n",
              nBetriebeCount, nBestandskontenCount);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg.Format("Fehler in ECT_SetzeBetriebeUndBestandskonten: %S", ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}
