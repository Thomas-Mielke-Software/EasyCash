// NewDownloadDialog.cpp : implementation file
//

#include "stdafx.h"
#include "DownloadDialog.h"
#include "AFXINET.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CThreadParam, CObject)

/////////////////////////////////////////////////////////////////////////////
// CDownloadDialog dialog


CDownloadDialog::CDownloadDialog(INFOSTRUCT *info, CWnd* pParent /*=NULL*/)
	: CDialog(CDownloadDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDownloadDialog)
	m_check = FALSE;
	//}}AFX_DATA_INIT

	m_thread = NULL;

	m_info.number = info->number;
	m_info.title = info->title;
	m_info.director = info->director;
	m_info.length = info->length;
	m_info.diskno = info->diskno;
	m_info.index = info->index;

    m_nLower=0;
    m_nUpper=-1;

	eta_n = 0;  // Anzahl Elemente im Array auf Null

}


void CDownloadDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDownloadDialog)
	DDX_Check(pDX, IDC_CHECK, m_check);
	//}}AFX_DATA_MAP

	m_pThreadParam = NULL;
}


BEGIN_MESSAGE_MAP(CDownloadDialog, CDialog)
	//{{AFX_MSG_MAP(CDownloadDialog)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDownloadDialog message handlers

BOOL CDownloadDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	bNotYetDownload = TRUE;

	//CheckForUpdate
	SetDlgItemText(IDC_INFO, "Checking for software update...");

	((CProgressCtrl *)GetDlgItem(CG_IDC_PROGDLG_PROGRESS))->SetRange(0, 100);

	return TRUE;  
}

void CDownloadDialog::BuildComboBoxList()
{
	// ComboBox zurücksetzen:
	CComboBox *cb = (CComboBox *)GetDlgItem(IDC_DOWNLOAD_DIRECTORY);
	cb->ResetContent();

	// zuerst das Windows-Temp-Verzeichnis eintragen
	char temp_path[2000];
	if (GetTempPath(sizeof(temp_path), temp_path))
		cb->AddString(temp_path);

	// Danach die Such-Verzeichnisse aus der Registry:
	CString dir;
	CRegistry reg;
	int n;
	BOOL bWinTempEingetragen = FALSE;

	if (reg.GetFirstVideoPath(dir))
	{
		do
		{
			if (dir != (CString)temp_path) // WINTEMP nicht doppelt aufführen!
				cb->AddString((LPCSTR)dir);
			else
				bWinTempEingetragen = TRUE;
		}
		while (reg.GetNextVideoPath(dir));
	}

	if (!reg.GetDefaultDownloadPath(dir))
		cb->SetCurSel(0);
	else if ((n = cb->FindString(0, dir)) == CB_ERR)
		cb->SetCurSel(0);
	else
		cb->SetCurSel(n);

	if (!bWinTempEingetragen)
	{
		reg.AppendVideoPath((CString)temp_path);
	}
}

// --- Thread Funktion ---

static int CheckForUpdate(CThreadParam *pThreadParam); // 1. Hilfsfunktion
static int DownloadUpdate(CThreadParam *pThreadParam, char *filename); // 2. Hilfsfunktion

