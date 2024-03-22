// EasyCash.cpp : Defines the class behaviors for the application.
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2020  Thomas Mielke
// 
// Dies ist freie Software; Sie dürfen sie unter den Bedingungen der 
// GNU General Public License, wie von der Free Software Foundation 
// veröffentlicht, weiterverteilen und/oder modifizieren; entweder gemäß 
// Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA. 

#include "stdafx.h"
#include "EasyCash.h"

#include "ExtSplitter.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "ECTIFace\EasyCashDoc.h"
#include "BuchenDlg.h"
#include "EasyCashView.h"
#include "RegistrierenDlg.h"
#include "RegDlg.h"
#include "GetProgramVersion.h"
#include "IconAuswahlMandant.h"
#include "XFolderDialog.h"
#if defined(NDEBUG)
#include "CrashRpt.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEasyCashApp

BEGIN_MESSAGE_MAP(CEasyCashApp, CWinAppEx)
	ON_COMMAND(CG_IDS_TIPOFTHEDAY, ShowTipOfTheDay)
	//{{AFX_MSG_MAP(CEasyCashApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyCashApp construction

CEasyCashApp::CEasyCashApp()
{
	m_bCrashReportingAktiv = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CEasyCashApp object

CEasyCashApp theApp;

CComModule _Module;


// ein paar globale Variablen:
char last_path[500];
char reg[100];
char reg_name[100];
BOOL vollversion;

/////////////////////////////////////////////////////////////////////////////
// CEasyCashApp initialization

char version_string[50];
char version_string_exakt[50];
char app_name_und_version_string_exakt[100];


BOOL CEasyCashApp::InitInstance()
{
	extern BOOL CheckReg(char *);
	extern void GetRegName(char *, char *);
	char last_file[300];
	CString csInitalStatusText;
	
	HRESULT hrCoInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	switch (hrCoInit)
	{
	case S_FALSE:
		AfxMessageBox("CoInitializeEx(): Die COM-Bibliothek wurde bereits in diesem Thread initialisiert."); break;
	case RPC_E_CHANGED_MODE:
		AfxMessageBox("CoInitializeEx(): Ein vorheriger Aufruf von CoInitializeEx hat das Parallelitätsmodell für diesen Thread als Multithread-Apartment (MTA) angegeben."); break;
	case E_INVALIDARG:
		AfxMessageBox("CoInitializeEx(): ungültiger Parameter"); break;
	case E_OUTOFMEMORY:
		AfxMessageBox("CoInitializeEx(): Speicher voll"); break;
	case E_UNEXPECTED:
		AfxMessageBox("CoInitializeEx(): unerwarteter Fehler"); break;
	case S_OK:
	default:
		;
		// Die COM-Bibliothek wurde in diesem Thread erfolgreich initialisiert.
	}

	//if (!AfxOleInit())  // führte mal zu heap corruption, deshalb lieber CoInitialize()
	//	DSAMessageBox(IDS_OLEINIT);
	// Version abholen
	HMODULE hExe = GetModuleHandle("EASYCT.EXE");
	_Module.Init(NULL, hExe);
	
	{
		VS_FIXEDFILEINFO vs_ffi;
		if (hExe) 
		{
			GetProgramVerion(hExe, &vs_ffi);
			sprintf(version_string, "v%0lu.%0lu", vs_ffi.dwProductVersionMS >> 16, vs_ffi.dwProductVersionMS & 0xffff);
			sprintf(version_string_exakt, "v%0lu.%0lu.%0lu.%0lu", vs_ffi.dwProductVersionMS >> 16, vs_ffi.dwProductVersionMS & 0xffff
				, vs_ffi.dwProductVersionLS >> 16, vs_ffi.dwProductVersionLS & 0xffff);
		}
		else 
		{
			*version_string = '\0';
			*version_string_exakt = '\0';
		}
	}
	 
	strcpy(app_name_und_version_string_exakt, "EasyCash&Tax ");
	strcat(app_name_und_version_string_exakt, version_string_exakt);

#if defined(NDEBUG)
	/*if (*m_lpCmdLine == '/' && toupper(m_lpCmdLine[1]) == 'D')*/ m_bCrashReportingAktiv = TRUE;

	// Install crash reporting
	if (m_bCrashReportingAktiv)
	{
		CR_INSTALL_INFO info;
		memset(&info, 0, sizeof(CR_INSTALL_INFO));
		info.cb = sizeof(CR_INSTALL_INFO); 
		info.pszAppName = app_name_und_version_string_exakt; // Define application name.
		info.pszAppVersion = version_string_exakt; // Define application version.
		info.uPriorities[CR_HTTP] = 2;
		info.uPriorities[CR_SMAPI] = 1;
		info.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;
		info.pszEmailTo = "thomas@mielke.software";
		info.pszEmailSubject = "EC&T Absturzbericht";
		info.pszUrl = _T("https://www.easyct.de/crashrpt.php");	// URL for sending error reports over HTTP.                
		// Install all available exception handlers.
		info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS; 
		// Use binary encoding for HTTP uploads (recommended).
		info.dwFlags |= CR_INST_HTTP_BINARY_ENCODING;     
		// Provide privacy policy URL
		//info.pszPrivacyPolicyURL = _T("http://easyct.de/privacy.html");
		info.dwFlags |= CR_INST_SHOW_ADDITIONAL_INFO_FIELDS;	// EMail und Beschreibung per default einblenden
		// info.dwFlags |= CR_INST_ALLOW_ATTACH_MORE_FILES;		// weitere Anhänge erlauben

		int nResult = crInstall(&info);
		if(nResult!=0)
		{
			TCHAR buff[256];
			crGetLastErrorMsg(buff, 256);
			MessageBox(NULL, buff, _T("Crash Report Installationsfehler"), MB_OK);
			return 1;
		}

		// Take screenshot of the app window at the moment of crash
		crAddScreenshot2(CR_AS_MAIN_WINDOW|CR_AS_USE_JPEG_FORMAT, 95);
	}
#endif

	// Don't Show Again Messagebox
	DSASetModule(hExe);
	DSASetRegKey("SOFTWARE\\Thomas Mielke Softwareentwicklung\\EasyCash&Tax\\DSA");

	AtlAxWinInit();

	AfxEnableControlContainer();

	// alten Ini-Kram ausmisten (C:\Windows\Easyct.ini) und in die Registry damit!
	{
		CString Windowsverzeichnis, csKey;
		if (!GetWindowsDirectory(Windowsverzeichnis.GetBuffer(MAX_PATH), MAX_PATH))
			Windowsverzeichnis = "C:\\Windows";
		Windowsverzeichnis.ReleaseBuffer();
		Windowsverzeichnis = Windowsverzeichnis + "\\easyct.ini";
		DWORD dw = GetFileAttributes(Windowsverzeichnis);
		if (dw != 0xFFFFFFFF)
		{
			AfxMessageBox("Hinweis: EC&T wird ab v1.58 globale Daten wie die Registriernummer, Mandanten und die Position des Datenverzeichnisses nicht mehr in einer zweiten easyct.ini Datei im Windows-Verzeichnis speichern (zu unterscheinden von der easyct.ini im Datenverzeichnis), sondern in der Windows-Registrierungsdatenbank (Registry). Das hat den Grund darin, dass Windows nicht möchte, dass Programme Daten im Windows-Verzeichnis speichern, und deshalb die ini tatsächlich unter C:\\Benutzer\\<Account-Name>\\AppData\\Local\\VirtualStore\\Windows speichert, wobei es manchmal dazu kommt, dass Windows diese ini-Datei einfach 'vergisst'. Nach der Übertragung in die Registry wird die obsolete easyct.ini im Windows-Verzeichnis gelöscht. Es besteht aber im Datei-Menüpunkt unter 'Registrierungsinformation sichern' die Möglichkeit, die in der Registrierung gespeicherten Informationen als Datei auf ein externes Medium zu speichern.");

			// ___ Master-Einstellungsdaten aus ini lesen ___
			//
			// Allgemein
			CString Datenverzeichnis = GetProfileString("Allgemein", "Datenverzeichnis");
			CString RegKey			 = GetProfileString("Allgemein", "RegKey");
			// Tip
			int		Startup    = GetProfileInt("Tip", "StartUp", 0);
			int		FilePos    = GetProfileInt("Tip", "FilePos", 0);
			CString TimeStamp  = GetProfileString("Tip", "TimeStamp", NULL);
			// FolderHistory 
			CString Folder0000 = GetProfileString("FolderHistory", "Folder0000");
			CString Folder	   = GetProfileString("FolderHistory", "Folder");
			// XFolderDialog
			int		ViewMode   = GetProfileInt("XFolderDialog", "ViewMode", 0);
			// Mandanten
			CString csNameKey[100], csName[100], csPropertyKey[100], csProperty[100], csIconKey[100], csIcon[100];
			int i, nMandanten = 0;
			for (i = 0; i < 100; i++)
			{
				csNameKey[i].Format("Mandant%-02.2dName", i);
				csName[i] = GetProfileString("Mandanten", csNameKey[i].GetBuffer(0), "");
				csNameKey[i].ReleaseBuffer();
				csPropertyKey[i].Format("Mandant%-02.2dDatenverzeichnis", i);
				csProperty[i] = GetProfileString("Mandanten", csPropertyKey[i].GetBuffer(0), "");
				csPropertyKey[i].ReleaseBuffer();
				csIconKey[i].Format("Mandant%-02.2dIcon", i);
				csIcon[i] = GetProfileString("Mandanten", csIconKey[i].GetBuffer(0), "0");
				csIconKey[i].ReleaseBuffer();

				if (!csName[i].IsEmpty() || !csPropertyKey[i].IsEmpty()) nMandanten++;
			}

			// ___ bisherige Registry-Daten aus HKCU\\SOFTWARE\\Tools\\EasyCash lesen ___
			//
			int BuchenPosX = 0, BuchenPosY = 0;
			int FrameWndPosX = 0, FrameWndPosY = 0;
			int FrameWndSizeX = 640, FrameWndSizeY = 480;
			CString DSA61447;
			{
				HKEY hKey;
				char buffer2[256];

				if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Tools\\EasyCash", &hKey) == ERROR_SUCCESS)
				{
					long lRetCode, lDummy;
					long lType = REG_SZ;
					long lCb = sizeof(buffer2);
					lDummy = 0L;
					
					lRetCode = RegQueryValueEx(hKey, TEXT("BuchenPosX"), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) BuchenPosX = atoi(buffer2);

					lCb = sizeof(buffer2);
					lRetCode = RegQueryValueEx(hKey, TEXT("BuchenPosY"), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) BuchenPosY = atoi(buffer2);
					
					lCb = sizeof(buffer2);
					lRetCode = RegQueryValueEx(hKey, TEXT("FrameWndPosX"), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) FrameWndPosX = atoi(buffer2);

					lCb = sizeof(buffer2);
					lRetCode = RegQueryValueEx(hKey, TEXT("FrameWndPosY"), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) FrameWndPosY = atoi(buffer2);
					
					lCb = sizeof(buffer2);
					lRetCode = RegQueryValueEx(hKey, TEXT("FrameWndSizeX"), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) FrameWndSizeX = atoi(buffer2);

					lCb = sizeof(buffer2);
					lRetCode = RegQueryValueEx(hKey, TEXT("FrameWndSizeY"), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) FrameWndSizeY = atoi(buffer2);
					
					lCb = sizeof(buffer2);
					lRetCode = RegQueryValueEx(hKey, TEXT("DSA61447"), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) DSA61447 = buffer2;

					RegCloseKey(hKey);
				}
			}

			// ___ auf Registry umschalten zum schreiben ___
			SetRegistryKey("Thomas Mielke Softwareentwicklung");
			
			// ___ checken, ob schon mal eine Übernahme in die Registry stattgefunden hat ___
			CString DatenverzeichnisDoppelt = GetProfileString("Allgemein", "Datenverzeichnis");
			CString RegKeyDoppelt			= GetProfileString("Allgemein", "RegKey");
			if (!RegKeyDoppelt.IsEmpty()) 
				RegKeyDoppelt = " und dem Registriercode " + RegKeyDoppelt;

			if (DatenverzeichnisDoppelt.IsEmpty() || AfxMessageBox("Ups! Es gibt bereits übernommene Daten in der Registry mit dem Datenverzeichnis '" + DatenverzeichnisDoppelt + "'" + RegKeyDoppelt + ". Die esayct.ini im Windows-Verzeichnis ist nach einer ersten Übernahme wieder aufgetaucht -- oder wurde gar nicht erst richtig gelöscht. Sollen die Registry-Daten wirklich noch einmal überschrieben werden? (Nicht empfohlen, wenn das schon in der Registry gespeicherte Datenverzeichnis '" + DatenverzeichnisDoppelt + "' gut aussieht gegenüber dem in 'C:\\Windows\\EasyCT.ini' angegebenen '" + Datenverzeichnis + "'.)", MB_YESNO) == IDYES)
			{
				// ___ Master-Einstellungsdaten in Registry schreiben ___
				//
				// Allgemein
				if (!Datenverzeichnis.IsEmpty()) WriteProfileString("Allgemein", "Datenverzeichnis", Datenverzeichnis);
				if (!RegKey.IsEmpty()) WriteProfileString("Allgemein", "RegKey", RegKey);
				// Tip
				if (Startup) WriteProfileInt("Tip", "StartUp", Startup);
				if (FilePos) WriteProfileInt("Tip", "FilePos", FilePos);
				if (!TimeStamp.IsEmpty()) WriteProfileString("Tip", "TimeStamp", TimeStamp);
				// FolderHistory 
				if (!Folder0000.IsEmpty()) WriteProfileString("FolderHistory", "Folder0000", Folder0000);
				if (!Folder.IsEmpty()) WriteProfileString("FolderHistory", "Folder", Folder);
				// XFolderDialog
				if (ViewMode) WriteProfileInt("XFolderDialog", "ViewMode", ViewMode);
				// Fenster
				if (BuchenPosX) WriteProfileInt("Fenster", "BuchenPosX", BuchenPosX);
				if (BuchenPosY) WriteProfileInt("Fenster", "BuchenPosY", BuchenPosY);
				if (FrameWndPosX) WriteProfileInt("Fenster", "FrameWndPosX", FrameWndPosX);
				if (FrameWndPosY) WriteProfileInt("Fenster", "FrameWndPosY", FrameWndPosY);
				if (FrameWndSizeX) WriteProfileInt("Fenster", "FrameWndSizeX", FrameWndSizeX);
				if (FrameWndSizeY) WriteProfileInt("Fenster", "FrameWndSizeY", FrameWndSizeY);
				// DSA
				if (DSA61447) WriteProfileString("DSA", "DSA61447", DSA61447);
				// Mandanten
				if (nMandanten)
					for (i = 0; i < 100; i++)
					{
						WriteProfileString("Mandanten", csNameKey[i].GetBuffer(0), csName[i]);
						WriteProfileString("Mandanten", csPropertyKey[i].GetBuffer(0), csProperty[i]);
						WriteProfileString("Mandanten", csIconKey[i].GetBuffer(0), csIcon[i]);
					}
			}

			// zum Schluss c:\Windows\easyct.ini löschen
			if (!DeleteFile(Windowsverzeichnis))
				AfxMessageBox("Das Löschen von '" + Windowsverzeichnis + "' schlug fehl. Bitte diese Datei manuell löschen, da sonst keine Änderungen etwa an Mandanten oder dem Datenverzeichnispfad möglich sind. Ggf. sind dafür Administrator-Privilegien nötig.");

		}
		else
			SetRegistryKey("Thomas Mielke Softwareentwicklung");
	}	
	
	LoadStdProfileSettings(20);  // Load standard INI file options (including MRU)
	if (m_pRecentFileList)
	{	// MRU file list nur mit im Datenverzeichnis befindlichen Dateien füllen
		CString Datenverzeichnis = GetProfileString("Allgemein", "Datenverzeichnis");
		if (!Datenverzeichnis.IsEmpty())
		{
			int i;
			for (i = 0; i < m_pRecentFileList->GetSize() && !m_pRecentFileList->m_arrNames[i].IsEmpty(); i++)
			{
				if (m_pRecentFileList->m_arrNames[i].Left(Datenverzeichnis.GetLength() + 1) != Datenverzeichnis + "\\"
					|| m_pRecentFileList->m_arrNames[i].Find('\\', Datenverzeichnis.GetLength() + 1) > 0)
				{
					m_pRecentFileList->Remove(i--);
				}
			}
		}	
	}
	SetRegistryBase (_T("Settings"));
	
	// Initialize all Managers for usage. They are automatically constructed
	// if not yet present
	InitContextMenuManager();
	InitKeyboardManager();

	// TODO: Remove this if you don't want extended tooltips:
	InitTooltipManager();

	CMFCToolTipInfo params;
	params.m_bVislManagerTheme = TRUE;

	theApp.GetTooltipManager ()->SetTooltipParams (
		AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl),
		&params);

	// Um Verhalten zu steuern je nach Mandanten/Nicht-Mandantenmodus
	CString Mandant0Existiert = theApp.GetProfileString("Mandanten", "Mandant00Datenverzeichnis", "");

//	if (Mandant0Existiert.IsEmpty())
//	{	// Nicht-Mandantenmodus: Lade die normalen 4 MRUs
//		LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
//	}
//	else
	{	// Mandantenmodus: Spezialbehandlung in CMainFrame und CEasyCashView: Alle .eca-Dateien aus dem Mandantenverzeichnis ins Menü einbauen
//		LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)
	}

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CMultiDocTemplate* pDocTemplate;		// CMultiDocTemplate erlaubt mehrere Dateiendungen, aber nur die erste ist wirklich Shell-Registriert :(
	pDocTemplate = new CMyMultiDocTemplate( 
		IDR_EASYCATYPE,
		RUNTIME_CLASS(CEasyCashDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CEasyCashView));
	AddDocTemplate(pDocTemplate);

	CLoadExtensionDLLs(version_string);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;

	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	m_pMainWnd = pMainFrame;
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// keine Mandanten vorhanden: trivial, einfach Datenverzeichnis entspr. Registry setzen -- ansonsten: siehe Parse command line direkt unten
	CString csDatenverzeichnis;
	if (Mandant0Existiert.IsEmpty())
	{
		csDatenverzeichnis = theApp.GetProfileString("Allgemein", "Datenverzeichnis");	// keine Mandanten? Dann aus der Allgemein Sektion des Windows-Verzeichnisses nehmen!
		SetMandant(-1);
	}

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;

	if (!strncmp(m_lpCmdLine, "/P=", 3) && !strchr(m_lpCmdLine, ' '))	// nur /P-Option mit Pluginnamen angegeben?
	{
		m_csStartupPlugin = m_lpCmdLine + 3;							// Startup-Plugin setzen und
		m_lpCmdLine[0] = '\0';											// im "kein Dateiname angegeben"-Modus verarbeiten
	}

	if (m_lpCmdLine[0] == '\0')
	{
		// Mandanten auswählen, wenn vorhanden --> Datenverzeichnis setzen
		// Dies ist nötig, wenn kein Parameter übergeben wurde, aus dem auf das Datenverzeichnis geschlossen werden kann
		CIconAuswahlMandant dlgIcon;
		CString csKey;
		if (!Mandant0Existiert.IsEmpty())
		{
			dlgIcon.m_nModus = 1;
			dlgIcon.DoModal();
			if (dlgIcon.m_nSelected == -1) return FALSE;
			csKey.Format("Mandant%-02.2dDatenverzeichnis", dlgIcon.m_nSelected);
			csDatenverzeichnis = theApp.GetProfileString("Mandanten", csKey, "");
			if (!csDatenverzeichnis.IsEmpty())
				theApp.WriteProfileString("Allgemein", "Datenverzeichnis", csDatenverzeichnis);	
			SetMandant(dlgIcon.m_nSelected);
		}
		char szIniFileName[500];
		strcpy(szIniFileName, csDatenverzeichnis.GetBuffer(0));
		strcat(szIniFileName, "\\easyct.ini");
		SetIniFileName(szIniFileName);

		GetPrivateProfileString("Allgemein", "LetzteDatei", "", last_file, sizeof(last_file), szIniFileName);
		if (!*last_file && Mandant0Existiert.IsEmpty()) 
			GetPrivateProfileString("Allgemein", "LetzteDatei", "", last_file, sizeof(last_file), "EasyCash.ini");
		if (last_file && GetFileAttributes(last_file) != 0xFFFFFFFF)
		{
			cmdInfo.m_strFileName = last_file;
			cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
		}
	}	
	else
	{
		ParseCommandLine(cmdInfo);

		CString csKey, csDatenverzeichnis;
		if (!Mandant0Existiert.IsEmpty())
		{
			// weil es einen Dateinamen als Parameter gibt, können wir versuchen das Datenverzeichnis daraus abzuleiten
			int nPathsMatching = 0;
			int nFirstPathMatching = -1;
			CString csFilePath = cmdInfo.m_strFileName;
			char *cp = strrchr(csFilePath.GetBuffer(0), '\\');
			if (cp)
			{
				*cp = '\0';
				int i;
				for (i = 0; i < 100; i++)
				{
					csKey.Format("Mandant%-02.2dDatenverzeichnis", i);
					csDatenverzeichnis = theApp.GetProfileString("Mandanten", csKey, "");
					if (csDatenverzeichnis.IsEmpty()) 
						break;
					if (!stricmp(csDatenverzeichnis, csFilePath))
					{
						nPathsMatching++;
						if (nFirstPathMatching == -1) nFirstPathMatching = i;
					}
				}
			}
				
			if (nPathsMatching != 1)
			{	// wenn keiner oder mehrere Mandantenpfade mit dem der Datei übereinstimmten, doch Auswahldialog öffnen
				CIconAuswahlMandant dlgIcon;
				dlgIcon.m_nModus = 1;
				dlgIcon.DoModal();
				if (dlgIcon.m_nSelected == -1) return FALSE;
				csKey.Format("Mandant%-02.2dDatenverzeichnis", dlgIcon.m_nSelected);
			}
			else
			{
				csKey.Format("Mandant%-02.2dName", nFirstPathMatching);
				CString csMandantName = theApp.GetProfileString("Mandanten", csKey, "");
				csInitalStatusText = "Für Datei " + cmdInfo.m_strFileName + " wurde implizit Mandant '" + csMandantName + "' ausgewählt.";
				csKey.Format("Mandant%-02.2dDatenverzeichnis", nFirstPathMatching);
			}

			csDatenverzeichnis = theApp.GetProfileString("Mandanten", csKey, "");
			if (!csDatenverzeichnis.IsEmpty())
				theApp.WriteProfileString("Allgemein", "Datenverzeichnis", csDatenverzeichnis);	
			SetMandant(nFirstPathMatching);
		}
		char szIniFileName[500];
		strcpy(szIniFileName, csDatenverzeichnis.GetBuffer(0));
		strcat(szIniFileName, "\\easyct.ini");
		SetIniFileName(szIniFileName);
	}

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
	{
		/*if (AfxMessageBox("Soll EasyCash mit einem leeren Dokument gestartet werden?",
			MB_ICONQUESTION|MB_YESNO) == IDYES)
		{
			cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
			ProcessShellCommand(cmdInfo);
		}
		else
			return FALSE;*/
		return TRUE;
	}

	// Registrierungs-Kram
	CString csReg = theApp.GetProfileString("Allgemein", "RegKey", "");
	csReg = csReg.Left(99);
	strcpy (reg, csReg);

	if (!CheckReg(reg))
	{
		shareware = TRUE;
		strcpy(reg_name, "unregistrierte Version");
	}
	else
	{
		char regstr[100];

		shareware = FALSE;
		
		GetRegName(regstr, reg);
		if (*regstr == '#')
		{
			strcpy(reg_name, regstr+1);
			vollversion = FALSE;
		}
		else
		{
			strcpy(reg_name, regstr);
			vollversion = TRUE;
		}
	}

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	if (!csInitalStatusText.IsEmpty()) pMainFrame->SetStatus(csInitalStatusText);

	// CG: This line inserted by 'Tip of the Day' component.
	ShowTipAtStartup();

	return TRUE;
}

