// ProgrammUpdate.cpp: Update-Prüfung für das Hauptprogramm (v4 Preview), siehe ProgrammUpdate.h
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2026  Thomas Mielke
//
// Dies ist freie Software; Sie dürfen sie unter den Bedingungen der
// GNU General Public License, wie von der Free Software Foundation
// veröffentlicht, weiterverteilen und/oder modifizieren; entweder gemäß
// Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
//
// Threading: Die beiden Worker-Threads (Prüfung, Download) fassen weder MFC-
// Fenster noch den Einstellungs-Cache an. Sie legen ihr Ergebnis in eine
// Heap-Struktur und posten sie ans Hauptfenster; der Handler im UI-Thread
// gibt sie frei. Scheitert das PostMessage (Programm wird gerade beendet),
// räumt der Thread selbst auf.

#include "stdafx.h"
#include <shlobj.h>
#include <wininet.h>
#include <urlmon.h>
#include "easycash.h"
#include "ProgrammUpdate.h"
#include "ECTBridge\EinstellungenExports.h"

#pragma comment(lib, "version.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern char version_string_exakt[50];	// easycash.cpp, z.B. "v4.0.27.1234"

#define SKIP_VERSION_KEY	"[Allgemein]SkipVersion"

// ----------------------------------------------------------
// Ergebnis-Strukturen der Worker-Threads

struct ProgrammUpdatePruefErgebnis
{
	BOOL	bInfoOk;			// update_v4.txt geladen und lesbar
	int		nVerfuegbar[3];
	CString	csAenderung;
	BOOL	bSetupOk;			// ECTSetup4.exe vorhanden und Version lesbar
	int		nSetup[3];
	CString	csSetupPfad;
};

struct ProgrammUpdateDownloadAuftrag
{
	HWND	hwnd;
	int		nErwartet[3];
	CString	csSetupPfad;
	BOOL	bOk;
	CString	csFehler;
};

// ----------------------------------------------------------
// Zustand (nur im UI-Thread benutzt)

static CString	s_csVerfuegbar;			// z.B. "v4.0.28"
static CString	s_csAenderung;
static BOOL		s_bDownloadLaeuft = FALSE;
static CString	s_csSetupZumBeenden;	// für WM_PROGRAMMUPDATE_BEENDEN
static CString	s_csSetupNachProgrammende;

// ----------------------------------------------------------
// Hilfsfunktionen

// "v4.0.28", "4.0.28" oder "v4.0.27.1234" -> {4,0,28}; streng, damit z.B. eine
// HTML-Fehlerseite des Servers nicht als Version durchgeht.
static BOOL ParseVersion(LPCSTR s, int v[3])
{
	if (!s) return FALSE;
	while (*s == ' ' || *s == '\t') s++;
	if (*s == 'v' || *s == 'V') s++;
	for (int i = 0; i < 3; i++)
	{
		if (!isdigit((unsigned char)*s)) return FALSE;
		v[i] = 0;
		while (isdigit((unsigned char)*s)) v[i] = v[i] * 10 + (*s++ - '0');
		if (i < 2 && *s++ != '.') return FALSE;
	}
	return TRUE;
}

static int VergleicheVersion(const int a[3], const int b[3])
{
	for (int i = 0; i < 3; i++)
		if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
	return 0;
}

static CString VersionText(const int v[3])
{
	CString cs;
	cs.Format("v%d.%d.%d", v[0], v[1], v[2]);
	return cs;
}

// Dateiversion (VS_FIXEDFILEINFO) einer exe, beim Inno-Setup = VersionInfoVersion
static BOOL HoleDateiVersion(LPCSTR szPfad, int v[3])
{
	DWORD dwDummy = 0;
	DWORD dwGroesse = GetFileVersionInfoSize(szPfad, &dwDummy);
	if (!dwGroesse) return FALSE;

	CByteArray daten;
	daten.SetSize(dwGroesse);
	if (!GetFileVersionInfo(szPfad, 0, dwGroesse, daten.GetData())) return FALSE;

	VS_FIXEDFILEINFO *pInfo = NULL;
	UINT uLaenge = 0;
	if (!VerQueryValue(daten.GetData(), "\\", (LPVOID *)&pInfo, &uLaenge) || !pInfo || uLaenge < sizeof(VS_FIXEDFILEINFO))
		return FALSE;

	v[0] = HIWORD(pInfo->dwFileVersionMS);
	v[1] = LOWORD(pInfo->dwFileVersionMS);
	v[2] = HIWORD(pInfo->dwFileVersionLS);
	return TRUE;
}

