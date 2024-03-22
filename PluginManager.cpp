// PluginManager.cpp : implementation file
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
#include "easycash.h"
#include "MainFrm.h"
#include "PluginManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern "C" AFX_EXT_CLASS BOOL GetProgrammverzeichnis(char *buffer3, int size);

/////////////////////////////////////////////////////////////////////////////
// CPluginManager dialog


CPluginManager::CPluginManager(CMainFrame* pParent)
	: CDialog(CPluginManager::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPluginManager)
	m_status = _T("");
	//}}AFX_DATA_INIT

	m_pParent = pParent;
	m_pbmp = NULL; 
}


void CPluginManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginManager)
	DDX_Control(pDX, IDC_LISTE, m_liste);
	DDX_Text(pDX, IDC_STATUS, m_status);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_MANUELLER_DOWNLOAD, m_btnManuellerDownload);
	DDX_Control(pDX, IDC_REPARATURINSTALLATION, m_ButtonReparaturinstallation);
	DDX_Control(pDX, IDOK, m_ButtonDownload);
	DDX_Control(pDX, IDCANCEL, m_ButtonSchliessen);
}


BEGIN_MESSAGE_MAP(CPluginManager, CDialog)
	//{{AFX_MSG_MAP(CPluginManager)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CLICK, IDC_LISTE, OnClickListe)
	//}}AFX_MSG_MAP
//	ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK_FULLROW, IDC_CHECK_NOSEL3, OnSomeCheckbox)

	ON_MESSAGE(WM_QUICKLIST_GETLISTITEMDATA, OnGetListItem) 
//	ON_MESSAGE(WM_QUICKLIST_NAVIGATIONTEST, OnNavigationTest) 
	ON_MESSAGE(WM_QUICKLIST_CLICK, OnListClick) 