void CEasyCashApp::ParseCommandLine(CCommandLineInfo& rCmdInfo)
{
	for (int i = 1; i < __argc; i++)
	{
		LPCTSTR pszParam = __targv[i];
		if (!strncmp(pszParam, "/P=", 3))
		{
			// bei Programmstart als Parameter angegebenes Plugin öffnen
			m_csStartupPlugin = pszParam+3;
		}
	}

	CWinAppEx::ParseCommandLine(rCmdInfo);
}

int CEasyCashApp::ExitInstance() 
{
	// ExtensionDLLs Exit-Hook
	CIterateExtensionDLLs("ECTE_Exit", NULL);

	CFreeExtensionDLLs();

	_Module.Term();
	
	AtlAxWinTerm();

	CoUninitialize();  // kann bei AfxOleInit() weggelassen werden: wird dann automatisch erledigt. AfxOleInit() führte aber zu heap corruptions

#if defined(NDEBUG)
	// Uninstall crash reporting
	if (m_bCrashReportingAktiv) crUninstall();
#endif

	return CWinAppEx::ExitInstance();
}

void CEasyCashApp::RegistrierungsinformationenSichern(char *DateinameParam)
{	
	extern char reg_name[100];

	char Dateiname[1000];
	*Dateiname = '\0';

	if (DateinameParam)
		strcpy(Dateiname, DateinameParam);
	else
	{
		if (!GetIniFileName(Dateiname, sizeof(Dateiname))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); return; }
		char *cp;
		if (cp = strrchr(Dateiname, '\\'))
		{
			if (*reg_name)
				strcpy(cp+1, reg_name);
			else
				strcpy(cp+1, "EC&T-Registrierungsinformationen");
		}
		else
			strcpy(Dateiname, "EC&T-Registrierungsinformationen");
		while (strlen(Dateiname))
			if (Dateiname[strlen(Dateiname)-1] == ' ')
				Dateiname[strlen(Dateiname)-1] = '\0';
			else 
				break;
		strcat(Dateiname, ".ectreg");
		CFileDialog cf(FALSE, "*.ectreg", Dateiname, 0, "EasyCash&Tax Registrierungsinformationen (*.ectreg)|*.ectreg||",AfxGetMainWnd());
		if (cf.DoModal() != IDOK)
			return;
		strcpy(Dateiname, cf.GetPathName().GetBuffer(0));
	}

	// ___ Registry-Daten in Datei sichern ___
	int nCount = 0;

	// Allgemein
	CString Datenverzeichnis = theApp.GetProfileString("Allgemein", "Datenverzeichnis", "");
	if (!Datenverzeichnis.IsEmpty()) nCount++;
	CString RegKey = theApp.GetProfileString("Allgemein", "RegKey", "");
	if (!RegKey.IsEmpty()) nCount++; 

	// Tip
	int		Startup    = theApp.GetProfileInt("Tip", "StartUp", 0);
	if (Startup) nCount++;
	int		FilePos    = theApp.GetProfileInt("Tip", "FilePos", 0);
	if (FilePos) nCount++;
	CString TimeStamp = theApp.GetProfileString("Tip", "TimeStamp", NULL);
	if (!TimeStamp.IsEmpty()) nCount++;
	// FolderHistory 
	CString Folder0000 = theApp.GetProfileString("FolderHistory", "Folder0000", "");
	if (!Folder0000.IsEmpty()) nCount++;
	CString Folder = theApp.GetProfileString("FolderHistory", "Folder", "");
	if (!Folder.IsEmpty()) nCount++;

	// XFolderDialog
	int		ViewMode   = theApp.GetProfileInt("XFolderDialog", "ViewMode", 0);
	if (ViewMode) nCount++;

	// Mandanten
	CString csNameKey[100], csName[100], csPropertyKey[100], csProperty[100], csIconKey[100], csIcon[100];
	int i, nMandanten = 0;
	for (i = 0; i < 100; i++)
	{
		csNameKey[i].Format("Mandant%-02.2dName", i);
		csName[i] = theApp.GetProfileString("Mandanten", csNameKey[i].GetBuffer(0), "");
		csName[i].ReleaseBuffer();
		if (!csName[i].IsEmpty()) nCount++;
		csPropertyKey[i].Format("Mandant%-02.2dDatenverzeichnis", i);
		csProperty[i] = theApp.GetProfileString("Mandanten", csPropertyKey[i].GetBuffer(0), "");
		csProperty[i].ReleaseBuffer();
		if (!csProperty[i].IsEmpty()) nCount++;
		csIconKey[i].Format("Mandant%-02.2dIcon", i);
		csIcon[i] = theApp.GetProfileString("Mandanten", csIconKey[i].GetBuffer(0), "0");
		csIcon[i].ReleaseBuffer();		
		if (csIcon[i] != "0") nCount++;

		if (!csName[i].IsEmpty() || !csPropertyKey[i].IsEmpty()) nMandanten++;
	}

	// Fenster
	int BuchenPosX, BuchenPosY;
	int FrameWndPosX, FrameWndPosY;
	int FrameWndSizeX, FrameWndSizeY;
	BuchenPosX = theApp.GetProfileInt("Fenster", "BuchenPosX", 0);
	if (BuchenPosX) nCount++;
	BuchenPosY = theApp.GetProfileInt("Fenster", "BuchenPosY", 0);
	if (BuchenPosY) nCount++;
	FrameWndPosX = theApp.GetProfileInt("Fenster", "FrameWndPosX", 0);
	if (FrameWndPosX) nCount++;
	FrameWndPosY = theApp.GetProfileInt("Fenster", "FrameWndPosY", 0);
	if (FrameWndPosY) nCount++;
	FrameWndSizeX = theApp.GetProfileInt("Fenster", "FrameWndSizeX", 0);
	if (FrameWndSizeX) nCount++;
	FrameWndSizeY = theApp.GetProfileInt("Fenster", "FrameWndSizeY", 0);
	if (FrameWndSizeY) nCount++;

	// DSA
	CString DSA61447;
	DSA61447 = theApp.GetProfileString("DSA", "DSA61447", "");
	if (!DSA61447.IsEmpty()) nCount++;

	// ___ in Datei schreiben ___
	DeleteFile(Dateiname);
	if (!Datenverzeichnis.IsEmpty()) WritePrivateProfileString("Allgemein", "Datenverzeichnis", Datenverzeichnis, Dateiname);
	if (!RegKey.IsEmpty()) WritePrivateProfileString("Allgemein", "RegKey", RegKey, Dateiname);
	// Tip
	if (Startup) { CString csInt2String; csInt2String.Format("%d", Startup); WritePrivateProfileString("Tip", "StartUp", csInt2String, Dateiname); }
	if (FilePos) { CString csInt2String; csInt2String.Format("%d", FilePos); WritePrivateProfileString("Tip", "FilePos", csInt2String, Dateiname); }
	if (!TimeStamp.IsEmpty()) WritePrivateProfileString("Tip", "TimeStamp", TimeStamp, Dateiname);
	// FolderHistory 
	if (!Folder0000.IsEmpty()) WritePrivateProfileString("FolderHistory", "Folder0000", Folder0000, Dateiname);
	if (!Folder.IsEmpty()) WritePrivateProfileString("FolderHistory", "Folder", Folder, Dateiname);
	// XFolderDialog
	if (ViewMode) { CString csInt2String; csInt2String.Format("%d", ViewMode); WritePrivateProfileString("XFolderDialog", "ViewMode", csInt2String, Dateiname); }
	// Fenster
	if (BuchenPosX) { CString csInt2String; csInt2String.Format("%d", BuchenPosX); WritePrivateProfileString("Fenster", "BuchenPosX", csInt2String, Dateiname); }
	if (BuchenPosY) { CString csInt2String; csInt2String.Format("%d", BuchenPosY); WritePrivateProfileString("Fenster", "BuchenPosY", csInt2String, Dateiname); }
	if (FrameWndPosX) { CString csInt2String; csInt2String.Format("%d", FrameWndPosX); WritePrivateProfileString("Fenster", "FrameWndPosX", csInt2String, Dateiname); }
	if (FrameWndPosY) { CString csInt2String; csInt2String.Format("%d", FrameWndPosY); WritePrivateProfileString("Fenster", "FrameWndPosY", csInt2String, Dateiname); }
	if (FrameWndSizeX) { CString csInt2String; csInt2String.Format("%d", FrameWndSizeX); WritePrivateProfileString("Fenster", "FrameWndSizeX", csInt2String, Dateiname); }
	if (FrameWndSizeY) { CString csInt2String; csInt2String.Format("%d", FrameWndSizeY); WritePrivateProfileString("Fenster", "FrameWndSizeY", csInt2String, Dateiname); }
	// DSA
	if (DSA61447) WritePrivateProfileString("DSA", "DSA61447", DSA61447, Dateiname);
	// Mandanten
	if (nMandanten)
		for (i = 0; i < 100; i++)
		{
			WritePrivateProfileString("Mandanten", csNameKey[i].GetBuffer(0), csName[i], Dateiname);
			WritePrivateProfileString("Mandanten", csPropertyKey[i].GetBuffer(0), csProperty[i], Dateiname);
			WritePrivateProfileString("Mandanten", csIconKey[i].GetBuffer(0), csIcon[i], Dateiname);
		}
	if (!DateinameParam)
	{
		CString csMsg;
		csMsg.Format("Es wurden %d Registrierungseinträge in '%s' geschrieben.", nCount, Dateiname);
		AfxMessageBox(csMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnErnst();
	afx_msg void OnStaticDblClk();
	afx_msg void OnWww();
	afx_msg void OnRegister();
	virtual BOOL OnInitDialog();
	afx_msg void OnKontaktEmail();
	afx_msg void OnKontaktRegistriercode();
	afx_msg void OnVote();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGithub();
};

// App command to run the dialog
void CEasyCashApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_ERNST, OnErnst)
	ON_BN_CLICKED(IDC_STATIC_INFO, OnStaticDblClk)
	ON_BN_CLICKED(IDC_WWW, OnWww)
	ON_BN_CLICKED(IDC_REGISTER, OnRegister)
	ON_BN_CLICKED(IDC_KONTAKT_EMAIL, OnKontaktEmail)
	ON_BN_CLICKED(IDC_KONTAKT_REGISTRIERCODE, OnKontaktRegistriercode)
	ON_BN_CLICKED(IDC_VOTE, OnVote)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_GITHUB, &CAboutDlg::OnBnClickedGithub)
