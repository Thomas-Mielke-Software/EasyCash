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

/// <summary>
/// Ermittelt den Index einer Buchung in engine->Buchungen anhand
/// ihres nativen CBuchung*-Pointers.
///
/// Vorgehen:
/// 1. Position des Pointers in der nativen Einnahmen- oder
///    Ausgaben-Linked-List ermitteln.
/// 2. Die n-te Einnahme bzw. n-te Ausgabe in engine->Buchungen finden
///    (die Listen werden beim Sync in derselben Reihenfolge befüllt).
///
/// Funktioniert nur direkt nach einem SyncNativeToManaged — wird
/// daher intern von allen pointer-basierten Funktionen vorab aufgerufen.
/// </summary>
static int FindeManagedIndex(
    CEasyCashDocBridge* bridge, CBuchung* pSuche)
{
    if (!bridge || !pSuche) return -1;
    auto engine = GetEngine(bridge);

    // Position in der nativen Liste + Art ermitteln
    int posInListe = 0;
    bool istEinnahme = false;
    bool gefunden = false;

    for (CBuchung* p = bridge->Einnahmen; p; p = p->next)
    {
        if (p == pSuche) { istEinnahme = true; gefunden = true; break; }
        posInListe++;
    }

    if (!gefunden)
    {
        posInListe = 0;
        for (CBuchung* p = bridge->Ausgaben; p; p = p->next)
        {
            if (p == pSuche) { istEinnahme = false; gefunden = true; break; }
            posInListe++;
        }
    }

    if (!gefunden) return -1;

    // Die n-te Einnahme / n-te Ausgabe in engine->Buchungen finden.
    // engine->Buchungen ist sortiert, die getrennten Linked Lists
    // wurden aber aus den sortierten Einnahmen-/Ausgaben-Teilmengen
    // aufgebaut (siehe ManagedListToLinkedList in BuchungConverter.cpp).
    auto zielArt = istEinnahme
        ? ECTEngine::Buchungsart::Einnahme
        : ECTEngine::Buchungsart::Ausgabe;

    int zaehler = 0;
    for (int i = 0; i < engine->Buchungen->Count; i++)
    {
        if (engine->Buchungen[i]->Art == zielArt)
        {
            if (zaehler == posInListe) return i;
            zaehler++;
        }
    }
    return -1;
}

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

        bridge->SyncNativeToManaged();

        int idx = FindeManagedIndex(bridge, pNative);
        if (idx < 0)
        {
            TRACE0("ECT_ShowBuchungBearbeitenDialogFuerPointer: Pointer nicht gefunden\n");
            return FALSE;
        }

        return ECT_ShowBuchungBearbeitenDialog(pDocBridge, idx, hWndOwner);
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

        auto engine = GetEngine(bridge);
        bridge->SyncNativeToManaged();

        int idx = FindeManagedIndex(bridge, pNative);
        if (idx < 0) return FALSE;

        // Vorlage klonen (nicht die Original-Referenz verwenden!)
        ECTEngine::Buchung^ vorlage = engine->Buchungen[idx]->Clone();

        // Bei "Neue Belegnummer" auf die nächste freie Nummer setzen
        if (bNeueBelegnummer)
        {
            vorlage->Belegnummer = (vorlage->Art == ECTEngine::Buchungsart::Einnahme)
                ? engine->LaufendeBelegnrEinnahmen.ToString()
                : engine->LaufendeBelegnrAusgaben.ToString();
        }

        // Dialog im Bearbeiten-Modus öffnen, aber mit der Klon-Vorlage —
        // das Ergebnis ist eine NEUE Buchung, nicht ein Ersatz.
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        ECTEngine::Buchung^ neu =
            ECTViews::ViewHost::ZeigeBuchungBearbeitenDialog(
                engine, vorlage, hwnd);

        if (neu == nullptr)
            return FALSE;  // Abgebrochen

        // Als NEUE Buchung einfügen (nicht ersetzen)
        engine->Buchungen->Add(neu);
        engine->InkrementBuchungszaehler();

        // Belegnummernzähler erhöhen, wenn neu vergeben
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

        auto engine = GetEngine(bridge);
        bridge->SyncNativeToManaged();

        int idx = FindeManagedIndex(bridge, pNative);
        if (idx < 0)
        {
            TRACE0("ECT_LoescheBuchungPerPointer: Pointer nicht gefunden\n");
            return FALSE;
        }

        // Beschreibung für die Modified-Meldung VOR dem Löschen merken
        CString csBeschreibung = ECTBridge::ToNative(
            engine->Buchungen[idx]->Beschreibung);

        // Aus der Engine entfernen
        engine->Buchungen->RemoveAt(idx);

        // Native Linked Lists neu aufbauen. ACHTUNG: Der Aufrufer-Pointer
        // pNative wird dadurch ungültig! Der Aufrufer darf ihn nach diesem
        // Aufruf nicht mehr verwenden.
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
