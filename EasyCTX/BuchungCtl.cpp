// BuchungCtl.cpp : Implementation of the CBuchungCtrl ActiveX Control class.
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
#include "BuchungCtl.h"
#include "BuchungPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CBuchungCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CBuchungCtrl, COleControl)
	//{{AFX_MSG_MAP(CBuchungCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CBuchungCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CBuchungCtrl)
	DISP_PROPERTY_NOTIFY(CBuchungCtrl, "ID", m_ID, OnIDChanged, VT_I4)
	DISP_PROPERTY_EX(CBuchungCtrl, "Betrag", GetBetrag, SetBetrag, VT_CY)
	DISP_PROPERTY_EX(CBuchungCtrl, "MWSt", GetMWSt, SetMWSt, VT_R8)
	DISP_PROPERTY_EX(CBuchungCtrl, "AbschreibungNr", GetAbschreibungNr, SetAbschreibungNr, VT_I2)
	DISP_PROPERTY_EX(CBuchungCtrl, "AbschreibungJahre", GetAbschreibungJahre, SetAbschreibungJahre, VT_I2)
	DISP_PROPERTY_EX(CBuchungCtrl, "Beschreibung", GetBeschreibung, SetBeschreibung, VT_BSTR)
	DISP_PROPERTY_EX(CBuchungCtrl, "Datum", GetDatum, SetDatum, VT_DATE)
	DISP_PROPERTY_EX(CBuchungCtrl, "Konto", GetKonto, SetKonto, VT_BSTR)
	DISP_PROPERTY_EX(CBuchungCtrl, "Belegnummer", GetBelegnummer, SetBelegnummer, VT_BSTR)
	DISP_PROPERTY_EX(CBuchungCtrl, "AbschreibungRestwert", GetAbschreibungRestwert, SetAbschreibungRestwert, VT_CY)
	DISP_PROPERTY_EX(CBuchungCtrl, "AbschreibungDegressiv", GetAbschreibungDegressiv, SetAbschreibungDegressiv, VT_BOOL)
	DISP_PROPERTY_EX(CBuchungCtrl, "AbschreibungSatz", GetAbschreibungSatz, SetAbschreibungSatz, VT_R8)
	DISP_FUNCTION(CBuchungCtrl, "HoleNetto", HoleNetto, VT_CY, VTS_NONE)
	DISP_FUNCTION(CBuchungCtrl, "HoleBuchungsjahrNetto", HoleBuchungsjahrNetto, VT_CY, VTS_I4)
	DISP_FUNCTION(CBuchungCtrl, "HoleBenutzerdefWert", HoleBenutzerdefWert, VT_BSTR, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CBuchungCtrl, "SpeichereBenutzerdefWert", SpeichereBenutzerdefWert, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_PROPERTY_EX(CBuchungCtrl, "AbschreibungGenauigkeit", GetAbschreibungGenauigkeit, SetAbschreibungGenauigkeit, VT_I4)
	DISP_PROPERTY_EX(CBuchungCtrl, "Betrieb", GetBetrieb, SetBetrieb, VT_BSTR)
	DISP_PROPERTY_EX(CBuchungCtrl, "Bestandskonto", GetBestandskonto, SetBestandskonto, VT_BSTR)
	DISP_DEFVALUE(CBuchungCtrl, "ID")
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CBuchungCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CBuchungCtrl, COleControl)
	//{{AFX_EVENT_MAP(CBuchungCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CBuchungCtrl, 1)
	PROPPAGEID(CBuchungPropPage::guid)
END_PROPPAGEIDS(CBuchungCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CBuchungCtrl, "EASYCTX.BuchungCtrl.1",
	0x3bc20630, 0x93ba, 0x47b2, 0x82, 0xd1, 0x82, 0xc, 0x7f, 0x19, 0x86, 0xa1)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CBuchungCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DBuchung =
		{ 0xa0d24962, 0xb111, 0x4e4a, { 0x86, 0x49, 0x58, 0x9f, 0xd6, 0x84, 0x57, 0x72 } };
