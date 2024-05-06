#if !defined(AFX_ICONAUSWAHL_H__00B24629_A888_4701_93C4_5873FBE905A1__INCLUDED_)
#define AFX_ICONAUSWAHL_H__00B24629_A888_4701_93C4_5873FBE905A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IconAuswahl.h : header file
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

#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahl dialog

class IconInfo {
public:
	char *Text;
};

class CIconAuswahl : public CDialog
{
// Construction
public:
	int m_nModus;
	int m_nSelected;
	CIconAuswahl(CEasyCashView* pParent = NULL) { ASSERT(FALSE); } // never create!
	CIconAuswahl(UINT id, CEasyCashView * parent = NULL);

// Dialog Data
	//{{AFX_DATA(CIconAuswahl)
	enum { IDD = IDD_ICONAUSWAHL };
	CListCtrl	m_liste;
	//}}AFX_DATA

	virtual void InitDialog();
	virtual IconInfo* GetIcons() { return NULL; };
	virtual int GetIconCount() { return 0; };
	virtual UINT GetIconBitmaps() { return IDB_ICONS; };
	virtual CString GetIconText(int index, int icon) { return (CString)(GetIcons()[icon].Text); };
	virtual void Neu() { return; };
	virtual BOOL ChooseProperty(CString &csProperty) { csProperty = ""; return TRUE; };
	virtual void Loeschen(int nElement) { return; };
	virtual void Umbenennen(int nElement) { return; };
	virtual int IconAuswahl() { return -1; };
	virtual CString GetProfileString(char *section, char *key, char *defaultvalue) { return theApp.GetProfileString(section, key, defaultvalue); };
	virtual void WriteProfileString(char *section, char *key, char *property) { theApp.WriteProfileString(section, key, property); };
	virtual char* GetKey() { return "Bitte_Key_Setzen_in_Subclass_von_CIconAuswahl"; };
	virtual char* GetSection() { return "Bitte_Section_Setzen_in_Subclass_von_CIconAuswahl"; };
	virtual char* GetProperty() { return "Bitte_Property_Setzen_in_Subclass_von_CIconAuswahl"; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconAuswahl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	CEasyCashView *m_pParent;
	CImageList m_imgList;

	// Generated message map functions
	//{{AFX_MSG(CIconAuswahl)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkIconliste(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCancel();
	afx_msg void OnNeu();
	afx_msg void OnLoeschen();
	afx_msg void OnUmbenennen();
	afx_msg void OnIconAendern();
	afx_msg void OnProperty();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICONAUSWAHL_H__00B24629_A888_4701_93C4_5873FBE905A1__INCLUDED_)
