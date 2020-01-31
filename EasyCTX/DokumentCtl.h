#if !defined(AFX_DOKUMENTCTL_H__69DD2A6E_C022_4938_938F_4972BC497FB5__INCLUDED_)
#define AFX_DOKUMENTCTL_H__69DD2A6E_C022_4938_938F_4972BC497FB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// DokumentCtl.h : Declaration of the CDokumentCtrl ActiveX Control class.
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
// CDokumentCtrl : See DokumentCtl.cpp for implementation.

class CDokumentCtrl : public COleControl
{
	DECLARE_DYNCREATE(CDokumentCtrl)

// Constructor
public:
	CDokumentCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDokumentCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CDokumentCtrl();

	DECLARE_OLECREATE_EX(CDokumentCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CDokumentCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CDokumentCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CDokumentCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CDokumentCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CDokumentCtrl)
	afx_msg long GetID();
	afx_msg void SetID(long nNewValue);
	afx_msg short GetJahr();
	afx_msg void SetJahr(short nNewValue);
	afx_msg long GetLaufendeBuchungsnummerEinnahmen();
	afx_msg void SetLaufendeBuchungsnummerEinnahmen(long nNewValue);
	afx_msg long GetLaufendeBuchungsnummerAusgaben();
	afx_msg void SetLaufendeBuchungsnummerAusgaben(long nNewValue);
	afx_msg BSTR GetWaehrung();
	afx_msg void SetWaehrung(LPCTSTR lpszNewValue);
	afx_msg long GetAbschreibungGenauigkeit();
	afx_msg void SetAbschreibungGenauigkeit(long nNewValue);
	afx_msg long GetLaufendeBuchungsnummerBank();
	afx_msg void SetLaufendeBuchungsnummerBank(long nNewValue);
	afx_msg long GetLaufendeBuchungsnummerKasse();
	afx_msg void SetLaufendeBuchungsnummerKasse(long nNewValue);
	afx_msg long FindeErsteEinnahmenBuchung();
	afx_msg long FindeErsteAusgabenBuchung();
	afx_msg long FindeNaechsteBuchung(long BuchungID);
	afx_msg long NeueEinnahmenbuchungEinfuegen();
	afx_msg long NeueAusgabenbuchungEinfuegen();
	afx_msg void Sortieren();
	afx_msg void LoescheBuchung(long BuchungID);
	afx_msg CURRENCY HoleEinnahmenSumme();
	afx_msg CURRENCY HoleAusgabenSumme();
	afx_msg long HoleDokumentVersion();
	afx_msg BSTR HoleBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName);
	afx_msg void SpeichereBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName, LPCTSTR Wert);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

	int m_id;
	int m_jahr;

// Event maps
	//{{AFX_EVENT(CDokumentCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CDokumentCtrl)
	dispidID = 1L,
	dispidJahr = 2L,
	dispidLaufendeBuchungsnummerEinnahmen = 3L,
	dispidLaufendeBuchungsnummerAusgaben = 4L,
	dispidWaehrung = 5L,
	dispidAbschreibungGenauigkeit = 6L,
	dispidLaufendeBuchungsnummerBank = 7L,
	dispidLaufendeBuchungsnummerKasse = 8L,
	dispidFindeErsteEinnahmenBuchung = 9L,
	dispidFindeErsteAusgabenBuchung = 10L,
	dispidFindeNaechsteBuchung = 11L,
	dispidNeueEinnahmenbuchungEinfuegen = 12L,
	dispidNeueAusgabenbuchungEinfuegen = 13L,
	dispidSortieren = 14L,
	dispidLoescheBuchung = 15L,
	dispidHoleEinnahmenSumme = 16L,
	dispidHoleAusgabenSumme = 17L,
	dispidHoleDokumentVersion = 18L,
	dispidHoleBenutzerdefWert = 19L,
	dispidSpeichereBenutzerdefWert = 20L,
	//}}AFX_DISP_ID
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOKUMENTCTL_H__69DD2A6E_C022_4938_938F_4972BC497FB5__INCLUDED)
