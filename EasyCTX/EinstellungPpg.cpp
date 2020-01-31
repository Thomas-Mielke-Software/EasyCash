// EinstellungPpg.cpp : Implementation of the CEinstellungPropPage property page class.
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
#include "EinstellungPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CEinstellungPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CEinstellungPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CEinstellungPropPage)
	ON_BN_CLICKED(IDC_WWW, OnWww)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CEinstellungPropPage, "EASYCTX.EinstellungPropPage.1",
	0xf68eb5d2, 0xea65, 0x4f01, 0xb9, 0x6b, 0x95, 0x5f, 0x3f, 0x14, 0xd0, 0x8f)


/////////////////////////////////////////////////////////////////////////////
// CEinstellungPropPage::CEinstellungPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CEinstellungPropPage

BOOL CEinstellungPropPage::CEinstellungPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_EINSTELLUNG_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungPropPage::CEinstellungPropPage - Constructor

CEinstellungPropPage::CEinstellungPropPage() :
	COlePropertyPage(IDD, IDS_EINSTELLUNG_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CEinstellungPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT

	SetHelpInfo(_T("Names to appear in the control"), _T("EASYCTX.HLP"), 0);
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungPropPage::DoDataExchange - Moves data between page and properties

void CEinstellungPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CEinstellungPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungPropPage message handlers

void CEinstellungPropPage::OnWww() 
{
	ShellExecute(m_hWnd, "open", "http://easyct.no-ip.org/readarticle.php?article_id=23#Einstellung", 
		NULL, ".", SW_SHOWNORMAL);		
}