END_MESSAGE_MAP()

void CAboutDlg::OnErnst() 
{
	RegistrierenDlg dlg(this);
	dlg.DoModal();	
}

void CAboutDlg::OnStaticDblClk() 
{
	if(AfxMessageBox("ACHTUNG: Du hast jetzt eine versteckte Funktion gefunden, um den Crash-Reporter bewusst auszulösen. Dies ist nur für Testzwecke. Bitte nur auf Anweisung eines Entwicklers auslösen. Möchtest Du das Programm hiermit wirklich bewusst zum Absturz bringen, um den Crash-Reporter auszulösen?", MB_OKCANCEL) == IDOK)
	{
		WORD *w = (WORD*)0xBAADF00D;
		*w = 0xFEED;
	}
}

void CAboutDlg::OnWww() 
{
	ShellExecute(m_hWnd, "open", "https://www.easyct.de", NULL, ".", SW_SHOWNORMAL);		
}


void CAboutDlg::OnBnClickedGithub()
{
	ShellExecute(m_hWnd, "open", "https://github.com/Thomas-Mielke-Software/EasyCash", NULL, ".", SW_SHOWNORMAL);	
}

void CAboutDlg::OnRegister() 
{
	extern void GetRegName(char *, char *);

	CRegDlg dlgReg;
	if (dlgReg.DoModal() == IDOK)
	{
		char reg[100], regstr[200];
		CString csReg = theApp.GetProfileString("Allgemein", "RegKey", "");
		csReg = csReg.Left(99);
		strcpy (reg, csReg);

		GetRegName(regstr, reg);
		if (*regstr == '#')
		{
			strcpy(reg_name, regstr+1);
		}
		else
		{
			strcpy(reg_name, regstr);
		}

		char reg_string[200];
		strcpy(reg_string, "Registriert für ");
		strcat(reg_string, reg_name);
		SetDlgItemText(IDC_REG_STRING, reg_string);
	}
}

