#if !defined(AFX_RECHNDLG_H__440CDE73_7CCC_403D_89A5_62DFE44EBF24__INCLUDED_)
#define AFX_RECHNDLG_H__440CDE73_7CCC_403D_89A5_62DFE44EBF24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RechnDlg.h : header file
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

#include "VCalcParser.h"

/////////////////////////////////////////////////////////////////////////////
// CRechnDlg dialog

class CRechnDlg : public CDialog
{
// Construction
public:
	CRechnDlg(CWnd* pParent = NULL);   // standard constructor
	double m_betrag;
	double m_mwst;
	double m_betrag_neu;

// Dialog Data
	//{{AFX_DATA(CRechnDlg)
	enum { IDD = IDD_RECHN };
	CButton	m_f10;
	CButton	m_f9;
	CButton	m_f8;
	CButton	m_f7;
	CButton	m_f6;
	CButton	m_f5;
	CButton	m_f4;
	CButton	m_f3;
	CButton	m_f2;
	CButton	m_f1;
	CButton	m_uebernehmen;
	CButton	m_berechnen;
	CEdit	m_eingabe;
	CListBox	m_variablen;
	CListBox	m_ergebnisse;
	CListBox	m_funktionen;
	CStatic	m_ergebnis;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRechnDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CVCalcParser m_Parser;
	void UpdateAnswersList();
	void UpdateVariablesList();
	void UpdateFunctionsList();

	void OnFormel(int n, CButton *pbutton);

	// Generated message map functions
	//{{AFX_MSG(CRechnDlg)
	afx_msg void OnDblclkErgebnisseLbox();
	afx_msg void OnDblclkFunktionenLbox();
	afx_msg void OnDblclkVariablenLbox();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBerechnen();
	afx_msg void OnUebernehmen();
	afx_msg void OnFormel1();
	afx_msg void OnFormel2();
	afx_msg void OnFormel3();
	afx_msg void OnFormel4();
	afx_msg void OnFormel5();
	afx_msg void OnFormel6();
	afx_msg void OnFormel7();
	afx_msg void OnFormel8();
	afx_msg void OnFormel9();
	afx_msg void OnFormel10();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECHNDLG_H__440CDE73_7CCC_403D_89A5_62DFE44EBF24__INCLUDED_)
