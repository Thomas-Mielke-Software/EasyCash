// BuchungsjahrWaehlen.cpp : implementation file
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
#include "resource.h"
#include "BuchungsjahrWaehlen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBuchungsjahrWaehlen dialog


CBuchungsjahrWaehlen::CBuchungsjahrWaehlen(CWnd* pParent /*=NULL*/)
	: CDialog(CBuchungsjahrWaehlen::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBuchungsjahrWaehlen)
	m_jahr = 0;
	m_waehrung = _T("");
	//}}AFX_DATA_INIT
}


void CBuchungsjahrWaehlen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBuchungsjahrWaehlen)
	DDX_Text(pDX, IDC_JAHR, m_jahr);
	DDV_MinMaxInt(pDX, m_jahr, 1900, 2035);
	DDX_Text(pDX, IDC_WAEHRUNG, m_waehrung);
	DDV_MaxChars(pDX, m_waehrung, 3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBuchungsjahrWaehlen, CDialog)
	//{{AFX_MSG_MAP(CBuchungsjahrWaehlen)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBuchungsjahrWaehlen message handlers
