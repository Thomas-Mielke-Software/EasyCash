// EinstellungCtl.cpp : Implementation of the CEinstellungCtrl ActiveX Control class.
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
#include "EinstellungCtl.h"
#include "EinstellungPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CEinstellungCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CEinstellungCtrl, COleControl)
	//{{AFX_MSG_MAP(CEinstellungCtrl)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CEinstellungCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CEinstellungCtrl)
	DISP_FUNCTION(CEinstellungCtrl, "HoleEinstellung", HoleEinstellung, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CEinstellungCtrl, "SpeichereEinstellung", SpeichereEinstellung, VT_EMPTY, VTS_BSTR VTS_BSTR)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CEinstellungCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CEinstellungCtrl, COleControl)
	//{{AFX_EVENT_MAP(CEinstellungCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CEinstellungCtrl, 1)
	PROPPAGEID(CEinstellungPropPage::guid)
END_PROPPAGEIDS(CEinstellungCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CEinstellungCtrl, "EASYCTX.EinstellungCtrl.1",
	0xdfa091a6, 0xd040, 0x444f, 0x80, 0xa, 0x65, 0x6b, 0xea, 0x33, 0x61, 0xc3)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CEinstellungCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DEinstellung =
		{ 0x8bdfadf1, 0x8317, 0x4eba, { 0x88, 0xbb, 0xe5, 0xbc, 0x12, 0x90, 0xdd, 0x72 } };
const IID BASED_CODE IID_DEinstellungEvents =
		{ 0x9910463d, 0x5929, 0x4e27, { 0x83, 0x68, 0x4c, 0xc0, 0x93, 0x2a, 0xcf, 0x62 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwEinstellungOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CEinstellungCtrl, IDS_EINSTELLUNG, _dwEinstellungOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl::CEinstellungCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CEinstellungCtrl

BOOL CEinstellungCtrl::CEinstellungCtrlFactory::UpdateRegistry(BOOL bRegister)
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
			IDS_EINSTELLUNG,
			IDB_EINSTELLUNG,
			afxRegInsertable | afxRegApartmentThreading,
			_dwEinstellungOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl::CEinstellungCtrl - Constructor

CEinstellungCtrl::CEinstellungCtrl()
{
	InitializeIIDs(&IID_DEinstellung, &IID_DEinstellungEvents);

	// TODO: Initialize your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl::~CEinstellungCtrl - Destructor

CEinstellungCtrl::~CEinstellungCtrl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl::OnDraw - Drawing function

void CEinstellungCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	CRect r = rcBounds;
	DrawAndCenterBitmap(pdc, IDB_EINSTELLUNG, &r);
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl::DoPropExchange - Persistence support

void CEinstellungCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl::OnResetState - Reset control to default state

void CEinstellungCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl::AboutBox - Display an "About" box to the user

void CEinstellungCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_EINSTELLUNG);
	if (dlgAbout.DoModal() == IDCANCEL)
		ShellExecute(m_hWnd, "open", "http://easyct.no-ip.org/readarticle.php?article_id=23#Einstellung", 
			NULL, ".", SW_SHOWNORMAL);		
}


/////////////////////////////////////////////////////////////////////////////
// CEinstellungCtrl message handlers

#define MAX_LAENGE_FUER_INI_EINTRAG 10000

BSTR CEinstellungCtrl::HoleEinstellung(LPCTSTR ID) 
{
	CString strResult;

	char EasyCashIniFilenameBuffer[1000];
	GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer));

	if (*ID == '[')	// implizite ini-Sektion der Form "[sektion]key" ?
	{
		const char *cp;
		cp = strchr(ID, ']');
		if (cp)
		{
			char sektion[10000];
			strncpy(sektion, ID+1, cp-ID-1);
			sektion[cp-ID-1] = '\0';

			++cp;	// nun auf dem key

			GetPrivateProfileString(sektion, cp, "", strResult.GetBuffer(MAX_LAENGE_FUER_INI_EINTRAG), MAX_LAENGE_FUER_INI_EINTRAG, EasyCashIniFilenameBuffer);
		}
	}
	else
		GetPrivateProfileString(IniSektion(ID), !strcmp(IniSektion(ID), "Finanzamt") || !strcmp(IniSektion(ID), "EinnahmenRechnungsposten") || !strcmp(IniSektion(ID), "AusgabenRechnungsposten")? ID+1 : ID, "", strResult.GetBuffer(MAX_LAENGE_FUER_INI_EINTRAG), MAX_LAENGE_FUER_INI_EINTRAG, EasyCashIniFilenameBuffer);

	strResult.ReleaseBuffer();
	return strResult.AllocSysString();
}

void CEinstellungCtrl::SpeichereEinstellung(LPCTSTR ID, LPCTSTR Wert) 
{
	char EasyCashIniFilenameBuffer[1000];
	GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer));

	if (*ID == '[')	// implizite ini-Sektion der Form "[sektion]key" ?
	{
		const char *cp;
		if (cp = strchr(ID, ']'))
		{
			char sektion[10000];
			strncpy(sektion, ID+1, cp-ID-1);
			sektion[cp-ID-1] = '\0';

			++cp;	// nun auf dem key

			WritePrivateProfileString(sektion, cp, Wert, EasyCashIniFilenameBuffer);
		}
	}
	else
		WritePrivateProfileString(IniSektion(ID), !strcmp(IniSektion(ID), "Finanzamt") || !strcmp(IniSektion(ID), "EinnahmenRechnungsposten") || !strcmp(IniSektion(ID), "AusgabenRechnungsposten") ? ID+1 : ID, Wert, EasyCashIniFilenameBuffer);
}
