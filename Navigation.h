#pragma once

#include "MyListView.h"
// CNavigation

class CNavigation : public CMyListView
{
protected: // create from serialization only
	CNavigation();
	DECLARE_DYNCREATE(CNavigation)

// Attributes
public:
	CEasyCashDoc* GetDocument() const;
protected:
	CSplitterWnd m_wndSplitter;

// Operations
public:

// Overrides
public:

// Implementation
	virtual ~CNavigation();
	CEasyCashView *m_pViewWnd;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};


