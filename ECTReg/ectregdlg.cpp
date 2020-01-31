// ECTRegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ECTReg.h"
#include "ECTRegDlg.h"
#include <afxdao.h>  

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

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
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CECTRegDlg dialog

CECTRegDlg::CECTRegDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CECTRegDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CECTRegDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CECTRegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CECTRegDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CECTRegDlg, CDialog)
	//{{AFX_MSG_MAP(CECTRegDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DATENBANK, OnDatenbank)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_EN_CHANGE(IDC_KEY, OnChangeKey)
	ON_EN_CHANGE(IDC_NAME1, OnChangeName1)
	ON_EN_CHANGE(IDC_NAME2, OnChangeName2)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CECTRegDlg message handlers

BOOL CECTRegDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	SetTimer(1, 1, NULL);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CECTRegDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CECTRegDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CECTRegDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CECTRegDlg::OnChangeEdit1() 
{
	CString text;
	GetDlgItemText(IDC_EDIT1, text);
	char *cp;
	char *cp2;
	char name[1000];
	char email[1000];
	char unknown[1000];
	memset(unknown, 0x00, sizeof(unknown));
	cp = text.GetBuffer(0);
	
	if (cp = strstr(cp, "From:"))
	{
		cp += 5;

		while (*cp == ' ' || *cp == '\t')
			cp++;

		while (*cp != '\r' && *cp != '\n' && *cp != '\0')
		{
			if (*cp == '<')
			{
				cp++;
				memset(email, 0x00, sizeof(email));
				cp2 = email;
				while (*cp != '>')
				{
					*cp2++ = *cp++;					
				}
				cp++;
			}

			if (*cp == '(')
			{
				cp++;
				memset(name, 0x00, sizeof(name));
				cp2 = name;
				while (*cp != ')')
				{
					*cp2++ = *cp++;					
				}
				cp++;
			}

			if (*cp == '"')
			{
				cp++;
				memset(name, 0x00, sizeof(name));
				cp2 = name;
				while (*cp != '"')
				{
					*cp2++ = *cp++;					
				}
				cp++;
			}

			if (*cp == '\r' || *cp == '\n' || *cp == '\0') break;

			if (*cp == ' ' || *cp == '\t')
			{
				cp++;
			}
			else
			{
				cp2 = unknown;
				if (*cp2)
				{
					strcat(cp2, " ");
					cp2 += strlen(cp2);
				}
				while (*cp != ' ' && *cp != '\t' && *cp != '\r' && *cp != '\n' && *cp != '\0')
				{
					*cp2++ = *cp++;					
				}
				cp++;
			}
		}
		if (!*email) strcpy(email, unknown);
		else if (!*name) strcpy(name, unknown);

		cp = strchr((LPCSTR)name, ',');
		if (cp)
		{
			char first[1000];
			char last [1000];
			memset (first, 0, sizeof(first));
			memset(last, 0, sizeof(last));
			strncpy(last, name, cp-name);
			while (*cp == ' ' || *cp == '\t' || *cp == ',')
				cp++;
			strcpy(first, cp);
			strcpy(name, first); 
			strcat(name, " ");
			strcat(name, last);
		}

		SetDlgItemText(IDC_NAME1, name);
		SetDlgItemText(IDC_EMAIL, email);
	}
}

void CECTRegDlg::OnChangeName1() 
{
	CString text;
	char buffer[1000];
	GetDlgItemText(IDC_NAME1, text);

	char *cp;
	cp = text.GetBuffer(0);

	strcpy(buffer, "#");
	strncat(buffer, cp, 1);
	strcat(buffer, ".");
	cp = strchr(cp, ' ');
	if (!cp) return;
	while (*cp == ' ')
		cp++;
	strcat(buffer, cp);
	buffer[12] = '\0';

	SetDlgItemText(IDC_NAME2, buffer);
}

