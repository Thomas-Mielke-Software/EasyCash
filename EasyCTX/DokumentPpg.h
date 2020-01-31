#if !defined(AFX_DOKUMENTPPG_H__5DC56CE8_6ABB_4DE6_A148_699FEF25A326__INCLUDED_)
#define AFX_DOKUMENTPPG_H__5DC56CE8_6ABB_4DE6_A148_699FEF25A326__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// DokumentPpg.h : Declaration of the CDokumentPropPage property page class.
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
// CDokumentPropPage : See DokumentPpg.cpp.cpp for implementation.

class CDokumentPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CDokumentPropPage)
	DECLARE_OLECREATE_EX(CDokumentPropPage)

// Constructor
public:
	CDokumentPropPage();

// Dialog Data
	//{{AFX_DATA(CDokumentPropPage)
	enum { IDD = IDD_PROPPAGE_DOKUMENT };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CDokumentPropPage)
	afx_msg void OnWww();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOKUMENTPPG_H__5DC56CE8_6ABB_4DE6_A148_699FEF25A326__INCLUDED)