BOOL CAboutDlg::OnInitDialog() 
{
	char s[100];
	char buf[100];

	CDialog::OnInitDialog();
	
	if (strlen(version_string_exakt) > 2 && version_string_exakt[strlen(version_string_exakt)-2] == '.' && version_string_exakt[strlen(version_string_exakt)-1] == '1')
		sprintf(buf, "EasyCash&Tax %s", version_string);
	else
		sprintf(buf, "EasyCash&Tax %s", version_string_exakt);
	SetDlgItemText(IDC_VERSION, buf);
	strcpy(s, "Registriert für ");
	strcat(s, reg_name);
	SetDlgItemText(IDC_REG_STRING, s);
	
	return TRUE;  
} 


void CAboutDlg::OnKontaktEmail() 
{
	int n = (int)ShellExecute(m_hWnd, "open", "mailto:thomas@mielke.software?subject=EasyCash-Kontakt", 
		NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Es gab ein kleines Problem mit dem E-Mail Programm auf diesem Rechner. \
Die Kontakt-Email konnte nicht geöffnet werden. Bitte sende sie manuell.", MB_ICONSTOP);
	}
}

void CAboutDlg::OnKontaktRegistriercode() 
{
	int n = (int)ShellExecute(m_hWnd, "open", "mailto:thomas@mielke.software?subject=ECT-Registrierung&body=Wenn%20nicht%20in%20der%20Absender-Adresse%20enthalten,%0aVorname%20+%20Nachname:%20%", NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Es gab ein kleines Problem mit dem E-Mail Programm auf diesem Rechner. \
Die Registriercode-Anforderung konnte nicht geöffnet werden. Bitte sende sie manuell.", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Die Registrierung sollte jetzt im E-Mail-Programm geöffnet worden sein. (Wenn nicht, bitte das Mail-Programm manuell starten und eine E-Mail mit Betreff 'EasyCash-Registrierung' an MielkeT@gmx.de senden.) Nach dem Absenden sollte der Code in wenigen Tagen eintreffen.", MB_ICONINFORMATION);

		GetDlgItem(IDC_KONTAKT_REGISTRIERCODE)->EnableWindow(FALSE);
	}
}