const IID BASED_CODE IID_DBuchungEvents =
		{ 0x4b052fcf, 0x4f4c, 0x4df0, { 0x9f, 0x54, 0x7c, 0xa8, 0x52, 0xc3, 0xe9, 0xbe } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwBuchungOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CBuchungCtrl, IDS_BUCHUNG, _dwBuchungOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl::CBuchungCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CBuchungCtrl

BOOL CBuchungCtrl::CBuchungCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_BUCHUNG,
			IDB_BUCHUNG,
			afxRegInsertable | afxRegApartmentThreading,
			_dwBuchungOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl::CBuchungCtrl - Constructor

CBuchungCtrl::CBuchungCtrl()
{
	InitializeIIDs(&IID_DBuchung, &IID_DBuchungEvents);

	m_ID = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl::~CBuchungCtrl - Destructor

CBuchungCtrl::~CBuchungCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl::OnDraw - Drawing function

void CBuchungCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	CRect r = rcBounds;
	DrawAndCenterBitmap(pdc, IDB_BUCHUNG, &r);
}


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl::DoPropExchange - Persistence support

void CBuchungCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl::OnResetState - Reset control to default state

void CBuchungCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl::AboutBox - Display an "About" box to the user

void CBuchungCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_BUCHUNG);
	if (dlgAbout.DoModal() == IDCANCEL)
		ShellExecute(m_hWnd, "open", "https://www.easyct.de/readarticle.php?article_id=23#Buchung", 
			NULL, ".", SW_SHOWNORMAL);		

}


/////////////////////////////////////////////////////////////////////////////
// CBuchungCtrl message handlers


void CBuchungCtrl::OnIDChanged() 
{
	// TODO: Add notification handler code
}

CURRENCY CBuchungCtrl::GetBetrag() 
{
	CURRENCY cyResult={0,0};
	
	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			cyResult.int64 = ((CBuchung*)m_ID)->Betrag * 100;
		}

	return cyResult;
}

void CBuchungCtrl::SetBetrag(CURRENCY newValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->Betrag = (int)newValue.int64 / 100;

		TRACE1("%d", ((CBuchung*)m_ID)->Betrag);
	}

	SetModifiedFlag();
}

double CBuchungCtrl::GetMWSt() 
{
	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			return (double)((CBuchung*)m_ID)->MWSt / 1000.0;
		}

	return 0.0;
}

void CBuchungCtrl::SetMWSt(double newValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->MWSt = (int)((double)newValue * 1000.0);
	}

	SetModifiedFlag();
}

short CBuchungCtrl::GetAbschreibungNr() 
{
	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			return (short)((CBuchung*)m_ID)->AbschreibungNr;
		}

	return 0;
}

void CBuchungCtrl::SetAbschreibungNr(short nNewValue)
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->AbschreibungNr = (int)nNewValue;
	}

	SetModifiedFlag();
}

short CBuchungCtrl::GetAbschreibungJahre() 
{
	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			return (short)((CBuchung*)m_ID)->AbschreibungJahre;
		}

	return 0;
}

void CBuchungCtrl::SetAbschreibungJahre(short nNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->AbschreibungJahre = (int)nNewValue;
	}

	SetModifiedFlag();
}

BSTR CBuchungCtrl::GetBeschreibung() 
{
	CString strResult;

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			strResult = ((CBuchung*)m_ID)->Beschreibung;
		}

	return strResult.AllocSysString();
}

void CBuchungCtrl::SetBeschreibung(LPCTSTR lpszNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->Beschreibung = lpszNewValue;
	}

	SetModifiedFlag();
}

DATE CBuchungCtrl::GetDatum() 
{
	DATE dtResult = 0;

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			SYSTEMTIME timeTemp;
			((CBuchung*)m_ID)->Datum.GetAsSystemTime(timeTemp);
			COleDateTime cdtBuchungsdatum(timeTemp);
			dtResult = cdtBuchungsdatum.m_dt;
		}

	return dtResult;
}

void CBuchungCtrl::SetDatum(DATE newValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		SYSTEMTIME timeTemp;
		COleDateTime odtTemp(newValue);
		odtTemp.GetAsSystemTime(timeTemp);
		CTime ctTemp(timeTemp);

		((CBuchung*)m_ID)->Datum = ctTemp;
	}

	SetModifiedFlag();
}

BSTR CBuchungCtrl::GetKonto() 
{
	CString strResult;

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			strResult = ((CBuchung*)m_ID)->Konto;
		}

	return strResult.AllocSysString();
}

void CBuchungCtrl::SetKonto(LPCTSTR lpszNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->Konto = lpszNewValue;
	}

	SetModifiedFlag();
}

BSTR CBuchungCtrl::GetBelegnummer() 
{
	CString strResult;

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			strResult = ((CBuchung*)m_ID)->Belegnummer;
		}

	return strResult.AllocSysString();
}

