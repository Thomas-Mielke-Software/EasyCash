// DatenverzeichnisDlg.cpp : implementation file
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
#include "DatenverzeichnisDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDatenverzeichnisDlg dialog


CDatenverzeichnisDlg::CDatenverzeichnisDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDatenverzeichnisDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDatenverzeichnisDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDatenverzeichnisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDatenverzeichnisDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDatenverzeichnisDlg, CDialog)
	//{{AFX_MSG_MAP(CDatenverzeichnisDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDatenverzeichnisDlg message handlers
