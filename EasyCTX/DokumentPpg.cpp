// DokumentPpg.cpp : Implementation of the CDokumentPropPage property page class.
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
#include "EasyCTX.h"
#include "DokumentPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CDokumentPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CDokumentPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CDokumentPropPage)
	ON_BN_CLICKED(IDC_WWW, OnWww)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CDokumentPropPage, "EASYCTX.DokumentPropPage.1",
	0x7daabd56, 0x2796, 0x4da1, 0xa3, 0xb2, 0xac, 0x85, 0, 0xe1, 0x2a, 0x8)


/////////////////////////////////////////////////////////////////////////////
// CDokumentPropPage::CDokumentPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CDokumentPropPage

BOOL CDokumentPropPage::CDokumentPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_DOKUMENT_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentPropPage::CDokumentPropPage - Constructor

CDokumentPropPage::CDokumentPropPage() :
	COlePropertyPage(IDD, IDS_DOKUMENT_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CDokumentPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT

	SetHelpInfo(_T("Names to appear in the control"), _T("EASYCTX.HLP"), 0);
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentPropPage::DoDataExchange - Moves data between page and properties

void CDokumentPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CDokumentPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentPropPage message handlers

void CDokumentPropPage::OnWww() 
{
	ShellExecute(m_hWnd, "open", "http://easyct.no-ip.org/readarticle.php?article_id=23#Dokument", 
		NULL, ".", SW_SHOWNORMAL);		
}
