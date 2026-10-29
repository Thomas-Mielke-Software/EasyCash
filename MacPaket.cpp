// MacPaket.cpp: Erkennung des EC&T-Mac-Pakets (PortJump-/CrossOver-App-Bundle), siehe MacPaket.h
//
// PortJump bietet keine Laufzeit-API fuer die Paketversion. Das Bundle hat aber eine feste
// Struktur: EasyCash&Tax.app/Contents/Info.plist enthaelt die Version, die Wine-Engine liegt in
// EasyCash&Tax.app/Contents/SharedSupport/<id> (= CX_ROOT). Ueber die Unix-Umgebungsvariablen,
// die Wine an Windows-Prozesse durchreicht, finden wir das Bundle und lesen die Info.plist.
//
// Achtung: Diese Datei bitte rein ASCII halten, sie wird zwischen v3.x (windows-1252)
// und v4.x (UTF-8) geteilt.

#include "stdafx.h"
#include <afxpriv.h>
#include "MacPaket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Wine-spezifische Exporte von ntdll.dll bzw. kernel32.dll (fehlen unter echtem Windows)
typedef const char * (__cdecl *PFN_WINE_GET_VERSION)(void);
typedef void (__cdecl *PFN_WINE_GET_HOST_VERSION)(const char **sysname, const char **release);
typedef WCHAR * (__cdecl *PFN_WINE_GET_DOS_FILE_NAME)(const char *unix_name);
typedef char * (__cdecl *PFN_WINE_GET_UNIX_FILE_NAME)(const WCHAR *dos_name);

BOOL LaeuftUnterMacOS()
{
	HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
	if (!hNtdll || !GetProcAddress(hNtdll, "wine_get_version"))
		return FALSE;	// kein Wine

	PFN_WINE_GET_HOST_VERSION pfnWineGetHostVersion = (PFN_WINE_GET_HOST_VERSION)GetProcAddress(hNtdll, "wine_get_host_version");
	if (!pfnWineGetHostVersion)
		return FALSE;

	const char *szSysName = NULL, *szRelease = NULL;
	pfnWineGetHostVersion(&szSysName, &szRelease);
	return szSysName && !_stricmp(szSysName, "Darwin");
}

// Unix-Pfade sind unter macOS UTF-8-kodiert
static CStringA NachUtf8(const CStringW& csw)
{
	CStringA csa;
	int n = WideCharToMultiByte(CP_UTF8, 0, csw, -1, NULL, 0, NULL, NULL);
	if (n > 1)
	{
		WideCharToMultiByte(CP_UTF8, 0, csw, -1, csa.GetBuffer(n), n, NULL, NULL);
		csa.ReleaseBuffer();
	}
	return csa;
}

static CStringW VonUtf8(const char *sz)
{
	CStringW csw;
	int n = MultiByteToWideChar(CP_UTF8, 0, sz, -1, NULL, 0);
	if (n > 1)
	{
		MultiByteToWideChar(CP_UTF8, 0, sz, -1, csw.GetBuffer(n), n);
		csw.ReleaseBuffer();
	}
	return csw;
}

// Unix-Pfad -> Windows-Pfad, unter dem Wine die Datei erreichbar macht
static CStringW UnixPfadNachDos(const CStringW& csUnixPfad)
{
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	PFN_WINE_GET_DOS_FILE_NAME pfnWineGetDosFileName = hKernel32 ? (PFN_WINE_GET_DOS_FILE_NAME)GetProcAddress(hKernel32, "wine_get_dos_file_name") : NULL;
	if (pfnWineGetDosFileName)
	{
		WCHAR *pDosPfad = pfnWineGetDosFileName(NachUtf8(csUnixPfad));
		if (pDosPfad)
		{
			CStringW csDosPfad(pDosPfad);
			HeapFree(GetProcessHeap(), 0, pDosPfad);
			return csDosPfad;
		}
	}

	// Fallback: Wine akzeptiert Unix-Pfade auch mit dem Praefix \\?\unix
	CStringW csDosPfad = L"\\\\?\\unix" + csUnixPfad;
	csDosPfad.Replace(L'/', L'\\');
	return csDosPfad;
}

// Aus einem Unix-Pfad innerhalb des App-Bundles (z.B. CX_ROOT = ".../EasyCash&Tax.app/Contents/SharedSupport/<id>")
// den Pfad der Info.plist des aeussersten Bundles ableiten
static BOOL InfoPlistAusBundlePfad(const CStringW& csUnixPfad, CStringW& csInfoPlist)
{
	CStringW csKlein = csUnixPfad;
	csKlein.MakeLower();
	int n = csKlein.Find(L".app/contents");
	if (n < 0)
		return FALSE;
	csInfoPlist = csUnixPfad.Left(n + (int)wcslen(L".app/contents")) + L"/Info.plist";
	return TRUE;
}

