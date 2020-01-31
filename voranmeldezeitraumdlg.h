// VoranmeldezeitraumDlg.h : header file
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
// VoranmeldezeitraumDlg dialog

class VoranmeldezeitraumDlg : public CDialog
{
// Construction
public:
	VoranmeldezeitraumDlg(CEasyCashView* pParent);   // standard constructor
	CEasyCashView *m_pParent;
	CTime m_von_datum;
	CTime m_bis_datum;
	BOOL  m_bVorauszahlungssollMerken;

// Dialog Data
	//{{AFX_DATA(VoranmeldezeitraumDlg)
	enum { IDD = IDD_VORANMELDUNGSZEITRAUM };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VoranmeldezeitraumDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBitmap Button1Bmp;
	CBitmap StopBmp;

	// Generated message map functions
	//{{AFX_MSG(VoranmeldezeitraumDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
