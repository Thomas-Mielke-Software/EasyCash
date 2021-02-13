// ChildFrm.cpp : implementation of the CChildFrame class
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
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "Navigation.h"

#include "ExtSplitter.h"
#include "ChildFrm.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	//{{AFX_MSG_MAP(CChildFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_WM_MDIACTIVATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_bInitSplitter = false;
	
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if(CMDIChildWndEx::PreCreateWindow(cs)==0) return FALSE;
	cs.style &= ~(LONG)FWS_ADDTOTITLE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
    CRect cr;  
    GetWindowRect( &cr );  

    if (!m_wndSplitter.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE /* | WS_VSCROLL*/))   
    {   
        MessageBox("Fehler beim erzeugen des geteilten Journalfensters (CreateStatic).", "Initialisierungsfehler", MB_OK | MB_ICONERROR);   
        return FALSE;   
    }  

    if (!m_wndSplitter.CreateView( 0, 0, RUNTIME_CLASS(CEasyCashView), CSize(cr.Width()/2, cr.Height()), pContext ))   
    {   
        MessageBox("Fehler beim erzeugen des geteilten Journalfensters (CEasyCashView).", "Initialisierungsfehler", MB_OK | MB_ICONERROR);  
        return FALSE;   
    }  

    if (!m_wndSplitter.CreateView( 0, 1, RUNTIME_CLASS(CNavigation), CSize(cr.Width()/2, cr.Height()), pContext))   
    {    
        MessageBox("Fehler beim erzeugen des geteilten Journalfensters (CNavigation).", "Initialisierungsfehler", MB_OK | MB_ICONERROR);  
		return FALSE;   
    } 

	// Navigaion und view sollen voneinander wissen:
	((CEasyCashView*)m_wndSplitter.GetPane(0, 0))->m_pNavigationWnd = (CNavigation*)m_wndSplitter.GetPane(0, 1);
	((CNavigation*)m_wndSplitter.GetPane(0, 1))->m_pViewWnd = (CEasyCashView*)m_wndSplitter.GetPane(0, 0);

    m_bInitSplitter = TRUE;  

    return TRUE;  
}

void CChildFrame::ActivateFrame(int nCmdShow) 
{

	// die plug-ins
	//if (AfxGetMainWnd()->GetMenu()->InsertMenu(4, MF_BYPOSITION|MF_STRING, 0, "Erweiterungen"))
	{
//VS9		CMenu *pm = AfxGetMainWnd()->GetMenu()->GetSubMenu(3);	
//VS9
/*		ExtensionDLLsClass *pExtensionDLLs = ExtensionDLLs;
		if (pExtensionDLLs)
		{
			int i = pm->GetMenuItemCount();
			while (i > 15 && pm->GetMenuItemID(--i) != MF_SEPARATOR)
				pm->RemoveMenu(i, MF_BYPOSITION);

			pm->AppendMenu(MF_STRING|MF_SEPARATOR, 0);	
			while (pExtensionDLLs)
			{
				pm->AppendMenu(MF_STRING, pExtensionDLLs->menu_item_id, pExtensionDLLs->menu_item_string);
				//pm->EnableMenuItem(pExtensionDLLs->menu_item_id, MF_BYCOMMAND|MF_ENABLED);
				pExtensionDLLs = pExtensionDLLs->next;
//VS9			}
//VS9		}
*/	}

    // if another window is open, use default
    if(GetMDIFrame()->MDIGetActive())
    {
        CMDIChildWnd::ActivateFrame(nCmdShow);
    }
    else // else open the child window maximized.
    {
        CMDIChildWnd::ActivateFrame(SW_SHOWMAXIMIZED);
    }	
	
	CMDIChildWndEx::ActivateFrame(nCmdShow);
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	// // Saldo im status bar aktualisieren, z.B. bei Strg-Tab
	if (bActivate)
		((CMainFrame*)m_pMDIFrame)->UpdateSaldo();
}

void CChildFrame::OnSize(UINT nType, int cx, int cy)
{
    CMDIChildWnd::OnSize(nType, cx, cy);  
    CRect cr;  
    GetWindowRect(&cr);  

    if (m_bInitSplitter && nType != SIZE_MINIMIZED)  
    {  
		int nNavBreite = theApp.GetProfileInt("Fenster", "NavigationBreitenverhaeltnis", 150);  // Nav-Leiste Breite in promille der Gesamtfensterbreite
		int nNavBreiteAbsolut = nNavBreite * cx / 1000;

        m_wndSplitter.SetRowInfo( 0, cy, 0 );  
		if (cx < 2 * nNavBreiteAbsolut)
		{
			m_wndSplitter.SetColumnInfo(0, cr.Width() * 5 / 10, 50);  
			m_wndSplitter.SetColumnInfo(1, cr.Width() * 5 / 10, 50);  
		}
		else
		{
			m_wndSplitter.SetColumnInfo(0, cr.Width() - nNavBreiteAbsolut, 50);  
			m_wndSplitter.SetColumnInfo(1, nNavBreiteAbsolut, 50);  
		}

        m_wndSplitter.RecalcLayout();  
    }
}