static BOOL LiesDatei(const CStringW& csDosPfad, CStringA& csInhalt)
{
	HANDLE hDatei = CreateFileW(csDosPfad, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDatei == INVALID_HANDLE_VALUE)
		return FALSE;

	BOOL bOk = FALSE;
	DWORD dwGroesse = GetFileSize(hDatei, NULL);
	if (dwGroesse != INVALID_FILE_SIZE && dwGroesse > 0 && dwGroesse < 1024 * 1024)
	{
		DWORD dwGelesen = 0;
		bOk = ReadFile(hDatei, csInhalt.GetBuffer(dwGroesse), dwGroesse, &dwGelesen, NULL) && dwGelesen == dwGroesse;
		csInhalt.ReleaseBuffer(bOk ? dwGelesen : 0);
	}
	CloseHandle(hDatei);
	return bOk;
}

// <key>szKey</key><string>Wert</string> aus einer XML-plist lesen
static BOOL PlistString(const CStringA& csPlist, LPCSTR szKey, CString& csWert)
{
	CStringA csKeyTag;
	csKeyTag.Format("<key>%s</key>", szKey);
	int n = csPlist.Find(csKeyTag);
	if (n < 0)
		return FALSE;
	n += csKeyTag.GetLength();
	int nStart = csPlist.Find("<string>", n);
	if (nStart < 0)
		return FALSE;
	CStringA csZwischenraum = csPlist.Mid(n, nStart - n);
	if (!csZwischenraum.Trim().IsEmpty())
		return FALSE;	// Wert ist kein <string>
	nStart += (int)strlen("<string>");
	int nEnde = csPlist.Find("</string>", nStart);
	if (nEnde < 0)
		return FALSE;
	csWert = CString(csPlist.Mid(nStart, nEnde - nStart));
	csWert.Trim();
	return !csWert.IsEmpty();
}

BOOL GetMacPaketVersion(CString& csVersion)
{
	csVersion.Empty();
	if (!LaeuftUnterMacOS())
		return FALSE;

	CStringW csKandidaten[4];
	int nKandidaten = 0;

	// 1. CrossOver-/Wine-Umgebungsvariablen, die ins Bundle zeigen
	LPCWSTR aszVariablen[] = { L"CX_ROOT", L"WINEPREFIX" };
	for (int i = 0; i < sizeof(aszVariablen) / sizeof(aszVariablen[0]); i++)
	{
		WCHAR szWert[2048];
		DWORD dwLaenge = GetEnvironmentVariableW(aszVariablen[i], szWert, sizeof(szWert) / sizeof(szWert[0]));
		if (dwLaenge > 0 && dwLaenge < sizeof(szWert) / sizeof(szWert[0]) &&
			InfoPlistAusBundlePfad(szWert, csKandidaten[nKandidaten]))
			nKandidaten++;
	}

	// 2. Unix-Pfad der laufenden EasyCT.exe (falls die Bottle im Bundle liegt)
	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	PFN_WINE_GET_UNIX_FILE_NAME pfnWineGetUnixFileName = hKernel32 ? (PFN_WINE_GET_UNIX_FILE_NAME)GetProcAddress(hKernel32, "wine_get_unix_file_name") : NULL;
	WCHAR szExe[MAX_PATH];
	if (pfnWineGetUnixFileName && GetModuleFileNameW(NULL, szExe, MAX_PATH))
	{
		char *pUnixPfad = pfnWineGetUnixFileName(szExe);
		if (pUnixPfad)
		{
			if (InfoPlistAusBundlePfad(VonUtf8(pUnixPfad), csKandidaten[nKandidaten]))
				nKandidaten++;
			HeapFree(GetProcessHeap(), 0, pUnixPfad);
		}
	}

	// 3. Standard-Installationsort
	csKandidaten[nKandidaten++] = L"/Applications/EasyCash&Tax.app/Contents/Info.plist";

	for (int i = 0; i < nKandidaten; i++)
	{
		CStringA csPlist;
		if (LiesDatei(UnixPfadNachDos(csKandidaten[i]), csPlist) &&
			(PlistString(csPlist, "CFBundleShortVersionString", csVersion) || PlistString(csPlist, "CFBundleVersion", csVersion)))
			return TRUE;
	}
	return FALSE;
}

