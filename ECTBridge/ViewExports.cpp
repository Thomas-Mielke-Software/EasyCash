// ViewExports.cpp -- C++/CLI-Brücke zu den WPF-Dialogen in ECTViews
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
#include "AfaAbgangShared.h"   // ECTBridge_FuehreAfaAbgang (geteilt mit JournalExports)
#include "BuchungenLoeschenShared.h" // gemeinsames Löschen mit Buchungsgruppen-Abfrage

#using "ECTEngine.dll"
#using "ECTViews.dll"

using namespace System;

// Markiert Buchungen im offenen WPF-Journal (Buchungsgruppe als Block;
// das Journal scrollt so, dass moeglichst viele Mitglieder sichtbar sind.
// Der Selektions-Merker im JournalViewModel uebersteht den nachfolgenden
// Journal-Refresh des nativen Aufrufers).
static void ECTBridge_SelektiereImJournal(
    System::Collections::Generic::IReadOnlyList<ECTEngine::Buchung^>^ buchungen)
{
    if (buchungen == nullptr || buchungen->Count == 0) return;
    auto uuids = gcnew System::Collections::Generic::List<System::Guid>();
    for each (ECTEngine::Buchung^ b in buchungen)
        uuids->Add(b->Uuid);
    ECTViews::Journal::JournalEmbed::SelektiereBuchungen(uuids);
}

// ----------------------------------------------------------
// Handler fuer "Buchen & naechste": persistiert eine Buchung,
// ohne den Dialog zu schliessen. Ueber eine ref class, weil
// C++/CLI-Lambdas keine managed Delegates bedienen koennen
// (siehe CLAUDE.md).
// ----------------------------------------------------------
namespace ECTBridge
{
    ref class BuchenWeiterHandler
    {
    public:
        System::IntPtr m_pBridge;

        void OnBuchenUndNaechste(System::Collections::Generic::IReadOnlyList<ECTEngine::Buchung^>^ buchungen)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            if (!bridge || buchungen == nullptr || buchungen->Count == 0) return;

            auto engine = GetEngine(bridge);
            // Bei Buchungsgruppen kommen mehrere Buchungen auf einmal
            // (Basis + Zusatz-Zeilen, per Gruppen-UUID verknüpft).
            for each (ECTEngine::Buchung^ buchung in buchungen)
            {
                engine->Buchungen->Add(buchung);
                engine->InkrementBuchungszaehler();
            }
            engine->Sort();

            bridge->SyncManagedToNative();
            CString csMeldung = (CString)"Buchung '"
                + ECTBridge::ToNative(buchungen[0]->Beschreibung) + "' hinzugefügt";
            if (buchungen->Count > 1)
            {
                CString csZusatz;
                csZusatz.Format(" (Buchungsgruppe, %d Buchungen)", buchungen->Count);
                csMeldung += csZusatz;
            }
            bridge->SetModifiedFlag(csMeldung);

            // Offenes WPF-Journal sofort nachziehen.
            ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();

            // Buchungsgruppe als Block markieren
            if (buchungen->Count > 1)
                ECTBridge_SelektiereImJournal(buchungen);
        }
    };
}

// ----------------------------------------------------------
// ECT_ShowBuchungDialog
// ----------------------------------------------------------

