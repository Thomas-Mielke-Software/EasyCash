#if !defined(AFX_PLUGINMANAGER_H__D26DDF08_4CD5_47C2_A17E_19A2D851210A__INCLUDED_)
#define AFX_PLUGINMANAGER_H__D26DDF08_4CD5_47C2_A17E_19A2D851210A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PluginManager.h : header file
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

#include "QuickList.h"
#include "WebUpdate.h"
#include "afxlinkctrl.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CPluginManager dialog

class CPluginManager : public CDialog
{
// Construction
public:
	CPluginManager(CMainFrame* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPluginManager)
	enum { IDD = IDD_PLUGIN_MANAGER };
	CQuickList m_liste;
	CString	m_status;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	afx_msg LRESULT OnGetListItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListClick(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnNavigationTest(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnHeaderRightClick(WPARAM wParam, LPARAM lParam);

	CMainFrame *m_pParent;
	CWebUpdate m_update;
	CImageList m_imgList;
	CBitmap *m_pbmp;
	CStringArray m_csaCheckedAktuell;
	CStringArray m_csaCheckedOriginal;

#ifdef USEXPTHEMES
	CTheme m_themeManager;
#endif

	void Download(BOOL bForce);
	void UpdateList();
	void Statusmeldung(CString &text);
	void Check();

	// Generated message map functions
	//{{AFX_MSG(CPluginManager)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClickListe(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedReparaturinstallation();
	CToolTipCtrl m_ToolTip;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CMFCLinkCtrl m_btnManuellerDownload;
	CButton m_ButtonReparaturinstallation;
	CButton m_ButtonDownload;
	CButton m_ButtonSchliessen;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLUGINMANAGER_H__D26DDF08_4CD5_47C2_A17E_19A2D851210A__INCLUDED_)
