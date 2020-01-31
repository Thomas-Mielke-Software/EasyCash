// MyInternetSession.cpp: implementation of the CMyInternetSession class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PluginManager.h"
#include "MyInternetSession.h"
#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>
#include <iostream.h>
#include <stdlib.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyInternetSession::CMyInternetSession(LPCTSTR pszAppName, int nMethod, LPCTSTR pstrProxyName, LPCTSTR pstrProxyBypass, DWORD dwFlags)
	: CInternetSession(pszAppName, 1, nMethod, pstrProxyName, pstrProxyBypass, dwFlags)
{
}

void CMyInternetSession::OnStatusCallback(DWORD  dwContext , DWORD dwInternetStatus,
	LPVOID  lpvStatusInformation , DWORD  dwStatusInformationLength )
{
    // Status callbacks need thread-state protection. 
    AFX_MANAGE_STATE(AfxGetAppModuleState( ));

    CString strStatus;

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		strStatus.Format("Resolving name %s", lpvStatusInformation);
		break;

	case INTERNET_STATUS_NAME_RESOLVED:
		strStatus.Format("Name resolved %s", lpvStatusInformation);
		break;

	case INTERNET_STATUS_HANDLE_CREATED:
		strStatus.Format("Handle created");
		break;

	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		strStatus.Format("Connecting to socket address");
		break;

	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		strStatus.Format("Connected to socket address");
		break;

	case INTERNET_STATUS_SENDING_REQUEST:
		strStatus.Format("Sending request");
		break;

	case INTERNET_STATUS_REQUEST_SENT:
		strStatus.Format("Request sent");
		break;

	case INTERNET_STATUS_RECEIVING_RESPONSE:
        return;
		strStatus.Format("Receiving response");
		break;

	case INTERNET_STATUS_RESPONSE_RECEIVED:
		strStatus.Format("Response received");
		break;

	case INTERNET_STATUS_CLOSING_CONNECTION:
		strStatus.Format("Closing the connection to the server");
		break;

	case INTERNET_STATUS_CONNECTION_CLOSED:
		strStatus.Format("Connection to the server closed");
		break;

	case INTERNET_STATUS_HANDLE_CLOSING:
        return;
		strStatus.Format("Handle closed");
		break;
	
	//Check MSDN information about CInternetSession dwFlag INTERNET_FLAG_ASYNC
	//I have never been able to get this to work to my liking
	case INTERNET_STATUS_REQUEST_COMPLETE:
  		strStatus.Format("Request complete");
		break;

	case INTERNET_STATUS_REDIRECT:
		strStatus.Format("Being redirected");
		break;

	default:
		strStatus.Format("Unknown status: %d", dwInternetStatus);
		break;
	}

    ShowStatus(strStatus);

    TRACE("CMyInternetSession::OnStatusCallback: %s\n",strStatus);
}

//use this function to update any status, edit or what ever control
//that needs updating
void CMyInternetSession::ShowStatus(LPCTSTR strStatus)
{
	m_pParent->ShowStatus(strStatus);
}


