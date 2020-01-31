#if !defined(AFX_QUICKEDITCELL_H__2EB671B5_0711_11D3_90AB_00E029355177__INCLUDED_)
#define AFX_QUICKEDITCELL_H__2EB671B5_0711_11D3_90AB_00E029355177__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditCell.h : header file
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

#include "quicklist.h"
class CQuickList;

/*
	The code in this class is very much based on Lee Nowotny's article 
	"Easy Navigation Through an Editable List View",
	http://www.codeproject.com/listctrl/listeditor.asp

	And that article is based on another article by Zafir Anjum, "Editable Subitems":
	http://www.codeguru.com/listview/edit_subitems.shtml

    However, currently most of it is now rewritten. But I thought it 
	would be unfair to not mention it.
*/

/////////////////////////////////////////////////////////////////////////////
// CQuickEdit window
class CQuickEdit : public CEdit
{
public:
	CQuickEdit (CQuickList* pCtrl, int iItem, int iSubItem, CString sInitText, bool endonlostfocus);
    virtual ~CQuickEdit();

	void SetEndOnLostFocus(bool autoend);
	void StopEdit(bool cancel, UINT endkey=0);

    //{{AFX_VIRTUAL(CQuickEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
    CQuickList* pListCtrl;
    int			Item;
    int			SubItem;
    CString		InitText;
	
	bool		m_endOnLostFocus;
	bool		m_isClosing;
    
    //{{AFX_MSG(CQuickEdit)
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUICKEDITCELL_H__2EB671B5_0711_11D3_90AB_00E029355177__INCLUDED_)
