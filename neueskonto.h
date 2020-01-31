// NeuesKonto.h : header file
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
// CNeuesKonto dialog

class CNeuesKonto : public CDialog
{
// Construction
public:
	CNeuesKonto(CWnd* pParent = NULL);   // standard constructor

	BOOL m_aendern;
	CString  m_formulardatei;
	int	m_feldID;

// Dialog Data
	//{{AFX_DATA(CNeuesKonto)
	enum { IDD = IDD_NEUES_KONTO };
	CComboBox	m_feldzuweisung;
	CString	m_konto;
	CString	m_formularname;
	CString	m_kategorie;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeuesKonto)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNeuesKonto)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnUebertragen();
	afx_msg void OnFeldzuweisungLoeschen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