static BOOL ECTBridge_ShowBuchungDialogIntern(
    void* pDocBridge, BOOL bAusgaben, HWND hWndOwner, int nVorlagenSlot)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        auto engine = GetEngine(bridge);

        // Vor dem Dialog: Native --> Managed synchronisieren
        // (falls seit dem letzten Sync native Änderungen passiert sind)
        bridge->SyncNativeToManaged();

        // WPF-Dialog anzeigen. Der "Buchen & naechste"-Callback
        // persistiert Zwischenbuchungen, ohne den Dialog zu schliessen.
        IntPtr hwnd = IntPtr((void*)hWndOwner);

        auto weiterHandler = gcnew ECTBridge::BuchenWeiterHandler();
        weiterHandler->m_pBridge = IntPtr(pDocBridge);
        auto weiterCb = gcnew System::Action<System::Collections::Generic::IReadOnlyList<ECTEngine::Buchung^>^>(
            weiterHandler, &ECTBridge::BuchenWeiterHandler::OnBuchenUndNaechste);

        // Ergebnis ist eine LISTE: bei Buchungsgruppen-Vorlagen Basis +
        // Zusatz-Zeilen (per Gruppen-UUID verknüpft), sonst genau eine.
        System::Collections::Generic::IReadOnlyList<ECTEngine::Buchung^>^ ergebnis =
            ECTViews::ViewHost::ZeigeBuchungDialog(
                engine, bAusgaben != 0, hwnd, weiterCb, nVorlagenSlot);

        if (ergebnis == nullptr || ergebnis->Count == 0)
            return FALSE;  // Abgebrochen

        // Buchung(en) in die Engine einfügen
        for each (ECTEngine::Buchung^ b in ergebnis)
        {
            engine->Buchungen->Add(b);
            engine->InkrementBuchungszaehler();
        }
        engine->Sort();

        // Managed --> Native synchronisieren
        // (damit die Views die neuen Buchungen sehen)
        bridge->SyncManagedToNative();

        // Dokument als geändert markieren
        CString csMeldung = (CString)"Buchung '"
            + ECTBridge::ToNative(ergebnis[0]->Beschreibung) + "' hinzugefügt";
        if (ergebnis->Count > 1)
        {
            CString csZusatz;
            csZusatz.Format(" (Buchungsgruppe, %d Buchungen)", ergebnis->Count);
            csMeldung += csZusatz;
        }
        bridge->SetModifiedFlag(csMeldung);

        // Buchungsgruppe im offenen WPF-Journal als Block markieren
        if (ergebnis->Count > 1)
        {
            ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
            ECTBridge_SelektiereImJournal(ergebnis);
        }

        return TRUE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler im Buchungsdialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

BOOL ECT_ShowBuchungDialog(void* pDocBridge, BOOL bAusgaben, HWND hWndOwner)
{
    return ECTBridge_ShowBuchungDialogIntern(pDocBridge, bAusgaben, hWndOwner, -1);
}

BOOL ECT_ShowBuchungDialogMitVorlage(
    void* pDocBridge, BOOL bAusgaben, HWND hWndOwner, int nVorlagenSlot)
{
    return ECTBridge_ShowBuchungDialogIntern(
        pDocBridge, bAusgaben, hWndOwner, nVorlagenSlot);
}

// ----------------------------------------------------------
// ECT_ShowBuchungBearbeitenDialog
// ----------------------------------------------------------