void CAboutDlg::OnVote() 
{
	if (AfxMessageBox("Wenn der Computer eine Einwahlverbindung ins Internet hat, diese bitte jetzt aktivieren. Wenn die Verbindung steht bitte auf 'OK' klicken. Der Web-Browser wird dann mit dem EC&T-Bugtracker gestartet. Beim ersten Mal muss mit 'Neues Konto anmelden' ein Berichterstatter-Konto erzeugt werden. Die Login-Details werden dann per E-Mail zugesandt.", MB_OKCANCEL) == IDOK)
		ShellExecute(m_hWnd, "open", "https://www.easyct.de/tracker", NULL, ".", SW_SHOWNORMAL);			
}

void CEasyCashApp::ShowTipAtStartup(void)
{
	// CG: This function added by 'Tip of the Day' component.

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (cmdInfo.m_bShowSplash)
	{
		CTipDlg dlg;
		if (dlg.m_bStartup)
			dlg.DoModal();
	}

}

void CEasyCashApp::ShowTipOfTheDay(void)
{
	// CG: This function added by 'Tip of the Day' component.

	CTipDlg dlg;
	dlg.DoModal();

}

// try to display message in caption box, if it exists
void CEasyCashApp::CaptionBox(LPCTSTR str)
{
	CaptionBox(str, 0, "", "");
}