UINT DownloadControllingFunction( LPVOID pParam )
{
	char local_Path_plus_Filename[2000];
    CThreadParam *pThreadParam = (CThreadParam*)pParam;

    if (pThreadParam == NULL ||
        !pThreadParam->IsKindOf(RUNTIME_CLASS(CThreadParam)))
		    return 1;	// illegal parameter

	pThreadParam->trans_bytes = 0;

	strcpy(local_Path_plus_Filename, pThreadParam->local_Path);
	if (local_Path_plus_Filename[strlen(local_Path_plus_Filename)-1] != '\\')
		strcat(local_Path_plus_Filename, "\\");
	strcat(local_Path_plus_Filename, pThreadParam->Filename);

	if (pThreadParam->cancel)
	{
		pThreadParam->status_text = "User canceled!";
		pThreadParam->canceled = TRUE;
		return 0;
	}

	if (pThreadParam->bOnlyUpdateCheck)
	{	
		CheckForUpdate(pThreadParam);
		pThreadParam->cancel = TRUE;
		pThreadParam->canceled = TRUE;
		return 0;	// thread completed successfully
	}	

	pThreadParam->status_text = "Starting internet session";
	CInternetSession S("Roland Collection FTP",1);//INTERNET_OPEN_TYPE_PRECONFIG | INTERNET_OPEN_TYPE_DIRECT
	CFtpConnection *f; 

	if (pThreadParam->cancel)
	{
		pThreadParam->status_text = "User canceled!";
		pThreadParam->canceled = TRUE;
		return 0;
	}

	try
	{ 
		pThreadParam->status_text = "Connecting to server";
		f = S.GetFtpConnection(pThreadParam->Server,"anonymous","anonymous@nowhere.com",21,FALSE); 

		if (pThreadParam->cancel)
		{
			pThreadParam->status_text = "User canceled!";
			pThreadParam->canceled = TRUE;
			return 0;
		}

		pThreadParam->status_text = "Opening source direcory";
		f->SetCurrentDirectory(pThreadParam->FTP_Path); 
		//f->GetFile(pThreadParam->Filename,local_Path_plus_Filename,FALSE,FILE_ATTRIBUTE_NORMAL,FTP_TRANSFER_TYPE_BINARY,1);

		if (pThreadParam->cancel)
		{
			pThreadParam->status_text = "User canceled!";
			pThreadParam->canceled = TRUE;
			return 0;
		}

		// Dateinamen nach lowercase konvertieren
		char filename_buffer[1000], *cp1, *cp2;
		for (cp1 = pThreadParam->Filename, cp2 = filename_buffer; *cp1; cp1++, cp2++)
			*cp2 = tolower(*cp1);

		pThreadParam->status_text = "Looking for clip file";
		CFtpFileFind find(f);
		if (find.FindFile(filename_buffer))
		{
			find.FindNextFile(); // sucht nach nächstem file - ergebnis ist egal - und stellt die daten des ersten files zur verfügung

			pThreadParam->status_text = "Retrieving file size";
			pThreadParam->total_bytes = find.GetLength();

			CInternetFile *pFtpFile;

			pThreadParam->status_text = "Opening file for download";
			try
			{
				pFtpFile = f->OpenFile(filename_buffer, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 1);
			}
			catch (CInternetException* pEx)
			{
				char buffer[5000];
				pEx->GetErrorMessage(buffer, sizeof(buffer));
				AfxMessageBox(buffer, MB_ICONSTOP);
				pThreadParam->status_text = "FAILED: Couldn't open file on server!"; 
				pThreadParam->canceled = TRUE;
				pEx->Delete();
				return 0;
			}

			pThreadParam->status_text = "Downloading clip file";
			CFile file;
			if (file.Open(local_Path_plus_Filename, CFile::modeCreate|CFile::modeWrite))
			{
				while (TRUE)
				{
					char buf[1024];
					int n = pFtpFile->Read(buf, sizeof(buf));
					if (n) file.Write(buf, n);
					pThreadParam->trans_bytes += n;
					if (pThreadParam->cancel)break;
					if (n < sizeof(buf)) break;
					if (pThreadParam->trans_bytes == pThreadParam->total_bytes) break;
				}
				file.Close();
				pFtpFile->Close();
				delete pFtpFile;

				delete f; 
				S.Close(); 
				
				if (pThreadParam->trans_bytes == pThreadParam->total_bytes)
				{
					static char statbuf[200];
					strcpy(statbuf, "Clip file was downloaded successfully! ");
					pThreadParam->status_text = statbuf;

					// nach Update gucken?
					if (pThreadParam->check || pThreadParam->bOnlyUpdateCheck)
					{
						char bufstat[1000];
						memset(bufstat, 0x00, sizeof(bufstat));
						pThreadParam->status_text = bufstat;
						strcat(pThreadParam->status_text, "Now checking for update ...");

						CheckForUpdate(pThreadParam);
					}
				}
				else
				{
					pThreadParam->status_text = "User canceled!";
					DeleteFile(local_Path_plus_Filename);
				}
			}
			else
			{
				pThreadParam->status_text = "FAILED: File open error on local system!"; 

				pFtpFile->Close();
				delete pFtpFile;

				delete f; 
				S.Close(); 
			}	
		}
		else
		{
			pThreadParam->status_text = "FAILED: Couldn't retrieve information about download file from server!"; 
			DWORD dw = GetLastError();
			LPVOID lpMsgBuf;
 
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);

			if (lpMsgBuf)
			{
				// Display the string.
				MessageBox( NULL, (char *)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

				// Free the buffer.
				LocalFree( lpMsgBuf );
			}

			delete f; 
			S.Close(); 
		}
	} 
	catch (CInternetException* pEx) 
	{ 
		char buffer[5000];
		pEx->GetErrorMessage(buffer, sizeof(buffer));
		AfxMessageBox(buffer, MB_ICONSTOP);
		pThreadParam->status_text = "FAILED: Couldn't get a connection to the internet!"; 
		pThreadParam->canceled = TRUE;
		pEx->Delete();
		return 0;
	} 

	pThreadParam->cancel = TRUE;
	pThreadParam->canceled = TRUE;
    return 0;	// thread completed successfully
}