// Gemeinsamer Kern der beiden Bearbeiten-Exports: öffnet den Kombi-Dialog
// (Gruppen-Bearbeitung, wenn die Buchung Mitglied einer Buchungsgruppe mit
// auffindbarer Vorlage ist; sonst Einzel-Bearbeitung mit "Abgang buchen")
// und ersetzt die Buchung(en) im Dokument. Bei Gruppen werden die alten
// Zusatz-Mitglieder entfernt und durch die neu berechneten ersetzt; die
// Gruppen-UUID bleibt erhalten (ViewModel).
static BOOL ECTBridge_BearbeiteBuchung(CEasyCashDocBridge* bridge,
    ECTEngine::Buchung^ angeklickt, HWND hWndOwner)
{
    auto engine = GetEngine(bridge);
    IntPtr hwnd = IntPtr((void*)hWndOwner);

    auto erg = ECTViews::ViewHost::ZeigeBuchungBearbeitenKombiDialog(
        engine, angeklickt, hwnd);

    // "Abgang buchen": gleiche AfA-Abgang-Logik wie der Journal-
    // Kontextmenue-Eintrag (nur im Einzel-Fall moeglich).
    if (erg->AbgangGewuenscht)
    {
        ECTBridge_FuehreAfaAbgang(bridge, angeklickt);
        return TRUE;
    }

    if (erg->Buchungen == nullptr || erg->Buchungen->Count == 0)
        return FALSE;  // Abgebrochen

    ECTEngine::Buchung^ basisAlt = erg->ErsetzteBasis;
    int idx = engine->Buchungen->IndexOf(basisAlt);
    if (idx < 0) return FALSE;

    // Bei Gruppen-Bearbeitung: alte Zusatz-Mitglieder entfernen -- sie
    // werden gleich durch die neu berechneten ersetzt. (Auch beim Aufloesen
    // der Gruppe via "Vorlage entfernen": dann kommt nur die Basis zurueck.)
    if (erg->WarGruppenBearbeitung && basisAlt->GruppenUuid != nullptr)
    {
        System::String^ uuid = basisAlt->GruppenUuid;
        for (int i = engine->Buchungen->Count - 1; i >= 0; i--)
        {
            auto b = engine->Buchungen[i];
            if (!System::Object::ReferenceEquals(b, basisAlt)
                && b->GruppenUuid != nullptr && b->GruppenUuid->Equals(uuid))
                engine->Buchungen->RemoveAt(i);
        }
        idx = engine->Buchungen->IndexOf(basisAlt);
        if (idx < 0) return FALSE;
    }

    // Identitaet (Uuid) der Basis uebernehmen, damit die Journal-Selektion
    // die Buchung trotz neuer Buchung^-Instanz wiederfindet.
    erg->Buchungen[0]->Uuid = basisAlt->Uuid;
    engine->Buchungen[idx] = erg->Buchungen[0];
    for (int k = 1; k < erg->Buchungen->Count; k++)
        engine->Buchungen->Add(erg->Buchungen[k]);

    engine->Sort();
    bridge->SyncManagedToNative();

    CString csMeldung = (CString)"Buchung '"
        + ECTBridge::ToNative(erg->Buchungen[0]->Beschreibung) + "' geändert";
    if (erg->WarGruppenBearbeitung)
    {
        CString csZusatz;
        csZusatz.Format(" (Buchungsgruppe, %d Buchungen)", erg->Buchungen->Count);
        csMeldung += csZusatz;
    }
    bridge->SetModifiedFlag(csMeldung);

    // Bei Gruppen-Bearbeitung: alle Mitglieder im Journal markieren
    if (erg->WarGruppenBearbeitung && erg->Buchungen->Count > 1)
    {
        ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
        ECTBridge_SelektiereImJournal(erg->Buchungen);
    }
    return TRUE;
}

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

        return ECTBridge_BearbeiteBuchung(bridge,
            engine->Buchungen[nBuchungIdx], hWndOwner);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler bei Buchungsbearbeitung: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ----------------------------------------------------------
// Pointer-basierte API
// ----------------------------------------------------------
//
// Lookup über die Pointer-Map in CEasyCashDocBridge. Die Map wird
// bei jedem Sync (in beide Richtungen) befüllt und enthält die
// Zuordnung CBuchung* --> Buchung^. O(1)-Lookup, unabhängig von der
// Reihenfolge der Listen oder Sort-Stabilität.

// ----------------------------------------------
// ECT_ShowBuchungBearbeitenDialogFuerPointer
// ----------------------------------------------

BOOL ECT_ShowBuchungBearbeitenDialogFuerPointer(
    void* pDocBridge, CBuchung* pNative, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge || !pNative) return FALSE;

        // Direkter Lookup in der Pointer-Map. Die Map ist nach dem
        // letzten Sync aktuell -- solange seitdem keine Buchungen
        // hinzugefügt/entfernt wurden, stimmt sie.
        ECTEngine::Buchung^ original = FindManagedFor(bridge, pNative);
        if (original == nullptr)
        {
            TRACE0("ECT_ShowBuchungBearbeitenDialogFuerPointer: "
                   "Pointer in Map nicht gefunden -- evtl. veraltet?\n");
            return FALSE;
        }

        return ECTBridge_BearbeiteBuchung(bridge, original, hWndOwner);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler in ECT_ShowBuchungBearbeitenDialogFuerPointer: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ----------------------------------------------