// <0, wenn szA aelter als szB ist, 0 bei Gleichheit, >0 wenn neuer
static int VergleicheVersionen(LPCSTR szA, LPCSTR szB)
{
	int a[4] = { 0, 0, 0, 0 }, b[4] = { 0, 0, 0, 0 };
	sscanf_s(szA, "%d.%d.%d.%d", &a[0], &a[1], &a[2], &a[3]);
	sscanf_s(szB, "%d.%d.%d.%d", &b[0], &b[1], &b[2], &b[3]);
	for (int i = 0; i < 4; i++)
		if (a[i] != b[i])
			return a[i] < b[i] ? -1 : 1;
	return 0;
}

BOOL IstMacPaketVeraltet(CString& csVersion)
{
	return GetMacPaketVersion(csVersion) && VergleicheVersionen(CStringA(csVersion), MAC_PAKET_MINDESTVERSION) < 0;
}

// Platz fuer Treiber, Geraet und Port in DEVNAMES; CrossOver erlaubt Ports bis PATH_MAX (1024 Zeichen)
#define DEVNAMES_RESERVE 8192

void DevNamesFuerCrossOverVergroessern(HGLOBAL& hDevNames)
{
	if (!hDevNames || !LaeuftUnterMacOS() || GlobalSize(hDevNames) >= DEVNAMES_RESERVE)
		return;

	// GMEM_MOVEABLE, weil hDevNames durch MFC schon gesperrt sein kann
	HGLOBAL hNeu = GlobalReAlloc(hDevNames, DEVNAMES_RESERVE, GMEM_MOVEABLE);
	if (hNeu)
		hDevNames = hNeu;
}

// Nur Zugriffsverletzungen innerhalb von comdlg32.dll unter CrossOver/macOS abfangen
static int CrossOverDruckdialogFilter(EXCEPTION_POINTERS* pEP)
{
	if (pEP->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION || !LaeuftUnterMacOS())
		return EXCEPTION_CONTINUE_SEARCH;

	HMODULE hComdlg32 = GetModuleHandleA("comdlg32.dll");
	MEMORY_BASIC_INFORMATION mbi;
	if (hComdlg32 && VirtualQuery(pEP->ExceptionRecord->ExceptionAddress, &mbi, sizeof(mbi)) &&
		mbi.AllocationBase == (PVOID)hComdlg32)
		return EXCEPTION_EXECUTE_HANDLER;
	return EXCEPTION_CONTINUE_SEARCH;
}

// __try/__except darf nicht in Funktionen mit C++-Objekten stehen, deshalb eine eigene Funktion
static BOOL RufeDruckdialogAuf(PFN_DRUCKDIALOG pfnDruckdialog, void* pKontext, INT_PTR* pnErgebnis)
{
	__try
	{
		*pnErgebnis = pfnDruckdialog(pKontext);
		return TRUE;
	}
	__except (CrossOverDruckdialogFilter(GetExceptionInformation()))
	{
		return FALSE;
	}
}

// Zugriff auf das geschuetzte CDialog::PostModal(), das wegen des Absturzes nicht mehr aufgerufen wurde
class CPrintDialogPostModal : public CPrintDialog
{
public:
	static void (CDialog::*PostModalZeiger())() { return &CPrintDialogPostModal::PostModal; }
};

BOOL DruckdialogMitCrossOverSchutz(CPrintDialog* pPD, PFN_DRUCKDIALOG pfnDruckdialog, void* pKontext, INT_PTR& nErgebnis)
{
	nErgebnis = IDCANCEL;
	if (RufeDruckdialogAuf(pfnDruckdialog, pKontext, &nErgebnis))
		return FALSE;

	// Aufraeumen, was CPrintDialog::DoModal() nach PrintDlgA sonst erledigt haette
	(pPD->*CPrintDialogPostModal::PostModalZeiger())();
	pPD->m_pd.hDC = NULL;
	if (AfxGetMainWnd())
		AfxGetMainWnd()->EnableWindow(TRUE);

	AfxMessageBox("Der Druck wurde abgebrochen, weil die CrossOver-Schicht des Mac-Pakets im macOS-Druckdialog abgest\xfcrzt ist. "
		"Das passiert, wenn dort \"Als PDF sichern\" (oder eine andere PDF-Option) gew\xe4hlt wird.\n\n"
		"Bitte im Druckdialog einen Drucker ausw\xe4hlen. Um ein PDF zu erzeugen, kann unter macOS ein virtueller PDF-Drucker "
		"(z.B. PDFwriter) installiert und dann wie ein normaler Drucker ausgew\xe4hlt werden.\n\n"
		"Sicherheitshalber bitte die Buchungsdaten speichern und EC&T neu starten.", MB_ICONEXCLAMATION | MB_OK);
	return TRUE;
}
