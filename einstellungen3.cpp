// Einstellungen3.cpp : implementation file
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
#include "Einstellungen3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen3 property page

IMPLEMENT_DYNCREATE(CEinstellungen3, CPropertyPage)

CEinstellungen3::CEinstellungen3() : CPropertyPage(CEinstellungen3::IDD)
{
	//{{AFX_DATA_INIT(CEinstellungen3)
	m_name = _T("");
	m_name2 = _T("");
	m_ort = _T("");
	m_plz = _T("");
	m_steuernummer = _T("");
	m_strasse = _T("");
	//}}AFX_DATA_INIT
}

CEinstellungen3::~CEinstellungen3()
{
}

void CEinstellungen3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEinstellungen3)
	DDX_Text(pDX, IDC_NAME, m_name);
	DDV_MaxChars(pDX, m_name, 30);
	DDX_Text(pDX, IDC_NAME2, m_name2);
	DDV_MaxChars(pDX, m_name2, 30);
	DDX_Text(pDX, IDC_ORT, m_ort);
	DDV_MaxChars(pDX, m_ort, 24);
	DDX_Text(pDX, IDC_PLZ, m_plz);
	DDV_MaxChars(pDX, m_plz, 5);
	DDX_Text(pDX, IDC_STEUERNUMMER, m_steuernummer);
	DDV_MaxChars(pDX, m_steuernummer, 19);
	DDX_Text(pDX, IDC_STRASSE, m_strasse);
	DDV_MaxChars(pDX, m_strasse, 30);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEinstellungen3, CPropertyPage)
	//{{AFX_MSG_MAP(CEinstellungen3)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEinstellungen3 message handlers