// ECT_ShowBuchungKopierenDialog
// ----------------------------------------------

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

        // Vorlage klonen -- die Original-Buchung bleibt unverändert
        ECTEngine::Buchung^ vorlage = originalRef->Clone();
        vorlage->EntferneGruppe();   // Kopie soll NICHT Gruppen-Mitglied werden

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
        msg = "Fehler in ECT_ShowBuchungKopierenDialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ----------------------------------------------
// ECT_LoescheBuchungPerPointer
// ----------------------------------------------

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

        // Aus der Engine entfernen -- Reference-Equality, unabhängig vom Index
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
        msg = "Fehler in ECT_LoescheBuchungPerPointer: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ----------------------------------------------------------
// Listen-Initialisierung
// ----------------------------------------------------------

/// Hilfsfunktion: native LPCSTR-Array --> managed List<String^>
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

/// Hilfsfunktion: native int-Array --> managed List<int>
static System::Collections::Generic::List<int>^
MachIntListe(const int* pArray, int nCount)
{
    auto liste = gcnew System::Collections::Generic::List<int>();
    for (int i = 0; i < nCount; i++)
        liste->Add(pArray ? pArray[i] : 0);
    return liste;
}

void ECT_SetzeBetriebeUndBestandskonten(
    LPCSTR* pBetriebeNamen, const int* pBetriebeIcons, int nBetriebeCount,
    LPCSTR* pBestandskontenNamen, const int* pBestandskontenIcons,
    const int* pBestandskontenSalden, int nBestandskontenCount)
{
    try
    {
        ECTViews::ViewHost::BetriebeNamen =
            MachListe(pBetriebeNamen, nBetriebeCount);
        ECTViews::ViewHost::BetriebeIcons =
            MachIntListe(pBetriebeIcons, nBetriebeCount);
        ECTViews::ViewHost::BestandskontenNamen =
            MachListe(pBestandskontenNamen, nBestandskontenCount);
        ECTViews::ViewHost::BestandskontenIcons =
            MachIntListe(pBestandskontenIcons, nBestandskontenCount);
        ECTViews::ViewHost::BestandskontenSalden =
            MachIntListe(pBestandskontenSalden, nBestandskontenCount);

        // Bereits eingebettete Journals neu rendern lassen, damit Betriebs-
        // und Bestandskonto-Icons sofort erscheinen. Ohne diesen Push waeren
        // die Icons erst nach dem ersten manuellen Modus-Wechsel sichtbar,
        // weil das Journal beim Programmstart oft VOR dieser Funktion
        // eingebettet wird.
        ECTViews::Journal::JournalEmbed::AktualisiereAlle(nullptr);

        TRACE("ECT_SetzeBetriebeUndBestandskonten: %d Betriebe, %d Bestandskonten\n",
              nBetriebeCount, nBestandskontenCount);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler in ECT_SetzeBetriebeUndBestandskonten: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}

// ----------------------------------------------------------
// Dauerbuchungen
// ----------------------------------------------------------

BOOL ECT_ZeigeDauerbuchungenDialog(void* pDocBridge, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge) return FALSE;

        // Engine-Stand garantieren (Dauerbuchungen kommen aus den nativen
        // Linked Lists)
        bridge->SyncNativeToManaged();

        auto engine = GetEngine(bridge);
        IntPtr hwnd = IntPtr((void*)hWndOwner);

        bool geaendert = ECTViews::ViewHost::ZeigeDauerbuchungenDialog(
            engine, ECTBridge::ToManaged(bridge->GetPathName()), hwnd);

        if (geaendert)
        {
            // Managed Liste zurueck in die native CDauerbuchung-Kette
            bridge->SyncManagedToNative();
            bridge->SetModifiedFlag("Dauerbuchungen geaendert");
        }
        return geaendert ? TRUE : FALSE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler im Dauerbuchungen-Dialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

BOOL ECT_ZeigeUstVorauszahlungenDialog(void* pDocBridge, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge) return FALSE;

        // Engine-Stand garantieren (der Dialog liest die Zahlbetraege aus
        // dem managed Dokument-ErweiterungStore)
        bridge->SyncNativeToManaged();

        auto engine = GetEngine(bridge);
        IntPtr hwnd = IntPtr((void*)hWndOwner);

        bool geaendert = ECTViews::ViewHost::ZeigeUstVorauszahlungenDialog(
            engine, hwnd);

        if (geaendert)
        {
            // geaenderte Erweiterungen zurueck ins native Dokument
            bridge->SyncManagedToNative();
            bridge->SetModifiedFlag("Umsatzsteuervorauszahlungen geaendert");
        }
        return geaendert ? TRUE : FALSE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler im USt-Vorauszahlungen-Dialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

BOOL ECT_ZeigeDauerbuchungenAusfuehrenDialog(
    int nBuchungsjahr, HWND hWndOwner, int* pnMonatOut, int* pnJahrOut)
{
    try
    {
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        int monat = 0, jahr = 0;
        bool ok = ECTViews::ViewHost::ZeigeDauerbuchungenAusfuehrenDialog(
            nBuchungsjahr, hwnd, monat, jahr);
        if (pnMonatOut) *pnMonatOut = monat;
        if (pnJahrOut)  *pnJahrOut  = jahr;
        return ok ? TRUE : FALSE;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler im Dialog 'Dauerbuchungen ausfuehren': "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ----------------------------------------------------------
// Stammdaten-Verwaltung (Betriebe + Bestandskonten)
// ----------------------------------------------------------
//
// Beide Dialoge schreiben ihre Aenderungen sofort ueber den globalen
// Einstellungs-Cache in die easyct.ini (StammdatenVerwaltung in
// ECTEngine). Der native Aufrufer liest seine Listen danach per
// UpdateBetriebeMenu/UpdateBestandskontenMenu neu ein.

int ECT_ZeigeBetriebeVerwaltenDialog(HWND hWndOwner)
{
    try
    {
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        return ECTViews::ViewHost::ZeigeBetriebeVerwaltenDialog(hwnd);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler in ECT_ZeigeBetriebeVerwaltenDialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return -1;
    }
}

int ECT_ZeigeBestandskontenVerwaltenDialog(int nBuchungsjahr, HWND hWndOwner)
{
    try
    {
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        return ECTViews::ViewHost::ZeigeBestandskontenVerwaltenDialog(
            nBuchungsjahr, hwnd);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler in ECT_ZeigeBestandskontenVerwaltenDialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return -1;
    }
}

// ----------------------------------------------------------
// "Buchungsjahr waehlen"-Dialog
// ----------------------------------------------------------

// Bridge-internes Flag fuer den verzoegerten Jahreswechsel (siehe ViewExports.h).
// Native Variable, auch in dieser /clr-TU unproblematisch.
static bool g_bJahreswechselNachInit = false;

void ECT_MerkeJahreswechselNachInit()
{
    g_bJahreswechselNachInit = true;
}

BOOL ECT_HoleUndLoescheJahreswechselNachInit()
{
    BOOL b = g_bJahreswechselNachInit ? TRUE : FALSE;
    g_bJahreswechselNachInit = false;
    return b;
}

/// Hilfsfunktion: managed String^ --> nativen char-Puffer (ANSI/cp1252).
static void KopiereInPuffer(System::String^ s, char* pBuf, int nBufLen)
{
    if (!pBuf || nBufLen <= 0) return;
    CString cs = ECTBridge::ToNative(s);
    strncpy_s(pBuf, nBufLen, (LPCTSTR)cs, _TRUNCATE);
}

int ECT_ShowBuchungsjahrWaehlenDialog(
    LPCSTR pszDatenverzeichnis, int nDefaultJahr, LPCSTR pszDefaultWaehrung,
    HWND hWndOwner,
    int* pnJahrOut, char* pszWaehrungOut, int nWaehrungBufLen,
    char* pszQuelldateiOut, int nQuelldateiBufLen)
{
    try
    {
        System::String^ verzeichnis = pszDatenverzeichnis
            ? gcnew System::String(pszDatenverzeichnis) : System::String::Empty;
        System::String^ waehrung = pszDefaultWaehrung
            ? gcnew System::String(pszDefaultWaehrung) : gcnew System::String("EUR");

        IntPtr hwnd = IntPtr((void*)hWndOwner);

        ECTViews::BuchungsjahrWaehlenErgebnis^ ergebnis =
            ECTViews::ViewHost::ZeigeBuchungsjahrWaehlenDialog(
                verzeichnis, nDefaultJahr, waehrung, hwnd);

        if (pnJahrOut) *pnJahrOut = ergebnis->Jahr;
        KopiereInPuffer(ergebnis->Waehrung, pszWaehrungOut, nWaehrungBufLen);
        KopiereInPuffer(ergebnis->QuelldateiPfad, pszQuelldateiOut, nQuelldateiBufLen);

        return (int)ergebnis->Aktion;   // 0=Abbruch, 1=Neu, 2=Jahreswechsel
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler im Dialog 'Buchungsjahr waehlen': "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return 0;   // Abbruch
    }
}

// ----------------------------------------------------------
// Mandanten-Verwaltung
// ----------------------------------------------------------
//
// Die Mandanten liegen im App-Profil (theApp), das die Bridge nicht
// erreicht: Liste rein, geaenderte Liste ueber Out-Puffer zurueck,
// der native Aufrufer persistiert (siehe ViewExports.h).

// Fuer die Ordner-Picker-Vorauswahl: Verzeichnis der easyct.ini
// (= aktuelles Daten-/Mandantenverzeichnis), siehe ectifacemisc.cpp
extern "C" AFX_EXT_CLASS BOOL GetIniFileName(char* buffer3, int size);

int ECT_ZeigeMandantenVerwaltenDialog(
    LPCSTR* pNamen, const int* pIcons, LPCSTR* pVerzeichnisse, int nAnzahl,
    HWND hWndOwner,
    char* pszNamenOut, int nNamenStride,
    int* pIconsOut,
    char* pszVerzeichnisseOut, int nVerzStride,
    int nMaxAnzahlOut, int* pnAnzahlOut,
    char* pszNichtMandantVerzOut, int nNichtMandantVerzBufLen)
{
    if (pnAnzahlOut) *pnAnzahlOut = -1;   // Default: nichts zurueckschreiben
    // Default: kein Moduswechsel (nur beim Loeschen des letzten Mandanten gefuellt)
    if (pszNichtMandantVerzOut && nNichtMandantVerzBufLen > 0)
        pszNichtMandantVerzOut[0] = '\0';
    try
    {
        auto namen = MachListe(pNamen, nAnzahl);
        auto icons = MachIntListe(pIcons, nAnzahl);
        auto verzeichnisse = MachListe(pVerzeichnisse, nAnzahl);

        // Aktuelles Daten-/Mandantenverzeichnis als Vorauswahl-Fallback
        // fuer den Ordner-Picker (Verzeichnis der easyct.ini)
        char iniBuf[1024] = "";
        System::String^ aktuellesVerzeichnis = System::String::Empty;
        if (GetIniFileName(iniBuf, sizeof(iniBuf)) && *iniBuf)
        {
            char* cp = strrchr(iniBuf, '\\');
            if (cp) *cp = '\0';
            aktuellesVerzeichnis = gcnew System::String(iniBuf);
        }

        IntPtr hwnd = IntPtr((void*)hWndOwner);
        ECTViews::MandantenVerwaltenErgebnis^ ergebnis =
            ECTViews::ViewHost::ZeigeMandantenVerwaltenDialog(
                namen, icons, verzeichnisse, aktuellesVerzeichnis, hwnd);

        int nOut = ergebnis->Namen->Count;
        if (nOut > nMaxAnzahlOut) nOut = nMaxAnzahlOut;
        for (int i = 0; i < nOut; i++)
        {
            if (pszNamenOut && nNamenStride > 0)
                KopiereInPuffer(ergebnis->Namen[i],
                    pszNamenOut + i * nNamenStride, nNamenStride);
            if (pIconsOut)
                pIconsOut[i] = ergebnis->Icons[i];
            if (pszVerzeichnisseOut && nVerzStride > 0)
                KopiereInPuffer(ergebnis->Datenverzeichnisse[i],
                    pszVerzeichnisseOut + i * nVerzStride, nVerzStride);
        }
        if (pnAnzahlOut) *pnAnzahlOut = nOut;

        // Nach dem Loeschen des letzten Mandanten: dessen Datenverzeichnis, das
        // der Aufrufer im Nicht-Mandanten-Modus als Datenverzeichnis uebernimmt.
        KopiereInPuffer(ergebnis->NichtMandantenModusDatenverzeichnis,
            pszNichtMandantVerzOut, nNichtMandantVerzBufLen);

        return ergebnis->GewaehlterIndex < nOut ? ergebnis->GewaehlterIndex : -1;
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler in ECT_ZeigeMandantenVerwaltenDialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return -1;
    }
}

int ECT_ZeigeMandantIconAuswahlDialog(HWND hWndOwner)
{
    try
    {
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        return ECTViews::ViewHost::ZeigeMandantIconAuswahlDialog(hwnd);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler in ECT_ZeigeMandantIconAuswahlDialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return -1;
    }
}

// ----------------------------------------------------------
// Buchungsjournal
// ----------------------------------------------------------

namespace ECTBridge
{
    /// <summary>
    /// Managed Helfer-Klasse, die Native-Pointer als IntPtr-Felder
    /// hält und Methoden bereitstellt, die als Delegate-Targets
    /// für die JournalViewModel-Events dienen.
    ///
    /// Der Umweg über eine ref class ist nötig, weil C++/CLI keine
    /// Lambdas erlaubt, die managed Variablen capturen, und reine
    /// native Lambdas können nicht direkt einem managed Delegate
    /// (System::Action&lt;T&gt;) zugewiesen werden.
    /// </summary>
    ref class JournalEventHandler
    {
    public:
        // Native Pointer als IntPtr-Felder. Sicher, weil das Journal-
        // Fenster spätestens beim Schließen des Dokuments geschlossen
        // wird und der Bridge-Pointer bis dahin gültig ist.
        System::IntPtr m_pBridge;
        System::IntPtr m_hwnd;

        void OnBearbeiten(ECTEngine::Buchung^ b)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
            if (!bridge) return;

            int idx = GetEngine(bridge)->Buchungen->IndexOf(b);
            if (idx >= 0)
            {
                ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd);
                ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
            }
        }

        // Löscht eine ODER mehrere Buchungen; Bestätigung und
        // Kaskadenlösch-Abfrage für Buchungsgruppen stecken in der
        // geteilten Funktion (BuchungenLoeschenShared.h, Definition
        // in JournalExports.cpp).
        void OnLoeschenMehrere(System::Collections::Generic::IList<ECTEngine::Buchung^>^ buchungen)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            if (!bridge) return;
            if (ECTBridge_LoescheBuchungenMitGruppenAbfrage(bridge, buchungen))
                ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
        }

        void OnLoeschen(ECTEngine::Buchung^ b)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            if (!bridge) return;
            auto eng = GetEngine(bridge);

            // Konfirmation
            CString frage;
            frage.Format("Buchung '%s' wirklich löschen?",
                (LPCTSTR)ECTBridge::ToNative(b->Beschreibung));
            if (AfxMessageBox(frage, MB_YESNO | MB_DEFBUTTON2) != IDYES)
                return;

            eng->Buchungen->Remove(b);
            bridge->SyncManagedToNative();
            bridge->SetModifiedFlag("Buchung über Journal gelöscht");
            ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
        }

        void OnKopieren(ECTEngine::Buchung^ b)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
            if (!bridge) return;

            // Klon in Engine einfügen, dann Standard-Bearbeitungsdialog
            // mit dem Index des Klons öffnen.
            auto eng = GetEngine(bridge);
            auto klon = b->Clone();
            klon->EntferneGruppe();   // Kopie soll NICHT Gruppen-Mitglied werden
            eng->Buchungen->Add(klon);
            int idx = eng->Buchungen->IndexOf(klon);
            bridge->SyncManagedToNative();

            ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd);
            ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
        }

        void OnKopierenMitNeuerBelegnummer(ECTEngine::Buchung^ b)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            HWND hwnd = static_cast<HWND>(m_hwnd.ToPointer());
            if (!bridge) return;

            auto eng = GetEngine(bridge);
            auto klon = b->Clone();
            klon->EntferneGruppe();   // Kopie soll NICHT Gruppen-Mitglied werden
            klon->Belegnummer = (klon->Art == ECTEngine::Buchungsart::Einnahme)
                ? eng->LaufendeBelegnrEinnahmen.ToString()
                : eng->LaufendeBelegnrAusgaben.ToString();
            eng->Buchungen->Add(klon);
            int idx = eng->Buchungen->IndexOf(klon);
            bridge->SyncManagedToNative();

            ECT_ShowBuchungBearbeitenDialog(bridge, idx, hwnd);
            ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
        }
    };
}

