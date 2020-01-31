// RegDlg.cpp : implementation file
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
#include "RegDlg.h"
#include "ECTIFace\EasyCashDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////
// dies hier Auskommentieren!!! ///
///////////////////////////////////
//#define MASTER_VERSION


#ifdef MASTER_VERSION
	#pragma message("Master-Version wird erstellt")
#endif

/////////////////////////////////////////////////////////////////////////////
// CRegDlg dialog


CRegDlg::CRegDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegDlg, CDialog)
	//{{AFX_MSG_MAP(CRegDlg)
	ON_BN_CLICKED(IDC_MAKE_NR, OnMakeNr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegDlg message handlers

BOOL CRegDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	((CEdit *)GetDlgItem(IDC_REG_EDIT))->LimitText(40);
	CString RegKey = theApp.GetProfileString("Allgemein", "RegKey", "");
	SetDlgItemText(IDC_REG_EDIT, RegKey);

#ifndef MASTER_VERSION
	((CEdit *)GetDlgItem(IDC_MAKE_NR))->ShowWindow(SW_HIDE);
	((CEdit *)GetDlgItem(IDC_TESTVERSION))->ShowWindow(SW_HIDE);
#endif
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRegDlg::OnOK() 
{
	extern BOOL CheckReg(char *);
	char buffer0[100];
	char buffer[100];
	char *cp1, *cp2;

	GetDlgItemText(IDC_REG_EDIT, buffer0, sizeof(buffer0));
	cp1 = buffer0;
	cp2 = buffer;
	while (*cp2++ = *cp1++)
		if (!isdigit(*(cp2-1)))
			cp2--;

	if (CheckReg(buffer))
	{
		shareware = FALSE;
		theApp.WriteProfileString(
			"Allgemein",	// pointer to section name 
			"RegKey",		// pointer to key name 
			buffer);	
		if (AfxMessageBox("Registrierung ist erfolgt. Soll EC&T den Registriercode zusammen mit anderen globalen Programmeinstellungen jetzt für den Fall einer Windows-Neuinstallation oder Systemwiederherstellung auf ein externes Medium sichern? (empfohlen)", MB_YESNO) == IDYES)
		{
			theApp.RegistrierungsinformationenSichern(NULL);
		}

		CDialog::OnOK();
	}
	else
		MessageBox("Registrier-Nr. ist unrichtig.", "Fehler", MB_ICONSTOP|MB_OK);
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

void Decrypt(char *into, char *s)
{
	int i;
//strcpy(into, s); return;
	for (i = 0; i < (int)strlen(s); i++)
	{
		into[i] = (((s[i] - '0') + (key[keyindex[i]] - '0')) % 10) + '0';
	}
	into[i]=0;
}

#ifdef MASTER_VERSION
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
#endif

// Registriernummern müssen durch 3 teilbar sein
BOOL CheckReg(char *s)
{
	char buffer[100];
	int i;

	if (strlen(s) >= sizeof(buffer)) return FALSE;
	if (strlen(s) != 36) return FALSE;

	Decrypt(buffer, s);

	for (i = 0; i < (int)strlen(s); i+=3)
	{
		if (((buffer[i] - '0') - (buffer[i + 1] - '0')) % 10 != buffer[i + 2] - '0'
		 && ((buffer[i] - '0') - (buffer[i + 1] - '0') + 10) % 10 != buffer[i + 2] - '0')
			return FALSE;
	}

	return TRUE;
}

void GetRegName(char *into, char *s)
{
	int i;
	char buffer[100];

	if (!CheckReg(s))
	{
		*into = 0;
		return;
	}

	Decrypt(buffer, s);
	
	for (i = 0; i < (int)strlen(buffer); i+=3)
		into[i/3] = (buffer[i] - '0') * 10 + buffer[i+1] - '0' + ' ';
	into[i/3] = 0;
}

#ifdef MASTER_VERSION
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
#endif

void CRegDlg::OnMakeNr() 
{
#ifdef MASTER_VERSION
	char buffer[100];
	char buffer1[100];
	char buffer2[100];

	GetDlgItemText(IDC_REG_EDIT, buffer, sizeof(buffer));
	if (strlen(buffer) > 12)
		return;
	sprintf(buffer1, "%-12.12s", buffer);
	RegName2Nr(buffer2, buffer1);
	SetDlgItemText(IDC_REG_EDIT, buffer2);	
#endif
	;
}
