// DokumentCtl.cpp : Implementation of the CDokumentCtrl ActiveX Control class.
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
#include "DokumentCtl.h"
#include "DokumentPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CDokumentCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CDokumentCtrl, COleControl)
	//{{AFX_MSG_MAP(CDokumentCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CDokumentCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CDokumentCtrl)
	DISP_PROPERTY_EX(CDokumentCtrl, "ID", GetID, SetID, VT_I4)
	DISP_PROPERTY_EX(CDokumentCtrl, "Jahr", GetJahr, SetJahr, VT_I2)
	DISP_PROPERTY_EX(CDokumentCtrl, "LaufendeBuchungsnummerEinnahmen", GetLaufendeBuchungsnummerEinnahmen, SetLaufendeBuchungsnummerEinnahmen, VT_I4)
	DISP_PROPERTY_EX(CDokumentCtrl, "LaufendeBuchungsnummerAusgaben", GetLaufendeBuchungsnummerAusgaben, SetLaufendeBuchungsnummerAusgaben, VT_I4)
	DISP_PROPERTY_EX(CDokumentCtrl, "Waehrung", GetWaehrung, SetWaehrung, VT_BSTR)
	DISP_PROPERTY_EX(CDokumentCtrl, "AbschreibungGenauigkeit", GetAbschreibungGenauigkeit, SetAbschreibungGenauigkeit, VT_I4)
	DISP_PROPERTY_EX(CDokumentCtrl, "LaufendeBuchungsnummerBank", GetLaufendeBuchungsnummerBank, SetLaufendeBuchungsnummerBank, VT_I4)
	DISP_PROPERTY_EX(CDokumentCtrl, "LaufendeBuchungsnummerKasse", GetLaufendeBuchungsnummerKasse, SetLaufendeBuchungsnummerKasse, VT_I4)
	DISP_FUNCTION(CDokumentCtrl, "FindeErsteEinnahmenBuchung", FindeErsteEinnahmenBuchung, VT_I4, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "FindeErsteAusgabenBuchung", FindeErsteAusgabenBuchung, VT_I4, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "FindeNaechsteBuchung", FindeNaechsteBuchung, VT_I4, VTS_I4)
	DISP_FUNCTION(CDokumentCtrl, "NeueEinnahmenbuchungEinfuegen", NeueEinnahmenbuchungEinfuegen, VT_I4, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "NeueAusgabenbuchungEinfuegen", NeueAusgabenbuchungEinfuegen, VT_I4, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "Sortieren", Sortieren, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "LoescheBuchung", LoescheBuchung, VT_EMPTY, VTS_I4)
	DISP_FUNCTION(CDokumentCtrl, "HoleEinnahmenSumme", HoleEinnahmenSumme, VT_CY, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "HoleAusgabenSumme", HoleAusgabenSumme, VT_CY, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "HoleDokumentVersion", HoleDokumentVersion, VT_I4, VTS_NONE)
	DISP_FUNCTION(CDokumentCtrl, "HoleBenutzerdefWert", HoleBenutzerdefWert, VT_BSTR, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CDokumentCtrl, "SpeichereBenutzerdefWert", SpeichereBenutzerdefWert, VT_EMPTY, VTS_BSTR VTS_BSTR VTS_BSTR)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CDokumentCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

// LoescheBuchung: VT_I4 ersetzt durch VT_EMPTY in v36

/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CDokumentCtrl, COleControl)
	//{{AFX_EVENT_MAP(CDokumentCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CDokumentCtrl, 1)
	PROPPAGEID(CDokumentPropPage::guid)
END_PROPPAGEIDS(CDokumentCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CDokumentCtrl, "EASYCTX.DokumentCtrl.1",
	0xed09076f, 0xb744, 0x47a1, 0x8d, 0x60, 0xf9, 0xef, 0x9, 0x3f, 0x19, 0xf0)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CDokumentCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DDokument =
		{ 0x874cec1, 0x2df3, 0x415a, { 0xae, 0xc9, 0xbe, 0x6c, 0xa1, 0xc1, 0x9f, 0x2 } };
