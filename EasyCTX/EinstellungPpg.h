#if !defined(AFX_EINSTELLUNGPPG_H__7C6818DF_D09C_42EF_A18B_DD95A51AD0B0__INCLUDED_)
#define AFX_EINSTELLUNGPPG_H__7C6818DF_D09C_42EF_A18B_DD95A51AD0B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// EinstellungPpg.h : Declaration of the CEinstellungPropPage property page class.
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
// CEinstellungPropPage : See EinstellungPpg.cpp.cpp for implementation.

class CEinstellungPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CEinstellungPropPage)
	DECLARE_OLECREATE_EX(CEinstellungPropPage)

// Constructor
public:
	CEinstellungPropPage();

// Dialog Data
	//{{AFX_DATA(CEinstellungPropPage)
	enum { IDD = IDD_PROPPAGE_EINSTELLUNG };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CEinstellungPropPage)
	afx_msg void OnWww();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EINSTELLUNGPPG_H__7C6818DF_D09C_42EF_A18B_DD95A51AD0B0__INCLUDED)
