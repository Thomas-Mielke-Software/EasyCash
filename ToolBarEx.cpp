// ToolBarEx.cpp : implementation file
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
#include "MainFrm.h"
#include "EasyCashView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolBarWithCombo

BEGIN_MESSAGE_MAP(CToolBarWithCombo, CToolBar)
	//{{AFX_MSG_MAP(CToolBarWithCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CToolBarWithCombo::CreateComboBox(CComboBox& comboBox, UINT nIndex, UINT nID, 
								int nWidth, int nDropHeight)
{
	// Create the combo box
	SetButtonInfo(nIndex, nID, TBBS_SEPARATOR, nWidth);

	CRect rect;
	GetItemRect(nIndex, &rect);
	rect.top = 1;
	rect.bottom = rect.top + nDropHeight;
	if (!comboBox.Create(
			CBS_DROPDOWN|WS_VISIBLE|WS_TABSTOP|WS_VSCROLL,
			rect, this, nID))
	{
		TRACE("Failed to create combo-box\n");
		return FALSE;
	}

	return TRUE;
}

BOOL CToolBarWithCombo::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if( wParam == IDOK && lParam == 0 )
	{
		CString csText;
		m_comboBox1.GetWindowText(csText);

		if(!csText.IsEmpty())
		{
			m_comboBox1.AddString(csText);
			csText.MakeLower();

			CMainFrame *wndMainFrame = (CMainFrame*)GetParent()->GetParent();
			CMDIChildWndEx *pChild = (CMDIChildWndEx *)wndMainFrame->GetActiveFrame();
			CEasyCashView *pecv = (CEasyCashView*)pChild->GetActiveView();
			if (pecv && (pecv->IsKindOf( RUNTIME_CLASS( CEasyCashView ) ) || pecv->IsKindOf( RUNTIME_CLASS( CScrollView ) )))
			{
				POINT scrollpos = pecv->GetScrollPosition();
				int line = scrollpos.y / pecv->charheight;
				if (pecv->nSelected >= 0) line = pecv->nSelected + 1;
				CString csAktuelleBelegnummer;
				CString csAktuelleBeschreibung;
				int i;
				for (i = line; i < MAX_BUCHUNGEN; i++)
					if (pecv->ppPosBuchungsliste[i])
					{
						CBuchung *pb = *(pecv->ppPosBuchungsliste[i]);
						csAktuelleBelegnummer = pb->Belegnummer;
						csAktuelleBelegnummer.MakeLower();
						csAktuelleBeschreibung = pb->Beschreibung;
						csAktuelleBeschreibung.MakeLower();
						if (csAktuelleBelegnummer.Find(csText) != -1 || csAktuelleBeschreibung.Find(csText) != -1)
						{
							RECT r;
							pecv->nSelected = i;
							pecv->GetWindowRect(&r);
							int vpos = i * pecv->charheight + pecv->charheight/2 - (r.bottom - r.top) / 2;
							if (vpos < 0) vpos = 0;
							pecv->SetScrollPos(SB_VERT, vpos);
							pecv->RedrawWindow();
							pecv->GetDocument()->UpdateAllViews(NULL);
							break;
						}
					}

				if (i >= MAX_BUCHUNGEN) 
				{
					((CMFCRibbonStatusBarPane*) wndMainFrame->m_wndStatusBar.FindByID(ID_STATUS))->SetAlmostLargeText("Keine weiteren Vorkommen des Suchbegriffs gefunden.");
					MessageBeep(MB_ICONASTERISK);
				}
			}			
		}
	}
	else if( wParam == IDCANCEL && lParam == 0 )
	{
		CMainFrame *wndMainFrame = (CMainFrame*)GetParent()->GetParent();
		CMDIChildWndEx *pChild = (CMDIChildWndEx *)wndMainFrame->GetActiveFrame();
		CEasyCashView *pecv = (CEasyCashView*)pChild->GetActiveView();
		if (pecv && (pecv->IsKindOf( RUNTIME_CLASS( CEasyCashView ) ) || pecv->IsKindOf( RUNTIME_CLASS( CScrollView ) )))
		{
			pecv->ShowFindToolbar(SW_HIDE);
		}
	}
	
	return CToolBar::OnCommand(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CSmartComboBox

CSmartComboBox::CSmartComboBox()
{
	m_lfHeight = -10;
	m_lfWeight = FW_NORMAL;
	m_strFaceName = _T("MS Sans Serif");
	m_nMaxStrings = 10;
}

BEGIN_MESSAGE_MAP(CSmartComboBox, CComboBox)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CSmartComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	if( !CreateFont(m_lfHeight, m_lfWeight, m_strFaceName) )
	{
		TRACE0("Failed to create font for combo box\n");
		return -1;      // fail to create
	}

	return 0;
}

BOOL CSmartComboBox::CreateFont(LONG lfHeight, LONG lfWeight, LPCTSTR lpszFaceName)
{
	//  Create a font for the combobox
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));

	if (!::GetSystemMetrics(SM_DBCSENABLED))
	{
		// Since design guide says toolbars are fixed height so is the font.
		logFont.lfHeight = lfHeight;
		logFont.lfWeight = lfWeight;
		CString strDefaultFont = lpszFaceName;
		lstrcpy(logFont.lfFaceName, strDefaultFont);
		if (!m_font.CreateFontIndirect(&logFont))
		{
			TRACE("Could Not create font for combo\n");
			return FALSE;
		}
		SetFont(&m_font);
	}
	else
	{
		m_font.Attach(::GetStockObject(SYSTEM_FONT));
		SetFont(&m_font);
	}
	return TRUE;
}

int CSmartComboBox::AddString(LPCTSTR str)
{
	if( _tcslen(str) == 0 )
		return -1;

	int oldIndex = FindStringExact(-1, str);
	if( oldIndex >= 0 )
		DeleteString(oldIndex);

	if( GetCount() == m_nMaxStrings )
		DeleteString(m_nMaxStrings-1);

	return CComboBox::InsertString(0, str);
}

int CSmartComboBox::InsertString(int index, LPCTSTR str)
{
	if( _tcslen(str) == 0 )
		return -1;

	int oldIndex = FindStringExact(-1, str);
	if( oldIndex >= 0 )
	{
		DeleteString(oldIndex);
		if( index >= oldIndex )
			--index;
	}

	if( GetCount() == m_nMaxStrings )
		DeleteString(m_nMaxStrings-1);

	return CComboBox::InsertString(index, str);
}

/////////////////////////////////////////////////////////////////////////////