CDocument* CEasyCashApp::GenericGetActiveDocument(CRuntimeClass* pClass)
{
    ASSERT(NULL != pClass); // must be not NULL and derived from CDocument.
    ASSERT(pClass->IsDerivedFrom(RUNTIME_CLASS(CDocument))); 

    CDocument* pDoc = NULL;
    CWnd* pWndMain = AfxGetMainWnd();
    if(NULL != pWndMain)
    {
        if(pWndMain->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)))
        {
            // MDI application, so first we have to get the active MDI child frame.
            CFrameWnd* pFrame = ((CMDIFrameWnd*)pWndMain)->MDIGetActive();
            if(NULL != pFrame)
            {
                CDocument* pActiveDoc = pFrame->GetActiveDocument();
                if((NULL != pActiveDoc) && pActiveDoc->IsKindOf(pClass))
                {
                    // The found document is of required type
                    pDoc = pActiveDoc;
                }
            }
        }
        else if(pWndMain->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
        {
            // SDI appllication so main window is the active frame. 
            pDoc = ((CFrameWnd*)pWndMain)->GetActiveDocument();
        }
        else
        {
            ASSERT(FALSE); // Neither MDI nor SDI application.
        }
    }
    return pDoc;
}
#define GET_ACTIVE_DOC(x) (x*)GenericGetActiveDocument(RUNTIME_CLASS(x))

