// FormularCtl.cpp : Implementation of the CFormularCtrl ActiveX Control class.
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

#include "stdafx.h"
#include "EasyCTX.h"
#include "FormularCtl.h"
#include "FormularPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CFormularCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CFormularCtrl, COleControl)
	//{{AFX_MSG_MAP(CFormularCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CFormularCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CFormularCtrl)
	DISP_FUNCTION(CFormularCtrl, "SetzeDokumentID", SetzeDokumentID, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CFormularCtrl, "HoleFormularnamen", HoleFormularnamen, VT_BSTR, VTS_I4 VTS_BSTR)
	DISP_FUNCTION(CFormularCtrl, "HoleFormularpfad", HoleFormularpfad, VT_BSTR, VTS_I4) 
	DISP_FUNCTION(CFormularCtrl, "HoleFormularanzahl", HoleFormularanzahl, VT_I4, VTS_NONE)
	DISP_FUNCTION(CFormularCtrl, "WaehleFormular", WaehleFormular, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(CFormularCtrl, "HoleFeldanzahl", HoleFeldanzahl, VT_I4, VTS_NONE)
	DISP_FUNCTION(CFormularCtrl, "HoleFeldIDUeberIndex", HoleFeldIDUeberIndex, VT_I4, VTS_I4)
	DISP_FUNCTION(CFormularCtrl, "HoleFeldwertUeberIndex", HoleFeldwertUeberIndex, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CFormularCtrl, "HoleFeldwertUeberID", HoleFeldwertUeberID, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CFormularCtrl, "HoleFeldbeschreibungUeberID", HoleFeldbeschreibungUeberID, VT_BSTR, VTS_I4)
	DISP_FUNCTION(CFormularCtrl, "HoleVoranmeldungszeitraum", HoleVoranmeldungszeitraum, VT_I4, VTS_NONE)
	DISP_FUNCTION(CFormularCtrl, "HoleVerknuepfteKonten", HoleVerknuepfteKonten, VT_BSTR, VTS_BSTR VTS_I4)
	DISP_FUNCTION(CFormularCtrl, "WaehleFormularUndBetrieb", WaehleFormularUndBetrieb, VT_EMPTY, VTS_BSTR VTS_BSTR)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CFormularCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CFormularCtrl, COleControl)
	//{{AFX_EVENT_MAP(CFormularCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CFormularCtrl, 1)
	PROPPAGEID(CFormularPropPage::guid)
END_PROPPAGEIDS(CFormularCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CFormularCtrl, "EASYCTX.FormularCtrl.1",
	0xdfa091a6, 0xd040, 0x444f, 0x80, 0xa, 0x65, 0x6b, 0xea, 0x33, 0x61, 0xc4)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CFormularCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DFormular =
		{ 0x8bdfadf1, 0x8317, 0x4eba, { 0x88, 0xbb, 0xe5, 0xbc, 0x12, 0x90, 0xdd, 0x74 } };
const IID BASED_CODE IID_DFormularEvents =
		{ 0x9910463d, 0x5929, 0x4e27, { 0x83, 0x68, 0x4c, 0xc0, 0x93, 0x2a, 0xcf, 0x64 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwFormularOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CFormularCtrl, IDS_FORMULAR, _dwFormularOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl::CFormularCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CFormularCtrl

BOOL CFormularCtrl::CFormularCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_FORMULAR,
			IDB_FORMULAR,
			afxRegInsertable | afxRegApartmentThreading,
			_dwFormularOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl::CFormularCtrl - Constructor

CFormularCtrl::CFormularCtrl()
{
	InitializeIIDs(&IID_DFormular, &IID_DFormularEvents);

	// TODO: Initialize your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl::~CFormularCtrl - Destructor

CFormularCtrl::~CFormularCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl::OnDraw - Drawing function

void CFormularCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	CRect r = rcBounds;
	DrawAndCenterBitmap(pdc, IDB_FORMULAR, &r);
}


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl::DoPropExchange - Persistence support

void CFormularCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl::OnResetState - Reset control to default state

void CFormularCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl::AboutBox - Display an "About" box to the user

void CFormularCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_FORMULAR);
	if (dlgAbout.DoModal() == IDCANCEL)
		ShellExecute(m_hWnd, "open", "http://www.easyct.de/readarticle.php?article_id=23#Formular", 
			NULL, ".", SW_SHOWNORMAL);		
}


/////////////////////////////////////////////////////////////////////////////
// CFormularCtrl message handlers


BSTR CFormularCtrl::HoleFormularnamen(long Index, LPCTSTR Filter) 
{
	CString strResult;
	HoleFormularnamenAusCSA(Index, Filter, m_pDoc->m_csaFormulare, &strResult);	
/*	warum habe ich das hier mal gemacht? warum obige zeile auskommentiert?
	if (Index >=0 && Index < m_pDoc->m_csaFormulare.GetSize())
	{
		XDoc xmldoc;
#if defined(_DEBUG)
	TRACE2("HoleFormularnamen(): index %d name '%s'\r\n", (int)Index, (LPCSTR)m_pDoc->m_csaFormulare[Index]);
#endif
		xmldoc.LoadFile(m_pDoc->m_csaFormulare[Index]);
		LPXNode xml = xmldoc.GetRoot();
		if (xml) 
		{
			LPCTSTR attr_name = xml->GetAttrValue("name");

			if (attr_name && (CString)attr_name == (CString)Filter)
				strResult = xml->GetAttrValue("anzeigename");
		}
	}
*/
	return strResult.AllocSysString();
}