// --- 1. Hilfsfunktion für DownloadControllingFunction ---

static int CheckForUpdate(CThreadParam *pThreadParam)
{
	extern CRCPlayerApp theApp;

	static char statbuf[200];
	strcpy(statbuf, pThreadParam->status_text);
	pThreadParam->status_text = statbuf;
	if (pThreadParam->bOnlyUpdateCheck)
		strcat(pThreadParam->status_text, "Looking for update information...");

	char version_information[10000];

	CInternetSession S("Roland Collection FTP",1);//INTERNET_OPEN_TYPE_PRECONFIG | INTERNET_OPEN_TYPE_DIRECT
	CFtpConnection *f; 

	if (pThreadParam->cancel)
	{
		pThreadParam->status_text = "User canceled!";
		S.Close(); 
		return 0;
	}

	try
	{
		f = S.GetFtpConnection(pThreadParam->Server,"anonymous","anonymous@nowhere.com",21,FALSE); 

		if (pThreadParam->cancel)
		{
			pThreadParam->status_text = "User canceled!";
			delete f; 
			S.Close(); 
			return 0;
		}

		// Verzeichnis wechseln
		f->SetCurrentDirectory(FTP_UPDATE_PATH); 
		strcat(pThreadParam->status_text, ".");

		if (pThreadParam->cancel)
		{
			pThreadParam->status_text = "User canceled!";
			delete f; 
			S.Close(); 
			return 0;
		}

		// Datei suchen
		strcat(pThreadParam->status_text, ".");
		CFtpFileFind find(f);
		if (find.FindFile(FTP_UPDATE_VERSION_FILENAME, INTERNET_FLAG_RELOAD))
		{
			find.FindNextFile(); // sucht nach nächstem file - ergebnis ist egal - und stellt die daten des ersten files zur verfügung

			strcat(pThreadParam->status_text, ".");
			int version_information_bytes = find.GetLength();
			if (version_information_bytes > sizeof(version_information)-1)
				version_information_bytes = sizeof(version_information)-1;

			CInternetFile *pFtpFile;

			strcat(pThreadParam->status_text, ".");
			try
			{
				char filename_buffer[2000];
				strcpy(filename_buffer, FTP_UPDATE_PATH);
				strcat(filename_buffer, FTP_UPDATE_VERSION_FILENAME);
				pFtpFile = f->OpenFile(filename_buffer, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 1);
			}
			catch (CInternetException* pEx)
			{
				char buffer[5000];
				pEx->GetErrorMessage(buffer, sizeof(buffer));
				AfxMessageBox(buffer, MB_ICONSTOP);
				pThreadParam->status_text = "FAILED: Couldn't find update information on server!"; 
				pEx->Delete();
				delete f; 
				S.Close(); 
				return 0;
			}

			strcat(pThreadParam->status_text, ".");

			int n = pFtpFile->Read(version_information, version_information_bytes);
			if (pThreadParam->cancel)
			{
				pThreadParam->status_text = "User canceled!";
				pFtpFile->Close();
				delete pFtpFile;
				delete f; 
				S.Close(); 
				return 0;
			}
			version_information[version_information_bytes] = '\0';

			if (!version_information_bytes || toupper(*version_information) != 'V')
			{
				pThreadParam->status_text = "FAILED: Couldn't find valid update information on server!"; 
				pFtpFile->Close();
				delete pFtpFile;
				delete f; 
				S.Close(); 
				return 0;
			}

			char *cp = strchr(version_information+1, ':');
			if (!cp)
			{
				pThreadParam->status_text = "FAILED: Couldn't find valid update informations on the server!"; 
				pFtpFile->Close();
				delete pFtpFile;
				delete f; 
				S.Close(); 
				return 0;
			}


			*cp++ = '\0';
			if (!strcmp(version_information, theApp.version_string))
			{
				if (!pThreadParam->bOnlyUpdateCheck)
					pThreadParam->status_text = "Clip file was downloaded successfully! No application update available.";
				else
					pThreadParam->status_text = "No application update available.";
				pFtpFile->Close();
				delete pFtpFile;
				delete f; 
				S.Close(); 
				if (pThreadParam->bOnlyUpdateCheck) 
					AfxMessageBox("No application update available.");

				return 0;			// --> hier 'raus wenn kein neues update verfügbar
			}
			
			pFtpFile->Close();
			delete pFtpFile;
			delete f; 
			S.Close(); 

			if (!pThreadParam->bOnlyUpdateCheck)
				pThreadParam->status_text = "Clip file was downloaded successfully! Application update in progress...";
			else
				pThreadParam->status_text = "Application update in progress...";

			DownloadUpdate(pThreadParam, cp);

			return 1;				// --> hier 'raus wenn neues update verfügbar war

		}
		else
		{
			pThreadParam->status_text = "FAILED: Couldn't retrieve information about application update from server!"; 
			DWORD dw = GetLastError();
			LPVOID lpMsgBuf;

			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);

			if (lpMsgBuf)
			{
				// Display the string.
				MessageBox( NULL, (char *)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

				// Free the buffer.
				LocalFree( lpMsgBuf );
			} 
			delete f; 
			S.Close(); 

			return 0;
		}
	} 
	catch (CInternetException* pEx) 
	{ 
		char buffer[5000];
		pEx->GetErrorMessage(buffer, sizeof(buffer));
		AfxMessageBox(buffer, MB_ICONSTOP);
		pThreadParam->status_text = "FAILED: Couldn't get a connection to the internet!"; 
		pEx->Delete();
		S.Close(); 
		return 0;
	} 
	return 1;
}

