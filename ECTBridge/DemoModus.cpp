// DemoModus.cpp - Demo-Modus: Sandkasten-Verzeichnis, Beispieldokument,
// Sperr-Rueckfrage. Uebersicht und Sicherheitskonzept: DemoModus.h.
//
// /clr, kein PCH (wie die anderen Export-Dateien). Die Beispieldaten selbst
// liefert ECTEngine.DemoModus (eingebettete easyct.ini + Sammeldatei).

#include "stdafx.h"
#include "DemoModus.h"
#include "EasyCashDocBridge.h"
#include "Marshalling.h"
#include <vcclr.h>          // PtrToStringChars

#using "ECTEngine.dll"
#using <System.dll>

// windows.h macht aus GetTempPath ein Makro (GetTempPathA) -- hier ist
// System::IO::Path::GetTempPath gemeint
#undef GetTempPath

using namespace System;
using namespace System::IO;

// ----------------------------------------------------------
// Zustand (prozessweit, ECTBridge.dll hat genau eine Instanz)
// ----------------------------------------------------------
static BOOL g_bDemo = FALSE;
static BOOL g_bDokumentErlaubt = FALSE;   // genau EIN neues Dokument: die Beispieldaten
static int  g_nJahr = 0;
static char g_szVerzeichnis[MAX_PATH] = "";

// Name der Markierungsdatei im Demo-Verzeichnis. Geloescht wird beim
// Aufraeumen NUR ein Verzeichnis, das sie enthaelt -- so kann ein
// verdrehter Pfad nie ein fremdes Verzeichnis treffen.
static const char* MARKIERUNG = "EasyCT-Demo-Verzeichnis.txt";

// ----------------------------------------------------------
// Hilfen
// ----------------------------------------------------------
namespace
{
    String^ DemoBasis()
    {
        return Path::Combine(Path::GetTempPath(), "EasyCT-Demo");
    }

    bool HatMarkierung(String^ verz)
    {
        return File::Exists(Path::Combine(verz, gcnew String(MARKIERUNG)));
    }

    // Pfad fuer die nativen char*-APIs: verlustfrei in der ANSI-Codepage,
    // sonst die 8.3-Kurzform (Benutzernamen mit Zeichen ausserhalb der
    // Codepage). Leerstring, wenn beides nicht geht.
    CString AnsiPfad(String^ pfad)
    {
        CString ansi = ECTBridge::ToNative(pfad);
        if (String::Equals(ECTBridge::ToManaged(ansi), pfad))
            return ansi;
        pin_ptr<const wchar_t> lang = PtrToStringChars(pfad);
        wchar_t kurz[MAX_PATH];
        DWORD n = ::GetShortPathNameW(lang, kurz, MAX_PATH);
        if (n == 0 || n >= MAX_PATH) return CString();
        CString ansiKurz = ECTBridge::ToNative(gcnew String(kurz));
        return String::Equals(ECTBridge::ToManaged(ansiKurz), gcnew String(kurz)) ? ansiKurz : CString();
    }

    // Verzeichnisse frueherer Demo-Laeufe, deren Prozess nicht mehr lebt
    // (Absturz, Abbruch im Debugger). Name: <PID>-<Zeitstempel>.
    void RaeumeVerwaisteAuf(String^ basis)
    {
        if (!Directory::Exists(basis)) return;
        for each (String^ verz in Directory::GetDirectories(basis))
        {
            try
            {
                String^ name = Path::GetFileName(verz);
                int strich = name->IndexOf('-');
                int pid;
                if (strich <= 0 || !Int32::TryParse(name->Substring(0, strich), pid)) continue;
                if (!HatMarkierung(verz)) continue;
                bool lebt = true;
                try { System::Diagnostics::Process::GetProcessById(pid); }
                catch (ArgumentException^) { lebt = false; }
                if (!lebt || pid == System::Diagnostics::Process::GetCurrentProcess()->Id)
                    Directory::Delete(verz, true);
            }
            catch (Exception^) { /* belegt oder keine Rechte: beim naechsten Mal */ }
        }
    }
}

// ----------------------------------------------------------
// Exports
// ----------------------------------------------------------

BOOL ECT_IstDemoModus()
{
    return g_bDemo;
}

void ECT_DemoModusEinschalten()
{
    g_bDemo = TRUE;
    ECTEngine::DemoModus::Aktiv = true;   // managed Schreibwege (Formular-Designer)
}

BOOL ECT_DemoParameterVorhanden()
{
    array<String^>^ args = Environment::GetCommandLineArgs();
    for (int i = 1; i < args->Length; i++)
    {
        String^ a = args[i]->Trim();
        if (String::Equals(a, "/demo", StringComparison::OrdinalIgnoreCase)
            || String::Equals(a, "-demo", StringComparison::OrdinalIgnoreCase))
            return TRUE;
    }
    return FALSE;
}

