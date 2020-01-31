// DauerbuchungenDlg.h : header file
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
// DauerbuchungenDlg dialog

class DauerbuchungenDlg : public CDialog
{
// Construction
public:
	DauerbuchungenDlg(CEasyCashDoc *pDoc, BOOL ausgaben, CEasyCashView* pParent /*=NULL*/);

// Dialog Data
	//{{AFX_DATA(DauerbuchungenDlg)
	enum { IDD = IDD_DAUERBUCHUNGEN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DauerbuchungenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CEasyCashDoc *m_pDoc;
	CEasyCashView *m_pParent;
	BOOL m_ausgaben;
	CDauerbuchung *aktDauerbuchung;
	BOOL newFlag;


	void EnableCtrls(BOOL b=TRUE);
	void InitCtrls();
	void UpdateCombo(CString ea);
	void UpdateBeschreibungCombo(CString ea);

	// Generated message map functions
	//{{AFX_MSG(DauerbuchungenDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAendern();
	afx_msg void OnNeu();
	afx_msg void OnLoeschen();
	afx_msg void OnClose();
	afx_msg void OnVerwerfen();
	afx_msg void OnDblclkList();
	afx_msg void OnKillfocusDatumBisJahr();
	afx_msg void OnKillfocusDatumVonJahr();
	afx_msg void OnEinnahmen();
	afx_msg void OnAusgaben();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelchangeBeschreibung();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