// --- 2. Hilfsfunktion für DownloadControllingFunction ---

static int DownloadUpdate(CThreadParam *pThreadParam, char *filename)
{
	char local_Path_plus_Filename[2000];

	strcpy(local_Path_plus_Filename, pThreadParam->local_Path);
	if (local_Path_plus_Filename[strlen(local_Path_plus_Filename)-1] != '\\')
		strcat(local_Path_plus_Filename, "\\");
	strcat(local_Path_plus_Filename, filename);

	CInternetSession S("Roland Collection FTP",1);//INTERNET_OPEN_TYPE_PRECONFIG | INTERNET_OPEN_TYPE_DIRECT
	CFtpConnection *f; 

	if (pThreadParam->cancel)
	{
		pThreadParam->status_text = "User canceled!";
		S.Close(); 
		return 0;
	}

	try
	{
		f = S.GetFtpConnection(pThreadParam->Server,"anonymous","anonymous@nowhere.com",21,FALSE); 

		if (pThreadParam->cancel)
		{
			pThreadParam->status_text = "User canceled!";
			delete f; 
			S.Close(); 
			return 0;
		}

		// Verzeichnis wechseln
		f->SetCurrentDirectory(FTP_UPDATE_PATH); 
		strcat(pThreadParam->status_text, ".");

		if (pThreadParam->cancel)
		{
			pThreadParam->status_text = "User canceled!";
			delete f; 
			S.Close(); 
			return 0;
		}

		// Datei suchen
		strcat(pThreadParam->status_text, ".");
		CFtpFileFind find(f);
		if (find.FindFile(filename, INTERNET_FLAG_RELOAD))
		{
			find.FindNextFile(); // sucht nach nächstem file - ergebnis ist egal - und stellt die daten des ersten files zur verfügung

			strcat(pThreadParam->status_text, ".");
			pThreadParam->trans_bytes = 0;
			pThreadParam->total_bytes = find.GetLength();

			CInternetFile *pFtpFile;

			strcat(pThreadParam->status_text, ".");
			try
			{
				char filename_buffer[2000];
				strcpy(filename_buffer, FTP_UPDATE_PATH);
				strcat(filename_buffer, filename);
				pFtpFile = f->OpenFile(filename_buffer, GENERIC_READ, FTP_TRANSFER_TYPE_BINARY, 1);
			}
			catch (CInternetException* pEx)
			{
				char buffer[5000];
				pEx->GetErrorMessage(buffer, sizeof(buffer));
				AfxMessageBox(buffer, MB_ICONSTOP);
				pThreadParam->status_text = "FAILED: Couldn't find update file on server!"; 
				pEx->Delete();
				delete f; 
				S.Close(); 
				return 0;
			}

			strcat(pThreadParam->status_text, ".");
			CFile file;
			if (file.Open(local_Path_plus_Filename, CFile::modeCreate|CFile::modeWrite))
			{
				while (TRUE)
				{
					char buf[1024];
					int n = pFtpFile->Read(buf, sizeof(buf));
					if (n) file.Write(buf, n);
					pThreadParam->trans_bytes += n;
					if (pThreadParam->cancel)break;
					if (n < sizeof(buf)) break;
					if (pThreadParam->trans_bytes == pThreadParam->total_bytes) break;
				}
				file.Close();
				pFtpFile->Close();
				delete pFtpFile;

				delete f; 
				S.Close(); 
				
				if (pThreadParam->trans_bytes == pThreadParam->total_bytes)
				{
					pThreadParam->status_text = "Clip file and application update were downloaded successfully!";
					
					// hier Setup des Updates ausführen
					ShellExecute(NULL, "open", local_Path_plus_Filename, NULL, NULL, SW_SHOW);	
				}
				else
				{
					pThreadParam->status_text = "User canceled!";
					DeleteFile(local_Path_plus_Filename);
				}
			}
			else
			{
				pThreadParam->status_text = "FAILED: File open error on local system!"; 
				pFtpFile->Close();
				delete pFtpFile;
				delete f; 
				S.Close(); 
			}	

			return 1;			
		}
		else
		{
			pThreadParam->status_text = "FAILED: Couldn't retrieve application update file from server!"; 
			DWORD dw = GetLastError();
			LPVOID lpMsgBuf;

			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);

			if (lpMsgBuf)
			{
				// Display the string.
				MessageBox( NULL, (char *)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

				// Free the buffer.
				LocalFree( lpMsgBuf );
			} 
			delete f; 
			S.Close(); 

			return 0;
		}
	} 
	catch (CInternetException* pEx) 
	{ 
		char buffer[5000];
		pEx->GetErrorMessage(buffer, sizeof(buffer));
		AfxMessageBox(buffer, MB_ICONSTOP);
		pThreadParam->status_text = "FAILED: Couldn't get a connection to the internet!"; 
		pEx->Delete();
		S.Close(); 
		return 0;
	} 
	return 1;
}

