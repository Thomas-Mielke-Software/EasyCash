#if !defined(AFX_ICONAUSWAHLMANDANT_H__16C849DD_2047_4920_9CD9_747DD2B0FCD3__INCLUDED_)
#define AFX_ICONAUSWAHLMANDANT_H__16C849DD_2047_4920_9CD9_747DD2B0FCD3__INCLUDED_

#include "IconAuswahl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IconAuswahlMandant.h : header file
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
// CIconAuswahlMandant dialog

class CIconAuswahlMandant : public CIconAuswahl
{
// Construction
public:
	CIconAuswahlMandant(CEasyCashView* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIconAuswahlMandant)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	virtual IconInfo* GetIcons();
	virtual int GetIconCount();
	virtual CString GetIconText(int index) { CString csNummer; csNummer.Format("%s %d", GetKey(), index+1); return csNummer; };
	virtual void Neu();
	virtual BOOL ChooseProperty(CString &csProperty);
	virtual void Loeschen(int nElement);
	virtual void Umbenennen(int nElement);
	virtual int IconAuswahl();
	virtual char* GetKey() { return "Mandant"; };
	virtual char* GetSection() { return "Mandanten"; };
	virtual char* GetProperty() { return "Datenverzeichnis"; };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconAuswahlMandant)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIconAuswahlMandant)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICONAUSWAHLMANDANT_H__16C849DD_2047_4920_9CD9_747DD2B0FCD3__INCLUDED_)
