// MyListView.cpp : implementation file
//

#include "stdafx.h"
#include "EasyCash.h"
#include "MyListView.h"


// CMyListView

IMPLEMENT_DYNCREATE(CMyListView, CListView)

CMyListView::CMyListView()
{

}

CMyListView::~CMyListView()
{
}

BEGIN_MESSAGE_MAP(CMyListView, CListView)
END_MESSAGE_MAP()


// CMyListView diagnostics

#ifdef _DEBUG
void CMyListView::AssertValid() const
{
	CListView::AssertValid();
}

#ifndef _WIN32_WCE
void CMyListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif
#endif //_DEBUG


// CMyListView message handlers
