// Einstellungen1.h : header file
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
// CEinstellungen1 dialog

class CBuchungsposten : CObject
{
public:
	CString Beschreibung;
	BOOL    Ausgaben;
	int     MWSt;
	int     Abschreibungsjahre;
	CString Rechnungsposten;
};

class CEinstellungen1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CEinstellungen1)

// Construction
public:
	CEinstellungen1();
	~CEinstellungen1();

	// Daten
	CBuchungsposten Buchungsposten[100];
	CString EinnahmenRechnungsposten[100];
	CString EinnahmenFeldzuweisungen[100];
	CString AusgabenRechnungsposten[100];
	CString AusgabenFeldzuweisungen[100];

// Dialog Data
	//{{AFX_DATA(CEinstellungen1)
	enum { IDD = IDD_EINSTELLUNGEN1 };
	int		m_monatliche_voranmeldung;
	BOOL	m_taeglich_buchen;
	BOOL	m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen;
	BOOL	m_bErzeugeLaufendeBuchungsnummernFuerAusgaben;
	int		m_lfd_Buchungsnummer_Ausgaben;
	int		m_lfd_Buchungsnummer_Einnahmen;
	BOOL	m_BuchungsdatumBelassen;
	int		m_buchungsjahr;
	int		m_lfd_Buchungsnummer_Bank;
	int		m_lfd_Buchungsnummer_Kasse;
	CString	m_sondervorauszahlung;
	BOOL	m_bErzeugeLaufendeBuchungsnummernFuerBank;
	BOOL	m_bErzeugeLaufendeBuchungsnummernFuerKasse;
	//}}AFX_DATA

	BOOL	m_bJahresfeldAktiviert;
	BOOL	m_bMwstFeldAktiviert;
	CString m_Bildschirmschrift;
	int		m_Bildschirmschriftgroesse;
	int		m_BildschirmschriftFett;
	BOOL	m_BildschirmschriftKursiv;
	CString m_Druckerschrift;
	int		m_Druckerschriftgroesse;
	int		m_DruckerschriftFett;
	BOOL	m_DruckerschriftKursiv;
	int		m_AbschreibungGenauigkeit;
	CDocument *m_pDoc;
	CView *m_pView;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEinstellungen1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEinstellungen1)
	afx_msg void OnSelchangeListe();
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusEa();
	afx_msg void OnKillfocusBuchungspostentext();
	afx_msg void OnKillfocusMwst();
	afx_msg void OnKillfocusAfa();
	afx_msg void OnSelchangeEurechnungsposten();
	afx_msg void OnBildschirmschrift();
	afx_msg void OnDruckerschrift();
	afx_msg void OnAfaGenauigkeit();
	afx_msg void OnKillfocusSondervorauszahlung();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void UpdateListe();
	void UpdateCombo(CString ea);
};
