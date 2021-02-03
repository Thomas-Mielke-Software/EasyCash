// Navigation.cpp : implementation file
//

#include "stdafx.h"
#include "EasyCash.h"
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "Navigation.h"


// CNavigation

IMPLEMENT_DYNCREATE(CNavigation, CMyListView)

CNavigation::CNavigation()
{
	m_pViewWnd = NULL;
}

CNavigation::~CNavigation()
{
}


BEGIN_MESSAGE_MAP(CNavigation, CMyListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CNavigation message handlers



int CNavigation::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	lpCreateStruct->style |= WS_CHILD | WS_VISIBLE | LVS_REPORT;

	return 0;
}

void CNavigation::OnSize(UINT nType, int cx, int cy)
{
	CMyListView::OnSize(nType, cx, cy);

	CRect clir;
	GetClientRect(&clir);
	CListCtrl &nav = GetListCtrl();
	nav.SetColumnWidth(0, clir.Width());
	nav.EnableGroupView(TRUE);
}
