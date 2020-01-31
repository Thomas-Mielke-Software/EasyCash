#if !defined(AFX_VOTEDLG_H__F8A29C94_FE3E_4234_A8ED_AC40D651C0FA__INCLUDED_)
#define AFX_VOTEDLG_H__F8A29C94_FE3E_4234_A8ED_AC40D651C0FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VoteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVoteDlg dialog

class CVoteDlg : public CDialog
{
// Construction
public:
	CVoteDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVoteDlg)
	enum { IDD = IDD_VOTE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVoteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CVoteDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkListe();
	afx_msg void OnSelchangeListe();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOTEDLG_H__F8A29C94_FE3E_4234_A8ED_AC40D651C0FA__INCLUDED_)
