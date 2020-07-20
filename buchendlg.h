// BuchenDlg.h : header file
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
// BuchenDlg dialog

class CEasyCashView;

class BuchenDlg : public CDialog
{
// Construction
public:
	BuchenDlg(CEasyCashDoc *pDoc, BOOL ausgaben, 
		CEasyCashView* pParent = NULL, CBuchung **ppb = NULL, BOOL bKopieren = FALSE, BOOL bNeueBelegnummer = FALSE);   // standard constructor

	CBitmap Button1Bmp;
	CBitmap StopBmp;
	CBitmap SplitBmp;
	CBitmap RechnerBmp;

	CMenu PopUp;

	void PrivatSplit(int n);

// Dialog Data
	//{{AFX_DATA(BuchenDlg)
	enum { IDD = IDD_BUCHUNG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BuchenDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CEasyCashDoc *m_pDoc;
	CEasyCashView *m_pParent;
	BOOL m_ausgaben;
	CBuchung **m_ppb;
	BOOL m_bKopieren;
	BOOL m_bNeueBelegnummer;
	BOOL m_UpdateBeschreibung;
	CString m_csZielwaehrung;
	int m_nGewaehlterSplit;
	CImageList m_imgListIcons;
	CImageList m_imgListIconsBestandskonten;
	BOOL m_bUnterkategorien;

	void InitDlg(BOOL bBelasseEinigeFelder = FALSE);
	void InitRestwert();
	void UpdateCombo(CString ea);
	void OnAlt(int nCtrl, int nItem);

	// Generated message map functions
	//{{AFX_MSG(BuchenDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnAusgaben();
	afx_msg void OnEinnahmen();
	afx_msg void OnClose();
	virtual void OnCancel();
	afx_msg void OnKillfocusDatumJahr();
	afx_msg void OnSetfocusBeschreibung();
	afx_msg void OnSelchangeBeschreibung();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditchangeBeschreibung();
	afx_msg void OnSelchangeAbschreibungjahre();
	afx_msg void OnDestroy();
	afx_msg void OnJahrEnabled();
	afx_msg void OnChangeDatumTag();
	afx_msg void OnChangeDatumJahr();
	afx_msg void OnChangeDatumMonat();
	afx_msg void OnSetfocusDatumTag();
	afx_msg void OnSetfocusDatumJahr();
	afx_msg void OnSetfocusDatumMonat();
	afx_msg void OnChangeBetrag();
	afx_msg void OnKonvertieren();
	afx_msg void OnSplit();
	afx_msg void OnChangeBelegnummer();
	afx_msg void OnRechner();
	afx_msg void OnAlt1();
	afx_msg void OnAlt2();
	afx_msg void OnAlt3();
	afx_msg void OnAlt4();
	afx_msg void OnAlt5();
	afx_msg void OnAlt6();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeAbschreibungnummer();
	afx_msg void OnCbnSelchangeMwst();
	afx_msg void OnCbnEditchangeMwst();
	afx_msg void OnCbnEditchangeAbschreibungjahre();
	afx_msg void OnCbnEditchangeAbschreibungnummer();
	afx_msg void OnBnClickedMwstEnabled();
	afx_msg void OnCbnSelchangeEurechnungsposten();
};