void CBuchungCtrl::SetBelegnummer(LPCTSTR lpszNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->Belegnummer = lpszNewValue;
	}

	SetModifiedFlag();
}

CURRENCY CBuchungCtrl::GetAbschreibungRestwert() 
{
	CURRENCY cyResult={0,0};

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			cyResult.int64 = ((CBuchung*)m_ID)->AbschreibungRestwert * 100;
		}

	return cyResult;
}

void CBuchungCtrl::SetAbschreibungRestwert(CURRENCY newValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->AbschreibungRestwert = (int)newValue.int64 / 100;
	}

	SetModifiedFlag();
}

BOOL CBuchungCtrl::GetAbschreibungDegressiv() 
{
	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			return ((CBuchung*)m_ID)->AbschreibungDegressiv;
		}

	return FALSE;
}

void CBuchungCtrl::SetAbschreibungDegressiv(BOOL bNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->AbschreibungDegressiv = bNewValue;
	}

	SetModifiedFlag();
}

double CBuchungCtrl::GetAbschreibungSatz() 
{
	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			return (double)((CBuchung*)m_ID)->AbschreibungSatz;
		}

	return 0.0;
}

void CBuchungCtrl::SetAbschreibungSatz(double newValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->AbschreibungSatz = (int)newValue;
	}

	SetModifiedFlag();
}

CURRENCY CBuchungCtrl::HoleNetto() 
{
	CURRENCY cyResult={0,0};

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			cyResult.int64 = ((CBuchung*)m_ID)->GetNetto() * 100;
		}

	return cyResult;
}

CURRENCY CBuchungCtrl::HoleBuchungsjahrNetto(long dokID) 
{
	CURRENCY cyResult={0,0};

	if (m_ID && dokID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)) && ((CEasyCashDoc*)dokID)->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
		{
			cyResult.int64 = ((CBuchung*)m_ID)->GetBuchungsjahrNetto((CEasyCashDoc*)dokID) * 100;
		}

	return cyResult;
}


BSTR CBuchungCtrl::HoleBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName)
{
	CString strResult;

	CBuchung* pBuch = (CBuchung*)m_ID;
	if (pBuch && pBuch->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		char *pvalstart = GetErweiterungKey(pBuch->Erweiterung, PluginName, SchluesselName);
		if (pvalstart)
		{
			char *pvalend = strchr(pvalstart, '|');
			if (pvalend)
			{
				char *strResultBuffer = strResult.GetBuffer(pvalend-pvalstart+1);
				strncpy(strResultBuffer, pvalstart, pvalend-pvalstart);
				strResultBuffer[pvalend-pvalstart] = '\0';
				strResult.ReleaseBuffer();
			}
		}
	}	

	return strResult.AllocSysString();
}

void CBuchungCtrl::SpeichereBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName, LPCTSTR Wert) 
{
	CBuchung* pBuch = (CBuchung*)m_ID;
	if (pBuch && pBuch->IsKindOf(RUNTIME_CLASS(CBuchung)))
		SetErweiterungKey(pBuch->Erweiterung, PluginName, SchluesselName, Wert);
}

long CBuchungCtrl::GetAbschreibungGenauigkeit() 
{
	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			return ((CBuchung*)m_ID)->AbschreibungGenauigkeit;
		}

	return 0;
}

void CBuchungCtrl::SetAbschreibungGenauigkeit(long nNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->AbschreibungGenauigkeit = (int)nNewValue;
	}

	SetModifiedFlag();
}

BSTR CBuchungCtrl::GetBetrieb() 
{
	CString strResult;

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			strResult = ((CBuchung*)m_ID)->Betrieb;
		}

	return strResult.AllocSysString();
}

void CBuchungCtrl::SetBetrieb(LPCTSTR lpszNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->Betrieb = lpszNewValue;
	}

	SetModifiedFlag();
}

BSTR CBuchungCtrl::GetBestandskonto() 
{
	CString strResult;

	if (m_ID) 
		if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
		{
			strResult = ((CBuchung*)m_ID)->Bestandskonto;
		}

	return strResult.AllocSysString();
}

void CBuchungCtrl::SetBestandskonto(LPCTSTR lpszNewValue) 
{
	if (!m_ID) return;

	if (((CBuchung*)m_ID)->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		((CBuchung*)m_ID)->Bestandskonto = lpszNewValue;
	}

	SetModifiedFlag();
}
