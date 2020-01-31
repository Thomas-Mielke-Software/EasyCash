#if !defined(AFX_BUCHUNGCTL_H__95C49F76_21E1_432C_A2DD_E47DEA793544__INCLUDED_)
#define AFX_BUCHUNGCTL_H__95C49F76_21E1_432C_A2DD_E47DEA793544__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// BuchungCtl.h : Declaration of the CBuchungCtrl ActiveX Control class.
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
// CBuchungCtrl : See BuchungCtl.cpp for implementation.

class CBuchungCtrl : public COleControl
{
	DECLARE_DYNCREATE(CBuchungCtrl)

// Constructor
public:
	CBuchungCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBuchungCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CBuchungCtrl();

	DECLARE_OLECREATE_EX(CBuchungCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CBuchungCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CBuchungCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CBuchungCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CBuchungCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CBuchungCtrl)
	long m_ID;
	afx_msg void OnIDChanged();
	afx_msg CURRENCY GetBetrag();
	afx_msg void SetBetrag(CURRENCY newValue);
	afx_msg double GetMWSt();
	afx_msg void SetMWSt(double newValue);
	afx_msg short GetAbschreibungNr();
	afx_msg void SetAbschreibungNr(short nNewValue);
	afx_msg short GetAbschreibungJahre();
	afx_msg void SetAbschreibungJahre(short nNewValue);
	afx_msg BSTR GetBeschreibung();
	afx_msg void SetBeschreibung(LPCTSTR lpszNewValue);
	afx_msg DATE GetDatum();
	afx_msg void SetDatum(DATE newValue);
	afx_msg BSTR GetKonto();
	afx_msg void SetKonto(LPCTSTR lpszNewValue);
	afx_msg BSTR GetBelegnummer();
	afx_msg void SetBelegnummer(LPCTSTR lpszNewValue);
	afx_msg CURRENCY GetAbschreibungRestwert();
	afx_msg void SetAbschreibungRestwert(CURRENCY newValue);
	afx_msg BOOL GetAbschreibungDegressiv();
	afx_msg void SetAbschreibungDegressiv(BOOL bNewValue);
	afx_msg double GetAbschreibungSatz();
	afx_msg void SetAbschreibungSatz(double newValue);
	afx_msg CURRENCY HoleNetto();
	afx_msg CURRENCY HoleBuchungsjahrNetto(long dokID);
	afx_msg BSTR HoleBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName);
	afx_msg void SpeichereBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName, LPCTSTR Wert);
	afx_msg long GetAbschreibungGenauigkeit();
	afx_msg void SetAbschreibungGenauigkeit(long nNewValue);
	afx_msg BSTR GetBetrieb();
	afx_msg void SetBetrieb(LPCTSTR lpszNewValue);
	afx_msg BSTR GetBestandskonto();
	afx_msg void SetBestandskonto(LPCTSTR lpszNewValue);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CBuchungCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CBuchungCtrl)
	dispidID = 1L,
	dispidBetrag = 2L,
	dispidMWSt = 3L,
	dispidAbschreibungNr = 4L,
	dispidAbschreibungJahre = 5L,
	dispidBeschreibung = 6L,
	dispidDatum = 7L,
	dispidKonto = 8L,
	dispidBelegnummer = 9L,
	dispidAbschreibungRestwert = 10L,
	dispidAbschreibungDegressiv = 11L,
	dispidAbschreibungSatz = 12L,
	dispidHoleNetto = 13L,
	dispidHoleBuchungsjahrNetto = 14L,
	dispidHoleBenutzerdefWert = 15L,
	dispidSpeichereBenutzerdefWert = 16L,
	dispidAbschreibungGenauigkeit = 17L,
	dispidBetrieb = 18L,
	dispidBestandskonto = 19L,
	//}}AFX_DISP_ID
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUCHUNGCTL_H__95C49F76_21E1_432C_A2DD_E47DEA793544__INCLUDED)