// C:\ProgramData\EasyCash&Tax\download\ECTSetup4.exe (Verzeichnis wie CWebUpdate::SetLocalDirectory)
static CString SetupPfad()
{
	char szPfad[MAX_PATH] = "";
	CString csVerzeichnis;
	if (SHGetSpecialFolderPath(NULL, szPfad, CSIDL_COMMON_APPDATA, TRUE))
	{
		csVerzeichnis = (CString)szPfad + "\\EasyCash&Tax";
		CreateDirectory(csVerzeichnis, NULL);
		csVerzeichnis += "\\download";
		CreateDirectory(csVerzeichnis, NULL);
	}
	else
	{
		GetTempPath(sizeof(szPfad), szPfad);
		csVerzeichnis = szPfad;
		csVerzeichnis.TrimRight('\\');
	}
	return csVerzeichnis + "\\" PROGRAMMUPDATE_SETUP_DATEI;
}

// lädt eine URL am WinINet-Cache vorbei (sonst kann eine veraltete update_v4.txt hängenbleiben)
static HRESULT LadeHerunter(LPCSTR szUrl, LPCSTR szZiel)
{
	DeleteUrlCacheEntry(szUrl);
	return URLDownloadToFile(NULL, szUrl, szZiel, 0, NULL);
}

// erste Zeile "v4.0.28:Text" zerlegen
static BOOL ParseUpdateInfo(const CString& csInhalt, int v[3], CString& csAenderung)
{
	CString cs = csInhalt;
	if (cs.Left(3) == "\xEF\xBB\xBF") cs = cs.Mid(3);	// falls doch mal jemand mit BOM speichert
	int nZeilenende = cs.FindOneOf("\r\n");
	if (nZeilenende >= 0) cs = cs.Left(nZeilenende);

	int nDoppelpunkt = cs.Find(':');
	if (nDoppelpunkt < 0) return FALSE;
	if (!ParseVersion(cs.Left(nDoppelpunkt), v)) return FALSE;
	csAenderung = cs.Mid(nDoppelpunkt + 1);
	csAenderung.Trim();
	return TRUE;
}

// '&' würde die CaptionBar als Zugriffstaste lesen, '\b' schaltet dort auf Fett
static CString FuerCaptionBar(CString cs)
{
	cs.Remove('\b');
	cs.Replace("&", "&&");
	return cs;
}

static CString InstallierteVersion()
{
	int v[3];
	return ParseVersion(version_string_exakt, v) ? VersionText(v) : (CString)version_string_exakt;
}

static void ZeigeUpdateMeldung(LPCSTR szVorspann, LPCSTR szInstallierenText)
{
	CString cs;
	cs.Format("%s\bUpdate %s verfügbar\b; installiert ist %s.", szVorspann, (LPCSTR)s_csVerfuegbar, (LPCSTR)InstallierteVersion());
	if (!s_csAenderung.IsEmpty())
		cs += " Letzte Änderung: " + FuerCaptionBar(s_csAenderung);
	theApp.ProgrammCaptionBox(cs,
		ID_PROGRAMMUPDATE_INSTALLIEREN, szInstallierenText, "EasyCash&&Tax beenden, Update ggf. herunterladen und Setup starten",
		ID_PROGRAMMUPDATE_UEBERSPRINGEN, "Überspringen");
}

