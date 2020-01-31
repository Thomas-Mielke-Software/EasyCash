// DauBuchAusfuehren.h : header file
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
// DauBuchAusfuehren dialog

class DauBuchAusfuehren : public CDialog
{
// Construction
public:
	DauBuchAusfuehren(CEasyCashDoc *pDoc, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DauBuchAusfuehren)
	enum { IDD = IDD_DAUERBUCHUNGEN_AUSFUEHREN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	int m_jb, m_mb;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DauBuchAusfuehren)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CEasyCashDoc *m_pDoc;
	CWnd* m_pParent;
	CBitmap Button1Bmp;
	CBitmap StopBmp;

	// Generated message map functions
	//{{AFX_MSG(DauBuchAusfuehren)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