const IID BASED_CODE IID_DDokumentEvents =
		{ 0xbd5b39d5, 0x30d3, 0x4ad8, { 0x96, 0xa3, 0xc4, 0x7d, 0xf1, 0x9a, 0x4d, 0xfe } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwDokumentOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CDokumentCtrl, IDS_DOKUMENT, _dwDokumentOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl::CDokumentCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CDokumentCtrl

BOOL CDokumentCtrl::CDokumentCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_DOKUMENT,
			IDB_DOKUMENT,
			afxRegInsertable | afxRegApartmentThreading,
			_dwDokumentOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl::CDokumentCtrl - Constructor

CDokumentCtrl::CDokumentCtrl()
{
	InitializeIIDs(&IID_DDokument, &IID_DDokumentEvents);

	m_id = 0;
	m_jahr = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl::~CDokumentCtrl - Destructor

CDokumentCtrl::~CDokumentCtrl()
{
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl::OnDraw - Drawing function

void CDokumentCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	CRect r = rcBounds;
	DrawAndCenterBitmap(pdc, IDB_DOKUMENT, &r);
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl::DoPropExchange - Persistence support

void CDokumentCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl::OnResetState - Reset control to default state

void CDokumentCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl::AboutBox - Display an "About" box to the user

void CDokumentCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_DOKUMENT);
	if (dlgAbout.DoModal() == IDCANCEL)
		ShellExecute(m_hWnd, "open", "http://easyct.no-ip.org/readarticle.php?article_id=23#Dokument", 
			NULL, ".", SW_SHOWNORMAL);		
}


/////////////////////////////////////////////////////////////////////////////
// CDokumentCtrl message handlers


long CDokumentCtrl::GetID() 
{
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//CString csTemp;
	//csTemp.Format("GetID: %d", m_id);
	//AfxMessageBox(csTemp);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return (long)m_id;
}

void CDokumentCtrl::SetID(long nNewValue) 
{
	//!!TEST-CODE!!!!!!!!!!!!!!!!!!!!!!
	//CString csTemp;
	//csTemp.Format("SetID: %d", nNewValue);
	//AfxMessageBox(csTemp);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	m_id = (int)nNewValue;
}

short CDokumentCtrl::GetJahr() 
{
	//!!TEST-CODE!!!!!!!!!!!!!!!!!!!!!!
	//CString csTemp;
	//csTemp.Format("GetJahr: %d", m_id);
	//AfxMessageBox(csTemp);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return pDoc->nJahr;	
	}

	return 0;
}

void CDokumentCtrl::SetJahr(short nNewValue) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		pDoc->nJahr = nNewValue;	

		pDoc->SetModifiedFlag("Buchungsjahr wurde geändert");
	}
}

long CDokumentCtrl::FindeErsteEinnahmenBuchung() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return (long)pDoc->Einnahmen;
	}
	else
		return 0;
}

long CDokumentCtrl::FindeErsteAusgabenBuchung() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return (long)pDoc->Ausgaben;
	}
	else
		return 0;
}

long CDokumentCtrl::FindeNaechsteBuchung(long BuchungID) 
{
	CBuchung* pBuchung = (CBuchung*)BuchungID;
	if (pBuchung && pBuchung->IsKindOf(RUNTIME_CLASS(CBuchung)))
	{
		return (long)pBuchung->next;
	}
	else
		return 0;
}

long CDokumentCtrl::NeueEinnahmenbuchungEinfuegen() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		CBuchung **p;

		p = &(pDoc->Einnahmen);
		while (*p)
			p = &((*p)->next);

		*p = new CBuchung;	
		(*p)->next = NULL;
		(*p)->Datum = CTime::GetCurrentTime();
		(*p)->Betrag = 0;
		(*p)->MWSt = 0;
		(*p)->AbschreibungRestwert = 0;
		(*p)->AbschreibungNr = 1;
		(*p)->AbschreibungJahre = 1;
		(*p)->AbschreibungDegressiv = FALSE;
		(*p)->AbschreibungSatz = 0;

		pDoc->SetModifiedFlag("Neue Einnahmenbuchung wurde eingefügt");

		return (long)(*p);
	}
	else
		return 0;

}

long CDokumentCtrl::NeueAusgabenbuchungEinfuegen() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		CBuchung **p;

		p = &(pDoc->Ausgaben);
		while (*p)
			p = &((*p)->next);

		(*p) = *p = new CBuchung;	
		(*p)->next = NULL;
		(*p)->Datum = CTime::GetCurrentTime();
		(*p)->Betrag = 0;
		(*p)->MWSt = 0;
		(*p)->AbschreibungRestwert = 0;
		(*p)->AbschreibungNr = 1;
		(*p)->AbschreibungJahre = 1;
		(*p)->AbschreibungDegressiv = FALSE;
		(*p)->AbschreibungSatz = 0;

		pDoc->SetModifiedFlag("Neue Ausgabenbuchung wurde eingefügt");

		return (long)(*p);
	}
	else
		return 0;

}

void CDokumentCtrl::Sortieren() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{		
		pDoc->Sort();					// macht Probleme mit undefinierbaren Heap-Corruptions
		pDoc->SetModifiedFlag();
		//pDoc->UpdateAllViews(NULL);
	}
}

