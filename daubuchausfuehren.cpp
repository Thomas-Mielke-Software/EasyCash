// DauBuchAusfuehren.cpp : implementation file
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
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "BuchenDlg.h"
#include "DauBuchAusfuehren.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DauBuchAusfuehren dialog


DauBuchAusfuehren::DauBuchAusfuehren(CEasyCashDoc *pDoc, CWnd* pParent /*=NULL*/)
	: CDialog(DauBuchAusfuehren::IDD, pParent)
{
	//{{AFX_DATA_INIT(DauBuchAusfuehren)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pDoc = pDoc;
	m_pParent = pParent;
}


void DauBuchAusfuehren::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DauBuchAusfuehren)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DauBuchAusfuehren, CDialog)
	//{{AFX_MSG_MAP(DauBuchAusfuehren)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DauBuchAusfuehren message handlers

BOOL DauBuchAusfuehren::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CTime now = CTime::GetCurrentTime();
	char buf[100];

	if (now.GetYear() == m_pDoc->nJahr)
	{
		sprintf(buf, "%d", (int)now.GetMonth());
		SetDlgItemText(IDC_DATUM_BIS_MONAT, buf);
		sprintf(buf, "%d", (int)now.GetYear());
		SetDlgItemText(IDC_DATUM_BIS_JAHR, buf);
	}
	else
	{
		SetDlgItemText(IDC_DATUM_BIS_MONAT, "12");
		sprintf(buf, "%d", (int)m_pDoc->nJahr);
		SetDlgItemText(IDC_DATUM_BIS_JAHR, buf);
	}

	Button1Bmp.LoadBitmap(IDB_BUTTON1);
	((CButton *)GetDlgItem(IDOK))->SetBitmap(Button1Bmp);
	StopBmp.LoadBitmap(IDB_STOP);
	((CButton *)GetDlgItem(IDCANCEL))->SetBitmap(StopBmp);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DauBuchAusfuehren::OnOK() 
{
	char buf[300];

	// ausführen bis zu welchem datum ?
	GetDlgItemText(IDC_DATUM_BIS_MONAT, buf, sizeof(buf)); m_mb = atoi(buf);
	GetDlgItemText(IDC_DATUM_BIS_JAHR, buf, sizeof(buf)); m_jb = atoi(buf);
	CDialog::OnOK();
}