//	ON_MESSAGE(WM_QUICKLIST_HEADERRIGHTCLICK, OnHeaderRightClick)

	#ifdef USEXPTHEMES
		//WM_THEMECHANGED = 0x031A (WM_THEMECHANGED may not be avalible)
		ON_MESSAGE(0x031A, OnThemeChanged)	
	#endif
		ON_BN_CLICKED(IDC_REPARATURINSTALLATION, &CPluginManager::OnBnClickedReparaturinstallation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPluginManager message handlers

BOOL CPluginManager::PreTranslateMessage(MSG* pMsg)
{
     m_ToolTip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CPluginManager::OnInitDialog() 
{
	CDialog::OnInitDialog();

	EnableToolTips();

	//Create the ToolTip control
	if( !m_ToolTip.Create(this))
	{
		 TRACE0("Unable to create the ToolTip!");
	}
	else
	{
		m_ToolTip.AddTool( &m_ButtonReparaturinstallation, _T("forciert das nochmalige Herunterladen und Installieren der angewählen Module, falls es zuvor bei der Installation Probleme gab oder man andere Formulare installieren möchte"));
		m_ToolTip.AddTool( &m_ButtonDownload, _T("nur neu angewählte Plugins herunterladen -- und solche, für die es Updates gibt"));
		m_ToolTip.AddTool( &m_ButtonSchliessen, _T("Dialogfenster schließen, ohne etwas zu tun"));

		m_ToolTip.Activate(TRUE);
	}  

	ListView_SetExtendedListViewStyleEx(m_liste.m_hWnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyleEx(m_liste.m_hWnd, LVS_EX_SUBITEMIMAGES , LVS_EX_SUBITEMIMAGES );
	//ListView_SetExtendedListViewStyleEx(m_liste.m_hWnd, LVS_EX_TRACKSELECT, LVS_EX_TRACKSELECT );
	ListView_SetExtendedListViewStyleEx(m_liste.m_hWnd, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES );
	
//	m_liste.Create(IDB_IMAGE_LIST, 16, ILC_COLOR4, RGB(255, 255, 255));
	//m_imgLargeList.Create(IDB_IMAGE_LIST_LARGE, 32, ILC_COLOR4, RGB(255, 255, 255));

	//m_ListeLB.SetBackgroundColor(RGB(255,255,255));

//	m_liste.SetImageList(&m_imgList, LVSIL_SMALL);

	CBitmap generische_bitmap;
	generische_bitmap.LoadBitmap(IDB_EASYCASH_32);
	m_imgList.Create(32, 32, ILC_COLOR24, 0, 100);
	m_imgList.SetBkColor(RGB(255,255,255));
	m_imgList.Add(&generische_bitmap, RGB(255,0,255));   //  <-- funktionierte nach Umstellung auf VS9 nicht mehr, weil IDB_EASYCASH nur 26x26 groß war
	//m_imgList.Add(HICONFromCBitmap(generische_bitmap));	// alter workaround
	ASSERT(m_imgList.GetImageCount() != 0);

	m_liste.InsertColumn(0, "Plugin installieren/aktualisieren", LVCFMT_LEFT, 400);
	m_liste.InsertColumn(1, "Größe", LVCFMT_LEFT, 70);
	m_liste.InsertColumn(2, "Aktion", LVCFMT_CENTER, 120);
	
	m_btnManuellerDownload.SetURL(_T("https://www.easyct.de/downloads.php?cat_id=5"));
	m_btnManuellerDownload.SetTooltip(_T("Manchmal kann es vorkommen, dass der Plugin-Manager Plugins mit 'kein Update nötig' anzeigt, obwohl sie überhaupt nicht installiert sind, weil z.B. die Installation abgebrochen wurde. In diesen Fällen kann das Plugin manuell von der EC&&T-Homepage heruntergeladen und installiert werden."));
	m_btnManuellerDownload.SizeToContent();

#ifdef USEXPTHEMES
	m_themeManager.Init(m_hWnd);
	m_liste.SetThemeManager(&m_themeManager);
#endif

	//CString csEmpMessBuf = "";
	//m_ListeLB.SetEmptyMessage(csEmpMessBuf);

	m_liste.EnableToolTips(TRUE);

	m_liste.SetImageList(&m_imgList, LVSIL_SMALL);

	GetDlgItem(IDOK)->EnableWindow(FALSE);

	SetTimer(1, 100, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//User clicked on list. Toggle checkbox/image if hit
LRESULT CPluginManager::OnListClick(WPARAM wParam, LPARAM lParam)
{
	//Make sure message comes from list box
	ASSERT( (HWND)wParam == m_liste.GetSafeHwnd() );

	CQuickList::CListHitInfo *hit= (CQuickList::CListHitInfo*) lParam;

	TRACE(_T("User hit item %d, subitem %d "), hit->m_item, hit->m_subitem);

	if(hit->m_onButton)
	{
		if (m_csaCheckedAktuell[hit->m_item] == "0")
			m_csaCheckedAktuell[hit->m_item] = "1";
		else
			m_csaCheckedAktuell[hit->m_item] = "0";

		m_liste.RedrawCheckBoxs(hit->m_item, hit->m_item, hit->m_subitem);
	}

	TRACE(_T("\n"));

	return 0;
}

//Get item data
LRESULT CPluginManager::OnGetListItem(WPARAM wParam, LPARAM lParam)
{
	//wParam is a handler to the list
	//Make sure message comes from list box
	ASSERT( (HWND)wParam == m_liste.GetSafeHwnd() );

	//lParam is a pointer to the data that is needed for the element
	CQuickList::CListItemData* data = (CQuickList::CListItemData*) lParam;

	//Get which item and subitem that is asked for.
	int item = data->GetItem();
	int subItem = data->GetSubItem();

	data->m_allowEdit = FALSE;
	data->m_textStyle.m_bold = TRUE;
	data->m_textStyle.m_italic = FALSE;
	if (subItem == 1) data->m_textStyle.m_textPosition |= DT_CENTER;
	if (subItem == 2) data->m_textStyle.m_textPosition |= DT_CENTER;

	if(data->GetItem()%2 == 0)
		data->m_colors.m_backColor = RGB( 255, 255, 255);
	else	
		data->m_colors.m_backColor = RGB( 232, 232, 232);

	if (subItem == 0)
	{
		data->m_button.m_draw = true;
		data->m_button.m_noSelection = FALSE;
		data->m_button.m_style = DFCS_BUTTONCHECK;		
		data->m_button.m_style |= (m_csaCheckedAktuell[item] == "1" ? DFCS_CHECKED : 0);

		data->m_text = m_update.GetAvailableNameAt(item);
#ifndef QUICKLIST_NOIMAGE
		data->m_image.m_imageID = item;
		data->m_image.m_imageList = &m_imgList;		
		data->m_image.m_noSelection = TRUE;
#endif	
	}
	else if (subItem == 1)
		data->m_text = m_update.GetAvailableSizeAt(item) + " kB";
	else if (subItem == 2)
	{
		if (m_csaCheckedOriginal[item] == "1" && m_update.GetAvailableActionAt(item) != "kein Update nötig")
		{
			data->m_text = "Update";
			data->m_colors.m_backColor = RGB( 160, 255, 160);
		}
		else
		{
			data->m_text = m_update.GetAvailableActionAt(item);
			if (data->m_text == "Update")
				data->m_colors.m_backColor = RGB( 160, 255, 160);
			else if (data->m_text == "Neuinstallation")
				data->m_colors.m_backColor = RGB( 255, 255, 160);
		}
	}

	data->m_tooltip = "Plugin selektieren für Info";

	return 0;
}

void CPluginManager::UpdateList()
{
	m_liste.LockWindowUpdate();
	m_liste.SetItemCount(m_update.GetNumberAvailable());
	m_liste.UnlockWindowUpdate();
	m_liste.RedrawItems(
			m_liste.GetTopIndex(),
			m_liste.GetTopIndex()+m_liste.GetCountPerPage());
}

void CPluginManager::Statusmeldung(CString &text) 
{
	UpdateData();
	if (m_status.GetLength())
	{
		m_status += "\r\n";
	}
	
	m_status += text;
	UpdateData(FALSE);

	// ((CEdit*)GetDlgItem(IDC_STATUS))->SetSel(m_status.GetLength(), m_status.GetLength()); <-- hier kam es gelegentlich zu Abstürzen
	// ((CEdit*)GetDlgItem(IDC_STATUS))->ReplaceSel((LPCSTR)text, TRUE); -- durch m_status += text; obsolet geworden
	// Ersatzcode:
	if (::IsWindow(GetDlgItem(IDC_STATUS)->m_hWnd))	// Update 3/2024: access violation v2.51.0.1-39e4f594-855f-44cc-a0eb-e435c6baee32
	{
		int line = ((CEdit*)GetDlgItem(IDC_STATUS))->GetLineCount(); // <-- hier kam es immer noch zu Abstürzen, weil das CEdit angeblich kein Fenster war... siehe drei Zeilen zuvor
		line -= 3;
		if (line < 1) line = 1;
		((CEdit*)GetDlgItem(IDC_STATUS))->LineScroll(line);
	}
}

void CPluginManager::Check()
{
	// Have CWebUpdate get this exe's path
	m_update.SetLocalDirectory("", true);
    
	// Set the URL for the update file and where downloads are stored
	m_update.SetUpdateFileURL("https://www.easyct.de/update.txt");
	m_update.SetRemoteURL("https://www.easyct.de");
	m_update.SetLocalDirectory(NULL, TRUE);	// Programmverzeichnis als Downloadpfad setzen
	CString csVerzeichnis = m_update.GetLocalDirectory() + "\\download";
	CreateDirectory((LPCTSTR)csVerzeichnis, NULL);
	m_update.SetLocalDirectory((LPCSTR)csVerzeichnis, FALSE);

	Statusmeldung((CString)"Suche nach Updates...");

	m_csaCheckedAktuell.RemoveAll();
	m_csaCheckedOriginal.RemoveAll();

	// Check for updates
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	if (m_update.DoUpdateCheck())
	{
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		CString csText;
		if (m_update.GetNumberMissing() && m_update.GetNumberDifferent())
			csText.Format("Update-Informationen wurden abgerufen.\r\n\
Es können %d Modul(e) neu installiert und %d Modul(e) aktualisiert werden.", 
				m_update.GetNumberMissing(), m_update.GetNumberDifferent());
		else if (m_update.GetNumberMissing())
		{
			if (m_update.GetNumberMissing() == 1)
				csText.Format("Update-Informationen wurden abgerufen.\r\n\
Es kann 1 Modul neu installiert werden.");
			else
				csText.Format("Update-Informationen wurden abgerufen.\r\n\
Es können %d Module neu installiert werden.", 
					m_update.GetNumberMissing());
		}
		else if (m_update.GetNumberDifferent())
		{
			if (m_update.GetNumberDifferent() == 1)
				csText.Format("Update-Informationen wurden abgerufen.\r\n\
Es kann 1 Modul aktualisiert werden.", 
					m_update.GetNumberDifferent());
			else
				csText.Format("Update-Informationen wurden abgerufen.\r\n\
Es können %d Module aktualisiert werden.", 
					m_update.GetNumberDifferent());
		}
		else 
		{
			csText.Format("Keine Updates verfügbar");
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		}
		Statusmeldung(csText);
	}
	else
	{
		Statusmeldung((CString)"Update-Informationen konnten nicht abgerufen werden.\r\n\
Entweder stimmt etwas nicht mit der Internet-Verbindung (Firewall checken) oder der Update-Server ist vorübergehend \r\n\
nicht erreichbar.");
		GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
		return;
	}
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	
	int i;
	for (i = 0; i < m_update.GetNumberAvailable(); i++)
	{  
		if (i == 0)
			m_csaCheckedAktuell.Add("1");
		else
		{
			CPluginElement *p;
			if (!m_pParent) break;
			p = m_pParent->m_pPlugins;

			while (p)
			{
				if (p->name == m_update.GetAvailableNameAt(i))
					break;

				p = p->next;
			}

			if (p)
				m_csaCheckedAktuell.Add("1");
			else
				m_csaCheckedAktuell.Add("0");
		}
		m_csaCheckedOriginal.Add(m_csaCheckedAktuell[i]);
	}

	UpdateList();
}

void CPluginManager::OnOK() 
{
	Download(FALSE);
}

void CPluginManager::OnBnClickedReparaturinstallation()
{
	Download(TRUE);
}

void CPluginManager::Download(BOOL bForce)
{
	/* alte Updater-Methode mit batch file -- damit auch unter Wine lauffähig: eigene exe statt cmd benutzen!
	CTime now = CTime::GetCurrentTime();
	CString csNow = now.Format("\\install%Y%m%d%H%M%S.bat");
	CStdioFile batch(m_update.GetLocalDirectory() + csNow, CFile::modeCreate|CFile::modeWrite);

	m_status = "Download der ausgewählten Komponenten. Bitte warten...";	// Statusmeldung löschen
	UpdateData(FALSE);

	int nZuInstallierendeModule = 0;
	BOOL bRestart = TRUE;	// wenn Hauptprogramm auch update, dann Restart zum Schluss unterdrücken (ist bereits Option im Setup-Script)
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	CWaitCursor wait;
	int i;
	for (i = m_update.GetNumberAvailable()-1; i >= 0 ; i--)	// Hauptprogramm ggf. als letztes starten
	{       
		if (m_update.GetAvailableActionAt(i) != "kein Update nötig" && m_csaCheckedAktuell[i] == "1")
		{
			if (m_update.DownloadAvailable(i))
			{
				Statusmeldung(m_update.GetAvailableNameAt(i) + " wurde erfolgreich heruntergeladen.");
				batch.WriteString("\"" + m_update.GetAvailableAt(i) + "\"\r\n");
				nZuInstallierendeModule++;
				if (i == 0) bRestart = FALSE;
			}
			else
				Statusmeldung(m_update.GetAvailableNameAt(i) + " konnte nicht heruntergeladen werden.");
		}
	}
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	// zum Schluss noch Befehl für Neustart in das Batchfile
	if (bRestart)
	{
		char module_filename[MAX_PATH] = "";
		GetModuleFileName(0, module_filename, sizeof(module_filename) - 1);
		batch.WriteString((CString)"\"" + module_filename + "\"");
	}

	batch.Close();
	*/

	CTime now = CTime::GetCurrentTime();
	CString csNow = now.Format("\\install%Y%m%d%H%M%S.txt");
	CStdioFile batch(m_update.GetLocalDirectory() + csNow, CFile::modeCreate|CFile::modeWrite);

	m_status = "Download der ausgewählten Komponenten. Bitte warten...";	// Statusmeldung löschen
	UpdateData(FALSE);

	int nZuInstallierendeModule = 0;
	BOOL bRestart = TRUE;	// wenn Hauptprogramm auch update, dann Restart zum Schluss unterdrücken (ist bereits Option im Setup-Script)
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	CWaitCursor wait;
	int i;
	for (i = m_update.GetNumberAvailable()-1; i >= 0 ; i--)	// Hauptprogramm ggf. als letztes starten
	{       
		if ((m_update.GetAvailableActionAt(i) != "kein Update nötig" || bForce) && m_csaCheckedAktuell[i] == "1")
		{
			if (m_update.DownloadAvailable(i))
			{
				Statusmeldung(m_update.GetAvailableNameAt(i) + " wurde erfolgreich heruntergeladen.");
				batch.WriteString(m_update.GetLocalDirectory() + "\\" + m_update.GetAvailableAt(i) + "\r\n");
				nZuInstallierendeModule++;
				if (i == 0) bRestart = FALSE;
			}
			else
				Statusmeldung(m_update.GetAvailableNameAt(i) + " konnte nicht heruntergeladen werden.");
		}
	}
	GetDlgItem(IDOK)->EnableWindow(TRUE);

	// zum Schluss noch Befehl für Neustart in das Batchfile
	if (bRestart)
	{
		char module_filename[MAX_PATH] = "";
		GetModuleFileName(0, module_filename, sizeof(module_filename) - 1);
		batch.WriteString(module_filename);
	}

	batch.Close();

	if (nZuInstallierendeModule >= 1)
	{
		// Neustart mit Installation
		if (AfxMessageBox("Zur Installation der heruntergeladenen Module muss das Programm kurz geschlossen werden. (Sie werden ggf. zum Speichern noch ungesicherter Daten aufgefordert.)", MB_OKCANCEL) == IDOK)
		{
			AfxGetMainWnd()->PostMessage(WM_CLOSE, 0, 0L);
			char buffer[1000];
			if (GetProgrammverzeichnis(buffer, sizeof(buffer)))
			{
				if (CopyFile((CString)buffer + "\\ECTUpdater.exe", m_update.GetLocalDirectory() + "\\ECTUpdater.exe", FALSE))
					ShellExecute(NULL, "open", m_update.GetLocalDirectory() + "\\ECTUpdater.exe", m_update.GetLocalDirectory() + csNow, m_update.GetLocalDirectory(), SW_MINIMIZE);
				else
					AfxMessageBox("Konnte das Update-Programm nicht in das temporäre Verzeichnis '" + m_update.GetLocalDirectory() + "' kopieren. Gehen Sie mit dem Windows-Explorer in dieses Verzeichnis und führen die Setup-Dateien einzeln manuell aus. Hinweis: 'C:\\ProgramData' ist normalerweise ein verstecktes Verzeichnis, in das man nur gelangt, wenn man den Verzeichnisnamen oben in die Navigationszeile des Windows-Explorers eintippt.");
			}
		}
		else
			AfxMessageBox("Updatevorgang wurde abgebrochen. Die heruntergeladenen Komponenten werden mit 'kein Update nötig' angezeigt bis ein weiteres Update der Betroffenenn Komponente(n) zur Verfügung steht. Wenn Sie die heruntergeladenen Komponenten später doch noch installieren möchten, starten Sie '" + m_update.GetLocalDirectory() + csNow + "' oder laden sich die Installationsdateien manuell von www.easyct.de -> Downloads -> Plugins herunter.");

		CDialog::OnOK();
	}
	else
		AfxMessageBox("Keine Module zum Download ausgewählt.");

}

void CPluginManager::OnTimer(UINT nIDEvent) 
{
	KillTimer(1);

	Check();

	m_pbmp;
	CString csBmpFilePath = m_update.GetLocalDirectory() + "\\update.png";
	if (m_pParent)
	{
		m_pParent->LoadBitmap(&m_pbmp, csBmpFilePath.GetBuffer(0));
		if (m_pbmp)
		{
			m_imgList.Add(m_pbmp, RGB(255,0,255));
			m_pbmp->GetBitmapDimension();
		}
	}

	CDialog::OnTimer(nIDEvent);
}


void CPluginManager::OnClickListe(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int n;
	POSITION pos = m_liste.GetFirstSelectedItemPosition();
	if (pos)
		n = m_liste.GetNextSelectedItem(pos);
	else
		n = -1;

	if (n >= 0)
	{
		m_status = m_update.GetAvailableNoteAt(n);
		UpdateData(FALSE);
	}
	
	*pResult = 0;
}

void CPluginManager::OnCancel() 
{
	if (m_pbmp) { delete m_pbmp; m_pbmp = NULL; }
	
	CDialog::OnCancel();
}
