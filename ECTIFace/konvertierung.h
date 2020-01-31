#if !defined(AFX_KONVERTIERUNG_H__B3AB0D81_EEDA_11D4_B41F_0000B4336A1A__INCLUDED_)
#define AFX_KONVERTIERUNG_H__B3AB0D81_EEDA_11D4_B41F_0000B4336A1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Konvertierung.h : header file
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
// CKonvertierung dialog

class CKonvertierung : public CDialog
{
// Construction
public:
	CKonvertierung(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKonvertierung)
	enum { IDD = IDD_KONVERTIERUNG };
	int		m_waehrung_auswahl;
	CString	m_waehrungskuerzel;
	int		m_konvertieren;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKonvertierung)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKonvertierung)
	afx_msg void OnWaehrungAuswahl();
	afx_msg void OnWaehrungAuswahl2();
	afx_msg void OnWaehrungAuswahl3();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KONVERTIERUNG_H__B3AB0D81_EEDA_11D4_B41F_0000B4336A1A__INCLUDED_)