// --- OK Knopf ---

void CDownloadDialog::OnOK() 
{
	UpdateData(TRUE);

	bClipDownloadSuccess = FALSE;

	// Download path aus ComboBox lesen:
	CString download_dir;
	CComboBox *cb = (CComboBox *)GetDlgItem(IDC_DOWNLOAD_DIRECTORY);
	int n = cb->GetCurSel();
	cb->GetLBText(n, download_dir);
	if (download_dir.GetLength() == 0)
	{
		AfxMessageBox("Please select a download directory!");
		cb->SetFocus();
		return;
	}

	CRegistry reg;
	reg.SetDefaultDownloadPath(download_dir);
	if (download_dir.GetBuffer(0)[download_dir.GetLength()-1] == '\\')
		download_dir.GetBuffer(0)[download_dir.GetLength()-1] = '\0';
	m_pThreadParam = new CThreadParam;
	char ftp_path[1000];
	char filename[1000];
	sprintf(ftp_path, "%spcd%02d/", FTP_DOWNLOAD_PATH, m_info.diskno);
	sprintf(filename, "%s.rm", (LPCSTR)m_info.number);
	char *cp = filename;
	while (*cp)
	{
		*cp = toupper(*cp);
		cp++;
	}	
	
	strcpy(m_pThreadParam->Server, FTP_DOWNLOAD_SERVER);
	strcpy(m_pThreadParam->FTP_Path, ftp_path);
	strcpy(m_pThreadParam->local_Path, (LPCSTR)download_dir);
	strcpy(m_pThreadParam->Filename, filename);
	m_pThreadParam->status_text = "";
	m_pThreadParam->cancel = FALSE;
	m_pThreadParam->canceled = FALSE;
	m_pThreadParam->check = m_check;
	if (m_info.number.IsEmpty())
		m_pThreadParam->bOnlyUpdateCheck = TRUE;
	else
		m_pThreadParam->bOnlyUpdateCheck = FALSE;

	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK)->EnableWindow(FALSE);
	SetTimer(1, ETA_INTERVALL, NULL);

	m_thread = AfxBeginThread(&(DownloadControllingFunction), (LPVOID)m_pThreadParam, THREAD_PRIORITY_LOWEST);
	bNotYetDownload = FALSE;
}

