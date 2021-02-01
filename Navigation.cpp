// Navigation.cpp : implementation file
//

#include "stdafx.h"
#include "EasyCash.h"
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "Navigation.h"


// CNavigation

IMPLEMENT_DYNCREATE(CNavigation, CListCtrl)

CNavigation::CNavigation()
{
	m_pViewWnd = NULL;
}

CNavigation::~CNavigation()
{
}


BEGIN_MESSAGE_MAP(CNavigation, CListCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CNavigation message handlers



int CNavigation::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	lpCreateStruct->style |= WS_CHILD | WS_VISIBLE | LVS_REPORT;

	return 0;
}
