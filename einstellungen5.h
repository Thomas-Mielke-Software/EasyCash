#if !defined(AFX_EINSTELLUNGEN5_H__41E1A461_92AA_11D5_B421_0000B4336A1A__INCLUDED_)
#define AFX_EINSTELLUNGEN5_H__41E1A461_92AA_11D5_B421_0000B4336A1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Einstellungen5.h : header file
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
// CEinstellungen5 dialog

class CEinstellungen5 : public CPropertyPage
{
	DECLARE_DYNCREATE(CEinstellungen5)

// Construction
public:
	CEinstellungen5();
	~CEinstellungen5();

// Dialog Data
	//{{AFX_DATA(CEinstellungen5)
	enum { IDD = IDD_EINSTELLUNGEN5 };
	CString	m_art1;
	CString	m_art10;
	CString	m_art2;
	CString	m_art3;
	CString	m_art4;
	CString	m_art5;
	CString	m_art6;
	CString	m_art7;
	CString	m_art8;
	CString	m_art9;
	CString	m_satz1;
	CString	m_satz10;
	CString	m_satz2;
	CString	m_satz3;
	CString	m_satz4;
	CString	m_satz5;
	CString	m_satz6;
	CString	m_satz7;
	CString	m_satz8;
	CString	m_satz9;
	CString	m_ustsatz1;
	CString	m_ustsatz10;
	CString	m_ustsatz2;
	CString	m_ustsatz3;
	CString	m_ustsatz4;
	CString	m_ustsatz5;
	CString	m_ustsatz6;
	CString	m_ustsatz7;
	CString	m_ustsatz8;
	CString	m_ustsatz9;
	//}}AFX_DATA

	// pointer arrays für besseren Zugriff
	CString *m_part[10];
	CString	*m_psatz[10];
	CString	*m_pustsatz[10];
	int		m_privat_split_size;

	CEinstellungen1 *m_einstellungen1;

	void UpdateCombo(int ctrlid);

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEinstellungen5)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEinstellungen5)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EINSTELLUNGEN5_H__41E1A461_92AA_11D5_B421_0000B4336A1A__INCLUDED_)
