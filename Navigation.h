#pragma once

// CNavigation

class CNavigation : public CListCtrl
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
};