void CDokumentCtrl::LoescheBuchung(long BuchungID) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		CBuchung **ppb = &pDoc->Einnahmen;
		CBuchung *pb = (CBuchung *)BuchungID;

		// Einnahmen nach zu löschender Buchung durchsuchen
		BOOL bFound = FALSE;
		while (*ppb)
		{
			if (*ppb == pb)
			{
				pb = (*ppb)->next;
				(*ppb)->next = NULL;	// ganz wichtig wegen Kettenlöschung
				delete *ppb;
				*ppb = pb;

				bFound = TRUE;
				break;
			}

			ppb = &((*ppb)->next);
		}
	
		// wenn in Einnahmen nicht gefunden: Ausgaben nach zu löschender Buchung durchsuchen
		if (!bFound)
		{
			ppb = &pDoc->Ausgaben;

			while (*ppb)
			{
				if (*ppb == pb)
				{
					pb = (*ppb)->next;
					(*ppb)->next = NULL;	// ganz wichtig wegen Kettenlöschung
					delete *ppb;
					*ppb = pb;

					break;
				}

				ppb = &((*ppb)->next);
			}
		}

		pDoc->SetModifiedFlag("Buchung wurde gelöscht");
	}
}

long CDokumentCtrl::GetLaufendeBuchungsnummerEinnahmen() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return pDoc->nLaufendeBuchungsnummerFuerEinnahmen;	
	}

	return 0;
}

void CDokumentCtrl::SetLaufendeBuchungsnummerEinnahmen(long nNewValue) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		pDoc->nLaufendeBuchungsnummerFuerEinnahmen = nNewValue;	

		pDoc->SetModifiedFlag("Laufende Buchungsnummer für Einnahmen wurde geändert");
	}
}

long CDokumentCtrl::GetLaufendeBuchungsnummerAusgaben() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return pDoc->nLaufendeBuchungsnummerFuerAusgaben;	
	}

	return 0;
}

void CDokumentCtrl::SetLaufendeBuchungsnummerAusgaben(long nNewValue) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		pDoc->nLaufendeBuchungsnummerFuerAusgaben = nNewValue;	

		pDoc->SetModifiedFlag("Laufende Buchungsnummer für Ausgaben wurde geändert");
	}
}

BSTR CDokumentCtrl::GetWaehrung() 
{
	CString strResult;

	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		strResult = pDoc->csWaehrung;
	}	

	return strResult.AllocSysString();
}

void CDokumentCtrl::SetWaehrung(LPCTSTR lpszNewValue) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		pDoc->csWaehrung = lpszNewValue;
	}	

	SetModifiedFlag();
}

long CDokumentCtrl::GetAbschreibungGenauigkeit() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return pDoc->AbschreibungGenauigkeit;
	}	

	return 0;
}

void CDokumentCtrl::SetAbschreibungGenauigkeit(long nNewValue) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		pDoc->AbschreibungGenauigkeit = nNewValue;
	}	

	SetModifiedFlag();
}

CURRENCY CDokumentCtrl::HoleEinnahmenSumme() 
{
	CURRENCY cyResult={0,0};

	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (m_id) 
		if (((CEasyCashDoc*)m_id)->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
		{
			cyResult.int64 = pDoc->EinnahmenSumme(0) * 100;
		}

	return cyResult;
}

CURRENCY CDokumentCtrl::HoleAusgabenSumme() 
{
	CURRENCY cyResult={0,0};

	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (m_id) 
		if (((CEasyCashDoc*)m_id)->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
		{
			cyResult.int64 = pDoc->AusgabenSumme(0) * 100;
		}

	return cyResult;
}

long CDokumentCtrl::GetLaufendeBuchungsnummerBank() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return pDoc->nLaufendeBuchungsnummerFuerBank;	
	}

	return 0;
}

void CDokumentCtrl::SetLaufendeBuchungsnummerBank(long nNewValue) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		pDoc->nLaufendeBuchungsnummerFuerBank = nNewValue;	

		pDoc->SetModifiedFlag("Laufende Buchungsnummer für Bank wurde geändert");
	}
}

long CDokumentCtrl::GetLaufendeBuchungsnummerKasse() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return pDoc->nLaufendeBuchungsnummerFuerKasse;	
	}

	return 0;
}

void CDokumentCtrl::SetLaufendeBuchungsnummerKasse(long nNewValue) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		pDoc->nLaufendeBuchungsnummerFuerKasse = nNewValue;	

		pDoc->SetModifiedFlag("Laufende Buchungsnummer für Kasse wurde geändert");
	}
}

long CDokumentCtrl::HoleDokumentVersion() 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		return pDoc->Version;	
	}

	return 0;
}

BSTR CDokumentCtrl::HoleBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName) 
{
	CString strResult;

	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		char *pvalstart = GetErweiterungKey(pDoc->Erweiterung, PluginName, SchluesselName);
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

void CDokumentCtrl::SpeichereBenutzerdefWert(LPCTSTR PluginName, LPCTSTR SchluesselName, LPCTSTR Wert) 
{
	CEasyCashDoc* pDoc = (CEasyCashDoc*)m_id;
	if (pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)))
	{
		SetErweiterungKey(pDoc->Erweiterung, PluginName, SchluesselName, Wert);
		pDoc->SetModifiedFlag((CString)PluginName + " hat Daten in der Buchungsdatei hinterlegt");
	}
}
