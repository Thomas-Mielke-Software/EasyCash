#if !defined(AFX_ICONAUSWAHLBESTANDSKONTO_H__05B7ED18_9D51_4B8C_9F6B_7DAA4BAAB943__INCLUDED_)
#define AFX_ICONAUSWAHLBESTANDSKONTO_H__05B7ED18_9D51_4B8C_9F6B_7DAA4BAAB943__INCLUDED_

#include "IconAuswahl.h"
#include "ECTIFace\EasyCashDoc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IconAuswahlBestandskonto.h : header file
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
// CIconAuswahlBestandskonto dialog

class CIconAuswahlBestandskonto : public CIconAuswahl
{
// Construction
public:
	CIconAuswahlBestandskonto(CEasyCashView* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIconAuswahlBestandskonto)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	virtual void InitDialog();
	virtual IconInfo* GetIcons();
	virtual int GetIconCount();
	virtual UINT GetIconBitmaps() { return IDB_ICONS_BESTANDSKONTEN; };
	virtual void Neu();
	virtual BOOL ChooseProperty(CString &csProperty);
	virtual void Loeschen(int nElement);
	virtual void Umbenennen(int nElement);
	virtual int IconAuswahl();
	virtual CString GetProfileString(char *section, char *key, char *defaultvalue) { char inifile[1000], buffer[1000]; GetIniFileName(inifile, sizeof(inifile)); GetPrivateProfileString(section, key, defaultvalue, buffer, sizeof(buffer), inifile); return (CString)buffer; };
	virtual void WriteProfileString(char *section, char *key, char *property) { char inifile[1000]; GetIniFileName(inifile, sizeof(inifile)); WritePrivateProfileString(section, key, property, inifile); };
	virtual char* GetKey() { return "Bestandskonto"; };
	virtual char* GetSection() { return "Bestandskonten"; };
	virtual char* GetProperty() { sprintf(saldojahr, "Saldo%04d", m_pParent->GetDocument()->nJahr-1); return saldojahr; };


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIconAuswahlBestandskonto)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	char saldojahr[100]; 

	// Generated message map functions
	//{{AFX_MSG(CIconAuswahlBestandskonto)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICONAUSWAHLBESTANDSKONTO_H__05B7ED18_9D51_4B8C_9F6B_7DAA4BAAB943__INCLUDED_)
