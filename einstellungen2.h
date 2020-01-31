// Einstellungen2.h : header file
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
// CEinstellungen2 dialog

class CEinstellungen2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CEinstellungen2)

// Construction
public:
	CEinstellungen2();
	~CEinstellungen2();

// Dialog Data
	//{{AFX_DATA(CEinstellungen2)
	enum { IDD = IDD_EINSTELLUNGEN2 };
	CString	m_name;
	CString	m_ort;
	CString	m_plz;
	CString	m_strasse;
	CString	m_unternehmensart1;
	CString	m_unternehmensart2;
	CString	m_vorname;
	int		m_land;
	CString	m_vat1;
	CString	m_vat2;
	CString	m_vat3;
	CString m_vat4;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEinstellungen2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEinstellungen2)
	afx_msg void OnLand1();
	afx_msg void OnLand2();
	afx_msg void OnLand3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
};
