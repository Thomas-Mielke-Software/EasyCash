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

#using "ECTEngine.dll"
#using "ECTViews.dll"

using namespace System;

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

        void OnBuchenUndNaechste(ECTEngine::Buchung^ buchung)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            if (!bridge || buchung == nullptr) return;

            auto engine = GetEngine(bridge);
            engine->Buchungen->Add(buchung);
            engine->InkrementBuchungszaehler();
            engine->Sort();

            bridge->SyncManagedToNative();
            bridge->SetModifiedFlag(
                (CString)"Buchung '" + ECTBridge::ToNative(buchung->Beschreibung) + "' hinzugefügt");

            // Offenes WPF-Journal sofort nachziehen.
            ECTViews::Journal::JournalHost::AktualisiereOffenesJournal();
        }
    };
}

// ----------------------------------------------------------
// ECT_ShowBuchungDialog
// ----------------------------------------------------------

BOOL ECT_ShowBuchungDialog(void* pDocBridge, BOOL bAusgaben, HWND hWndOwner)
{
    try
    {
        auto* bridge = static_cast<CEasyCashDocBridge*>(pDocBridge);
        auto engine = GetEngine(bridge);

        // Vor dem Dialog: Native --> Managed synchronisieren
        // (falls seit dem letzten Sync native Ãnderungen passiert sind)
        bridge->SyncNativeToManaged();

        // WPF-Dialog anzeigen. Der "Buchen & naechste"-Callback
        // persistiert Zwischenbuchungen, ohne den Dialog zu schliessen.
        IntPtr hwnd = IntPtr((void*)hWndOwner);

        auto weiterHandler = gcnew ECTBridge::BuchenWeiterHandler();
        weiterHandler->m_pBridge = IntPtr(pDocBridge);
        auto weiterCb = gcnew System::Action<ECTEngine::Buchung^>(
            weiterHandler, &ECTBridge::BuchenWeiterHandler::OnBuchenUndNaechste);

        ECTEngine::Buchung^ ergebnis =
            ECTViews::ViewHost::ZeigeBuchungDialog(
                engine, bAusgaben != 0, hwnd, weiterCb);

        if (ergebnis == nullptr)
            return FALSE;  // Abgebrochen

        // Buchung in die Engine einfügen
        engine->Buchungen->Add(ergebnis);
        engine->InkrementBuchungszaehler();
        engine->Sort();

        // Managed --> Native synchronisieren
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
        msg = "Fehler im Buchungsdialog: "; msg += CString(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        return FALSE;
    }
}

// ----------------------------------------------------------
// ECT_ShowBuchungBearbeitenDialog
// ----------------------------------------------------------

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
        auto ergebnis =
            ECTViews::ViewHost::ZeigeBuchungBearbeitenDialogMitAbgang(
                engine, original, hwnd);

        // "Abgang buchen": gleiche AfA-Abgang-Logik wie der Journal-
        // Kontextmenue-Eintrag, ausgefuehrt auf der bearbeiteten Buchung.
        if (ergebnis->AbgangGewuenscht)
        {
            ECTBridge_FuehreAfaAbgang(bridge, original);
            return TRUE;
        }

        ECTEngine::Buchung^ geaendert = ergebnis->Buchung;
        if (geaendert == nullptr)
            return FALSE;  // Abgebrochen

        // Identitaet (Uuid) aus dem Original übernehmen, damit Selektion
        // im Journal trotz neuer Buchung^-Instanz wiedergefunden wird.
        geaendert->Uuid = original->Uuid;

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

        auto engine = GetEngine(bridge);

        // WPF-Dialog mit der gefundenen Original-Buchung anzeigen
        IntPtr hwnd = IntPtr((void*)hWndOwner);
        auto ergebnis =
            ECTViews::ViewHost::ZeigeBuchungBearbeitenDialogMitAbgang(
                engine, original, hwnd);

        // "Abgang buchen": gleiche AfA-Abgang-Logik wie der Journal-
        // Kontextmenue-Eintrag, ausgefuehrt auf der bearbeiteten Buchung.
        if (ergebnis->AbgangGewuenscht)
        {
            ECTBridge_FuehreAfaAbgang(bridge, original);
            return TRUE;
        }

        ECTEngine::Buchung^ geaendert = ergebnis->Buchung;
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
    int nMaxAnzahlOut, int* pnAnzahlOut)
{
    if (pnAnzahlOut) *pnAnzahlOut = -1;   // Default: nichts zurueckschreiben
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

        void OnLoeschenMehrere(System::Collections::Generic::IList<ECTEngine::Buchung^>^ buchungen)
        {
            auto* bridge = static_cast<CEasyCashDocBridge*>(m_pBridge.ToPointer());
            if (!bridge || buchungen == nullptr || buchungen->Count == 0) return;
            auto eng = GetEngine(bridge);

            // Encoding: \366 (Oktal = 0xF6 = cp1252 'oe-Umlaut') statt Literal,
            // damit AfxMessageBox (MBCS) korrekt anzeigt, unabhaengig vom Dateiencoding.
            CString frage;
            if (buchungen->Count == 1)
                frage.Format("Buchung '%s' wirklich l\366schen?",
                    (LPCTSTR)ECTBridge::ToNative(buchungen[0]->Beschreibung));
            else
                frage.Format("%d Buchungen wirklich l\366schen?", buchungen->Count);
            if (AfxMessageBox(frage, MB_YESNO | MB_DEFBUTTON2) != IDYES)
                return;

            for each (ECTEngine::Buchung^ b in buchungen)
                eng->Buchungen->Remove(b);
            bridge->SyncManagedToNative();
            bridge->SetModifiedFlag("Buchungen ueber Journal geloescht");
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