BOOL ECT_DemoVorbereiten(char* szVerzeichnis, int nGroesse, char* szFehler, int nFehlerGroesse)
{
    if (szVerzeichnis && nGroesse > 0) szVerzeichnis[0] = '\0';
    if (szFehler && nFehlerGroesse > 0) szFehler[0] = '\0';
    try
    {
        // Demojahr aus den installierten Formularen (Programmverzeichnis)
        int jahr = ECTEngine::DemoModus::ErmittleJahr(AppDomain::CurrentDomain->BaseDirectory);

        String^ basis = DemoBasis();
        RaeumeVerwaisteAuf(basis);

        String^ verz = Path::Combine(basis, String::Format("{0}-{1:yyyyMMdd-HHmmss-fff}",
            System::Diagnostics::Process::GetCurrentProcess()->Id, DateTime::Now));
        if (Directory::Exists(verz))
            throw gcnew IOException("Das Demo-Verzeichnis existiert bereits: " + verz);
        Directory::CreateDirectory(verz);
        File::WriteAllText(Path::Combine(verz, gcnew String(MARKIERUNG)),
            "Arbeitsverzeichnis des EasyCash&Tax-Demo-Modus. Wird beim Beenden geloescht.\r\n");

        // CreateNew: eine vorhandene Datei wuerde nie ueberschrieben
        ECTEngine::DemoModus::SchreibeIni(Path::Combine(verz, "easyct.ini"), jahr);

        CString ansi = AnsiPfad(verz);
        if (ansi.IsEmpty() || ansi.GetLength() >= nGroesse || ansi.GetLength() >= MAX_PATH)
            throw gcnew IOException("Der Pfad des Demo-Verzeichnisses laesst sich nicht verwenden: " + verz);

        strcpy_s(g_szVerzeichnis, MAX_PATH, ansi);
        strcpy_s(szVerzeichnis, nGroesse, ansi);
        g_nJahr = jahr;
        g_bDemo = TRUE;
        ECTEngine::DemoModus::Aktiv = true;   // managed Schreibwege (Formular-Designer)
        g_bDokumentErlaubt = TRUE;
        TRACE("Demo-Modus: Verzeichnis %s, Jahr %d\n", (LPCSTR)ansi, jahr);
        return TRUE;
    }
    catch (Exception^ ex)
    {
        if (szFehler && nFehlerGroesse > 0)
            strncpy_s(szFehler, nFehlerGroesse, ECTBridge::ToNative(ex->Message), _TRUNCATE);
        return FALSE;
    }
}

int ECT_DemoJahr()
{
    return g_nJahr;
}

BOOL ECT_DemoFunktionGesperrt()
{
    // Mehrere Sperren koennen fuer EINE Bedienung greifen (z.B. erst das
    // Kommando, dann die Dokumentvorlage) -- nur einmal fragen.
    static BOOL s_bFrageOffen = FALSE;
    if (s_bFrageOffen) return TRUE;
    s_bFrageOffen = TRUE;
    int nAntwort = AfxMessageBox(
        "Im Demo-Modus ist diese Funktion nicht erlaubt. Demo-Modus beenden?\r\n\r\n"
        "(Beim Beenden startet EasyCash&Tax normal neu, mit Ihren eigenen Daten. "
        "Die Beispieldaten werden nicht gespeichert.)",
        MB_YESNO | MB_ICONINFORMATION);
    s_bFrageOffen = FALSE;

    CWnd* pMain = AfxGetMainWnd();
    if (nAntwort == IDYES && pMain && ::IsWindow(pMain->GetSafeHwnd()))
        pMain->PostMessage(WM_COMMAND, ECT_ID_DEMO_BEENDEN_DIREKT, 0);
    return TRUE;
}

void ECT_DemoAufraeumen()
{
    if (!g_szVerzeichnis[0]) return;
    try
    {
        String^ verz = ECTBridge::ToManaged(CString(g_szVerzeichnis));
        String^ basis = DemoBasis();
        // nur das selbst angelegte Verzeichnis, nur unterhalb von
        // %TEMP%\EasyCT-Demo und nur mit Markierungsdatei
        String^ voll = Path::GetFullPath(verz);
        String^ vollBasis = Path::GetFullPath(basis)->TrimEnd('\\') + "\\";
        if (voll->StartsWith(vollBasis, StringComparison::OrdinalIgnoreCase)
            && Directory::Exists(voll) && HatMarkierung(voll))
            Directory::Delete(voll, true);
    }
    catch (Exception^) { /* z.B. von einem Plugin belegt: der naechste Demo-Start raeumt auf */ }
    g_szVerzeichnis[0] = '\0';
}

// ----------------------------------------------------------
// Bridge-intern
// ----------------------------------------------------------

BOOL ECT_DemoDokumentErlaubt()
{
    BOOL b = g_bDokumentErlaubt;
    g_bDokumentErlaubt = FALSE;
    return b;
}

BOOL ECT_DemoDokumentFuellen(CEasyCashDoc* pDoc)
{
    if (!pDoc || !pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDocBridge))) return FALSE;
    CEasyCashDocBridge* pBridge = static_cast<CEasyCashDocBridge*>(pDoc);
    if (!pBridge || !g_bDemo) return FALSE;
    try
    {
        ECTEngine::BuchungsDocument^ demo = ECTEngine::DemoModus::ErzeugeDokument(g_nJahr);
        GetEngine(pBridge)->UebernimmVon(demo);
        pBridge->SyncManagedToNative();
        return TRUE;
    }
    catch (Exception^ ex)
    {
        AfxMessageBox("Die Beispieldaten des Demo-Modus konnten nicht geladen werden:\r\n\r\n"
            + ECTBridge::ToNative(ex->Message), MB_ICONSTOP);
        return FALSE;
    }
}