BOOL ECT_ZeigeJournal(void* pDocBridge, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        if (!bridge) return FALSE;

        // Vor dem Anzeigen: Engine-Stand garantieren
        bridge->SyncNativeToManaged();

        auto engine = GetEngine(bridge);
        IntPtr hwnd = IntPtr((void*)hWndOwner);

        auto vm = ECTViews::Journal::JournalHost::ZeigeJournal(engine, hwnd);

        // Eventhandler -- eine einzige managed Helper-Instanz,
        // die alle Native-Pointer als IntPtr-Felder hält.
        auto handler = gcnew ECTBridge::JournalEventHandler();
        handler->m_pBridge = IntPtr(pDocBridge);
        handler->m_hwnd = hwnd;

        vm->BuchungBearbeiten += gcnew System::Action<ECTEngine::Buchung^>(
            handler, &ECTBridge::JournalEventHandler::OnBearbeiten);
        vm->BuchungenLoeschen += gcnew System::Action<
            System::Collections::Generic::IList<ECTEngine::Buchung^>^>(
            handler, &ECTBridge::JournalEventHandler::OnLoeschenMehrere);
        vm->BuchungKopieren += gcnew System::Action<ECTEngine::Buchung^>(
            handler, &ECTBridge::JournalEventHandler::OnKopieren);
        vm->BuchungKopierenMitNeuerBelegnummer += gcnew System::Action<ECTEngine::Buchung^>(
            handler, &ECTBridge::JournalEventHandler::OnKopierenMitNeuerBelegnummer);

        return TRUE;
    }
    catch (Exception^ ex)
    {
        CString msg = L"Fehler in ECT_ZeigeJournal: " + ECTBridge::ToNative(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

void ECT_AktualisiereJournal(
    int nAnzeigeModus,
    LPCSTR pszKontenFilter,
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    LPCSTR pszBestandskontoFilter,
    double dSchriftgroesse)
{
    try
    {
        auto filter = gcnew ECTViews::Journal::JournalFilter();
        filter->AnzeigeModus = (nAnzeigeModus == 1)
            ? ECTViews::Journal::JournalAnzeigeModus::Konten
            : ECTViews::Journal::JournalAnzeigeModus::Datum;
        filter->KontenFilter = pszKontenFilter
            ? gcnew System::String(pszKontenFilter) : System::String::Empty;
        filter->MonatsFilter = nMonatsFilter;
        filter->BetriebFilter = pszBetriebFilter
            ? gcnew System::String(pszBetriebFilter) : System::String::Empty;
        filter->BestandskontoFilter = pszBestandskontoFilter
            ? gcnew System::String(pszBestandskontoFilter) : System::String::Empty;
        filter->Schriftgroesse = dSchriftgroesse > 0 ? dSchriftgroesse : 13.0;

        ECTViews::Journal::JournalHost::AktualisiereOffenesJournal(filter);
    }
    catch (Exception^ ex)
    {
        CString msg;
        msg = "Fehler in ECT_AktualisiereJournal: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
    }
}