static char key[] = "34095861351326513487618376284719023560138471016658084735618435605611256781287463083176180763856461026";
static char keyindex[] = {
	68, 69, 61, 60, 66, 63, 64, 67, 62, 65, 
	 8,  9,  1,  0,  6,  3,  4,  7,  2,  5, 
	18, 19, 11, 10, 16, 13, 14, 17, 12, 15, 
	48, 49, 41, 40, 46, 43, 44, 47, 42, 45, 
	58, 59, 51, 50, 56, 53, 54, 57, 52, 55, 
	98, 99, 91, 90, 96, 93, 94, 97, 92, 95, 
	38, 39, 31, 30, 36, 33, 34, 37, 32, 35, 
	28, 29, 21, 20, 26, 23, 24, 27, 22, 25, 
	88, 89, 81, 80, 86, 83, 84, 87, 82, 85, 
	78, 79, 71, 70, 76, 73, 74, 77, 72, 75, 
};

void Crypt(char *into, char *s)
{
	int i;
//strcpy(into, s); return;
	for (i = 0; i < (int)strlen(s); i++)
	{
		into[i] = ((s[i] - '0') - (key[keyindex[i]] - '0') % 10);
		if (into[i] < 0)
			into[i] += 10;
		into[i] += '0';
	}
	into[i] = 0;
}

void RegName2Nr(char *into, char *s)
{
	int i;
	int t;
	char buffer[100];

	if (strlen(s) > 12)
	{
		*into = 0;
		return;
	}

	for (i = 0; i < (int)strlen(s) * 3; i+=3)
	{
		t = s[i/3] - ' ';
		into[i]   = (t / 10) % 10 + '0';
		into[i+1] = t % 10 + '0';
		into[i+2] = ((into[i] - '0') - (into[i+1] - '0'));
		if (into[i+2] < 0)
			into[i+2] += 10;
		into[i+2] += '0';
	}
	into[i] = 0;
	Crypt(buffer, into);
	strcpy(into, buffer);
}

void CECTRegDlg::OnChangeName2() 
{
	char buffer[100];
	char buffer1[100];
	char buffer2[100];

	GetDlgItemText(IDC_NAME2, buffer, sizeof(buffer));
	if (strlen(buffer) > 12)
		return;
	sprintf(buffer1, "%-12.12s", buffer);
	RegName2Nr(buffer2, buffer1);
	SetDlgItemText(IDC_KEY, buffer2);		
}

