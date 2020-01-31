// EasyCTXP.cpp : Defines the initialization routines for the DLL.
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
#include "EasyCTXP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CEasyCTXPApp

BEGIN_MESSAGE_MAP(CEasyCTXPApp, CWinApp)
END_MESSAGE_MAP()


// CEasyCTXPApp construction

CEasyCTXPApp::CEasyCTXPApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CEasyCTXPApp object

CEasyCTXPApp theApp;


// CEasyCTXPApp initialization

BOOL CEasyCTXPApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

// sFolder sollte MAX_FILE groß sein!
extern "C" BOOL SelectFolder(LPSTR sFolder, LPCTSTR sTitle)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	BOOL bRet = TRUE;

	IFileDialog *pfd;
	LPWSTR pwstrPath;
	HRESULT hr;
	if (SUCCEEDED(hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
	{
		DWORD dwOptions;
		hr = pfd->GetOptions(&dwOptions);
		if (SUCCEEDED(hr))
			hr = pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);	// put IFileDialog in folder mode
		if (SUCCEEDED(hr))
		{
			WCHAR wcTitle[MAX_PATH+1];
			if (MultiByteToWideChar(CP_ACP, 0, sTitle, (int)min(strlen(sTitle), MAX_PATH), wcTitle, sizeof(MAX_PATH + 1)))
			{
				wcTitle[min(strlen(sTitle), MAX_PATH)] = '\0';
				pfd->SetTitle(wcTitle);			// Set dialog title
			}

			char *cp = sFolder;
			WCHAR wcDefaultPath[MAX_PATH+1];
			IShellItem *psi;
			if (MultiByteToWideChar(CP_ACP, 0, cp, (int)strlen(cp), wcDefaultPath, MAX_PATH + 1))
			{
				wcDefaultPath[strlen(cp)] = '\0';
				if (SUCCEEDED(::SHCreateItemFromParsingName(wcDefaultPath, NULL, IID_PPV_ARGS(&psi))))
				{
					hr = pfd->SetFileName(wcDefaultPath);
					hr = pfd->SetFolder(psi);
					psi->Release();
				}
			}
		}
		if (SUCCEEDED(hr))
			hr = pfd->Show(AfxGetMainWnd()->GetSafeHwnd());
		if (SUCCEEDED(hr))
		{
			IShellItem *psi;
			if (SUCCEEDED(pfd->GetResult(&psi)))
			{
				if(SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pwstrPath)))
				{
					int nSize = (int)wcslen(pwstrPath);
					WideCharToMultiByte(CP_ACP, 0, pwstrPath, nSize, sFolder, MAX_PATH+1, NULL, NULL);
					sFolder[nSize] = '\0';
					::CoTaskMemFree(pwstrPath);
				}
				else
				{
					AfxMessageBox("IShellItem::GetDisplayName() failed.");
					bRet = FALSE;
				}

				psi->Release();
			}
			else
			{
				AfxMessageBox("IFileDialog failed.");
				bRet = FALSE;
			}
		}
		pfd->Release();
	}

	if(!SUCCEEDED(hr))
		bRet = FALSE;		
	
	return bRet;
}