void CEasyCashApp::CaptionBox(LPCTSTR str, int id, LPCTSTR buttontext, LPCTSTR tooltiptext)
{
	int nDocCount = m_pDocManager->GetOpenDocumentCount();
	CDocument* pDoc = GET_ACTIVE_DOC(CDocument);

	if (m_pMainWnd && ((CMainFrame*)m_pMainWnd)->GetCaptionBar())
	{	
		if (((CMainFrame*)m_pMainWnd)->GetCaptionBar()->m_hWnd == NULL)
			((CMainFrame*)m_pMainWnd)->CreateCaptionBar();
		if (nDocCount > 1 && pDoc)
			((CMainFrame*)m_pMainWnd)->GetCaptionBar()->SetText(pDoc->GetTitle() + ": " + str, CMFCCaptionBar::ALIGN_LEFT);
		else
			((CMainFrame*)m_pMainWnd)->GetCaptionBar()->SetText(str, CMFCCaptionBar::ALIGN_LEFT);
		((CMainFrame*)m_pMainWnd)->GetCaptionBar()->ShowWindow(SW_SHOW);
		if (id && buttontext != "") 
		{
			((CMainFrame*)m_pMainWnd)->GetCaptionBar()->SetButton(buttontext, id, CMFCCaptionBar::ALIGN_LEFT, FALSE);
			((CMainFrame*)m_pMainWnd)->GetCaptionBar()->SetButtonToolTip(tooltiptext);	
		}
		else
			((CMainFrame*)m_pMainWnd)->GetCaptionBar()->RemoveButton();
		((CMainFrame*)m_pMainWnd)->RecalcLayout(FALSE);
	}
	else
		AfxMessageBox(str);
}

void CEasyCashApp::CaptionBoxHide()
{
	if (m_pMainWnd && ((CMainFrame*)m_pMainWnd)->GetCaptionBar() && IsWindow(((CMainFrame*)m_pMainWnd)->GetCaptionBar()->m_hWnd))
	{
		((CMainFrame*)m_pMainWnd)->GetCaptionBar()->ShowWindow(SW_HIDE);
		((CMainFrame*)m_pMainWnd)->RecalcLayout(FALSE);
	}
}

BOOL CEasyCashApp::IsCaptionBoxShown()
{
	if (m_pMainWnd && ((CMainFrame*)m_pMainWnd)->GetCaptionBar() && IsWindow(((CMainFrame*)m_pMainWnd)->GetCaptionBar()->m_hWnd))
		return ((CMainFrame*)m_pMainWnd)->GetCaptionBar()->IsWindowVisible();
	return FALSE;
}

