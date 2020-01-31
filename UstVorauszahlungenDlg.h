#if !defined(AFX_USTVORAUSZAHLUNGENDLG_H__603E602F_DF02_41AC_B4C3_F083717740E9__INCLUDED_)
#define AFX_USTVORAUSZAHLUNGENDLG_H__603E602F_DF02_41AC_B4C3_F083717740E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UstVorauszahlungenDlg.h : header file
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
// CUstVorauszahlungenDlg dialog

class CUstVorauszahlungenDlg : public CDialog
{
// Construction
public:
	CUstVorauszahlungenDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUstVorauszahlungenDlg)
	enum { IDD = IDD_USTVORAUSZAHLUNGEN };
	CString	m_m1;
	CString	m_m2;
	CString	m_m3;
	CString	m_m4;
	CString	m_m5;
	CString	m_m6;
	CString	m_m7;
	CString	m_m8;
	CString	m_m9;
	CString	m_m10;
	CString	m_m11;
	CString	m_m12;
	CString	m_q1;
	CString	m_q2;
	CString	m_q3;
	CString	m_q4;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUstVorauszahlungenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUstVorauszahlungenDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CString m_vorauszahlung;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USTVORAUSZAHLUNGENDLG_H__603E602F_DF02_41AC_B4C3_F083717740E9__INCLUDED_)