// Hauptfenster schließen und, wenn das geklappt hat, in ExitInstance das Setup starten.
// Nur aus einem Nachrichten-Handler des Hauptfensters heraus aufrufen (wie CWinApp::OnAppExit),
// NICHT aus einem Klick-Handler der CaptionBar -- die gehört zum Hauptfenster und wäre danach weg.
static void BeendenUndSetupStarten(const CString& csSetupPfad)
{
	CWnd *pHauptfenster = AfxGetMainWnd();
	if (!pHauptfenster) return;
	HWND hwnd = pHauptfenster->GetSafeHwnd();

	theApp.CaptionBoxHide();
	s_csSetupNachProgrammende = csSetupPfad;
	pHauptfenster->SendMessage(WM_CLOSE);

	if (::IsWindow(hwnd))
	{	// Beenden abgebrochen (z.B. "Abbrechen" bei ungespeicherten Änderungen)
		s_csSetupNachProgrammende.Empty();
		if (!s_csVerfuegbar.IsEmpty())
			ZeigeUpdateMeldung("Installation abgebrochen, weil EC&&T nicht beendet wurde. ", "Installieren");
	}
}

static void FrageInstallation(const CString& csSetupPfad, const int nSetup[3])
{
	CString csFrage;
	csFrage.Format("Das Update %s von EasyCash&Tax wurde bereits heruntergeladen (installiert ist %s).\n\n"
		"Soll EasyCash&Tax jetzt beendet und das Update installiert werden?\n\n"
		"Ja: EasyCash&Tax beenden und Setup starten\n"
		"Nein: heruntergeladene Setup-Datei löschen\n"
		"Abbrechen: nichts tun, beim nächsten Programmstart wieder fragen",
		(LPCSTR)VersionText(nSetup), (LPCSTR)InstallierteVersion());

	switch (AfxMessageBox(csFrage, MB_YESNOCANCEL | MB_ICONQUESTION))
	{
	case IDYES:
		BeendenUndSetupStarten(csSetupPfad);
		break;
	case IDNO:
		if (!DeleteFile(csSetupPfad))
			AfxMessageBox("Die Setup-Datei '" + csSetupPfad + "' konnte nicht gelöscht werden.", MB_ICONEXCLAMATION);
		break;
	}
}

// ----------------------------------------------------------
// Worker-Threads

static UINT __cdecl PruefThread(LPVOID pParam)
{
	HWND hwnd = (HWND)pParam;
	ProgrammUpdatePruefErgebnis *p = new ProgrammUpdatePruefErgebnis;
	p->bInfoOk = FALSE;
	p->bSetupOk = FALSE;

	HRESULT hrCo = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	char szTempVerzeichnis[MAX_PATH], szTempDatei[MAX_PATH];
	if (GetTempPath(sizeof(szTempVerzeichnis), szTempVerzeichnis) && GetTempFileName(szTempVerzeichnis, "ect", 0, szTempDatei))
	{
		if (SUCCEEDED(LadeHerunter(PROGRAMMUPDATE_INFO_URL, szTempDatei)))
		{
			FILE *f = fopen(szTempDatei, "rb");
			if (f)
			{
				char buffer[4096];
				size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
				fclose(f);
				buffer[n] = '\0';
				p->bInfoOk = ParseUpdateInfo(buffer, p->nVerfuegbar, p->csAenderung);
			}
		}
		DeleteFile(szTempDatei);
	}

	if (SUCCEEDED(hrCo)) CoUninitialize();

	p->csSetupPfad = SetupPfad();
	p->bSetupOk = HoleDateiVersion(p->csSetupPfad, p->nSetup);

	TRACE("ProgrammUpdate: Info %s, Setup %s\n", p->bInfoOk ? (LPCSTR)VersionText(p->nVerfuegbar) : "nicht abrufbar",
		p->bSetupOk ? (LPCSTR)VersionText(p->nSetup) : "nicht vorhanden");

	if (!::IsWindow(hwnd) || !::PostMessage(hwnd, WM_PROGRAMMUPDATE_GEPRUEFT, 0, (LPARAM)p))
		delete p;
	return 0;
}