//Lets get the file via http
DWORD CMyInternetSession::GetWebFile(LPCTSTR pstrAgent, LPCTSTR lpstrServer, int nPort, CString strFile)
{
	//Check what file types we will allow to be requested
	CString extension = strFile.Right(3);
/*
	if(extension == "exe")
	{
		return 0;
	}
	if(extension == "com")
	{
		return 0;
	}
	if (extension == "dll")
	{
		return 0;
	}
	if (extension == "bat")
	{
		return 0;
	}
	if (extension == "sys")
	{
		return 0;
	}
	if (extension == "inf")
	{
		return 0;
	}
*/
	DWORD dwAccessType = PRE_CONFIG_INTERNET_ACCESS;
	DWORD dwHttpRequestFlags = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_DONT_CACHE;

	/*string containing the application name that is used to refer
	  client making the request. If this NULL the frame work will
	  call  the global function AfxGetAppName which returns the application
	  name.*/
	//LPCTSTR pstrAgent = NULL;

	//the verb we will be using for this connection
	//if NULL then GET is assumed
	LPCTSTR pstrVerb = "GET";
	
	//the address of the url in the request was obtained from
	LPCTSTR pstrReferer = NULL;

	//Http version we are using; NULL = HTTP/1.0
	LPCTSTR pstrVersion = NULL;

	//For the Accept request headers if we need them later on
	//LPCTSTR pstrAcceptTypes = "Accept: audio/x-aiff, audio/basic, audio/midi, audio/mpeg, audio/wav, image/jpeg, image/gif, image/jpg, image/png, image/mng, image/bmp, text/plain, text/html, text/htm\r\n";
	LPCTSTR pstrAcceptTypes = NULL;
	CString szHeaders = "Accept: audio/x-aiff, audio/basic, audio/midi, audio/mpeg, audio/wav, image/jpeg, image/gif, image/jpg, image/png, image/mng, image/bmp, text/plain, text/html, text/htm\r\n";

	//the server port we need changed
	//nPort = INTERNET_INVALID_PORT_NUMBER
	unsigned short usPort = nPort;
	
	//Username we will use if a secure site comes into play
	LPCTSTR pstrUserName = NULL; 
	//The password we will use
	LPCTSTR pstrPassword = NULL;

	//CInternetSession flags if we need them
	//DWORD dwFlags = INTERNET_FLAG_ASYNC;
	DWORD dwFlags = NULL;

	//Proxy setting if we need them
	LPCTSTR pstrProxyName = NULL;
	LPCTSTR pstrProxyBypass = NULL;

	CMyInternetSession	session(pstrAgent, dwAccessType, pstrProxyName, pstrProxyBypass, dwFlags);

	//Set any CInternetSession options we  may need
	int ntimeOut = 30;
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT,1000* ntimeOut);
	session.SetOption(INTERNET_OPTION_CONNECT_BACKOFF,1000);
	session.SetOption(INTERNET_OPTION_CONNECT_RETRIES,1);

	//Enable or disable status callbacks
	session.EnableStatusCallback(TRUE);

	CHttpConnection*	pServer = NULL;   
	CHttpFile* pFile = NULL;
	DWORD dwRet;
	try {		

		pServer = session.GetHttpConnection(lpstrServer, usPort, 
			pstrUserName, pstrPassword);
		pFile = pServer->OpenRequest(pstrVerb, strFile, pstrReferer, 
			1, &pstrAcceptTypes, pstrVersion, dwHttpRequestFlags);

		pFile->AddRequestHeaders(szHeaders);
		pFile->AddRequestHeaders("User-Agent: GetWebFile/1.0\r\n", HTTP_ADDREQ_FLAG_ADD_IF_NEW);
		pFile->SendRequest();

		pFile->QueryInfoStatusCode(dwRet);//Check wininet.h for info
										  //about the status codes

		if (dwRet == HTTP_STATUS_DENIED)
		{
			return dwRet;
		}

		if (dwRet == HTTP_STATUS_MOVED ||
			dwRet == HTTP_STATUS_REDIRECT ||
			dwRet == HTTP_STATUS_REDIRECT_METHOD)
		{
			CString strNewAddress;
			//again check wininet.h for info on the query info codes
			//there is alot one can do and re-act to based on these codes
			pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewAddress);
			
			int nPos = strNewAddress.Find(_T("Location: "));
			if (nPos == -1)
				{
					return 0;
				}
			strNewAddress = strNewAddress.Mid(nPos + 10);
			nPos = strNewAddress.Find('\n');
			if (nPos > 0)
				strNewAddress = strNewAddress.Left(nPos);

			pFile->Close();      
			delete pFile;
			pServer->Close();  
			delete pServer;

			CString strServerName;
			CString strObject;
			INTERNET_PORT nNewPort;
			DWORD dwServiceType;

			if (!AfxParseURL(strNewAddress, dwServiceType, strServerName, strObject, nNewPort))
				{
					return 0;
				}

			pServer = session.GetHttpConnection(strServerName, nNewPort, 
				pstrUserName, pstrPassword);
			pFile = pServer->OpenRequest(pstrVerb, strObject, 
				pstrReferer, 1, &pstrAcceptTypes, pstrVersion, dwHttpRequestFlags);
			pFile->AddRequestHeaders(szHeaders);
			pFile->SendRequest();

			pFile->QueryInfoStatusCode(dwRet);
			if (dwRet != HTTP_STATUS_OK)
				{
					return dwRet;
				}
		}

		if(dwRet == HTTP_STATUS_OK)
			{
			int len = pFile->GetLength();
			char buf[2000];
			int numread;
			CString filepath;
			filepath.Format("D:\\Work\\Easycash\\debug\\Files\\%s", strFile);
			CFile myfile(filepath, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
			while ((numread = pFile->Read(buf,sizeof(buf)-1)) > 0)
			{
				buf[numread] = '\0';
				strFile += buf;
				myfile.Write(buf, numread);
				m_pParent->m_ctrlProgress.StepIt();
				m_pParent->PeekAndPump();
			}
			myfile.Close();
		}
		pFile->Close();      
		delete pFile;
		pServer->Close();  
		delete pServer;
		session.Close();
		m_pParent->m_ctrlProgress.SetPos(0);
	}

	catch (CInternetException* pEx) 
	{
      // catch any exceptions from WinINet      
		TCHAR szErr[1024];
		szErr[0] = '\0';
        if(!pEx->GetErrorMessage(szErr, 1024))
			strcpy(szErr,"Some crazy unknown error");
		TRACE("File transfer failed!! - %s",szErr);      
		pEx->Delete();
		if(pFile)
			delete pFile;
		if(pServer)
			delete pServer;
		session.Close(); 
		return 0;
	}

	return dwRet;
}
