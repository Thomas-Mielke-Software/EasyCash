#if !defined(AFX_VORANMELDUNGSZEITRAUMDROPLIST_H__1431931F_3172_4C01_AE0E_8590C4229D08__INCLUDED_)
#define AFX_VORANMELDUNGSZEITRAUMDROPLIST_H__1431931F_3172_4C01_AE0E_8590C4229D08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VoranmeldungszeitraumDroplist.h : header file
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

/////////////////////////////////////////////////////////////////////////////
// CVoranmeldungszeitraumDroplist dialog

class CVoranmeldungszeitraumDroplist : public CDialog
{
// Construction
public:
	CVoranmeldungszeitraumDroplist(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVoranmeldungszeitraumDroplist)
	enum { IDD = IDD_VORANMELDUNGSZEITRAUM_DROPLIST };
	CComboBox	m_combo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVoranmeldungszeitraumDroplist)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	CEasyCashDoc *m_pDoc;
	int m_monatliche_voranmeldung;	// 0 = monatlich, 1 = quartalsweise

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVoranmeldungszeitraumDroplist)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VORANMELDUNGSZEITRAUMDROPLIST_H__1431931F_3172_4C01_AE0E_8590C4229D08__INCLUDED_)
