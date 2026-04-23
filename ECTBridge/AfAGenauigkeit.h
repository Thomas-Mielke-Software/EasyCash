#if !defined(AFX_AFAGENAUIGKEIT_H__A6718E31_7FCE_492F_BBD7_9C84A1126B14__INCLUDED_)
#define AFX_AFAGENAUIGKEIT_H__A6718E31_7FCE_492F_BBD7_9C84A1126B14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AfAGenauigkeit.h : header file
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

#include "resource.h"

#if !defined(AFX_EXT_CLASS)
#define AFX_EXT_CLASS __declspec(dllimport)
#endif // kann passieren...

/////////////////////////////////////////////////////////////////////////////
// CAfAGenauigkeit dialog

class AFX_EXT_CLASS CAfAGenauigkeit : public CDialog
{
// Construction
public:
	CAfAGenauigkeit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAfAGenauigkeit)
	enum { IDD = IDD_AFA_GENAUIGKEIT };
	int		m_afa_genauigkeit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAfAGenauigkeit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAfAGenauigkeit)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AFAGENAUIGKEIT_H__A6718E31_7FCE_492F_BBD7_9C84A1126B14__INCLUDED_)
