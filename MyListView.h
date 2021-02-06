#pragma once


// CMyListView view

class CMyListView : public CListView
{
	DECLARE_DYNCREATE(CMyListView)

protected:
	CMyListView();           // protected constructor used by dynamic creation
	virtual ~CMyListView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
};


