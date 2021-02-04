#pragma once

#include "MyListView.h"
// CNavigation

class CNavigation : public CMyListView
{
	DECLARE_DYNCREATE(CNavigation)

public:
	CNavigation();
	virtual ~CNavigation();
	CEasyCashView *m_pViewWnd;

private:
	CSplitterWnd m_wndSplitter;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};