void CEasyCashApp::OnFileOpen() 
{
	char szPathName[500], IniFileName[500];
	GetIniFileName(IniFileName, sizeof(IniFileName));
	if (*IniFileName) GetPrivateProfileString("Allgemein", "LetzteDatei", "", szPathName, sizeof(szPathName), IniFileName);
	//CString csDatenverzeichnis = theApp.GetProfileString("Allgemein", "Datenverzeichnis", "");
	//if (!csDatenverzeichnis.IsEmpty())
		//SetCurrentDirectory(csDatenverzeichnis);
	//CWinAppEx::OnFileOpen();

	if (!*szPathName)
	{
		GetIniFileName(szPathName, 500);
		char *cp1 = strrchr(szPathName, '\\');
		if (cp1) 
		{
			*cp1 = '\0';
			CTime now;
			now = CTime::GetCurrentTime();
			sprintf(cp1, "\\Jahr%-04.4d.eca", (int)now.GetYear());
		}
	}
	
	CString csPathName = szPathName;
	if(!DoPromptFileName(csPathName, AFX_IDS_OPENFILE,
            OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL)) {
        // open cancelled
        return;
    }

    AfxGetApp()->OpenDocumentFile(csPathName);    

	// bei Mandanten check, ob die Datei aus dem selben Datenverzeichnis kommt wie die easyct.ini
	CString Mandant0Existiert = theApp.GetProfileString("Mandanten", "Mandant00Datenverzeichnis", "");
	if (!Mandant0Existiert.IsEmpty())
	{
		int nBackslashPos, nBackslashPos2;
		CString csPathName2 = (CString)IniFileName;
		csPathName.MakeLower();
		csPathName2.MakeLower();
		if ((nBackslashPos = csPathName.ReverseFind('\\')) >= 0 && (nBackslashPos2 = csPathName2.ReverseFind('\\')) >= 0)
		{
			if (nBackslashPos != nBackslashPos2 ||
				csPathName.Left(nBackslashPos) != csPathName2.Left(nBackslashPos2))
				AfxMessageBox("Achtung: Die zu öffnende Datei befindet sich nicht im korrekten Datenverzeichnis für den Mandanten. Das kann zu Problemen führen, wenn z.B. der Kontenrahmen ein anderer ist wie der, mit dem die Buchungsdatei ursprünglich entstanden ist.");
		}
	}
}

BOOL CEasyCashApp::OnOpenRecentFile(UINT nID)
{
	CString Mandant0Existiert = theApp.GetProfileString("Mandanten", "Mandant00Datenverzeichnis", "");
	if (Mandant0Existiert.IsEmpty())
		return CWinAppEx::OnOpenRecentFile(nID);
	else
	{
		OpenDocumentFile(m_MandantenverzeichnisMRUDateinamen[nID-ID_FILE_MRU_FILE2]);		
		return TRUE;
	}
}

BOOL CEasyCashApp::Check4EasyCTX()
{
	HKEY hKey;
	char buffer2[256];

	if (RegOpenKey(HKEY_CLASSES_ROOT, "Wow6432Node\\CLSID\\{ED09076F-B744-47A1-8D60-F9EF093F19F0}\\InprocServer32", &hKey) == ERROR_SUCCESS ||
		RegOpenKey(HKEY_CLASSES_ROOT, "CLSID\\{ED09076F-B744-47A1-8D60-F9EF093F19F0}\\InprocServer32", &hKey) == ERROR_SUCCESS)
	{
		long lRetCode, lDummy;
		long lType = REG_SZ;
		long lCb = sizeof(buffer2);
		lDummy = 0L;
		
		lRetCode = RegQueryValueEx(hKey, TEXT(""), NULL, (ULONG *)&lType, (LPBYTE)buffer2, (ULONG *)&lCb);		

		RegCloseKey(hKey);

		if (lRetCode == ERROR_SUCCESS) // key und wert existieren
		{				
			DWORD dw = GetFileAttributes(buffer2);
			if (dw != 0xFFFFFFFF)
				return TRUE;			
		}
	}
		
	return FALSE;
}

HINSTANCE hEasyCTXP_DLL = NULL;

// Select Folder Dialog -- uses either CXFolderDialog or IFileDialog
// to open a folder picker dialogdepending on OS version
// returns "" if Cancel was pressed or something else went wrong
// Note: This is just multi-byte code and not yet unicode compatibel!
BOOL SelectFolder(LPSTR sFolder, LPCTSTR sTitle = "Verzeichnis auswählen")
{
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	if (GetVersionEx((OSVERSIONINFO *)&osvi) && osvi.dwMajorVersion >= 6) // Vista or higher?
	{
		if (!(hEasyCTXP_DLL = LoadLibrary("EasyCTXP.dll")))
		{
			AfxMessageBox("Fehler beim Öffnen des Verzeichnisauswahldialogs. EasyCTXP.dll ist evtl. nicht installiert.");
			return FALSE;
		}
		else
		{
			BOOL (__cdecl *pSelectFolder)(LPSTR, LPCTSTR);
			pSelectFolder = (BOOL (__cdecl *)(LPSTR, LPCTSTR))GetProcAddress(hEasyCTXP_DLL, "SelectFolder");
			if (pSelectFolder) 
			{
				return (*pSelectFolder)(sFolder, sTitle);
			}
			else
			{
				AfxMessageBox("Fehler beim Öffnen des Verzeichnisauswahldialogs. (SelectFolder() Einsprungpunkt nicht gefunden.) EasyCTXP.dll ist evtl. nicht richtig installiert.");
				return FALSE;
			}
		}	
	}
	else // XP
	{	
		CXFolderDialog dlg(sFolder);		
		dlg.SetTitle(sTitle);
		if (dlg.DoModal() == IDOK)
		{
			CString csPath = dlg.GetPath();
			strcpy(sFolder, (LPCTSTR)csPath);
			return TRUE;
		}
		else
			return FALSE;
	}
}

// HICONFromCBitmap -- Wandelt ein CBitmap in ein HICON
HICON HICONFromCBitmap(CBitmap& bitmap)
{
   BITMAP bmp;
   bitmap.GetBitmap(&bmp);
   
   HBITMAP hbmMask = ::CreateCompatibleBitmap(::GetDC(NULL), 
                                              bmp.bmWidth, bmp.bmHeight);

   ICONINFO ii = {0};
   ii.fIcon    = TRUE;
   ii.hbmColor = bitmap;
   ii.hbmMask  = hbmMask;

   HICON hIcon = ::CreateIconIndirect(&ii);
   ::DeleteObject(hbmMask);

   return hIcon;
}
