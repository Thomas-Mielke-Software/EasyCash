#if !defined(TIPDLG_H_INCLUDED_)
#define TIPDLG_H_INCLUDED_

// CG: This file added by 'Tip of the Day' component.

/////////////////////////////////////////////////////////////////////////////
// CTipDlg dialog
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

class CTipDlg : public CDialog
{
// Construction
public:
	CTipDlg(CWnd* pParent = NULL);	 // standard constructor

// Dialog Data
	//{{AFX_DATA(CTipDlg)
	// enum { IDD = IDD_TIP };
	BOOL	m_bStartup;
	CString	m_strTip;
	//}}AFX_DATA

	FILE* m_pStream;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTipDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTipDlg();

protected:
	// Generated message map functions
	//{{AFX_MSG(CTipDlg)
	afx_msg void OnNextTip();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void GetNextTipString(CString& strNext);
};

#endif // !defined(TIPDLG_H_INCLUDED_)
