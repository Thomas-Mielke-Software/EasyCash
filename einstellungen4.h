// Einstellungen4.h : header file
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
// CEinstellungen4 dialog

#include "QuickList.h"
#include "ECTIFace\XMLite.h"
#include "afxwin.h"

class CEinstellungen4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CEinstellungen4)

// Construction
public:
	CEinstellungen4();
	~CEinstellungen4();

// Dialog Data
	//{{AFX_DATA(CEinstellungen4)
	enum { IDD = IDD_EINSTELLUNGEN4 };
	CComboBox	m_formular;
	CQuickList	m_liste;
	BOOL	m_ustvst_gesondert;
	BOOL	m_nach_konten_seitenumbruch;
	int		m_kontenkategorie;
	//}}AFX_DATA
	CString m_finanzamtserstattungen;
	CString m_finanzamtszahlungen;

	CEinstellungen1 *m_einstellungen1;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEinstellungen4)
	public:
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEinstellungen4)
	virtual BOOL OnInitDialog();
	afx_msg void OnPlus();
	afx_msg void OnMinus();
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnDestroy();
	afx_msg void OnAendern();
	afx_msg void OnSelchangeKontenkategorie();
	afx_msg void OnDblclkListe(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeFormular();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnGetListItem(WPARAM wParam, LPARAM lParam);

#ifdef USEXPTHEMES
	CTheme m_themeManager;
#endif

	CBitmap PlusBmp;
	CBitmap MinusBmp;
	CBitmap UpBmp;
	CBitmap DownBmp;
	CBitmap AendernBmp;
	BOOL m_ButtonsNotYetSet;
	CStringArray m_csaFormulare;
	XDoc *m_aktFormular;

	void UpdateList();
public:
	CComboBox m_ctrlFinanzamtszahlungen1;
	CComboBox m_ctrlFinanzamtszahlungen2;
	afx_msg void OnCbnSelchangeFinanzamtszahlungen1();
	afx_msg void OnCbnSelchangeFinanzamtszahlungen2();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