// --- Cancel Knopf ---

// defaultmäßig diese Funktion zum Verlassen des Dialogs benutzen!
void CDownloadDialog::OnCancel() 
{
	if (bNotYetDownload)
	{

		if (bClipDownloadSuccess)
			CDialog::OnOK();
		else
			CDialog::OnCancel();
		
		return;
	}

	extern CRCPlayerApp theApp;
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

	if (m_pThreadParam) 
	{
		m_pThreadParam->cancel = TRUE;
		m_pThreadParam->status_text = "Please wait while canceling the internet session...";
	}

}

// --- WM_TIMER ---

void CDownloadDialog::OnTimer(UINT nIDEvent) 
{
	if (m_pThreadParam)
	{
		char buffer[1000];

		if (m_pThreadParam->trans_bytes <= m_pThreadParam->total_bytes)
		{
			((CProgressCtrl *)GetDlgItem(CG_IDC_PROGDLG_PROGRESS))->SetRange(0, m_pThreadParam->total_bytes/1024);
			SetPos();
		}

		if (!m_pThreadParam->trans_bytes)
			SetDlgItemText(IDC_STATUS, m_pThreadParam->status_text);
		else
		{
			if (eta_n >= ETA_ARRAY_SIZE) // array schon voll?
			{
				// nach unten schieben
				for (int i = 0; i < ETA_ARRAY_SIZE-1; i++)
				{
					eta_array[i] = eta_array[i+1];
				}
				eta_array[ETA_ARRAY_SIZE-1] = m_pThreadParam->trans_bytes;
			}
			else
				eta_array[eta_n++] = m_pThreadParam->trans_bytes; // einfach hinzufügen

			int data_rate = 0; // Bytes pro 1/10-Sekunde
			int remaining_time = 0; // Rest-Zeit in Sekunden
			if (eta_n >= 2) //min. 2 Meßwerte um Differenz zu bestimmen
			{
				data_rate = (eta_array[eta_n-1] - eta_array[0]) * 10 / (ETA_INTERVALL * (eta_n-1)); 
				if (data_rate == 0) 
					remaining_time = 23 * 3600 + 59 * 60 + 59;
				else
					remaining_time = (m_pThreadParam->total_bytes - m_pThreadParam->trans_bytes) / 100 / data_rate;
			
				if (m_pThreadParam->total_bytes == m_pThreadParam->trans_bytes)
					data_rate = 0;			
			}
			sprintf(buffer, "%s\r\nBytes transferred: %d\tkb/sec.: %3.1f \teta: %02d:%02d:%02d", 
				m_pThreadParam->status_text, 
				(int)m_pThreadParam->trans_bytes,
				(float)data_rate / 10.0,
				remaining_time / 3600/*h*/, (remaining_time / 60) % 60/*m*/, remaining_time % 60
			);
			SetDlgItemText(IDC_STATUS, buffer);
		}

		if (m_pThreadParam->trans_bytes == m_pThreadParam->total_bytes)
		{
			SetDlgItemText(IDCANCEL, "OK");
			bClipDownloadSuccess = TRUE;
		}

		// Auf Thread warten bei cancel:
		if (m_pThreadParam->cancel)
		{
			if (m_pThreadParam->canceled)
			{
				delete m_pThreadParam;
				m_pThreadParam = NULL;

				if (bClipDownloadSuccess)
					CDialog::OnOK();
				else
					CDialog::OnCancel();
			}
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CDownloadDialog::UpdatePercent(int nNewPos)
{
    CWnd *pWndPercent = GetDlgItem(CG_IDC_PROGDLG_PERCENT);
    int nPercent;
    
    int nDivisor = m_pThreadParam->total_bytes;

    int nDividend = nNewPos;

	if (!nDivisor) return;
	if (nDividend >= 0x01000000)
		nPercent = nDividend / (nDivisor / 100); // nur wenn die Gefahr eines Überlaufs besteht
	else
		nPercent = nDividend * 100 / nDivisor;

    // Since the Progress Control wraps, we will wrap the percentage
    // along with it. However, don't reset 100% back to 0%
    if(nPercent<100)
      nPercent %= 100;
    if(nPercent>100)
      nPercent = 100;

    // Display the percentage
    CString strBuf;
    strBuf.Format(_T("%d%c"),nPercent,_T('%'));

	CString strCur; // get current percentage
    pWndPercent->GetWindowText(strCur);

	if (strCur != strBuf)
		pWndPercent->SetWindowText(strBuf);
}

int CDownloadDialog::SetPos()
{
	if (!m_pThreadParam) return 0;

    PumpMessages();

	if (!m_pThreadParam) return 0;
    int iResult = ((CProgressCtrl *)GetDlgItem(CG_IDC_PROGDLG_PROGRESS))->SetPos(m_pThreadParam->trans_bytes/1024);
    UpdatePercent(m_pThreadParam->trans_bytes);
    return iResult;
}

void CDownloadDialog::PumpMessages()
{
    // Must call Create() before using the dialog
    ASSERT(m_hWnd!=NULL);

    MSG msg;
    // Handle dialog messages
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if(!IsDialogMessage(&msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);  
      }
    }
}

/*
BOOL CDownloadDialog::CheckForUpdate()
{	
	// Download path aus ComboBox lesen:
	CString download_dir;
	CComboBox *cb = (CComboBox *)GetDlgItem(IDC_DOWNLOAD_DIRECTORY);
	int n = cb->GetCurSel();
	cb->GetLBText(n, download_dir);
	if (download_dir.GetLength() == 0)
	{
		AfxMessageBox("Please select a download directory!");
		cb->SetFocus();
		return FALSE;
	}

	m_pThreadParam = new CThreadParam;
	char ftp_path[1000];
	char filename[1000];
	sprintf(ftp_path, "%spcd%02d/", FTP_DOWNLOAD_PATH, m_info.diskno);
	sprintf(filename, "%s.rm", (LPCSTR)m_info.number);
	char *cp = filename;
	while (*cp)
	{
		*cp = toupper(*cp);
		cp++;
	}	
	
	strcpy(m_pThreadParam->Server, FTP_DOWNLOAD_SERVER);
	strcpy(m_pThreadParam->FTP_Path, FTP_UPDATE_PATH);
	strcpy(m_pThreadParam->local_Path, (LPCSTR)download_dir);
	strcpy(m_pThreadParam->Filename, FTP_UPDATE_VERSION_FILENAME);
	m_pThreadParam->status_text = "Checking for Update";
	m_pThreadParam->cancel = FALSE;
	m_pThreadParam->canceled = FALSE;

	GetDlgItem(IDOK)->EnableWindow(FALSE);
	SetTimer(1, ETA_INTERVALL, NULL);

	m_thread = AfxBeginThread(&(DownloadControllingFunction), (LPVOID)m_pThreadParam, THREAD_PRIORITY_LOWEST);
	bNotYetDownload = FALSE;

	return TRUE;
}
*/
