// EasyCashFormularDupliziererDlg.h : header file
//

#if !defined(AFX_EASYCASHFORMULARDUPLIZIERERDLG_H__DA6A3D67_7E53_48BD_9A39_0329DC81BF54__INCLUDED_)
#define AFX_EASYCASHFORMULARDUPLIZIERERDLG_H__DA6A3D67_7E53_48BD_9A39_0329DC81BF54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CEasyCashFormularDupliziererDlg dialog

class CEasyCashFormularDupliziererDlg : public CDialog
{
// Construction
public:
	CEasyCashFormularDupliziererDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CEasyCashFormularDupliziererDlg)
	enum { IDD = IDD_EASYCASHFORMULARDUPLIZIERER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasyCashFormularDupliziererDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEasyCashFormularDupliziererDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EASYCASHFORMULARDUPLIZIERERDLG_H__DA6A3D67_7E53_48BD_9A39_0329DC81BF54__INCLUDED_)
