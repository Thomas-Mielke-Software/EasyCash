// UstVorauszahlungenDlg.cpp : implementation file
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
#include "UstVorauszahlungenDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUstVorauszahlungenDlg dialog


CUstVorauszahlungenDlg::CUstVorauszahlungenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUstVorauszahlungenDlg::IDD, pParent)
	, m_vorauszahlung(_T(""))
{
	//{{AFX_DATA_INIT(CUstVorauszahlungenDlg)
	m_m1 = _T("");
	m_m2 = _T("");
	m_m3 = _T("");
	m_m4 = _T("");
	m_m5 = _T("");
	m_m6 = _T("");
	m_m7 = _T("");
	m_m8 = _T("");
	m_m9 = _T("");
	m_m10 = _T("");
	m_m11 = _T("");
	m_m12 = _T("");
	m_q1 = _T("");
	m_q2 = _T("");
	m_q3 = _T("");
	m_q4 = _T("");
	//}}AFX_DATA_INIT
}


void CUstVorauszahlungenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUstVorauszahlungenDlg)
	DDX_Text(pDX, IDC_M1, m_m1);
	DDX_Text(pDX, IDC_M2, m_m2);
	DDX_Text(pDX, IDC_M3, m_m3);
	DDX_Text(pDX, IDC_M4, m_m4);
	DDX_Text(pDX, IDC_M5, m_m5);
	DDX_Text(pDX, IDC_M6, m_m6);
	DDX_Text(pDX, IDC_M7, m_m7);
	DDX_Text(pDX, IDC_M8, m_m8);
	DDX_Text(pDX, IDC_M9, m_m9);
	DDX_Text(pDX, IDC_M10, m_m10);
	DDX_Text(pDX, IDC_M11, m_m11);
	DDX_Text(pDX, IDC_M12, m_m12);
	DDX_Text(pDX, IDC_Q1, m_q1);
	DDX_Text(pDX, IDC_Q2, m_q2);
	DDX_Text(pDX, IDC_Q3, m_q3);
	DDX_Text(pDX, IDC_Q4, m_q4);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_VORAUSZAHLUNG, m_vorauszahlung);
}


BEGIN_MESSAGE_MAP(CUstVorauszahlungenDlg, CDialog)
	//{{AFX_MSG_MAP(CUstVorauszahlungenDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUstVorauszahlungenDlg message handlers

BOOL CUstVorauszahlungenDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	
	
	return TRUE; 
}