void CECTRegDlg::OnChangeKey() 
{
	char *liste;
	long size_of_liste;
	CString key;
	GetDlgItemText(IDC_KEY, key);
	if (key.GetLength() != 36) return;

	CFile pFile;
	
	if (pFile.Open("D:\\Doc\\EasyCash\\TestReg.txt", CFile::modeRead))
	{
		liste = new char[(size_of_liste = pFile.GetLength() + 20)]; 
		
		memset(liste, '\0', size_of_liste);
		
		pFile.Seek(0, CFile::begin);
		pFile.Read(liste, size_of_liste);

		pFile.Close();
	}
	else
	{
		liste = new char[(size_of_liste = 20)];
		memset(liste, '\0', size_of_liste);
	}
	
	char *cp = strstr(liste, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	if (cp)
		strncpy(cp, (LPCSTR)key, 36);

	SetDlgItemText(IDC_EDIT3, liste);

	if (liste) delete liste;
}

void FillVariant(COleVariant & var, LPCTSTR lpszSrc)
{
    var.vt=VT_BSTR;
    if (lpszSrc==NULL)
        var.bstrVal=NULL;
    else
    {
#ifndef _UNICODE
    int nLen = lstrlen(lpszSrc);
    var.bstrVal = SysAllocStringByteLen(lpszSrc, nLen);
#else
    var.bstrVal = SysAllocString(lpszSrc);
#endif
    }
}

void CECTRegDlg::OnDatenbank() 
{
	CString name;
	CString email;
	CString key;
	CString bemerkung;
	GetDlgItemText(IDC_NAME1, name);
	GetDlgItemText(IDC_EMAIL, email);
	GetDlgItemText(IDC_KEY, key);
	GetDlgItemText(IDC_BEMERKUNG, bemerkung);

    try
    {
		CDaoDatabase db;
		db.Open("D:\\Doc\\EasyCash\\easyreg.mdb");
		CDaoTableDef table(&db);
		table.Open("Registrierungen");
		CDaoRecordset recset(&db);
		recset.Open(&table, dbOpenTable);
		recset.AddNew();
		COleVariant var;
		if (name.GetLength() == 0) name = " ";
		FillVariant(var, name);
		recset.SetFieldValue("Name", var);
		if (email.GetLength() == 0) email = " ";
		FillVariant(var, email);
		recset.SetFieldValue("E-Mail", var);
		recset.SetFieldValue("Test-Reg", COleVariant(1L, VT_I4));
		recset.SetFieldValue("Voll-Reg", COleVariant(0L, VT_I4));
		recset.SetFieldValue("Bezahlt", COleVariant(0L, VT_I4));
		if (key.GetLength() == 0) key = " ";
		FillVariant(var, key);
		recset.SetFieldValue("Reg-Nr", var);
		recset.SetFieldValue("Rech-Nr", COleVariant(0L, VT_I4));
		if (bemerkung.GetLength() == 0) bemerkung = " ";
		FillVariant(var, bemerkung);
		recset.SetFieldValue("Bemerkungen", var);
		recset.Update();
		recset.Close();
		db.Close();
    }
    catch( CDaoException* e )
    {
         AfxMessageBox( 
                e->m_pErrorInfo->m_strDescription, 
                MB_ICONEXCLAMATION );
        e->Delete();
		return;
    }

// ---	

	char textbuffer1[20000];
	char textbuffer2[10000];
	memset(textbuffer1, 0, sizeof(textbuffer1));
	memset(textbuffer2, 0, sizeof(textbuffer2));
	GetDlgItemText(IDC_EDIT2, textbuffer1, sizeof(textbuffer1));
	GetDlgItemText(IDC_EDIT3, textbuffer2, sizeof(textbuffer2));
	if (strlen(textbuffer1))
	{
		strcat(textbuffer1, "\n\nNun zum offiziellen Teil dieser mail...\n---8<------\n");
	}
	strcat(textbuffer1, textbuffer2);

	CFile pFile;
	
	if (pFile.Open("D:\\Doc\\EasyCash\\Mails\\Email.txt", CFile::modeCreate | CFile::modeWrite))
	{
		pFile.Write(textbuffer1, strlen(textbuffer1));
		pFile.Close();
	}
	else
	{
		AfxMessageBox("Konnte Datei nicht schreiben! Satz ist aber in der Datenbank!");
		return;
	}

	char buffer1[1000];

	sprintf(buffer1, "mailto:%s?subject=ECT-Registrierung&body=D:\\Doc\\EasyCash\\Mails\\Email.txt", (LPCSTR)email);

	int n = (int)ShellExecute(m_hWnd, "open", buffer1, NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Es gab ein kleines Problem mit dem E-Mail Programm auf diesem Rechner.", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Der Registriercode wurde verschickt.", MB_ICONINFORMATION);
	}

// ---

	SetDlgItemText(IDC_EDIT1, "");
	SetDlgItemText(IDC_EDIT2, "");
	SetDlgItemText(IDC_NAME1, "");
	SetDlgItemText(IDC_NAME2, "");
	SetDlgItemText(IDC_EMAIL, "");
	SetDlgItemText(IDC_KEY, "");
	SetDlgItemText(IDC_BEMERKUNG, "");
	SetDlgItemText(IDC_EDIT3, "");

	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void CECTRegDlg::OnTimer(UINT nIDEvent) 
{
	KillTimer(1);

	extern CECTRegApp theApp;
	if (*theApp.m_lpCmdLine)
	{
		char *liste;
		long size_of_liste;

		CFile pFile;
		
		if (pFile.Open(theApp.m_lpCmdLine, CFile::modeRead))
		{
			liste = new char[(size_of_liste = pFile.GetLength() + 20)]; 
			
			memset(liste, '\0', size_of_liste);
			
			pFile.Seek(0, CFile::begin);
			pFile.Read(liste, size_of_liste);

			pFile.Close();
		}
		else
		{
			liste = new char[(size_of_liste = 20)];
			memset(liste, '\0', size_of_liste);
		}
		
		if (liste)
		{
			SetDlgItemText(IDC_EDIT1, liste);
			delete liste;
		
			OnChangeEdit1();
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}