static UINT __cdecl DownloadThread(LPVOID pParam)
{
	ProgrammUpdateDownloadAuftrag *p = (ProgrammUpdateDownloadAuftrag *)pParam;
	p->bOk = FALSE;

	HRESULT hrCo = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// erst unter Hilfsnamen laden: ein abgebrochener Download darf kein halbes Setup hinterlassen
	CString csTemp = p->csSetupPfad + ".download";
	DeleteFile(csTemp);
	HRESULT hr = LadeHerunter(PROGRAMMUPDATE_SETUP_URL, csTemp);
	if (SUCCEEDED(hr))
	{
		int v[3];
		if (!HoleDateiVersion(csTemp, v))
			p->csFehler = "Die heruntergeladene Datei ist kein gültiges Setup.";
		else if (VergleicheVersion(v, p->nErwartet) < 0)
			p->csFehler.Format("Auf dem Server liegt noch das Setup für %s statt %s. Bitte später noch einmal versuchen.",
				(LPCSTR)VersionText(v), (LPCSTR)VersionText(p->nErwartet));
		else if (!MoveFileEx(csTemp, p->csSetupPfad, MOVEFILE_REPLACE_EXISTING))
			p->csFehler.Format("Die Datei '%s' konnte nicht geschrieben werden (Fehler %lu).", (LPCSTR)p->csSetupPfad, GetLastError());
		else
			p->bOk = TRUE;
	}
	else
		p->csFehler.Format("Download von " PROGRAMMUPDATE_SETUP_URL " fehlgeschlagen (Fehlercode 0x%08lX).", (unsigned long)hr);

	if (!p->bOk) DeleteFile(csTemp);

	if (SUCCEEDED(hrCo)) CoUninitialize();

	if (!::IsWindow(p->hwnd) || !::PostMessage(p->hwnd, WM_PROGRAMMUPDATE_HERUNTERGELADEN, 0, (LPARAM)p))
		delete p;
	return 0;
}

// ----------------------------------------------------------
// öffentliche Funktionen

void ProgrammUpdatePruefungStarten(HWND hwndHauptfenster)
{
	if (!hwndHauptfenster) return;
	AfxBeginThread(PruefThread, (LPVOID)hwndHauptfenster, THREAD_PRIORITY_BELOW_NORMAL);
}

LRESULT ProgrammUpdateGeprueft(LPARAM lParam)
{
	ProgrammUpdatePruefErgebnis *p = (ProgrammUpdatePruefErgebnis *)lParam;
	if (!p) return 0;

	int nInstalliert[3];
	if (!ParseVersion(version_string_exakt, nInstalliert))
	{
		delete p;
		return 0;
	}

	int nSkip[3];
	BOOL bSkip = ParseVersion(ECT_HoleEinstellung(SKIP_VERSION_KEY), nSkip);

	// Setup-Datei, die nicht neuer als die installierte Version ist: Rest einer erledigten Installation
	BOOL bSetupNeuer = p->bSetupOk && VergleicheVersion(p->nSetup, nInstalliert) > 0;
	if (p->bSetupOk && !bSetupNeuer)
		DeleteFile(p->csSetupPfad);

	if (p->bInfoOk && VergleicheVersion(p->nVerfuegbar, nInstalliert) > 0)
	{
		if (bSkip && VergleicheVersion(p->nVerfuegbar, nSkip) == 0)
			;	// Anwender hat genau diese Version übersprungen
		else if (bSetupNeuer && VergleicheVersion(p->nSetup, p->nVerfuegbar) >= 0)
		{
			s_csVerfuegbar = VersionText(p->nVerfuegbar);
			s_csAenderung = p->csAenderung;
			FrageInstallation(p->csSetupPfad, p->nSetup);
		}
		else
		{
			s_csVerfuegbar = VersionText(p->nVerfuegbar);
			s_csAenderung = p->csAenderung;
			ZeigeUpdateMeldung("", "Installieren");
		}
	}
	else if (!p->bInfoOk && bSetupNeuer && !(bSkip && VergleicheVersion(p->nSetup, nSkip) == 0))
	{	// Server nicht erreichbar, aber ein neueres Setup wurde schon früher heruntergeladen
		FrageInstallation(p->csSetupPfad, p->nSetup);
	}

	delete p;
	return 0;
}

