// EasyCTX.cpp : Implementation of CEasyCTXApp and DLL registration.
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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CEasyCTXApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0xd235eec4, 0xd7db, 0x404e, { 0x85, 0xdb, 0x6, 0x42, 0xfa, 0xaa, 0xd7, 0x4d } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


const GUID CDECL CLSID_SafeItem1 =
		{ 0xed09076f, 0xb744, 0x47a1, { 0x8d, 0x60, 0xf9, 0xef, 0x9, 0x3f, 0x19, 0xf0 } };

const GUID CDECL CLSID_SafeItem2 =
		{ 0xdfa091a6, 0xd040, 0x444f, { 0x80, 0xa, 0x65, 0x6b, 0xea, 0x33, 0x61, 0xc3 } };

const GUID CDECL CLSID_SafeItem3 =
		{ 0x3bc20630, 0x93ba, 0x47b2, { 0x82, 0xd1, 0x82, 0xc, 0x7f, 0x19, 0x86, 0xa1 } };

////////////////////////////////////////////////////////////////////////////
// CEasyCTXApp::InitInstance - DLL initialization

BOOL CEasyCTXApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		;
	}

	return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CEasyCTXApp::ExitInstance - DLL termination

int CEasyCTXApp::ExitInstance()
{
	// TODO: Add your own module termination code here.

// ~CCmdTarget Problem -- dies hier hilft auch nicht?!?
/*COleMessageFilter* pFilter = AfxOleGetMessageFilter();
ULONG lCount = pFilter->m_xMessageFilter.Release();
if(lCount<=1)
pFilter->m_xMessageFilter.AddRef();
return CWinApp::ExitInstance();*/

	return COleControlModule::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    HRESULT hr;    // HResult used by Safety Functions

	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

    // Mark the control as safe for initializing.

    hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data!");
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem1, CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem2, CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem3, CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    // Mark the control as safe for scripting.

    hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls safely scriptable!");
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem1, CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem2, CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

    hr = RegisterCLSIDInCategory(CLSID_SafeItem3, CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

	return NOERROR;
}


/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;    // HResult used by Safety Functions

	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

    // Remove entries from the registry.

    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem1, CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem1, CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem2, CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem2, CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem3, CATID_SafeForInitializing);
    if (FAILED(hr))
        return hr;

    hr=UnRegisterCLSIDInCategory(CLSID_SafeItem3, CATID_SafeForScripting);
    if (FAILED(hr))
        return hr;

	return NOERROR;
}

extern void DrawAndCenterBitmap(CDC* pDC, int ResourceBitmapID, RECT *r)
{
	HDC  hdc;
	CDC  *dc;
	CDC dcMem;
	BITMAP bm;
	CBitmap cbm;

	VERIFY(hdc = pDC->m_hDC);
	VERIFY(dc = pDC);

	//----- bitmap anzeigen

	// Create a memory DC for the bitmap.
	dcMem.CreateCompatibleDC(dc);

	// Get the size of the bitmap.
	cbm.LoadBitmap(ResourceBitmapID);
	cbm.GetObject(sizeof(bm), &bm);

	// Select the bitmap into the memory DC.
	CBitmap* pbmOld;
	VERIFY(pbmOld = dcMem.SelectObject(&cbm));

	// thumbnails: Blt & center the bitmaps to the screen DC.
	int x = r->left+(r->right-r->left-bm.bmWidth)/2;
	int y = r->top+(r->bottom-r->top-bm.bmHeight)/2;
	dc->BitBlt(x, y,		// Destination
				bm.bmWidth, // Width
				bm.bmHeight,// Height
				&dcMem,     // Source DC
				0, 0,       // Source
				SRCCOPY);   // Operation

	// Done with memory DC now, so clean up.
	VERIFY(dcMem.SelectObject(pbmOld));
	VERIFY(dcMem.DeleteDC());
}


HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription)
{

    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (FAILED(hr))
        return hr;

    // Make sure the HKCR\Component Categories\{..catid...}
    // key is registered.
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ; // english

    // Make sure the provided description is not too long.
    // Only copy the first 127 characters if it is.
    int len = wcslen(catDescription);
    if (len>127)
        len = 127;
    wcsncpy(catinfo.szDescription, catDescription, len);
        // Make sure the description is null terminated.
        catinfo.szDescription[len] = '\0';

    hr = pcr->RegisterCategories(1, &catinfo);
        pcr->Release();

        return hr;
}


HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
// Register your component categories information.
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
                NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Register this category as being "implemented" by the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();
		
    return hr;
}

HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
    ICatRegister *pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
       // Unregister this category as being "implemented" by the class.
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();

    return hr;
}
