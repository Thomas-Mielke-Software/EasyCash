#if !defined(AFX_BUCHUNGPPG_H__133232D5_E4E9_411E_8573_080A5E907974__INCLUDED_)
#define AFX_BUCHUNGPPG_H__133232D5_E4E9_411E_8573_080A5E907974__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// BuchungPpg.h : Declaration of the CBuchungPropPage property page class.
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

////////////////////////////////////////////////////////////////////////////
// CBuchungPropPage : See BuchungPpg.cpp.cpp for implementation.

class CBuchungPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CBuchungPropPage)
	DECLARE_OLECREATE_EX(CBuchungPropPage)

// Constructor
public:
	CBuchungPropPage();

// Dialog Data
	//{{AFX_DATA(CBuchungPropPage)
	enum { IDD = IDD_PROPPAGE_BUCHUNG };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CBuchungPropPage)
	afx_msg void OnWww();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUCHUNGPPG_H__133232D5_E4E9_411E_8573_080A5E907974__INCLUDED)