void ProgrammUpdateInstallieren()
{
	int nVerfuegbar[3];
	if (s_bDownloadLaeuft || !ParseVersion(s_csVerfuegbar, nVerfuegbar)) return;

	CString csSetupPfad = SetupPfad();
	int nSetup[3];
	if (HoleDateiVersion(csSetupPfad, nSetup) && VergleicheVersion(nSetup, nVerfuegbar) >= 0)
	{	// schon da: Beenden über eine gepostete Nachricht, wir stecken noch im Klick der CaptionBar
		s_csSetupZumBeenden = csSetupPfad;
		AfxGetMainWnd()->PostMessage(WM_PROGRAMMUPDATE_BEENDEN);
		return;
	}

	// fehlt, ist älter oder Version nicht lesbar: (neu) herunterladen und überschreiben
	ProgrammUpdateDownloadAuftrag *p = new ProgrammUpdateDownloadAuftrag;
	p->hwnd = AfxGetMainWnd()->GetSafeHwnd();
	memcpy(p->nErwartet, nVerfuegbar, sizeof(nVerfuegbar));
	p->csSetupPfad = csSetupPfad;

	s_bDownloadLaeuft = TRUE;
	theApp.ProgrammCaptionBox("Update " + s_csVerfuegbar + " wird heruntergeladen. Danach wird EC&&T beendet und das Setup gestartet...",
		0, NULL, NULL, 0, NULL);
	AfxBeginThread(DownloadThread, (LPVOID)p, THREAD_PRIORITY_BELOW_NORMAL);
}

LRESULT ProgrammUpdateHeruntergeladen(LPARAM lParam)
{
	ProgrammUpdateDownloadAuftrag *p = (ProgrammUpdateDownloadAuftrag *)lParam;
	if (!p) return 0;
	s_bDownloadLaeuft = FALSE;

	if (p->bOk)
		BeendenUndSetupStarten(p->csSetupPfad);
	else
		ZeigeUpdateMeldung(FuerCaptionBar(p->csFehler) + " ", "Erneut versuchen");

	delete p;
	return 0;
}

LRESULT ProgrammUpdateBeenden()
{
	CString csSetupPfad = s_csSetupZumBeenden;
	s_csSetupZumBeenden.Empty();
	if (!csSetupPfad.IsEmpty())
		BeendenUndSetupStarten(csSetupPfad);
	return 0;
}

void ProgrammUpdateUeberspringen()
{
	if (s_bDownloadLaeuft || s_csVerfuegbar.IsEmpty()) return;
	ECT_SpeichereEinstellung(SKIP_VERSION_KEY, s_csVerfuegbar);
	theApp.CaptionBoxHide();
}

void ProgrammUpdateSetupNachProgrammende()
{
	if (s_csSetupNachProgrammende.IsEmpty()) return;

	CString csVerzeichnis = s_csSetupNachProgrammende.Left(s_csSetupNachProgrammende.ReverseFind('\\'));
	// ShellExecute statt CreateProcess: das Setup braucht Adminrechte (UAC-Abfrage)
	HINSTANCE h = ShellExecute(NULL, "open", s_csSetupNachProgrammende, NULL, csVerzeichnis, SW_SHOWNORMAL);
	if ((INT_PTR)h <= 32)
		::MessageBox(NULL, "Das Setup konnte nicht gestartet werden. Bitte die Datei '" + s_csSetupNachProgrammende +
			"' manuell ausführen. Hinweis: 'C:\\ProgramData' ist normalerweise ein verstecktes Verzeichnis, in das man nur gelangt, "
			"wenn man den Verzeichnisnamen oben in die Navigationszeile des Windows-Explorers eintippt.",
			"EasyCash&Tax", MB_ICONEXCLAMATION | MB_OK);
	s_csSetupNachProgrammende.Empty();
}