void CFormularCtrl::SetzeDokumentID(long ID) 
{
	m_pDoc = (CEasyCashDoc *)ID;
	LadeECFormulare(m_pDoc->m_csaFormulare);
}

BSTR CFormularCtrl::HoleFormularpfad(long Index) 
{
	if (Index >=0 && Index < m_pDoc->m_csaFormulare.GetSize())
		return m_pDoc->m_csaFormulare[Index].AllocSysString();
	else 
		return ((CString)"").AllocSysString();
}

long CFormularCtrl::HoleFormularanzahl() 
{
	return m_pDoc->m_csaFormulare.GetSize();
}

void CFormularCtrl::WaehleFormular(LPCTSTR Formular) 
{
	m_Formular = Formular;
	m_pDoc->BerechneFormularfeldwerte((CString &)Formular, m_pDoc->m_csaFormularfeldwerte, m_FeldIDs, &m_pDoc->m_csaFormularfeldbeschreibungen);
}

long CFormularCtrl::HoleFeldanzahl() 
{
	return m_pDoc->m_csaFormularfeldwerte.GetSize();
}

long CFormularCtrl::HoleFeldIDUeberIndex(long Index) 
{
	return m_FeldIDs[m_ID];
}

BSTR CFormularCtrl::HoleFeldwertUeberIndex(long Index) 
{
	if (Index >= 0 && Index < m_pDoc->m_csaFormularfeldwerte.GetSize())
		return m_pDoc->m_csaFormularfeldwerte[Index].AllocSysString();
	else 
		return ((CString)"").AllocSysString();
}

BSTR CFormularCtrl::HoleFeldwertUeberID(long FeldID) 
{
	if (m_FeldIDs[FeldID] >= 0 && m_FeldIDs[FeldID] < m_pDoc->m_csaFormularfeldwerte.GetSize())
		return m_pDoc->m_csaFormularfeldwerte[m_FeldIDs[FeldID]].AllocSysString();
	else 
		return ((CString)"").AllocSysString();
}

BSTR CFormularCtrl::HoleFeldbeschreibungUeberID(long FeldID) 
{
	if (m_FeldIDs[FeldID] >= 0 && m_FeldIDs[FeldID] < m_pDoc->m_csaFormularfeldbeschreibungen.GetSize())
		return m_pDoc->m_csaFormularfeldbeschreibungen[m_FeldIDs[FeldID]].AllocSysString();
	else 
		return ((CString)"").AllocSysString();
}

long CFormularCtrl::HoleVoranmeldungszeitraum() 
{
	return m_pDoc->m_nZeitraum;
}

// gibt alle mit einer bestimmte Formular/FeldID-Kombination verknüpften Konten als tabulator-getrennte Liste zurück, 'E' oder 'A' ist den Kontennamen vorangestellt
BSTR CFormularCtrl::HoleVerknuepfteKonten(LPCTSTR Formularname, long FeldID) 
{
	m_pDoc->ReadKontenCache();
	struct Kontenart 
	{
		char *typ;  // 'E' für Einnahmen, 'A' für Ausgaben
		CStringArray *konto;
		CStringArray *feldzuweisungen;
	} Kontenarten[] = 
	{
		"E", &m_pDoc->m_csEinnahmenKonten, &m_pDoc->m_csEinnahmenFeldzuweisungen,
		"A", &m_pDoc->m_csAusgabenKonten, &m_pDoc->m_csAusgabenFeldzuweisungen
	};

	CString csVerknuepfteKonten = "";
	for (int k = 0; k < 1; k++)
		for (int i = 0; i < Kontenarten[k].feldzuweisungen->GetSize() && !Kontenarten[k].konto[i].IsEmpty(); i++)
		{
			CString csTemp = GetErweiterungKey((*Kontenarten[k].feldzuweisungen)[i], "ECT", Formularname);
			if (FeldID == atol(csTemp))
				csVerknuepfteKonten += Kontenarten[k].typ + (*Kontenarten[k].konto)[i] + "\t";
		}

	return csVerknuepfteKonten.AllocSysString();
}

void CFormularCtrl::WaehleFormularUndBetrieb(LPCTSTR Formular, LPCTSTR Betrieb)  
{
	m_Formular = Formular;
	m_Betrieb = Betrieb;
	m_pDoc->BerechneFormularfeldwerte(m_Formular, m_pDoc->m_csaFormularfeldwerte, m_FeldIDs, &m_pDoc->m_csaFormularfeldbeschreibungen, m_Betrieb);
}
