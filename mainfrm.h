// MainFrm.h : interface of the CMainFrame class
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
//
/////////////////////////////////////////////////////////////////////////////

#include "WebUpdate.h"
#include "ToolBarEx.h"

class CPluginElement : public CObject
{
public:
	CPluginElement();
	~CPluginElement();
	CString name;
	CString beschreibung;
	CString aufruf;
	CBitmap *bitmap;
	HICON icon;
	int		id;		// command id
	CString typ;
	CPluginElement *next;
};

class CMFCRibbonButtonEx : public CMFCRibbonButton
{
// Construction
public:
	CMFCRibbonButtonEx();
	CMFCRibbonButtonEx(UINT nID, LPCTSTR lpszText, int nSmallImageIndex = -1, int nLargeImageIndex = -1, BOOL bAlwaysShowDescription = FALSE);
	CMFCRibbonButtonEx(UINT nID, LPCTSTR lpszText, HICON hIcon, BOOL bAlwaysShowDescription = FALSE, HICON hIconSmall = NULL, BOOL bAutoDestroyIcon = FALSE, BOOL bAlphaBlendIcon = FALSE);

// Overridden
	void SetCheck(BOOL bCheck = TRUE);
	void DrawImage(CDC* pDC, RibbonImageType type, CRect rectImage);

// Attributes
private:
	BOOL m_bChecked;

// Helper
private:
	void DrawCheckmark(CDC* pDC, int CheckmarkResourceBitmapID, RECT *r);
	void PremultiplyBitmapAlpha(HDC hDC, HBITMAP hBmp);
};

#define ID_CMD_PLUGIN_BASE 38000	// hier fangen die Comand-IDs der Plugin-Knöpfe an

class CMainFrame : public CMDIFrameWndEx
{
	friend class CEasyCashView;

	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:
	void ShowPlugins(char *szName);
	void ShowPluginsRibbonMenu(CMFCRibbonButton *pAnsichtPluginsButton, char *szName = NULL, int nIndex = 0);
	void DockControlBarLeftOf(CToolBar* Bar, CToolBar* LeftOf);
	void LoadBitmap(CBitmap **ppcbm, char* filename);
	void AttachToolbarImages (UINT inNormalImageID, UINT inDisabledImageID, UINT inHotImageID);
	afx_msg void OnInitMenu(CMenu* pMenu);
    void UpdateSaldo();
    void UpdateSaldo(CEasyCashView *pecv);
	void OnLink(UINT nID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
	inline CMFCCaptionBar *GetCaptionBar() { return &m_wndCaptionBar; };
	BOOL CreateCaptionBar();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void Add_MainPanel ();
	void Add_QAT ();
	void Add_Category1();
	void Add_Category2();


	CPluginElement *m_pPlugins;
	int				m_nPluginCount;
	BOOL			m_bInformationModeElapsed;

	//CStatusBar  m_wndStatusBar;	VS9
	void SetStatus(LPCTSTR status);
	CMFCRibbonStatusBar	m_wndStatusBar;

	CMFCRibbonButton* m_pInfoButton;
	CMFCRibbonButton* m_pFilterKontoButton;
	CMFCRibbonButton* m_pFilterBetriebButton;
	CMFCRibbonButton* m_pFilterBestandskontoButton;
	CMFCRibbonComboBox* m_pSucheCombobox;
	CMFCRibbonButton* m_pAnsichtFormulareButton;
	CMFCRibbonButton* m_pAnsichtPluginsButton;

protected:  // control bar embedded members
	CMFCRibbonBar				m_wndRibbonBar;
	CMFCRibbonApplicationButton	m_MainButton;
	CMFCToolBarImages			m_PanelImages16;
	CMFCToolBarImages			m_PanelImages32;

	CToolBar			m_wndToolBar;
	CToolBarWithCombo   m_wndFindToolBar;		// Find Toolbar
	CToolBar			m_wndPluginToolBar; 	// Plugin Toolbar
	CMFCCaptionBar		m_wndCaptionBar;

	CImageList m_ToolbarImages;
	CImageList m_ToolbarImagesDisabled;
	CImageList m_ToolbarImagesHot;


// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnAppLook(UINT id);
	afx_msg void OnUpdateAppLook(CCmdUI* pCmdUI);
	//afx_msg void OnViewCaptionBar();
	//afx_msg void OnUpdateViewCaptionBar(CCmdUI* pCmdUI);
	afx_msg void OnFilePluginmanager();
	afx_msg void OnFileOeffneDatenverzeichnis();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFileMandanten();
	afx_msg void OnFileWaehleDatenverzeichnis();
	afx_msg void OnFileRegistrierungsinformationenWiederherstellen();
	afx_msg void OnFileRegistrierungsinformationenSichern();
	afx_msg void OnSpendeBank();
	afx_msg void OnSpendePayPal();
	afx_msg void OnSpendeBitcoin();
	afx_msg void OnSpendeFlattr();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL CreateRibbonBar ();
//	void ShowOptions (int nPage);

	UINT	m_nAppLook;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

/////////////////////////////////////////////////////////////////////////////
