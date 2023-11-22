#if !defined(AFX_FORMULARCTL_H__99F74220_7C47_4E2D_ADDC_19E235DD1BF3__INCLUDED_)
#define AFX_FORMULARCTL_H__99F74220_7C47_4E2D_ADDC_19E235DD1BF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// FormularCtl.h : Declaration of the CFormularCtrl ActiveX Control class.
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
// CFormularCtrl : See FormularCtl.cpp for implementation.

class CFormularCtrl : public COleControl
{
	DECLARE_DYNCREATE(CFormularCtrl)

// Constructor
public:
	CFormularCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormularCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CFormularCtrl();

	DECLARE_OLECREATE_EX(CFormularCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CFormularCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CFormularCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CFormularCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CFormularCtrl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CFormularCtrl)
	afx_msg void SetzeDokumentID(long ID);
	afx_msg BSTR HoleFormularnamen(long Index, LPCTSTR Filter);
	afx_msg BSTR HoleFormularpfad(long Index);
	afx_msg long HoleFormularanzahl();
	afx_msg void WaehleFormular(LPCTSTR Formularpfad);
	afx_msg long HoleFeldanzahl();
	afx_msg long HoleFeldIDUeberIndex(long Index);
	afx_msg BSTR HoleFeldwertUeberIndex(long Index);
	afx_msg BSTR HoleFeldwertUeberID(long FeldID);
	afx_msg BSTR HoleFeldbeschreibungUeberID(long FeldID);
	afx_msg long HoleVoranmeldungszeitraum();
	afx_msg BSTR HoleVerknuepfteKonten(LPCTSTR Formularname, long FeldID);
	afx_msg void WaehleFormularUndBetrieb(LPCTSTR Formularpfad, LPCTSTR Betrieb);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CFormularCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CFormularCtrl)
	dispidSetzeDokumentID = 1L,
	dispidHoleFormularnamen = 2L,
	dispidHoleFormularpfad = 3L,
	dispidHoleFormularanzahl = 4L,
	dispidWaehleFormular = 5L,
	dispidHoleFeldanzahl = 6L,
	dispidHoleFeldIDUeberIndex = 7L,
	dispidHoleFeldwertUeberIndex = 8L,
	dispidHoleFeldwertUeberID = 9L,
	dispidHoleFeldbeschreibungUeberID = 10L,
	dispidHoleVoranmeldungszeitraum = 11L,
	dispidHoleVerknuepfteKonten = 12L,
	dispidWaehleFormularUndBetrieb = 13L,
	//}}AFX_DISP_ID
	};
private:
	long m_ID;
	CEasyCashDoc *m_pDoc;
	int m_FeldIDs[10000];
	CString m_Formular;
	CString m_Betrieb;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMULARCTL_H__99F74220_7C47_4E2D_ADDC_19E235DD1BF3__INCLUDED)
