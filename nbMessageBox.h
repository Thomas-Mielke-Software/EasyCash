#if !defined(AFX_NBMESSAGEBOX_H__D9FAFBD4_744B_45DB_9CA6_77DDB6F6C49D__INCLUDED_)
#define AFX_NBMESSAGEBOX_H__D9FAFBD4_744B_45DB_9CA6_77DDB6F6C49D__INCLUDED_

// nbMessageBox.cpp : header file
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Separator.h"

class CnbMessageBox : public CWnd
{
// Construction
public:
	CnbMessageBox(CWnd *pParent = NULL);

// Attributes
public:

// Operations
public:
    int MessageBox(LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0 );
    int MessageBox( UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT) -1 );

    void SetChecked(BOOL bChecked);
    BOOL GetChecked();

    static void SetDoNotAgainStrings(CString &strDoNotShowAgain, CString &strDoNotAskAgain);
    static void SetDoNotAgainStrings(int nDoNotShowAgainId, int nDoNotAskAgainId);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CnbMessageBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CnbMessageBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CnbMessageBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    LRESULT OnSubclassedInit(WPARAM wParam, LPARAM lParam);
    afx_msg void OnDoNotAgain();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CWnd *m_pParent;
    CButton m_cbDoNotAgain;
    //CSeparator m_ctrlSep;

    BOOL m_bChecked;

    int m_nType;

    static CString m_strDoNotShowAgain;
    static CString m_strDoNotAskAgain;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NBMESSAGEBOX_H__D9FAFBD4_744B_45DB_9CA6_77DDB6F6C49D__INCLUDED_)
