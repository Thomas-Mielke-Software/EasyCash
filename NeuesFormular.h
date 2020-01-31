#if !defined(AFX_NEUESFORMULAR_H__74A2E4E5_BBDB_4FB2_8F85_F276E73F594B__INCLUDED_)
#define AFX_NEUESFORMULAR_H__74A2E4E5_BBDB_4FB2_8F85_F276E73F594B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NeuesFormular.h : header file
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
// CNeuesFormular dialog

class CNeuesFormular : public CDialog
{
// Construction
public:
	CNeuesFormular(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNeuesFormular)
	enum { IDD = IDD_NEUES_FORMULAR };
	CString	m_name;
	UINT	m_seiten;
	CString	m_dateiname;
	CString	m_grafikdateien;
	CString	m_anzeigename;
	CString	m_schriftart;
	UINT	m_schriftgroesse;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeuesFormular)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNeuesFormular)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEUESFORMULAR_H__74A2E4E5_BBDB_4FB2_8F85_F276E73F594B__INCLUDED_)
