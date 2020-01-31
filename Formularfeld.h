#include "afxwin.h"
#if !defined(AFX_FORMULARFELD_H__0F95163E_3CE6_430E_875F_990683F8BD8C__INCLUDED_)
#define AFX_FORMULARFELD_H__0F95163E_3CE6_430E_875F_990683F8BD8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Formularfeld.h : header file
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
// CFormularfeld dialog

class CEasyCashView;

class CFormularfeld : public CDialog
{
// Construction
public:
	BOOL m_bKeineFeldUeberschreibenMeldung;
	CFormularfeld(CEasyCashView* pParent);   // standard constructor
	CString m_csFormulardatei;

// Dialog Data
	//{{AFX_DATA(CFormularfeld)
	enum { IDD = IDD_FORMULARFELD };
	CComboBox	m_typ;
	CString	m_langform;
	CString	m_name;
	int		m_ausrichtung;
	int		m_seite;
	int		m_horizontal;
	int		m_vertikal;
	int		m_id;
	int		m_anteil;
	int		m_nachkommaanteil;
	int		m_nNullwertdarstellung;
	int		m_veraltet;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormularfeld)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CEasyCashView *m_pParent;

	// Generated message map functions
	//{{AFX_MSG(CFormularfeld)
	virtual BOOL OnInitDialog();
	afx_msg void OnSpeichern();
	afx_msg void OnDeltaposHorizontalSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposVertikalSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAusrichtung();
	afx_msg void OnDeltaposSeiteSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeSeite();
	afx_msg void OnChangeVertikal();
	afx_msg void OnChangeHorizontal();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeTyp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void UpdateFeldmarke();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMULARFELD_H__0F95163E_3CE6_430E_875F_990683F8BD8C__INCLUDED_)
